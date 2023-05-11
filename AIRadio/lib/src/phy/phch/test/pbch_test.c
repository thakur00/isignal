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
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "isrran/isrran.h"

isrran_cell_t cell = {
    6,                 // nof_prb
    1,                 // nof_ports
    1,                 // cell_id
    ISRRAN_CP_NORM,    // cyclic prefix
    ISRRAN_PHICH_NORM, // PHICH length
    ISRRAN_PHICH_R_1,  // PHICH resources
    ISRRAN_FDD,

};

void usage(char* prog)
{
  printf("Usage: %s [cpv]\n", prog);
  printf("\t-c cell id [Default %d]\n", cell.id);
  printf("\t-p cell.nof_ports [Default %d]\n", cell.nof_ports);
  printf("\t-n cell.nof_prb [Default %d]\n", cell.nof_prb);
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "cpnv")) != -1) {
    switch (opt) {
      case 'p':
        cell.nof_ports = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'n':
        cell.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'c':
        cell.id = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'v':
        increase_isrran_verbose_level();
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
}

int main(int argc, char** argv)
{
  isrran_chest_dl_res_t chest_dl_res;
  isrran_pbch_t         pbch;
  uint8_t               bch_payload_tx[ISRRAN_BCH_PAYLOAD_LEN], bch_payload_rx[ISRRAN_BCH_PAYLOAD_LEN];
  int                   i, j, k;
  int                   nof_re;
  cf_t*                 sf_symbols[ISRRAN_MAX_PORTS];
  uint32_t              nof_rx_ports;
  isrran_random_t       random_gen = isrran_random_init(0x1234);

  parse_args(argc, argv);

  nof_re = ISRRAN_SF_LEN_RE(cell.nof_prb, ISRRAN_CP_NORM);

  /* init memory */
  isrran_chest_dl_res_init(&chest_dl_res, cell.nof_prb);
  for (i = 0; i < cell.nof_ports; i++) {
    for (j = 0; j < cell.nof_ports; j++) {
      for (k = 0; k < nof_re; k++) {
        chest_dl_res.ce[i][j][k] = 1;
      }
    }
    sf_symbols[i] = isrran_vec_cf_malloc(nof_re);
    if (!sf_symbols[i]) {
      perror("malloc");
      exit(-1);
    }
  }
  if (isrran_pbch_init(&pbch)) {
    ERROR("Error creating PBCH object");
    exit(-1);
  }
  if (isrran_pbch_set_cell(&pbch, cell)) {
    ERROR("Error creating PBCH object");
    exit(-1);
  }

  for (i = 0; i < ISRRAN_BCH_PAYLOAD_LEN; i++) {
    bch_payload_tx[i] = (uint8_t)isrran_random_uniform_int_dist(random_gen, 0, 1);
  }

  isrran_pbch_encode(&pbch, bch_payload_tx, sf_symbols, 0);

  /* combine outputs */
  for (i = 1; i < cell.nof_ports; i++) {
    for (j = 0; j < nof_re; j++) {
      sf_symbols[0][j] += sf_symbols[i][j];
    }
  }

  isrran_pbch_decode_reset(&pbch);
  if (1 != isrran_pbch_decode(&pbch, &chest_dl_res, sf_symbols, bch_payload_rx, &nof_rx_ports, NULL)) {
    printf("Error decoding\n");
    exit(-1);
  }

  isrran_pbch_free(&pbch);

  for (i = 0; i < cell.nof_ports; i++) {
    free(sf_symbols[i]);
  }

  isrran_chest_dl_res_free(&chest_dl_res);
  isrran_random_free(random_gen);

  printf("Tx ports: %d - Rx ports: %d\n", cell.nof_ports, nof_rx_ports);
  printf("Tx payload: ");
  isrran_vec_fprint_hex(stdout, bch_payload_tx, ISRRAN_BCH_PAYLOAD_LEN);
  printf("Rx payload: ");
  isrran_vec_fprint_hex(stdout, bch_payload_rx, ISRRAN_BCH_PAYLOAD_LEN);

  if (nof_rx_ports == cell.nof_ports &&
      !memcmp(bch_payload_rx, bch_payload_tx, sizeof(uint8_t) * ISRRAN_BCH_PAYLOAD_LEN)) {
    printf("OK\n");
    exit(0);
  } else {
    printf("Error\n");
    exit(-1);
  }
}
