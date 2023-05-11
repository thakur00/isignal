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

#include "isrran/phy/sync/ssb.h"
#include "isrran/phy/ch_estimation/dmrs_pbch.h"
#include "isrran/phy/sync/pss_nr.h"
#include "isrran/phy/sync/sss_nr.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <complex.h>

/*
 * Maximum allowed maximum sampling rate error in Hz
 */
#define SSB_SRATE_MAX_ERROR_HZ 0.01

/*
 * Maximum allowed maximum frequency error offset in Hz
 */
#define SSB_FREQ_OFFSET_MAX_ERROR_HZ 0.01

/*
 * Correlation size in function of the symbol size. It selects a power of two number at least 8 times bigger than the
 * given symbol size but not bigger than 2^13 points.
 */
#define SSB_CORR_SZ(SYMB_SZ) ISRRAN_MIN(1U << (uint32_t)ceil(log2((double)(SYMB_SZ)) + 3.0), 1U << 13U)

/*
 * Default NR-PBCH DMRS normalised correlation (RSRP/EPRE) threshold
 */
#define SSB_PBCH_DMRS_DEFAULT_CORR_THR 0.5f

static int ssb_init_corr(isrran_ssb_t* q)
{
  // Initialise correlation only if it is enabled
  if (!q->args.enable_search) {
    return ISRRAN_SUCCESS;
  }

  // For each PSS sequence allocate
  for (uint32_t N_id_2 = 0; N_id_2 < ISRRAN_NOF_NID_2_NR; N_id_2++) {
    // Allocate sequences
    q->pss_seq[N_id_2] = isrran_vec_cf_malloc(q->max_corr_sz);
    if (q->pss_seq[N_id_2] == NULL) {
      ERROR("Malloc");
      return ISRRAN_ERROR;
    }
  }

  q->sf_buffer = isrran_vec_cf_malloc(q->max_ssb_sz + q->max_sf_sz);
  if (q->sf_buffer == NULL) {
    ERROR("Malloc");
    return ISRRAN_ERROR;
  }
  isrran_vec_cf_zero(q->sf_buffer, q->max_ssb_sz + q->max_sf_sz);

  return ISRRAN_SUCCESS;
}

static int ssb_init_pbch(isrran_ssb_t* q)
{
  isrran_pbch_nr_args_t args = {};
  args.enable_encode         = q->args.enable_encode;
  args.enable_decode         = q->args.enable_decode;
  args.disable_simd          = q->args.disable_polar_simd;

  if (!args.enable_encode && !args.enable_decode) {
    return ISRRAN_SUCCESS;
  }

  if (isrran_pbch_nr_init(&q->pbch, &args) < ISRRAN_SUCCESS) {
    ERROR("Error init NR PBCH");
    return ISRRAN_SUCCESS;
  }

  return ISRRAN_SUCCESS;
}

int isrran_ssb_init(isrran_ssb_t* q, const isrran_ssb_args_t* args)
{
  // Verify input parameters
  if (q == NULL || args == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Copy arguments
  q->args = *args;

  // Check if the maximum sampling rate is in range, force default otherwise
  q->args.max_srate_hz  = (!isnormal(q->args.max_srate_hz)) ? ISRRAN_SSB_DEFAULT_MAX_SRATE_HZ : q->args.max_srate_hz;
  q->args.pbch_dmrs_thr = (!isnormal(q->args.pbch_dmrs_thr)) ? SSB_PBCH_DMRS_DEFAULT_CORR_THR : q->args.pbch_dmrs_thr;

  q->scs_hz        = (float)ISRRAN_SUBC_SPACING_NR(q->args.min_scs);
  q->max_sf_sz     = (uint32_t)round(1e-3 * q->args.max_srate_hz);
  q->max_symbol_sz = (uint32_t)round(q->args.max_srate_hz / q->scs_hz);
  q->max_corr_sz   = SSB_CORR_SZ(q->max_symbol_sz);
  q->max_ssb_sz    = ISRRAN_SSB_DURATION_NSYMB * (q->max_symbol_sz + (144 * q->max_symbol_sz) / 2048);

  // Allocate temporal data
  q->tmp_time = isrran_vec_cf_malloc(q->max_corr_sz);
  q->tmp_freq = isrran_vec_cf_malloc(q->max_corr_sz);
  q->tmp_corr = isrran_vec_cf_malloc(q->max_corr_sz);
  if (q->tmp_time == NULL || q->tmp_freq == NULL || q->tmp_corr == NULL) {
    ERROR("Malloc");
    return ISRRAN_ERROR;
  }

  // Allocate correlation buffers
  if (ssb_init_corr(q) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // PBCH
  if (ssb_init_pbch(q) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

void isrran_ssb_free(isrran_ssb_t* q)
{
  if (q == NULL) {
    return;
  }

  if (q->tmp_time != NULL) {
    free(q->tmp_time);
  }

  if (q->tmp_freq != NULL) {
    free(q->tmp_freq);
  }

  if (q->tmp_corr != NULL) {
    free(q->tmp_corr);
  }

  // For each PSS sequence allocate
  for (uint32_t N_id_2 = 0; N_id_2 < ISRRAN_NOF_NID_2_NR; N_id_2++) {
    if (q->pss_seq[N_id_2] != NULL) {
      free(q->pss_seq[N_id_2]);
    }
  }

  if (q->sf_buffer != NULL) {
    free(q->sf_buffer);
  }

  isrran_dft_plan_free(&q->ifft);
  isrran_dft_plan_free(&q->fft);
  isrran_dft_plan_free(&q->fft_corr);
  isrran_dft_plan_free(&q->ifft_corr);
  isrran_pbch_nr_free(&q->pbch);

  ISRRAN_MEM_ZERO(q, isrran_ssb_t, 1);
}

static uint32_t ssb_first_symbol_caseA(const isrran_ssb_cfg_t* cfg, uint32_t indexes[ISRRAN_SSB_NOF_CANDIDATES])
{
  // Case A - 15 kHz SCS: the first symbols of the candidate SS/PBCH blocks have indexes of { 2 , 8 } + 14 ⋅ n . For
  // carrier frequencies smaller than or equal to 3 GHz, n = 0 , 1 . For carrier frequencies within FR1 larger than 3
  // GHz, n = 0 , 1 , 2 , 3 .
  uint32_t count           = 0;
  uint32_t base_indexes[2] = {2, 8};

  uint32_t N = 2;
  if (cfg->center_freq_hz > 3e9) {
    N = 4;
  }

  for (uint32_t n = 0; n < N; n++) {
    for (uint32_t i = 0; i < 2; i++) {
      indexes[count++] = base_indexes[i] + 14 * n;
    }
  }

  return count;
}

static uint32_t ssb_first_symbol_caseB(const isrran_ssb_cfg_t* cfg, uint32_t indexes[ISRRAN_SSB_NOF_CANDIDATES])
{
  // Case B - 30 kHz SCS: the first symbols of the candidate SS/PBCH blocks have indexes { 4 , 8 , 16 , 20 } + 28 ⋅ n .
  // For carrier frequencies smaller than or equal to 3 GHz, n = 0 . For carrier frequencies within FR1 larger than 3
  // GHz, n = 0 , 1 .
  uint32_t count           = 0;
  uint32_t base_indexes[4] = {4, 8, 16, 20};

  uint32_t N = 1;
  if (cfg->center_freq_hz > 3e9) {
    N = 2;
  }

  for (uint32_t n = 0; n < N; n++) {
    for (uint32_t i = 0; i < 4; i++) {
      indexes[count++] = base_indexes[i] + 28 * n;
    }
  }

  return count;
}

static uint32_t ssb_first_symbol_caseC(const isrran_ssb_cfg_t* cfg, uint32_t indexes[ISRRAN_SSB_NOF_CANDIDATES])
{
  // Case C - 30 kHz SCS: the first symbols of the candidate SS/PBCH blocks have indexes { 2 , 8 } +14 ⋅ n .
  // - For paired spectrum operation
  //   - For carrier frequencies smaller than or equal to 3 GHz, n = 0 , 1 . For carrier frequencies within FR1 larger
  //     than 3 GHz, n = 0 , 1 , 2 , 3 .
  // - For unpaired spectrum operation
  //   - For carrier frequencies smaller than or equal to 2.3 GHz, n = 0 , 1 . For carrier frequencies within FR1
  //     larger than 2.3 GHz, n = 0 , 1 , 2 , 3 .
  uint32_t count           = 0;
  uint32_t base_indexes[2] = {2, 8};

  uint32_t N = 4;
  if ((cfg->duplex_mode == ISRRAN_DUPLEX_MODE_FDD && cfg->center_freq_hz <= 3e9) ||
      (cfg->duplex_mode == ISRRAN_DUPLEX_MODE_TDD && cfg->center_freq_hz <= 2.3e9)) {
    N = 2;
  }

  for (uint32_t n = 0; n < N; n++) {
    for (uint32_t i = 0; i < 2; i++) {
      indexes[count++] = base_indexes[i] + 14 * n;
    }
  }

  return count;
}

static uint32_t ssb_first_symbol_caseD(const isrran_ssb_cfg_t* cfg, uint32_t indexes[ISRRAN_SSB_NOF_CANDIDATES])
{
  // Case D - 120 kHz SCS: the first symbols of the candidate SS/PBCH blocks have indexes { 4 , 8 , 16 , 20 } + 28 ⋅ n .
  // For carrier frequencies within FR2, n = 0 , 1 , 2 , 3 , 5 , 6 , 7 , 8 , 10 , 11 , 12 , 13 , 15 , 16 , 17 , 18 .
  uint32_t count           = 0;
  uint32_t base_indexes[4] = {4, 8, 16, 20};
  uint32_t n_indexes[16]   = {0, 1, 2, 3, 5, 6, 7, 8, 10, 11, 12, 13, 15, 16, 17, 18};

  for (uint32_t j = 0; j < 16; j++) {
    for (uint32_t i = 0; i < 4; i++) {
      indexes[count++] = base_indexes[i] + 28 * n_indexes[j];
    }
  }

  return count;
}

static uint32_t ssb_first_symbol_caseE(const isrran_ssb_cfg_t* cfg, uint32_t indexes[ISRRAN_SSB_NOF_CANDIDATES])
{
  // Case E - 240 kHz SCS: the first symbols of the candidate SS/PBCH blocks have indexes
  //{ 8 , 12 , 16 , 20 , 32 , 36 , 40 , 44 } + 56 ⋅ n . For carrier frequencies within FR2, n = 0 , 1 , 2 , 3 , 5 , 6 ,
  // 7 , 8 .
  uint32_t count           = 0;
  uint32_t base_indexes[8] = {8, 12, 16, 20, 32, 38, 40, 44};
  uint32_t n_indexes[8]    = {0, 1, 2, 3, 5, 6, 7, 8};

  for (uint32_t j = 0; j < 8; j++) {
    for (uint32_t i = 0; i < 8; i++) {
      indexes[count++] = base_indexes[i] + 56 * n_indexes[j];
    }
  }

  return count;
}

static uint32_t ssb_first_symbol(const isrran_ssb_cfg_t* cfg, uint32_t indexes[ISRRAN_SSB_NOF_CANDIDATES])
{
  uint32_t Lmax = 0;

  switch (cfg->pattern) {
    case ISRRAN_SSB_PATTERN_A:
      Lmax = ssb_first_symbol_caseA(cfg, indexes);
      break;
    case ISRRAN_SSB_PATTERN_B:
      Lmax = ssb_first_symbol_caseB(cfg, indexes);
      break;
    case ISRRAN_SSB_PATTERN_C:
      Lmax = ssb_first_symbol_caseC(cfg, indexes);
      break;
    case ISRRAN_SSB_PATTERN_D:
      Lmax = ssb_first_symbol_caseD(cfg, indexes);
      break;
    case ISRRAN_SSB_PATTERN_E:
      Lmax = ssb_first_symbol_caseE(cfg, indexes);
      break;
    case ISRRAN_SSB_PATTERN_INVALID:
      ERROR("Invalid case");
  }
  return Lmax;
}

// Modulates a given symbol l and stores the time domain signal in q->tmp_time
static void ssb_modulate_symbol(isrran_ssb_t* q, cf_t ssb_grid[ISRRAN_SSB_NOF_RE], uint32_t l)
{
  // Select symbol in grid
  cf_t* ptr = &ssb_grid[l * ISRRAN_SSB_BW_SUBC];

  // Initialise frequency domain
  isrran_vec_cf_zero(q->tmp_freq, q->symbol_sz);

  // Map grid into frequency domain symbol
  if (q->f_offset >= ISRRAN_SSB_BW_SUBC / 2) {
    isrran_vec_cf_copy(&q->tmp_freq[q->f_offset - ISRRAN_SSB_BW_SUBC / 2], ptr, ISRRAN_SSB_BW_SUBC);
  } else if (q->f_offset <= -ISRRAN_SSB_BW_SUBC / 2) {
    isrran_vec_cf_copy(&q->tmp_freq[q->symbol_sz + q->f_offset - ISRRAN_SSB_BW_SUBC / 2], ptr, ISRRAN_SSB_BW_SUBC);
  } else {
    isrran_vec_cf_copy(
        &q->tmp_freq[0], &ptr[ISRRAN_SSB_BW_SUBC / 2 - q->f_offset], ISRRAN_SSB_BW_SUBC / 2 + q->f_offset);
    isrran_vec_cf_copy(&q->tmp_freq[q->symbol_sz - ISRRAN_SSB_BW_SUBC / 2 + q->f_offset],
                       &ptr[0],
                       ISRRAN_SSB_BW_SUBC / 2 - q->f_offset);
  }

  // Convert to time domain
  isrran_dft_run_guru_c(&q->ifft);

  // Normalise output
  float norm = sqrtf((float)q->symbol_sz);
  if (isnormal(norm)) {
    isrran_vec_sc_prod_cfc(q->tmp_time, 1.0f / norm, q->tmp_time, q->symbol_sz);
  }
}

static int ssb_setup_corr(isrran_ssb_t* q)
{
  // Skip if disabled
  if (!q->args.enable_search) {
    return ISRRAN_SUCCESS;
  }

  // Compute new correlation size
  uint32_t corr_sz = SSB_CORR_SZ(q->symbol_sz);

  // Skip if the symbol size is unchanged
  if (q->corr_sz == corr_sz) {
    return ISRRAN_SUCCESS;
  }
  q->corr_sz = corr_sz;

  // Select correlation window, return error if the correlation window is smaller than a symbol
  if (corr_sz < 2 * q->symbol_sz) {
    ERROR("Correlation size (%d) is not sufficient (min. %d)", corr_sz, q->symbol_sz * 2);
    return ISRRAN_ERROR;
  }
  q->corr_window = corr_sz - q->symbol_sz;

  // Free correlation
  isrran_dft_plan_free(&q->fft_corr);
  isrran_dft_plan_free(&q->ifft_corr);

  // Prepare correlation FFT
  if (isrran_dft_plan_guru_c(&q->fft_corr, (int)corr_sz, ISRRAN_DFT_FORWARD, q->tmp_time, q->tmp_freq, 1, 1, 1, 1, 1) <
      ISRRAN_SUCCESS) {
    ERROR("Error planning correlation DFT");
    return ISRRAN_ERROR;
  }
  if (isrran_dft_plan_guru_c(
          &q->ifft_corr, (int)corr_sz, ISRRAN_DFT_BACKWARD, q->tmp_corr, q->tmp_time, 1, 1, 1, 1, 1) < ISRRAN_SUCCESS) {
    ERROR("Error planning correlation DFT");
    return ISRRAN_ERROR;
  }

  // Zero the time domain signal last samples
  isrran_vec_cf_zero(&q->tmp_time[q->symbol_sz], q->corr_window);

  // Temporal grid
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};

  // Initialise correlation sequence
  for (uint32_t N_id_2 = 0; N_id_2 < ISRRAN_NOF_NID_2_NR; N_id_2++) {
    // Put the PSS in SSB grid
    if (isrran_pss_nr_put(ssb_grid, N_id_2, 1.0f) < ISRRAN_SUCCESS) {
      ERROR("Error putting PDD N_id_2=%d", N_id_2);
      return ISRRAN_ERROR;
    }

    // Modulate symbol with PSS
    ssb_modulate_symbol(q, ssb_grid, ISRRAN_PSS_NR_SYMBOL_IDX);

    // Convert to frequency domain
    isrran_dft_run_guru_c(&q->fft_corr);

    // Copy frequency domain sequence
    isrran_vec_cf_copy(q->pss_seq[N_id_2], q->tmp_freq, q->corr_sz);
  }

  return ISRRAN_SUCCESS;
}

static inline int ssb_get_t_offset(isrran_ssb_t* q, uint32_t ssb_idx)
{
  // Get baseband time offset from the begining of the half radio frame to the first symbol
  if (ssb_idx >= ISRRAN_SSB_NOF_CANDIDATES) {
    ERROR("Invalid SSB candidate index (%d)", ssb_idx);
    return ISRRAN_ERROR;
  }

  float t_offset_s = isrran_symbol_offset_s(q->l_first[ssb_idx], q->cfg.scs);
  if (isnan(t_offset_s) || isinf(t_offset_s) || t_offset_s < 0.0f) {
    ERROR("Invalid first symbol (l_first=%d)", q->l_first[ssb_idx]);
    return ISRRAN_ERROR;
  }

  return (int)round(t_offset_s * q->cfg.srate_hz);
}

int isrran_ssb_set_cfg(isrran_ssb_t* q, const isrran_ssb_cfg_t* cfg)
{
  // Verify input parameters
  if (q == NULL || cfg == NULL || cfg->pattern == ISRRAN_SSB_PATTERN_INVALID ||
      cfg->duplex_mode == ISRRAN_DUPLEX_MODE_INVALID) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Calculate subcarrier spacing in Hz
  q->scs_hz = (float)ISRRAN_SUBC_SPACING_NR(cfg->scs);

  // Get first symbol
  q->Lmax = ssb_first_symbol(cfg, q->l_first);

  // Calculate SSB symbol size and integer frequency offset
  double   freq_offset_hz = cfg->ssb_freq_hz - cfg->center_freq_hz;
  uint32_t symbol_sz      = (uint32_t)round(cfg->srate_hz / q->scs_hz);
  q->f_offset             = (int32_t)round(freq_offset_hz / q->scs_hz);

  // Calculate cyclic prefix
  q->cp_sz = (144U * symbol_sz) / 2048U;

  // Calculate SSB sampling error and check
  double ssb_srate_error_Hz = ((double)symbol_sz * q->scs_hz) - cfg->srate_hz;
  if (fabs(ssb_srate_error_Hz) > SSB_SRATE_MAX_ERROR_HZ) {
    ERROR("Invalid sampling rate (%.2f MHz)", cfg->srate_hz / 1e6);
    return ISRRAN_ERROR;
  }

  // Calculate SSB offset error and check
  double ssb_offset_error_Hz = ((double)q->f_offset * q->scs_hz) - freq_offset_hz;
  if (fabs(ssb_offset_error_Hz) > SSB_FREQ_OFFSET_MAX_ERROR_HZ) {
    ERROR("SSB Offset (%.1f kHz) error exceeds maximum allowed", freq_offset_hz / 1e3);
    return ISRRAN_ERROR;
  }

  // Verify symbol size
  if (q->max_symbol_sz < symbol_sz) {
    ERROR("New symbol size (%d) exceeds maximum symbol size (%d)", symbol_sz, q->max_symbol_sz);
    return ISRRAN_ERROR;
  }

  // Replan iFFT
  if ((q->args.enable_encode || q->args.enable_search) && q->symbol_sz != symbol_sz) {
    // free the current IFFT, it internally checks if the plan was created
    isrran_dft_plan_free(&q->ifft);

    // Creates DFT plan
    if (isrran_dft_plan_guru_c(&q->ifft, (int)symbol_sz, ISRRAN_DFT_BACKWARD, q->tmp_freq, q->tmp_time, 1, 1, 1, 1, 1) <
        ISRRAN_SUCCESS) {
      ERROR("Error creating iDFT");
      return ISRRAN_ERROR;
    }
  }

  // Replan FFT
  if ((q->args.enable_measure || q->args.enable_decode || q->args.enable_search) && q->symbol_sz != symbol_sz) {
    // free the current FFT, it internally checks if the plan was created
    isrran_dft_plan_free(&q->fft);

    // Creates DFT plan
    if (isrran_dft_plan_guru_c(&q->fft, (int)symbol_sz, ISRRAN_DFT_FORWARD, q->tmp_time, q->tmp_freq, 1, 1, 1, 1, 1) <
        ISRRAN_SUCCESS) {
      ERROR("Error creating iDFT");
      return ISRRAN_ERROR;
    }
  }

  // Finally, copy configuration
  q->cfg       = *cfg;
  q->symbol_sz = symbol_sz;
  q->sf_sz     = (uint32_t)round(1e-3 * cfg->srate_hz);
  q->ssb_sz    = ISRRAN_SSB_DURATION_NSYMB * (q->symbol_sz + q->cp_sz);

  // Initialise correlation
  if (ssb_setup_corr(q) < ISRRAN_SUCCESS) {
    ERROR("Error initialising correlation");
    return ISRRAN_ERROR;
  }

  if (!isnormal(q->cfg.beta_pss)) {
    q->cfg.beta_pss = ISRRAN_SSB_DEFAULT_BETA;
  }

  if (!isnormal(q->cfg.beta_sss)) {
    q->cfg.beta_sss = ISRRAN_SSB_DEFAULT_BETA;
  }

  if (!isnormal(q->cfg.beta_pbch)) {
    q->cfg.beta_pbch = ISRRAN_SSB_DEFAULT_BETA;
  }

  if (!isnormal(q->cfg.beta_pbch_dmrs)) {
    q->cfg.beta_pbch_dmrs = ISRRAN_SSB_DEFAULT_BETA;
  }

  return ISRRAN_SUCCESS;
}

bool isrran_ssb_send(isrran_ssb_t* q, uint32_t sf_idx)
{
  // Verify input
  if (q == NULL) {
    return false;
  }

  // Verify periodicity
  if (q->cfg.periodicity_ms == 0) {
    return false;
  }

  // Check periodicity
  return (sf_idx % q->cfg.periodicity_ms == 0);
}

static int ssb_encode(isrran_ssb_t* q, uint32_t N_id, const isrran_pbch_msg_nr_t* msg, cf_t ssb_grid[ISRRAN_SSB_NOF_RE])
{
  uint32_t N_id_1 = ISRRAN_NID_1_NR(N_id);
  uint32_t N_id_2 = ISRRAN_NID_2_NR(N_id);

  // Put PSS
  if (isrran_pss_nr_put(ssb_grid, N_id_2, q->cfg.beta_pss) < ISRRAN_SUCCESS) {
    ERROR("Error putting PSS");
    return ISRRAN_ERROR;
  }

  // Put SSS
  if (isrran_sss_nr_put(ssb_grid, N_id_1, N_id_2, q->cfg.beta_sss) < ISRRAN_SUCCESS) {
    ERROR("Error putting PSS");
    return ISRRAN_ERROR;
  }

  // Put PBCH DMRS
  isrran_dmrs_pbch_cfg_t pbch_dmrs_cfg = {};
  pbch_dmrs_cfg.N_id                   = N_id;
  pbch_dmrs_cfg.n_hf                   = msg->hrf ? 1 : 0;
  pbch_dmrs_cfg.ssb_idx                = msg->ssb_idx;
  pbch_dmrs_cfg.L_max                  = q->Lmax;
  pbch_dmrs_cfg.beta                   = 0.0f;
  pbch_dmrs_cfg.scs                    = q->cfg.scs;
  if (isrran_dmrs_pbch_put(&pbch_dmrs_cfg, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Error putting PBCH DMRS");
    return ISRRAN_ERROR;
  }

  // Put PBCH payload
  isrran_pbch_nr_cfg_t pbch_cfg = {};
  pbch_cfg.N_id                 = N_id;
  pbch_cfg.n_hf                 = msg->hrf;
  pbch_cfg.ssb_idx              = msg->ssb_idx;
  pbch_cfg.Lmax                 = q->Lmax;
  pbch_cfg.beta                 = 0.0f;
  if (isrran_pbch_nr_encode(&q->pbch, &pbch_cfg, msg, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Error encoding PBCH");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

ISRRAN_API int
isrran_ssb_put_grid(isrran_ssb_t* q, uint32_t N_id, const isrran_pbch_msg_nr_t* msg, cf_t* re_grid, uint32_t grid_bw_sc)
{
  // Verify input parameters
  if (q == NULL || N_id >= ISRRAN_NOF_NID_NR || msg == NULL || re_grid == NULL ||
      grid_bw_sc * ISRRAN_NRE < ISRRAN_SSB_BW_SUBC) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_encode) {
    ERROR("SSB is not configured for encode");
    return ISRRAN_ERROR;
  }

  // Put signals in SSB grid
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  if (ssb_encode(q, N_id, msg, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Putting SSB in grid");
    return ISRRAN_ERROR;
  }

  // First symbol in the half frame
  uint32_t l_first = q->l_first[msg->ssb_idx];

  // Frequency offset fom the bottom of the grid
  uint32_t f_offset = grid_bw_sc / 2 + q->f_offset - ISRRAN_SSB_BW_SUBC / 2;

  // Put SSB grid in the actual resource grid
  for (uint32_t l = 0; l < ISRRAN_SSB_DURATION_NSYMB; l++) {
    isrran_vec_cf_copy(
        &re_grid[grid_bw_sc * (l_first + l) + f_offset], &ssb_grid[ISRRAN_SSB_BW_SUBC * l], ISRRAN_SSB_BW_SUBC);
  }

  return ISRRAN_SUCCESS;
}

int isrran_ssb_add(isrran_ssb_t* q, uint32_t N_id, const isrran_pbch_msg_nr_t* msg, const cf_t* in, cf_t* out)
{
  // Verify input parameters
  if (q == NULL || N_id >= ISRRAN_NOF_NID_NR || msg == NULL || in == NULL || out == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_encode) {
    ERROR("SSB is not configured for encode");
    return ISRRAN_ERROR;
  }

  // Put signals in SSB grid
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  if (ssb_encode(q, N_id, msg, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Putting SSB in grid");
    return ISRRAN_ERROR;
  }

  // Select start symbol from SSB candidate index
  int t_offset = ssb_get_t_offset(q, msg->ssb_idx);
  if (t_offset < ISRRAN_SUCCESS) {
    ERROR("Invalid SSB candidate index");
    return ISRRAN_ERROR;
  }

  // Select input/ouput pointers considering the time offset in the slot
  const cf_t* in_ptr  = &in[t_offset];
  cf_t*       out_ptr = &out[t_offset];

  // For each SSB symbol, modulate
  for (uint32_t l = 0; l < ISRRAN_SSB_DURATION_NSYMB; l++) {
    // Map SSB in resource grid and perform IFFT
    ssb_modulate_symbol(q, ssb_grid, l);

    // Phase compensation
    cf_t phase_compensation = (cf_t)cexp(-I * 2.0 * M_PI * q->cfg.center_freq_hz * (double)t_offset / q->cfg.srate_hz);
    isrran_vec_sc_prod_ccc(q->tmp_time, phase_compensation, q->tmp_time, q->symbol_sz);
    t_offset += (int)(q->symbol_sz + q->cp_sz);

    // Add cyclic prefix to input;
    isrran_vec_sum_ccc(in_ptr, &q->tmp_time[q->symbol_sz - q->cp_sz], out_ptr, q->cp_sz);
    in_ptr += q->cp_sz;
    out_ptr += q->cp_sz;

    // Add symbol to the input baseband
    isrran_vec_sum_ccc(in_ptr, q->tmp_time, out_ptr, q->symbol_sz);
    in_ptr += q->symbol_sz;
    out_ptr += q->symbol_sz;
  }

  return ISRRAN_SUCCESS;
}

static int ssb_demodulate(isrran_ssb_t* q,
                          const cf_t*   in,
                          uint32_t      t_offset,
                          float         coarse_cfo_hz,
                          cf_t          ssb_grid[ISRRAN_SSB_NOF_RE])
{
  const cf_t* in_ptr = &in[t_offset];
  for (uint32_t l = 0; l < ISRRAN_SSB_DURATION_NSYMB; l++) {
    // Advance half CP, to avoid inter symbol interference
    in_ptr += ISRRAN_FLOOR(q->cp_sz, 2);

    // Copy FFT window in temporal time domain buffer
    if (isnormal(coarse_cfo_hz)) {
      isrran_vec_apply_cfo(in_ptr, (float)(-coarse_cfo_hz / q->cfg.srate_hz), q->tmp_time, q->symbol_sz);
    } else {
      isrran_vec_cf_copy(q->tmp_time, in_ptr, q->symbol_sz);
    }
    in_ptr += q->symbol_sz + ISRRAN_CEIL(q->cp_sz, 2);

    // Phase compensation
    cf_t phase_compensation =
        (cf_t)cexp(-I * 2.0 * M_PI * (q->cfg.center_freq_hz - coarse_cfo_hz) * (double)t_offset / q->cfg.srate_hz);
    t_offset += q->symbol_sz + q->cp_sz;

    // Convert to frequency domain
    isrran_dft_run_guru_c(&q->fft);

    // Compensate half CP delay
    isrran_vec_apply_cfo(q->tmp_freq, ISRRAN_CEIL(q->cp_sz, 2) / (float)(q->symbol_sz), q->tmp_freq, q->symbol_sz);

    // Select symbol in grid
    cf_t* ptr = &ssb_grid[l * ISRRAN_SSB_BW_SUBC];

    // Map frequency domain symbol into the SSB grid
    if (q->f_offset >= ISRRAN_SSB_BW_SUBC / 2) {
      isrran_vec_cf_copy(ptr, &q->tmp_freq[q->f_offset - ISRRAN_SSB_BW_SUBC / 2], ISRRAN_SSB_BW_SUBC);
    } else if (q->f_offset <= -ISRRAN_SSB_BW_SUBC / 2) {
      isrran_vec_cf_copy(ptr, &q->tmp_freq[q->symbol_sz + q->f_offset - ISRRAN_SSB_BW_SUBC / 2], ISRRAN_SSB_BW_SUBC);
    } else {
      isrran_vec_cf_copy(
          &ptr[ISRRAN_SSB_BW_SUBC / 2 - q->f_offset], &q->tmp_freq[0], ISRRAN_SSB_BW_SUBC / 2 + q->f_offset);
      isrran_vec_cf_copy(&ptr[0],
                         &q->tmp_freq[q->symbol_sz - ISRRAN_SSB_BW_SUBC / 2 + q->f_offset],
                         ISRRAN_SSB_BW_SUBC / 2 - q->f_offset);
    }

    // Normalize
    float norm = sqrtf((float)q->symbol_sz);
    if (isnormal(norm)) {
      isrran_vec_sc_prod_ccc(ptr, conjf(phase_compensation) / norm, ptr, ISRRAN_SSB_BW_SUBC);
    }
  }

  return ISRRAN_SUCCESS;
}

static int
ssb_measure(isrran_ssb_t* q, const cf_t ssb_grid[ISRRAN_SSB_NOF_RE], uint32_t N_id, isrran_csi_trs_measurements_t* meas)
{
  uint32_t N_id_1 = ISRRAN_NID_1_NR(N_id);
  uint32_t N_id_2 = ISRRAN_NID_2_NR(N_id);

  // Extract PSS and SSS LSE
  cf_t pss_lse[ISRRAN_PSS_NR_LEN];
  cf_t sss_lse[ISRRAN_SSS_NR_LEN];
  if (isrran_pss_nr_extract_lse(ssb_grid, N_id_2, pss_lse) < ISRRAN_SUCCESS ||
      isrran_sss_nr_extract_lse(ssb_grid, N_id_1, N_id_2, sss_lse) < ISRRAN_SUCCESS) {
    ERROR("Error extracting LSE");
    return ISRRAN_ERROR;
  }

  // Estimate average delay
  float delay_pss_norm = isrran_vec_estimate_frequency(pss_lse, ISRRAN_PSS_NR_LEN);
  float delay_sss_norm = isrran_vec_estimate_frequency(sss_lse, ISRRAN_SSS_NR_LEN);
  float delay_avg_norm = (delay_pss_norm + delay_sss_norm) / 2.0f;
  float delay_avg_us   = 1e6f * delay_avg_norm / q->scs_hz;

  // Pre-compensate delay
  cf_t ssb_grid_corrected[ISRRAN_SSB_NOF_RE];
  for (uint32_t l = 0; l < ISRRAN_SSB_DURATION_NSYMB; l++) {
    isrran_vec_apply_cfo(&ssb_grid[ISRRAN_SSB_BW_SUBC * l],
                         delay_avg_norm,
                         &ssb_grid_corrected[ISRRAN_SSB_BW_SUBC * l],
                         ISRRAN_SSB_BW_SUBC);
  }

  // Extract LSE again
  if (isrran_pss_nr_extract_lse(ssb_grid_corrected, N_id_2, pss_lse) < ISRRAN_SUCCESS ||
      isrran_sss_nr_extract_lse(ssb_grid_corrected, N_id_1, N_id_2, sss_lse) < ISRRAN_SUCCESS) {
    ERROR("Error extracting LSE");
    return ISRRAN_ERROR;
  }

  // Estimate average EPRE
  float epre_pss = isrran_vec_avg_power_cf(pss_lse, ISRRAN_PSS_NR_LEN);
  float epre_sss = isrran_vec_avg_power_cf(sss_lse, ISRRAN_SSS_NR_LEN);
  float epre     = (epre_pss + epre_sss) / 2.0f;

  // Compute correlation
  cf_t corr_pss = isrran_vec_acc_cc(pss_lse, ISRRAN_PSS_NR_LEN) / ISRRAN_PSS_NR_LEN;
  cf_t corr_sss = isrran_vec_acc_cc(sss_lse, ISRRAN_SSS_NR_LEN) / ISRRAN_SSS_NR_LEN;

  // Compute CFO in Hz
  float distance_s = isrran_symbol_distance_s(ISRRAN_PSS_NR_SYMBOL_IDX, ISRRAN_SSS_NR_SYMBOL_IDX, q->cfg.scs);
  float cfo_hz_max = 1.0f / distance_s;
  float cfo_hz     = cargf(corr_sss * conjf(corr_pss)) / (2.0f * M_PI) * cfo_hz_max;

  // Compute average RSRP
  float rsrp_pss = ISRRAN_CSQABS(corr_pss);
  float rsrp_sss = ISRRAN_CSQABS(corr_sss);
  float rsrp     = (rsrp_pss + rsrp_sss) / 2.0f;

  // Avoid taking log of 0 or another abnormal value
  if (!isnormal(rsrp)) {
    rsrp = 1e-9f;
  }

  // Estimate Noise:
  // - Infinite (1e9), if the EPRE or RSRP is zero
  // - EPRE-RSRP if EPRE > RSRP
  // - zero (1e-9), otherwise
  float n0_pss = 1e-9f;
  if (!isnormal(epre_pss) || !isnormal(rsrp_pss)) {
    n0_pss = 1e9f;
  } else if (epre_pss > rsrp_pss) {
    n0_pss = epre - rsrp_pss;
  }
  float n0_sss = 1e-9f;
  if (!isnormal(epre_sss) || !isnormal(rsrp_sss)) {
    n0_sss = 1e9f;
  } else if (epre_sss > rsrp_sss) {
    n0_sss = epre - rsrp_sss;
  }
  float n0 = (n0_pss + n0_sss) / 2.0f;

  // Put measurements together
  meas->epre       = epre;
  meas->epre_dB    = isrran_convert_power_to_dB(epre);
  meas->rsrp       = rsrp;
  meas->rsrp_dB    = isrran_convert_power_to_dB(rsrp);
  meas->n0         = n0;
  meas->n0_dB      = isrran_convert_power_to_dB(n0);
  meas->snr_dB     = meas->rsrp_dB - meas->n0_dB;
  meas->cfo_hz     = cfo_hz;
  meas->cfo_hz_max = cfo_hz_max;
  meas->delay_us   = delay_avg_us; // Convert the delay to microseconds
  meas->nof_re     = ISRRAN_PSS_NR_LEN + ISRRAN_SSS_NR_LEN;

  return ISRRAN_SUCCESS;
}

static void ssb_vec_prod_conj_circ_shift(const cf_t* a, const cf_t* b, cf_t* c, uint32_t n, int shift)
{
  uint32_t offset = (uint32_t)abs(shift);

  // Avoid negative number of samples
  if (offset > n) {
    isrran_vec_cf_zero(c, n);
    return;
  }

  // Shift is negative
  if (shift < 0) {
    isrran_vec_prod_conj_ccc(&a[offset], &b[0], &c[0], n - offset);
    isrran_vec_prod_conj_ccc(&a[0], &b[n - offset], &c[n - offset], offset);
    return;
  }

  // Shift is positive
  if (shift > 0) {
    isrran_vec_prod_conj_ccc(&a[0], &b[offset], &c[0], n - offset);
    isrran_vec_prod_conj_ccc(&a[n - offset], &b[0], &c[n - offset], offset);
    return;
  }

  // Shift is zero
  isrran_vec_prod_conj_ccc(a, b, c, n);
}

static int ssb_pss_search(isrran_ssb_t* q,
                          const cf_t*   in,
                          uint32_t      nof_samples,
                          uint32_t*     found_N_id_2,
                          uint32_t*     found_delay,
                          float*        coarse_cfo_hz)
{
  // verify it is initialised
  if (q->corr_sz == 0) {
    return ISRRAN_ERROR;
  }

  // Calculate correlation CFO coarse precision
  double coarse_cfo_ref_hz = (q->cfg.srate_hz / q->corr_sz);

  // Calculate shift integer range to detect the signal with a maximum CFO equal to the SSB subcarrier spacing
  int shift_range = (int)ceil(ISRRAN_SUBC_SPACING_NR(q->cfg.scs) / coarse_cfo_ref_hz);

  // Calculate the coarse shift increment for half of the subcarrier spacing
  int shift_coarse_inc = shift_range / 2;

  // Correlation best sequence
  float    best_corr   = 0;
  uint32_t best_delay  = 0;
  uint32_t best_N_id_2 = 0;
  int      best_shift  = 0;

  // Delay in correlation window
  uint32_t t_offset = 0;
  while ((t_offset + q->symbol_sz) < nof_samples) {
    // Number of samples taken in this iteration
    uint32_t n = q->corr_sz;

    // Detect if the correlation input exceeds the input length, take the maximum amount of samples
    if (t_offset + q->corr_sz > nof_samples) {
      n = nof_samples - t_offset;
    }

    // Copy the amount of samples
    isrran_vec_cf_copy(q->tmp_time, &in[t_offset], n);

    // Append zeros if there is space left
    if (n < q->corr_sz) {
      isrran_vec_cf_zero(&q->tmp_time[n], q->corr_sz - n);
    }

    // Convert to frequency domain
    isrran_dft_run_guru_c(&q->fft_corr);

    // Try each N_id_2 sequence
    for (uint32_t N_id_2 = 0; N_id_2 < ISRRAN_NOF_NID_2_NR; N_id_2++) {
      // Steer coarse frequency offset
      for (int shift = -shift_range; shift <= shift_range; shift += shift_coarse_inc) {
        // Actual correlation in frequency domain
        ssb_vec_prod_conj_circ_shift(q->tmp_freq, q->pss_seq[N_id_2], q->tmp_corr, q->corr_sz, shift);

        // Convert to time domain
        isrran_dft_run_guru_c(&q->ifft_corr);

        // Find maximum
        uint32_t peak_idx = isrran_vec_max_abs_ci(q->tmp_time, q->corr_window);

        // Average power, take total power of the frequency domain signal after filtering, skip correlation window if
        // value is invalid (0.0, nan or inf)
        float avg_pwr_corr = isrran_vec_avg_power_cf(q->tmp_corr, q->corr_sz);
        if (!isnormal(avg_pwr_corr)) {
          continue;
        }

        // Normalise correlation
        float corr = ISRRAN_CSQABS(q->tmp_time[peak_idx]) / avg_pwr_corr / sqrtf(ISRRAN_PSS_NR_LEN);

        // Update if the correlation is better than the current best
        if (best_corr < corr) {
          best_corr   = corr;
          best_delay  = peak_idx + t_offset;
          best_N_id_2 = N_id_2;
          best_shift  = shift;
        }
      }
    }

    // Advance time
    t_offset += q->corr_window;
  }

  // From the best sequence correlate in frequency domain
  {
    // Reset best correlation
    best_corr = 0.0f;

    // Number of samples taken in this iteration
    uint32_t n = q->corr_sz;

    // Detect if the correlation input exceeds the input length, take the maximum amount of samples
    if (best_delay + q->corr_sz > nof_samples) {
      n = nof_samples - best_delay;
    }

    // Copy the amount of samples
    isrran_vec_cf_copy(q->tmp_time, &in[best_delay], n);

    // Append zeros if there is space left
    if (n < q->corr_sz) {
      isrran_vec_cf_zero(&q->tmp_time[n], q->corr_sz - n);
    }

    // Convert to frequency domain
    isrran_dft_run_guru_c(&q->fft_corr);

    for (int shift = -shift_range; shift <= shift_range; shift++) {
      // Actual correlation in frequency domain
      ssb_vec_prod_conj_circ_shift(q->tmp_freq, q->pss_seq[best_N_id_2], q->tmp_corr, q->corr_sz, shift);

      // Calculate correlation assuming the peak is in the first sample
      float corr = ISRRAN_CSQABS(isrran_vec_acc_cc(q->tmp_corr, q->corr_sz));

      // Update if the correlation is better than the current best
      if (best_corr < corr) {
        best_corr  = corr;
        best_shift = shift;
      }
    }
  }

  // Save findings
  *found_delay   = best_delay;
  *found_N_id_2  = best_N_id_2;
  *coarse_cfo_hz = -(float)best_shift * coarse_cfo_ref_hz;

  return ISRRAN_SUCCESS;
}

int isrran_ssb_csi_search(isrran_ssb_t*                  q,
                          const cf_t*                    in,
                          uint32_t                       nof_samples,
                          uint32_t*                      N_id,
                          isrran_csi_trs_measurements_t* meas)
{
  // Verify inputs
  if (q == NULL || in == NULL || N_id == NULL || meas == NULL || !isnormal(q->scs_hz)) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_search) {
    ERROR("SSB is not configured for search");
    return ISRRAN_ERROR;
  }

  // Avoid finding a peak in a region that cannot be demodulated
  if (nof_samples < (q->symbol_sz + q->cp_sz) * ISRRAN_SSB_DURATION_NSYMB) {
    ERROR("Insufficient number of samples (%d/%d)", nof_samples, (q->symbol_sz + q->cp_sz) * ISRRAN_SSB_DURATION_NSYMB);
    return ISRRAN_ERROR;
  }
  nof_samples -= (q->symbol_sz + q->cp_sz) * ISRRAN_SSB_DURATION_NSYMB;

  // Search for PSS in time domain
  uint32_t N_id_2        = 0;
  uint32_t t_offset      = 0;
  float    coarse_cfo_hz = 0.0f;
  if (ssb_pss_search(q, in, nof_samples, &N_id_2, &t_offset, &coarse_cfo_hz) < ISRRAN_SUCCESS) {
    ERROR("Error searching for N_id_2");
    return ISRRAN_ERROR;
  }

  // Remove CP offset prior demodulation
  if (t_offset >= q->cp_sz) {
    t_offset -= q->cp_sz;
  } else {
    t_offset = 0;
  }

  // Make sure SSB time offset is in bounded in the input buffer
  if (t_offset + q->ssb_sz > nof_samples) {
    return ISRRAN_SUCCESS;
  }

  // Demodulate
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  if (ssb_demodulate(q, in, t_offset, coarse_cfo_hz, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Error demodulating");
    return ISRRAN_ERROR;
  }

  // Find best N_id_1
  uint32_t N_id_1   = 0;
  float    sss_corr = 0.0f;
  if (isrran_sss_nr_find(ssb_grid, N_id_2, &sss_corr, &N_id_1) < ISRRAN_SUCCESS) {
    ERROR("Error searching for N_id_2");
    return ISRRAN_ERROR;
  }

  // Select N_id
  *N_id = ISRRAN_NID_NR(N_id_1, N_id_2);

  // Measure selected N_id
  if (ssb_measure(q, ssb_grid, *N_id, meas)) {
    ERROR("Error measuring");
    return ISRRAN_ERROR;
  }

  // Add delay to measure
  meas->delay_us += (float)(1e6 * t_offset / q->cfg.srate_hz);
  meas->cfo_hz -= coarse_cfo_hz;

  return ISRRAN_SUCCESS;
}

int isrran_ssb_csi_measure(isrran_ssb_t*                  q,
                           uint32_t                       N_id,
                           uint32_t                       ssb_idx,
                           const cf_t*                    in,
                           isrran_csi_trs_measurements_t* meas)
{
  // Verify inputs
  if (q == NULL || N_id >= ISRRAN_NOF_NID_NR || in == NULL || meas == NULL || !isnormal(q->scs_hz)) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_measure) {
    ERROR("SSB is not configured to measure");
    return ISRRAN_ERROR;
  }

  // Select start symbol from SSB candidate index
  int t_offset = ssb_get_t_offset(q, ssb_idx);
  if (t_offset < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Demodulate
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  if (ssb_demodulate(q, in, (uint32_t)t_offset, 0.0f, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Error demodulating");
    return ISRRAN_ERROR;
  }

  // Actual measurement
  if (ssb_measure(q, ssb_grid, N_id, meas)) {
    ERROR("Error measuring");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int ssb_select_pbch(isrran_ssb_t*            q,
                           uint32_t                 N_id,
                           const cf_t               ssb_grid[ISRRAN_SSB_NOF_RE],
                           uint32_t*                found_n_hf,
                           uint32_t*                found_ssb_idx_4lsb,
                           isrran_dmrs_pbch_meas_t* pbch_meas)
{
  // Prepare PBCH DMRS configuration
  isrran_dmrs_pbch_cfg_t pbch_dmrs_cfg = {};
  pbch_dmrs_cfg.N_id                   = N_id;
  pbch_dmrs_cfg.n_hf                   = 0; // Parameter to guess
  pbch_dmrs_cfg.ssb_idx                = 0; // Parameter to guess
  pbch_dmrs_cfg.L_max                  = q->Lmax;
  pbch_dmrs_cfg.beta                   = 0.0f;
  pbch_dmrs_cfg.scs                    = q->cfg.scs;

  // Initialise best values
  isrran_dmrs_pbch_meas_t best_meas    = {};
  uint32_t                best_n_hf    = 0;
  uint32_t                best_ssb_idx = 0;

  // Iterate over all the parameters to guess and select the most suitable
  for (uint32_t n_hf = 0; n_hf < 2; n_hf++) {
    for (uint32_t ssb_idx = 0; ssb_idx < ISRRAN_MIN(8, q->Lmax); ssb_idx++) {
      // Set parameters
      pbch_dmrs_cfg.n_hf    = n_hf;
      pbch_dmrs_cfg.ssb_idx = ssb_idx;

      // Measure
      isrran_dmrs_pbch_meas_t meas = {};
      if (isrran_dmrs_pbch_measure(&pbch_dmrs_cfg, ssb_grid, &meas) < ISRRAN_SUCCESS) {
        ERROR("Error measure for n_hf=%d ssb_idx=%d", n_hf, ssb_idx);
        return ISRRAN_ERROR;
      }

      // Select the result with highest correlation (most suitable)
      if (meas.corr > best_meas.corr) {
        best_meas    = meas;
        best_n_hf    = n_hf;
        best_ssb_idx = ssb_idx;
      }
    }
  }

  // Save findings
  *found_n_hf         = best_n_hf;
  *found_ssb_idx_4lsb = best_ssb_idx;
  *pbch_meas          = best_meas;

  return ISRRAN_SUCCESS;
}

static int ssb_decode_pbch(isrran_ssb_t*         q,
                           uint32_t              N_id,
                           uint32_t              n_hf,
                           uint32_t              ssb_idx,
                           const cf_t            ssb_grid[ISRRAN_SSB_NOF_RE],
                           isrran_pbch_msg_nr_t* msg)
{
  // Prepare PBCH DMRS configuration
  isrran_dmrs_pbch_cfg_t pbch_dmrs_cfg = {};
  pbch_dmrs_cfg.N_id                   = N_id;
  pbch_dmrs_cfg.n_hf                   = n_hf;
  pbch_dmrs_cfg.ssb_idx                = ssb_idx;
  pbch_dmrs_cfg.L_max                  = q->Lmax;
  pbch_dmrs_cfg.beta                   = 0.0f;
  pbch_dmrs_cfg.scs                    = q->cfg.scs;

  // Compute PBCH channel estimates
  cf_t ce[ISRRAN_SSB_NOF_RE] = {};
  if (isrran_dmrs_pbch_estimate(&pbch_dmrs_cfg, ssb_grid, ce) < ISRRAN_SUCCESS) {
    ERROR("Error estimating channel");
    return ISRRAN_ERROR;
  }

  // Prepare PBCH configuration
  isrran_pbch_nr_cfg_t pbch_cfg = {};
  pbch_cfg.N_id                 = N_id;
  pbch_cfg.n_hf                 = n_hf;
  pbch_cfg.ssb_idx              = ssb_idx;
  pbch_cfg.Lmax                 = q->Lmax;
  pbch_cfg.beta                 = 0.0f;

  // Decode
  if (isrran_pbch_nr_decode(&q->pbch, &pbch_cfg, ssb_grid, ce, msg) < ISRRAN_SUCCESS) {
    ERROR("Error decoding PBCH");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_ssb_decode_grid(isrran_ssb_t*         q,
                           uint32_t              N_id,
                           uint32_t              n_hf,
                           uint32_t              ssb_idx,
                           const cf_t*           re_grid,
                           uint32_t              grid_bw_sc,
                           isrran_pbch_msg_nr_t* msg)
{
  // Verify input parameters
  if (q == NULL || N_id >= ISRRAN_NOF_NID_NR || msg == NULL || re_grid == NULL || grid_bw_sc < ISRRAN_SSB_BW_SUBC) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_encode) {
    ERROR("SSB is not configured for encode");
    return ISRRAN_ERROR;
  }

  // First symbol in the half frame
  uint32_t l_first = q->l_first[ssb_idx];

  // Frequency offset fom the bottom of the grid
  uint32_t f_offset = grid_bw_sc / 2 + q->f_offset - ISRRAN_SSB_BW_SUBC / 2;

  // Get SSB grid from resource grid
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  for (uint32_t l = 0; l < ISRRAN_SSB_DURATION_NSYMB; l++) {
    isrran_vec_cf_copy(
        &ssb_grid[ISRRAN_SSB_BW_SUBC * l], &re_grid[grid_bw_sc * (l_first + l) + f_offset], ISRRAN_SSB_BW_SUBC);
  }

  // Decode PBCH
  if (ssb_decode_pbch(q, N_id, n_hf, ssb_idx, ssb_grid, msg) < ISRRAN_SUCCESS) {
    ERROR("Error decoding");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_ssb_decode_pbch(isrran_ssb_t*         q,
                           uint32_t              N_id,
                           uint32_t              n_hf,
                           uint32_t              ssb_idx,
                           const cf_t*           in,
                           isrran_pbch_msg_nr_t* msg)
{
  // Verify inputs
  if (q == NULL || N_id >= ISRRAN_NOF_NID_NR || in == NULL || msg == NULL || !isnormal(q->scs_hz)) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_decode) {
    ERROR("SSB is not configured to decode");
    return ISRRAN_ERROR;
  }

  // Select start symbol from SSB candidate index
  int t_offset = ssb_get_t_offset(q, ssb_idx);
  if (t_offset < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Demodulate
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  if (ssb_demodulate(q, in, (uint32_t)t_offset, 0.0f, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Error demodulating");
    return ISRRAN_ERROR;
  }

  // Decode PBCH
  if (ssb_decode_pbch(q, N_id, n_hf, ssb_idx, ssb_grid, msg) < ISRRAN_SUCCESS) {
    ERROR("Error decoding");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_ssb_search(isrran_ssb_t* q, const cf_t* in, uint32_t nof_samples, isrran_ssb_search_res_t* res)
{
  // Verify inputs
  if (q == NULL || in == NULL || res == NULL || !isnormal(q->scs_hz)) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_search || !q->args.enable_decode) {
    ERROR("SSB is not configured to search (%c) and decode (%c)",
          q->args.enable_search ? 'y' : 'n',
          q->args.enable_decode ? 'y' : 'n');
    return ISRRAN_ERROR;
  }

  // Set the SSB search result with default value with PBCH CRC unmatched, meaning no cell is found
  ISRRAN_MEM_ZERO(res, isrran_ssb_search_res_t, 1);

  // Search for PSS in time domain
  uint32_t N_id_2        = 0;
  uint32_t t_offset      = 0;
  float    coarse_cfo_hz = 0.0f;
  if (ssb_pss_search(q, in, nof_samples, &N_id_2, &t_offset, &coarse_cfo_hz) < ISRRAN_SUCCESS) {
    ERROR("Error searching for N_id_2");
    return ISRRAN_ERROR;
  }

  // Remove CP offset prior demodulation
  if (t_offset >= q->cp_sz) {
    t_offset -= q->cp_sz;
  } else {
    t_offset = 0;
  }

  // Make sure SSB time offset is in bounded in the input buffer
  if (t_offset + q->ssb_sz > nof_samples) {
    return ISRRAN_SUCCESS;
  }

  // Demodulate
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  if (ssb_demodulate(q, in, t_offset, coarse_cfo_hz, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Error demodulating");
    return ISRRAN_ERROR;
  }

  // Find best N_id_1
  uint32_t N_id_1   = 0;
  float    sss_corr = 0.0f;
  if (isrran_sss_nr_find(ssb_grid, N_id_2, &sss_corr, &N_id_1) < ISRRAN_SUCCESS) {
    ERROR("Error searching for N_id_2");
    return ISRRAN_ERROR;
  }

  // Select N_id
  uint32_t N_id = ISRRAN_NID_NR(N_id_1, N_id_2);

  // Select the most suitable SSB candidate
  uint32_t                n_hf      = 0;
  uint32_t                ssb_idx   = 0;
  isrran_dmrs_pbch_meas_t pbch_meas = {};
  if (ssb_select_pbch(q, N_id, ssb_grid, &n_hf, &ssb_idx, &pbch_meas) < ISRRAN_SUCCESS) {
    ERROR("Error selecting PBCH");
    return ISRRAN_ERROR;
  }

  // Avoid decoding if the selected PBCH DMRS do not reach the minimum threshold
  if (pbch_meas.corr < q->args.pbch_dmrs_thr) {
    return ISRRAN_SUCCESS;
  }

  // Decode PBCH
  isrran_pbch_msg_nr_t pbch_msg = {};
  if (ssb_decode_pbch(q, N_id, n_hf, ssb_idx, ssb_grid, &pbch_msg) < ISRRAN_SUCCESS) {
    ERROR("Error decoding PBCH");
    return ISRRAN_ERROR;
  }

  // If PBCH was not decoded, skip measurements
  if (!pbch_msg.crc) {
    return ISRRAN_SUCCESS;
  }

  // Perform measurements from PSS and SSS
  isrran_csi_trs_measurements_t measurements = {};
  if (ssb_measure(q, ssb_grid, N_id, &measurements) < ISRRAN_SUCCESS) {
    ERROR("Error measuring");
    return ISRRAN_ERROR;
  }

  // Save result
  res->N_id         = N_id;
  res->t_offset     = t_offset;
  res->pbch_msg     = pbch_msg;
  res->measurements = measurements;
  res->measurements.cfo_hz += coarse_cfo_hz;

  return ISRRAN_SUCCESS;
}

static int ssb_pss_find(isrran_ssb_t* q, const cf_t* in, uint32_t nof_samples, uint32_t N_id_2, uint32_t* found_delay)
{
  // verify it is initialised
  if (q->corr_sz == 0) {
    return ISRRAN_ERROR;
  }

  // Correlation best sequence
  float    best_corr  = 0;
  uint32_t best_delay = 0;

  // Delay in correlation window
  uint32_t t_offset = 0;
  while ((t_offset + q->symbol_sz) < nof_samples) {
    // Number of samples taken in this iteration
    uint32_t n = q->corr_sz;

    // Detect if the correlation input exceeds the input length, take the maximum amount of samples
    if (t_offset + q->corr_sz > nof_samples) {
      n = nof_samples - t_offset;
    }

    // Copy the amount of samples
    isrran_vec_cf_copy(q->tmp_time, &in[t_offset], n);

    // Append zeros if there is space left
    if (n < q->corr_sz) {
      isrran_vec_cf_zero(&q->tmp_time[n], q->corr_sz - n);
    }

    // Convert to frequency domain
    isrran_dft_run_guru_c(&q->fft_corr);

    // Actual correlation in frequency domain
    isrran_vec_prod_conj_ccc(q->tmp_freq, q->pss_seq[N_id_2], q->tmp_corr, q->corr_sz);

    // Convert to time domain
    isrran_dft_run_guru_c(&q->ifft_corr);

    // Find maximum
    uint32_t peak_idx = isrran_vec_max_abs_ci(q->tmp_time, q->corr_window);

    // Average power, skip window if value is invalid (0.0, nan or inf)
    float avg_pwr_corr = isrran_vec_avg_power_cf(&q->tmp_time[peak_idx], q->symbol_sz);
    if (!isnormal(avg_pwr_corr)) {
      // Advance time
      t_offset += q->corr_window;
      continue;
    }

    // Normalise correlation
    float corr = ISRRAN_CSQABS(q->tmp_time[peak_idx]) / avg_pwr_corr / sqrtf(ISRRAN_PSS_NR_LEN);

    // Update if the correlation is better than the current best
    if (best_corr < corr) {
      best_corr  = corr;
      best_delay = peak_idx + t_offset;
    }

    // Advance time
    t_offset += q->corr_window;
  }

  // Save findings
  *found_delay = best_delay;

  return ISRRAN_SUCCESS;
}

int isrran_ssb_find(isrran_ssb_t*                  q,
                    const cf_t*                    sf_buffer,
                    uint32_t                       N_id,
                    isrran_csi_trs_measurements_t* meas,
                    isrran_pbch_msg_nr_t*          pbch_msg)
{
  // Verify inputs
  if (q == NULL || sf_buffer == NULL || meas == NULL || !isnormal(q->scs_hz)) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_search) {
    ERROR("SSB is not configured for search");
    return ISRRAN_ERROR;
  }

  // Set the PBCH message result with default value (CRC unmatched), meaning no cell is found
  ISRRAN_MEM_ZERO(pbch_msg, isrran_pbch_msg_nr_t, 1);

  // Copy tail from previous execution into the start of this
  isrran_vec_cf_copy(q->sf_buffer, &q->sf_buffer[q->sf_sz], q->ssb_sz);

  // Append new samples
  isrran_vec_cf_copy(&q->sf_buffer[q->ssb_sz], sf_buffer, q->sf_sz);

  // Search for PSS in time domain
  uint32_t t_offset = 0;
  if (ssb_pss_find(q, q->sf_buffer, q->sf_sz + q->ssb_sz, ISRRAN_NID_2_NR(N_id), &t_offset) < ISRRAN_SUCCESS) {
    ERROR("Error searching for N_id_2");
    return ISRRAN_ERROR;
  }

  // Remove CP offset prior demodulation
  if (t_offset >= q->cp_sz) {
    t_offset -= q->cp_sz;
  } else {
    t_offset = 0;
  }

  // Make sure SSB time offset is in bounded in the input buffer
  if (t_offset > q->sf_sz) {
    return ISRRAN_SUCCESS;
  }

  // Demodulate
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  if (ssb_demodulate(q, q->sf_buffer, t_offset, 0.0f, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Error demodulating");
    return ISRRAN_ERROR;
  }

  // Measure selected N_id
  if (ssb_measure(q, ssb_grid, N_id, meas)) {
    ERROR("Error measuring");
    return ISRRAN_ERROR;
  }

  // Select the most suitable SSB candidate
  uint32_t                n_hf      = 0;
  uint32_t                ssb_idx   = 0; // SSB candidate index
  isrran_dmrs_pbch_meas_t pbch_meas = {};
  if (ssb_select_pbch(q, N_id, ssb_grid, &n_hf, &ssb_idx, &pbch_meas) < ISRRAN_SUCCESS) {
    ERROR("Error selecting PBCH");
    return ISRRAN_ERROR;
  }

  // Avoid decoding if the selected PBCH DMRS do not reach the minimum threshold
  if (pbch_meas.corr < q->args.pbch_dmrs_thr) {
    return ISRRAN_SUCCESS;
  }

  // Calculate the SSB offset in the subframe
  uint32_t ssb_offset = isrran_ssb_candidate_sf_offset(q, ssb_idx);

  // Compute PBCH channel estimates
  if (ssb_decode_pbch(q, N_id, n_hf, ssb_idx, ssb_grid, pbch_msg) < ISRRAN_SUCCESS) {
    ERROR("Error decoding PBCH");
    return ISRRAN_ERROR;
  }

  // SSB delay in SF
  float ssb_delay_us = (float)(1e6 * (((double)t_offset - (double)q->ssb_sz - (double)ssb_offset) / q->cfg.srate_hz));

  // Add delay to measure
  meas->delay_us += ssb_delay_us;

  return ISRRAN_SUCCESS;
}

int isrran_ssb_track(isrran_ssb_t*                  q,
                     const cf_t*                    sf_buffer,
                     uint32_t                       N_id,
                     uint32_t                       ssb_idx,
                     uint32_t                       n_hf,
                     isrran_csi_trs_measurements_t* meas,
                     isrran_pbch_msg_nr_t*          pbch_msg)
{
  // Verify inputs
  if (q == NULL || sf_buffer == NULL || meas == NULL || !isnormal(q->scs_hz)) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!q->args.enable_search) {
    ERROR("SSB is not configured for search");
    return ISRRAN_ERROR;
  }

  // Calculate SSB offset
  uint32_t t_offset = isrran_ssb_candidate_sf_offset(q, ssb_idx);

  // Demodulate
  cf_t ssb_grid[ISRRAN_SSB_NOF_RE] = {};
  if (ssb_demodulate(q, sf_buffer, t_offset, 0.0f, ssb_grid) < ISRRAN_SUCCESS) {
    ERROR("Error demodulating");
    return ISRRAN_ERROR;
  }

  // Measure selected N_id
  if (ssb_measure(q, ssb_grid, N_id, meas)) {
    ERROR("Error measuring");
    return ISRRAN_ERROR;
  }

  // Compute PBCH channel estimates
  if (ssb_decode_pbch(q, N_id, n_hf, ssb_idx, ssb_grid, pbch_msg) < ISRRAN_SUCCESS) {
    ERROR("Error decoding PBCH");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

uint32_t isrran_ssb_candidate_sf_idx(const isrran_ssb_t* q, uint32_t ssb_idx, bool half_frame)
{
  if (q == NULL) {
    return 0;
  }

  uint32_t nof_symbols_subframe = ISRRAN_NSYMB_PER_SLOT_NR * ISRRAN_NSLOTS_PER_SF_NR(q->cfg.scs);

  return q->l_first[ssb_idx] / nof_symbols_subframe + (half_frame ? (ISRRAN_NOF_SF_X_FRAME / 2) : 0);
}

uint32_t isrran_ssb_candidate_sf_offset(const isrran_ssb_t* q, uint32_t ssb_idx)
{
  if (q == NULL) {
    return 0;
  }

  uint32_t nof_symbols_subframe = ISRRAN_NSYMB_PER_SLOT_NR * ISRRAN_NSLOTS_PER_SF_NR(q->cfg.scs);
  uint32_t l                    = q->l_first[ssb_idx] % nof_symbols_subframe;

  uint32_t cp_sz_0 = (16U * q->symbol_sz) / 2048U;

  return cp_sz_0 + l * (q->symbol_sz + q->cp_sz);
}

uint32_t isrran_ssb_cfg_to_str(const isrran_ssb_cfg_t* cfg, char* str, uint32_t str_len)
{
  uint32_t n = 0;

  n = isrran_print_check(str,
                         str_len,
                         n,
                         "srate=%.2f MHz; c-freq=%.3f MHz; ss-freq=%.3f MHz; scs=%s; pattern=%s; duplex=%s;",
                         cfg->srate_hz / 1e6,
                         cfg->center_freq_hz / 1e6,
                         cfg->ssb_freq_hz / 1e6,
                         isrran_subcarrier_spacing_to_str(cfg->scs),
                         isrran_ssb_pattern_to_str(cfg->pattern),
                         cfg->duplex_mode == ISRRAN_DUPLEX_MODE_FDD ? "fdd" : "tdd");

  if (cfg->periodicity_ms > 0) {
    n = isrran_print_check(str, str_len, n, " period=%d ms;", cfg->periodicity_ms);
  }

  return n;
}
