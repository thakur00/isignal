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

#ifndef ISRRAN_UE_NAS_INTERFACES_H
#define ISRRAN_UE_NAS_INTERFACES_H

#include "isrran/asn1/nas_5g_ies.h"
#include "isrran/interfaces/rrc_interface_types.h"

namespace isrue {

enum apn_types {
  ipv4         = 0b001,
  ipv6         = 0b010,
  ipv4v6       = 0b011,
  unstructured = 0b100,
  ethernet     = 0b101,
};
class pdu_session_cfg_t
{
public:
  std::string apn_name;
  apn_types   apn_type = ipv4;
  std::string apn_user;
  std::string apn_pass;
};
class nas_interface_rrc
{
public:
  const static int MAX_FOUND_PLMNS = 16;
  struct found_plmn_t {
    isrran::plmn_id_t plmn_id;
    uint16_t          tac;
  };

  virtual void     left_rrc_connected()                                                                  = 0;
  virtual void     set_barring(isrran::barring_t barring)                                                = 0;
  virtual bool     paging(isrran::s_tmsi_t* ue_identity)                                                 = 0;
  virtual bool     is_registered()                                                                       = 0;
  virtual void     write_pdu(uint32_t lcid, isrran::unique_byte_buffer_t pdu)                            = 0;
  virtual uint32_t get_k_enb_count()                                                                     = 0;
  virtual bool     get_k_asme(uint8_t* k_asme_, uint32_t n)                                              = 0;
  virtual uint32_t get_ipv4_addr()                                                                       = 0;
  virtual bool     get_ipv6_addr(uint8_t* ipv6_addr)                                                     = 0;
  virtual void     plmn_search_completed(const found_plmn_t found_plmns[MAX_FOUND_PLMNS], int nof_plmns) = 0;
  virtual bool     connection_request_completed(bool outcome)                                            = 0;
};

class nas_5g_interface_rrc_nr
{
public:
  virtual int      write_pdu(isrran::unique_byte_buffer_t pdu) = 0;
  virtual int      get_k_amf(isrran::as_key_t& k_amf)          = 0;
  virtual uint32_t get_ul_nas_count()                          = 0;
};

class nas_5g_interface_procedures
{
public:
  virtual int send_registration_request()                                                  = 0;
  virtual int send_pdu_session_establishment_request(uint32_t                 transaction_identity,
                                                     uint16_t                 pdu_session_id,
                                                     const pdu_session_cfg_t& pdu_session) = 0;
  virtual int
  add_pdu_session(uint16_t pdu_session_id, uint16_t pdu_session_type, isrran::nas_5g::pdu_address_t pdu_address) = 0;

  virtual uint32_t allocate_next_proc_trans_id() = 0;
};

} // namespace isrue

#endif // ISRRAN_UE_NAS_INTERFACES_H
