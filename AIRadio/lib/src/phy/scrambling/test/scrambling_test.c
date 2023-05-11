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

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "isrran/isrran.h"

char*       isrran_sequence_name = NULL;
bool        do_floats            = false;
isrran_cp_t cp                   = ISRRAN_CP_NORM;
int         cell_id              = -1;
int         nof_bits             = 100;

void usage(char* prog)
{
  printf("Usage: %s [ef] -c cell_id -s [PBCH, PDSCH, PDCCH, PMCH, PUCCH]\n", prog);
  printf("\t -l nof_bits [Default %d]\n", nof_bits);
  printf("\t -e CP extended [Default CP Normal]\n");
  printf("\t -f scramble floats [Default bits]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "csefl")) != -1) {
    switch (opt) {
      case 'c':
        cell_id = (int)strtol(argv[optind], NULL, 10);
        break;
      case 'l':
        nof_bits = (int)strtol(argv[optind], NULL, 10);
        break;
      case 'e':
        cp = ISRRAN_CP_EXT;
        break;
      case 'f':
        do_floats = true;
        break;
      case 's':
        isrran_sequence_name = argv[optind];
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
  if (cell_id == -1) {
    usage(argv[0]);
    exit(-1);
  }
  if (!isrran_sequence_name) {
    usage(argv[0]);
    exit(-1);
  }
}

int init_sequence(isrran_sequence_t* seq, char* name)
{
  if (!strcmp(name, "PBCH")) {
    bzero(seq, sizeof(isrran_sequence_t));
    return isrran_sequence_pbch(seq, cp, cell_id);
  } else if (!strcmp(name, "PDSCH")) {
    bzero(seq, sizeof(isrran_sequence_t));
    return isrran_sequence_pdsch(seq, 1234, 0, 0, cell_id, nof_bits);
  } else {
    ERROR("Unsupported sequence name %s", name);
    return -1;
  }
}

int main(int argc, char** argv)
{
  int               i;
  isrran_sequence_t seq;
  uint8_t *         input_b, *scrambled_b;
  float *           input_f, *scrambled_f;
  struct timeval    t[3];

  parse_args(argc, argv);

  if (init_sequence(&seq, isrran_sequence_name) == -1) {
    ERROR("Error initiating sequence %s", isrran_sequence_name);
    exit(-1);
  }

  if (!do_floats) {
    input_b = isrran_vec_u8_malloc(seq.cur_len);
    if (!input_b) {
      perror("malloc");
      exit(-1);
    }
    scrambled_b = isrran_vec_u8_malloc(seq.cur_len);
    if (!scrambled_b) {
      perror("malloc");
      exit(-1);
    }

    for (i = 0; i < seq.cur_len; i++) {
      input_b[i]     = rand() % 2;
      scrambled_b[i] = input_b[i];
    }

    gettimeofday(&t[1], NULL);
    isrran_scrambling_b(&seq, scrambled_b);
    gettimeofday(&t[2], NULL);
    isrran_scrambling_b(&seq, scrambled_b);

    get_time_interval(t);
    printf("Texec=%ld us for %d bits\n", t[0].tv_usec, seq.cur_len);

    for (i = 0; i < seq.cur_len; i++) {
      if (scrambled_b[i] != input_b[i]) {
        printf("Error in %d\n", i);
        exit(-1);
      }
    }
    free(input_b);
    free(scrambled_b);
  } else {
    input_f = isrran_vec_f_malloc(seq.cur_len);
    if (!input_f) {
      perror("malloc");
      exit(-1);
    }
    scrambled_f = isrran_vec_f_malloc(seq.cur_len);
    if (!scrambled_f) {
      perror("malloc");
      exit(-1);
    }

    for (i = 0; i < seq.cur_len; i++) {
      input_f[i]     = 100 * (rand() / RAND_MAX);
      scrambled_f[i] = input_f[i];
    }

    gettimeofday(&t[1], NULL);
    isrran_scrambling_f(&seq, scrambled_f);
    gettimeofday(&t[2], NULL);
    isrran_scrambling_f(&seq, scrambled_f);

    get_time_interval(t);
    printf("Texec=%ld us for %d bits\n", t[0].tv_usec, seq.cur_len);

    for (i = 0; i < seq.cur_len; i++) {
      if (scrambled_f[i] != input_f[i]) {
        printf("Error in %d\n", i);
        exit(-1);
      }
    }

    free(input_f);
    free(scrambled_f);

    int16_t *input_s, *scrambled_s;

    // Scramble also shorts
    input_s = isrran_vec_i16_malloc(seq.cur_len);
    if (!input_s) {
      perror("malloc");
      exit(-1);
    }
    scrambled_s = isrran_vec_i16_malloc(seq.cur_len);
    if (!scrambled_s) {
      perror("malloc");
      exit(-1);
    }

    for (i = 0; i < seq.cur_len; i++) {
      input_s[i]     = 100 * (rand() / RAND_MAX) - 50;
      scrambled_s[i] = input_s[i];
    }

    gettimeofday(&t[1], NULL);
    isrran_scrambling_s(&seq, scrambled_s);
    gettimeofday(&t[2], NULL);
    isrran_scrambling_s(&seq, scrambled_s);

    get_time_interval(t);
    printf("Texec short=%ld us for %d bits\n", t[0].tv_usec, seq.cur_len);

    for (i = 0; i < seq.cur_len; i++) {
      if (scrambled_s[i] != input_s[i]) {
        printf("Error in %d\n", i);
        exit(-1);
      }
    }

    free(input_s);
    free(scrambled_s);

    int8_t *input_b, *scrambled_b;

    // Scramble also bytes
    input_b = isrran_vec_i8_malloc(seq.cur_len);
    if (!input_b) {
      perror("malloc");
      exit(-1);
    }
    scrambled_b = isrran_vec_i8_malloc(seq.cur_len);
    if (!scrambled_b) {
      perror("malloc");
      exit(-1);
    }

    for (i = 0; i < seq.cur_len; i++) {
      input_b[i]     = 100 * (rand() / RAND_MAX) - 50;
      scrambled_b[i] = input_b[i];
    }

    gettimeofday(&t[1], NULL);
    isrran_scrambling_sb_offset(&seq, scrambled_b, 0, seq.cur_len);
    gettimeofday(&t[2], NULL);
    isrran_scrambling_sb_offset(&seq, scrambled_b, 0, seq.cur_len);

    get_time_interval(t);
    printf("Texec char=%ld us for %d bits\n", t[0].tv_usec, seq.cur_len);

    for (i = 0; i < seq.cur_len; i++) {
      if (scrambled_b[i] != input_b[i]) {
        printf("Error in %d\n", i);
        exit(-1);
      }
    }

    free(input_b);
    free(scrambled_b);
  }
  printf("Ok\n");
  isrran_sequence_free(&seq);
  exit(0);
}
