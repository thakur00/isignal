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
 *  File:         bit.h
 *
 *  Description:  Bit-level utilities.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_BIT_H
#define ISRRAN_BIT_H

#include <stdint.h>
#include <stdio.h>

#include "isrran/config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t  nof_bits;
  uint16_t* interleaver;
  uint16_t* byte_idx;
  uint8_t*  bit_mask;
  uint8_t   n_128;
} isrran_bit_interleaver_t;

ISRRAN_API void isrran_bit_interleaver_init(isrran_bit_interleaver_t* q, uint16_t* interleaver, uint32_t nof_bits);

ISRRAN_API void isrran_bit_interleaver_free(isrran_bit_interleaver_t* q);

ISRRAN_API void
isrran_bit_interleaver_run(isrran_bit_interleaver_t* q, uint8_t* input, uint8_t* output, uint16_t w_offset);

ISRRAN_API void isrran_bit_interleave(uint8_t* input, uint8_t* output, uint16_t* interleaver, uint32_t nof_bits);

ISRRAN_API void
isrran_bit_copy(uint8_t* dst, uint32_t dst_offset, uint8_t* src, uint32_t src_offset, uint32_t nof_bits);

ISRRAN_API void isrran_bit_interleave_i(uint8_t* input, uint8_t* output, uint32_t* interleaver, uint32_t nof_bits);

ISRRAN_API void isrran_bit_interleave_i_w_offset(uint8_t*  input,
                                                 uint8_t*  output,
                                                 uint32_t* interleaver,
                                                 uint32_t  nof_bits,
                                                 uint32_t  w_offset);

ISRRAN_API void isrran_bit_interleave_w_offset(uint8_t*  input,
                                               uint8_t*  output,
                                               uint16_t* interleaver,
                                               uint32_t  nof_bits,
                                               uint32_t  w_offset);

ISRRAN_API void isrran_bit_unpack_vector(const uint8_t* packed, uint8_t* unpacked, int nof_bits);

ISRRAN_API void isrran_bit_pack_vector(uint8_t* unpacked, uint8_t* packed, int nof_bits);

ISRRAN_API uint32_t isrran_bit_pack(uint8_t** bits, int nof_bits);

ISRRAN_API uint64_t isrran_bit_pack_l(uint8_t** bits, int nof_bits);

ISRRAN_API void isrran_bit_unpack_l(uint64_t value, uint8_t** bits, int nof_bits);

ISRRAN_API void isrran_bit_unpack(uint32_t value, uint8_t** bits, int nof_bits);

ISRRAN_API void isrran_bit_unpack_lsb(uint32_t value, uint8_t** bits, int nof_bits);

ISRRAN_API void isrran_bit_fprint(FILE* stream, uint8_t* bits, int nof_bits);

ISRRAN_API uint32_t isrran_bit_diff(const uint8_t* x, const uint8_t* y, int nbits);

ISRRAN_API uint32_t isrran_bit_count(uint32_t n);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_BIT_H
