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
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <strings.h>

#include "isrran/phy/resampling/interp.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

/*************** STATIC FUNCTIONS ***********************/

cf_t isrran_interp_linear_onesample(cf_t input0, cf_t input1)
{
  return 2 * input1 - input0;
}

cf_t isrran_interp_linear_onesample_cabs(cf_t input0, cf_t input1)
{
  float re0 = 0, im0 = 0, re1 = 0, im1 = 0, re = 0, im = 0;
  re0 = crealf(input0);
  im0 = cimagf(input1);
  re1 = crealf(input0);
  im1 = cimagf(input1);
  re  = 2 * re1 - re0;
  im  = 2 * im1 - im0;
  return (re + im * _Complex_I);
}

/* Performs 1st order integer linear interpolation */
void isrran_interp_linear_f(float* input, float* output, uint32_t M, uint32_t len)
{
  uint32_t i, j;
  for (i = 0; i < len - 1; i++) {
    for (j = 0; j < M; j++) {
      output[i * M + j] = input[i] + j * (input[i + 1] - input[i]) / M;
    }
  }
}

/* Performs 1st order linear interpolation with out-of-bound interpolation */
void isrran_interp_linear_offset_cabs(cf_t*    input,
                                      cf_t*    output,
                                      uint32_t M,
                                      uint32_t len,
                                      uint32_t off_st,
                                      uint32_t off_end)
{
  uint32_t i, j;
  float    mag0 = 0, mag1 = 0, arg0 = 0, arg1 = 0, mag = 0, arg = 0;

  for (i = 0; i < len - 1; i++) {
    mag0 = cabsf(input[i]);
    mag1 = cabsf(input[i + 1]);
    arg0 = cargf(input[i]);
    arg1 = cargf(input[i + 1]);
    if (i == 0) {
      for (j = 0; j < off_st; j++) {
        mag       = mag0 - (j + 1) * (mag1 - mag0) / M;
        arg       = arg0 - (j + 1) * (arg1 - arg0) / M;
        output[j] = mag * cexpf(I * arg);
      }
    }
    for (j = 0; j < M; j++) {
      mag                        = mag0 + j * (mag1 - mag0) / M;
      arg                        = arg0 + j * (arg1 - arg0) / M;
      output[i * M + j + off_st] = mag * cexpf(I * arg);
    }
  }
  if (len > 1) {
    for (j = 0; j < off_end; j++) {
      mag                        = mag1 + j * (mag1 - mag0) / M;
      arg                        = arg1 + j * (arg1 - arg0) / M;
      output[i * M + j + off_st] = mag * cexpf(I * arg);
    }
  }
}

int isrran_interp_linear_vector_init(isrran_interp_linisrran_vec_t* q, uint32_t vector_len)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q) {
    bzero(q, sizeof(isrran_interp_linisrran_vec_t));
    ret         = ISRRAN_SUCCESS;
    q->diff_vec = isrran_vec_cf_malloc(vector_len);
    if (!q->diff_vec) {
      perror("malloc");
      return ISRRAN_ERROR;
    }
    q->vector_len     = vector_len;
    q->max_vector_len = vector_len;
  }
  return ret;
}

int isrran_interp_linear_vector_resize(isrran_interp_linisrran_vec_t* q, uint32_t vector_len)
{
  if (vector_len <= q->max_vector_len) {
    q->vector_len = vector_len;
    return ISRRAN_SUCCESS;
  } else {
    ERROR("Error resizing interp_linear: vector_len must be lower or equal than initialized");
    return ISRRAN_ERROR;
  }
}

void isrran_interp_linear_vector_free(isrran_interp_linisrran_vec_t* q)
{
  if (q->diff_vec) {
    free(q->diff_vec);
  }

  bzero(q, sizeof(isrran_interp_linisrran_vec_t));
}

void isrran_interp_linear_vector(isrran_interp_linisrran_vec_t* q,
                                 cf_t*                          in0,
                                 cf_t*                          in1,
                                 cf_t*                          between,
                                 uint32_t                       in1_in0_d,
                                 uint32_t                       M)
{
  isrran_interp_linear_vector2(q, in0, in1, NULL, between, in1_in0_d, M);
}

void isrran_interp_linear_vector2(isrran_interp_linisrran_vec_t* q,
                                  cf_t*                          in0,
                                  cf_t*                          in1,
                                  cf_t*                          start,
                                  cf_t*                          between,
                                  uint32_t                       in1_in0_d,
                                  uint32_t                       M)
{
  isrran_interp_linear_vector3(q, in0, in1, start, between, in1_in0_d, M, true, q->vector_len);
}

void isrran_interp_linear_vector3(isrran_interp_linisrran_vec_t* q,
                                  cf_t*                          in0,
                                  cf_t*                          in1,
                                  cf_t*                          start,
                                  cf_t*                          between,
                                  uint32_t                       in1_in0_d,
                                  uint32_t                       M,
                                  bool                           to_right,
                                  uint32_t                       len)
{
  uint32_t i;

  isrran_vec_sub_ccc(in1, in0, q->diff_vec, len);
  isrran_vec_sc_prod_cfc(q->diff_vec, (float)1 / in1_in0_d, q->diff_vec, len);

  if (start) {
    isrran_vec_sum_ccc(start, q->diff_vec, between, len);
  } else {
    isrran_vec_sum_ccc(in0, q->diff_vec, between, len);
  }
  for (i = 0; i < M - 1; i++) {
    // Operations are done to len samples but pointers are moved the full vector length
    if (to_right) {
      isrran_vec_sum_ccc(between, q->diff_vec, &between[q->vector_len], len);
      between += q->vector_len;
    } else {
      between -= q->vector_len;
      isrran_vec_sum_ccc(&between[q->vector_len], q->diff_vec, between, len);
    }
  }
}

int isrran_interp_linear_init(isrran_interp_lin_t* q, uint32_t vector_len, uint32_t M)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q) {
    bzero(q, sizeof(isrran_interp_lin_t));
    ret         = ISRRAN_SUCCESS;
    q->diff_vec = isrran_vec_cf_malloc(vector_len);
    if (!q->diff_vec) {
      perror("malloc");
      return ISRRAN_ERROR;
    }
    q->diff_vec2 = isrran_vec_cf_malloc(M * vector_len);
    if (!q->diff_vec2) {
      perror("malloc");
      free(q->diff_vec);
      return ISRRAN_ERROR;
    }
    q->ramp = isrran_vec_f_malloc(M);
    if (!q->ramp) {
      perror("malloc");
      free(q->ramp);
      free(q->diff_vec);
      return ISRRAN_ERROR;
    }

    for (int i = 0; i < M; i++) {
      q->ramp[i] = (float)i;
    }

    q->vector_len     = vector_len;
    q->M              = M;
    q->max_vector_len = vector_len;
    q->max_M          = M;
  }
  return ret;
}

void isrran_interp_linear_free(isrran_interp_lin_t* q)
{
  if (q->diff_vec) {
    free(q->diff_vec);
  }
  if (q->diff_vec2) {
    free(q->diff_vec2);
  }
  if (q->ramp) {
    free(q->ramp);
  }

  bzero(q, sizeof(isrran_interp_lin_t));
}

int isrran_interp_linear_resize(isrran_interp_lin_t* q, uint32_t vector_len, uint32_t M)
{
  if (vector_len <= q->max_vector_len && M <= q->max_M) {
    for (int i = 0; i < M; i++) {
      q->ramp[i] = (float)i;
    }

    q->vector_len = vector_len;
    q->M          = M;
    return ISRRAN_SUCCESS;
  } else {
    ERROR("Error resizing interp_linear: vector_len and M must be lower or equal than initialized");
    return ISRRAN_ERROR;
  }
}

void isrran_interp_linear_offset(isrran_interp_lin_t* q, cf_t* input, cf_t* output, uint32_t off_st, uint32_t off_end)
{
  uint32_t i, j;
  cf_t     diff;

  i = 0;
  for (j = 0; j < off_st; j++) {
    output[off_st - j - 1] = input[i] - (j + 1) * (input[i + 1] - input[i]) / q->M;
  }
  isrran_vec_sub_ccc(&input[1], input, q->diff_vec, (q->vector_len - 1));
  isrran_vec_sc_prod_cfc(q->diff_vec, (float)1 / q->M, q->diff_vec, q->vector_len - 1);
  for (i = 0; i < q->vector_len - 1; i++) {
    for (j = 0; j < q->M; j++) {
      output[i * q->M + j + off_st] = input[i];
      q->diff_vec2[i * q->M + j]    = q->diff_vec[i];
    }
    isrran_vec_prod_cfc(&q->diff_vec2[i * q->M], q->ramp, &q->diff_vec2[i * q->M], q->M);
  }
  isrran_vec_sum_ccc(&output[off_st], q->diff_vec2, &output[off_st], q->M * (q->vector_len - 1));

  if (q->vector_len > 1) {
    diff = input[q->vector_len - 1] - input[q->vector_len - 2];
    for (j = 0; j < off_end; j++) {
      output[i * q->M + j + off_st] = input[i] + j * diff / q->M;
    }
  }
}
