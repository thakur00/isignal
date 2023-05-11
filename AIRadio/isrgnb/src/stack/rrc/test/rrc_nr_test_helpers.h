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

#ifndef ISRRAN_RRC_NR_TEST_HELPERS_H
#define ISRRAN_RRC_NR_TEST_HELPERS_H

#include "isrenb/test/common/dummy_classes_common.h"
#include "isrgnb/hdr/stack/common/test/dummy_nr_classes.h"
#include "isrgnb/hdr/stack/rrc/rrc_nr.h"

namespace isrenb {

class pdcp_nr_rrc_tester : public pdcp_dummy
{
public:
  void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t sdu, int pdcp_sn = -1) override
  {
    last_sdu_rnti = rnti;
    last_sdu_lcid = lcid;
    last_sdu      = std::move(sdu);
  }

  uint16_t                     last_sdu_rnti = ISRRAN_INVALID_RNTI;
  uint32_t                     last_sdu_lcid = isrran::MAX_NR_NOF_BEARERS;
  isrran::unique_byte_buffer_t last_sdu;
};

class rlc_nr_rrc_tester : public rlc_dummy
{
public:
  void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t sdu)
  {
    last_sdu_rnti = rnti;
    last_sdu_lcid = lcid;
    last_sdu      = std::move(sdu);
  }

  uint16_t                     last_sdu_rnti;
  uint32_t                     last_sdu_lcid;
  isrran::unique_byte_buffer_t last_sdu;
};

class ngap_rrc_tester : public ngap_dummy
{
public:
  void initial_ue(uint16_t                             rnti,
                  uint32_t                             gnb_cc_idx,
                  asn1::ngap::rrcestablishment_cause_e cause,
                  isrran::const_byte_span              pdu,
                  uint32_t                             s_tmsi)
  {
    last_sdu_rnti = rnti;
    last_pdu.resize(pdu.size());
    memcpy(last_pdu.data(), pdu.data(), pdu.size());
  }

  void write_pdu(uint16_t rnti, isrran::const_byte_span pdu)
  {
    last_sdu_rnti = rnti;
    last_pdu.resize(pdu.size());
    memcpy(last_pdu.data(), pdu.data(), pdu.size());
  }

  void ue_notify_rrc_reconf_complete(uint16_t rnti, bool outcome) { last_rrc_recnf_complete = outcome; }

  uint16_t            last_sdu_rnti;
  asn1::dyn_octstring last_pdu;
  bool                last_rrc_recnf_complete = false;
};

/**
 * Run TS 38.331, 5.3.3 "RRC connection establishment" to completion
 * RRC actions:
 * - Rx RRCSetupRequest
 * - Tx RRCSetup to lower layers
 * - Tx RRCSetupComplete
 * Checks:
 * - the RRC sends RRCSetup as reply to RRCSetupRequest
 * - verify that RRCSetup rnti, lcid are correct
 * - verify that RRCSetup adds an SRB1
 */
void test_rrc_nr_connection_establishment(isrran::task_scheduler& task_sched,
                                          rrc_nr&                 rrc_obj,
                                          rlc_nr_rrc_tester&      rlc,
                                          mac_nr_dummy&           mac,
                                          ngap_rrc_tester&        ngap,
                                          uint16_t                rnti);

void test_rrc_nr_info_transfer(isrran::task_scheduler& task_sched,
                               rrc_nr&                 rrc_obj,
                               pdcp_nr_rrc_tester&     pdcp,
                               ngap_rrc_tester&        ngap,
                               uint16_t                rnti);

void test_rrc_nr_security_mode_cmd(isrran::task_scheduler& task_sched,
                                   rrc_nr&                 rrc_obj,
                                   pdcp_nr_rrc_tester&     pdcp,
                                   uint16_t                rnti);

void test_rrc_nr_ue_capability_enquiry(isrran::task_scheduler& task_sched,
                                       rrc_nr&                 rrc_obj,
                                       pdcp_nr_rrc_tester&     pdcp,
                                       uint16_t                rnti);

void test_rrc_nr_reconfiguration(isrran::task_scheduler& task_sched,
                                 rrc_nr&                 rrc_obj,
                                 pdcp_nr_rrc_tester&     pdcp,
                                 ngap_rrc_tester&        ngap,
                                 uint16_t                rnti);

void test_rrc_nr_2nd_reconfiguration(isrran::task_scheduler& task_sched,
                                     rrc_nr&                 rrc_obj,
                                     pdcp_nr_rrc_tester&     pdcp,
                                     ngap_rrc_tester&        ngap,
                                     uint16_t                rnti);

} // namespace isrenb

#endif // ISRRAN_RRC_NR_TEST_HELPERS_H
