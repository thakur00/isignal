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

#ifndef ISRRAN_GNB_NGAP_INTERFACES_H
#define ISRRAN_GNB_NGAP_INTERFACES_H

#include "isrran/asn1/ngap_utils.h"

namespace isrenb {

struct ngap_args_t {
  uint32_t    gnb_id             = 0; // 20-bit id (lsb bits)
  uint8_t     cell_id            = 0; // 8-bit cell id
  uint16_t    tac                = 0; // 16-bit tac
  uint16_t    mcc                = 0; // BCD-coded with 0xF filler
  uint16_t    mnc                = 0; // BCD-coded with 0xF filler
  std::string amf_addr           = "";
  std::string gtp_bind_addr      = "";
  std::string gtp_advertise_addr = "";
  std::string ngc_bind_addr      = "";
  std::string gnb_name           = "";
};

// NGAP interface for RRC
class ngap_interface_rrc_nr
{
public:
  virtual void initial_ue(uint16_t                             rnti,
                          uint32_t                             gnb_cc_idx,
                          asn1::ngap::rrcestablishment_cause_e cause,
                          isrran::const_byte_span              pdu) = 0;
  virtual void initial_ue(uint16_t                             rnti,
                          uint32_t                             gnb_cc_idx,
                          asn1::ngap::rrcestablishment_cause_e cause,
                          isrran::const_byte_span              pdu,
                          uint32_t                             m_tmsi)             = 0;

  virtual void write_pdu(uint16_t rnti, isrran::const_byte_span pdu)                              = 0;
  virtual bool user_exists(uint16_t rnti)                                                         = 0;
  virtual void user_mod(uint16_t old_rnti, uint16_t new_rnti)                                     = 0;
  virtual void user_release_request(uint16_t rnti, asn1::ngap::cause_radio_network_e cause_radio) = 0;
  virtual bool is_amf_connected()                                                                 = 0;
  virtual void ue_notify_rrc_reconf_complete(uint16_t rnti, bool outcome)                         = 0;
};

} // namespace isrenb

#endif // ISRRAN_GNB_NGAP_INTERFACES_H
