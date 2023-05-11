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
 *  File:         debug.h
 *
 *  Description:  Debug output utilities.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_FILTER_H
#define ISRRAN_FILTER_H

#include "isrran/config.h"
#include "isrran/phy/utils/vector.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct ISRRAN_API {
  cf_t*  filter_input;
  cf_t*  downsampled_input;
  cf_t*  filter_output;
  bool   is_decimator;
  int    factor;
  int    num_taps;
  float* taps;

} isrran_filt_cc_t;

void isrran_filt_decim_cc_init(isrran_filt_cc_t* q, int factor, int order);

void isrran_filt_decim_cc_free(isrran_filt_cc_t* q);

void isrran_filt_decim_cc_execute(isrran_filt_cc_t* q, cf_t* input, cf_t* downsampled_input, cf_t* output, int size);

void isrran_downsample_cc(cf_t* input, cf_t* output, int M, int size);
#endif // ISRRAN_FILTER_H