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
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "isrran/phy/ch_estimation/chest_sl.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/io/filesource.h"
#include "isrran/phy/phch/mib_sl.h"
#include "isrran/phy/phch/psbch.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

isrran_cell_sl_t cell = {.nof_prb = 6, .N_sl_id = 168, .tm = ISRRAN_SIDELINK_TM2, .cp = ISRRAN_CP_NORM};
char*            input_file_name;
uint32_t         offset                 = 0;
float            frequency_offset       = 0.0;
float            snr                    = 100.0;
bool             use_standard_lte_rates = false;
bool             do_equalization        = true;

isrran_filesource_t fsrc;

void usage(char* prog)
{
  printf("Usage: %s [cdeipt]\n", prog);
  printf("\t-i input_file_name\n");
  printf("\t-p nof_prb [Default %d]\n", cell.nof_prb);
  printf("\t-e extended CP [Default normal]\n");
  printf("\t-d use_standard_lte_rates [Default %i]\n", use_standard_lte_rates);
  printf("\t-s skip equalization [Default no]\n");
  printf("\t-c N_sl_id [Default %d]\n", cell.N_sl_id);
  printf("\t-t Sidelink transmission mode {1,2,3,4} [Default %d]\n", (cell.tm + 1));
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "cdeisptv")) != -1) {
    switch (opt) {
      case 'c':
        cell.N_sl_id = (int32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'd':
        use_standard_lte_rates = true;
        break;
      case 's':
        do_equalization = false;
        break;
      case 'e':
        cell.cp = ISRRAN_CP_EXT;
        break;
      case 'i':
        input_file_name = argv[optind];
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
  if (ISRRAN_CP_ISEXT(cell.cp) && cell.tm >= ISRRAN_SIDELINK_TM3) {
    ERROR("Selected TM does not support extended CP");
    usage(argv[0]);
    exit(-1);
  }
}

int main(int argc, char** argv)
{
  int ret = ISRRAN_ERROR;

  parse_args(argc, argv);
  isrran_use_standard_symbol_size(use_standard_lte_rates);

  int32_t symbol_sz = isrran_symbol_sz(cell.nof_prb);
  printf("Symbol SZ: %i\n", symbol_sz);

  uint32_t sf_n_samples = isrran_symbol_sz(cell.nof_prb) * 15;
  printf("sf_n_samples: %i\n", sf_n_samples);

  uint32_t sf_n_re             = ISRRAN_SF_LEN_RE(cell.nof_prb, cell.cp);
  cf_t*    sf_buffer           = isrran_vec_cf_malloc(sf_n_re);
  cf_t*    equalized_sf_buffer = isrran_vec_cf_malloc(sf_n_re);

  cf_t* input_buffer  = isrran_vec_cf_malloc(sf_n_samples);
  cf_t* output_buffer = isrran_vec_cf_malloc(sf_n_samples);

  // TX
  isrran_ofdm_t ifft;
  if (isrran_ofdm_tx_init(&ifft, cell.cp, sf_buffer, output_buffer, cell.nof_prb)) {
    ERROR("Error creating IFFT object");
    return ISRRAN_ERROR;
  }
  isrran_ofdm_set_normalize(&ifft, true);
  isrran_ofdm_set_freq_shift(&ifft, 0.5);

  // RX
  isrran_ofdm_t fft;
  if (isrran_ofdm_rx_init(&fft, cell.cp, input_buffer, sf_buffer, cell.nof_prb)) {
    fprintf(stderr, "Error creating FFT object\n");
    return ISRRAN_ERROR;
  }
  isrran_ofdm_set_normalize(&fft, true);
  isrran_ofdm_set_freq_shift(&fft, -0.5);

  // PSBCH
  isrran_psbch_t psbch;
  if (isrran_psbch_init(&psbch, cell.nof_prb, cell.N_sl_id, cell.tm, cell.cp) != ISRRAN_SUCCESS) {
    ERROR("Error in psbch init");
    return ISRRAN_ERROR;
  }

  // PSCBH DMRS
  isrran_sl_comm_resource_pool_t sl_comm_resource_pool;
  if (isrran_sl_comm_resource_pool_get_default_config(&sl_comm_resource_pool, cell) != ISRRAN_SUCCESS) {
    ERROR("Error initializing sl_comm_resource_pool");
    return ISRRAN_ERROR;
  }

  isrran_chest_sl_t psbch_chest;
  if (isrran_chest_sl_init(&psbch_chest, ISRRAN_SIDELINK_PSBCH, cell, &sl_comm_resource_pool) != ISRRAN_SUCCESS) {
    ERROR("Error in chest PSBCH init");
    return ISRRAN_ERROR;
  }

  // Read subframe from third party implementations
  if (!input_file_name || isrran_filesource_init(&fsrc, input_file_name, ISRRAN_COMPLEX_FLOAT_BIN)) {
    printf("Error opening file %s\n", input_file_name);
    return ISRRAN_ERROR;
  }

  isrran_filesource_read(&fsrc, input_buffer, sf_n_samples);
  // isrran_vec_sc_prod_cfc(input_buffer, sqrtf(symbol_sz), input_buffer, sf_n_samples);

  // Run FFT
  isrran_ofdm_rx_sf(&fft);

  // Equalize
  if (do_equalization) {
    isrran_chest_sl_ls_estimate_equalize(&psbch_chest, sf_buffer, equalized_sf_buffer);
  } else {
    // just copy symbols
    memcpy(equalized_sf_buffer, sf_buffer, sizeof(cf_t) * sf_n_re);
  }

  // prepare Rx buffer
  uint8_t mib_sl_rx[ISRRAN_MIB_SL_MAX_LEN] = {};

  // Decode PSBCH
  if (isrran_psbch_decode(&psbch, equalized_sf_buffer, mib_sl_rx, sizeof(mib_sl_rx)) == ISRRAN_SUCCESS) {
    printf("Rx payload: ");
    isrran_vec_fprint_hex(stdout, mib_sl_rx, sizeof(mib_sl_rx));

    // Unpack and print MIB-SL
    isrran_mib_sl_t mib_sl;
    isrran_mib_sl_init(&mib_sl, cell.tm);
    isrran_mib_sl_unpack(&mib_sl, mib_sl_rx);
    isrran_mib_sl_printf(stdout, &mib_sl);

    // check decoded bandwidth matches user configured value
    if (isrran_mib_sl_bandwith_to_prb[mib_sl.sl_bandwidth_r12] == cell.nof_prb) {
      ret = ISRRAN_SUCCESS;
    }
  }

  if (ISRRAN_VERBOSE_ISDEBUG()) {
    char* filename = (do_equalization) ? "psbch_rx_syms_eq_on.bin" : "psbch_rx_syms_eq_off.bin";
    printf("Saving PSBCH symbols (%d) to %s\n", psbch.E / psbch.Qm, filename);
    isrran_vec_save_file(filename, psbch.mod_symbols, psbch.E / psbch.Qm * sizeof(cf_t));
  }

  isrran_ofdm_tx_free(&ifft);
  isrran_ofdm_rx_free(&fft);

  isrran_filesource_free(&fsrc);

  isrran_chest_sl_free(&psbch_chest);
  isrran_psbch_free(&psbch);

  free(sf_buffer);
  free(equalized_sf_buffer);
  free(input_buffer);
  free(output_buffer);

  return ret;
}
