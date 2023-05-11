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
 *  File:         cfo.h
 *
 *  Description:  Carrier frequency offset correction using complex exponentials.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_CFO_H
#define ISRRAN_CFO_H

#include <complex.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/utils/cexptab.h"

#define ISRRAN_CFO_CEXPTAB_SIZE 4096

typedef struct ISRRAN_API {
  float            last_freq;
  float            tol;
  int              nsamples;
  int              max_samples;
  isrran_cexptab_t tab;
  cf_t*            cur_cexp;
} isrran_cfo_t;

ISRRAN_API int isrran_cfo_init(isrran_cfo_t* h, uint32_t nsamples);

ISRRAN_API void isrran_cfo_free(isrran_cfo_t* h);

ISRRAN_API int isrran_cfo_resize(isrran_cfo_t* h, uint32_t samples);

ISRRAN_API void isrran_cfo_set_tol(isrran_cfo_t* h, float tol);

ISRRAN_API void isrran_cfo_correct(isrran_cfo_t* h, const cf_t* input, cf_t* output, float freq);

ISRRAN_API void
isrran_cfo_correct_offset(isrran_cfo_t* h, const cf_t* input, cf_t* output, float freq, int cexp_offset, int nsamples);

ISRRAN_API float isrran_cfo_est_corr_cp(cf_t* input_buffer, uint32_t nof_prb);

#endif // ISRRAN_CFO_H
