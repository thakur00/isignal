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

#include "isrran/phy/ue/ue_dl_nbiot.h"

isrran_nbiot_cell_t cell = {
    .base       = {.nof_prb = 1, .nof_ports = 1, .cp = ISRRAN_CP_NORM, .id = 0},
    .nbiot_prb  = 0,
    .nof_ports  = 1,
    .n_id_ncell = 0,
};

void usage(char* prog)
{
  printf("Usage: %s [v]\n", prog);
  printf("\t-l n_id_ncell to sync [Default %d]\n", cell.n_id_ncell);
  printf("\t-v isrran_verbose\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "lv")) != -1) {
    switch (opt) {
      case 'l':
        cell.n_id_ncell = (uint32_t)strtol(argv[optind], NULL, 10);
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

int dl_grant_with_sib1_test()
{
  int   ret = ISRRAN_ERROR;
  cf_t  rx_buff[ISRRAN_SF_LEN_PRB_NBIOT];
  cf_t* buff_ptrs[ISRRAN_MAX_PORTS] = {rx_buff, NULL, NULL, NULL};

  isrran_nbiot_ue_dl_t ue_dl;
  isrran_nbiot_ue_dl_init(&ue_dl, buff_ptrs, ISRRAN_NBIOT_MAX_PRB, ISRRAN_NBIOT_NUM_RX_ANTENNAS);
  if (isrran_nbiot_ue_dl_set_cell(&ue_dl, cell)) {
    fprintf(stderr, "Setting cell in UE DL\n");
    return ret;
  }

  // pass MIB to compute SIB1 parameters
  isrran_mib_nb_t mib = {};
  mib.sched_info_sib1 = 3;
  // isrran_mib_nb_printf(stdout, cell, &mib);
  isrran_nbiot_ue_dl_set_mib(&ue_dl, mib);

  // setup grant who's start collides with a SIB1 transmission
  isrran_ra_nbiot_dl_grant_t grant;
  isrran_nbiot_ue_dl_get_sib1_grant(&ue_dl, 0, &grant);

  // call helper to fix grant
  isrran_nbiot_ue_dl_check_grant(&ue_dl, &grant);

  // grant must not start on sf_idx 4
  if (grant.start_sfidx != 4) {
    ret = ISRRAN_SUCCESS;
  }

  isrran_nbiot_ue_dl_free(&ue_dl);

  return ret;
}

int dl_grant_with_sib2_test()
{
  int   ret = ISRRAN_ERROR;
  cf_t  rx_buff[ISRRAN_SF_LEN_PRB_NBIOT];
  cf_t* buff_ptrs[ISRRAN_MAX_PORTS] = {rx_buff, NULL, NULL, NULL};

  isrran_nbiot_ue_dl_t ue_dl;
  isrran_nbiot_ue_dl_init(&ue_dl, buff_ptrs, ISRRAN_NBIOT_MAX_PRB, ISRRAN_NBIOT_NUM_RX_ANTENNAS);
  if (isrran_nbiot_ue_dl_set_cell(&ue_dl, cell)) {
    fprintf(stderr, "Setting cell in UE DL\n");
    return ret;
  }

  // pass MIB to compute SIB1 parameters
  isrran_mib_nb_t mib = {};
  mib.sched_info_sib1 = 3;
  isrran_nbiot_ue_dl_set_mib(&ue_dl, mib);

  // configure SIB2-NB parameters
  isrran_nbiot_si_params_t sib2_params;
  sib2_params.n                     = 1;
  sib2_params.si_periodicity        = 128;
  sib2_params.si_radio_frame_offset = 0;
  sib2_params.si_repetition_pattern = 2; // Every 2nd radio frame
  sib2_params.si_tb                 = 208;
  sib2_params.si_window_length      = 160;
  isrran_nbiot_ue_dl_set_si_params(&ue_dl, ISRRAN_NBIOT_SI_TYPE_SIB2, sib2_params);

  int dummy_tti = 0;
  isrran_nbiot_ue_dl_decode_sib(&ue_dl, dummy_tti, dummy_tti, ISRRAN_NBIOT_SI_TYPE_SIB2, sib2_params);

  // setup grant who's start collides with the above configured SI transmission
  isrran_ra_nbiot_dl_grant_t grant;
  grant.start_sfn   = 898;
  grant.start_sfidx = 1;

  // UE DL object should automatically fix the grant
  isrran_nbiot_ue_dl_check_grant(&ue_dl, &grant);

  if (grant.start_sfn == 899 && grant.start_sfidx == 3) {
    ret = ISRRAN_SUCCESS;
  }

  isrran_nbiot_ue_dl_free(&ue_dl);

  return ret;
}

int main(int argc, char** argv)
{
  parse_args(argc, argv);

  if (dl_grant_with_sib1_test()) {
    printf("DL grant with SIB1 test failed\n");
    return ISRRAN_ERROR;
  }

  if (dl_grant_with_sib2_test()) {
    printf("DL grant with SIB2 test failed\n");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}
