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
 *  File:         psss.h
 *
 *  Description:  Primary sidelink synchronization signal (PSSS) generation and detection.
 *
 *
 *  Reference:    3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.7.1
 *****************************************************************************/

#ifndef ISRRAN_PSSS_H
#define ISRRAN_PSSS_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/dft.h"

#define ISRRAN_PSSS_LEN 62

typedef struct ISRRAN_API {

  cf_t psss_signal[2][ISRRAN_PSSS_LEN]; // One sequence for each N_id_2

  cf_t** psss_sf_freq;

  cf_t* input_pad_freq;
  cf_t* input_pad_time;

  cf_t* dot_prod_output;
  cf_t* dot_prod_output_time;

  cf_t* shifted_output;

  float* shifted_output_abs;

  int32_t corr_peak_pos;
  float   corr_peak_value;

  uint32_t N_id_2;

  isrran_dft_plan_t plan_input;
  isrran_dft_plan_t plan_out;

} isrran_psss_t;

ISRRAN_API int isrran_psss_init(isrran_psss_t* q, uint32_t nof_prb, isrran_cp_t cp);

ISRRAN_API int isrran_psss_generate(cf_t* psss_signal, uint32_t N_id_2);

ISRRAN_API void isrran_psss_put_sf_buffer(cf_t* psss_signal, cf_t* sf_buffer, uint32_t nof_prb, isrran_cp_t cp);

ISRRAN_API int isrran_psss_find(isrran_psss_t* q, cf_t* input, uint32_t nof_prb, isrran_cp_t cp);

ISRRAN_API void isrran_psss_free(isrran_psss_t* q);

#endif // ISRRAN_PSSS_H
