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

#include "isrran/isrran.h"
#include "isrran/support/isrran_test.h"
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

static isrran_cell_t cell = {6,              // nof_prb
                             1,              // nof_ports
                             1,              // cell_id
                             ISRRAN_CP_NORM, // cyclic prefix
                             ISRRAN_PHICH_NORM,
                             ISRRAN_PHICH_R_1, // PHICH length
                             ISRRAN_FDD};

static isrran_refsignal_dmrs_pusch_cfg_t dmrs_pusch_cfg = {};
static isrran_refsignal_isr_cfg_t        isr_cfg        = {};

static uint32_t test_counter = 0;

static float                 snr_db  = 20.0f;
static float                 n0_dbm  = 30.0f - 20.0f;
static isrran_channel_awgn_t channel = {};

#define CHEST_TEST_ISR_SNR_DB_TOLERANCE 10.0f
#define CHEST_TEST_ISR_TA_US_TOLERANCE 0.5f

// ISR index and sub-frame configuration possible values limited to SF=0
#define I_ISR_COUNT 9
static const uint32_t i_isr_values[I_ISR_COUNT] = {0, 2, 7, 17, 37, 77, 157, 317, 637};

#define SF_CONFIG_COUNT 9
static const uint32_t sf_config_values[SF_CONFIG_COUNT] = {0, 1, 3, 7, 9, 13, 14};

void usage(char* prog)
{
  printf("Usage: %s [recov]\n", prog);

  printf("\t-r nof_prb [Default %d]\n", cell.nof_prb);
  printf("\t-e extended cyclic prefix [Default normal]\n");

  printf("\t-c cell_id [Default %d]\n", cell.id);

  printf("\t-v increase verbosity\n");
}

typedef struct {
  uint32_t                      sf_size;
  isrran_refsignal_ul_t         refsignal_ul;
  isrran_refsignal_isr_pregen_t isr_pregen;
  isrran_chest_ul_t             chest_ul;
  cf_t*                         sf_symbols;
  isrran_chest_ul_res_t         chest_ul_res;
} isr_test_context_t;

int isr_test_context_init(isr_test_context_t* q)
{
  q->sf_size = ISRRAN_SF_LEN_RE(ISRRAN_MAX_PRB, cell.cp);

  // Set cell
  if (isrran_refsignal_ul_set_cell(&q->refsignal_ul, cell) != ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Pregenerate signals
  if (isrran_refsignal_isr_pregen(&q->refsignal_ul, &q->isr_pregen, &isr_cfg, &dmrs_pusch_cfg) != ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Allocate buffer
  q->sf_symbols = isrran_vec_cf_malloc(q->sf_size);
  if (q->sf_symbols == NULL) {
    return ISRRAN_ERROR;
  }

  // Create UL channel estimator
  if (isrran_chest_ul_init(&q->chest_ul, ISRRAN_MAX_PRB) != ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Set cell in UL channel estimator
  if (isrran_chest_ul_set_cell(&q->chest_ul, cell) != ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Initialise UL channel estimator result
  if (isrran_chest_ul_res_init(&q->chest_ul_res, ISRRAN_MAX_PRB) != ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

void isr_test_context_free(isr_test_context_t* q)
{
  isrran_refsignal_isr_pregen_free(&q->refsignal_ul, &q->isr_pregen);
  isrran_chest_ul_free(&q->chest_ul);
  isrran_chest_ul_res_free(&q->chest_ul_res);
  if (q->sf_symbols) {
    free(q->sf_symbols);
  }
}
int isr_test_context_run(isr_test_context_t* q)
{
  isrran_ul_sf_cfg_t ul_sf_cfg = {};

  INFO("   TEST: bw_cfg=%d; sf_cfg=%d; B=%d; b_hops=%d; n_isr=%d; I_isr=%d;",
       isr_cfg.bw_cfg,
       isr_cfg.subframe_config,
       isr_cfg.B,
       isr_cfg.b_hop,
       isr_cfg.n_isr,
       isr_cfg.I_isr);

  // Assert ISR transmission SF
  TESTASSERT(isrran_refsignal_isr_send_cs(isr_cfg.subframe_config, ul_sf_cfg.tti) == 1);
  TESTASSERT(isrran_refsignal_isr_send_cs(isr_cfg.subframe_config, ul_sf_cfg.tti) == 1);

  // Set resource grid to zero
  isrran_vec_cf_zero(q->sf_symbols, q->sf_size);

  // Put sounding reference signals
  TESTASSERT(isrran_refsignal_isr_put(
                 &q->refsignal_ul, &isr_cfg, ul_sf_cfg.tti, q->isr_pregen.r[ul_sf_cfg.tti], q->sf_symbols) ==
             ISRRAN_SUCCESS);

  // Apply AWGN channel
  if (!isnan(snr_db) && !isinf(snr_db)) {
    isrran_channel_awgn_run_c(&channel, q->sf_symbols, q->sf_symbols, q->sf_size);
  }

  // Estimate
  TESTASSERT(isrran_chest_ul_estimate_isr(
                 &q->chest_ul, &ul_sf_cfg, &isr_cfg, &dmrs_pusch_cfg, q->sf_symbols, &q->chest_ul_res) ==
             ISRRAN_SUCCESS);

  INFO("RESULTS: tti=%d; snr_db=%+.1f; noise_estimate_dbm=%+.1f; ta_us=%+.1f;",
       ul_sf_cfg.tti,
       q->chest_ul_res.snr_db,
       q->chest_ul_res.noise_estimate_dbFs,
       q->chest_ul_res.ta_us);

  // Assert ISR measurements
  TESTASSERT(fabsf(q->chest_ul_res.snr_db - snr_db) < CHEST_TEST_ISR_SNR_DB_TOLERANCE);
  TESTASSERT(fabsf(q->chest_ul_res.noise_estimate_dbFs - n0_dbm) < CHEST_TEST_ISR_SNR_DB_TOLERANCE);
  TESTASSERT(fabsf(q->chest_ul_res.ta_us) < CHEST_TEST_ISR_TA_US_TOLERANCE);

  return ISRRAN_SUCCESS;
}

void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "recov")) != -1) {
    switch (opt) {
      case 'r':
        cell.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'e':
        cell.cp = ISRRAN_CP_EXT;
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
  isr_test_context_t context = {};
  int                ret     = ISRRAN_SUCCESS;

  parse_args(argc, argv);

  if (isrran_channel_awgn_init(&channel, 123456789) != ISRRAN_SUCCESS) {
    ret = ISRRAN_ERROR;
  }

  if (!isnan(snr_db) && !isinf(snr_db)) {
    n0_dbm = 30.0f - snr_db;
    isrran_channel_awgn_set_n0(&channel, n0_dbm - 33.0f);
  }

  for (isr_cfg.bw_cfg = 0; isr_cfg.bw_cfg < 8 && !ret; isr_cfg.bw_cfg++) {
    for (isr_cfg.B = 0; isr_cfg.B < 4 && !ret; isr_cfg.B++) {
      for (isr_cfg.n_isr = 0; isr_cfg.n_isr < 8 && !ret; isr_cfg.n_isr++) {
        // Initialise context
        ret = isr_test_context_init(&context);
        if (ret) {
          printf("Failed setting context: bw_cfg=%d; B=%d; n_isr=%d;\n", isr_cfg.bw_cfg, isr_cfg.B, isr_cfg.n_isr);
        }

        for (uint32_t sf_config = 0; sf_config < SF_CONFIG_COUNT && !ret; sf_config++) {
          isr_cfg.subframe_config = sf_config_values[sf_config];

          for (isr_cfg.b_hop = 0; isr_cfg.b_hop < 4 && !ret; isr_cfg.b_hop++) {
            for (uint32_t i_isr = 0; i_isr < I_ISR_COUNT && !ret; i_isr++) {
              isr_cfg.I_isr = i_isr_values[i_isr];

              // Run actual test
              ret = isr_test_context_run(&context);
              if (!ret) {
                test_counter++;
              }
            }
          }
        }

        // Free context
        isr_test_context_free(&context);
      }
    }
  }

  isrran_channel_awgn_free(&channel);

  if (!ret) {
    printf("OK, %d test passed successfully.\n", test_counter);
  } else {
    printf("Failed at test %d.\n", test_counter);
  }

  return ret;
}
