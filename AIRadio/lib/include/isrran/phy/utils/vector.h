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
 *  File:         vector.h
 *
 *  Description:  Vector functions using SIMD instructions where possible.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_VECTOR_H
#define ISRRAN_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "isrran/config.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ISRRAN_MEM_ALLOC(T, N) ((T*)isrran_vec_malloc((uint32_t)sizeof(T) * (N)))
#define ISRRAN_MEM_ZERO(Q, T, N)                                                                                       \
  do {                                                                                                                 \
    T* ptr_ = (Q);                                                                                                     \
    isrran_vec_zero((void*)ptr_, (uint32_t)sizeof(T) * (N));                                                           \
  } while (false)

#define ISRRAN_MAX(a, b) ((a) > (b) ? (a) : (b))
#define ISRRAN_MIN(a, b) ((a) < (b) ? (a) : (b))
#define ISRRAN_CEIL(NUM, DEN) (((NUM) + ((DEN)-1)) / (DEN))
#define ISRRAN_FLOOR(NUM, DEN) ((NUM) / (DEN))
#define ISRRAN_ROUND(NUM, DEN) ((uint32_t)round((double)(NUM) / (double)(DEN)))
#define ISRRAN_CEIL_LOG2(N) (((N) == 0) ? 0 : ceil(log2((double)(N))))

// Complex squared absolute value
#define ISRRAN_CSQABS(X) (__real__(X) * __real__(X) + __imag__(X) * __imag__(X))

// Cumulative moving average
#define ISRRAN_VEC_CMA(data, average, n) ((average) + ((data) - (average)) / ((n) + 1))

// Cumulative moving average
#ifdef __cplusplus
#define ISRRAN_VEC_SAFE_CMA(data, average, n) (std::isnormal(average) ? ISRRAN_VEC_CMA(data, average, n) : (data))
#else
#define ISRRAN_VEC_SAFE_CMA(data, average, n) (isnormal(average) ? ISRRAN_VEC_CMA(data, average, n) : (data))
#endif

// Proportional moving average
#define ISRRAN_VEC_PMA(average1, n1, average2, n2) (((average1) * (n1) + (average2) * (n2)) / ((n1) + (n2)))

// Safe Proportional moving average
#ifdef __cplusplus
#define ISRRAN_VEC_SAFE_PMA(average1, n1, average2, n2)                                                                \
  (std::isnormal((n1) + (n2)) ? ISRRAN_VEC_PMA(average1, n1, average2, n2) : (0))
#else
#define ISRRAN_VEC_SAFE_PMA(average1, n1, average2, n2)                                                                \
  (isnormal((n1) + (n2)) ? ISRRAN_VEC_PMA(average1, n1, average2, n2) : (0))
#endif

// Exponential moving average
#define ISRRAN_VEC_EMA(data, average, alpha) ((alpha) * (data) + (1 - alpha) * (average))

// Safe exponential moving average
#ifdef __cplusplus
#define ISRRAN_VEC_SAFE_EMA(data, average, alpha)                                                                      \
  (std::isnormal(average) ? ISRRAN_VEC_EMA(data, average, alpha) : (data))
#else
#define ISRRAN_VEC_SAFE_EMA(data, average, alpha) (isnormal(average) ? ISRRAN_VEC_EMA(data, average, alpha) : (data))
#endif

static inline float isrran_convert_amplitude_to_dB(float v)
{
  return 20.0f * log10f(v);
}
static inline float isrran_convert_power_to_dB(float v)
{
  return 10.0f * log10f(v);
}
static inline float isrran_convert_power_to_dBm(float v)
{
  return isrran_convert_power_to_dB(v) + 30.0f;
}
static inline float isrran_convert_dB_to_amplitude(float v)
{
  return powf(10.0f, v / 20.0f);
}
static inline float isrran_convert_dB_to_power(float v)
{
  return powf(10.0f, v / 10.0f);
}

/*!
 * Computes \f$ z = x \oplus y \f$ elementwise.
 * \param[in] x A pointer to a vector of uint8_t with 0's and 1's.
 * \param[in] y A pointer to a vector of uint8_t with 0's and 1's.
 * \param[out] z A pointer to a vector of uint8_t with 0's and 1's.
 * \param[in] len Length of vectors x, y and z.
 */
ISRRAN_API void isrran_vec_xor_bbb(const uint8_t* x, const uint8_t* y, uint8_t* z, const uint32_t len);

/** Return the sum of all the elements */
ISRRAN_API float isrran_vec_acc_ff(const float* x, const uint32_t len);
ISRRAN_API cf_t  isrran_vec_acc_cc(const cf_t* x, const uint32_t len);

ISRRAN_API void* isrran_vec_malloc(uint32_t size);
ISRRAN_API cf_t*  isrran_vec_cf_malloc(uint32_t size);
ISRRAN_API float* isrran_vec_f_malloc(uint32_t size);
ISRRAN_API int32_t* isrran_vec_i32_malloc(uint32_t nsamples);
ISRRAN_API uint32_t* isrran_vec_u32_malloc(uint32_t nsamples);
ISRRAN_API int16_t* isrran_vec_i16_malloc(uint32_t nsamples);
ISRRAN_API uint16_t* isrran_vec_u16_malloc(uint32_t nsamples);
ISRRAN_API int8_t* isrran_vec_i8_malloc(uint32_t nsamples);
ISRRAN_API uint8_t* isrran_vec_u8_malloc(uint32_t nsamples);

ISRRAN_API void* isrran_vec_realloc(void* ptr, uint32_t old_size, uint32_t new_size);

/* Zero memory */
ISRRAN_API void isrran_vec_zero(void* ptr, uint32_t nsamples);
ISRRAN_API void isrran_vec_cf_zero(cf_t* ptr, uint32_t nsamples);
ISRRAN_API void isrran_vec_f_zero(float* ptr, uint32_t nsamples);
ISRRAN_API void isrran_vec_i8_zero(int8_t* ptr, uint32_t nsamples);
ISRRAN_API void isrran_vec_u8_zero(uint8_t* ptr, uint32_t nsamples);
ISRRAN_API void isrran_vec_i16_zero(int16_t* ptr, uint32_t nsamples);
ISRRAN_API void isrran_vec_u32_zero(uint32_t* ptr, uint32_t nsamples);

/* Copy memory */
ISRRAN_API void isrran_vec_cf_copy(cf_t* dst, const cf_t* src, uint32_t len);
ISRRAN_API void isrran_vec_f_copy(float* dst, const float* src, uint32_t len);
ISRRAN_API void isrran_vec_u8_copy(uint8_t* dst, const uint8_t* src, uint32_t len);
ISRRAN_API void isrran_vec_i8_copy(int8_t* dst, const int8_t* src, uint32_t len);
ISRRAN_API void isrran_vec_u16_copy(uint16_t* dst, const uint16_t* src, uint32_t len);
ISRRAN_API void isrran_vec_i16_copy(int16_t* dst, const int16_t* src, uint32_t len);

/* print vectors */
ISRRAN_API void     isrran_vec_fprint_c(FILE* stream, const cf_t* x, const uint32_t len);
ISRRAN_API void     isrran_vec_fprint_f(FILE* stream, const float* x, const uint32_t len);
ISRRAN_API void     isrran_vec_fprint_b(FILE* stream, const uint8_t* x, const uint32_t len);
ISRRAN_API void     isrran_vec_fprint_bs(FILE* stream, const int8_t* x, const uint32_t len);
ISRRAN_API void     isrran_vec_fprint_byte(FILE* stream, const uint8_t* x, const uint32_t len);
ISRRAN_API void     isrran_vec_fprint_i(FILE* stream, const int* x, const uint32_t len);
ISRRAN_API void     isrran_vec_fprint_s(FILE* stream, const int16_t* x, const uint32_t len);
ISRRAN_API void     isrran_vec_fprint_hex(FILE* stream, uint8_t* x, const uint32_t len);
ISRRAN_API uint32_t isrran_vec_sprint_hex(char* str, const uint32_t max_str_len, uint8_t* x, const uint32_t len);
ISRRAN_API void     isrran_vec_sprint_bin(char* str, const uint32_t max_str_len, const uint8_t* x, const uint32_t len);

/* Saves/loads a vector to a file */
ISRRAN_API void isrran_vec_save_file(char* filename, const void* buffer, const uint32_t len);
ISRRAN_API void isrran_vec_load_file(char* filename, void* buffer, const uint32_t len);

/* sum two vectors */
ISRRAN_API void isrran_vec_sum_fff(const float* x, const float* y, float* z, const uint32_t len);
ISRRAN_API void isrran_vec_sum_ccc(const cf_t* x, const cf_t* y, cf_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_sum_sss(const int16_t* x, const int16_t* y, int16_t* z, const uint32_t len);

/* substract two vectors z=x-y */
ISRRAN_API void isrran_vec_sub_fff(const float* x, const float* y, float* z, const uint32_t len);
ISRRAN_API void isrran_vec_sub_ccc(const cf_t* x, const cf_t* y, cf_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_sub_sss(const int16_t* x, const int16_t* y, int16_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_sub_bbb(const int8_t* x, const int8_t* y, int8_t* z, const uint32_t len);

/* sum a scalar to all elements of a vector */
ISRRAN_API void isrran_vec_sc_sum_fff(const float* x, float h, float* z, uint32_t len);

/* scalar product */
ISRRAN_API void isrran_vec_sc_prod_cfc(const cf_t* x, const float h, cf_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_sc_prod_fcc(const float* x, const cf_t h, cf_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_sc_prod_ccc(const cf_t* x, const cf_t h, cf_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_sc_prod_fff(const float* x, const float h, float* z, const uint32_t len);

ISRRAN_API void isrran_vec_convert_fi(const float* x, const float scale, int16_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_convert_conj_cs(const cf_t* x, const float scale, int16_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_convert_if(const int16_t* x, const float scale, float* z, const uint32_t len);
ISRRAN_API void isrran_vec_convert_fb(const float* x, const float scale, int8_t* z, const uint32_t len);

ISRRAN_API void isrran_vec_lut_sss(const short* x, const unsigned short* lut, short* y, const uint32_t len);
ISRRAN_API void isrran_vec_lut_bbb(const int8_t* x, const unsigned short* lut, int8_t* y, const uint32_t len);
ISRRAN_API void isrran_vec_lut_sis(const short* x, const unsigned int* lut, short* y, const uint32_t len);

/* vector product (element-wise) */
ISRRAN_API void isrran_vec_prod_ccc(const cf_t* x, const cf_t* y, cf_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_prod_ccc_split(const float*   x_re,
                                          const float*   x_im,
                                          const float*   y_re,
                                          const float*   y_im,
                                          float*         z_re,
                                          float*         z_im,
                                          const uint32_t len);

/* vector product (element-wise) */
ISRRAN_API void isrran_vec_prod_cfc(const cf_t* x, const float* y, cf_t* z, const uint32_t len);

/* conjugate vector product (element-wise) */
ISRRAN_API void isrran_vec_prod_conj_ccc(const cf_t* x, const cf_t* y, cf_t* z, const uint32_t len);

/* real vector product (element-wise) */
ISRRAN_API void isrran_vec_prod_fff(const float* x, const float* y, float* z, const uint32_t len);
ISRRAN_API void isrran_vec_prod_sss(const int16_t* x, const int16_t* y, int16_t* z, const uint32_t len);

// Negate sign (scrambling)
ISRRAN_API void isrran_vec_neg_sss(const int16_t* x, const int16_t* y, int16_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_neg_bbb(const int8_t* x, const int8_t* y, int8_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_neg_bb(const int8_t* x, int8_t* z, const uint32_t len);

/* Dot-product */
ISRRAN_API cf_t    isrran_vec_dot_prod_cfc(const cf_t* x, const float* y, const uint32_t len);
ISRRAN_API cf_t    isrran_vec_dot_prod_ccc(const cf_t* x, const cf_t* y, const uint32_t len);
ISRRAN_API cf_t    isrran_vec_dot_prod_conj_ccc(const cf_t* x, const cf_t* y, const uint32_t len);
ISRRAN_API float   isrran_vec_dot_prod_fff(const float* x, const float* y, const uint32_t len);
ISRRAN_API int32_t isrran_vec_dot_prod_sss(const int16_t* x, const int16_t* y, const uint32_t len);

/* z=x/y vector division (element-wise) */
ISRRAN_API void isrran_vec_div_ccc(const cf_t* x, const cf_t* y, cf_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_div_cfc(const cf_t* x, const float* y, cf_t* z, const uint32_t len);
ISRRAN_API void isrran_vec_div_fff(const float* x, const float* y, float* z, const uint32_t len);

/* conjugate */
ISRRAN_API void isrran_vec_conj_cc(const cf_t* x, cf_t* y, const uint32_t len);

/* average vector power */
ISRRAN_API float isrran_vec_avg_power_cf(const cf_t* x, const uint32_t len);
ISRRAN_API float isrran_vec_avg_power_sf(const int16_t* x, const uint32_t len);
ISRRAN_API float isrran_vec_avg_power_bf(const int8_t* x, const uint32_t len);
ISRRAN_API float isrran_vec_avg_power_ff(const float* x, const uint32_t len);

/* Correlation between complex vectors x and y */
ISRRAN_API float isrran_vec_corr_ccc(const cf_t* x, cf_t* y, const uint32_t len);

/* return the index of the maximum value in the vector */
ISRRAN_API uint32_t isrran_vec_max_fi(const float* x, const uint32_t len);
ISRRAN_API uint32_t isrran_vec_max_abs_fi(const float* x, const uint32_t len);
ISRRAN_API uint32_t isrran_vec_max_abs_ci(const cf_t* x, const uint32_t len);

/*!
 * Quantizes an array of floats into an array of 16-bit signed integers. It is
 * ensured that *-inf* and *inf* map to -32767 and 32767, respectively (useful
 * when quantizing on less than 16 bits).
 * \param[in]  in     Real values to be quantized.
 * \param[out] out    Quantized values.
 * \param[in]  gain   Quantization gain, controls the output range.
 * \param[in]  offset Quantization offset, for asymmetric quantization.
 * \param[in]  clip   Saturation value.
 * \param[in]  len    Number of values to be quantized.
 */
ISRRAN_API void isrran_vec_quant_fs(const float* in, int16_t* out, float gain, float offset, float clip, uint32_t len);

/*!
 * Quantizes an array of floats into an array of 8-bit signed integers. It is
 * ensured that *-inf* and *inf* map to -127 and 127, respectively (useful
 * when quantizing on less than 8 bits).
 * \param[in]  in     Real values to be quantized.
 * \param[out] out    Quantized values.
 * \param[in]  gain   Quantization gain, controls the output range.
 * \param[in]  offset Quantization offset, for asymmetric quantization.
 * \param[in]  clip   Saturation value.
 * \param[in]  len    Number of values to be quantized.
 */
ISRRAN_API void isrran_vec_quant_fc(const float* in, int8_t* out, float gain, float offset, float clip, uint32_t len);

/* quantify vector of floats or int16 and convert to uint8_t */
ISRRAN_API void isrran_vec_quant_fuc(const float*   in,
                                     uint8_t*       out,
                                     const float    gain,
                                     const float    offset,
                                     const uint8_t  clip,
                                     const uint32_t len);
ISRRAN_API void isrran_vec_quant_fus(const float*   in,
                                     uint16_t*      out,
                                     const float    gain,
                                     const float    offset,
                                     const uint16_t clip,
                                     const uint32_t len);
ISRRAN_API void isrran_vec_quant_suc(const int16_t* in,
                                     uint8_t*       out,
                                     const float    gain,
                                     const float    offset,
                                     const uint8_t  clip,
                                     const uint32_t len);

ISRRAN_API void isrran_vec_quant_sus(const int16_t* in,
                                     uint16_t*      out,
                                     const float    gain,
                                     const float    offset,
                                     const uint16_t clip,
                                     const uint32_t len);
/* magnitude of each vector element */
ISRRAN_API void isrran_vec_abs_cf(const cf_t* x, float* abs, const uint32_t len);
ISRRAN_API void isrran_vec_abs_square_cf(const cf_t* x, float* abs_square, const uint32_t len);

/**
 * @brief Extracts module in decibels of a complex vector
 *
 * This function extracts the module in decibels of a complex array input. Abnormal absolute value inputs (zero,
 * infinity and not-a-number) are set to default_value outputs.
 *
 * Equivalent code:
 *   for (int i = 0; i < len; i++) {
 *     float mag = x[i];
 *
 *     // Check boundaries
 *     if (isnormal(mag)) {
 *       // Avoid infinites and zeros
 *       abs[i] = 20.0f * log10f(mag);
 *     } else {
 *       // Set to default value instead
 *       abs[i] = default_value;
 *     }
 *   }
 *
 * @param x is the input complex vector
 * @param default_value is the value to use in case of having an abnormal absolute value.
 * @param abs is the destination vector
 * @param len is the input and output number of samples
 *
 */
ISRRAN_API void isrran_vec_abs_dB_cf(const cf_t* x, float default_value, float* abs, const uint32_t len);

/**
 * @brief Extracts argument in degrees from a complex vector
 *
 * This function extracts the argument from a complex vector. Infinity and not-a-number results are set to
 * default_value.
 *
 * Equivalent code:
 *   for(int i = 0; i < len; i++) {
 *     arg[i] = cargf(x[i]) * (180.0f / M_PI);
 *
 *     if (arg[i]!=0.0f && !isnormal(arg[i])) {
 *      arg[i] = default_value;
 *     }
 *   }
 *
 * @param x is the input complex vector
 * @param default_value is the value to use in case of having an abnormal result.
 * @param arg is the destination vector
 * @param len is the input and output number of samples
 *
 */
ISRRAN_API void isrran_vec_arg_deg_cf(const cf_t* x, float default_value, float* arg, const uint32_t len);

ISRRAN_API float isrran_mean_arg_cf(const cf_t* x, uint32_t len);

ISRRAN_API void isrran_vec_interleave(const cf_t* x, const cf_t* y, cf_t* z, const int len);

ISRRAN_API void isrran_vec_interleave_add(const cf_t* x, const cf_t* y, cf_t* z, const int len);

ISRRAN_API cf_t isrran_vec_gen_sine(cf_t amplitude, float freq, cf_t* z, int len);

ISRRAN_API void isrran_vec_apply_cfo(const cf_t* x, float cfo, cf_t* z, int len);

ISRRAN_API float isrran_vec_estimate_frequency(const cf_t* x, int len);

/*!
 * @brief Generates an amplitude envelope that, multiplied point-wise with a vector, results in clipping
 * by a specified amplitude threshold.
 * @param[in]  x_abs     Absolute value vector of the signal to be clipped
 * @param[in]  thres     Clipping threshold
 * @param[out] clip_env  The generated clipping envelope
 * @param[in]  len       Length of the vector.
 */
ISRRAN_API void
isrran_vec_gen_clip_env(const float* x_abs, const float thres, const float alpha, float* env, const int len);

/*!
 * @brief Calculates the PAPR of a complex vector
 * @param[in]  in  Input vector
 * @param[in]  len Vector length.
 */
ISRRAN_API float isrran_vec_papr_c(const cf_t* in, const int len);

/*!
 * @brief Calculates the ACPR of a signal using its baseband spectrum
 * @attention The spectrum passed by x_f needs to be in FFT form
 * @param[in]  x_f          Spectrum of the signal
 * @param[in]  win_pos_len  Channel frequency window for the positive side of the spectrum
 * @param[in]  win_neg_len  Channel frequency window for the negative side of the spectrum
 * @param[in]  len          Length of the x_f vector
 * @returns    The ACPR in linear form
 */
ISRRAN_API float
isrran_vec_acpr_c(const cf_t* x_f, const uint32_t win_pos_len, const uint32_t win_neg_len, const uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_VECTOR_H
