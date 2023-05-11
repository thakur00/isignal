/**
 * Copyright 2013-2022 iSignal Research Labs Pvt Ltd.
 *
 * This file is part of isrRAN.
 *
 * isrRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * isrRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "isrran/phy/ch_estimation/refsignal_ul.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/common/zc_sequence.h"
#include "isrran/phy/dft/dft_precoding.h"
#include "isrran/phy/phch/pucch.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/primes.h"
#include "isrran/phy/utils/vector.h"

// n_dmrs_2 table 5.5.2.1.1-1 from 36.211
static const uint32_t n_dmrs_2[8] = {0, 6, 3, 4, 2, 8, 10, 9};

// n_dmrs_1 table 5.5.2.1.1-2 from 36.211
static const uint32_t n_dmrs_1[8] = {0, 2, 3, 4, 6, 8, 9, 10};

/* Orthogonal sequences for PUCCH formats 1a, 1b and 1c. Table 5.5.2.2.1-2
 */
static const float w_arg_pucch_format1_cpnorm[3][3] = {{0, 0, 0},
                                                       {0, 2 * M_PI / 3, 4 * M_PI / 3},
                                                       {0, 4 * M_PI / 3, 2 * M_PI / 3}};

static const float w_arg_pucch_format1_cpext[3][2] = {{0, 0}, {0, M_PI}, {0, 0}};

static const float w_arg_pucch_format2_cpnorm[2] = {0, 0};
static const float w_arg_pucch_format2_cpext[1]  = {0};

static const uint32_t pucch_dmrs_symbol_format1_cpnorm[3] = {2, 3, 4};
static const uint32_t pucch_dmrs_symbol_format1_cpext[2]  = {2, 3};
static const uint32_t pucch_dmrs_symbol_format2_cpnorm[2] = {1, 5};
static const uint32_t pucch_dmrs_symbol_format2_cpext[1]  = {3};

/* Table 5.5.3.3-1: Frame structure type 1 sounding reference signal subframe configuration. */
static const uint32_t T_sfc[15]     = {1, 2, 2, 5, 5, 5, 5, 5, 5, 10, 10, 10, 10, 10, 10};
static const uint32_t Delta_sfc1[7] = {0, 0, 1, 0, 1, 2, 3};
static const uint32_t Delta_sfc2[4] = {0, 1, 2, 3};

static const uint32_t m_isr_b[4][4][8] = {{/* m_isr for 6<n_rb<40. Table 5.5.3.2-1 */
                                           {36, 32, 24, 20, 16, 12, 8, 4},
                                           {12, 16, 4, 4, 4, 4, 4, 4},
                                           {4, 8, 4, 4, 4, 4, 4, 4},
                                           {4, 4, 4, 4, 4, 4, 4, 4}},
                                          {/* m_isr for 40<n_rb<60. Table 5.5.3.2-2 */
                                           {48, 48, 40, 36, 32, 24, 20, 16},
                                           {24, 16, 20, 12, 16, 4, 4, 4},
                                           {12, 8, 4, 4, 8, 4, 4, 4},
                                           {4, 4, 4, 4, 4, 4, 4, 4}},
                                          {/* m_isr for 60<n_rb<80. Table 5.5.3.2-3 */
                                           {72, 64, 60, 48, 48, 40, 36, 32},
                                           {24, 32, 20, 24, 16, 20, 12, 16},
                                           {12, 16, 4, 12, 8, 4, 4, 8},
                                           {4, 4, 4, 4, 4, 4, 4, 4}},

                                          {/* m_isr for 80<n_rb<110. Table 5.5.3.2-4 */
                                           {96, 96, 80, 72, 64, 60, 48, 48},
                                           {48, 32, 40, 24, 32, 20, 24, 16},
                                           {24, 16, 20, 12, 16, 4, 12, 8},
                                           {4, 4, 4, 4, 4, 4, 4, 4}}};

/* Same tables for Nb */
static const uint32_t Nb[4][4][8] = {
    {{1, 1, 1, 1, 1, 1, 1, 1}, {3, 2, 6, 5, 4, 3, 2, 1}, {3, 2, 1, 1, 1, 1, 1, 1}, {1, 2, 1, 1, 1, 1, 1, 1}},
    {{1, 1, 1, 1, 1, 1, 1, 1}, {2, 3, 2, 3, 2, 6, 5, 4}, {2, 2, 5, 3, 2, 1, 1, 1}, {3, 2, 1, 1, 2, 1, 1, 1}},
    {{1, 1, 1, 1, 1, 1, 1, 1}, {3, 2, 3, 2, 3, 2, 3, 2}, {2, 2, 5, 2, 2, 5, 3, 2}, {3, 4, 1, 3, 2, 1, 1, 2}},
    {{1, 1, 1, 1, 1, 1, 1, 1}, {2, 3, 2, 3, 2, 3, 2, 3}, {2, 2, 2, 2, 2, 5, 2, 2}, {6, 4, 5, 3, 4, 1, 3, 2}}};

/** Computes n_prs values used to compute alpha as defined in 5.5.2.1.1 of 36.211 */
static int generate_n_prs(isrran_refsignal_ul_t* q)
{
  /* Calculate n_prs */
  uint32_t c_init;

  isrran_sequence_t seq;
  bzero(&seq, sizeof(isrran_sequence_t));

  for (uint32_t delta_ss = 0; delta_ss < ISRRAN_NOF_DELTA_SS; delta_ss++) {
    c_init = ((q->cell.id / 30) << 5) + (((q->cell.id % 30) + delta_ss) % 30);
    if (isrran_sequence_LTE_pr(&seq, 8 * ISRRAN_CP_NSYMB(q->cell.cp) * 20, c_init)) {
      return ISRRAN_ERROR;
    }
    for (uint32_t ns = 0; ns < ISRRAN_NSLOTS_X_FRAME; ns++) {
      uint32_t n_prs = 0;
      for (int i = 0; i < 8; i++) {
        n_prs += (seq.c[8 * ISRRAN_CP_NSYMB(q->cell.cp) * ns + i] << i);
      }
      q->n_prs_pusch[delta_ss][ns] = n_prs;
    }
  }
  isrran_sequence_free(&seq);

  return ISRRAN_SUCCESS;
}

static int generate_isrran_sequence_hopping_v(isrran_refsignal_ul_t* q)
{
  isrran_sequence_t seq;
  bzero(&seq, sizeof(isrran_sequence_t));

  for (uint32_t ns = 0; ns < ISRRAN_NSLOTS_X_FRAME; ns++) {
    for (uint32_t delta_ss = 0; delta_ss < ISRRAN_NOF_DELTA_SS; delta_ss++) {
      if (isrran_sequence_LTE_pr(&seq, 20, ((q->cell.id / 30) << 5) + ((q->cell.id % 30) + delta_ss) % 30)) {
        return ISRRAN_ERROR;
      }
      q->v_pusch[ns][delta_ss] = seq.c[ns];
    }
  }
  isrran_sequence_free(&seq);
  return ISRRAN_SUCCESS;
}

/** Initializes isrran_refsignal_ul_t object according to 3GPP 36.211 5.5
 *
 */
int isrran_refsignal_ul_set_cell(isrran_refsignal_ul_t* q, isrran_cell_t cell)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && isrran_cell_isvalid(&cell)) {
    if (cell.id != q->cell.id || q->cell.nof_prb == 0) {
      q->cell = cell;

      // Precompute n_prs
      if (generate_n_prs(q)) {
        return ISRRAN_ERROR;
      }

      // Precompute group hopping values u.
      if (isrran_group_hopping_f_gh(q->f_gh, q->cell.id)) {
        return ISRRAN_ERROR;
      }

      // Precompute sequence hopping values v. Uses f_ss_pusch
      if (generate_isrran_sequence_hopping_v(q)) {
        return ISRRAN_ERROR;
      }

      if (isrran_pucch_n_cs_cell(q->cell, q->n_cs_cell)) {
        return ISRRAN_ERROR;
      }
    }
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

/* Calculates alpha according to 5.5.2.1.1 of 36.211 */
static float pusch_alpha(isrran_refsignal_ul_t*             q,
                         isrran_refsignal_dmrs_pusch_cfg_t* cfg,
                         uint32_t                           cyclic_shift_for_dmrs,
                         uint32_t                           ns)
{
  uint32_t n_dmrs_2_val = n_dmrs_2[cyclic_shift_for_dmrs];
  uint32_t n_cs         = (n_dmrs_1[cfg->cyclic_shift] + n_dmrs_2_val + q->n_prs_pusch[cfg->delta_ss][ns]) % 12;

  return 2 * M_PI * (n_cs) / 12;
}

static bool pusch_cfg_isvalid(isrran_refsignal_ul_t* q, isrran_refsignal_dmrs_pusch_cfg_t* cfg, uint32_t nof_prb)
{
  if (cfg->cyclic_shift < ISRRAN_NOF_CSHIFT && cfg->delta_ss < ISRRAN_NOF_DELTA_SS && nof_prb <= q->cell.nof_prb) {
    return true;
  } else {
    return false;
  }
}

void isrran_refsignal_dmrs_pusch_put(isrran_refsignal_ul_t* q,
                                     isrran_pusch_cfg_t*    pusch_cfg,
                                     cf_t*                  r_pusch,
                                     cf_t*                  sf_symbols)
{
  for (uint32_t ns_idx = 0; ns_idx < 2; ns_idx++) {
    INFO("Putting DMRS to n_prb: %d, L: %d, ns_idx: %d",
         pusch_cfg->grant.n_prb_tilde[ns_idx],
         pusch_cfg->grant.L_prb,
         ns_idx);
    uint32_t L = ISRRAN_REFSIGNAL_UL_L(ns_idx, q->cell.cp);
    memcpy(&sf_symbols[ISRRAN_RE_IDX(q->cell.nof_prb, L, pusch_cfg->grant.n_prb_tilde[ns_idx] * ISRRAN_NRE)],
           &r_pusch[ns_idx * ISRRAN_NRE * pusch_cfg->grant.L_prb],
           pusch_cfg->grant.L_prb * ISRRAN_NRE * sizeof(cf_t));
  }
}

void isrran_refsignal_dmrs_pusch_get(isrran_refsignal_ul_t* q,
                                     isrran_pusch_cfg_t*    pusch_cfg,
                                     cf_t*                  sf_symbols,
                                     cf_t*                  r_pusch)
{
  for (uint32_t ns_idx = 0; ns_idx < 2; ns_idx++) {
    INFO("Getting DMRS from n_prb: %d, L: %d, ns_idx: %d",
         pusch_cfg->grant.n_prb_tilde[ns_idx],
         pusch_cfg->grant.L_prb,
         ns_idx);
    uint32_t L = ISRRAN_REFSIGNAL_UL_L(ns_idx, q->cell.cp);
    memcpy(&r_pusch[ns_idx * ISRRAN_NRE * pusch_cfg->grant.L_prb],
           &sf_symbols[ISRRAN_RE_IDX(q->cell.nof_prb, L, pusch_cfg->grant.n_prb_tilde[ns_idx] * ISRRAN_NRE)],
           pusch_cfg->grant.L_prb * ISRRAN_NRE * sizeof(cf_t));
  }
}

/* Computes r sequence */
static void compute_r(isrran_refsignal_ul_t*             q,
                      isrran_refsignal_dmrs_pusch_cfg_t* cfg,
                      uint32_t                           nof_prb,
                      uint32_t                           ns,
                      uint32_t                           delta_ss,
                      float                              alpha,
                      cf_t*                              sequence)
{
  // Get group hopping number u
  uint32_t f_gh = 0;
  if (cfg->group_hopping_en) {
    f_gh = q->f_gh[ns];
  }
  uint32_t u = (f_gh + (q->cell.id % 30) + delta_ss) % 30;

  // Get sequence hopping number v
  uint32_t v = 0;
  if (nof_prb >= 6 && cfg->sequence_hopping_en) {
    v = q->v_pusch[ns][cfg->delta_ss];
  }

  // Compute signal argument
  isrran_zc_sequence_generate_lte(u, v, alpha, nof_prb, sequence);
}

int isrran_refsignal_dmrs_pusch_pregen_init(isrran_refsignal_ul_dmrs_pregen_t* pregen, uint32_t max_prb)
{
  pregen->max_prb = max_prb;

  for (uint32_t sf_idx = 0; sf_idx < ISRRAN_NOF_SF_X_FRAME; sf_idx++) {
    for (uint32_t cs = 0; cs < ISRRAN_NOF_CSHIFT; cs++) {
      pregen->r[cs][sf_idx] = (cf_t**)calloc(sizeof(cf_t*), max_prb + 1);
      if (pregen->r[cs][sf_idx]) {
        for (uint32_t n = 0; n <= max_prb; n++) {
          if (isrran_dft_precoding_valid_prb(n)) {
            pregen->r[cs][sf_idx][n] = isrran_vec_cf_malloc(n * 2 * ISRRAN_NRE);
            if (!pregen->r[cs][sf_idx][n]) {
              return ISRRAN_ERROR;
            }
          }
        }
      } else {
        return ISRRAN_ERROR;
      }
    }
  }
  return ISRRAN_SUCCESS;
}

int isrran_refsignal_dmrs_pusch_pregen(isrran_refsignal_ul_t*             q,
                                       isrran_refsignal_ul_dmrs_pregen_t* pregen,
                                       isrran_refsignal_dmrs_pusch_cfg_t* cfg)
{
  for (uint32_t sf_idx = 0; sf_idx < ISRRAN_NOF_SF_X_FRAME; sf_idx++) {
    for (uint32_t cs = 0; cs < ISRRAN_NOF_CSHIFT; cs++) {
      if (pregen->r[cs][sf_idx]) {
        for (uint32_t n = 1; n <= q->cell.nof_prb; n++) {
          if (isrran_dft_precoding_valid_prb(n)) {
            if (pregen->r[cs][sf_idx][n]) {
              if (isrran_refsignal_dmrs_pusch_gen(q, cfg, n, sf_idx, cs, pregen->r[cs][sf_idx][n])) {
                return ISRRAN_ERROR;
              }
            } else {
              return ISRRAN_ERROR;
            }
          }
        }
      } else {
        return ISRRAN_ERROR;
      }
    }
  }
  return ISRRAN_SUCCESS;
}

void isrran_refsignal_dmrs_pusch_pregen_free(isrran_refsignal_ul_t* q, isrran_refsignal_ul_dmrs_pregen_t* pregen)
{
  for (uint32_t sf_idx = 0; sf_idx < ISRRAN_NOF_SF_X_FRAME; sf_idx++) {
    for (uint32_t cs = 0; cs < ISRRAN_NOF_CSHIFT; cs++) {
      if (pregen->r[cs][sf_idx]) {
        for (uint32_t n = 0; n <= pregen->max_prb; n++) {
          if (pregen->r[cs][sf_idx][n]) {
            free(pregen->r[cs][sf_idx][n]);
          }
        }
        free(pregen->r[cs][sf_idx]);
      }
    }
  }
}

int isrran_refsignal_dmrs_pusch_pregen_put(isrran_refsignal_ul_t*             q,
                                           isrran_ul_sf_cfg_t*                sf_cfg,
                                           isrran_refsignal_ul_dmrs_pregen_t* pregen,
                                           isrran_pusch_cfg_t*                pusch_cfg,
                                           cf_t*                              sf_symbols)
{
  uint32_t sf_idx = sf_cfg->tti % 10;

  if (isrran_dft_precoding_valid_prb(pusch_cfg->grant.L_prb) && pusch_cfg->grant.n_dmrs < ISRRAN_NOF_CSHIFT) {
    isrran_refsignal_dmrs_pusch_put(
        q, pusch_cfg, pregen->r[pusch_cfg->grant.n_dmrs][sf_idx][pusch_cfg->grant.L_prb], sf_symbols);
    return ISRRAN_SUCCESS;
  } else {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }
}

/* Generate DMRS for PUSCH signal according to 5.5.2.1 of 36.211 */
int isrran_refsignal_dmrs_pusch_gen(isrran_refsignal_ul_t*             q,
                                    isrran_refsignal_dmrs_pusch_cfg_t* cfg,
                                    uint32_t                           nof_prb,
                                    uint32_t                           sf_idx,
                                    uint32_t                           cyclic_shift_for_dmrs,
                                    cf_t*                              r_pusch)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (pusch_cfg_isvalid(q, cfg, nof_prb)) {
    ret = ISRRAN_ERROR;

    for (uint32_t ns = 2 * sf_idx; ns < 2 * (sf_idx + 1); ns++) {
      // Add cyclic prefix alpha
      float alpha = pusch_alpha(q, cfg, cyclic_shift_for_dmrs, ns);

      compute_r(q, cfg, nof_prb, ns, cfg->delta_ss, alpha, &r_pusch[(ns % 2) * ISRRAN_NRE * nof_prb]);
    }
    ret = 0;
  }
  return ret;
}

/* Number of PUCCH demodulation reference symbols per slot N_rs_pucch tABLE 5.5.2.2.1-1 36.211 */
uint32_t isrran_refsignal_dmrs_N_rs(isrran_pucch_format_t format, isrran_cp_t cp)
{
  switch (format) {
    case ISRRAN_PUCCH_FORMAT_1:
    case ISRRAN_PUCCH_FORMAT_1A:
    case ISRRAN_PUCCH_FORMAT_1B:
      if (ISRRAN_CP_ISNORM(cp)) {
        return 3;
      } else {
        return 2;
      }
    case ISRRAN_PUCCH_FORMAT_2:
    case ISRRAN_PUCCH_FORMAT_3:
      if (ISRRAN_CP_ISNORM(cp)) {
        return 2;
      } else {
        return 1;
      }
    case ISRRAN_PUCCH_FORMAT_2A:
    case ISRRAN_PUCCH_FORMAT_2B:
      return 2;
    default:
      ERROR("DMRS Nof RS: Unsupported format %d", format);
      return 0;
  }
  return 0;
}

/* Table 5.5.2.2.2-1: Demodulation reference signal location for different PUCCH formats. 36.211 */
uint32_t isrran_refsignal_dmrs_pucch_symbol(uint32_t m, isrran_pucch_format_t format, isrran_cp_t cp)
{
  switch (format) {
    case ISRRAN_PUCCH_FORMAT_1:
    case ISRRAN_PUCCH_FORMAT_1A:
    case ISRRAN_PUCCH_FORMAT_1B:
      if (ISRRAN_CP_ISNORM(cp)) {
        if (m < 3) {
          return pucch_dmrs_symbol_format1_cpnorm[m];
        }
      } else {
        if (m < 2) {
          return pucch_dmrs_symbol_format1_cpext[m];
        }
      }
      break;
    case ISRRAN_PUCCH_FORMAT_2:
    case ISRRAN_PUCCH_FORMAT_3:
      if (ISRRAN_CP_ISNORM(cp)) {
        if (m < 2) {
          return pucch_dmrs_symbol_format2_cpnorm[m];
        }
      } else {
        if (m < 1) {
          return pucch_dmrs_symbol_format2_cpext[m];
        }
      }
      break;
    case ISRRAN_PUCCH_FORMAT_2A:
    case ISRRAN_PUCCH_FORMAT_2B:
      if (m < 2) {
        return pucch_dmrs_symbol_format2_cpnorm[m];
      }
      break;
    default:
      ERROR("DMRS Symbol indexes: Unsupported format %d", format);
      return 0;
  }
  return 0;
}

/* Generates DMRS for PUCCH according to 5.5.2.2 in 36.211 */
int isrran_refsignal_dmrs_pucch_gen(isrran_refsignal_ul_t* q,
                                    isrran_ul_sf_cfg_t*    sf,
                                    isrran_pucch_cfg_t*    cfg,
                                    cf_t*                  r_pucch)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q && r_pucch) {
    ret = ISRRAN_ERROR;

    uint32_t N_rs = isrran_refsignal_dmrs_N_rs(cfg->format, q->cell.cp);

    uint32_t sf_idx = sf->tti % 10;

    cf_t z_m_1 = 1.0;
    if (cfg->format == ISRRAN_PUCCH_FORMAT_2A || cfg->format == ISRRAN_PUCCH_FORMAT_2B) {
      isrran_pucch_format2ab_mod_bits(cfg->format, cfg->pucch2_drs_bits, &z_m_1);
    }

    for (uint32_t ns = 2 * sf_idx; ns < 2 * (sf_idx + 1); ns++) {
      // Get group hopping number u
      uint32_t f_gh = 0;
      if (cfg->group_hopping_en) {
        f_gh = q->f_gh[ns];
      }
      uint32_t u = (f_gh + (q->cell.id % 30)) % 30;

      for (uint32_t m = 0; m < N_rs; m++) {
        uint32_t n_oc = 0;

        uint32_t l = isrran_refsignal_dmrs_pucch_symbol(m, cfg->format, q->cell.cp);
        // Add cyclic prefix alpha
        float alpha = 0.0;
        if (cfg->format < ISRRAN_PUCCH_FORMAT_2) {
          alpha = isrran_pucch_alpha_format1(q->n_cs_cell, cfg, q->cell.cp, true, ns, l, &n_oc, NULL);
        } else {
          alpha = isrran_pucch_alpha_format2(q->n_cs_cell, cfg, ns, l);
        }

        // Choose number of symbols and orthogonal sequence from Tables 5.5.2.2.1-1 to -3
        const float* w = NULL;
        switch (cfg->format) {
          case ISRRAN_PUCCH_FORMAT_1:
          case ISRRAN_PUCCH_FORMAT_1A:
          case ISRRAN_PUCCH_FORMAT_1B:
            if (ISRRAN_CP_ISNORM(q->cell.cp)) {
              w = w_arg_pucch_format1_cpnorm[n_oc];
            } else {
              w = w_arg_pucch_format1_cpext[n_oc];
            }
            break;
          case ISRRAN_PUCCH_FORMAT_2:
          case ISRRAN_PUCCH_FORMAT_3:
            if (ISRRAN_CP_ISNORM(q->cell.cp)) {
              w = w_arg_pucch_format2_cpnorm;
            } else {
              w = w_arg_pucch_format2_cpext;
            }
            break;
          case ISRRAN_PUCCH_FORMAT_2A:
          case ISRRAN_PUCCH_FORMAT_2B:
            w = w_arg_pucch_format2_cpnorm;
            break;
          default:
            ERROR("DMRS Generator: Unsupported format %d", cfg->format);
            return ISRRAN_ERROR;
        }

        cf_t* r_sequence = &r_pucch[(ns % 2) * ISRRAN_NRE * N_rs + m * ISRRAN_NRE];
        isrran_zc_sequence_generate_lte(u, 0, alpha, 1, r_sequence);

        cf_t z_m = cexpf(I * w[m]);
        if (m == 1) {
          z_m *= z_m_1;
        }
        isrran_vec_sc_prod_ccc(r_sequence, z_m, r_sequence, ISRRAN_NRE);
      }
    }
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

int isrran_refsignal_dmrs_pucch_cp(isrran_refsignal_ul_t* q,
                                   isrran_pucch_cfg_t*    cfg,
                                   cf_t*                  source,
                                   cf_t*                  dest,
                                   bool                   source_is_grid)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q && source && dest) {
    uint32_t nsymbols = ISRRAN_CP_ISNORM(q->cell.cp) ? ISRRAN_CP_NORM_NSYMB : ISRRAN_CP_EXT_NSYMB;

    uint32_t N_rs = isrran_refsignal_dmrs_N_rs(cfg->format, q->cell.cp);
    for (uint32_t ns = 0; ns < 2; ns++) {
      // Determine n_prb
      uint32_t n_prb = isrran_pucch_n_prb(&q->cell, cfg, ns);

      for (uint32_t i = 0; i < N_rs; i++) {
        uint32_t l = isrran_refsignal_dmrs_pucch_symbol(i, cfg->format, q->cell.cp);
        if (!source_is_grid) {
          memcpy(&dest[ISRRAN_RE_IDX(q->cell.nof_prb, l + ns * nsymbols, n_prb * ISRRAN_NRE)],
                 &source[ns * N_rs * ISRRAN_NRE + i * ISRRAN_NRE],
                 ISRRAN_NRE * sizeof(cf_t));
        } else {
          memcpy(&dest[ns * N_rs * ISRRAN_NRE + i * ISRRAN_NRE],
                 &source[ISRRAN_RE_IDX(q->cell.nof_prb, l + ns * nsymbols, n_prb * ISRRAN_NRE)],
                 ISRRAN_NRE * sizeof(cf_t));
        }
      }
    }

    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

/* Maps PUCCH DMRS to the physical resources as defined in 5.5.2.2.2 in 36.211 */
int isrran_refsignal_dmrs_pucch_put(isrran_refsignal_ul_t* q, isrran_pucch_cfg_t* cfg, cf_t* r_pucch, cf_t* output)
{
  return isrran_refsignal_dmrs_pucch_cp(q, cfg, r_pucch, output, false);
}

/* Gets PUCCH DMRS from the physical resources as defined in 5.5.2.2.2 in 36.211 */
int isrran_refsignal_dmrs_pucch_get(isrran_refsignal_ul_t* q, isrran_pucch_cfg_t* cfg, cf_t* input, cf_t* r_pucch)
{
  return isrran_refsignal_dmrs_pucch_cp(q, cfg, input, r_pucch, true);
}

static uint32_t T_isr_table(uint32_t I_isr)
{
  uint32_t T_isr;
  /* This is Table 8.2-1 */
  if (I_isr < 2) {
    T_isr = 2;
  } else if (I_isr < 7) {
    T_isr = 5;
  } else if (I_isr < 17) {
    T_isr = 10;
  } else if (I_isr < 37) {
    T_isr = 20;
  } else if (I_isr < 77) {
    T_isr = 40;
  } else if (I_isr < 157) {
    T_isr = 80;
  } else if (I_isr < 317) {
    T_isr = 160;
  } else if (I_isr < 637) {
    T_isr = 320;
  } else {
    T_isr = 0;
  }
  return T_isr;
}

/* Returns 1 if tti is a valid subframe for ISR transmission according to I_isr (UE-specific
 * configuration index), as defined in Section 8.1 of 36.213.
 * Returns 0 if no ISR shall be transmitted or a negative number if error.
 */
int isrran_refsignal_isr_send_ue(uint32_t I_isr, uint32_t tti)
{
  if (I_isr < 1024 && tti < 10240) {
    uint32_t Toffset = 0;
    /* This is Table 8.2-1 */
    if (I_isr < 2) {
      Toffset = I_isr;
    } else if (I_isr < 7) {
      Toffset = I_isr - 2;
    } else if (I_isr < 17) {
      Toffset = I_isr - 7;
    } else if (I_isr < 37) {
      Toffset = I_isr - 17;
    } else if (I_isr < 77) {
      Toffset = I_isr - 37;
    } else if (I_isr < 157) {
      Toffset = I_isr - 77;
    } else if (I_isr < 317) {
      Toffset = I_isr - 157;
    } else if (I_isr < 637) {
      Toffset = I_isr - 317;
    } else {
      return 0;
    }
    if (((tti - Toffset) % T_isr_table(I_isr)) == 0) {
      return 1;
    } else {
      return 0;
    }
  } else {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }
}

// Shortened PUCCH happen in every cell-specific ISR subframes for Format 1/1a/1b
void isrran_refsignal_isr_pucch_shortened(isrran_refsignal_ul_t*      q,
                                          isrran_ul_sf_cfg_t*         sf,
                                          isrran_refsignal_isr_cfg_t* isr_cfg,
                                          isrran_pucch_cfg_t*         pucch_cfg)
{
  bool shortened = false;
  if (isr_cfg->configured && pucch_cfg->format < ISRRAN_PUCCH_FORMAT_2) {
    shortened = false;
    // If CQI is not transmitted, PUCCH will be normal unless ACK/NACK and ISR simultaneous transmission is enabled
    if (isr_cfg->simul_ack) {
      // If simultaneous ACK and ISR is enabled, PUCCH is shortened in cell-specific ISR subframes
      if (isrran_refsignal_isr_send_cs(isr_cfg->subframe_config, sf->tti % 10) == 1) {
        shortened = true;
      }
    }
  }
  sf->shortened = shortened;
}

void isrran_refsignal_isr_pusch_shortened(isrran_refsignal_ul_t*      q,
                                          isrran_ul_sf_cfg_t*         sf,
                                          isrran_refsignal_isr_cfg_t* isr_cfg,
                                          isrran_pusch_cfg_t*         pusch_cfg)
{
  bool shortened = false;

  // A UE shall not transmit ISR whenever ISR and a PUSCH transmission corresponding to a Random Access Response
  // Grant or a retransmission of the same transport block as part of the contention based random access procedure
  // coincide in the same subframe.
  if (pusch_cfg->grant.is_rar) {
    sf->shortened = false;
    return;
  }

  if (isr_cfg->configured) {
    // If UE-specific ISR is configured, PUSCH is shortened every time UE transmits ISR even if overlaping in the same
    // RB or not
    if (isrran_refsignal_isr_send_cs(isr_cfg->subframe_config, sf->tti % 10) == 1 &&
        isrran_refsignal_isr_send_ue(isr_cfg->I_isr, sf->tti) == 1) {
      shortened = true;
      /* If RBs are contiguous, PUSCH is not shortened */
      uint32_t k0_isr  = isrran_refsignal_isr_rb_start_cs(isr_cfg->bw_cfg, q->cell.nof_prb);
      uint32_t nrb_isr = isrran_refsignal_isr_rb_L_cs(isr_cfg->bw_cfg, q->cell.nof_prb);
      for (uint32_t ns = 0; ns < 2 && shortened; ns++) {
        if (pusch_cfg->grant.n_prb_tilde[ns] ==
                k0_isr + nrb_isr || // If PUSCH is contiguous on the right-hand side of ISR
            pusch_cfg->grant.n_prb_tilde[ns] + pusch_cfg->grant.L_prb ==
                k0_isr) // If ISR is contiguous on the left-hand side of PUSCH
        {
          shortened = false;
        }
      }
    }
    // If not coincides with UE transmission. PUSCH shall be shortened if cell-specific ISR transmission RB
    // coincides with PUSCH allocated RB
    if (!shortened) {
      if (isrran_refsignal_isr_send_cs(isr_cfg->subframe_config, sf->tti % 10) == 1) {
        uint32_t k0_isr  = isrran_refsignal_isr_rb_start_cs(isr_cfg->bw_cfg, q->cell.nof_prb);
        uint32_t nrb_isr = isrran_refsignal_isr_rb_L_cs(isr_cfg->bw_cfg, q->cell.nof_prb);
        for (uint32_t ns = 0; ns < 2 && !shortened; ns++) {
          if ((pusch_cfg->grant.n_prb_tilde[ns] >= k0_isr && pusch_cfg->grant.n_prb_tilde[ns] < k0_isr + nrb_isr) ||
              (pusch_cfg->grant.n_prb_tilde[ns] + pusch_cfg->grant.L_prb > k0_isr &&
               pusch_cfg->grant.n_prb_tilde[ns] + pusch_cfg->grant.L_prb < k0_isr + nrb_isr) ||
              (pusch_cfg->grant.n_prb_tilde[ns] <= k0_isr &&
               pusch_cfg->grant.n_prb_tilde[ns] + pusch_cfg->grant.L_prb >= k0_isr + nrb_isr)) {
            shortened = true;
          }
        }
      }
    }
  }
  sf->shortened = shortened;
}

/* Returns 1 if sf_idx is a valid subframe for ISR transmission according to subframe_config (cell-specific),
 * as defined in Section 5.5.3.3 of 36.211. Returns 0 if no ISR shall be transmitted or a negative
 * number if error.
 */
int isrran_refsignal_isr_send_cs(uint32_t subframe_config, uint32_t sf_idx)
{
  if (subframe_config < 15 && sf_idx < 10) {
    uint32_t tsfc = T_sfc[subframe_config];
    if (subframe_config < 7) {
      if ((sf_idx % tsfc) == Delta_sfc1[subframe_config]) {
        return 1;
      } else {
        return 0;
      }
    } else if (subframe_config == 7) {
      if (((sf_idx % tsfc) == 0) || ((sf_idx % tsfc) == 1)) {
        return 1;
      } else {
        return 0;
      }
    } else if (subframe_config == 8) {
      if (((sf_idx % tsfc) == 2) || ((sf_idx % tsfc) == 3)) {
        return 1;
      } else {
        return 0;
      }
    } else if (subframe_config < 13) {
      if ((sf_idx % tsfc) == Delta_sfc2[subframe_config - 9]) {
        return 1;
      } else {
        return 0;
      }
    } else if (subframe_config == 13) {
      if (((sf_idx % tsfc) == 5) || ((sf_idx % tsfc) == 7) || ((sf_idx % tsfc) == 9)) {
        return 0;
      } else {
        return 1;
      }
    }
    // subframe_config == 14
    else {
      if (((sf_idx % tsfc) == 7) || ((sf_idx % tsfc) == 9)) {
        return 0;
      } else {
        return 1;
      }
    }
  } else {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }
}

static uint32_t isrbwtable_idx(uint32_t nof_prb)
{
  if (nof_prb <= 40) {
    return 0;
  } else if (nof_prb <= 60) {
    return 1;
  } else if (nof_prb <= 80) {
    return 2;
  } else {
    return 3;
  }
}

/* Returns start of common ISR BW region */
uint32_t isrran_refsignal_isr_rb_start_cs(uint32_t bw_cfg, uint32_t nof_prb)
{
  if (bw_cfg < 8) {
    return nof_prb / 2 - m_isr_b[isrbwtable_idx(nof_prb)][0][bw_cfg] / 2;
  }
  return 0;
}

/* Returns number of RB defined for the cell-specific ISR */
uint32_t isrran_refsignal_isr_rb_L_cs(uint32_t bw_cfg, uint32_t nof_prb)
{
  if (bw_cfg < 8) {
    return m_isr_b[isrbwtable_idx(nof_prb)][0][bw_cfg];
  }
  return 0;
}

static uint32_t isr_Fb(isrran_refsignal_isr_cfg_t* cfg, uint32_t b, uint32_t nof_prb, uint32_t tti)
{
  uint32_t Fb = 0;
  uint32_t T  = T_isr_table(cfg->I_isr);
  if (T) {
    uint32_t n_isr = tti / T;
    uint32_t N_b   = Nb[isrbwtable_idx(nof_prb)][b][cfg->bw_cfg];

    uint32_t prod_1 = 1;
    for (uint32_t bp = cfg->b_hop + 1; bp < b; bp++) {
      prod_1 *= Nb[isrbwtable_idx(nof_prb)][bp][cfg->bw_cfg];
    }
    uint32_t prod_2 = prod_1 * Nb[isrbwtable_idx(nof_prb)][b][cfg->bw_cfg];
    if ((N_b % 2) == 0) {
      Fb = (N_b / 2) * ((n_isr % prod_2) / prod_1) + ((n_isr % prod_2) / prod_1 / 2);
    } else {
      Fb = (N_b / 2) * (n_isr / prod_1);
    }
  }
  return Fb;
}

/* Returns k0: frequency-domain starting position for ue-specific ISR */
static uint32_t isr_k0_ue(isrran_refsignal_isr_cfg_t* cfg, uint32_t nof_prb, uint32_t tti)
{
  if (cfg->bw_cfg < 8 && cfg->B < 4 && cfg->k_tc < 2) {
    uint32_t k0p = isrran_refsignal_isr_rb_start_cs(cfg->bw_cfg, nof_prb) * ISRRAN_NRE + cfg->k_tc;
    uint32_t k0  = k0p;
    uint32_t nb  = 0;
    for (int b = 0; b <= cfg->B; b++) {
      uint32_t m_isr = m_isr_b[isrbwtable_idx(nof_prb)][b][cfg->bw_cfg];
      uint32_t m_sc  = m_isr * ISRRAN_NRE / 2;
      if (b <= cfg->b_hop) {
        nb = (4 * cfg->n_rrc / m_isr) % Nb[isrbwtable_idx(nof_prb)][b][cfg->bw_cfg];
      } else {
        uint32_t Fb = isr_Fb(cfg, b, nof_prb, tti);
        nb          = ((4 * cfg->n_rrc / m_isr) + Fb) % Nb[isrbwtable_idx(nof_prb)][b][cfg->bw_cfg];
      }
      k0 += 2 * m_sc * nb;
    }
    return k0;
  }
  return 0;
}

uint32_t isrran_refsignal_isr_M_sc(isrran_refsignal_ul_t* q, isrran_refsignal_isr_cfg_t* cfg)
{
  return m_isr_b[isrbwtable_idx(q->cell.nof_prb)][cfg->B][cfg->bw_cfg] * ISRRAN_NRE / 2;
}

int isrran_refsignal_isr_pregen(isrran_refsignal_ul_t*             q,
                                isrran_refsignal_isr_pregen_t*     pregen,
                                isrran_refsignal_isr_cfg_t*        cfg,
                                isrran_refsignal_dmrs_pusch_cfg_t* dmrs)
{
  uint32_t M_sc = isrran_refsignal_isr_M_sc(q, cfg);
  for (uint32_t sf_idx = 0; sf_idx < ISRRAN_NOF_SF_X_FRAME; sf_idx++) {
    pregen->r[sf_idx] = isrran_vec_cf_malloc(2 * M_sc);
    if (pregen->r[sf_idx]) {
      if (isrran_refsignal_isr_gen(q, cfg, dmrs, sf_idx, pregen->r[sf_idx])) {
        return ISRRAN_ERROR;
      }
    } else {
      return ISRRAN_ERROR;
    }
  }
  return ISRRAN_SUCCESS;
}

void isrran_refsignal_isr_pregen_free(isrran_refsignal_ul_t* q, isrran_refsignal_isr_pregen_t* pregen)
{
  for (uint32_t sf_idx = 0; sf_idx < ISRRAN_NOF_SF_X_FRAME; sf_idx++) {
    if (pregen->r[sf_idx]) {
      free(pregen->r[sf_idx]);
    }
  }
}

int isrran_refsignal_isr_pregen_put(isrran_refsignal_ul_t*         q,
                                    isrran_refsignal_isr_pregen_t* pregen,
                                    isrran_refsignal_isr_cfg_t*    cfg,
                                    uint32_t                       tti,
                                    cf_t*                          sf_symbols)
{
  return isrran_refsignal_isr_put(q, cfg, tti, pregen->r[tti % ISRRAN_NOF_SF_X_FRAME], sf_symbols);
}

/* Genearte ISR signal as defined in Section 5.5.3.1 */
int isrran_refsignal_isr_gen(isrran_refsignal_ul_t*             q,
                             isrran_refsignal_isr_cfg_t*        cfg,
                             isrran_refsignal_dmrs_pusch_cfg_t* pusch_cfg,
                             uint32_t                           sf_idx,
                             cf_t*                              r_isr)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (r_isr && q && cfg && pusch_cfg) {
    ret = ISRRAN_ERROR;

    uint32_t M_sc = isrran_refsignal_isr_M_sc(q, cfg);
    for (uint32_t ns = 2 * sf_idx; ns < 2 * (sf_idx + 1); ns++) {
      float alpha = 2 * M_PI * cfg->n_isr / 8;
      compute_r(q, pusch_cfg, M_sc / ISRRAN_NRE, ns, 0, alpha, &r_isr[(ns % 2) * M_sc]);
    }
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

int isrran_refsignal_isr_put(isrran_refsignal_ul_t*      q,
                             isrran_refsignal_isr_cfg_t* cfg,
                             uint32_t                    tti,
                             cf_t*                       r_isr,
                             cf_t*                       sf_symbols)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (r_isr && q && sf_symbols && cfg) {
    uint32_t M_sc = isrran_refsignal_isr_M_sc(q, cfg);
    uint32_t k0   = isr_k0_ue(cfg, q->cell.nof_prb, tti);
    for (int i = 0; i < M_sc; i++) {
      sf_symbols[ISRRAN_RE_IDX(q->cell.nof_prb, 2 * ISRRAN_CP_NSYMB(q->cell.cp) - 1, k0 + 2 * i)] = r_isr[i];
    }
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

int isrran_refsignal_isr_get(isrran_refsignal_ul_t*      q,
                             isrran_refsignal_isr_cfg_t* cfg,
                             uint32_t                    tti,
                             cf_t*                       r_isr,
                             cf_t*                       sf_symbols)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (r_isr && q && sf_symbols && cfg) {
    uint32_t M_sc = isrran_refsignal_isr_M_sc(q, cfg);
    uint32_t k0   = isr_k0_ue(cfg, q->cell.nof_prb, tti);
    for (int i = 0; i < M_sc; i++) {
      r_isr[i] = sf_symbols[ISRRAN_RE_IDX(q->cell.nof_prb, 2 * ISRRAN_CP_NSYMB(q->cell.cp) - 1, k0 + 2 * i)];
    }
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}
