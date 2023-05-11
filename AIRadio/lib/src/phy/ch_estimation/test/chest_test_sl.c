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

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "isrran/phy/ch_estimation/chest_sl.h"
#include "isrran/phy/common/phy_common_sl.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

isrran_cell_sl_t cell           = {.nof_prb = 6, .N_sl_id = 168, .tm = ISRRAN_SIDELINK_TM2, .cp = ISRRAN_CP_NORM};
bool             run_psbch_test = true;

void usage(char* prog)
{
  printf("Usage: %s [recov]\n", prog);
  printf("\t-p nof_prb [Default %d]\n", cell.nof_prb);
  printf("\t-e extended cyclic prefix [Default normal]\n");
  printf("\t-c N_sl_id [Default %d]\n", cell.N_sl_id);
  printf("\t-t Sidelink transmission mode {1,2,3,4} [Default %d]\n", (cell.tm + 1));
  printf("\t-v increase verbosity\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "pecotv")) != -1) {
    switch (opt) {
      case 'p':
        cell.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'e':
        cell.cp = ISRRAN_CP_EXT;
        break;
      case 'c':
        cell.N_sl_id = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 't':
        switch (strtol(argv[optind], NULL, 10)) {
          case 1:
            cell.tm = ISRRAN_SIDELINK_TM1;
            break;
          case 2:
            cell.tm = ISRRAN_SIDELINK_TM2;
            break;
          case 3:
            cell.tm = ISRRAN_SIDELINK_TM3;
            break;
          case 4:
            cell.tm = ISRRAN_SIDELINK_TM4;
            break;
          default:
            usage(argv[0]);
            exit(-1);
        }
        break;
      case 'v':
        increase_isrran_verbose_level();
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
  if (cell.cp == ISRRAN_CP_EXT && cell.tm >= ISRRAN_SIDELINK_TM3) {
    ERROR("Selected TM does not support extended CP");
    usage(argv[0]);
    exit(-1);
  }
}

int main(int argc, char** argv)
{
  parse_args(argc, argv);

  int   sf_n_re   = ISRRAN_SF_LEN_RE(cell.nof_prb, cell.cp);
  cf_t* sf_buffer = isrran_vec_cf_malloc(sf_n_re);
  bzero(sf_buffer, sizeof(cf_t) * sf_n_re);

  // Variables init Rx
  cf_t* equalized_sf_buffer = isrran_vec_cf_malloc(sf_n_re);
  bzero(equalized_sf_buffer, sizeof(cf_t) * sf_n_re);

  cf_t* dmrs_received[ISRRAN_SL_MAX_DMRS_SYMB] = {NULL};
  for (int i = 0; i < ISRRAN_SL_MAX_DMRS_SYMB; i++) {
    dmrs_received[i] = isrran_vec_cf_malloc(ISRRAN_NRE * cell.nof_prb);
  }

  isrran_sl_comm_resource_pool_t sl_comm_resource_pool = {};

  // Variables init Tx
  isrran_chest_sl_t q = {};

  if (run_psbch_test) {
    // Tx
    isrran_chest_sl_init(&q, ISRRAN_SIDELINK_PSBCH, cell, &sl_comm_resource_pool);
    isrran_chest_sl_put_dmrs(&q, sf_buffer);

    // Rx
    isrran_chest_sl_ls_estimate_equalize(&q, sf_buffer, equalized_sf_buffer);
    isrran_chest_sl_get_dmrs(&q, equalized_sf_buffer, dmrs_received);

    // Test
    // TODO: add proper test
  }

  // Print of received DMRS
  printf("\n\nPrint of received DMRS\n");
  for (int i = 0; i < q.M_sc_rs; i++) {
    printf("%i\t", i + 1);
    for (int j = 0; j < q.nof_dmrs_symbols; j++) {
      printf("%f\t%f\t", __real__ dmrs_received[j][i], __imag__ dmrs_received[j][i]);
    }
    printf("\n");
  }

  isrran_chest_sl_free(&q);

  if (sf_buffer) {
    free(sf_buffer);
  }
  if (equalized_sf_buffer) {
    free(equalized_sf_buffer);
  }
  for (int i = 0; i < ISRRAN_SL_MAX_DMRS_SYMB; i++) {
    if (dmrs_received[i]) {
      free(dmrs_received[i]);
    }
  }

  return ISRRAN_SUCCESS;
}