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

#ifndef ISRRAN_UE_PDCP_INTERFACES_H
#define ISRRAN_UE_PDCP_INTERFACES_H

#include "pdcp_interface_types.h"
#include "isrran/common/byte_buffer.h"

namespace isrue {

class pdcp_interface_rrc
{
public:
  virtual void reestablish()                                                                                        = 0;
  virtual void reestablish(uint32_t lcid)                                                                           = 0;
  virtual void reset()                                                                                              = 0;
  virtual void set_enabled(uint32_t lcid, bool enabled)                                                             = 0;
  virtual void write_sdu(uint32_t lcid, isrran::unique_byte_buffer_t sdu, int sn = -1)                              = 0;
  virtual int  add_bearer(uint32_t lcid, const isrran::pdcp_config_t& cnfg)                                         = 0;
  virtual void del_bearer(uint32_t lcid)                                                                            = 0;
  virtual void change_lcid(uint32_t old_lcid, uint32_t new_lcid)                                                    = 0;
  virtual void config_security(uint32_t lcid, const isrran::as_security_config_t& sec_cfg)                          = 0;
  virtual void config_security_all(const isrran::as_security_config_t& sec_cfg)                                     = 0;
  virtual void enable_integrity(uint32_t lcid, isrran::isrran_direction_t direction)                                = 0;
  virtual void enable_encryption(uint32_t                   lcid,
                                 isrran::isrran_direction_t direction = isrran::isrran_direction_t::DIRECTION_TXRX) = 0;
  virtual void send_status_report()                                                                                 = 0;
  virtual void send_status_report(uint32_t lcid)                                                                    = 0;
};

class pdcp_interface_rlc
{
public:
  /* RLC calls PDCP to push a PDCP PDU. */
  virtual void write_pdu(uint32_t lcid, isrran::unique_byte_buffer_t sdu)              = 0;
  virtual void write_pdu_bcch_bch(isrran::unique_byte_buffer_t sdu)                    = 0;
  virtual void write_pdu_bcch_dlsch(isrran::unique_byte_buffer_t sdu)                  = 0;
  virtual void write_pdu_pcch(isrran::unique_byte_buffer_t sdu)                        = 0;
  virtual void write_pdu_mch(uint32_t lcid, isrran::unique_byte_buffer_t sdu)          = 0;
  virtual void notify_delivery(uint32_t lcid, const isrran::pdcp_sn_vector_t& pdcp_sn) = 0;
  virtual void notify_failure(uint32_t lcid, const isrran::pdcp_sn_vector_t& pdcp_sn)  = 0;
};

// Data-plane interface for Stack after EPS bearer to LCID conversion
class pdcp_interface_stack
{
public:
  virtual void write_sdu(uint32_t lcid, isrran::unique_byte_buffer_t sdu, int sn = -1) = 0;
  virtual bool is_lcid_enabled(uint32_t lcid)                                          = 0;
};

// SDAP interface
class pdcp_interface_sdap_nr
{
public:
  virtual void write_sdu(uint32_t lcid, isrran::unique_byte_buffer_t pdu) = 0;
};

// STACK interface for GW (based on EPS-bearer IDs)
class stack_interface_gw
{
public:
  virtual bool is_registered()         = 0;
  virtual bool start_service_request() = 0;
  virtual void write_sdu(uint32_t eps_bearer_id, isrran::unique_byte_buffer_t sdu) = 0;
  ///< Allow GW to query if a radio bearer for a given EPS bearer ID is currently active
  virtual bool has_active_radio_bearer(uint32_t eps_bearer_id) = 0;
};

} // namespace isrue

#endif // ISRRAN_UE_PDCP_INTERFACES_H
