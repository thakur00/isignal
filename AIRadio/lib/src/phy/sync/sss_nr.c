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

#include "isrran/phy/sync/sss_nr.h"
#include "isrran/phy/utils/vector.h"

/**
 * NR SSS First Subcarrier index
 */
#define SSS_NR_SUBC_BEGIN 56

/**
 * Number of possible M1 shifts
 */
#define SSS_NR_NOF_M1 112U

/**
 * Number of possible M0 shifts
 */
#define SSS_NR_NOF_M0 ISRRAN_FLOOR(ISRRAN_NOF_NID_1_NR, SSS_NR_NOF_M1)

/**
 * Calculates Sequence circular offset M0 value
 */
#define SSS_NR_SEQUENCE_M0(N_id_1, N_id_2)                                                                             \
  ((15U * ISRRAN_FLOOR(N_id_1, SSS_NR_NOF_M1) + 5 * (N_id_2)) % ISRRAN_SSS_NR_LEN)

/**
 * Calculates Sequence circular offset M1 value
 */
#define SSS_NR_SEQUENCE_M1(N_id_1) (N_id_1 % SSS_NR_NOF_M1)

/**
 * Pregenerated modulated sequences
 */
static cf_t sss_nr_d0[ISRRAN_SSS_NR_LEN] = {};
static cf_t sss_nr_d1[ISRRAN_SSS_NR_LEN] = {};

/**
 * Sequence generation as described in TS 38.211 clause 7.4.2.2.1
 */
__attribute__((constructor)) __attribute__((unused)) static void sss_nr_pregen()
{
  // Initialise M sequence x0
  uint32_t x0[ISRRAN_SSS_NR_LEN + 7];
  x0[6] = 0;
  x0[5] = 0;
  x0[4] = 0;
  x0[3] = 0;
  x0[2] = 0;
  x0[1] = 0;
  x0[0] = 1;

  // Initialise M sequence x1
  uint32_t x1[ISRRAN_SSS_NR_LEN + 7];
  x1[6] = 0;
  x1[5] = 0;
  x1[4] = 0;
  x1[3] = 0;
  x1[2] = 0;
  x1[1] = 0;
  x1[0] = 1;

  // Generate M sequence x
  for (uint32_t i = 0; i < ISRRAN_SSS_NR_LEN; i++) {
    x0[i + 7] = (x0[i + 4] + x0[i]) % 2;
    x1[i + 7] = (x1[i + 1] + x1[i]) % 2;
  }

  // Modulate M sequence d
  for (uint32_t i = 0; i < ISRRAN_SSS_NR_LEN; i++) {
    sss_nr_d0[i] = 1.0f - 2.0f * (float)x0[i];
    sss_nr_d1[i] = 1.0f - 2.0f * (float)x1[i];
  }
}

int isrran_sss_nr_put(cf_t ssb_grid[ISRRAN_SSB_NOF_RE], uint32_t N_id_1, uint32_t N_id_2, float beta)
{
  // Verify inputs
  if (ssb_grid == NULL || N_id_1 >= ISRRAN_NOF_NID_1_NR || N_id_2 >= ISRRAN_NOF_NID_2_NR) {
    return ISRRAN_ERROR;
  }

  // Calculate generation parameters
  uint32_t m0            = SSS_NR_SEQUENCE_M0(N_id_1, N_id_2);
  uint32_t m1            = SSS_NR_SEQUENCE_M1(N_id_1);
  uint32_t grid_idx_m0_1 = ISRRAN_SSS_NR_SYMBOL_IDX * ISRRAN_SSB_BW_SUBC + SSS_NR_SUBC_BEGIN;
  uint32_t grid_idx_m0_2 = grid_idx_m0_1 + (ISRRAN_SSS_NR_LEN - m0);
  uint32_t grid_idx_m1_1 = ISRRAN_SSS_NR_SYMBOL_IDX * ISRRAN_SSB_BW_SUBC + SSS_NR_SUBC_BEGIN;
  uint32_t grid_idx_m1_2 = grid_idx_m1_1 + (ISRRAN_SSS_NR_LEN - m1);

  // Copy d0 sequence first part from m0 to the end
  isrran_vec_sc_prod_cfc(&sss_nr_d0[m0], beta, &ssb_grid[grid_idx_m0_1], ISRRAN_SSS_NR_LEN - m0);

  // Copy d0 sequence second part from 0 to m0
  isrran_vec_sc_prod_cfc(&sss_nr_d0[0], beta, &ssb_grid[grid_idx_m0_2], m0);

  // Multiply d1 sequence first part from m1 to the end
  isrran_vec_prod_ccc(&ssb_grid[grid_idx_m1_1], &sss_nr_d1[m1], &ssb_grid[grid_idx_m1_1], ISRRAN_SSS_NR_LEN - m1);

  // Multiply d1 sequence second part from 0 to m1
  isrran_vec_prod_ccc(&ssb_grid[grid_idx_m1_2], &sss_nr_d1[0], &ssb_grid[grid_idx_m1_2], m1);

  return ISRRAN_SUCCESS;
}

int isrran_sss_nr_extract_lse(const cf_t* ssb_grid, uint32_t N_id_1, uint32_t N_id_2, cf_t lse[ISRRAN_SSS_NR_LEN])
{
  // Verify inputs
  if (ssb_grid == NULL || N_id_2 >= ISRRAN_NOF_NID_2_NR || lse == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Extract SSS
  isrran_vec_cf_copy(
      lse, &ssb_grid[ISRRAN_SSS_NR_SYMBOL_IDX * ISRRAN_SSB_BW_SUBC + SSS_NR_SUBC_BEGIN], ISRRAN_SSS_NR_LEN);

  // Estimate
  uint32_t m0 = SSS_NR_SEQUENCE_M0(N_id_1, N_id_2);
  isrran_vec_prod_ccc(&sss_nr_d0[m0], lse, lse, ISRRAN_SSS_NR_LEN - m0);
  isrran_vec_prod_ccc(&sss_nr_d0[0], &lse[ISRRAN_SSS_NR_LEN - m0], &lse[ISRRAN_SSS_NR_LEN - m0], m0);

  uint32_t m1 = SSS_NR_SEQUENCE_M1(N_id_1);
  isrran_vec_prod_ccc(&sss_nr_d1[m1], lse, lse, ISRRAN_SSS_NR_LEN - m1);
  isrran_vec_prod_ccc(&sss_nr_d1[0], &lse[ISRRAN_SSS_NR_LEN - m1], &lse[ISRRAN_SSS_NR_LEN - m1], m1);

  return ISRRAN_SUCCESS;
}

int isrran_sss_nr_find(const cf_t ssb_grid[ISRRAN_SSB_NOF_RE],
                       uint32_t   N_id_2,
                       float*     norm_corr,
                       uint32_t*  found_N_id_1)
{
  // Verify inputs
  if (ssb_grid == NULL || N_id_2 >= ISRRAN_NOF_NID_2_NR || norm_corr == NULL || found_N_id_1 == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Extract SSS ptr
  const cf_t* sss_ptr = &ssb_grid[ISRRAN_SSS_NR_SYMBOL_IDX * ISRRAN_SSB_BW_SUBC + SSS_NR_SUBC_BEGIN];

  // Measure SSS average power
  float avg_power = isrran_vec_avg_power_cf(sss_ptr, ISRRAN_SSS_NR_LEN);

  // If the measured power is invalid or zero, consider no SSS signal
  if (!isnormal(avg_power)) {
    *norm_corr    = 0.0f;
    *found_N_id_1 = 0;
    return ISRRAN_SUCCESS;
  }

  // Search state
  float    max_corr = -INFINITY; //< Stores best correlation
  uint32_t N_id_1   = 0;         //< Best N_id_1

  // Iterate over all M1 shifts
  for (uint32_t m1 = 0; m1 < SSS_NR_NOF_M1; m1++) {
    // Temporal storage of SSS after applying d1 sequence
    cf_t sss_seq_m1[ISRRAN_SSS_NR_LEN];

    // Apply d1 sequence fist part
    isrran_vec_prod_ccc(&sss_ptr[0], &sss_nr_d1[m1], &sss_seq_m1[0], ISRRAN_SSS_NR_LEN - m1);

    // Apply d1 sequence second part
    isrran_vec_prod_ccc(&sss_ptr[ISRRAN_SSS_NR_LEN - m1], &sss_nr_d1[0], &sss_seq_m1[ISRRAN_SSS_NR_LEN - m1], m1);

    // Iterate over all N_id_1 with the given m1 sequence
    for (uint32_t N_id_1_blind = m1; N_id_1_blind < ISRRAN_NOF_NID_1_NR; N_id_1_blind += SSS_NR_NOF_M1) {
      uint32_t m0  = SSS_NR_SEQUENCE_M0(N_id_1_blind, N_id_2);
      cf_t     acc = 0.0f;

      // Correlate d0 sequence fist part
      acc += isrran_vec_dot_prod_ccc(&sss_seq_m1[0], &sss_nr_d0[m0], ISRRAN_SSS_NR_LEN - m0);

      // Correlate d0 sequence second part
      acc += isrran_vec_dot_prod_ccc(&sss_seq_m1[ISRRAN_SSS_NR_LEN - m0], &sss_nr_d0[0], m0);

      // Normalise
      float corr = ISRRAN_CSQABS(acc);
      if (corr > max_corr) {
        N_id_1   = N_id_1_blind;
        max_corr = corr;
      }
    }
  }

  // Copy found result
  *norm_corr    = max_corr / avg_power / ISRRAN_SSS_NR_LEN;
  *found_N_id_1 = N_id_1;

  return ISRRAN_SUCCESS;
}
