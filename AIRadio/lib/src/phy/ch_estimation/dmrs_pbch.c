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

#include "isrran/phy/ch_estimation/dmrs_pbch.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <complex.h>
#include <math.h>

/*
 * Number of NR PBCH DMRS resource elements present in an SSB resource grid
 */
#define DMRS_PBCH_NOF_RE 144

static uint32_t dmrs_pbch_cinit(const isrran_dmrs_pbch_cfg_t* cfg)
{
  // Default values for L_max == 4
  uint64_t i_ssb = (cfg->ssb_idx & 0b11U) + 4UL * cfg->n_hf; // Least 2 significant bits

  if (cfg->L_max == 8 || cfg->L_max == 64) {
    i_ssb = cfg->ssb_idx & 0b111U; // Least 3 significant bits
  }

  return ISRRAN_SEQUENCE_MOD(((i_ssb + 1UL) * (ISRRAN_FLOOR(cfg->N_id, 4UL) + 1UL) << 11UL) + ((i_ssb + 1UL) << 6UL) +
                             (cfg->N_id % 4));
}

int isrran_dmrs_pbch_put(const isrran_dmrs_pbch_cfg_t* cfg, cf_t ssb_grid[ISRRAN_SSB_NOF_RE])
{
  // Validate inputs
  if (cfg == NULL || ssb_grid == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Calculate index shift
  uint32_t v = cfg->N_id % 4;

  // Calculate power allocation
  float beta = M_SQRT1_2;
  if (isnormal(cfg->beta)) {
    beta = cfg->beta;
  }

  // Initialise sequence
  uint32_t                cinit          = dmrs_pbch_cinit(cfg);
  isrran_sequence_state_t sequence_state = {};
  isrran_sequence_state_init(&sequence_state, cinit);

  // Generate sequence
  cf_t r[DMRS_PBCH_NOF_RE];
  isrran_sequence_state_gen_f(&sequence_state, beta, (float*)r, DMRS_PBCH_NOF_RE * 2);

  // r sequence read index
  uint32_t r_idx = 0;

  // Put sequence in symbol 1
  for (uint32_t k = v; k < ISRRAN_SSB_BW_SUBC; k += 4) {
    ssb_grid[ISRRAN_SSB_BW_SUBC * 1 + k] = r[r_idx++];
  }

  // Put sequence in symbol 2, lower section
  for (uint32_t k = v; k < 48; k += 4) {
    ssb_grid[ISRRAN_SSB_BW_SUBC * 2 + k] = r[r_idx++];
  }

  // Put sequence in symbol 2, upper section
  for (uint32_t k = 192 + v; k < ISRRAN_SSB_BW_SUBC; k += 4) {
    ssb_grid[ISRRAN_SSB_BW_SUBC * 2 + k] = r[r_idx++];
  }

  // Put sequence in symbol 3
  for (uint32_t k = v; k < ISRRAN_SSB_BW_SUBC; k += 4) {
    ssb_grid[ISRRAN_SSB_BW_SUBC * 3 + k] = r[r_idx++];
  }

  return ISRRAN_SUCCESS;
}

int dmrs_pbch_extract_lse(const isrran_dmrs_pbch_cfg_t* cfg,
                          const cf_t                    ssb_grid[ISRRAN_SSB_NOF_RE],
                          cf_t                          lse[DMRS_PBCH_NOF_RE])
{
  // Calculate index shift
  uint32_t v = cfg->N_id % 4;

  // Calculate power allocation
  float beta = M_SQRT1_2;
  if (isnormal(cfg->beta)) {
    beta = cfg->beta;
  }

  // Initialise sequence
  uint32_t                cinit          = dmrs_pbch_cinit(cfg);
  isrran_sequence_state_t sequence_state = {};
  isrran_sequence_state_init(&sequence_state, cinit);

  // Generate sequence
  cf_t r[DMRS_PBCH_NOF_RE];
  isrran_sequence_state_gen_f(&sequence_state, beta, (float*)r, DMRS_PBCH_NOF_RE * 2);

  // r sequence read index
  uint32_t r_idx = 0;

  // Put sequence in symbol 1
  for (uint32_t k = v; k < ISRRAN_SSB_BW_SUBC; k += 4) {
    lse[r_idx++] = ssb_grid[ISRRAN_SSB_BW_SUBC * 1 + k];
  }

  // Put sequence in symbol 2, lower section
  for (uint32_t k = v; k < 48; k += 4) {
    lse[r_idx++] = ssb_grid[ISRRAN_SSB_BW_SUBC * 2 + k];
  }

  // Put sequence in symbol 2, upper section
  for (uint32_t k = 192 + v; k < ISRRAN_SSB_BW_SUBC; k += 4) {
    lse[r_idx++] = ssb_grid[ISRRAN_SSB_BW_SUBC * 2 + k];
  }

  // Put sequence in symbol 3
  for (uint32_t k = v; k < ISRRAN_SSB_BW_SUBC; k += 4) {
    lse[r_idx++] = ssb_grid[ISRRAN_SSB_BW_SUBC * 3 + k];
  }

  // Calculate actual least square estimates
  isrran_vec_prod_conj_ccc(lse, r, lse, DMRS_PBCH_NOF_RE);

  return ISRRAN_SUCCESS;
}

static int dmrs_pbch_meas_estimate(const isrran_dmrs_pbch_cfg_t* cfg,
                                   const cf_t                    ssb_grid[ISRRAN_SSB_NOF_RE],
                                   cf_t                          ce[ISRRAN_SSB_NOF_RE],
                                   isrran_dmrs_pbch_meas_t*      meas)
{
  // Extract least square estimates
  cf_t lse[DMRS_PBCH_NOF_RE];
  if (dmrs_pbch_extract_lse(cfg, ssb_grid, lse) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  float scs_hz = ISRRAN_SUBC_SPACING_NR(cfg->scs);
  if (!isnormal(scs_hz)) {
    ERROR("Invalid SCS");
    return ISRRAN_ERROR;
  }

  // Compute average delay in microseconds from the symbols 1 and 3 (symbol 2 does not carry PBCH in all the grid)
  float avg_delay1_norm = isrran_vec_estimate_frequency(&lse[0], 60) / 4.0f;
  float avg_delay3_norm = isrran_vec_estimate_frequency(&lse[84], 60) / 4.0f;
  float avg_delay_norm  = (avg_delay1_norm + avg_delay3_norm) / 2.0f;
  float avg_delay_us    = avg_delay_norm / scs_hz;

  // Generate a second SSB grid with the corrected average delay
  cf_t ssb_grid_corrected[ISRRAN_SSB_NOF_RE];
  for (uint32_t l = 0; l < ISRRAN_SSB_DURATION_NSYMB; l++) {
    isrran_vec_apply_cfo(&ssb_grid[ISRRAN_SSB_BW_SUBC * l],
                         avg_delay_norm,
                         &ssb_grid_corrected[ISRRAN_SSB_BW_SUBC * l],
                         ISRRAN_SSB_BW_SUBC);
  }

  // Extract LSE from corrected grid
  if (dmrs_pbch_extract_lse(cfg, ssb_grid_corrected, lse) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Compute correlation of symbols 1 and 3
  cf_t corr1 = isrran_vec_acc_cc(&lse[0], 60) / 60.0f;
  cf_t corr3 = isrran_vec_acc_cc(&lse[84], 60) / 60.0f;

  // Estimate CFO from correlation
  float distance_s = isrran_symbol_distance_s(1, 3, cfg->scs);
  float cfo_hz     = 0.0f;
  if (isnormal(distance_s)) {
    cfo_hz = cargf(corr1 * conjf(corr3)) / (2.0f * (float)M_PI * distance_s);
  }

  // Estimate wideband gain at each symbol carrying DMRS
  cf_t wideband_gain_1 =
      isrran_vec_acc_cc(&lse[0], 60) * cexpf(I * 2.0f * M_PI * isrran_symbol_offset_s(1, cfg->scs) * cfo_hz);
  cf_t wideband_gain_2 =
      isrran_vec_acc_cc(&lse[60], 24) * cexpf(I * 2.0f * M_PI * isrran_symbol_offset_s(2, cfg->scs) * cfo_hz);
  cf_t wideband_gain_3 =
      isrran_vec_acc_cc(&lse[84], 60) * cexpf(I * 2.0f * M_PI * isrran_symbol_offset_s(3, cfg->scs) * cfo_hz);

  // Estimate wideband gain equivalent at symbol 0
  cf_t wideband_gain = (wideband_gain_1 + wideband_gain_2 + wideband_gain_3) / DMRS_PBCH_NOF_RE;

  // Compute RSRP from correlation
  float rsrp = (ISRRAN_CSQABS(corr1) + ISRRAN_CSQABS(corr3)) / 2.0f;

  // Compute EPRE
  float epre = isrran_vec_avg_power_cf(lse, DMRS_PBCH_NOF_RE);

  // Write measurements
  if (meas != NULL) {
    meas->corr         = rsrp / epre;
    meas->epre         = epre;
    meas->rsrp         = rsrp;
    meas->cfo_hz       = cfo_hz;
    meas->avg_delay_us = avg_delay_us;
  }

  // Generate estimated grid
  if (ce != NULL) {
    // Compute channel estimates
    for (uint32_t l = 0; l < ISRRAN_SSB_DURATION_NSYMB; l++) {
      float t_s                  = isrran_symbol_offset_s(l, cfg->scs);
      cf_t  symbol_wideband_gain = cexpf(-I * 2.0f * M_PI * cfo_hz * t_s) * wideband_gain;
      isrran_vec_gen_sine(symbol_wideband_gain, -avg_delay_norm, &ce[l * ISRRAN_SSB_BW_SUBC], ISRRAN_SSB_BW_SUBC);
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_dmrs_pbch_measure(const isrran_dmrs_pbch_cfg_t* cfg,
                             const cf_t                    ssb_grid[ISRRAN_SSB_NOF_RE],
                             isrran_dmrs_pbch_meas_t*      meas)
{
  // Validate inputs
  if (cfg == NULL || ssb_grid == NULL || meas == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  return dmrs_pbch_meas_estimate(cfg, ssb_grid, NULL, meas);
}

int isrran_dmrs_pbch_estimate(const isrran_dmrs_pbch_cfg_t* cfg,
                              const cf_t                    ssb_grid[ISRRAN_SSB_NOF_RE],
                              cf_t                          ce[ISRRAN_SSB_NOF_RE])
{
  // Validate inputs
  if (cfg == NULL || ssb_grid == NULL || ce == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  return dmrs_pbch_meas_estimate(cfg, ssb_grid, ce, NULL);
}