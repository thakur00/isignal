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
#include "isrran/phy/channel/ch_awgn.h"
#include "isrran/phy/sync/ssb.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <complex.h>
#include <getopt.h>
#include <isrran/phy/utils/random.h>
#include <stdlib.h>

#define SSB_DECODE_TEST_PCI_STRIDE 53
#define SSB_DECODE_TEST_SSB_STRIDE 3

// NR parameters
static uint32_t                    carrier_nof_prb = 52;
static isrran_subcarrier_spacing_t carrier_scs     = isrran_subcarrier_spacing_15kHz;
static double                      carrier_freq_hz = 3.5e9 + 960e3;
static isrran_subcarrier_spacing_t ssb_scs         = isrran_subcarrier_spacing_30kHz;
static double                      ssb_freq_hz     = 3.5e9;
static isrran_ssb_pattern_t        ssb_pattern     = ISRRAN_SSB_PATTERN_A;

// Channel parameters
static cf_t    wideband_gain = 1.0f + 0.5 * I;
static int32_t delay_n       = 1;
static float   cfo_hz        = 1000.0f;
static float   n0_dB         = -10.0f;

// Test context
static isrran_random_t       random_gen = NULL;
static isrran_channel_awgn_t awgn       = {};
static double                srate_hz   = 0.0f; // Base-band sampling rate
static uint32_t              hf_len     = 0;    // Half-frame length
static cf_t*                 buffer     = NULL; // Base-band buffer

static void usage(char* prog)
{
  printf("Usage: %s [v]\n", prog);
  printf("\t-s SSB subcarrier spacing [default, %s kHz]\n", isrran_subcarrier_spacing_to_str(ssb_scs));
  printf("\t-f SSB center frequency [default, %.3f MHz]\n", ssb_freq_hz / 1e6);
  printf("\t-S cell/carrier subcarrier spacing [default, %s kHz]\n", isrran_subcarrier_spacing_to_str(carrier_scs));
  printf("\t-F cell/carrier center frequency in Hz [default, %.3f MHz]\n", carrier_freq_hz / 1e6);
  printf("\t-P SSB pattern [default, %s]\n", isrran_ssb_pattern_to_str(ssb_pattern));
  printf("\t-v [set isrran_verbose to debug, default none]\n");
}

static void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "SsFfPv")) != -1) {
    switch (opt) {
      case 's':
        ssb_scs = isrran_subcarrier_spacing_from_str(argv[optind]);
        if (ssb_scs == isrran_subcarrier_spacing_invalid) {
          ERROR("Invalid SSB subcarrier spacing %s\n", argv[optind]);
          exit(-1);
        }
        break;
      case 'f':
        ssb_freq_hz = strtod(argv[optind], NULL);
        break;
      case 'S':
        carrier_scs = isrran_subcarrier_spacing_from_str(argv[optind]);
        if (carrier_scs == isrran_subcarrier_spacing_invalid) {
          ERROR("Invalid Cell/Carrier subcarrier spacing %s\n", argv[optind]);
          exit(-1);
        }
        break;
      case 'F':
        carrier_freq_hz = strtod(argv[optind], NULL);
        break;
      case 'P':
        ssb_pattern = isrran_ssb_pattern_fom_str(argv[optind]);
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

static void run_channel()
{
  // Delay
  for (uint32_t i = 0; i < hf_len; i++) {
    buffer[i] = buffer[(i + delay_n) % hf_len];
  }

  // CFO
  isrran_vec_apply_cfo(buffer, -cfo_hz / srate_hz, buffer, hf_len);

  // AWGN
  isrran_channel_awgn_run_c(&awgn, buffer, buffer, hf_len);

  // Wideband gain
  isrran_vec_sc_prod_ccc(buffer, wideband_gain, buffer, hf_len);
}

static void gen_pbch_msg(isrran_pbch_msg_nr_t* pbch_msg, uint32_t ssb_idx)
{
  // Default all to zero
  ISRRAN_MEM_ZERO(pbch_msg, isrran_pbch_msg_nr_t, 1);

  // Generate payload
  isrran_random_bit_vector(random_gen, pbch_msg->payload, ISRRAN_PBCH_MSG_NR_SZ);

  pbch_msg->ssb_idx = ssb_idx;
  pbch_msg->crc     = true;
}

static int test_case_true(isrran_ssb_t* ssb)
{
  // For benchmarking purposes
  uint64_t t_encode_usec = 0;
  uint64_t t_decode_usec = 0;
  uint64_t t_search_usec = 0;

  // SSB configuration
  isrran_ssb_cfg_t ssb_cfg = {};
  ssb_cfg.srate_hz         = srate_hz;
  ssb_cfg.center_freq_hz   = carrier_freq_hz;
  ssb_cfg.ssb_freq_hz      = ssb_freq_hz;
  ssb_cfg.scs              = ssb_scs;
  ssb_cfg.pattern          = ssb_pattern;

  TESTASSERT(isrran_ssb_set_cfg(ssb, &ssb_cfg) == ISRRAN_SUCCESS);

  // For each PCI...
  uint64_t count = 0;
  for (uint32_t pci = 0; pci < ISRRAN_NOF_NID_NR; pci += SSB_DECODE_TEST_PCI_STRIDE) {
    for (uint32_t ssb_idx = 0; ssb_idx < ssb->Lmax; ssb_idx += SSB_DECODE_TEST_SSB_STRIDE, count++) {
      struct timeval t[3] = {};

      // Build PBCH message
      isrran_pbch_msg_nr_t pbch_msg_tx = {};
      gen_pbch_msg(&pbch_msg_tx, ssb_idx);

      // Print encoded PBCH message
      char str[512] = {};
      isrran_pbch_msg_info(&pbch_msg_tx, str, sizeof(str));
      INFO("test_case_true - encoded pci=%d %s", pci, str);

      // Initialise baseband
      isrran_vec_cf_zero(buffer, hf_len);

      // Add the SSB base-band
      gettimeofday(&t[1], NULL);
      TESTASSERT(isrran_ssb_add(ssb, pci, &pbch_msg_tx, buffer, buffer) == ISRRAN_SUCCESS);
      gettimeofday(&t[2], NULL);
      get_time_interval(t);
      t_encode_usec += t[0].tv_usec + t[0].tv_sec * 1000000UL;

      // Run channel
      run_channel();

      // Decode
      gettimeofday(&t[1], NULL);
      isrran_pbch_msg_nr_t pbch_msg_rx = {};
      TESTASSERT(isrran_ssb_decode_pbch(ssb, pci, pbch_msg_tx.hrf, pbch_msg_tx.ssb_idx, buffer, &pbch_msg_rx) ==
                 ISRRAN_SUCCESS);
      gettimeofday(&t[2], NULL);
      get_time_interval(t);
      t_decode_usec += t[0].tv_usec + t[0].tv_sec * 1000000UL;

      // Print decoded PBCH message
      isrran_pbch_msg_info(&pbch_msg_rx, str, sizeof(str));
      INFO("test_case_true - decoded pci=%d %s crc=%s", pci, str, pbch_msg_rx.crc ? "OK" : "KO");

      // Assert PBCH message CRC
      TESTASSERT(pbch_msg_rx.crc);
      TESTASSERT(memcmp(&pbch_msg_rx, &pbch_msg_tx, sizeof(isrran_pbch_msg_nr_t)) == 0);

      // Search
      isrran_ssb_search_res_t res = {};
      gettimeofday(&t[1], NULL);
      TESTASSERT(isrran_ssb_search(ssb, buffer, hf_len, &res) == ISRRAN_SUCCESS);
      gettimeofday(&t[2], NULL);
      get_time_interval(t);
      t_search_usec += t[0].tv_usec + t[0].tv_sec * 1000000UL;

      // Print decoded PBCH message
      isrran_pbch_msg_info(&res.pbch_msg, str, sizeof(str));
      INFO("test_case_true - found   pci=%d %s crc=%s", res.N_id, str, res.pbch_msg.crc ? "OK" : "KO");

      // Assert PBCH message CRC
      TESTASSERT(res.pbch_msg.crc);
      TESTASSERT(memcmp(&res.pbch_msg, &pbch_msg_tx, sizeof(isrran_pbch_msg_nr_t)) == 0);
    }
  }

  if (!count) {
    ERROR("Error in test case true: undefined division");
    return ISRRAN_ERROR;
  }

  INFO("test_case_true - %.1f usec/encode; %.1f usec/decode; %.1f usec/decode;",
       (double)t_encode_usec / (double)(count),
       (double)t_decode_usec / (double)(count),
       (double)t_search_usec / (double)(count));

  return ISRRAN_SUCCESS;
}

static int test_case_false(isrran_ssb_t* ssb)
{
  // For benchmarking purposes
  uint64_t t_decode_usec = 0;
  uint64_t t_search_usec = 0;

  // SSB configuration
  isrran_ssb_cfg_t ssb_cfg = {};
  ssb_cfg.srate_hz         = srate_hz;
  ssb_cfg.center_freq_hz   = carrier_freq_hz;
  ssb_cfg.ssb_freq_hz      = ssb_freq_hz;
  ssb_cfg.scs              = ssb_scs;
  ssb_cfg.pattern          = ssb_pattern;

  TESTASSERT(isrran_ssb_set_cfg(ssb, &ssb_cfg) == ISRRAN_SUCCESS);

  // For each PCI...
  uint32_t count = 0;
  for (uint32_t pci = 0; pci < ISRRAN_NOF_NID_NR; pci += SSB_DECODE_TEST_PCI_STRIDE, count++) {
    struct timeval t[3] = {};

    // Initialise baseband
    isrran_vec_cf_zero(buffer, hf_len);

    // Channel, as it is zero it only adds noise
    isrran_channel_awgn_run_c(&awgn, buffer, buffer, hf_len);

    // Decode
    gettimeofday(&t[1], NULL);
    isrran_pbch_msg_nr_t pbch_msg_rx = {};
    TESTASSERT(isrran_ssb_decode_pbch(ssb, pci, false, 0, buffer, &pbch_msg_rx) == ISRRAN_SUCCESS);
    gettimeofday(&t[2], NULL);
    get_time_interval(t);
    t_decode_usec += t[0].tv_usec + t[0].tv_sec * 1000000UL;

    // Print decoded PBCH message
    char str[512] = {};
    isrran_pbch_msg_info(&pbch_msg_rx, str, sizeof(str));
    INFO("test_case_false - decoded pci=%d %s crc=%s", pci, str, pbch_msg_rx.crc ? "OK" : "KO");

    // Assert PBCH message CRC is not okay
    TESTASSERT(!pbch_msg_rx.crc);

    // Search
    isrran_ssb_search_res_t res = {};
    gettimeofday(&t[1], NULL);
    TESTASSERT(isrran_ssb_search(ssb, buffer, hf_len, &res) == ISRRAN_SUCCESS);
    gettimeofday(&t[2], NULL);
    get_time_interval(t);
    t_search_usec += t[0].tv_usec + t[0].tv_sec * 1000000UL;

    // Print decoded PBCH message
    isrran_pbch_msg_info(&res.pbch_msg, str, sizeof(str));
    INFO("test_case_false - false found pci=%d %s crc=%s", res.N_id, str, res.pbch_msg.crc ? "OK" : "KO");

    // Assert PBCH message CRC
    TESTASSERT(!res.pbch_msg.crc);
  }

  if (!count) {
    ERROR("Error in test case true: undefined division");
    return ISRRAN_ERROR;
  }

  INFO("test_case_false - %.1f usec/decode; %.1f usec/decode;",
       (double)t_decode_usec / (double)(count),
       (double)t_search_usec / (double)(count));

  return ISRRAN_SUCCESS;
}

int main(int argc, char** argv)
{
  int ret = ISRRAN_ERROR;
  parse_args(argc, argv);

  random_gen = isrran_random_init(1234);
  srate_hz   = (double)ISRRAN_SUBC_SPACING_NR(carrier_scs) * isrran_min_symbol_sz_rb(carrier_nof_prb);
  hf_len     = (uint32_t)ceil(srate_hz * (5.0 / 1000.0));
  buffer     = isrran_vec_cf_malloc(hf_len);

  isrran_ssb_t      ssb      = {};
  isrran_ssb_args_t ssb_args = {};
  ssb_args.enable_encode     = true;
  ssb_args.enable_decode     = true;
  ssb_args.enable_search     = true;

  if (buffer == NULL) {
    ERROR("Malloc");
    goto clean_exit;
  }

  if (isrran_channel_awgn_init(&awgn, 0x0) < ISRRAN_SUCCESS) {
    ERROR("AWGN");
    goto clean_exit;
  }

  if (isrran_channel_awgn_set_n0(&awgn, n0_dB) < ISRRAN_SUCCESS) {
    ERROR("AWGN");
    goto clean_exit;
  }

  if (isrran_ssb_init(&ssb, &ssb_args) < ISRRAN_SUCCESS) {
    ERROR("Init");
    goto clean_exit;
  }

  if (test_case_true(&ssb) != ISRRAN_SUCCESS) {
    ERROR("test case failed");
    goto clean_exit;
  }

  if (test_case_false(&ssb) != ISRRAN_SUCCESS) {
    ERROR("test case failed");
    goto clean_exit;
  }

  ret = ISRRAN_SUCCESS;

clean_exit:
  isrran_random_free(random_gen);
  isrran_ssb_free(&ssb);

  isrran_channel_awgn_free(&awgn);

  if (buffer) {
    free(buffer);
  }

  return ret;
}