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

#include "isrran/phy/scrambling/scrambling.h"
#include "isrran/phy/utils/bit.h"
#include "isrran/phy/utils/vector.h"
#include <assert.h>

void isrran_scrambling_f(isrran_sequence_t* s, float* data)
{
  isrran_scrambling_f_offset(s, data, 0, s->cur_len);
}

void isrran_scrambling_f_offset(isrran_sequence_t* s, float* data, int offset, int len)
{
  assert(len + offset <= s->cur_len);
  isrran_vec_prod_fff(data, &s->c_float[offset], data, len);
}

void isrran_scrambling_s(isrran_sequence_t* s, short* data)
{
  isrran_scrambling_s_offset(s, data, 0, s->cur_len);
}

void isrran_scrambling_s_offset(isrran_sequence_t* s, short* data, int offset, int len)
{
  assert(len + offset <= s->cur_len);
  isrran_vec_neg_sss(data, &s->c_short[offset], data, len);
}

void isrran_scrambling_sb_offset(isrran_sequence_t* s, int8_t* data, int offset, int len)
{
  assert(len + offset <= s->cur_len);
  isrran_vec_neg_bbb(data, &s->c_char[offset], data, len);
}

void isrran_scrambling_c(isrran_sequence_t* s, cf_t* data)
{
  isrran_scrambling_c_offset(s, data, 0, s->cur_len);
}

void isrran_scrambling_c_offset(isrran_sequence_t* s, cf_t* data, int offset, int len)
{
  assert(len + offset <= s->cur_len);
  isrran_vec_prod_cfc(data, &s->c_float[offset], data, len);
}

static inline void scrambling_b(uint8_t* c, uint8_t* data, int len)
{
  isrran_vec_xor_bbb(c, data, data, len);
}

void isrran_scrambling_b(isrran_sequence_t* s, uint8_t* data)
{
  isrran_scrambling_b_offset(s, data, 0, s->cur_len);
}

void isrran_scrambling_b_offset(isrran_sequence_t* s, uint8_t* data, int offset, int len)
{
  scrambling_b(&s->c[offset], data, len);
}

void isrran_scrambling_bytes(isrran_sequence_t* s, uint8_t* data, int len)
{
  scrambling_b(s->c_bytes, data, len / 8);
  // Scramble last bits
  if (len % 8) {
    uint8_t tmp_bits[8];
    isrran_bit_unpack_vector(&data[len / 8], tmp_bits, len % 8);
    scrambling_b(&s->c[8 * (len / 8)], tmp_bits, len % 8);
    isrran_bit_pack_vector(tmp_bits, &data[len / 8], len % 8);
  }
}
