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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>

#include <unistd.h>

#include "isrran/isrran.h"

#include "isrran/common/crash_handler.h"
#include "isrran/phy/rf/rf_utils.h"

#ifndef DISABLE_RF
#include "isrran/phy/rf/rf.h"
#endif

#define MHZ 1000000
#define SAMP_FREQ 1920000
#define FLEN 9600
#define FLEN_PERIOD 0.005

#define MAX_EARFCN 1000

int band         = -1;
int earfcn_start = -1, earfcn_end = -1;

cell_search_cfg_t cell_detect_config = {.max_frames_pbch      = ISRRAN_DEFAULT_MAX_FRAMES_PBCH,
                                        .max_frames_pss       = ISRRAN_DEFAULT_MAX_FRAMES_PSS,
                                        .nof_valid_pss_frames = ISRRAN_DEFAULT_NOF_VALID_PSS_FRAMES,
                                        .init_agc             = 0,
                                        .force_tdd            = false};

struct cells {
  isrran_cell_t cell;
  float         freq;
  int           dl_earfcn;
  float         power;
};
struct cells results[1024];

float rf_gain = 70.0;
char* rf_args = "";
char* rf_dev  = "";

void usage(char* prog)
{
  printf("Usage: %s [agsendtvb] -b band\n", prog);
  printf("\t-a RF args [Default %s]\n", rf_args);
  printf("\t-d RF devicename [Default %s]\n", rf_dev);
  printf("\t-g RF gain [Default %.2f dB]\n", rf_gain);
  printf("\t-s earfcn_start [Default All]\n");
  printf("\t-e earfcn_end [Default All]\n");
  printf("\t-n nof_frames_total [Default 100]\n");
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "agsendvb")) != -1) {
    switch (opt) {
      case 'a':
        rf_args = argv[optind];
        break;
      case 'b':
        band = (int)strtol(argv[optind], NULL, 10);
        break;
      case 'd':
        rf_dev = argv[optind];
        break;
      case 's':
        earfcn_start = (int)strtol(argv[optind], NULL, 10);
        break;
      case 'e':
        earfcn_end = (int)strtol(argv[optind], NULL, 10);
        break;
      case 'n':
        cell_detect_config.max_frames_pss = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'g':
        rf_gain = strtof(argv[optind], NULL);
        break;
      case 'v':
        increase_isrran_verbose_level();
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
  if (band == -1) {
    usage(argv[0]);
    exit(-1);
  }
}

int isrran_rf_recv_wrapper(void* h, void* data, uint32_t nsamples, isrran_timestamp_t* t)
{
  DEBUG(" ----  Receive %d samples  ---- ", nsamples);
  return isrran_rf_recv_with_time((isrran_rf_t*)h, data, nsamples, 1, NULL, NULL);
}

bool go_exit = false;

void sig_int_handler(int signo)
{
  printf("SIGINT received. Exiting...\n");
  if (signo == SIGINT) {
    go_exit = true;
  }
}

static ISRRAN_AGC_CALLBACK(isrran_rf_set_rx_gain_wrapper)
{
  isrran_rf_set_rx_gain((isrran_rf_t*)h, gain_db);
}

int main(int argc, char** argv)
{
  int                           n;
  isrran_rf_t                   rf;
  isrran_ue_cellsearch_t        cs;
  isrran_ue_cellsearch_result_t found_cells[3];
  int                           nof_freqs;
  isrran_earfcn_t               channels[MAX_EARFCN];
  uint32_t                      freq;
  uint32_t                      n_found_cells = 0;

  isrran_debug_handle_crash(argc, argv);

  parse_args(argc, argv);

  printf("Opening RF device...\n");

  if (isrran_rf_open_devname(&rf, rf_dev, rf_args, 1)) {
    ERROR("Error opening rf");
    exit(-1);
  }
  if (!cell_detect_config.init_agc) {
    isrran_rf_set_rx_gain(&rf, rf_gain);
  } else {
    printf("Starting AGC thread...\n");
    if (isrran_rf_start_gain_thread(&rf, false)) {
      ERROR("Error opening rf");
      exit(-1);
    }
    isrran_rf_set_rx_gain(&rf, 50);
  }

  // Supress RF messages
  isrran_rf_suppress_stdout(&rf);

  nof_freqs = isrran_band_get_fd_band(band, channels, earfcn_start, earfcn_end, MAX_EARFCN);
  if (nof_freqs < 0) {
    ERROR("Error getting EARFCN list");
    exit(-1);
  }

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigprocmask(SIG_UNBLOCK, &sigset, NULL);
  signal(SIGINT, sig_int_handler);

  if (isrran_ue_cellsearch_init(&cs, cell_detect_config.max_frames_pss, isrran_rf_recv_wrapper, (void*)&rf)) {
    ERROR("Error initiating UE cell detect");
    exit(-1);
  }

  if (cell_detect_config.max_frames_pss) {
    isrran_ue_cellsearch_set_nof_valid_frames(&cs, cell_detect_config.nof_valid_pss_frames);
  }
  if (cell_detect_config.init_agc) {
    isrran_rf_info_t* rf_info = isrran_rf_get_info(&rf);
    isrran_ue_sync_start_agc(&cs.ue_sync,
                             isrran_rf_set_rx_gain_wrapper,
                             rf_info->min_rx_gain,
                             rf_info->max_rx_gain,
                             cell_detect_config.init_agc);
  }

  for (freq = 0; freq < nof_freqs && !go_exit; freq++) {
    /* set rf_freq */
    isrran_rf_set_rx_freq(&rf, 0, (double)channels[freq].fd * MHZ);
    INFO("Set rf_freq to %.3f MHz", (double)channels[freq].fd * MHZ / 1000000);

    printf(
        "[%3d/%d]: EARFCN %d Freq. %.2f MHz looking for PSS.\n", freq, nof_freqs, channels[freq].id, channels[freq].fd);
    fflush(stdout);

    if (ISRRAN_VERBOSE_ISINFO()) {
      printf("\n");
    }

    bzero(found_cells, 3 * sizeof(isrran_ue_cellsearch_result_t));

    INFO("Setting sampling frequency %.2f MHz for PSS search", ISRRAN_CS_SAMP_FREQ / 1000000);
    isrran_rf_set_rx_srate(&rf, ISRRAN_CS_SAMP_FREQ);
    INFO("Starting receiver...");
    isrran_rf_start_rx_stream(&rf, false);

    n = isrran_ue_cellsearch_scan(&cs, found_cells, NULL);
    if (n < 0) {
      ERROR("Error searching cell");
      exit(-1);
    } else if (n > 0) {
      for (int i = 0; i < 3; i++) {
        if (found_cells[i].psr > 2.0) {
          isrran_cell_t cell;
          cell.id = found_cells[i].cell_id;
          cell.cp = found_cells[i].cp;
          int ret = rf_mib_decoder(&rf, 1, &cell_detect_config, &cell, NULL);
          if (ret < 0) {
            ERROR("Error decoding MIB");
            exit(-1);
          }
          if (ret == ISRRAN_UE_MIB_FOUND) {
            printf("Found CELL ID %d. %d PRB, %d ports\n", cell.id, cell.nof_prb, cell.nof_ports);
            if (cell.nof_ports > 0) {
              results[n_found_cells].cell      = cell;
              results[n_found_cells].freq      = channels[freq].fd;
              results[n_found_cells].dl_earfcn = channels[freq].id;
              results[n_found_cells].power     = found_cells[i].peak;
              n_found_cells++;
            }
          }
        }
      }
    }
  }

  printf("\n\nFound %d cells\n", n_found_cells);
  for (int i = 0; i < n_found_cells; i++) {
    printf("Found CELL %.1f MHz, EARFCN=%d, PHYID=%d, %d PRB, %d ports, PSS power=%.1f dBm\n",
           results[i].freq,
           results[i].dl_earfcn,
           results[i].cell.id,
           results[i].cell.nof_prb,
           results[i].cell.nof_ports,
           isrran_convert_power_to_dB(results[i].power));
  }

  printf("\nBye\n");

  isrran_ue_cellsearch_free(&cs);
  isrran_rf_close(&rf);
  exit(0);
}
