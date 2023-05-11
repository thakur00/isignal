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
 *  File:         viterbi.h
 *
 *  Description:  Viterbi decoder for convolutionally encoded data.
 *                Used for decoding of PBCH and PDCCH (type 37 decoder).
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_VITERBI_H
#define ISRRAN_VITERBI_H

#include "isrran/config.h"
#include <stdbool.h>

typedef enum { ISRRAN_VITERBI_27 = 0, ISRRAN_VITERBI_29, ISRRAN_VITERBI_37, ISRRAN_VITERBI_39 } isrran_viterbi_type_t;

typedef struct ISRRAN_API {
  void*    ptr;
  uint32_t R;
  uint32_t K;
  uint32_t framebits;
  bool     tail_biting;
  float    gain_quant;
  int16_t  gain_quant_s;
  int (*decode)(void*, uint8_t*, uint8_t*, uint32_t);
  int (*decode_s)(void*, uint16_t*, uint8_t*, uint32_t);
  int (*decode_f)(void*, float*, uint8_t*, uint32_t);
  void (*free)(void*);
  uint8_t*  tmp;
  uint16_t* tmp_s;
  uint8_t*  symbols_uc;
  uint16_t* symbols_us;
} isrran_viterbi_t;

ISRRAN_API int isrran_viterbi_init(isrran_viterbi_t*     q,
                                   isrran_viterbi_type_t type,
                                   int                   poly[3],
                                   uint32_t              max_frame_length,
                                   bool                  tail_bitting);

ISRRAN_API void isrran_viterbi_set_gain_quant(isrran_viterbi_t* q, float gain_quant);

ISRRAN_API void isrran_viterbi_set_gain_quant_s(isrran_viterbi_t* q, int16_t gain_quant);

ISRRAN_API void isrran_viterbi_free(isrran_viterbi_t* q);

ISRRAN_API int isrran_viterbi_decode_f(isrran_viterbi_t* q, float* symbols, uint8_t* data, uint32_t frame_length);

ISRRAN_API int isrran_viterbi_decode_s(isrran_viterbi_t* q, int16_t* symbols, uint8_t* data, uint32_t frame_length);

ISRRAN_API int isrran_viterbi_decode_us(isrran_viterbi_t* q, uint16_t* symbols, uint8_t* data, uint32_t frame_length);

ISRRAN_API int isrran_viterbi_decode_uc(isrran_viterbi_t* q, uint8_t* symbols, uint8_t* data, uint32_t frame_length);

ISRRAN_API int isrran_viterbi_init_sse(isrran_viterbi_t*     q,
                                       isrran_viterbi_type_t type,
                                       int                   poly[3],
                                       uint32_t              max_frame_length,
                                       bool                  tail_bitting);

ISRRAN_API int isrran_viterbi_init_neon(isrran_viterbi_t*     q,
                                        isrran_viterbi_type_t type,
                                        int                   poly[3],
                                        uint32_t              max_frame_length,
                                        bool                  tail_bitting);

ISRRAN_API int isrran_viterbi_init_avx2(isrran_viterbi_t*     q,
                                        isrran_viterbi_type_t type,
                                        int                   poly[3],
                                        uint32_t              max_frame_length,
                                        bool                  tail_bitting);

#endif // ISRRAN_VITERBI_H
