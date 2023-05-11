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

#include "isrran/phy/phch/pscch.h"
#include "isrran/phy/phch/sci.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

isrran_cell_sl_t cell = {.nof_prb = 6, .N_sl_id = 168, .tm = ISRRAN_SIDELINK_TM2, .cp = ISRRAN_CP_NORM};

uint32_t prb_start_idx = 0;

void usage(char* prog)
{
  printf("Usage: %s [cdeipt]\n", prog);
  printf("\t-p nof_prb [Default %d]\n", cell.nof_prb);
  printf("\t-c N_sl_id [Default %d]\n", cell.N_sl_id);
  printf("\t-t Sidelink transmission mode {1,2,3,4} [Default %d]\n", (cell.tm + 1));
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "ceiptv")) != -1) {
    switch (opt) {
      case 'c':
        cell.N_sl_id = (int32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'p':
        cell.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
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
            break;
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
  int ret = ISRRAN_ERROR;

  parse_args(argc, argv);

  isrran_sl_comm_resource_pool_t sl_comm_resource_pool;
  if (isrran_sl_comm_resource_pool_get_default_config(&sl_comm_resource_pool, cell) != ISRRAN_SUCCESS) {
    ERROR("Error initializing sl_comm_resource_pool");
    return ISRRAN_ERROR;
  }

  char sci_msg[ISRRAN_SCI_MSG_MAX_LEN] = {};

  uint32_t sf_n_re   = ISRRAN_SF_LEN_RE(cell.nof_prb, cell.cp);
  cf_t*    sf_buffer = isrran_vec_cf_malloc(sf_n_re);

  // SCI
  isrran_sci_t sci;
  isrran_sci_init(&sci, &cell, &sl_comm_resource_pool);
  sci.mcs_idx = 2;

  // PSCCH
  isrran_pscch_t pscch;
  if (isrran_pscch_init(&pscch, ISRRAN_MAX_PRB) != ISRRAN_SUCCESS) {
    ERROR("Error in PSCCH init");
    return ISRRAN_ERROR;
  }

  if (isrran_pscch_set_cell(&pscch, cell) != ISRRAN_SUCCESS) {
    ERROR("Error in PSCCH init");
    return ISRRAN_ERROR;
  }

  // SCI message bits
  uint8_t sci_tx[ISRRAN_SCI_MAX_LEN] = {};
  if (sci.format == ISRRAN_SCI_FORMAT0) {
    if (isrran_sci_format0_pack(&sci, sci_tx) != ISRRAN_SUCCESS) {
      printf("Error packing sci format 0\n");
      return ISRRAN_ERROR;
    }
  } else if (sci.format == ISRRAN_SCI_FORMAT1) {
    if (isrran_sci_format1_pack(&sci, sci_tx) != ISRRAN_SUCCESS) {
      printf("Error packing sci format 1\n");
      return ISRRAN_ERROR;
    }
  }

  printf("Tx payload: ");
  isrran_vec_fprint_hex(stdout, sci_tx, sci.sci_len);

  // Put SCI into PSCCH
  isrran_pscch_encode(&pscch, sci_tx, sf_buffer, prb_start_idx);

  // Prepare Rx buffer
  uint8_t sci_rx[ISRRAN_SCI_MAX_LEN] = {};

  // Decode PSCCH
  if (isrran_pscch_decode(&pscch, sf_buffer, sci_rx, prb_start_idx) == ISRRAN_SUCCESS) {
    printf("Rx payload: ");
    isrran_vec_fprint_hex(stdout, sci_rx, sci.sci_len);

    uint32_t riv_txed = sci.riv;
    if (sci.format == ISRRAN_SCI_FORMAT0) {
      if (isrran_sci_format0_unpack(&sci, sci_rx) != ISRRAN_SUCCESS) {
        printf("Error unpacking sci format 0\n");
        return ISRRAN_ERROR;
      }
    } else if (sci.format == ISRRAN_SCI_FORMAT1) {
      if (isrran_sci_format1_unpack(&sci, sci_rx) != ISRRAN_SUCCESS) {
        printf("Error unpacking sci format 1\n");
        return ISRRAN_ERROR;
      }
    }

    isrran_sci_info(&sci, sci_msg, sizeof(sci_msg));
    fprintf(stdout, "%s\n", sci_msg);
    if (sci.riv == riv_txed) {
      ret = ISRRAN_SUCCESS;
    }
  }

  free(sf_buffer);
  isrran_sci_free(&sci);
  isrran_pscch_free(&pscch);

  return ret;
}
