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

/******************************************************************************
 *  File:         sss.h
 *
 *  Description:  Secondary synchronization signal (SSS) generation and detection.
 *
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.11.2
 *****************************************************************************/

#ifndef ISRRAN_SSS_H
#define ISRRAN_SSS_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/dft.h"

#define ISRRAN_SSS_N 31
#define ISRRAN_SSS_LEN 2 * ISRRAN_SSS_N

typedef struct ISRRAN_API {
  int z1[ISRRAN_SSS_N][ISRRAN_SSS_N];
  int c[2][ISRRAN_SSS_N];
  int s[ISRRAN_SSS_N][ISRRAN_SSS_N];
} isrran_sss_tables_t;

/* Allocate 32 complex to make it multiple of 32-byte AVX instructions alignment requirement.
 * Should use isrran_vec_malloc() to make it platform agnostic.
 */
typedef struct ISRRAN_API {
  float z1[ISRRAN_SSS_N][ISRRAN_SSS_N];
  float c[2][ISRRAN_SSS_N];
  float s[ISRRAN_SSS_N][ISRRAN_SSS_N];
  float sd[ISRRAN_SSS_N][ISRRAN_SSS_N - 1];
} isrran_sss_fc_tables_t;

/* Low-level API */
typedef struct ISRRAN_API {
  isrran_dft_plan_t dftp_input;

  uint32_t fft_size;
  uint32_t max_fft_size;

  float    corr_peak_threshold;
  uint32_t symbol_sz;
  uint32_t subframe_sz;
  uint32_t N_id_2;

  uint32_t               N_id_1_table[30][30];
  isrran_sss_fc_tables_t fc_tables[3]; // one for each N_id_2

  float corr_output_m0[ISRRAN_SSS_N];
  float corr_output_m1[ISRRAN_SSS_N];

} isrran_sss_t;

/* Basic functionality */
ISRRAN_API int isrran_sss_init(isrran_sss_t* q, uint32_t fft_size);

ISRRAN_API int isrran_sss_resize(isrran_sss_t* q, uint32_t fft_size);

ISRRAN_API void isrran_sss_free(isrran_sss_t* q);

ISRRAN_API void isrran_sss_generate(float* signal0, float* signal5, uint32_t cell_id);

ISRRAN_API void isrran_sss_put_slot(float* sss, cf_t* symbol, uint32_t nof_prb, isrran_cp_t cp);

ISRRAN_API void isrran_sss_put_symbol(float* sss, cf_t* symbol, uint32_t nof_prb);

ISRRAN_API int isrran_sss_set_N_id_2(isrran_sss_t* q, uint32_t N_id_2);

ISRRAN_API int isrran_sss_m0m1_partial(isrran_sss_t* q,
                                       const cf_t*   input,
                                       uint32_t      M,
                                       cf_t          ce[2 * ISRRAN_SSS_N],
                                       uint32_t*     m0,
                                       float*        m0_value,
                                       uint32_t*     m1,
                                       float*        m1_value);

ISRRAN_API int isrran_sss_m0m1_diff_coh(isrran_sss_t* q,
                                        const cf_t*   input,
                                        cf_t          ce[2 * ISRRAN_SSS_N],
                                        uint32_t*     m0,
                                        float*        m0_value,
                                        uint32_t*     m1,
                                        float*        m1_value);

ISRRAN_API int
isrran_sss_m0m1_diff(isrran_sss_t* q, const cf_t* input, uint32_t* m0, float* m0_value, uint32_t* m1, float* m1_value);

ISRRAN_API uint32_t isrran_sss_subframe(uint32_t m0, uint32_t m1);

ISRRAN_API int isrran_sss_N_id_1(isrran_sss_t* q, uint32_t m0, uint32_t m1, float corr);

ISRRAN_API int isrran_sss_frame(isrran_sss_t* q, cf_t* input, uint32_t* subframe_idx, uint32_t* N_id_1);

ISRRAN_API void isrran_sss_set_threshold(isrran_sss_t* q, float threshold);

ISRRAN_API void isrran_sss_set_symbol_sz(isrran_sss_t* q, uint32_t symbol_sz);

ISRRAN_API void isrran_sss_set_subframe_sz(isrran_sss_t* q, uint32_t subframe_sz);

#endif // ISRRAN_SSS_H
