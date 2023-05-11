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

#include "isrran/phy/phch/pucch_nr.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/common/zc_sequence.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <complex.h>

int isrran_pucch_nr_group_sequence(const isrran_carrier_nr_t*          carrier,
                                   const isrran_pucch_nr_common_cfg_t* cfg,
                                   uint32_t*                           u_,
                                   uint32_t*                           v_)
{
  uint32_t f_gh = 0;
  uint32_t f_ss = 0;
  uint32_t n_id = cfg->hopping_id_present ? cfg->hopping_id : carrier->pci;

  switch (cfg->group_hopping) {
    case ISRRAN_PUCCH_NR_GROUP_HOPPING_NEITHER:
      f_ss = n_id % ISRRAN_ZC_SEQUENCE_NOF_GROUPS;
      break;
    case ISRRAN_PUCCH_NR_GROUP_HOPPING_ENABLE:
      ERROR("Group hopping is not implemented");
      return ISRRAN_ERROR;
    case ISRRAN_PUCCH_NR_GROUP_HOPPING_DISABLE:
      ERROR("Hopping is not implemented");
      return ISRRAN_ERROR;
  }

  uint32_t u = (f_gh + f_ss) % ISRRAN_ZC_SEQUENCE_NOF_GROUPS;
  uint32_t v = 0;

  if (u_) {
    *u_ = u;
  }

  if (v_) {
    *v_ = v;
  }

  return ISRRAN_SUCCESS;
}

// Implements TS 38.211 clause 6.3.2.2.2 Cyclic shift hopping
int isrran_pucch_nr_alpha_idx(const isrran_carrier_nr_t*          carrier,
                              const isrran_pucch_nr_common_cfg_t* cfg,
                              const isrran_slot_cfg_t*            slot,
                              uint32_t                            l,
                              uint32_t                            l_prime,
                              uint32_t                            m0,
                              uint32_t                            m_cs,
                              uint32_t*                           alpha_idx)
{
  if (carrier == NULL || cfg == NULL || slot == NULL || alpha_idx == NULL) {
    return ISRRAN_ERROR;
  }

  // Compute number of slot
  uint32_t n_slot = ISRRAN_SLOT_NR_MOD(carrier->scs, slot->idx);

  // Generate pseudo-random sequence
  uint32_t cinit = cfg->hopping_id_present ? cfg->hopping_id : carrier->pci;
  uint8_t  cs[ISRRAN_NSYMB_PER_SLOT_NR * ISRRAN_NSLOTS_PER_FRAME_NR(ISRRAN_NR_MAX_NUMEROLOGY) * 8U] = {};
  isrran_sequence_apply_bit(cs, cs, ISRRAN_NSYMB_PER_SLOT_NR * ISRRAN_NSLOTS_PER_FRAME_NR(carrier->scs) * 8, cinit);

  // Create n_cs parameter
  uint32_t n_cs = 0;
  for (uint32_t m = 0; m < 8; m++) {
    n_cs += cs[(ISRRAN_NSYMB_PER_SLOT_NR * n_slot + (l + l_prime)) * 8 + m] << m;
  }

  *alpha_idx = (m0 + m_cs + n_cs) % ISRRAN_NRE;

  return ISRRAN_SUCCESS;
}

// TS 38.211 Table 6.3.2.4.1-2: Orthogonal sequences for PUCCH format 1
static uint32_t
    pucch_nr_format1_rho[ISRRAN_PUCCH_NR_FORMAT1_N_MAX][ISRRAN_PUCCH_NR_FORMAT1_N_MAX][ISRRAN_PUCCH_NR_FORMAT1_N_MAX] =
        {{{0}, {0, 0}, {0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}},
         {{}, {0, 1}, {0, 1, 2}, {0, 2, 0, 2}, {0, 1, 2, 3, 4}, {0, 1, 2, 3, 4, 5}, {0, 1, 2, 3, 4, 5, 6}},
         {{}, {}, {0, 2, 1}, {0, 0, 2, 2}, {0, 2, 4, 1, 3}, {0, 2, 4, 0, 2, 4}, {0, 2, 4, 6, 1, 3, 5}},
         {{}, {}, {}, {0, 2, 2, 0}, {0, 3, 1, 4, 2}, {0, 3, 0, 3, 0, 3}, {0, 3, 6, 2, 5, 1, 4}},
         {{}, {}, {}, {}, {0, 4, 3, 2, 1}, {0, 4, 2, 0, 4, 2}, {0, 4, 1, 5, 2, 6, 3}},
         {{}, {}, {}, {}, {}, {0, 5, 4, 3, 2, 1}, {0, 5, 3, 1, 6, 4, 2}},
         {{}, {}, {}, {}, {}, {}, {0, 6, 5, 4, 3, 2, 1}}};

int isrran_pucch_nr_init(isrran_pucch_nr_t* q, const isrran_pucch_nr_args_t* args)
{
  if (q == NULL || args == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Make sure object is zeroed
  ISRRAN_MEM_ZERO(q, isrran_pucch_nr_t, 1);

  // Save maximum number of PRB
  q->max_prb = ISRRAN_MAX_PRB_NR;
  if (args->max_nof_prb != 0) {
    q->max_prb = args->max_nof_prb;
  }

  // Initialise ZC sequences for 1PRB
  float alphas_1prb[ISRRAN_NRE] = {};
  for (uint32_t i = 0; i < ISRRAN_NRE; i++) {
    alphas_1prb[i] = 2.0f * (float)M_PI * (float)i / (float)ISRRAN_NRE;
  }
  isrran_zc_sequence_lut_init_nr(&q->r_uv_1prb, 1, 0, alphas_1prb, ISRRAN_NRE);

  // Initialise BPSK modulation table
  if (isrran_modem_table_lte(&q->bpsk, ISRRAN_MOD_BPSK) < ISRRAN_SUCCESS) {
    ERROR("Initiating modem table");
    return ISRRAN_ERROR;
  }

  // Initialise QPSK modulation table
  if (isrran_modem_table_lte(&q->qpsk, ISRRAN_MOD_QPSK) < ISRRAN_SUCCESS) {
    ERROR("Initiating modem table");
    return ISRRAN_ERROR;
  }

  for (uint32_t n_pucch = 1; n_pucch <= ISRRAN_PUCCH_NR_FORMAT1_N_MAX; n_pucch++) {
    for (uint32_t i = 0; i < ISRRAN_PUCCH_NR_FORMAT1_N_MAX; i++) {
      for (uint32_t m = 0; m < ISRRAN_PUCCH_NR_FORMAT1_N_MAX; m++) {
        uint32_t rho                        = pucch_nr_format1_rho[i][n_pucch - 1][m];
        q->format1_w_i_m[i][n_pucch - 1][m] = cexpf(I * 2.0f * (float)M_PI * (float)rho / n_pucch);
      }
    }
  }

  if (isrran_uci_nr_init(&q->uci, &args->uci) < ISRRAN_SUCCESS) {
    ERROR("Initiating UCI encoder/decoder");
    return ISRRAN_ERROR;
  }

  // Allocate encoded bits b
  uint32_t max_encoded_bits = q->max_prb * ISRRAN_NRE * 2 * ISRRAN_NSYMB_PER_SLOT_NR; // Assumes QPSK (Qm = 2)
  q->b                      = isrran_vec_u8_malloc(max_encoded_bits);
  if (q->b == NULL) {
    ERROR("Malloc");
    return ISRRAN_ERROR;
  }

  // Allocate encoded symbols d
  q->d = isrran_vec_cf_malloc(max_encoded_bits / 2);
  if (q->d == NULL) {
    ERROR("Malloc");
    return ISRRAN_ERROR;
  }

  // Allocate temporal channel estimates
  q->ce = isrran_vec_cf_malloc(max_encoded_bits / 2);
  if (q->ce == NULL) {
    ERROR("Malloc");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_pucch_nr_set_carrier(isrran_pucch_nr_t* q, const isrran_carrier_nr_t* carrier)
{
  if (q == NULL || carrier == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  q->carrier = *carrier;

  return ISRRAN_SUCCESS;
}

void isrran_pucch_nr_free(isrran_pucch_nr_t* q)
{
  if (q == NULL) {
    return;
  }

  isrran_uci_nr_free(&q->uci);
  isrran_zc_sequence_lut_free(&q->r_uv_1prb);

  isrran_modem_table_free(&q->bpsk);
  isrran_modem_table_free(&q->qpsk);

  if (q->b != NULL) {
    free(q->b);
  }
  if (q->d != NULL) {
    free(q->d);
  }

  if (q->ce != NULL) {
    free(q->ce);
  }

  ISRRAN_MEM_ZERO(q, isrran_pucch_nr_t, 1);
}

int isrran_pucch_nr_format0_encode(const isrran_pucch_nr_t*            q,
                                   const isrran_pucch_nr_common_cfg_t* cfg,
                                   const isrran_slot_cfg_t*            slot,
                                   isrran_pucch_nr_resource_t*         resource,
                                   uint32_t                            m_cs,
                                   cf_t*                               slot_symbols)
{
  if (cfg == NULL || slot == NULL || resource == NULL || slot_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    ERROR("Invalid PUCCH format 0 resource");
    return ISRRAN_SUCCESS;
  }

  uint32_t u = 0;
  uint32_t v = 0;
  if (isrran_pucch_nr_group_sequence(&q->carrier, cfg, &u, &v) < ISRRAN_SUCCESS) {
    ERROR("Error getting group sequence");
    return ISRRAN_ERROR;
  }

  uint32_t l_prime = resource->start_symbol_idx;
  for (uint32_t l = 0; l < resource->nof_symbols; l++) {
    // Get Alpha index
    uint32_t alpha_idx = 0;
    if (isrran_pucch_nr_alpha_idx(
            &q->carrier, cfg, slot, l, l_prime, resource->initial_cyclic_shift, m_cs, &alpha_idx) < ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }

    // get r_uv sequence from LUT object
    const cf_t* r_uv = isrran_zc_sequence_lut_get(&q->r_uv_1prb, u, v, alpha_idx);
    if (r_uv == NULL) {
      ERROR("Getting r_uv sequence");
      return ISRRAN_ERROR;
    }

    // Get start of the sequence in resource grid
    cf_t* slot_symbols_ptr = &slot_symbols[(q->carrier.nof_prb * (l + l_prime) + resource->starting_prb) * ISRRAN_NRE];

    // Copy sequence in grid
    isrran_vec_cf_copy(slot_symbols_ptr, r_uv, ISRRAN_NRE);
  }

  return ISRRAN_SUCCESS;
}

int isrran_pucch_nr_format0_measure(const isrran_pucch_nr_t*            q,
                                    const isrran_pucch_nr_common_cfg_t* cfg,
                                    const isrran_slot_cfg_t*            slot,
                                    isrran_pucch_nr_resource_t*         resource,
                                    uint32_t                            m_cs,
                                    const cf_t*                         slot_symbols,
                                    isrran_pucch_nr_measure_t*          measure)
{
  if (cfg == NULL || slot == NULL || resource == NULL || slot_symbols == NULL || measure == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    ERROR("Invalid PUCCH format 0 resource");
    return ISRRAN_SUCCESS;
  }

  uint32_t u = 0;
  uint32_t v = 0;
  if (isrran_pucch_nr_group_sequence(&q->carrier, cfg, &u, &v) < ISRRAN_SUCCESS) {
    ERROR("Error getting group sequence");
    return ISRRAN_ERROR;
  }

  uint32_t l_prime = resource->start_symbol_idx;
  float    epre    = 0.0f;
  float    rsrp    = 0.0f;
  for (uint32_t l = 0; l < resource->nof_symbols; l++) {
    // Get Alpha index
    uint32_t alpha_idx = 0;
    if (isrran_pucch_nr_alpha_idx(
            &q->carrier, cfg, slot, l, l_prime, resource->initial_cyclic_shift, m_cs, &alpha_idx) < ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }

    // get r_uv sequence from LUT object
    const cf_t* r_uv = isrran_zc_sequence_lut_get(&q->r_uv_1prb, u, v, alpha_idx);
    if (r_uv == NULL) {
      ERROR("Getting r_uv sequence");
      return ISRRAN_ERROR;
    }

    // Get start of the sequence in resource grid
    const cf_t* slot_symbols_ptr =
        &slot_symbols[(q->carrier.nof_prb * (l + l_prime) + resource->starting_prb) * ISRRAN_NRE];

    // Measure EPRE and average
    epre += isrran_vec_avg_power_cf(slot_symbols_ptr, ISRRAN_NRE) / resource->nof_symbols;

    // Do correlation
    cf_t corr = isrran_vec_dot_prod_conj_ccc(r_uv, slot_symbols_ptr, ISRRAN_NRE);
    corr /= ISRRAN_NRE;

    // Measure RSRP and average
    rsrp += (__real__ corr * __real__ corr + __imag__ corr * __imag__ corr) / resource->nof_symbols;
  }

  // Save measurement
  measure->rsrp      = rsrp;
  measure->rsrp_dBfs = isrran_convert_power_to_dB(rsrp);
  measure->epre      = epre;
  measure->epre_dBfs = isrran_convert_power_to_dB(epre);
  if (isnormal(epre)) {
    measure->norm_corr = rsrp / epre;
  } else {
    measure->norm_corr = 0.0f;
  }

  return ISRRAN_SUCCESS;
}

// Implements TS 38.211 table 6.3.2.4.1-1 Number of PUCCH symbols and the corresponding N_PUC...
static uint32_t pucch_nr_format1_n_pucch(const isrran_pucch_nr_resource_t* resource, uint32_t m_prime)
{
  if (resource->intra_slot_hopping) {
    if (m_prime == 0) {
      return resource->nof_symbols / 4;
    }

    return resource->nof_symbols / 2 - resource->nof_symbols / 4;
  }

  if (m_prime == 1) {
    return 0;
  }

  return resource->nof_symbols / 2;
}

cf_t isrran_pucch_nr_format1_w(const isrran_pucch_nr_t* q, uint32_t n_pucch, uint32_t i, uint32_t m)
{
  if (n_pucch < 1 || n_pucch > ISRRAN_PUCCH_NR_FORMAT1_N_MAX) {
    ERROR("Invalid n_pucch");
    return NAN;
  }
  if (i >= ISRRAN_PUCCH_NR_FORMAT1_N_MAX) {
    ERROR("Invalid i");
    return NAN;
  }
  if (m >= ISRRAN_PUCCH_NR_FORMAT1_N_MAX) {
    ERROR("Invalid m");
    return NAN;
  }

  // Get value
  return q->format1_w_i_m[i][n_pucch - 1][m];
}

int isrran_pucch_nr_format1_encode(const isrran_pucch_nr_t*            q,
                                   const isrran_pucch_nr_common_cfg_t* cfg,
                                   const isrran_slot_cfg_t*            slot,
                                   const isrran_pucch_nr_resource_t*   resource,
                                   uint8_t*                            b,
                                   uint32_t                            nof_bits,
                                   cf_t*                               slot_symbols)
{
  if (q == NULL || cfg == NULL || slot == NULL || resource == NULL || b == NULL || slot_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    ERROR("Invalid PUCCH format 1 resource");
    return ISRRAN_SUCCESS;
  }

  if (nof_bits > ISRRAN_PUCCH_NR_FORMAT1_MAX_NOF_BITS) {
    ERROR("Invalid number of bits (%d)", nof_bits);
    return ISRRAN_ERROR;
  }

  // Modulate d
  cf_t d[1] = {};
  if (nof_bits == 1) {
    isrran_mod_modulate(&q->bpsk, b, d, 1);
  } else {
    isrran_mod_modulate(&q->qpsk, b, d, 2);
  }

  INFO("[PUCCH Format 1 Data TX] d=%+.3f%+.3f", __real__ d[0], __imag__ d[0]);

  // Get group sequence
  uint32_t u = 0;
  uint32_t v = 0;
  if (isrran_pucch_nr_group_sequence(&q->carrier, cfg, &u, &v) < ISRRAN_SUCCESS) {
    ERROR("Error getting group sequence");
    return ISRRAN_ERROR;
  }

  // First symbol of this PUCCH transmission
  uint32_t l_prime = resource->start_symbol_idx;

  // For each hop
  for (uint32_t m_prime = 0, l = 1; m_prime < (resource->intra_slot_hopping ? 2 : 1); m_prime++) {
    // Calculate number of symbols carrying PUCCH (No DMRS)
    uint32_t n_pucch = pucch_nr_format1_n_pucch(resource, m_prime);

    // Get the starting PRB
    uint32_t starting_prb = (m_prime == 0) ? resource->starting_prb : resource->second_hop_prb;

    // For each symbol carrying PUCCH data
    for (uint32_t m = 0; m < n_pucch; m++, l += 2) {
      // Get start of the sequence in resource grid
      cf_t* slot_symbols_ptr = &slot_symbols[(q->carrier.nof_prb * (l + l_prime) + starting_prb) * ISRRAN_NRE];

      // Get Alpha index
      uint32_t alpha_idx = 0;
      if (isrran_pucch_nr_alpha_idx(&q->carrier, cfg, slot, l, l_prime, resource->initial_cyclic_shift, 0, &alpha_idx) <
          ISRRAN_SUCCESS) {
        return ISRRAN_ERROR;
      }

      // get r_uv sequence from LUT object
      const cf_t* r_uv = isrran_zc_sequence_lut_get(&q->r_uv_1prb, u, v, alpha_idx);
      if (r_uv == NULL) {
        ERROR("Getting r_uv sequence");
        return ISRRAN_ERROR;
      }

      // Get w_i_m
      cf_t w_i_m = isrran_pucch_nr_format1_w(q, n_pucch, resource->time_domain_occ, m);

      // Compute z(n) = w(i) * r_uv(n)
      cf_t z[ISRRAN_NRE];
      isrran_vec_sc_prod_ccc(r_uv, w_i_m, z, ISRRAN_NRE);

      if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
        printf("[PUCCH Format 1 Data TX] m_prime=%d; m=%d; w_i_m=%+.3f%+.3f z=",
               m_prime,
               m,
               __real__ w_i_m,
               __imag__ w_i_m);
        isrran_vec_fprint_c(stdout, z, ISRRAN_NRE);
      }

      // Put z in the grid
      isrran_vec_sc_prod_ccc(z, d[0], slot_symbols_ptr, ISRRAN_NRE);

      if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
        printf("[PUCCH Format 1 TX] l=%d; x=", l + l_prime);
        isrran_vec_fprint_c(stdout, slot_symbols_ptr, ISRRAN_NRE);
      }
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_pucch_nr_format1_decode(isrran_pucch_nr_t*                  q,
                                   const isrran_pucch_nr_common_cfg_t* cfg,
                                   const isrran_slot_cfg_t*            slot,
                                   const isrran_pucch_nr_resource_t*   resource,
                                   isrran_chest_ul_res_t*              chest_res,
                                   cf_t*                               slot_symbols,
                                   uint8_t                             b[ISRRAN_PUCCH_NR_FORMAT1_MAX_NOF_BITS],
                                   uint32_t                            nof_bits,
                                   float*                              norm_corr)
{
  uint32_t m_cs = 0;

  if (q == NULL || cfg == NULL || slot == NULL || resource == NULL || chest_res == NULL || b == NULL ||
      slot_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    ERROR("Invalid PUCCH format 1 resource");
    return ISRRAN_SUCCESS;
  }

  if (nof_bits > ISRRAN_PUCCH_NR_FORMAT1_MAX_NOF_BITS) {
    ERROR("Invalid number of bits (%d)", nof_bits);
    return ISRRAN_ERROR;
  }

  // Accumulates received symbol d and average power
  cf_t  d       = 0;
  float pwr_acc = 0.0f;

  // Get group sequence
  uint32_t u = 0;
  uint32_t v = 0;
  if (isrran_pucch_nr_group_sequence(&q->carrier, cfg, &u, &v) < ISRRAN_SUCCESS) {
    ERROR("Error getting group sequence");
    return ISRRAN_ERROR;
  }

  // First symbol of this PUCCH transmission
  uint32_t l_prime = resource->start_symbol_idx;

  // For each hop
  uint32_t n_pucch_sum = 0;
  for (uint32_t m_prime = 0, l = 1; m_prime < (resource->intra_slot_hopping ? 2 : 1); m_prime++) {
    // Calculate number of symbols carrying PUCCH (No DMRS)
    uint32_t n_pucch = pucch_nr_format1_n_pucch(resource, m_prime);

    // Get the starting PRB
    uint32_t starting_prb = (m_prime == 0) ? resource->starting_prb : resource->second_hop_prb;

    // For each symbol carrying PUCCH data
    for (uint32_t m = 0; m < n_pucch; m++, l += 2) {
      // Get start of the sequence in resource grid
      cf_t* slot_symbols_ptr = &slot_symbols[(q->carrier.nof_prb * (l + l_prime) + starting_prb) * ISRRAN_NRE];
      cf_t* ce_ptr           = &chest_res->ce[ISRRAN_NRE * n_pucch_sum];
      n_pucch_sum++;

      if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
        printf("[PUCCH Format 1 CE   RX] ce=");
        isrran_vec_fprint_c(stdout, ce_ptr, ISRRAN_NRE);
      }

      // Equalise x = w(i) * d' * r_uv(n)
      cf_t x[ISRRAN_NRE];
      isrran_predecoding_single(slot_symbols_ptr, ce_ptr, x, NULL, ISRRAN_NRE, 1.0f, chest_res->noise_estimate);

      if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
        printf("[PUCCH Format 1 RX] l=%d; x=", l + l_prime);
        isrran_vec_fprint_c(stdout, x, ISRRAN_NRE);
      }

      // Get Alpha index
      uint32_t alpha_idx = 0;
      if (isrran_pucch_nr_alpha_idx(
              &q->carrier, cfg, slot, l, l_prime, resource->initial_cyclic_shift, m_cs, &alpha_idx) < ISRRAN_SUCCESS) {
        return ISRRAN_ERROR;
      }

      // get r_uv sequence from LUT object
      const cf_t* r_uv = isrran_zc_sequence_lut_get(&q->r_uv_1prb, u, v, alpha_idx);
      if (r_uv == NULL) {
        ERROR("Getting r_uv sequence");
        return ISRRAN_ERROR;
      }

      // Get w_i_m
      cf_t w_i_m = isrran_pucch_nr_format1_w(q, n_pucch, resource->time_domain_occ, m);

      // Compute z(n) = w(i) * r_uv(n)
      cf_t z[ISRRAN_NRE];
      isrran_vec_sc_prod_ccc(r_uv, w_i_m, z, ISRRAN_NRE);

      if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
        printf("[PUCCH Format 1 Data RX] m_prime=%d; m=%d; w_i_m=%+.3f%+.3f z=",
               m_prime,
               m,
               __real__ w_i_m,
               __imag__ w_i_m);
        isrran_vec_fprint_c(stdout, z, ISRRAN_NRE);
      }

      // Compute d = sum(x * conj(w(i) * r_uv(n))) = sum(w(i) * d' * r_uv(n) * conj(w(i) * r_uv(n))) = d'
      d += isrran_vec_dot_prod_conj_ccc(x, z, ISRRAN_NRE) / ISRRAN_NRE;

      // Compute and accumulate average symbol power
      pwr_acc += isrran_vec_avg_power_cf(x, ISRRAN_NRE);
    }
  }

  INFO("[PUCCH Format 1 Data RX] d=%+.3f%+.3f", __real__ d, __imag__ d);

  // Demodulate d
  float llr[ISRRAN_PUCCH_NR_FORMAT1_MAX_NOF_BITS];
  isrran_demod_soft_demodulate((nof_bits == 1) ? ISRRAN_MOD_BPSK : ISRRAN_MOD_QPSK, &d, llr, 1);

  // Hard decision based on the LLRs sign
  for (uint32_t i = 0; i < nof_bits; i++) {
    b[i] = llr[i] > 0.0f ? 1 : 0;
  }

  // Calculate normalised correlation, it uses the absolute value of d and accumulated average power
  if (norm_corr != NULL) {
    // Get the number of payload symbols. As the one of every 2 symbols carry DMRS, the payload symbols is half of the
    // total symbols rounding down
    float nsymb = (float)ISRRAN_FLOOR(resource->nof_symbols, 2);

    // Avoid zero, INF or NAN division, set correlation to 0 in this case
    if (isnormal(pwr_acc) && isnormal(nsymb)) {
      *norm_corr = cabsf(d) / sqrtf(pwr_acc * nsymb);
    } else {
      *norm_corr = 0.0f;
    }
  }

  return ISRRAN_SUCCESS;
}

static uint32_t pucch_nr_format2_cinit(const isrran_carrier_nr_t*          carrier,
                                       const isrran_pucch_nr_common_cfg_t* pucch_cfg,
                                       const isrran_uci_cfg_nr_t*          uci_cfg)
{
  uint32_t n_id = (pucch_cfg->scrambling_id_present) ? pucch_cfg->scrambling_id_present : carrier->pci;

  return ((uint32_t)uci_cfg->pucch.rnti << 15U) + n_id;
}

// Implements TS 38.211 section 6.3.2.5 PUCCH format 2
static int pucch_nr_format2_encode(isrran_pucch_nr_t*                  q,
                                   const isrran_carrier_nr_t*          carrier,
                                   const isrran_pucch_nr_common_cfg_t* cfg,
                                   const isrran_pucch_nr_resource_t*   resource,
                                   const isrran_uci_cfg_nr_t*          uci_cfg,
                                   cf_t*                               slot_symbols)
{
  // Validate configuration
  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Calculate number of encoded bits
  int e = isrran_uci_nr_pucch_format_2_3_4_E(resource);
  if (e < ISRRAN_SUCCESS) {
    ERROR("Error selecting E");
    return ISRRAN_ERROR;
  }
  uint32_t E = (uint32_t)e;

  // 6.3.2.5.1 Scrambling
  uint32_t cinit = pucch_nr_format2_cinit(carrier, cfg, uci_cfg);
  isrran_sequence_apply_bit(q->b, q->b, E, cinit);

  // 6.3.2.5.2 Modulation
  isrran_mod_modulate(&q->qpsk, q->b, q->d, E);

  // 6.3.2.5.3 Mapping to physical resources
  uint32_t l_start = resource->start_symbol_idx;
  uint32_t l_end   = resource->start_symbol_idx + resource->nof_symbols;
  uint32_t k_start = ISRRAN_MIN(carrier->nof_prb - 1, resource->starting_prb) * ISRRAN_NRE;
  uint32_t k_end   = ISRRAN_MIN(carrier->nof_prb, resource->starting_prb + resource->nof_prb) * ISRRAN_NRE;
  for (uint32_t l = l_start, i = 0; l < l_end; l++) {
    cf_t* symbol_ptr = &slot_symbols[l * carrier->nof_prb * ISRRAN_NRE];
    for (uint32_t k = k_start; k < k_end; k += 3) {
      symbol_ptr[k]     = q->d[i++];
      symbol_ptr[k + 2] = q->d[i++];
    }
  }

  return ISRRAN_SUCCESS;
}

static int pucch_nr_format2_decode(isrran_pucch_nr_t*                  q,
                                   const isrran_carrier_nr_t*          carrier,
                                   const isrran_pucch_nr_common_cfg_t* cfg,
                                   const isrran_pucch_nr_resource_t*   resource,
                                   const isrran_uci_cfg_nr_t*          uci_cfg,
                                   isrran_chest_ul_res_t*              chest_res,
                                   cf_t*                               slot_symbols,
                                   int8_t*                             llr)
{
  // Validate configuration
  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Calculate number of encoded bits
  int e = isrran_uci_nr_pucch_format_2_3_4_E(resource);
  if (e < ISRRAN_SUCCESS) {
    ERROR("Error selecting E");
    return ISRRAN_ERROR;
  }
  uint32_t E = (uint32_t)e;

  // Undo mapping to physical resources
  uint32_t l_start = resource->start_symbol_idx;
  uint32_t l_end   = resource->start_symbol_idx + resource->nof_symbols;
  uint32_t k_start = resource->starting_prb * ISRRAN_NRE;
  uint32_t k_end   = (resource->starting_prb + resource->nof_prb) * ISRRAN_NRE;
  for (uint32_t l = l_start, i = 0; l < l_end; l++) {
    cf_t* symbol_ptr = &slot_symbols[l * carrier->nof_prb * ISRRAN_NRE];
    cf_t* ce_ptr     = &chest_res->ce[l * carrier->nof_prb * ISRRAN_NRE];
    for (uint32_t k = k_start; k < k_end; k += 3) {
      q->d[i]  = symbol_ptr[k];
      q->ce[i] = ce_ptr[k];
      i++;
      q->d[i]  = symbol_ptr[k + 2];
      q->ce[i] = ce_ptr[k + 2];
      i++;
    }
  }

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    INFO("d=");
    isrran_vec_fprint_c(stdout, q->d, resource->nof_symbols * resource->nof_prb * (ISRRAN_NRE - 4));
    INFO("ce=");
    isrran_vec_fprint_c(stdout, q->ce, resource->nof_symbols * resource->nof_prb * (ISRRAN_NRE - 4));
  }

  // Equalise
  if (isrran_predecoding_single(q->d, q->ce, q->d, NULL, E / 2, 1.0f, chest_res->noise_estimate) < ISRRAN_SUCCESS) {
    ERROR("Error Pre-decoding");
    return ISRRAN_ERROR;
  }

  // Soft-demodulate
  if (isrran_demod_soft_demodulate_b(ISRRAN_MOD_QPSK, q->d, llr, E / 2) < ISRRAN_SUCCESS) {
    ERROR("Error soft-demodulate");
    return ISRRAN_ERROR;
  }

  // Undo Scrambling
  uint32_t cinit = pucch_nr_format2_cinit(carrier, cfg, uci_cfg);
  isrran_sequence_apply_c(llr, llr, E, cinit);

  return ISRRAN_SUCCESS;
}

int isrran_pucch_nr_format_2_3_4_encode(isrran_pucch_nr_t*                  q,
                                        const isrran_pucch_nr_common_cfg_t* cfg,
                                        const isrran_slot_cfg_t*            slot,
                                        const isrran_pucch_nr_resource_t*   resource,
                                        const isrran_uci_cfg_nr_t*          uci_cfg,
                                        const isrran_uci_value_nr_t*        uci_value,
                                        cf_t*                               slot_symbols)
{
  // Validate input pointers
  if (q == NULL || cfg == NULL || slot == NULL || resource == NULL || uci_cfg == NULL || uci_value == NULL ||
      slot_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Encode PUCCH message
  if (isrran_uci_nr_encode_pucch(&q->uci, resource, uci_cfg, uci_value, q->b) < ISRRAN_SUCCESS) {
    ERROR("Error encoding UCI");
    return ISRRAN_ERROR;
  }

  // Modulate PUCCH
  switch (resource->format) {
    case ISRRAN_PUCCH_NR_FORMAT_2:
      return pucch_nr_format2_encode(q, &q->carrier, cfg, resource, uci_cfg, slot_symbols);
    case ISRRAN_PUCCH_NR_FORMAT_3:
    case ISRRAN_PUCCH_NR_FORMAT_4:
      ERROR("Not implemented");
      return ISRRAN_ERROR;
    default:
    case ISRRAN_PUCCH_NR_FORMAT_ERROR:
      ERROR("Invalid format");
  }

  return ISRRAN_ERROR;
}

int isrran_pucch_nr_format_2_3_4_decode(isrran_pucch_nr_t*                  q,
                                        const isrran_pucch_nr_common_cfg_t* cfg,
                                        const isrran_slot_cfg_t*            slot,
                                        const isrran_pucch_nr_resource_t*   resource,
                                        const isrran_uci_cfg_nr_t*          uci_cfg,
                                        isrran_chest_ul_res_t*              chest_res,
                                        cf_t*                               slot_symbols,
                                        isrran_uci_value_nr_t*              uci_value)
{
  // Validate input pointers
  if (q == NULL || cfg == NULL || slot == NULL || resource == NULL || uci_cfg == NULL || chest_res == NULL ||
      uci_value == NULL || slot_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Demodulate PUCCH message
  int8_t* llr = (int8_t*)q->b;
  switch (resource->format) {
    case ISRRAN_PUCCH_NR_FORMAT_2:
      if (pucch_nr_format2_decode(q, &q->carrier, cfg, resource, uci_cfg, chest_res, slot_symbols, llr) <
          ISRRAN_SUCCESS) {
        ERROR("Demodulating PUCCH format 2");
        return ISRRAN_ERROR;
      }
      break;
    case ISRRAN_PUCCH_NR_FORMAT_3:
    case ISRRAN_PUCCH_NR_FORMAT_4:
      ERROR("Not implemented");
      return ISRRAN_ERROR;
    default:
    case ISRRAN_PUCCH_NR_FORMAT_ERROR:
      ERROR("Invalid format");
  }

  // Decode PUCCH message
  if (isrran_uci_nr_decode_pucch(&q->uci, resource, uci_cfg, llr, uci_value) < ISRRAN_SUCCESS) {
    ERROR("Error encoding UCI");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static uint32_t pucch_nr_resource_info(const isrran_pucch_nr_resource_t* r, char* str, uint32_t str_len)
{
  uint32_t len = 0;

  uint32_t nof_prb = 1;
  if (r->format == ISRRAN_PUCCH_NR_FORMAT_2 || r->format == ISRRAN_PUCCH_NR_FORMAT_3) {
    nof_prb = r->nof_prb;
  }

  len = isrran_print_check(str,
                           str_len,
                           len,
                           "f=%d prb=%d:%d symb=%d:%d ",
                           (int)r->format,
                           r->starting_prb,
                           nof_prb,
                           r->start_symbol_idx,
                           r->nof_symbols);

  if (r->intra_slot_hopping) {
    len = isrran_print_check(str, str_len, len, "hop=%d ", r->second_hop_prb);
  }

  if (r->format == ISRRAN_PUCCH_NR_FORMAT_0 || r->format == ISRRAN_PUCCH_NR_FORMAT_1) {
    len = isrran_print_check(str, str_len, len, "cs=%d ", r->initial_cyclic_shift);
  }

  if (r->format == ISRRAN_PUCCH_NR_FORMAT_1) {
    len = isrran_print_check(str, str_len, len, "occ=%d ", r->time_domain_occ);
  }

  if (r->format == ISRRAN_PUCCH_NR_FORMAT_4) {
    len = isrran_print_check(str, str_len, len, "occ=%d:%d ", r->occ_index, r->occ_lenth);
  }

  return len;
}

uint32_t isrran_pucch_nr_info(const isrran_pucch_nr_resource_t* resource,
                              const isrran_uci_data_nr_t*       uci_data,
                              char*                             str,
                              uint32_t                          str_len)
{
  uint32_t len = 0;

  len += pucch_nr_resource_info(resource, &str[len], str_len - len);

  len = isrran_print_check(str, str_len, len, "rnti=0x%x ", uci_data->cfg.pucch.rnti);

  len += isrran_uci_nr_info(uci_data, &str[len], str_len - len);

  return len;
}
