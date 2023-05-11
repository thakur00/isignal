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
 *  File:         resample_arb.h
 *
 *  Description:  Arbitrary rate resampler using a polyphase filter bank
 *                implementation.
 *
 *  Reference:    Multirate Signal Processing for Communication Systems
 *                fredric j. harris
 *****************************************************************************/

#ifndef ISRRAN_RESAMPLE_ARB_H
#define ISRRAN_RESAMPLE_ARB_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"

#define ISRRAN_RESAMPLE_ARB_N_35 35
#define ISRRAN_RESAMPLE_ARB_N 32 // Polyphase filter rows
#define ISRRAN_RESAMPLE_ARB_M 8  // Polyphase filter columns

typedef struct ISRRAN_API {
  float rate; // Resample rate
  float step; // Step increment through filter
  float acc;  // Index into filter
  bool  interpolate;
  cf_t  reg[ISRRAN_RESAMPLE_ARB_M]; // Our window of samples

} isrran_resample_arb_t;

ISRRAN_API void isrran_resample_arb_init(isrran_resample_arb_t* q, float rate, bool interpolate);

ISRRAN_API int isrran_resample_arb_compute(isrran_resample_arb_t* q, cf_t* input, cf_t* output, int n_in);

#endif // ISRRAN_RESAMPLE_ARB_
