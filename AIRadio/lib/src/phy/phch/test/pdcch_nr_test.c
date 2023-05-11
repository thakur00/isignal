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

#include "isrran/common/test_common.h"
#include "isrran/phy/phch/pdcch_nr.h"
#include "isrran/phy/utils/random.h"
#include <getopt.h>

static isrran_carrier_nr_t carrier = ISRRAN_DEFAULT_CARRIER_NR;

static uint16_t rnti        = 0x1234;
static bool     fast_sweep  = true;
static bool     interleaved = false;

typedef struct {
  uint64_t time_us;
  uint64_t count;
} proc_time_t;

static proc_time_t enc_time[ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR] = {};
static proc_time_t dec_time[ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR] = {};

static int test(isrran_pdcch_nr_t*      tx,
                isrran_pdcch_nr_t*      rx,
                cf_t*                   grid,
                isrran_dmrs_pdcch_ce_t* ce,
                isrran_dci_msg_nr_t*    dci_msg_tx)
{
  // Encode PDCCH
  TESTASSERT(isrran_pdcch_nr_encode(tx, dci_msg_tx, grid) == ISRRAN_SUCCESS);

  enc_time[dci_msg_tx->ctx.location.L].time_us += tx->meas_time_us;
  enc_time[dci_msg_tx->ctx.location.L].count++;

  // Init Rx MSG
  isrran_pdcch_nr_res_t res        = {};
  isrran_dci_msg_nr_t   dci_msg_rx = *dci_msg_tx;
  isrran_vec_u8_zero(dci_msg_rx.payload, dci_msg_rx.nof_bits);

  // Decode PDCCH
  TESTASSERT(isrran_pdcch_nr_decode(rx, grid, ce, &dci_msg_rx, &res) == ISRRAN_SUCCESS);

  dec_time[dci_msg_tx->ctx.location.L].time_us += rx->meas_time_us;
  dec_time[dci_msg_tx->ctx.location.L].count++;

  // Assert
  TESTASSERT(res.evm < 0.01f);
  TESTASSERT(res.crc);

  return ISRRAN_SUCCESS;
}

static void usage(char* prog)
{
  printf("Usage: %s [pFIv] \n", prog);
  printf("\t-p Number of carrier PRB [Default %d]\n", carrier.nof_prb);
  printf("\t-F Fast CORESET frequency resource sweeping [Default %s]\n", fast_sweep ? "Enabled" : "Disabled");
  printf("\t-I Enable interleaved CCE-to-REG [Default %s]\n", interleaved ? "Enabled" : "Disabled");
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

static int parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "pFIv")) != -1) {
    switch (opt) {
      case 'p':
        carrier.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'F':
        fast_sweep ^= true;
        break;
      case 'I':
        interleaved ^= true;
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
  int ret = ISRRAN_ERROR;

  isrran_pdcch_nr_args_t args = {};
  args.disable_simd           = false;
  args.measure_evm            = true;
  args.measure_time           = true;

  isrran_pdcch_nr_t pdcch_tx = {};
  isrran_pdcch_nr_t pdcch_rx = {};

  if (parse_args(argc, argv) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  uint32_t                grid_sz  = carrier.nof_prb * ISRRAN_NRE * ISRRAN_NSYMB_PER_SLOT_NR;
  isrran_random_t         rand_gen = isrran_random_init(1234);
  isrran_dmrs_pdcch_ce_t* ce       = ISRRAN_MEM_ALLOC(isrran_dmrs_pdcch_ce_t, 1);
  cf_t*                   buffer   = isrran_vec_cf_malloc(grid_sz);
  if (rand_gen == NULL || ce == NULL || buffer == NULL) {
    ERROR("Error malloc");
    goto clean_exit;
  }

  ISRRAN_MEM_ZERO(ce, isrran_dmrs_pdcch_ce_t, 1);

  if (isrran_pdcch_nr_init_tx(&pdcch_tx, &args) < ISRRAN_SUCCESS) {
    ERROR("Error init");
    goto clean_exit;
  }

  if (isrran_pdcch_nr_init_rx(&pdcch_rx, &args) < ISRRAN_SUCCESS) {
    ERROR("Error init");
    goto clean_exit;
  }

  isrran_coreset_t coreset = {};
  if (interleaved) {
    coreset.mapping_type         = isrran_coreset_mapping_type_interleaved;
    coreset.reg_bundle_size      = isrran_coreset_bundle_size_n6;
    coreset.interleaver_size     = isrran_coreset_bundle_size_n2;
    coreset.precoder_granularity = isrran_coreset_precoder_granularity_reg_bundle;
    coreset.shift_index          = carrier.pci;

    carrier.nof_prb = 52;
    carrier.pci     = 500;
  }

  uint32_t nof_frequency_resource = ISRRAN_MIN(ISRRAN_CORESET_FREQ_DOMAIN_RES_SIZE, carrier.nof_prb / 6);
  for (uint32_t frequency_resources = 1; frequency_resources < (1U << nof_frequency_resource);
       frequency_resources          = (fast_sweep) ? ((frequency_resources << 1U) | 1U) : (frequency_resources + 1)) {
    for (uint32_t i = 0; i < nof_frequency_resource; i++) {
      uint32_t mask             = ((frequency_resources >> i) & 1U);
      coreset.freq_resources[i] = (mask == 1);
    }

    for (coreset.duration = ISRRAN_CORESET_DURATION_MIN; coreset.duration <= ISRRAN_CORESET_DURATION_MAX;
         coreset.duration++) {
      // Skip case if CORESET bandwidth is not enough
      uint32_t N = isrran_coreset_get_bw(&coreset) * coreset.duration;
      if (interleaved && N % 12 != 0) {
        continue;
      }

      isrran_search_space_t search_space               = {};
      search_space.type                                = isrran_search_space_type_ue;
      search_space.formats[search_space.nof_formats++] = isrran_dci_format_nr_0_0;
      search_space.formats[search_space.nof_formats++] = isrran_dci_format_nr_1_0;

      isrran_dci_cfg_nr_t dci_cfg = {};
      dci_cfg.coreset0_bw         = 0;
      dci_cfg.bwp_dl_initial_bw   = carrier.nof_prb;
      dci_cfg.bwp_dl_active_bw    = carrier.nof_prb;
      dci_cfg.bwp_ul_initial_bw   = carrier.nof_prb;
      dci_cfg.bwp_ul_active_bw    = carrier.nof_prb;
      dci_cfg.monitor_common_0_0  = true;
      dci_cfg.monitor_0_0_and_1_0 = true;
      dci_cfg.monitor_0_1_and_1_1 = true;

      // Precompute DCI sizes
      isrran_dci_nr_t dci = {};
      if (isrran_dci_nr_set_cfg(&dci, &dci_cfg) < ISRRAN_SUCCESS) {
        ERROR("Error setting DCI configuratio");
        goto clean_exit;
      }

      if (isrran_pdcch_nr_set_carrier(&pdcch_tx, &carrier, &coreset) < ISRRAN_SUCCESS) {
        ERROR("Error setting carrier");
        goto clean_exit;
      }

      if (isrran_pdcch_nr_set_carrier(&pdcch_rx, &carrier, &coreset) < ISRRAN_SUCCESS) {
        ERROR("Error setting carrier");
        goto clean_exit;
      }

      // Fill search space maximum number of candidates
      for (uint32_t aggregation_level = 0; aggregation_level < ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR;
           aggregation_level++) {
        search_space.nof_candidates[aggregation_level] =
            isrran_pdcch_nr_max_candidates_coreset(&coreset, aggregation_level);
      }

      for (uint32_t aggregation_level = 0; aggregation_level < ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR;
           aggregation_level++) {
        uint32_t L = 1U << aggregation_level;

        for (uint32_t slot_idx = 0; slot_idx < ISRRAN_NSLOTS_PER_FRAME_NR(carrier.scs); slot_idx++) {
          uint32_t dci_locations[ISRRAN_SEARCH_SPACE_MAX_NOF_CANDIDATES_NR] = {};

          // Calculate candidate locations
          int n = isrran_pdcch_nr_locations_coreset(
              &coreset, &search_space, rnti, aggregation_level, slot_idx, dci_locations);
          if (n < ISRRAN_SUCCESS) {
            ERROR("Error calculating locations in CORESET");
            goto clean_exit;
          }

          // Skip if no candidates
          if (n == 0) {
            continue;
          }

          for (uint32_t ncce_idx = 0; ncce_idx < n; ncce_idx++) {
            // Init MSG
            isrran_dci_msg_nr_t dci_msg = {};
            dci_msg.ctx.format          = isrran_dci_format_nr_1_0;
            dci_msg.ctx.rnti_type       = isrran_rnti_type_c;
            dci_msg.ctx.location.L      = aggregation_level;
            dci_msg.ctx.location.ncce   = dci_locations[ncce_idx];
            dci_msg.nof_bits            = isrran_dci_nr_size(&dci, search_space.type, isrran_dci_format_nr_1_0);

            // Generate random payload
            for (uint32_t i = 0; i < dci_msg.nof_bits; i++) {
              dci_msg.payload[i] = isrran_random_uniform_int_dist(rand_gen, 0, 1);
            }

            // Set channel estimate number of elements and set out-of-range values to zero
            ce->nof_re = (ISRRAN_NRE - 3) * 6 * L;
            for (uint32_t i = 0; i < ISRRAN_PDCCH_MAX_RE; i++) {
              ce->ce[i] = (i < ce->nof_re) ? 1.0f : 0.0f;
            }
            ce->noise_var = 0.0f;

            if (test(&pdcch_tx, &pdcch_rx, buffer, ce, &dci_msg) < ISRRAN_SUCCESS) {
              ERROR("test failed");
              goto clean_exit;
            }
          }
        }
      }
    }
  }

  printf("+--------+--------+--------+--------+\n");
  printf("| %6s | %6s | %6s | %6s |\n", " ", " ", " Time ", " Time ");
  printf("| %6s | %6s | %6s | %6s |\n", "  L  ", "Count", "Encode", "Decode");
  printf("| %6s | %6s | %6s | %6s |\n", " ", " ", " (us) ", " (us) ");
  printf("+--------+--------+--------+--------+\n");
  for (uint32_t i = 0; i < ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR; i++) {
    if (enc_time[i].count > 0 && dec_time[i].count) {
      printf("| %6" PRIu32 "| %6" PRIu64 " | %6.1f | %6.1f |\n",
             i,
             enc_time[i].count,
             (double)enc_time[i].time_us / (double)enc_time[i].count,
             (double)dec_time[i].time_us / (double)dec_time[i].count);
    }
  }
  printf("+--------+--------+--------+--------+\n");

  ret = ISRRAN_SUCCESS;
clean_exit:
  isrran_random_free(rand_gen);

  if (ce) {
    free(ce);
  }

  if (buffer) {
    free(buffer);
  }

  isrran_pdcch_nr_free(&pdcch_tx);
  isrran_pdcch_nr_free(&pdcch_rx);

  if (ret == ISRRAN_SUCCESS) {
    printf("Passed!\n");
  } else {
    printf("Failed!\n");
  }

  return ret;
}