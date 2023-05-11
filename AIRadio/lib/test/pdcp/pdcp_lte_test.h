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

#ifndef ISRRAN_PDCP_LTE_TEST_H
#define ISRRAN_PDCP_LTE_TEST_H

#include "pdcp_base_test.h"
#include "isrran/test/ue_test_interfaces.h"
#include "isrran/upper/pdcp_entity_lte.h"

// Helper struct to hold a packet and the number of clock
// ticks to run after writing the packet to test timeouts.
struct pdcp_test_event_t {
  isrran::unique_byte_buffer_t pkt;
  uint32_t                     ticks = 0;
};

/*
 * Constant definitions that are common to multiple tests
 */
// Encryption and Integrity Keys
std::array<uint8_t, 32> k_int = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10,
                                 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21,
                                 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31};
std::array<uint8_t, 32> k_enc = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10,
                                 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21,
                                 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31};

// Security Configuration, common to all tests.
isrran::as_security_config_t sec_cfg = {
    k_int,
    k_enc,
    k_int,
    k_enc,
    isrran::INTEGRITY_ALGORITHM_ID_128_EIA2,
    isrran::CIPHERING_ALGORITHM_ID_128_EEA2,
};

// Test SDUs for tx
uint8_t sdu1[] = {0x18, 0xe2};
uint8_t sdu2[] = {0xde, 0xad};

// This is the normal initial state. All state variables are set to zero
isrran::pdcp_lte_state_t normal_init_state = {};

/*
 * Helper classes to reduce copy / pasting in setting up tests
 */
// PDCP helper to setup PDCP + Dummy
class pdcp_lte_test_helper
{
public:
  pdcp_lte_test_helper(isrran::pdcp_config_t cfg, isrran::as_security_config_t sec_cfg_, isrlog::basic_logger& logger) :
    rlc(logger), rrc(logger), gw(logger), pdcp(&rlc, &rrc, &gw, &stack.task_sched, logger, 0)
  {
    pdcp.configure(cfg);
    pdcp.config_security(sec_cfg_);
    pdcp.enable_integrity(isrran::DIRECTION_TXRX);
    pdcp.enable_encryption(isrran::DIRECTION_TXRX);
  }

  void set_pdcp_initial_state(const isrran::pdcp_lte_state_t& init_state) { pdcp.set_bearer_state(init_state, false); }

  rlc_dummy               rlc;
  rrc_dummy               rrc;
  gw_dummy                gw;
  isrue::stack_test_dummy stack;
  isrran::pdcp_entity_lte pdcp;
};

// Helper function to generate PDUs
isrran::unique_byte_buffer_t gen_expected_pdu(const isrran::unique_byte_buffer_t& in_sdu,
                                              uint32_t                            count,
                                              uint8_t                             pdcp_sn_len,
                                              isrran::pdcp_rb_type_t              rb_type,
                                              isrran::as_security_config_t        sec_cfg,
                                              isrlog::basic_logger&               logger)
{
  isrran::pdcp_config_t cfg = {1,
                               rb_type,
                               isrran::SECURITY_DIRECTION_UPLINK,
                               isrran::SECURITY_DIRECTION_DOWNLINK,
                               pdcp_sn_len,
                               isrran::pdcp_t_reordering_t::ms500,
                               isrran::pdcp_discard_timer_t::infinity,
                               false,
                               isrran::isrran_rat_t::lte};

  pdcp_lte_test_helper     pdcp_hlp(cfg, sec_cfg, logger);
  isrran::pdcp_entity_lte* pdcp = &pdcp_hlp.pdcp;
  rlc_dummy*               rlc  = &pdcp_hlp.rlc;

  isrran::pdcp_lte_state_t init_state = {};
  init_state.tx_hfn                   = pdcp->HFN(count);
  init_state.next_pdcp_tx_sn          = pdcp->SN(count);
  pdcp_hlp.set_pdcp_initial_state(init_state);

  isrran::unique_byte_buffer_t sdu = isrran::make_byte_buffer();
  *sdu                             = *in_sdu;
  pdcp->write_sdu(std::move(sdu));
  isrran::unique_byte_buffer_t out_pdu = isrran::make_byte_buffer();
  rlc->get_last_sdu(out_pdu);

  return out_pdu;
}

// Helper function to generate vector of PDU from a vector of TX_NEXTs for generating expected pdus
std::vector<pdcp_test_event_t> gen_expected_pdus_vector(const isrran::unique_byte_buffer_t& in_sdu,
                                                        const std::vector<uint32_t>&        tx_nexts,
                                                        uint8_t                             pdcp_sn_len,
                                                        isrran::pdcp_rb_type_t              rb_type,
                                                        isrran::as_security_config_t        sec_cfg_,
                                                        isrlog::basic_logger&               logger)
{
  std::vector<pdcp_test_event_t> pdu_vec;
  for (uint32_t tx_next : tx_nexts) {
    pdcp_test_event_t event;
    event.pkt   = gen_expected_pdu(in_sdu, tx_next, pdcp_sn_len, rb_type, sec_cfg_, logger);
    event.ticks = 0;
    pdu_vec.push_back(std::move(event));
  }
  return pdu_vec;
}

#endif // ISRRAN_PDCP_NR_TEST_H
