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

#include "isrran/isrran.h"

char* input_file_name = NULL;

isrran_cell_t cell = {
    6,                 // cell.cell.cell.nof_prb
    1,                 // cell.cell.nof_ports
    0,                 // cell.id
    ISRRAN_CP_NORM,    // cyclic prefix
    ISRRAN_PHICH_NORM, // PHICH length
    ISRRAN_PHICH_R_1,  // PHICH resources
    ISRRAN_FDD,

};

uint32_t cfi = 2;
int      flen;
uint16_t rnti       = ISRRAN_SIRNTI;
int      max_frames = 10;

isrran_dci_format_t   dci_format = ISRRAN_DCI_FORMAT1A;
isrran_filesource_t   fsrc;
isrran_pdcch_t        pdcch;
cf_t *                input_buffer, *fft_buffer[ISRRAN_MAX_PORTS];
isrran_regs_t         regs;
isrran_ofdm_t         fft;
isrran_chest_dl_t     chest;
isrran_chest_dl_res_t chest_res;

void usage(char* prog)
{
  printf("Usage: %s [vcfoe] -i input_file\n", prog);
  printf("\t-c cell.id [Default %d]\n", cell.id);
  printf("\t-f cfi [Default %d]\n", cfi);
  printf("\t-o DCI Format [Default %s]\n", isrran_dci_format_string(dci_format));
  printf("\t-r rnti [Default SI-RNTI]\n");
  printf("\t-p cell.nof_ports [Default %d]\n", cell.nof_ports);
  printf("\t-n cell.nof_prb [Default %d]\n", cell.nof_prb);
  printf("\t-m max_frames [Default %d]\n", max_frames);
  printf("\t-e Set extended prefix [Default Normal]\n");
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "irvofcenmp")) != -1) {
    switch (opt) {
      case 'i':
        input_file_name = argv[optind];
        break;
      case 'c':
        cell.id = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'r':
        rnti = strtoul(argv[optind], NULL, 0);
        break;
      case 'm':
        max_frames = strtoul(argv[optind], NULL, 0);
        break;
      case 'f':
        cfi = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'n':
        cell.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'p':
        cell.nof_ports = (uint32_t)strtol(argv[optind], NULL, 10);
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
      case 'e':
        cell.cp = ISRRAN_CP_EXT;
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
  if (!input_file_name) {
    usage(argv[0]);
    exit(-1);
  }
}

int base_init()
{
  if (isrran_filesource_init(&fsrc, input_file_name, ISRRAN_COMPLEX_FLOAT_BIN)) {
    ERROR("Error opening file %s", input_file_name);
    exit(-1);
  }

  flen = 2 * (ISRRAN_SLOT_LEN(isrran_symbol_sz(cell.nof_prb)));

  input_buffer = isrran_vec_cf_malloc(flen);
  if (!input_buffer) {
    perror("malloc");
    exit(-1);
  }

  fft_buffer[0] = isrran_vec_cf_malloc(ISRRAN_NOF_RE(cell));
  if (!fft_buffer[0]) {
    perror("malloc");
    return -1;
  }

  if (isrran_chest_dl_init(&chest, cell.nof_prb, 1)) {
    ERROR("Error initializing equalizer");
    return -1;
  }
  if (isrran_chest_dl_res_init(&chest_res, cell.nof_prb)) {
    ERROR("Error initializing equalizer");
    return -1;
  }
  if (isrran_chest_dl_set_cell(&chest, cell)) {
    ERROR("Error initializing equalizer");
    return -1;
  }

  if (isrran_ofdm_rx_init(&fft, cell.cp, input_buffer, fft_buffer[0], cell.nof_prb)) {
    ERROR("Error initializing FFT");
    return -1;
  }

  if (isrran_regs_init(&regs, cell)) {
    ERROR("Error initiating regs");
    return -1;
  }

  if (isrran_pdcch_init_ue(&pdcch, cell.nof_prb, 1)) {
    ERROR("Error creating PDCCH object");
    exit(-1);
  }
  if (isrran_pdcch_set_cell(&pdcch, &regs, cell)) {
    ERROR("Error creating PDCCH object");
    exit(-1);
  }

  DEBUG("Memory init OK");
  return 0;
}

void base_free()
{
  isrran_filesource_free(&fsrc);

  free(input_buffer);
  free(fft_buffer[0]);

  isrran_filesource_free(&fsrc);

  isrran_chest_dl_res_free(&chest_res);
  isrran_chest_dl_free(&chest);
  isrran_ofdm_rx_free(&fft);

  isrran_pdcch_free(&pdcch);
  isrran_regs_free(&regs);
}

int main(int argc, char** argv)
{
  int                   i;
  int                   frame_cnt;
  int                   ret;
  isrran_dci_location_t locations[ISRRAN_MAX_CANDIDATES];
  uint32_t              nof_locations;
  isrran_dci_msg_t      dci_msg;

  if (argc < 3) {
    usage(argv[0]);
    exit(-1);
  }

  parse_args(argc, argv);

  if (base_init()) {
    ERROR("Error initializing memory");
    exit(-1);
  }

  ret       = -1;
  frame_cnt = 0;
  do {
    isrran_filesource_read(&fsrc, input_buffer, flen);

    INFO("Reading %d samples sub-frame %d", flen, frame_cnt);

    isrran_ofdm_rx_sf(&fft);

    isrran_dl_sf_cfg_t dl_sf;
    ZERO_OBJECT(dl_sf);
    dl_sf.tti = frame_cnt;
    dl_sf.cfi = cfi;

    /* Get channel estimates for each port */
    isrran_chest_dl_estimate(&chest, &dl_sf, fft_buffer, &chest_res);

    if (isrran_pdcch_extract_llr(&pdcch, &dl_sf, &chest_res, fft_buffer)) {
      ERROR("Error extracting LLRs");
      return -1;
    }
    if (rnti == ISRRAN_SIRNTI) {
      INFO("Initializing common search space for SI-RNTI");
      nof_locations = isrran_pdcch_common_locations(&pdcch, locations, ISRRAN_MAX_CANDIDATES, cfi);
    } else {
      INFO("Initializing user-specific search space for RNTI: 0x%x", rnti);
      nof_locations = isrran_pdcch_ue_locations(&pdcch, &dl_sf, locations, ISRRAN_MAX_CANDIDATES, rnti);
    }

    isrran_dci_cfg_t dci_cfg;
    ZERO_OBJECT(dci_cfg);

    ZERO_OBJECT(dci_msg);

    for (i = 0; i < nof_locations && dci_msg.rnti != rnti; i++) {
      dci_msg.location = locations[i];
      dci_msg.format   = dci_format;
      if (isrran_pdcch_decode_msg(&pdcch, &dl_sf, &dci_cfg, &dci_msg)) {
        ERROR("Error decoding DCI msg");
        return -1;
      }
    }

    if (dci_msg.rnti == rnti) {
      isrran_dci_dl_t dci;
      bzero(&dci, sizeof(isrran_dci_dl_t));
      if (isrran_dci_msg_unpack_pdsch(&cell, &dl_sf, &dci_cfg, &dci_msg, &dci)) {
        ERROR("Can't unpack DCI message");
      } else {
        if (dci.alloc_type == ISRRAN_RA_ALLOC_TYPE2 && dci.type2_alloc.mode == ISRRAN_RA_TYPE2_LOC &&
            dci.type2_alloc.riv == 11 && dci.tb[0].rv == 0 && dci.pid == 0 && dci.tb[0].mcs_idx == 2) {
          printf("This is the file signal.1.92M.amar.dat\n");
          ret = 0;
        }
      }
    }

    frame_cnt++;
  } while (frame_cnt <= max_frames);

  base_free();
  exit(ret);
}
