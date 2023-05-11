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

#include "isrran/phy/phch/dci_nr.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/random.h"
#include "isrran/support/isrran_test.h"
#include <getopt.h>

static uint32_t        nof_repetitions = 1024;
static isrran_random_t random_gen      = NULL;

static int test_52prb_base()
{
  // Default configuration with all options disabled
  isrran_dci_cfg_nr_t cfg = {};

  // Set bandwidths
  cfg.coreset0_bw       = 0;
  cfg.bwp_dl_initial_bw = 52;
  cfg.bwp_dl_active_bw  = 52;
  cfg.bwp_ul_initial_bw = 52;
  cfg.bwp_ul_active_bw  = 52;

  // Enable all monitoring
  cfg.monitor_common_0_0  = true;
  cfg.monitor_0_0_and_1_0 = true;
  cfg.monitor_0_1_and_1_1 = true;

  // Common parameters
  cfg.enable_hopping = false;
  cfg.enable_sul     = false;

  // Common DCI 0_1 and 1_1 parameters
  cfg.carrier_indicator_size = 0;
  cfg.harq_ack_codebok       = isrran_pdsch_harq_ack_codebook_dynamic;
  cfg.nof_rb_groups          = 0; // Only for allocation type 0

  // DCI 0_1 parameters
  cfg.nof_ul_bwp                     = 0;
  cfg.nof_ul_time_res                = 2;
  cfg.nof_isr                        = 1;
  cfg.nof_ul_layers                  = 1;
  cfg.pusch_nof_cbg                  = 0;
  cfg.report_trigger_size            = 0;
  cfg.enable_transform_precoding     = false;
  cfg.dynamic_dual_harq_ack_codebook = false;
  cfg.pusch_tx_config_non_codebook   = false;
  cfg.pusch_ptrs                     = false;
  cfg.pusch_dynamic_betas            = false;
  cfg.pusch_alloc_type               = isrran_resource_alloc_type1;
  cfg.pusch_dmrs_type                = isrran_dmrs_sch_type_1;
  cfg.pusch_dmrs_max_len             = isrran_dmrs_sch_len_1;

  // DCI 1_1 parameters
  cfg.nof_dl_bwp             = 0;
  cfg.nof_dl_time_res        = 1;
  cfg.nof_aperiodic_zp       = 0;
  cfg.pdsch_nof_cbg          = 0;
  cfg.nof_dl_to_ul_ack       = 5;
  cfg.pdsch_inter_prb_to_prb = false;
  cfg.pdsch_rm_pattern1      = false;
  cfg.pdsch_rm_pattern2      = false;
  cfg.pdsch_2cw              = false;
  cfg.multiple_scell         = false;
  cfg.pdsch_tci              = false;
  cfg.pdsch_cbg_flush        = false;
  cfg.pdsch_dynamic_bundling = false;
  cfg.pdsch_alloc_type       = isrran_resource_alloc_type1;
  cfg.pdsch_dmrs_type        = isrran_dmrs_sch_type_1;
  cfg.pdsch_dmrs_max_len     = isrran_dmrs_sch_len_1;

  // Configure DCI
  isrran_dci_nr_t dci = {};
  TESTASSERT(isrran_dci_nr_set_cfg(&dci, &cfg) == ISRRAN_SUCCESS);

  // Check DCI sizes
  TESTASSERT(isrran_dci_nr_size(&dci, isrran_search_space_type_common_3, isrran_dci_format_nr_0_0) == 39);
  TESTASSERT(isrran_dci_nr_size(&dci, isrran_search_space_type_common_3, isrran_dci_format_nr_1_0) == 39);
  TESTASSERT(isrran_dci_nr_size(&dci, isrran_search_space_type_ue, isrran_dci_format_nr_0_0) == 39);
  TESTASSERT(isrran_dci_nr_size(&dci, isrran_search_space_type_ue, isrran_dci_format_nr_1_0) == 39);
  TESTASSERT(isrran_dci_nr_size(&dci, isrran_search_space_type_ue, isrran_dci_format_nr_0_1) == 36);
  TESTASSERT(isrran_dci_nr_size(&dci, isrran_search_space_type_ue, isrran_dci_format_nr_1_1) == 41);
  TESTASSERT(isrran_dci_nr_size(&dci, isrran_search_space_type_rar, isrran_dci_format_nr_rar) == 27);
  TESTASSERT(isrran_dci_nr_size(&dci, isrran_search_space_type_common_0, isrran_dci_format_nr_1_0) == 39);

  isrran_dci_ctx_t ctx = {};

  ctx.rnti             = 0x1234;
  ctx.ss_type          = isrran_search_space_type_common_3;
  ctx.rnti_type        = isrran_rnti_type_c;

  // Test UL DCI 0_0 Packing/Unpacking and info
  ctx.format = isrran_dci_format_nr_0_0;
  for (uint32_t i = 0; i < nof_repetitions; i++) {
    isrran_dci_ul_nr_t dci_tx    = {};
    dci_tx.ctx                   = ctx;
    dci_tx.freq_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 11U) - 1);
    dci_tx.time_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)cfg.nof_ul_time_res - 1);
    dci_tx.freq_hopping_flag     = 0; // unavailable
    dci_tx.mcs                   = isrran_random_uniform_int_dist(random_gen, 0, 31);
    dci_tx.rv                    = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.ndi                   = isrran_random_uniform_int_dist(random_gen, 0, 1);
    dci_tx.pid                   = isrran_random_uniform_int_dist(random_gen, 0, 15);
    dci_tx.tpc                   = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.frequency_offset      = 0; // unavailable
    dci_tx.csi_request           = 0; // unavailable
    dci_tx.sul                   = 0; // unavailable
    dci_tx.cc_id                 = 0; // unavailable
    dci_tx.bwp_id                = 0; // unavailable
    dci_tx.dai1                  = 0; // unavailable
    dci_tx.dai2                  = 0; // unavailable
    dci_tx.isr_id                = 0; // unavailable
    dci_tx.ports                 = 0; // unavailable
    dci_tx.isr_request           = 0; // unavailable
    dci_tx.cbg_info              = 0; // unavailable
    dci_tx.ptrs_id               = 0; // unavailable
    dci_tx.beta_id               = 0; // unavailable
    dci_tx.dmrs_id               = 0; // unavailable
    dci_tx.ulsch                 = 0; // unavailable

    // Pack
    isrran_dci_msg_nr_t dci_msg = {};
    TESTASSERT(isrran_dci_nr_ul_pack(&dci, &dci_tx, &dci_msg) == ISRRAN_SUCCESS);

    // Unpack
    isrran_dci_ul_nr_t dci_rx = {};
    TESTASSERT(isrran_dci_nr_ul_unpack(&dci, &dci_msg, &dci_rx) == ISRRAN_SUCCESS);

    // To string
    char str[512];
    TESTASSERT(isrran_dci_ul_nr_to_str(&dci, &dci_tx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Tx: %s", str);
    TESTASSERT(isrran_dci_ul_nr_to_str(&dci, &dci_rx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Rx: %s", str);

    // Assert
    TESTASSERT(memcmp(&dci_tx, &dci_rx, sizeof(isrran_dci_ul_nr_t)) == 0);
  }

  // Test UL DCI 0_1 Packing/Unpacking and info
  ctx.ss_type = isrran_search_space_type_ue;
  ctx.format  = isrran_dci_format_nr_0_1;
  for (uint32_t i = 0; i < nof_repetitions; i++) {
    isrran_dci_ul_nr_t dci_tx    = {};
    dci_tx.ctx                   = ctx;
    dci_tx.freq_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 11U) - 1);
    dci_tx.time_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)cfg.nof_ul_time_res - 1);
    dci_tx.freq_hopping_flag     = 0; // unavailable
    dci_tx.mcs                   = isrran_random_uniform_int_dist(random_gen, 0, 31);
    dci_tx.rv                    = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.ndi                   = isrran_random_uniform_int_dist(random_gen, 0, 1);
    dci_tx.pid                   = isrran_random_uniform_int_dist(random_gen, 0, 15);
    dci_tx.tpc                   = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.frequency_offset      = 0; // unavailable
    dci_tx.csi_request           = 0; // unavailable
    dci_tx.sul                   = 0; // unavailable
    dci_tx.cc_id                 = 0; // unavailable
    dci_tx.bwp_id                = 0; // unavailable
    dci_tx.dai1                  = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.dai2                  = 0; // unavailable
    dci_tx.isr_id                = 0; // unavailable
    dci_tx.ports                 = isrran_random_uniform_int_dist(random_gen, 0, 7);
    dci_tx.isr_request           = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.cbg_info              = 0; // unavailable
    dci_tx.ptrs_id               = 0; // unavailable
    dci_tx.beta_id               = 0; // unavailable
    dci_tx.dmrs_id               = isrran_random_uniform_int_dist(random_gen, 0, 1);
    dci_tx.ulsch                 = isrran_random_uniform_int_dist(random_gen, 0, 1);

    // Pack
    isrran_dci_msg_nr_t dci_msg = {};
    TESTASSERT(isrran_dci_nr_ul_pack(&dci, &dci_tx, &dci_msg) == ISRRAN_SUCCESS);

    // Unpack
    isrran_dci_ul_nr_t dci_rx = {};
    TESTASSERT(isrran_dci_nr_ul_unpack(&dci, &dci_msg, &dci_rx) == ISRRAN_SUCCESS);

    // To string
    char str[512];
    TESTASSERT(isrran_dci_ul_nr_to_str(&dci, &dci_tx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Tx: %s", str);
    TESTASSERT(isrran_dci_ul_nr_to_str(&dci, &dci_rx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Rx: %s", str);

    // Assert
    TESTASSERT(memcmp(&dci_tx, &dci_rx, sizeof(isrran_dci_ul_nr_t)) == 0);
  }

  // Test UL DCI RAR Packing/Unpacking and info
  ctx.ss_type = isrran_search_space_type_rar;
  ctx.format  = isrran_dci_format_nr_rar;
  for (uint32_t i = 0; i < nof_repetitions; i++) {
    isrran_dci_ul_nr_t dci_tx    = {};
    dci_tx.ctx                   = ctx;
    dci_tx.freq_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 14U) - 1); // 14 bit
    dci_tx.time_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 4U) - 1);  // 4 bit
    dci_tx.freq_hopping_flag     = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 1U) - 1);  // 1 bit
    dci_tx.mcs                   = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 4U) - 1);  // 4 bit
    dci_tx.rv                    = 0;                                                                   // unavailable
    dci_tx.ndi                   = 0;                                                                   // unavailable
    dci_tx.pid                   = 0;                                                                   // unavailable
    dci_tx.tpc                   = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 3U) - 1);  // 3 bit
    dci_tx.frequency_offset      = 0;                                                                   // unavailable
    dci_tx.csi_request           = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 1U) - 1);  // 1 bit
    dci_tx.sul                   = 0;                                                                   // unavailable
    dci_tx.cc_id                 = 0;                                                                   // unavailable
    dci_tx.bwp_id                = 0;                                                                   // unavailable
    dci_tx.dai1                  = 0;                                                                   // unavailable
    dci_tx.dai2                  = 0;                                                                   // unavailable
    dci_tx.isr_id                = 0;                                                                   // unavailable
    dci_tx.ports                 = 0;                                                                   // unavailabale
    dci_tx.isr_request           = 0;                                                                   // unavailabale
    dci_tx.cbg_info              = 0;                                                                   // unavailable
    dci_tx.ptrs_id               = 0;                                                                   // unavailable
    dci_tx.beta_id               = 0;                                                                   // unavailable
    dci_tx.dmrs_id               = 0;                                                                   // unavailabale
    dci_tx.ulsch                 = 0;                                                                   // unavailabale

    // Pack
    isrran_dci_msg_nr_t dci_msg = {};
    TESTASSERT(isrran_dci_nr_ul_pack(&dci, &dci_tx, &dci_msg) == ISRRAN_SUCCESS);

    // Unpack
    isrran_dci_ul_nr_t dci_rx = {};
    TESTASSERT(isrran_dci_nr_ul_unpack(&dci, &dci_msg, &dci_rx) == ISRRAN_SUCCESS);

    // To string
    char str[512];
    TESTASSERT(isrran_dci_ul_nr_to_str(&dci, &dci_tx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Tx: %s", str);
    TESTASSERT(isrran_dci_ul_nr_to_str(&dci, &dci_rx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Rx: %s", str);

    // Assert
    TESTASSERT(memcmp(&dci_tx, &dci_rx, sizeof(isrran_dci_ul_nr_t)) == 0);
  }

  // Test UL DCI 1_0 Packing/Unpacking and info
  ctx.format = isrran_dci_format_nr_1_0;
  for (uint32_t i = 0; i < nof_repetitions; i++) {
    isrran_dci_dl_nr_t dci_tx    = {};
    dci_tx.ctx                   = ctx;
    dci_tx.freq_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 11U) - 1);
    dci_tx.time_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)cfg.nof_ul_time_res - 1);
    dci_tx.vrb_to_prb_mapping    = 0; // unavailable
    dci_tx.mcs                   = isrran_random_uniform_int_dist(random_gen, 0, 31);
    dci_tx.rv                    = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.ndi                   = isrran_random_uniform_int_dist(random_gen, 0, 1);
    dci_tx.pid                   = isrran_random_uniform_int_dist(random_gen, 0, 15);
    dci_tx.tpc                   = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.pucch_resource        = isrran_random_uniform_int_dist(random_gen, 0, 7);
    dci_tx.harq_feedback         = isrran_random_uniform_int_dist(random_gen, 0, 7);
    dci_tx.smi                   = 0; // unavailable
    dci_tx.sm                    = 0; // unavailable
    dci_tx.tb_scaling            = 0; // unavailable
    dci_tx.sii                   = 0; // unavailable
    dci_tx.mcs2                  = 0; // unavailable
    dci_tx.ndi2                  = 0; // unavailable
    dci_tx.rv2                   = 0; // unavailable
    dci_tx.cc_id                 = 0; // unavailable
    dci_tx.bwp_id                = 0; // unavailable
    dci_tx.rm_pattern1           = 0; // unavailable
    dci_tx.rm_pattern2           = 0; // unavailable
    dci_tx.zp_csi_rs_id          = 0; // unavailable
    dci_tx.ports                 = 0; // unavailable
    dci_tx.tci                   = 0; // unavailable
    dci_tx.isr_request           = 0; // unavailable
    dci_tx.cbg_info              = 0; // unavailable
    dci_tx.cbg_flush             = 0; // unavailable
    dci_tx.dmrs_id               = 0; // unavailable

    // Pack
    isrran_dci_msg_nr_t dci_msg = {};
    TESTASSERT(isrran_dci_nr_dl_pack(&dci, &dci_tx, &dci_msg) == ISRRAN_SUCCESS);

    // Unpack
    isrran_dci_dl_nr_t dci_rx = {};
    TESTASSERT(isrran_dci_nr_dl_unpack(&dci, &dci_msg, &dci_rx) == ISRRAN_SUCCESS);

    // To string
    char str[512];
    TESTASSERT(isrran_dci_dl_nr_to_str(&dci, &dci_tx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Tx: %s", str);
    TESTASSERT(isrran_dci_dl_nr_to_str(&dci, &dci_rx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Rx: %s", str);

    // Assert
    TESTASSERT(memcmp(&dci_tx, &dci_rx, sizeof(isrran_dci_dl_nr_t)) == 0);
  }

  // Test UL DCI 1_1 Packing/Unpacking and info
  ctx.format = isrran_dci_format_nr_1_1;
  for (uint32_t i = 0; i < nof_repetitions; i++) {
    isrran_dci_dl_nr_t dci_tx    = {};
    dci_tx.ctx                   = ctx;
    dci_tx.freq_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)(1U << 11U) - 1);
    dci_tx.time_domain_assigment = isrran_random_uniform_int_dist(random_gen, 0, (int)cfg.nof_dl_time_res - 1);
    dci_tx.vrb_to_prb_mapping    = 0; // unavailable
    dci_tx.mcs                   = isrran_random_uniform_int_dist(random_gen, 0, 31);
    dci_tx.rv                    = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.ndi                   = isrran_random_uniform_int_dist(random_gen, 0, 1);
    dci_tx.pid                   = isrran_random_uniform_int_dist(random_gen, 0, 15);
    dci_tx.tpc                   = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.pucch_resource        = isrran_random_uniform_int_dist(random_gen, 0, 7);
    dci_tx.harq_feedback         = isrran_random_uniform_int_dist(random_gen, 0, 7);
    dci_tx.smi                   = 0; // unavailable
    dci_tx.sm                    = 0; // unavailable
    dci_tx.tb_scaling            = 0; // unavailable
    dci_tx.sii                   = 0; // unavailable
    dci_tx.mcs2                  = 0; // unavailable
    dci_tx.ndi2                  = 0; // unavailable
    dci_tx.rv2                   = 0; // unavailable
    dci_tx.cc_id                 = 0; // unavailable
    dci_tx.bwp_id                = 0; // unavailable
    dci_tx.rm_pattern1           = 0; // unavailable
    dci_tx.rm_pattern2           = 0; // unavailable
    dci_tx.zp_csi_rs_id          = 0; // unavailable
    dci_tx.ports                 = isrran_random_uniform_int_dist(random_gen, 0, 7);
    dci_tx.tci                   = 0; // unavailable
    dci_tx.isr_request           = 0; // unavailable
    dci_tx.cbg_info              = 0; // unavailable
    dci_tx.cbg_flush             = 0; // unavailable
    dci_tx.dmrs_id               = 0; // unavailable

    // Pack
    isrran_dci_msg_nr_t dci_msg = {};
    TESTASSERT(isrran_dci_nr_dl_pack(&dci, &dci_tx, &dci_msg) == ISRRAN_SUCCESS);

    // Unpack
    isrran_dci_dl_nr_t dci_rx = {};
    TESTASSERT(isrran_dci_nr_dl_unpack(&dci, &dci_msg, &dci_rx) == ISRRAN_SUCCESS);

    // To string
    char str[512];
    TESTASSERT(isrran_dci_dl_nr_to_str(&dci, &dci_tx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Tx: %s", str);
    TESTASSERT(isrran_dci_dl_nr_to_str(&dci, &dci_rx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Rx: %s", str);

    // Assert
    TESTASSERT(memcmp(&dci_tx, &dci_rx, sizeof(isrran_dci_dl_nr_t)) == 0);
  }

  // Test DL DCI 1_0 Packing/Unpacking and info for SI-RNTI
  ctx.format          = isrran_dci_format_nr_1_0;
  ctx.rnti            = 0xffff;
  ctx.ss_type         = isrran_search_space_type_common_0;
  ctx.rnti_type       = isrran_rnti_type_si;
  dci.cfg.coreset0_bw = 48;

  for (uint32_t i = 0; i < nof_repetitions; i++) {
    isrran_dci_dl_nr_t dci_tx    = {};
    dci_tx.ctx                   = ctx;
    dci_tx.freq_domain_assigment = 0x120;
    dci_tx.time_domain_assigment = 0;
    dci_tx.vrb_to_prb_mapping    = 0;
    dci_tx.mcs                   = isrran_random_uniform_int_dist(random_gen, 0, 31);
    dci_tx.rv                    = isrran_random_uniform_int_dist(random_gen, 0, 3);
    dci_tx.sii                   = 1; // bit set to 1 indicates SI message other than SIB1
    dci_tx.coreset0_bw           = 48;

    // Pack
    isrran_dci_msg_nr_t dci_msg = {};
    TESTASSERT(isrran_dci_nr_dl_pack(&dci, &dci_tx, &dci_msg) == ISRRAN_SUCCESS);

    // Unpack
    isrran_dci_dl_nr_t dci_rx = {};
    TESTASSERT(isrran_dci_nr_dl_unpack(&dci, &dci_msg, &dci_rx) == ISRRAN_SUCCESS);

    // To string
    char str[512];
    TESTASSERT(isrran_dci_dl_nr_to_str(&dci, &dci_tx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Tx: %s", str);
    TESTASSERT(isrran_dci_dl_nr_to_str(&dci, &dci_rx, str, (uint32_t)sizeof(str)) != 0);
    INFO("Rx: %s", str);

    // Assert
    TESTASSERT(memcmp(&dci_tx, &dci_rx, sizeof(isrran_dci_dl_nr_t)) == 0);
  }

  return ISRRAN_SUCCESS;
}

static void usage(char* prog)
{
  printf("Usage: %s [cpndv]\n", prog);
  printf("\t-v Increase verbose [default none]\n");
  printf("\t-R Set number of Packing/Unpacking [default %d]\n", nof_repetitions);
}

static void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "vR")) != -1) {
    switch (opt) {
      case 'v':
        increase_isrran_verbose_level();
        break;
      case 'R':
        nof_repetitions = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
}

int main(int argc, char** argv)
{
  parse_args(argc, argv);

  int ret    = ISRRAN_ERROR;
  random_gen = isrran_random_init(1234);

  if (test_52prb_base() < ISRRAN_SUCCESS) {
    goto clean_exit;
  }

  ret = ISRRAN_SUCCESS;

clean_exit:
  isrran_random_free(random_gen);
  return ret;
}
