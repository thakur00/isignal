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
#include "isrran/phy/phch/harq_ack.h"
#include "isrran/phy/utils/debug.h"
#include <getopt.h>

static int test_case_1()
{
  // Set configuration
  isrran_harq_ack_cfg_hl_t cfg = {};
  cfg.harq_ack_codebook        = isrran_pdsch_harq_ack_codebook_dynamic;

  // Generate ACK information
  isrran_pdsch_ack_nr_t ack_info = {};
  ack_info.nof_cc                = 1;
  ack_info.use_pusch             = true;

  isrran_harq_ack_m_t m = {};
  m.value[0]            = 1;
  m.present             = true;

  m.resource.k1       = 8;
  m.resource.v_dai_dl = 0;
  m.value[0]          = 1;
  m.present           = true;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  m.resource.k1       = 5;
  m.resource.v_dai_dl = 2;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  m.resource.k1       = 6;
  m.resource.v_dai_dl = 1;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  m.resource.k1       = 4;
  m.resource.v_dai_dl = 3;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  m.resource.k1       = 3;
  m.resource.v_dai_dl = 0;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  // Print trace
  char str[512] = {};
  TESTASSERT(isrran_harq_ack_info(&ack_info, str, (uint32_t)sizeof(str)) > ISRRAN_SUCCESS);
  INFO("%s", str);

  // Generate UCI data
  isrran_uci_data_nr_t uci_data = {};
  TESTASSERT(isrran_harq_ack_pack(&cfg, &ack_info, &uci_data) == ISRRAN_SUCCESS);

  // Assert UCI data
  TESTASSERT(uci_data.cfg.ack.count == 5);

  return ISRRAN_SUCCESS;
}

static int test_case_2()
{
  // Set configuration
  isrran_harq_ack_cfg_hl_t cfg = {};
  cfg.harq_ack_codebook        = isrran_pdsch_harq_ack_codebook_dynamic;

  // Generate ACK information
  isrran_pdsch_ack_nr_t ack_info = {};
  ack_info.nof_cc                = 1;
  ack_info.use_pusch             = true;

  isrran_harq_ack_m_t m = {};
  m.value[0]            = 1;
  m.present             = true;

  m.resource.k1       = 7;
  m.resource.v_dai_dl = 1;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  m.resource.k1       = 6;
  m.resource.v_dai_dl = 2;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  m.resource.k1       = 8;
  m.resource.v_dai_dl = 0;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  m.resource.k1       = 5;
  m.resource.v_dai_dl = 3;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  m.resource.k1       = 4;
  m.resource.v_dai_dl = 0;
  TESTASSERT(isrran_harq_ack_insert_m(&ack_info, &m) == ISRRAN_SUCCESS);

  // Print trace
  char str[512] = {};
  TESTASSERT(isrran_harq_ack_info(&ack_info, str, (uint32_t)sizeof(str)) > ISRRAN_SUCCESS);
  INFO("%s", str);

  // Generate UCI data
  isrran_uci_data_nr_t uci_data = {};
  TESTASSERT(isrran_harq_ack_pack(&cfg, &ack_info, &uci_data) == ISRRAN_SUCCESS);

  // Assert UCI data
  TESTASSERT(uci_data.cfg.ack.count == 5);

  // Unpack HARQ ACK UCI data
  isrran_pdsch_ack_nr_t ack_info_rx = ack_info;
  TESTASSERT(isrran_harq_ack_unpack(&cfg, &uci_data, &ack_info_rx) == ISRRAN_SUCCESS);

  // Assert unpacked data
  TESTASSERT(memcmp(&ack_info, &ack_info_rx, sizeof(isrran_pdsch_ack_nr_t)) == 0);

  return ISRRAN_SUCCESS;
}

static void usage(char* prog)
{
  printf("Usage: %s [v]\n", prog);
  printf("\t-v Increase isrran_verbose\n");
}

static void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "v")) != -1) {
    switch (opt) {
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
  parse_args(argc, argv);

  // Test only until Format1B - CS
  TESTASSERT(test_case_1() == ISRRAN_SUCCESS);
  TESTASSERT(test_case_2() == ISRRAN_SUCCESS);

  printf("Ok\n");
  return ISRRAN_SUCCESS;
}