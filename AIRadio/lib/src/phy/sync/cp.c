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

#include "isrran/isrran.h"
#include <stdlib.h>

#include "isrran/phy/sync/cp.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

int isrran_cp_synch_init(isrran_cp_synch_t* q, uint32_t symbol_sz)
{
  q->symbol_sz     = symbol_sz;
  q->max_symbol_sz = symbol_sz;

  q->corr = isrran_vec_cf_malloc(q->symbol_sz);
  if (!q->corr) {
    perror("malloc");
    return ISRRAN_ERROR;
  }
  return ISRRAN_SUCCESS;
}

void isrran_cp_synch_free(isrran_cp_synch_t* q)
{
  if (q->corr) {
    free(q->corr);
  }
}

int isrran_cp_synch_resize(isrran_cp_synch_t* q, uint32_t symbol_sz)
{
  if (symbol_sz > q->max_symbol_sz) {
    ERROR("Error in cp_synch_resize(): symbol_sz must be lower than initialized");
    return ISRRAN_ERROR;
  }
  q->symbol_sz = symbol_sz;

  return ISRRAN_SUCCESS;
}

uint32_t
isrran_cp_synch(isrran_cp_synch_t* q, const cf_t* input, uint32_t max_offset, uint32_t nof_symbols, uint32_t cp_len)
{
  if (max_offset > q->symbol_sz) {
    max_offset = q->symbol_sz;
  }
  for (int i = 0; i < max_offset; i++) {
    q->corr[i]           = 0;
    const cf_t* inputPtr = input;
    for (int n = 0; n < nof_symbols; n++) {
      uint32_t cplen = (n % 7) ? cp_len : cp_len + 1;
      q->corr[i] += isrran_vec_dot_prod_conj_ccc(&inputPtr[i], &inputPtr[i + q->symbol_sz], cplen) / nof_symbols;
      inputPtr += q->symbol_sz + cplen;
    }
  }
  uint32_t max_idx = isrran_vec_max_abs_ci(q->corr, max_offset);
  return max_idx;
}

cf_t isrran_cp_synch_corr_output(isrran_cp_synch_t* q, uint32_t offset)
{
  if (offset < q->symbol_sz) {
    return q->corr[offset];
  } else {
    return 0;
  }
}
