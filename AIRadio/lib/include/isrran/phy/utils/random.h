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

#ifndef ISRRAN_RANDOM_H
#define ISRRAN_RANDOM_H

#include "isrran/config.h"

#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* isrran_random_t;

ISRRAN_API isrran_random_t isrran_random_init(uint32_t seed);

ISRRAN_API int isrran_random_uniform_int_dist(isrran_random_t q, int min, int max);

ISRRAN_API float isrran_random_uniform_real_dist(isrran_random_t q, float min, float max);

ISRRAN_API cf_t isrran_random_uniform_complex_dist(isrran_random_t q, float min, float max);

ISRRAN_API void
isrran_random_uniform_complex_dist_vector(isrran_random_t q, cf_t* vector, uint32_t nsamples, float min, float max);

ISRRAN_API float isrran_random_gauss_dist(isrran_random_t q, float std_dev);

ISRRAN_API bool isrran_random_bool(isrran_random_t q, float prob_true);

ISRRAN_API void isrran_random_byte_vector(isrran_random_t q, uint8_t* c, uint32_t nsamples);

ISRRAN_API void isrran_random_bit_vector(isrran_random_t q, uint8_t* c, uint32_t nsamples);

ISRRAN_API void isrran_random_free(isrran_random_t q);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_RANDOM_H
