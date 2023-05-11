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

#include "isrran/phy/phch/pdsch_nr.h"
#include "isrran/phy/phch/ra_dl_nr.h"
#include "isrran/phy/phch/ra_nr.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/random.h"
#include "isrran/phy/utils/vector.h"
#include <complex.h>
#include <getopt.h>
#include <math.h>

static isrran_carrier_nr_t carrier = ISRRAN_DEFAULT_CARRIER_NR;

static uint32_t            n_prb     = 0;  // Set to 0 for steering
static uint32_t            mcs       = 30; // Set to 30 for steering
static isrran_sch_cfg_nr_t pdsch_cfg = {};
static uint16_t            rnti      = 0x1234;

void usage(char* prog)
{
  printf("Usage: %s [pTL] \n", prog);
  printf("\t-p Number of grant PRB, set to 0 for steering [Default %d]\n", n_prb);
  printf("\t-m MCS PRB, set to >28 for steering [Default %d]\n", mcs);
  printf("\t-T Provide MCS table (64qam, 256qam, 64qamLowSE) [Default %s]\n",
         isrran_mcs_table_to_str(pdsch_cfg.sch_cfg.mcs_table));
  printf("\t-L Provide number of layers [Default %d]\n", carrier.max_mimo_layers);
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

int parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "pmTLv")) != -1) {
    switch (opt) {
      case 'p':
        n_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'm':
        mcs = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'T':
        pdsch_cfg.sch_cfg.mcs_table = isrran_mcs_table_from_str(argv[optind]);
        break;
      case 'L':
        carrier.max_mimo_layers = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'v':
        increase_isrran_verbose_level();
        break;
      default:
        usage(argv[0]);
        return ISRRAN_ERROR;
    }
  }

  return ISRRAN_SUCCESS;
}

int main(int argc, char** argv)
{
  int                   ret       = ISRRAN_ERROR;
  isrran_pdsch_nr_t     pdsch_tx  = {};
  isrran_pdsch_nr_t     pdsch_rx  = {};
  isrran_chest_dl_res_t chest     = {};
  isrran_pdsch_res_nr_t pdsch_res = {};
  isrran_random_t       rand_gen  = isrran_random_init(1234);

  uint8_t* data_tx[ISRRAN_MAX_TB]           = {};
  uint8_t* data_rx[ISRRAN_MAX_CODEWORDS]    = {};
  cf_t*    sf_symbols[ISRRAN_MAX_LAYERS_NR] = {};

  // Set default PDSCH configuration
  pdsch_cfg.sch_cfg.mcs_table = isrran_mcs_table_64qam;

  if (parse_args(argc, argv) < ISRRAN_SUCCESS) {
    goto clean_exit;
  }

  isrran_pdsch_nr_args_t pdsch_args = {};
  pdsch_args.sch.disable_simd       = false;
  pdsch_args.measure_evm            = true;

  if (isrran_pdsch_nr_init_enb(&pdsch_tx, &pdsch_args) < ISRRAN_SUCCESS) {
    ERROR("Error initiating PDSCH for Tx");
    goto clean_exit;
  }

  if (isrran_pdsch_nr_init_ue(&pdsch_rx, &pdsch_args) < ISRRAN_SUCCESS) {
    ERROR("Error initiating SCH NR for Rx");
    goto clean_exit;
  }

  if (isrran_pdsch_nr_set_carrier(&pdsch_tx, &carrier)) {
    ERROR("Error setting SCH NR carrier");
    goto clean_exit;
  }

  if (isrran_pdsch_nr_set_carrier(&pdsch_rx, &carrier)) {
    ERROR("Error setting SCH NR carrier");
    goto clean_exit;
  }

  for (uint32_t i = 0; i < carrier.max_mimo_layers; i++) {
    sf_symbols[i] = isrran_vec_cf_malloc(ISRRAN_SLOT_LEN_RE_NR(carrier.nof_prb));
    if (sf_symbols[i] == NULL) {
      ERROR("Error malloc");
      goto clean_exit;
    }
  }

  for (uint32_t i = 0; i < pdsch_tx.max_cw; i++) {
    data_tx[i] = isrran_vec_u8_malloc(ISRRAN_SLOT_MAX_NOF_BITS_NR);
    data_rx[i] = isrran_vec_u8_malloc(ISRRAN_SLOT_MAX_NOF_BITS_NR);
    if (data_tx[i] == NULL || data_rx[i] == NULL) {
      ERROR("Error malloc");
      goto clean_exit;
    }

    pdsch_res.tb[i].payload = data_rx[i];
  }

  isrran_softbuffer_tx_t softbuffer_tx = {};
  isrran_softbuffer_rx_t softbuffer_rx = {};

  if (isrran_softbuffer_tx_init_guru(&softbuffer_tx, ISRRAN_SCH_NR_MAX_NOF_CB_LDPC, ISRRAN_LDPC_MAX_LEN_ENCODED_CB) <
      ISRRAN_SUCCESS) {
    ERROR("Error init soft-buffer");
    goto clean_exit;
  }

  if (isrran_softbuffer_rx_init_guru(&softbuffer_rx, ISRRAN_SCH_NR_MAX_NOF_CB_LDPC, ISRRAN_LDPC_MAX_LEN_ENCODED_CB) <
      ISRRAN_SUCCESS) {
    ERROR("Error init soft-buffer");
    goto clean_exit;
  }

  // Use grant default A time resources with m=0
  if (isrran_ra_dl_nr_time_default_A(0, pdsch_cfg.dmrs.typeA_pos, &pdsch_cfg.grant) < ISRRAN_SUCCESS) {
    ERROR("Error loading default grant");
    goto clean_exit;
  }

  // Set PDSCH grant without considering any procedure
  pdsch_cfg.grant.nof_dmrs_cdm_groups_without_data = 1; // No need for MIMO
  pdsch_cfg.grant.nof_layers                       = carrier.max_mimo_layers;
  pdsch_cfg.grant.dci_format                       = isrran_dci_format_nr_1_0;
  pdsch_cfg.grant.rnti                             = rnti;

  uint32_t n_prb_start = 1;
  uint32_t n_prb_end   = carrier.nof_prb + 1;
  if (n_prb > 0) {
    n_prb_start = ISRRAN_MIN(n_prb, n_prb_end - 1);
    n_prb_end   = ISRRAN_MIN(n_prb + 1, n_prb_end);
  }

  uint32_t mcs_start = 0;
  uint32_t mcs_end   = pdsch_cfg.sch_cfg.mcs_table == isrran_mcs_table_256qam ? 28 : 29;
  if (mcs < mcs_end) {
    mcs_start = ISRRAN_MIN(mcs, mcs_end - 1);
    mcs_end   = ISRRAN_MIN(mcs + 1, mcs_end);
  }

  if (isrran_chest_dl_res_init(&chest, carrier.nof_prb) < ISRRAN_SUCCESS) {
    ERROR("Initiating chest");
    goto clean_exit;
  }

  for (n_prb = n_prb_start; n_prb < n_prb_end; n_prb++) {
    for (mcs = mcs_start; mcs < mcs_end; mcs++) {
      for (uint32_t n = 0; n < ISRRAN_MAX_PRB_NR; n++) {
        pdsch_cfg.grant.prb_idx[n] = (n < n_prb);
      }

      if (isrran_ra_nr_fill_tb(&pdsch_cfg, &pdsch_cfg.grant, mcs, &pdsch_cfg.grant.tb[0]) < ISRRAN_SUCCESS) {
        ERROR("Error filing tb");
        goto clean_exit;
      }

      for (uint32_t tb = 0; tb < ISRRAN_MAX_TB; tb++) {
        // Skip TB if no allocated
        if (data_tx[tb] == NULL) {
          continue;
        }

        for (uint32_t i = 0; i < pdsch_cfg.grant.tb[tb].tbs; i++) {
          data_tx[tb][i] = (uint8_t)isrran_random_uniform_int_dist(rand_gen, 0, UINT8_MAX);
        }
        pdsch_cfg.grant.tb[tb].softbuffer.tx = &softbuffer_tx;
      }

      if (isrran_pdsch_nr_encode(&pdsch_tx, &pdsch_cfg, &pdsch_cfg.grant, data_tx, sf_symbols) < ISRRAN_SUCCESS) {
        ERROR("Error encoding");
        goto clean_exit;
      }

      for (uint32_t tb = 0; tb < ISRRAN_MAX_TB; tb++) {
        pdsch_cfg.grant.tb[tb].softbuffer.rx = &softbuffer_rx;
        isrran_softbuffer_rx_reset(pdsch_cfg.grant.tb[tb].softbuffer.rx);
      }

      for (uint32_t i = 0; i < pdsch_cfg.grant.tb->nof_re; i++) {
        chest.ce[0][0][i] = 1.0f;
      }
      chest.nof_re = pdsch_cfg.grant.tb->nof_re;

      if (isrran_pdsch_nr_decode(&pdsch_rx, &pdsch_cfg, &pdsch_cfg.grant, &chest, sf_symbols, &pdsch_res) <
          ISRRAN_SUCCESS) {
        ERROR("Error encoding");
        goto clean_exit;
      }

      if (pdsch_res.evm[0] > 0.001f) {
        ERROR("Error PDSCH EVM is too high %f", pdsch_res.evm[0]);
        goto clean_exit;
      }

      float    mse    = 0.0f;
      uint32_t nof_re = isrran_ra_dl_nr_slot_nof_re(&pdsch_cfg, &pdsch_cfg.grant);
      for (uint32_t i = 0; i < pdsch_cfg.grant.nof_layers; i++) {
        for (uint32_t j = 0; j < nof_re; j++) {
          mse += cabsf(pdsch_tx.d[i][j] - pdsch_rx.d[i][j]);
        }
      }
      if (nof_re * pdsch_cfg.grant.nof_layers > 0) {
        mse = mse / (nof_re * pdsch_cfg.grant.nof_layers);
      }
      if (mse > 0.001) {
        ERROR("MSE error (%f) is too high", mse);
        for (uint32_t i = 0; i < pdsch_cfg.grant.nof_layers; i++) {
          printf("d_tx[%d]=", i);
          isrran_vec_fprint_c(stdout, pdsch_tx.d[i], nof_re);
          printf("d_rx[%d]=", i);
          isrran_vec_fprint_c(stdout, pdsch_rx.d[i], nof_re);
        }
        goto clean_exit;
      }

      if (!pdsch_res.tb[0].crc) {
        ERROR("Failed to match CRC; n_prb=%d; mcs=%d; TBS=%d;", n_prb, mcs, pdsch_cfg.grant.tb[0].tbs);
        goto clean_exit;
      }

      if (memcmp(data_tx[0], data_rx[0], pdsch_cfg.grant.tb[0].tbs / 8) != 0) {
        ERROR("Failed to match Tx/Rx data; n_prb=%d; mcs=%d; TBS=%d;", n_prb, mcs, pdsch_cfg.grant.tb[0].tbs);
        printf("Tx data: ");
        isrran_vec_fprint_byte(stdout, data_tx[0], pdsch_cfg.grant.tb[0].tbs / 8);
        printf("Rx data: ");
        isrran_vec_fprint_byte(stdout, data_rx[0], pdsch_cfg.grant.tb[0].tbs / 8);
        goto clean_exit;
      }

      INFO("n_prb=%d; mcs=%d; TBS=%d; EVM=%f; PASSED!\n", n_prb, mcs, pdsch_cfg.grant.tb[0].tbs, pdsch_res.evm[0]);
    }
  }

  ret = ISRRAN_SUCCESS;

clean_exit:
  isrran_chest_dl_res_free(&chest);
  isrran_random_free(rand_gen);
  isrran_pdsch_nr_free(&pdsch_tx);
  isrran_pdsch_nr_free(&pdsch_rx);
  for (uint32_t i = 0; i < ISRRAN_MAX_CODEWORDS; i++) {
    if (data_tx[i]) {
      free(data_tx[i]);
    }
    if (data_rx[i]) {
      free(data_rx[i]);
    }
  }
  for (uint32_t i = 0; i < ISRRAN_MAX_LAYERS_NR; i++) {
    if (sf_symbols[i]) {
      free(sf_symbols[i]);
    }
  }
  isrran_softbuffer_tx_free(&softbuffer_tx);
  isrran_softbuffer_rx_free(&softbuffer_rx);

  return ret;
}
