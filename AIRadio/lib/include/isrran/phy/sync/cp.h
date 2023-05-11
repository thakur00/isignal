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

#ifndef ISRRAN_CP_H
#define ISRRAN_CP_H

#include <complex.h>
#include <stdint.h>

#include "isrran/config.h"

typedef struct {
  cf_t*    corr;
  uint32_t symbol_sz;
  uint32_t max_symbol_sz;
} isrran_cp_synch_t;

ISRRAN_API int isrran_cp_synch_init(isrran_cp_synch_t* q, uint32_t symbol_sz);

ISRRAN_API void isrran_cp_synch_free(isrran_cp_synch_t* q);

ISRRAN_API int isrran_cp_synch_resize(isrran_cp_synch_t* q, uint32_t symbol_sz);

ISRRAN_API uint32_t
isrran_cp_synch(isrran_cp_synch_t* q, const cf_t* input, uint32_t max_offset, uint32_t nof_symbols, uint32_t cp_len);

ISRRAN_API cf_t isrran_cp_synch_corr_output(isrran_cp_synch_t* q, uint32_t offset);

#endif // ISRRAN_CP_H
