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

#include "isrran/phy/sync/pss_nr.h"
#include "isrran/phy/utils/vector.h"

/**
 * NR PSS First Subcarrier index
 */
#define PSS_NR_SUBC_BEGIN 56

/**
 * Calculates Sequence circular offset
 */
#define PSS_NR_SEQUENCE_M(N_id_2) ((43U * (N_id_2)) % ISRRAN_PSS_NR_LEN)

/**
 * Pregenerated modulated sequence
 */
static cf_t pss_nr_d[ISRRAN_PSS_NR_LEN] = {};

/**
 * Sequence generation as described in TS 38.211 clause 7.4.2.2.1
 */
__attribute__((constructor)) __attribute__((unused)) static void pss_nr_pregen()
{
  // Initialise M sequence x
  uint32_t x[ISRRAN_PSS_NR_LEN + 7];
  x[6] = 1;
  x[5] = 1;
  x[4] = 1;
  x[3] = 0;
  x[2] = 1;
  x[1] = 1;
  x[0] = 0;

  // Generate M sequence x
  for (uint32_t i = 0; i < ISRRAN_PSS_NR_LEN; i++) {
    x[i + 7] = (x[i + 4] + x[i]) % 2;
  }

  // Modulate M sequence d
  for (uint32_t i = 0; i < ISRRAN_PSS_NR_LEN; i++) {
    pss_nr_d[i] = 1.0f - 2.0f * (float)x[i];
  }
}

int isrran_pss_nr_put(cf_t ssb_grid[ISRRAN_SSB_NOF_RE], uint32_t N_id_2, float beta)
{
  // Verify inputs
  if (ssb_grid == NULL || N_id_2 >= ISRRAN_NOF_NID_2_NR) {
    return ISRRAN_ERROR;
  }

  // Calculate generation parameters
  uint32_t m          = PSS_NR_SEQUENCE_M(N_id_2);
  uint32_t copy_sz_1  = ISRRAN_PSS_NR_LEN - m;
  uint32_t grid_idx_1 = ISRRAN_PSS_NR_SYMBOL_IDX * ISRRAN_SSB_BW_SUBC + PSS_NR_SUBC_BEGIN;
  uint32_t grid_idx_2 = grid_idx_1 + copy_sz_1;

  // Copy sequence from offset to the end
  isrran_vec_sc_prod_cfc(&pss_nr_d[m], beta, &ssb_grid[grid_idx_1], copy_sz_1);

  // Copy sequence from 0 to offset
  isrran_vec_sc_prod_cfc(&pss_nr_d[0], beta, &ssb_grid[grid_idx_2], m);

  return ISRRAN_SUCCESS;
}

int isrran_pss_nr_extract_lse(const cf_t* ssb_grid, uint32_t N_id_2, cf_t lse[ISRRAN_PSS_NR_LEN])
{
  // Verify inputs
  if (ssb_grid == NULL || N_id_2 >= ISRRAN_NOF_NID_2_NR || lse == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Extract PSS
  isrran_vec_cf_copy(
      lse, &ssb_grid[ISRRAN_PSS_NR_SYMBOL_IDX * ISRRAN_SSB_BW_SUBC + PSS_NR_SUBC_BEGIN], ISRRAN_PSS_NR_LEN);

  // Estimate
  uint32_t m = PSS_NR_SEQUENCE_M(N_id_2);
  isrran_vec_prod_ccc(&pss_nr_d[m], lse, lse, ISRRAN_PSS_NR_LEN - m);
  isrran_vec_prod_ccc(&pss_nr_d[0], &lse[ISRRAN_PSS_NR_LEN - m], &lse[ISRRAN_PSS_NR_LEN - m], m);

  return ISRRAN_SUCCESS;
}

int isrran_pss_nr_find(const cf_t ssb_grid[ISRRAN_SSB_NOF_RE], float* norm_corr, uint32_t* found_N_id_2)
{
  // Verify inputs
  if (ssb_grid == NULL || norm_corr == NULL || found_N_id_2 == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  const cf_t* pss_ptr = &ssb_grid[ISRRAN_PSS_NR_SYMBOL_IDX * ISRRAN_SSB_BW_SUBC + PSS_NR_SUBC_BEGIN];

  // Measure PSS region average power
  float avg_power = isrran_vec_avg_power_cf(pss_ptr, ISRRAN_PSS_NR_LEN);

  // If no energy detected or invalid, consider zero correlation
  if (!isnormal(avg_power)) {
    *norm_corr    = 0.0f;
    *found_N_id_2 = 0;
    return ISRRAN_SUCCESS;
  }

  // Search state
  float    max_corr = -INFINITY; //< Stores best correlation
  uint32_t N_id_2   = 0;         //< Best N_id_2

  // Iterate over all possible N_id_2
  for (uint32_t N_id_2_candidate = 0; N_id_2_candidate < ISRRAN_NOF_NID_2_NR; N_id_2_candidate++) {
    uint32_t m   = PSS_NR_SEQUENCE_M(N_id_2_candidate);
    cf_t     acc = 0.0f;

    // Correlate d sequence fist part
    acc += isrran_vec_dot_prod_ccc(&pss_nr_d[m], &pss_ptr[0], ISRRAN_PSS_NR_LEN - m);

    // Correlate d sequence second part
    acc += isrran_vec_dot_prod_ccc(&pss_nr_d[0], &pss_ptr[ISRRAN_PSS_NR_LEN - m], m);

    // Correlate
    float corr = ISRRAN_CSQABS(acc);
    if (corr > max_corr) {
      N_id_2   = N_id_2_candidate;
      max_corr = corr;
    }
  }

  // Copy found result
  *norm_corr    = max_corr / avg_power / ISRRAN_PSS_NR_LEN;
  *found_N_id_2 = N_id_2;

  return ISRRAN_SUCCESS;
}
