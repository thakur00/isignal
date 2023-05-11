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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "isrran/phy/ch_estimation/chest_sl.h"
#include "isrran/phy/channel/ch_awgn.h"
#include "isrran/phy/common/phy_common_sl.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/io/filesource.h"
#include "isrran/phy/phch/pscch.h"
#include "isrran/phy/phch/pssch.h"
#include "isrran/phy/phch/ra_sl.h"
#include "isrran/phy/phch/sci.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

static char*            input_file_name = NULL;
static isrran_cell_sl_t cell            = {.nof_prb = 6, .N_sl_id = 0, .tm = ISRRAN_SIDELINK_TM2, .cp = ISRRAN_CP_NORM};
static bool             use_standard_lte_rates = false;
static uint32_t         file_offset            = 0;

static uint32_t                       sf_n_samples          = 0;
static uint32_t                       sf_n_re               = 0;
static cf_t*                          sf_buffer             = NULL;
static cf_t*                          equalized_sf_buffer   = NULL;
static cf_t*                          input_buffer          = NULL;
static isrran_sci_t                   sci                   = {};
static isrran_pscch_t                 pscch                 = {};
static isrran_chest_sl_t              pscch_chest           = {};
static isrran_pssch_t                 pssch                 = {};
static isrran_chest_sl_t              pssch_chest           = {};
static isrran_ofdm_t                  fft                   = {};
static isrran_sl_comm_resource_pool_t sl_comm_resource_pool = {};
static uint32_t                       size_sub_channel      = 10;
static uint32_t                       num_sub_channel       = 5;
static uint32_t                       current_sf_idx        = 0;

static isrran_chest_sl_cfg_t pscch_chest_sl_cfg = {};
static isrran_chest_sl_cfg_t pssch_chest_sl_cfg = {};

static isrran_filesource_t fsrc = {};

void usage(char* prog)
{
  printf("Usage: %s [deinopstv]\n", prog);
  printf("\t-i input_file_name\n");
  printf("\t-o File offset samples [Default %d]\n", file_offset);
  printf("\t-p nof_prb [Default %d]\n", cell.nof_prb);
  printf("\t-s size_sub_channel [Default for 50 prbs %d]\n", size_sub_channel);
  printf("\t-n num_sub_channel [Default for 50 prbs %d]\n", num_sub_channel);
  printf("\t-m Subframe index [Default for %d]\n", current_sf_idx);
  printf("\t-e Extended CP [Default normal]\n");
  printf("\t-t Sidelink transmission mode {1,2,3,4} [Default %d]\n", (cell.tm + 1));
  printf("\t-d use_standard_lte_rates [Default %i]\n", use_standard_lte_rates);
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "deinmopstv")) != -1) {
    switch (opt) {
      case 'd':
        use_standard_lte_rates = true;
        break;
      case 'o':
        file_offset = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'e':
        cell.cp = ISRRAN_CP_EXT;
        break;
      case 'i':
        input_file_name = argv[optind];
        break;
      case 's':
        size_sub_channel = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'n':
        num_sub_channel = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'm':
        current_sf_idx = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'p':
        cell.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 't':
        if (isrran_sl_tm_to_cell_sl_tm_t(&cell, strtol(argv[optind], NULL, 10)) != ISRRAN_SUCCESS) {
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

int base_init()
{
  sf_n_samples = isrran_symbol_sz(cell.nof_prb) * 15;
  sf_n_re      = ISRRAN_SF_LEN_RE(cell.nof_prb, cell.cp);

  if (isrran_sl_comm_resource_pool_get_default_config(&sl_comm_resource_pool, cell) != ISRRAN_SUCCESS) {
    ERROR("Error initializing sl_comm_resource_pool");
    return ISRRAN_ERROR;
  }
  sl_comm_resource_pool.num_sub_channel  = num_sub_channel;
  sl_comm_resource_pool.size_sub_channel = size_sub_channel;

  sf_buffer = isrran_vec_cf_malloc(sf_n_re);
  if (!sf_buffer) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }
  isrran_vec_cf_zero(sf_buffer, sf_n_re);

  equalized_sf_buffer = isrran_vec_cf_malloc(sf_n_re);
  if (!equalized_sf_buffer) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }
  isrran_vec_cf_zero(equalized_sf_buffer, sf_n_re);

  input_buffer = isrran_vec_cf_malloc(sf_n_samples);
  if (!input_buffer) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }
  isrran_vec_cf_zero(input_buffer, sf_n_samples);

  if (isrran_sci_init(&sci, &cell, &sl_comm_resource_pool) < ISRRAN_SUCCESS) {
    ERROR("Error in SCI init");
    return ISRRAN_ERROR;
  }

  if (isrran_pscch_init(&pscch, ISRRAN_MAX_PRB) != ISRRAN_SUCCESS) {
    ERROR("Error in PSCCH init");
    return ISRRAN_ERROR;
  }

  if (isrran_pscch_set_cell(&pscch, cell) != ISRRAN_SUCCESS) {
    ERROR("Error in PSCCH set cell");
    return ISRRAN_ERROR;
  }

  if (isrran_chest_sl_init(&pscch_chest, ISRRAN_SIDELINK_PSCCH, cell, &sl_comm_resource_pool) != ISRRAN_SUCCESS) {
    ERROR("Error in PSCCH DMRS init");
    return ISRRAN_ERROR;
  }

  if (isrran_pssch_init(&pssch, &cell, &sl_comm_resource_pool) != ISRRAN_SUCCESS) {
    ERROR("Error initializing PSSCH");
    return ISRRAN_ERROR;
  }

  if (isrran_chest_sl_init(&pssch_chest, ISRRAN_SIDELINK_PSSCH, cell, &sl_comm_resource_pool) != ISRRAN_SUCCESS) {
    ERROR("Error in chest PSSCH init");
    return ISRRAN_ERROR;
  }

  if (input_file_name) {
    if (isrran_filesource_init(&fsrc, input_file_name, ISRRAN_COMPLEX_FLOAT_BIN)) {
      printf("Error opening file %s", input_file_name);
      return ISRRAN_ERROR;
    }
  } else {
    ERROR("Invalid input file name");
    return ISRRAN_ERROR;
  }

  if (isrran_ofdm_rx_init(&fft, cell.cp, input_buffer, sf_buffer, cell.nof_prb)) {
    fprintf(stderr, "Error creating FFT object\n");
    return ISRRAN_ERROR;
  }
  isrran_ofdm_set_normalize(&fft, true);
  isrran_ofdm_set_freq_shift(&fft, -0.5);

  return ISRRAN_SUCCESS;
}

void base_free()
{
  isrran_filesource_free(&fsrc);
  isrran_ofdm_rx_free(&fft);

  isrran_sci_free(&sci);
  isrran_pscch_free(&pscch);
  isrran_chest_sl_free(&pscch_chest);

  isrran_pssch_free(&pssch);
  isrran_chest_sl_free(&pssch_chest);

  if (sf_buffer) {
    free(sf_buffer);
  }
  if (equalized_sf_buffer) {
    free(equalized_sf_buffer);
  }
  if (input_buffer) {
    free(input_buffer);
  }
}

int main(int argc, char** argv)
{
  uint8_t sci_rx[ISRRAN_SCI_MAX_LEN] = {};
  int     ret                        = ISRRAN_ERROR;

  parse_args(argc, argv);
  isrran_use_standard_symbol_size(use_standard_lte_rates);

  if (base_init()) {
    ERROR("Error initializing");
    base_free();
    return ISRRAN_ERROR;
  }

  bool     sci_decoded                     = false;
  uint32_t num_decoded_sci                 = 0;
  char     sci_msg[ISRRAN_SCI_MSG_MAX_LEN] = {};

  uint32_t num_decoded_tb               = 0;
  uint8_t  tb[ISRRAN_SL_SCH_MAX_TB_LEN] = {};

  int max_num_subframes = 128;
  int num_subframes     = 0;
  int nread             = 0;

  uint32_t period_sf_idx        = 0;
  uint32_t allowed_pssch_sf_idx = 0;

  if (file_offset > 0) {
    printf("Offsetting file by %d samples.\n", file_offset);
    isrran_filesource_seek(&fsrc, file_offset * sizeof(cf_t));
  }

  do {
    nread = isrran_filesource_read(&fsrc, input_buffer, sf_n_samples);
    if (nread < 0) {
      fprintf(stderr, "Error reading from file\n");
      return ret;
    } else if (nread == 0) {
      goto clean_exit;
    } else if (nread < sf_n_samples) {
      fprintf(stderr, "Couldn't read entire subframe. Still processing ..\n");
      nread = -1;
    }

    // Convert to frequency domain
    isrran_ofdm_rx_sf(&fft);

    if (cell.tm == ISRRAN_SIDELINK_TM1 || cell.tm == ISRRAN_SIDELINK_TM2) {
      // 3GPP TS 36.213 Section 14.2.1.2 UE procedure for determining subframes
      // and resource blocks for transmitting PSCCH for sidelink transmission mode 2
      if (sl_comm_resource_pool.pscch_sf_bitmap[period_sf_idx] == 1) {
        for (uint32_t pscch_prb_start_idx = sl_comm_resource_pool.prb_start;
             pscch_prb_start_idx <= sl_comm_resource_pool.prb_end;
             pscch_prb_start_idx++) {
          // PSCCH Channel estimation
          pscch_chest_sl_cfg.prb_start_idx = pscch_prb_start_idx;
          isrran_chest_sl_set_cfg(&pscch_chest, pscch_chest_sl_cfg);
          isrran_chest_sl_ls_estimate_equalize(&pscch_chest, sf_buffer, equalized_sf_buffer);

          if (isrran_pscch_decode(&pscch, equalized_sf_buffer, sci_rx, pscch_prb_start_idx) == ISRRAN_SUCCESS) {
            if (isrran_sci_format0_unpack(&sci, sci_rx) == ISRRAN_SUCCESS) {
              isrran_sci_info(&sci, sci_msg, sizeof(sci_msg));
              fprintf(stdout, "%s\n", sci_msg);

              sci_decoded = true;
              num_decoded_sci++;
            }
          }

          if ((sl_comm_resource_pool.prb_num * 2) <=
              (sl_comm_resource_pool.prb_end - sl_comm_resource_pool.prb_start + 1)) {
            if ((pscch_prb_start_idx + 1) == (sl_comm_resource_pool.prb_start + sl_comm_resource_pool.prb_num)) {
              pscch_prb_start_idx = sl_comm_resource_pool.prb_end - sl_comm_resource_pool.prb_num;
            }
          }
        }
      }

      if ((sl_comm_resource_pool.pssch_sf_bitmap[period_sf_idx] == 1) && (sci_decoded == true)) {
        if (isrran_ra_sl_pssch_allowed_sf(current_sf_idx, sci.trp_idx, ISRRAN_SL_DUPLEX_MODE_FDD, 0)) {
          // Redundancy version
          uint32_t rv_idx = allowed_pssch_sf_idx % 4;

          uint32_t nof_prb_pssch       = 0;
          uint32_t pssch_prb_start_idx = 0;
          isrran_ra_sl_type0_from_riv(sci.riv, cell.nof_prb, &nof_prb_pssch, &pssch_prb_start_idx);
          printf("pssch_start_prb_idx = %i nof_prb = %i\n", pssch_prb_start_idx, nof_prb_pssch);

          // PSSCH Channel estimation
          pssch_chest_sl_cfg.N_x_id        = sci.N_sa_id;
          pssch_chest_sl_cfg.sf_idx        = current_sf_idx;
          pssch_chest_sl_cfg.prb_start_idx = pssch_prb_start_idx;
          pssch_chest_sl_cfg.nof_prb       = nof_prb_pssch;
          isrran_chest_sl_set_cfg(&pssch_chest, pssch_chest_sl_cfg);
          isrran_chest_sl_ls_estimate_equalize(&pssch_chest, sf_buffer, equalized_sf_buffer);

          isrran_pssch_cfg_t pssch_cfg = {
              pssch_prb_start_idx, nof_prb_pssch, sci.N_sa_id, sci.mcs_idx, rv_idx, current_sf_idx};
          if (isrran_pssch_set_cfg(&pssch, pssch_cfg) == ISRRAN_SUCCESS) {
            if (isrran_pssch_decode(&pssch, equalized_sf_buffer, tb, ISRRAN_SL_SCH_MAX_TB_LEN) == ISRRAN_SUCCESS) {
              isrran_vec_fprint_byte(stdout, tb, pssch.sl_sch_tb_len);
              num_decoded_tb++;
              printf("> Transport Block SUCCESS! TB count: %i\n", num_decoded_tb);
            }
          }
          allowed_pssch_sf_idx++;
        }
        current_sf_idx++;
      }
    } else if (cell.tm == ISRRAN_SIDELINK_TM3 || cell.tm == ISRRAN_SIDELINK_TM4) {
      for (int sub_channel_idx = 0; sub_channel_idx < sl_comm_resource_pool.num_sub_channel; sub_channel_idx++) {
        uint32_t pscch_prb_start_idx = sl_comm_resource_pool.size_sub_channel * sub_channel_idx;

        for (uint32_t cyclic_shift = 0; cyclic_shift <= 9; cyclic_shift += 3) {
          // PSCCH Channel estimation
          pscch_chest_sl_cfg.cyclic_shift  = cyclic_shift;
          pscch_chest_sl_cfg.prb_start_idx = pscch_prb_start_idx;
          isrran_chest_sl_set_cfg(&pscch_chest, pscch_chest_sl_cfg);
          isrran_chest_sl_ls_estimate_equalize(&pscch_chest, sf_buffer, equalized_sf_buffer);

          if (isrran_pscch_decode(&pscch, equalized_sf_buffer, sci_rx, pscch_prb_start_idx) == ISRRAN_SUCCESS) {
            if (isrran_sci_format1_unpack(&sci, sci_rx) == ISRRAN_SUCCESS) {
              isrran_sci_info(&sci, sci_msg, sizeof(sci_msg));
              fprintf(stdout, "%s\n", sci_msg);

              num_decoded_sci++;

              // Decode PSSCH
              uint32_t sub_channel_start_idx = 0;
              uint32_t L_subCH               = 0;
              isrran_ra_sl_type0_from_riv(
                  sci.riv, sl_comm_resource_pool.num_sub_channel, &L_subCH, &sub_channel_start_idx);

              // 3GPP TS 36.213 Section 14.1.1.4C
              uint32_t pssch_prb_start_idx = (sub_channel_idx * sl_comm_resource_pool.size_sub_channel) +
                                             pscch.pscch_nof_prb + sl_comm_resource_pool.start_prb_sub_channel;
              uint32_t nof_prb_pssch = ((L_subCH + sub_channel_idx) * sl_comm_resource_pool.size_sub_channel) -
                                       pssch_prb_start_idx + sl_comm_resource_pool.start_prb_sub_channel;

              // make sure PRBs are valid for DFT precoding
              nof_prb_pssch = isrran_dft_precoding_get_valid_prb(nof_prb_pssch);

              uint32_t N_x_id = 0;
              for (int j = 0; j < ISRRAN_SCI_CRC_LEN; j++) {
                N_x_id += pscch.sci_crc[j] * (1 << (ISRRAN_SCI_CRC_LEN - 1 - j));
              }

              uint32_t rv_idx = 0;
              if (sci.retransmission == true) {
                rv_idx = 1;
              }

              // PSSCH Channel estimation
              pssch_chest_sl_cfg.N_x_id        = N_x_id;
              pssch_chest_sl_cfg.sf_idx        = current_sf_idx;
              pssch_chest_sl_cfg.prb_start_idx = pssch_prb_start_idx;
              pssch_chest_sl_cfg.nof_prb       = nof_prb_pssch;
              isrran_chest_sl_set_cfg(&pssch_chest, pssch_chest_sl_cfg);
              isrran_chest_sl_ls_estimate_equalize(&pssch_chest, sf_buffer, equalized_sf_buffer);

              isrran_pssch_cfg_t pssch_cfg = {
                  pssch_prb_start_idx, nof_prb_pssch, N_x_id, sci.mcs_idx, rv_idx, current_sf_idx};
              if (isrran_pssch_set_cfg(&pssch, pssch_cfg) == ISRRAN_SUCCESS) {
                if (isrran_pssch_decode(&pssch, equalized_sf_buffer, tb, ISRRAN_SL_SCH_MAX_TB_LEN) == ISRRAN_SUCCESS) {
                  isrran_vec_fprint_byte(stdout, tb, pssch.sl_sch_tb_len);
                  num_decoded_tb++;
                }

                if (ISRRAN_VERBOSE_ISDEBUG()) {
                  char filename[64];
                  snprintf(filename, 64, "pssch_rx_syms_sf%d.bin", num_subframes);
                  printf("Saving PSSCH symbols (%d) to %s (current_sf_idx=%d)\n",
                         pssch.G / pssch.Qm,
                         filename,
                         current_sf_idx);
                  isrran_vec_save_file(filename, pssch.symbols, pssch.G / pssch.Qm * sizeof(cf_t));
                }
              }
            }
          }
          if (ISRRAN_VERBOSE_ISDEBUG()) {
            char filename[64];
            snprintf(filename,
                     64,
                     "pscch_rx_syms_sf%d_shift%d_prbidx%d.bin",
                     num_subframes,
                     cyclic_shift,
                     pscch_prb_start_idx);
            printf("Saving PSCCH symbols (%d) to %s\n", pscch.E / ISRRAN_PSCCH_QM, filename);
            isrran_vec_save_file(filename, pscch.mod_symbols, pscch.E / ISRRAN_PSCCH_QM * sizeof(cf_t));
          }
        }
      }
      current_sf_idx = (current_sf_idx + 1) % 10;
    }
    num_subframes++;
    period_sf_idx++;
  } while (nread > 0 && num_subframes < max_num_subframes);

clean_exit:

  base_free();

  printf("num_decoded_sci=%d num_decoded_tb=%d\n", num_decoded_sci, num_decoded_tb);

  ret = (num_decoded_sci > 0) ? ISRRAN_SUCCESS : ISRRAN_ERROR;

  return ret;
}