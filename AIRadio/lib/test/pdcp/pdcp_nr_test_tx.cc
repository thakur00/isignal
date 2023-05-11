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
#include "pdcp_nr_test.h"
#include <numeric>

/*
 * Generic class to test transmission of in-sequence packets
 */
class test_tx_helper
{
public:
  pdcp_nr_test_helper      pdcp_hlp_tx;
  isrran::pdcp_entity_nr&  pdcp_tx;
  rlc_dummy&               rlc_tx;
  isrue::stack_test_dummy& stack;
  isrlog::basic_logger&    logger;

  test_tx_helper(uint8_t pdcp_sn_len, isrlog::basic_logger& logger) :
    pdcp_hlp_tx({1,
                 isrran::PDCP_RB_IS_DRB,
                 isrran::SECURITY_DIRECTION_UPLINK,
                 isrran::SECURITY_DIRECTION_DOWNLINK,
                 pdcp_sn_len,
                 isrran::pdcp_t_reordering_t::ms500,
                 isrran::pdcp_discard_timer_t::ms500,
                 false,
                 isrran::isrran_rat_t::nr},
                sec_cfg,
                logger),
    pdcp_tx(pdcp_hlp_tx.pdcp),
    rlc_tx(pdcp_hlp_tx.rlc),
    stack(pdcp_hlp_tx.stack),
    logger(logger)
  {}
  int test_tx(uint32_t                     n_packets,
              const pdcp_initial_state&    init_state,
              uint64_t                     n_pdus_exp,
              isrran::unique_byte_buffer_t pdu_exp)
  {
    pdcp_hlp_tx.set_pdcp_initial_state(init_state);

    // Run test
    for (uint32_t i = 0; i < n_packets; ++i) {
      // Test SDU
      isrran::unique_byte_buffer_t sdu = isrran::make_byte_buffer();
      sdu->append_bytes(sdu1, sizeof(sdu1));
      pdcp_hlp_tx.pdcp.write_sdu(std::move(sdu));
    }

    isrran::unique_byte_buffer_t pdu_act = isrran::make_byte_buffer();
    pdcp_hlp_tx.rlc.get_last_sdu(pdu_act);

    TESTASSERT(pdcp_hlp_tx.rlc.rx_count == n_pdus_exp);
    TESTASSERT(compare_two_packets(pdu_act, pdu_exp) == 0);
    return 0;
  }
};

/*
 * TX Test: PDCP Entity with SN LEN = 12 and 18.
 * PDCP entity configured with EIA2 and EEA2
 */
int test_tx_all(isrlog::basic_logger& logger)
{
  uint64_t n_packets;
  /*
   * TX Test 1: PDCP Entity with SN LEN = 12
   * TX_NEXT = 0.
   * Input: {0x18, 0xE2}
   * Output:  {0x80, 0x00, 0x8f, 0xe3, 0xc7, 0x1b, 0xad, 0x14}
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("TX COUNT 0, 12 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_12, logger);
    n_packets                                         = 1;
    isrran::unique_byte_buffer_t pdu_exp_count0_len12 = isrran::make_byte_buffer();
    pdu_exp_count0_len12->append_bytes(pdu1_count0_snlen12, sizeof(pdu1_count0_snlen12));
    TESTASSERT(tx_helper.test_tx(n_packets, normal_init_state, n_packets, std::move(pdu_exp_count0_len12)) == 0);
  }
  /*
   * TX Test 2: PDCP Entity with SN LEN = 12
   * TX_NEXT = 2048.
   * Input: {0x18, 0xE2}
   * Output: {0x88, 0x00, 0x8d, 0x2c, 0xe5, 0x38, 0xc0, 0x42}
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("TX COUNT 2048, 12 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_12, logger);
    n_packets                                            = 2049;
    isrran::unique_byte_buffer_t pdu_exp_count2048_len12 = isrran::make_byte_buffer();
    pdu_exp_count2048_len12->append_bytes(pdu1_count2048_snlen12, sizeof(pdu1_count2048_snlen12));
    TESTASSERT(tx_helper.test_tx(n_packets, normal_init_state, n_packets, std::move(pdu_exp_count2048_len12)) == 0);
  }
  /*
   * TX Test 3: PDCP Entity with SN LEN = 12
   * TX_NEXT = 4096.
   * Input: {0x18, 0xE2}
   * Output: {0x80, 0x00, 0x97, 0xbe, 0xee, 0x62, 0xf5, 0xe0}
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("TX COUNT 4096, 12 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_12, logger);
    n_packets                                            = 4097;
    isrran::unique_byte_buffer_t pdu_exp_count4096_len12 = isrran::make_byte_buffer();
    pdu_exp_count4096_len12->append_bytes(pdu1_count4096_snlen12, sizeof(pdu1_count4096_snlen12));
    TESTASSERT(tx_helper.test_tx(n_packets, normal_init_state, n_packets, std::move(pdu_exp_count4096_len12)) == 0);
  }
  /*
   * TX Test 4: PDCP Entity with SN LEN = 18
   * TX_NEXT = 0.
   * Input: {0x18, 0xE2}
   * Output: {0x80, 0x00, 0x00, 0x8f, 0xe3, 0x37, 0x33, 0xd5, 0x64}
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("TX COUNT 0, 18 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_18, logger);
    n_packets                                         = 1;
    isrran::unique_byte_buffer_t pdu_exp_count0_len18 = isrran::make_byte_buffer();
    pdu_exp_count0_len18->append_bytes(pdu1_count0_snlen18, sizeof(pdu1_count0_snlen18));
    TESTASSERT(tx_helper.test_tx(n_packets, normal_init_state, n_packets, std::move(pdu_exp_count0_len18)) == 0);
  }

  /*
   * TX Test 5: PDCP Entity with SN LEN = 18
   * TX_NEXT = 131072.
   * Input: {0x18, 0xE2}
   * Output: {0x82, 0x00, 0x00, 0x15, 0x01, 0x99, 0x97, 0xe0, 0x4e}
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("TX COUNT 131072, 18 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_18, logger);
    n_packets                                           = 131073;
    isrran::unique_byte_buffer_t pdu_exp_sn131072_len18 = isrran::make_byte_buffer();
    pdu_exp_sn131072_len18->append_bytes(pdu1_count131072_snlen18, sizeof(pdu1_count131072_snlen18));
    TESTASSERT(tx_helper.test_tx(n_packets, normal_init_state, n_packets, std::move(pdu_exp_sn131072_len18)) == 0);
  }

  /*
   * TX Test 6: PDCP Entity with SN LEN = 18
   * TX_NEXT = 262144.
   * Input: {0x18, 0xE2}
   * Output: {0x80, 0x00, 0x00, 0xc2, 0x47, 0xc2, 0xee, 0x46, 0xd9}
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("TX COUNT 262144, 18 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_18, logger);
    n_packets                                              = 262145;
    isrran::unique_byte_buffer_t pdu_exp_count262144_len18 = isrran::make_byte_buffer();
    pdu_exp_count262144_len18->append_bytes(pdu1_count262144_snlen18, sizeof(pdu1_count262144_snlen18));
    TESTASSERT(tx_helper.test_tx(n_packets, normal_init_state, n_packets, std::move(pdu_exp_count262144_len18)) == 0);
  }
  /*
   * TX Test 7: PDCP Entity with SN LEN = 12
   * Test TX at COUNT wraparound.
   * Should print a warning and drop all packets after wrap around.
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("TX COUNT wrap around, 12 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_12, logger);
    n_packets                                                  = 5;
    isrran::unique_byte_buffer_t pdu_exp_count4294967295_len12 = isrran::make_byte_buffer();
    pdu_exp_count4294967295_len12->append_bytes(pdu1_count4294967295_snlen12, sizeof(pdu1_count4294967295_snlen12));
    TESTASSERT(tx_helper.test_tx(n_packets, near_wraparound_init_state, 1, std::move(pdu_exp_count4294967295_len12)) ==
               0);
  }

  /*
   * TX Test 8: PDCP Entity with SN LEN = 18
   * Test TX at COUNT wraparound.
   * Should print a warning and drop all packets after wraparound.
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("TX COUNT wrap around, 12 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_18, logger);
    n_packets                                                  = 5;
    isrran::unique_byte_buffer_t pdu_exp_count4294967295_len18 = isrran::make_byte_buffer();
    pdu_exp_count4294967295_len18->append_bytes(pdu1_count4294967295_snlen18, sizeof(pdu1_count4294967295_snlen18));
    TESTASSERT(tx_helper.test_tx(n_packets, near_wraparound_init_state, 1, std::move(pdu_exp_count4294967295_len18)) ==
               0);
  }

  /*
   * TX Test 9: PDCP Entity with SN LEN = 12
   * Test whether discard timers are correctly stopped after receiving a notification from the RLC
   */
  {
    auto&                       test_logger = isrlog::fetch_basic_logger("TESTER  ");
    isrran::test_delimit_logger delimiter("Stop discard timers upon RLC notification, 12 bit SN");
    test_tx_helper              tx_helper(isrran::PDCP_SN_LEN_12, logger);
    n_packets                                         = 1;
    isrran::unique_byte_buffer_t pdu_exp_count0_len12 = isrran::make_byte_buffer();
    pdu_exp_count0_len12->append_bytes(pdu1_count0_snlen12, sizeof(pdu1_count0_snlen12));
    TESTASSERT(tx_helper.test_tx(n_packets, normal_init_state, n_packets, std::move(pdu_exp_count0_len12)) == 0);
    TESTASSERT(tx_helper.pdcp_tx.nof_discard_timers() == 1);
    tx_helper.pdcp_tx.notify_delivery({0});
    TESTASSERT(tx_helper.pdcp_tx.nof_discard_timers() == 0);
  }
  return ISRRAN_SUCCESS;
}

// Setup all tests
int run_all_tests()
{
  // Setup log
  auto& logger = isrlog::fetch_basic_logger("PDCP NR Test TX", false);
  logger.set_level(isrlog::basic_levels::debug);
  logger.set_hex_dump_max_size(128);

  TESTASSERT(test_tx_all(logger) == 0);
  return 0;
}

int main()
{
  isrlog::init();

  if (run_all_tests() != ISRRAN_SUCCESS) {
    fprintf(stderr, "pdcp_nr_tests_tx() failed\n");
    return ISRRAN_ERROR;
  }
  return ISRRAN_SUCCESS;
}
