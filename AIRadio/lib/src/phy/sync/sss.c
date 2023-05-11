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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "isrran/phy/dft/dft.h"
#include "isrran/phy/sync/sss.h"
#include "isrran/phy/utils/convolution.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

void generate_sss_all_tables(isrran_sss_tables_t* tables, uint32_t N_id_2);
void convert_tables(isrran_sss_fc_tables_t* fc_tables, isrran_sss_tables_t* in);
void generate_N_id_1_table(uint32_t table[30][30]);

int isrran_sss_init(isrran_sss_t* q, uint32_t fft_size)
{
  if (q != NULL && fft_size <= 2048) {
    uint32_t            N_id_2;
    isrran_sss_tables_t sss_tables;

    bzero(q, sizeof(isrran_sss_t));

    if (isrran_dft_plan(&q->dftp_input, fft_size, ISRRAN_DFT_FORWARD, ISRRAN_DFT_COMPLEX)) {
      isrran_sss_free(q);
      return ISRRAN_ERROR;
    }
    isrran_dft_plan_set_mirror(&q->dftp_input, true);
    isrran_dft_plan_set_dc(&q->dftp_input, true);

    q->fft_size     = fft_size;
    q->max_fft_size = fft_size;

    generate_N_id_1_table(q->N_id_1_table);

    for (N_id_2 = 0; N_id_2 < 3; N_id_2++) {
      generate_sss_all_tables(&sss_tables, N_id_2);
      convert_tables(&q->fc_tables[N_id_2], &sss_tables);
    }
    q->N_id_2 = 0;
    return ISRRAN_SUCCESS;
  }
  return ISRRAN_ERROR_INVALID_INPUTS;
}

int isrran_sss_resize(isrran_sss_t* q, uint32_t fft_size)
{
  if (q != NULL && fft_size <= 2048) {
    if (fft_size > q->max_fft_size) {
      ERROR("Error in sss_synch_resize(): fft_size must be lower than initialized");
      return ISRRAN_ERROR;
    }
    if (isrran_dft_replan(&q->dftp_input, fft_size)) {
      isrran_sss_free(q);
      return ISRRAN_ERROR;
    }
    q->fft_size = fft_size;
    return ISRRAN_SUCCESS;
  }
  return ISRRAN_ERROR_INVALID_INPUTS;
}

void isrran_sss_free(isrran_sss_t* q)
{
  isrran_dft_plan_free(&q->dftp_input);
  bzero(q, sizeof(isrran_sss_t));
}

/** Sets the N_id_2 to search for */
int isrran_sss_set_N_id_2(isrran_sss_t* q, uint32_t N_id_2)
{
  if (!isrran_N_id_2_isvalid(N_id_2)) {
    ERROR("Invalid N_id_2 %d", N_id_2);
    return ISRRAN_ERROR;
  } else {
    q->N_id_2 = N_id_2;
    return ISRRAN_SUCCESS;
  }
}

/** 36.211 10.3 section 6.11.2.2
 */
void isrran_sss_put_slot(float* sss, cf_t* slot, uint32_t nof_prb, isrran_cp_t cp)
{
  uint32_t i, k;

  k = (ISRRAN_CP_NSYMB(cp) - 2) * nof_prb * ISRRAN_NRE + nof_prb * ISRRAN_NRE / 2 - 31;

  if (k > 5) {
    memset(&slot[k - 5], 0, 5 * sizeof(cf_t));
    for (i = 0; i < ISRRAN_SSS_LEN; i++) {
      __real__ slot[k + i] = sss[i];
      __imag__ slot[k + i] = 0;
    }
    memset(&slot[k + ISRRAN_SSS_LEN], 0, 5 * sizeof(cf_t));
  }
}

/** Sets the SSS correlation peak detection threshold */
void isrran_sss_set_threshold(isrran_sss_t* q, float threshold)
{
  q->corr_peak_threshold = threshold;
}

/** Returns the subframe index based on the m0 and m1 values */
uint32_t isrran_sss_subframe(uint32_t m0, uint32_t m1)
{
  if (m1 > m0) {
    return 0;
  } else {
    return 5;
  }
}

/** Returns the N_id_1 value based on the m0 and m1 values */
int isrran_sss_N_id_1(isrran_sss_t* q, uint32_t m0, uint32_t m1, float corr)
{
  int N_id_1 = ISRRAN_ERROR;

  // Check threshold, consider not found (error) if the correlation is not above the threshold
  if (corr > q->corr_peak_threshold) {
    if (m1 > m0) {
      if (m0 < 30 && m1 - 1 < 30) {
        N_id_1 = q->N_id_1_table[m0][m1 - 1];
      }
    } else {
      if (m1 < 30 && m0 - 1 < 30) {
        N_id_1 = q->N_id_1_table[m1][m0 - 1];
      }
    }
  }

  return N_id_1;
}
