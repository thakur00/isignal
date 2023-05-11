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
 *  File:         cexptab.h
 *
 *  Description:  Utility module for generation of complex exponential tables.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_CEXPTAB_H
#define ISRRAN_CEXPTAB_H

#include "isrran/config.h"
#include <complex.h>
#include <stdint.h>

typedef struct ISRRAN_API {
  uint32_t size;
  cf_t*    tab;
} isrran_cexptab_t;

ISRRAN_API int isrran_cexptab_init(isrran_cexptab_t* nco, uint32_t size);

ISRRAN_API void isrran_cexptab_free(isrran_cexptab_t* nco);

ISRRAN_API void isrran_cexptab_gen(isrran_cexptab_t* nco, cf_t* x, float freq, uint32_t len);

ISRRAN_API void isrran_cexptab_gen_direct(cf_t* x, float freq, uint32_t len);

ISRRAN_API void isrran_cexptab_gen_sf(cf_t* x, float freq, uint32_t fft_size);

#endif // ISRRAN_CEXPTAB_H
