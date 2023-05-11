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

/*!
 * \file polar_decoder.h
 * \brief Declaration of the polar decoder.
 * \author Jesus Gomez
 * \date 2020
 *
 * \copyright iSignal Research Labs Pvt Ltd.
 *
 * 5G uses a polar decoder with maximum sizes \f$2^n\f$ with \f$n = 5,...,10\f$.
 *
 */

#ifndef ISRRAN_POLARDECODER_H
#define ISRRAN_POLARDECODER_H
#include "isrran/config.h"
#include <stdbool.h>
#include <stdint.h>

/*!
 * Lists the different types of polar decoder.
 */
typedef enum ISRRAN_API {
  ISRRAN_POLAR_DECODER_SSC_F = 0, /*!< \brief Floating-point Simplified Successive Cancellation (SSC) decoder. */
  ISRRAN_POLAR_DECODER_SSC_S = 1, /*!< \brief Fixed-point (16 bit) Simplified Successive Cancellation (SSC) decoder. */
  ISRRAN_POLAR_DECODER_SSC_C = 2, /*!< \brief Fixed-point (8 bit) Simplified Successive Cancellation (SSC) decoder. */
  ISRRAN_POLAR_DECODER_SSC_C_AVX2 =
      3 /*!< \brief Fixed-point (8 bit, avx2) Simplified Successive Cancellation (SSC) decoder. */
} isrran_polar_decoder_type_t;

/*!
 * \brief Describes a polar decoder.
 */
typedef struct ISRRAN_API {
  void*   ptr;  /*!< \brief Pointer to the actual polar decoder structure. */
  uint8_t nMax; /*!< \brief Maximum \f$log_2(code_size)\f$. */
  int (*decode_f)(void*           ptr,
                  const float*    symbols,
                  uint8_t*        data_decoded,
                  const uint8_t   n,
                  const uint16_t* frozen_set,
                  const uint16_t  frozen_set_size); /*!< \brief Pointer to the decoder function (float version). */
  int (*decode_s)(void*           ptr,
                  const int16_t*  symbols,
                  uint8_t*        data_decoded,
                  const uint8_t   n,
                  const uint16_t* frozen_set,
                  const uint16_t  frozen_set_size); /*!< \brief Pointer to the decoder function (16-bit version). */
  int (*decode_c)(void*           ptr,
                  const int8_t*   symbols,
                  uint8_t*        data_decoded,
                  const uint8_t   n,
                  const uint16_t* frozen_set,
                  const uint16_t  frozen_set_size); /*!< \brief Pointer to the decoder function (8-bit version). */
  void (*free)(void*);                             /*!< \brief Pointer to a "destructor". */
} isrran_polar_decoder_t;

/*!
 * Initializes all the polar decoder variables according to the selected decoding
 * algorithm and the given code size.
 * \param[out] q A pointer to the initialized polar decoder.
 * \param[in] polar_decoder_type Polar decoder type.
 * \param[in] code_size_log The \f$ log_2\f$ of the number of bits of the decoder input/output vector.
 * \return An integer: 0 if the function executes correctly, -1 otherwise.
 */
ISRRAN_API int isrran_polar_decoder_init(isrran_polar_decoder_t*     q,
                                         isrran_polar_decoder_type_t polar_decoder_type,
                                         const uint8_t               code_size_log);

/*!
 * The polar decoder "destructor": it frees all the resources.
 * \param[in, out] q A pointer to the dismantled decoder.
 */
ISRRAN_API void isrran_polar_decoder_free(isrran_polar_decoder_t* q);

/*!
 * Decodes the input (float) codeword with the specified polar decoder.
 * \param[in] q A pointer to the desired polar decoder.
 * \param[in] input_llr The decoder LLR input vector.
 * \param[out] data_decoded The decoder output vector.
 * \param[in] code_size_log The \f$ log_2\f$ of the number of bits of the decoder input/output vector.
 * \param[in] frozen_set The position of the frozen bits in increasing order.
 * \param[in] frozen_set_size The size of the frozen_set.
 * \return An integer: 0 if the function executes correctly, -1 otherwise.
 */
ISRRAN_API int isrran_polar_decoder_decode_f(isrran_polar_decoder_t* q,
                                             const float*            input_llr,
                                             uint8_t*                data_decoded,
                                             const uint8_t           code_size_log,
                                             const uint16_t*         frozen_set,
                                             const uint16_t          frozen_set_size);

/*!
 * Decodes the input (int16_t) codeword with the specified polar decoder.
 * \param[in] q A pointer to the desired polar decoder.
 * \param[in] input_llr The decoder LLR input vector.
 * \param[out] data_decoded The decoder output vector.
 * \param[in] code_size_log The \f$ log_2\f$ of the number of bits of the decoder input/output vector.
 * \param[in] frozen_set The position of the frozen bits in increasing order.
 * \param[in] frozen_set_size The size of the frozen_set.

 * \return An integer: 0 if the function executes correctly, -1 otherwise.
 */
ISRRAN_API int isrran_polar_decoder_decode_s(isrran_polar_decoder_t* q,
                                             const int16_t*          input_llr,
                                             uint8_t*                data_decoded,
                                             const uint8_t           code_size_log,
                                             const uint16_t*         frozen_set,
                                             const uint16_t          frozen_set_size);

/*!
 * Decodes the input (int8_t) codeword with the specified polar decoder.
 * \param[in] q A pointer to the desired polar decoder.
 * \param[in] input_llr The decoder LLR input vector.
 * \param[out] data_decoded The decoder output vector.
 * \param[in] code_size_log The \f$ log_2\f$ of the number of bits of the decoder input/output vector.
 * \param[in] frozen_set The position of the frozen bits in increasing order.
 * \param[in] frozen_set_size The size of the frozen_set.
 * \return An integer: 0 if the function executes correctly, -1 otherwise.
 */
ISRRAN_API int isrran_polar_decoder_decode_c(isrran_polar_decoder_t* q,
                                             const int8_t*           input_llr,
                                             uint8_t*                data_decoded,
                                             const uint8_t           code_size_log,
                                             const uint16_t*         frozen_set,
                                             const uint16_t          frozen_set_size);

#endif // ISRRAN_POLARDECODER_H
