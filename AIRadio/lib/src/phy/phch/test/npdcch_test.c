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
#include <unistd.h>

#include "isrran/phy/phch/dci_nbiot.h"
#include "isrran/phy/phch/npdcch.h"
#include "isrran/phy/phch/ra_nbiot.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

#define RNTI (0x1234)
#define HAVE_OFDM 0

isrran_dci_format_t dci_format = ISRRAN_DCI_FORMATN0;

isrran_nbiot_cell_t cell = {.base       = {.nof_prb = 1, .nof_ports = 1, .cp = ISRRAN_CP_NORM, .id = 0},
                            .nbiot_prb  = 0,
                            .n_id_ncell = 0,
                            .nof_ports  = 1,
                            .mode       = ISRRAN_NBIOT_MODE_STANDALONE};

void usage(char* prog)
{
  printf("Usage: %s [cpndv]\n", prog);
  printf("\t-c cell id [Default %d]\n", cell.base.id);
  printf("\t-o DCI Format [Default %s]\n", isrran_dci_format_string(dci_format));
  printf("\t-p cell.nof_ports [Default %d]\n", cell.base.nof_ports);
  printf("\t-n cell.nof_prb [Default %d]\n", cell.base.nof_prb);
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "cpnov")) != -1) {
    switch (opt) {
      case 'p':
        cell.base.nof_ports = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'n':
        cell.base.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'c':
        cell.base.id = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'o':
        dci_format = isrran_dci_format_from_string(argv[optind]);
        if (dci_format == ISRRAN_DCI_NOF_FORMATS) {
          ERROR("Error unsupported format %s", argv[optind]);
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
}

int main(int argc, char** argv)
{
  isrran_npdcch_t       npdcch = {};
  isrran_dci_msg_t      dci_tx = {}, dci_rx = {};
  isrran_dci_location_t dci_location = {};

  uint32_t tti  = 6521;
  uint16_t rnti = 0x1234;

  cf_t*                    ce[ISRRAN_MAX_PORTS]           = {NULL};
  cf_t*                    slot_symbols[ISRRAN_MAX_PORTS] = {NULL};
  int                      ret                            = ISRRAN_ERROR;

  parse_args(argc, argv);

  // we need to allocate RE's for a full 6 PRB cell
  int nof_re = 6 * ISRRAN_SF_LEN_RE(cell.base.nof_prb, cell.base.cp);

  // init memory
  for (int i = 0; i < ISRRAN_MAX_PORTS; i++) {
    ce[i] = isrran_vec_cf_malloc(nof_re);
    if (!ce[i]) {
      perror("malloc");
      exit(-1);
    }
    for (int j = 0; j < nof_re; j++) {
      ce[i][j] = 1;
    }
    slot_symbols[i] = isrran_vec_cf_malloc(nof_re);
    if (!slot_symbols[i]) {
      perror("malloc");
      exit(-1);
    }
  }

#if HAVE_OFDM
  cf_t td_signal[nof_re * 2];
#endif

  if (isrran_npdcch_init(&npdcch)) {
    fprintf(stderr, "Error creating NPDCCH object\n");
    exit(-1);
  }

  if (isrran_npdcch_set_cell(&npdcch, cell)) {
    fprintf(stderr, "Error configuring NPDCCH object\n");
    exit(-1);
  }

  if (dci_format == ISRRAN_DCI_FORMATN0) {
    // UL grant
    isrran_ra_nbiot_ul_dci_t dci = {};
    dci.format                   = dci_format;

    isrran_ra_nbiot_ul_grant_t grant;
    if (isrran_ra_nbiot_ul_dci_to_grant(&dci, &grant, tti, ISRRAN_NPUSCH_SC_SPACING_15000)) {
      fprintf(stderr, "Error converting DCI message\n");
    }

    fprintf(stderr, "FormatN0 packing not supported\n");
    return ISRRAN_ERROR;
  } else if (dci_format == ISRRAN_DCI_FORMATN1) {
    // DL grant
    isrran_ra_nbiot_dl_dci_t ra_dl = {};
    ra_dl.mcs_idx                  = 5;
    ra_dl.ndi                      = 0;
    ra_dl.rv_idx                   = 0;

    // NB-IoT specific fields
    ra_dl.alloc.has_sib1  = false;
    ra_dl.alloc.is_ra     = false;
    ra_dl.alloc.i_delay   = 4;
    ra_dl.alloc.i_sf      = 0;
    ra_dl.alloc.i_rep     = 0;
    ra_dl.alloc.harq_ack  = 1;
    ra_dl.alloc.i_n_start = 0;
    isrran_nbiot_dl_dci_fprint(stdout, &ra_dl);

    // pack DCI
    isrran_dci_msg_pack_npdsch(&ra_dl, dci_format, &dci_tx, false);
    isrran_dci_location_set(&dci_location, 2, 0);
  } else {
    fprintf(stderr, "FormatN2 packing not supported\n");
    return ISRRAN_ERROR;
  }

  if (isrran_npdcch_encode(&npdcch, &dci_tx, dci_location, RNTI, slot_symbols, 0)) {
    fprintf(stderr, "Error encoding DCI message\n");
    goto quit;
  }

  // combine outputs
  for (int i = 1; i < cell.base.nof_ports; i++) {
    for (int j = 0; j < nof_re; j++) {
      slot_symbols[0][j] += slot_symbols[i][j];
    }
  }

#if HAVE_OFDM
  isrran_ofdm_t ofdm_tx;
  isrran_ofdm_t ofdm_rx;

  if (isrran_ofdm_tx_init(&ofdm_tx, ISRRAN_CP_NORM, slot_symbols[0], td_signal, cell.base.nof_prb)) {
    fprintf(stderr, "Error creating iFFT object\n");
    exit(-1);
  }
  // isrran_ofdm_set_normalize(&ofdm_tx, true);

  if (isrran_ofdm_rx_init(&ofdm_rx, ISRRAN_CP_NORM, td_signal, slot_symbols[0], cell.base.nof_prb)) {
    fprintf(stderr, "Error initializing FFT\n");
    return -1;
  }

  // transfer into time doamin and back
  isrran_ofdm_tx_sf(&ofdm_tx);
  isrran_ofdm_rx_sf(&ofdm_rx);

  isrran_ofdm_tx_free(&ofdm_tx);
  isrran_ofdm_rx_free(&ofdm_rx);
#endif

  if (isrran_npdcch_extract_llr(&npdcch, slot_symbols[0], ce, 0, 0)) {
    fprintf(stderr, "Error extracting LLRs\n");
    goto quit;
  }
  uint16_t crc_rem = 0;
  if (isrran_npdcch_decode_msg(&npdcch, &dci_rx, &dci_location, dci_format, &crc_rem)) {
    fprintf(stderr, "Error decoding DCI message\n");
    goto quit;
  }
  if (crc_rem != RNTI) {
    printf("Received invalid DCI CRC 0x%x\n", crc_rem);
    goto quit;
  }

  // compare DCIs
  if (memcmp(dci_tx.payload, dci_rx.payload, dci_tx.nof_bits)) {
    printf("Error in DCI: Received data does not match\n");
    goto quit;
  }

  if (dci_format == ISRRAN_DCI_FORMATN0) {
    // UL grant
    // ..
  } else {
    // DL grant
    isrran_ra_nbiot_dl_dci_t   dci_unpacked;
    isrran_ra_nbiot_dl_grant_t grant;
    if (isrran_nbiot_dci_msg_to_dl_grant(
            &dci_rx, rnti, &dci_unpacked, &grant, tti / 10, tti % 10, 64 /* todo: remove */, cell.mode)) {
      fprintf(stderr, "Error unpacking DCI\n");
      return ISRRAN_ERROR;
    }
    isrran_nbiot_dl_dci_fprint(stdout, &dci_unpacked);
  }

  ret = ISRRAN_SUCCESS;

quit:
  isrran_npdcch_free(&npdcch);

  for (int i = 0; i < ISRRAN_MAX_PORTS; i++) {
    if (ce[i]) {
      free(ce[i]);
    }
    if (slot_symbols[i]) {
      free(slot_symbols[i]);
    }
  }
  if (ret) {
    printf("Error\n");
  } else {
    printf("Ok\n");
  }
  exit(ret);
}
