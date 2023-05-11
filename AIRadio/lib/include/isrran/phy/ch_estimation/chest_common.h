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

#ifndef ISRRAN_CHEST_COMMON_H
#define ISRRAN_CHEST_COMMON_H

#include "isrran/config.h"
#include <stdint.h>

#define ISRRAN_CHEST_MAX_SMOOTH_FIL_LEN 64

typedef enum ISRRAN_API {
  ISRRAN_CHEST_FILTER_GAUSS = 0,
  ISRRAN_CHEST_FILTER_TRIANGLE,
  ISRRAN_CHEST_FILTER_NONE
} isrran_chest_filter_t;

ISRRAN_API void isrran_chest_average_pilots(cf_t*    input,
                                            cf_t*    output,
                                            float*   filter,
                                            uint32_t nof_ref,
                                            uint32_t nof_symbols,
                                            uint32_t filter_len);

ISRRAN_API uint32_t isrran_chest_set_smooth_filter3_coeff(float* smooth_filter, float w);

ISRRAN_API float isrran_chest_estimate_noise_pilots(cf_t* noisy, cf_t* noiseless, cf_t* noise_vec, uint32_t nof_pilots);

ISRRAN_API uint32_t isrran_chest_set_triangle_filter(float* fil, int filter_len);

ISRRAN_API uint32_t isrran_chest_set_smooth_filter_gauss(float* filter, uint32_t order, float std_dev);

#endif // ISRRAN_CHEST_COMMON_H
