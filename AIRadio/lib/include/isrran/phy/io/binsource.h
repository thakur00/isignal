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
 *  File:         binsource.h
 *
 *  Description:  Pseudo-random binary source.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_BINSOURCE_H
#define ISRRAN_BINSOURCE_H

#include "isrran/config.h"
#include <stdint.h>

/* Low-level API */
typedef struct ISRRAN_API {
  uint32_t  seed;
  uint32_t* seq_buff;
  int       seq_buff_nwords;
  int       seq_cache_nbits;
  int       seq_cache_rp;
} isrran_binsource_t;

ISRRAN_API void isrran_binsource_init(isrran_binsource_t* q);

ISRRAN_API void isrran_binsource_free(isrran_binsource_t* q);

ISRRAN_API void isrran_binsource_seed_set(isrran_binsource_t* q, uint32_t seed);

ISRRAN_API void isrran_binsource_seed_time(isrran_binsource_t* q);

ISRRAN_API int isrran_binsource_cache_gen(isrran_binsource_t* q, int nbits);

ISRRAN_API void isrran_binsource_cache_cpy(isrran_binsource_t* q, uint8_t* bits, int nbits);

ISRRAN_API int isrran_binsource_generate(isrran_binsource_t* q, uint8_t* bits, int nbits);

#endif // ISRRAN_BINSOURCE_H
