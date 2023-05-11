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
 *  File:         ssss.h
 *
 *  Description:  Secondary sidelink synchronization signal (SSSS) generation and detection.
 *
 *
 *  Reference:    3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.7.2
 *****************************************************************************/

#ifndef ISRRAN_SSSS_H
#define ISRRAN_SSSS_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/phy/common/phy_common_sl.h"
#include "isrran/phy/dft/dft.h"

#define ISRRAN_SSSS_NOF_SEQ 336
#define ISRRAN_SSSS_N 31
#define ISRRAN_SSSS_LEN 2 * ISRRAN_SSSS_N

typedef struct ISRRAN_API {

  float ssss_signal[ISRRAN_SSSS_NOF_SEQ][ISRRAN_SSSS_LEN]; // One sequence for each N_sl_id

  cf_t** ssss_sf_freq;

  cf_t* input_pad_freq;
  cf_t* input_pad_time;

  cf_t* dot_prod_output;
  cf_t* dot_prod_output_time;

  cf_t* shifted_output;

  float* shifted_output_abs;

  int32_t corr_peak_pos;
  float   corr_peak_value;

  uint32_t N_sl_id;

  isrran_dft_plan_t plan_input;
  isrran_dft_plan_t plan_out;

} isrran_ssss_t;

ISRRAN_API int isrran_ssss_init(isrran_ssss_t* q, uint32_t nof_prb, isrran_cp_t cp, isrran_sl_tm_t tm);

ISRRAN_API void isrran_ssss_generate(float* ssss_signal, uint32_t N_sl_id, isrran_sl_tm_t tm);

ISRRAN_API void isrran_ssss_put_sf_buffer(float* ssss_signal, cf_t* sf_buffer, uint32_t nof_prb, isrran_cp_t cp);

ISRRAN_API int isrran_ssss_find(isrran_ssss_t* q, cf_t* input, uint32_t nof_prb, uint32_t N_id_2, isrran_cp_t cp);

ISRRAN_API void isrran_ssss_free(isrran_ssss_t* q);

#endif