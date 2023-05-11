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

#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/utils/cexptab.h"
#include "isrran/phy/utils/vector.h"

int isrran_cexptab_init(isrran_cexptab_t* h, uint32_t size)
{
  uint32_t i;

  h->size = size;
  h->tab  = isrran_vec_cf_malloc((1 + size));
  if (h->tab) {
    for (i = 0; i < size; i++) {
      h->tab[i] = cexpf(_Complex_I * 2 * M_PI * (float)i / size);
    }
    return ISRRAN_SUCCESS;
  } else {
    return ISRRAN_ERROR;
  }
}

void isrran_cexptab_free(isrran_cexptab_t* h)
{
  if (h->tab) {
    free(h->tab);
  }
  bzero(h, sizeof(isrran_cexptab_t));
}

void isrran_cexptab_gen(isrran_cexptab_t* h, cf_t* x, float freq, uint32_t len)
{
  uint32_t i;
  uint32_t idx;
  float    phase_inc = freq * h->size;
  float    phase     = 0;

  for (i = 0; i < len; i++) {
    while (phase >= (float)h->size) {
      phase -= (float)h->size;
    }
    while (phase < 0) {
      phase += (float)h->size;
    }
    idx  = (uint32_t)phase;
    x[i] = h->tab[idx];
    phase += phase_inc;
  }
}

void isrran_cexptab_gen_direct(cf_t* x, float freq, uint32_t len)
{
  uint32_t i;
  for (i = 0; i < len; i++) {
    x[i] = cexpf(_Complex_I * 2 * M_PI * freq * i);
  }
}

void isrran_cexptab_gen_sf(cf_t* x, float freq, uint32_t fft_size)
{
  cf_t* ptr = x;
  for (uint32_t n = 0; n < 2; n++) {
    for (uint32_t i = 0; i < 7; i++) {
      uint32_t cplen = ISRRAN_CP_LEN_NORM(i, fft_size);
      for (uint32_t t = 0; t < fft_size + cplen; t++) {
        ptr[t] = cexpf(I * 2 * M_PI * ((float)t - (float)cplen) * freq / fft_size);
      }
      ptr += fft_size + cplen;
    }
  }
}
