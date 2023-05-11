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

#include "isrran/phy/common/sequence.h"
#include "isrran/phy/utils/bit.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/random.h"

#define Nc 1600
#define MAX_SEQ_LEN (256 * 1024)

static uint8_t x1[Nc + MAX_SEQ_LEN + 31];
static uint8_t x2[Nc + MAX_SEQ_LEN + 31];
static uint8_t c[Nc + MAX_SEQ_LEN + 31];
static float   c_float[Nc + MAX_SEQ_LEN + 31];
static int16_t c_short[Nc + MAX_SEQ_LEN + 31];
static int8_t  c_char[Nc + MAX_SEQ_LEN + 31];
static uint8_t c_packed_gold[MAX_SEQ_LEN / 8];
static uint8_t c_packed[MAX_SEQ_LEN / 8];
static uint8_t c_unpacked[MAX_SEQ_LEN];

static float   ones_float[Nc + MAX_SEQ_LEN + 31];
static int16_t ones_short[Nc + MAX_SEQ_LEN + 31];
static int8_t  ones_char[Nc + MAX_SEQ_LEN + 31];
static uint8_t ones_packed[(MAX_SEQ_LEN * 7) / 8];
static uint8_t ones_unpacked[MAX_SEQ_LEN];

static int test_sequence(isrran_sequence_t* sequence, uint32_t seed, uint32_t length, uint32_t repetitions)
{
  int            ret                      = ISRRAN_SUCCESS;
  struct timeval t[3]                     = {};
  uint64_t       interval_gen_us          = 0;
  uint64_t       interval_xor_float_us    = 0;
  uint64_t       interval_xor_short_us    = 0;
  uint64_t       interval_xor_char_us     = 0;
  uint64_t       interval_xor_unpacked_us = 0;
  uint64_t       interval_xor_packed_us   = 0;

  gettimeofday(&t[1], NULL);

  // Generate sequence
  for (uint32_t r = 0; r < repetitions; r++) {
    isrran_sequence_LTE_pr(sequence, length, seed);
  }

  gettimeofday(&t[2], NULL);
  get_time_interval(t);
  interval_gen_us = t->tv_sec * 1000000UL + t->tv_usec;

  // Generate gold sequence
  for (uint32_t n = 0; n < 31; n++) {
    x2[n] = (seed >> n) & 0x1;
  }
  x1[0] = 1;

  for (uint32_t n = 0; n < Nc + length; n++) {
    x1[n + 31] = (x1[n + 3] + x1[n]) & 0x1;
    x2[n + 31] = (x2[n + 3] + x2[n + 2] + x2[n + 1] + x2[n]) & 0x1;
  }

  for (uint32_t n = 0; n < length; n++) {
    c[n]       = (x1[n + Nc] + x2[n + Nc]) & 0x1;
    c_float[n] = c[n] ? -1.0f : +1.0f;
    c_short[n] = c[n] ? -1 : +1;
    c_char[n]  = c[n] ? -1 : +1;
  }

  isrran_bit_pack_vector(c, c_packed_gold, length);

  if (memcmp(c, sequence->c, length) != 0) {
    ERROR("Unmatched c");
    ret = ISRRAN_ERROR;
  }

  // Check Float sequence
  if (memcmp(c_float, sequence->c_float, length * sizeof(float)) != 0) {
    ERROR("Unmatched c_float");
    ret = ISRRAN_ERROR;
  }

  // Test in-place Float XOR
  gettimeofday(&t[1], NULL);
  for (uint32_t r = 0; r < repetitions; r++) {
    isrran_sequence_apply_f(ones_float, sequence->c_float, length, seed);
  }
  gettimeofday(&t[2], NULL);
  get_time_interval(t);
  interval_xor_float_us = t->tv_sec * 1000000UL + t->tv_usec;

  // Check Short Sequence
  if (memcmp(c_short, sequence->c_short, length * sizeof(int16_t)) != 0) {
    ERROR("Unmatched XOR c_short");
    ret = ISRRAN_ERROR;
  }

  // Test in-place Short XOR
  gettimeofday(&t[1], NULL);
  for (uint32_t r = 0; r < repetitions; r++) {
    isrran_sequence_apply_s(ones_short, sequence->c_short, length, seed);
  }
  gettimeofday(&t[2], NULL);
  get_time_interval(t);
  interval_xor_short_us = t->tv_sec * 1000000UL + t->tv_usec;

  if (memcmp(c_short, sequence->c_short, length * sizeof(int16_t)) != 0) {
    ERROR("Unmatched XOR c_short");
    ret = ISRRAN_ERROR;
  }

  // Check Char Sequence
  if (memcmp(c_char, sequence->c_char, length * sizeof(int8_t)) != 0) {
    ERROR("Unmatched c_char");
    ret = ISRRAN_ERROR;
  }

  // Test in-place Char XOR
  gettimeofday(&t[1], NULL);
  for (uint32_t r = 0; r < repetitions; r++) {
    isrran_sequence_apply_c(ones_char, sequence->c_char, length, seed);
  }
  gettimeofday(&t[2], NULL);
  get_time_interval(t);
  interval_xor_char_us = t->tv_sec * 1000000UL + t->tv_usec;

  // Test in-place unpacked XOR
  gettimeofday(&t[1], NULL);
  for (uint32_t r = 0; r < repetitions; r++) {
    isrran_sequence_apply_bit(ones_unpacked, c_unpacked, length, seed);
  }
  gettimeofday(&t[2], NULL);
  get_time_interval(t);
  interval_xor_unpacked_us = t->tv_sec * 1000000UL + t->tv_usec;

  // Test in-place packed XOR
  gettimeofday(&t[1], NULL);
  for (uint32_t r = 0; r < repetitions; r++) {
    isrran_sequence_apply_packed(ones_packed, c_packed, length, seed);
  }
  gettimeofday(&t[2], NULL);
  get_time_interval(t);
  interval_xor_packed_us = t->tv_sec * 1000000UL + t->tv_usec;

  if (memcmp(c_char, sequence->c_char, length * sizeof(int8_t)) != 0) {
    ERROR("Unmatched XOR c_char");
    ret = ISRRAN_ERROR;
  }

  if (memcmp(c_packed_gold, sequence->c_bytes, (length + 7) / 8) != 0) {
    ERROR("Unmatched c_packed");
    ret = ISRRAN_ERROR;
  }

  if (memcmp(c_packed_gold, c_packed, (length + 7) / 8) != 0) {
    ERROR("Unmatched c_packed");
    ret = ISRRAN_ERROR;
  }

  if (memcmp(c, c_unpacked, length) != 0) {
    ERROR("Unmatched c_unpacked");
    ret = ISRRAN_ERROR;
  }

  printf("%08x; %8d; %8.1f; %8.1f; %8.1f; %8.1f; %8.1f; %8.1f; %8c\n",
         seed,
         length,
         (double)(length * repetitions) / (double)interval_gen_us,
         (double)(length * repetitions) / (double)interval_xor_float_us,
         (double)(length * repetitions) / (double)interval_xor_short_us,
         (double)(length * repetitions) / (double)interval_xor_char_us,
         (double)(length * repetitions) / (double)interval_xor_unpacked_us,
         (double)(length * repetitions) / (double)interval_xor_packed_us,
         ret == ISRRAN_SUCCESS ? 'y' : 'n');

  return ISRRAN_SUCCESS;
}

int main(int argc, char** argv)
{
  uint32_t repetitions = 1;
  uint32_t min_length  = 16;
  uint32_t max_length  = MAX_SEQ_LEN;

  isrran_sequence_t sequence   = {};
  isrran_random_t   random_gen = isrran_random_init(0);

  // Initialise vectors with ones
  for (uint32_t i = 0; i < MAX_SEQ_LEN; i++) {
    ones_float[i]    = 1.0F;
    ones_short[i]    = 1;
    ones_char[i]     = 1;
    ones_unpacked[i] = 0;
    if (i < (MAX_SEQ_LEN * 7) / 8) {
      ones_packed[i] = 0;
    }
  }

  // Initialise sequence object
  if (isrran_sequence_init(&sequence, max_length) != ISRRAN_SUCCESS) {
    fprintf(stderr, "Error initializing sequence object\n");
    return ISRRAN_ERROR;
  }

  printf("%8s; %8s; %8s; %8s; %8s; %8s; %8s; %8s; %8s;\n",
         "seed",
         "length",
         "GEN",
         "XOR PS",
         "XOR 16",
         "XOR 8",
         "XOR Unpack",
         "XOR Pack",
         "Passed");

  for (uint32_t length = min_length; length <= max_length; length = (length * 5) / 4) {
    test_sequence(&sequence, (uint32_t)isrran_random_uniform_int_dist(random_gen, 1, INT32_MAX), length, repetitions);
  }

  // Free sequence object
  isrran_sequence_free(&sequence);
  isrran_random_free(random_gen);

  return ISRRAN_SUCCESS;
}
