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

#include "isrran/phy/phch/pbch_msg_nr.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/random.h"
#include "isrran/support/isrran_test.h"
#include <getopt.h>
#include <string.h>

static uint32_t        nof_repetitions = 16;
static isrran_random_t random_gen      = NULL;

static int test_packing_unpacking()
{
  for (uint32_t r = 0; r < nof_repetitions; r++) {
    isrran_mib_nr_t mib = {};
    mib.sfn             = isrran_random_uniform_int_dist(random_gen, 0, 1023);
    mib.ssb_idx         = isrran_random_uniform_int_dist(random_gen, 0, 127);
    mib.hrf             = isrran_random_bool(random_gen, 0.5f);
    mib.scs_common =
        isrran_random_bool(random_gen, 0.5f) ? isrran_subcarrier_spacing_15kHz : isrran_subcarrier_spacing_30kHz;
    mib.ssb_offset = isrran_random_uniform_int_dist(random_gen, 0, 31);
    mib.dmrs_typeA_pos =
        isrran_random_bool(random_gen, 0.5f) ? isrran_dmrs_sch_typeA_pos_2 : isrran_dmrs_sch_typeA_pos_3;
    mib.coreset0_idx           = isrran_random_uniform_int_dist(random_gen, 0, 15);
    mib.ss0_idx                = isrran_random_uniform_int_dist(random_gen, 0, 15);
    mib.cell_barred            = isrran_random_bool(random_gen, 0.5f);
    mib.intra_freq_reselection = isrran_random_bool(random_gen, 0.5f);
    mib.spare                  = isrran_random_uniform_int_dist(random_gen, 0, 1);

    isrran_pbch_msg_nr_t pbch_msg = {};
    TESTASSERT(isrran_pbch_msg_nr_mib_pack(&mib, &pbch_msg) == ISRRAN_SUCCESS);

    TESTASSERT(isrran_pbch_msg_nr_is_mib(&pbch_msg));

    isrran_mib_nr_t mib2 = {};
    TESTASSERT(isrran_pbch_msg_nr_mib_unpack(&pbch_msg, &mib2) == ISRRAN_SUCCESS);

    char str1[256];
    char str2[256];
    char strp[256];
    isrran_pbch_msg_nr_mib_info(&mib, str1, (uint32_t)sizeof(str1));
    isrran_pbch_msg_nr_mib_info(&mib2, str2, (uint32_t)sizeof(str2));
    isrran_pbch_msg_info(&pbch_msg, strp, (uint32_t)sizeof(strp));

    if (memcmp(&mib, &mib2, sizeof(isrran_mib_nr_t)) != 0) {
      ERROR("Failed packing/unpacking MIB");
      printf("  Source: %s\n", str1);
      printf("Unpacked: %s\n", str2);
      printf("  Packed: %s\n", strp);
      return ISRRAN_ERROR;
    }
  }

  return ISRRAN_SUCCESS;
}

static void usage(char* prog)
{
  printf("Usage: %s [cpndv]\n", prog);
  printf("\t-v Increase verbose [default none]\n");
  printf("\t-R Set number of Packing/Unpacking [default %d]\n", nof_repetitions);
}

static void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "vR")) != -1) {
    switch (opt) {
      case 'v':
        increase_isrran_verbose_level();
        break;
      case 'R':
        nof_repetitions = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
}

int main(int argc, char** argv)
{
  parse_args(argc, argv);

  int ret    = ISRRAN_ERROR;
  random_gen = isrran_random_init(1234);

  if (test_packing_unpacking() < ISRRAN_SUCCESS) {
    goto clean_exit;
  }

  ret = ISRRAN_SUCCESS;

clean_exit:
  isrran_random_free(random_gen);
  return ret;
}
