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

#ifndef ISRENB_DUMMY_CLASSES_COMMON_H
#define ISRENB_DUMMY_CLASSES_COMMON_H

#include "isrran/interfaces/enb_pdcp_interfaces.h"
#include "isrran/interfaces/enb_rlc_interfaces.h"

namespace isrenb {

class rlc_dummy : public rlc_interface_rrc
{
public:
  void clear_buffer(uint16_t rnti) override {}
  void add_user(uint16_t rnti) override {}
  void rem_user(uint16_t rnti) override {}
  void add_bearer(uint16_t rnti, uint32_t lcid, const isrran::rlc_config_t& cnfg) override {}
  void add_bearer_mrb(uint16_t rnti, uint32_t lcid) override {}
  void del_bearer(uint16_t rnti, uint32_t lcid) override {}
  void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t sdu) override { last_sdu = std::move(sdu); }
  bool has_bearer(uint16_t rnti, uint32_t lcid) override { return false; }
  bool suspend_bearer(uint16_t rnti, uint32_t lcid) override { return true; }
  bool is_suspended(uint16_t rnti, uint32_t lcid) override { return false; }
  bool resume_bearer(uint16_t rnti, uint32_t lcid) override { return true; }
  void reestablish(uint16_t rnti) override {}

  isrran::unique_byte_buffer_t last_sdu;
};

class pdcp_dummy : public pdcp_interface_rrc, public pdcp_interface_gtpu
{
public:
  void set_enabled(uint16_t rnti, uint32_t lcid, bool enabled) override {}
  void reset(uint16_t rnti) override {}
  void add_user(uint16_t rnti) override {}
  void rem_user(uint16_t rnti) override {}
  void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t sdu, int pdcp_sn) override {}
  void add_bearer(uint16_t rnti, uint32_t lcid, const isrran::pdcp_config_t& cnfg) override {}
  void del_bearer(uint16_t rnti, uint32_t lcid) override {}
  void config_security(uint16_t rnti, uint32_t lcid, const isrran::as_security_config_t& sec_cfg_) override {}
  void enable_integrity(uint16_t rnti, uint32_t lcid) override {}
  void enable_encryption(uint16_t rnti, uint32_t lcid) override {}
  bool get_bearer_state(uint16_t rnti, uint32_t lcid, isrran::pdcp_lte_state_t* state) override { return true; }
  bool set_bearer_state(uint16_t rnti, uint32_t lcid, const isrran::pdcp_lte_state_t& state) override { return true; }
  void reestablish(uint16_t rnti) override {}
  void send_status_report(uint16_t rnti) override {}
  void send_status_report(uint16_t rnti, uint32_t lcid) override {}
  std::map<uint32_t, isrran::unique_byte_buffer_t> get_buffered_pdus(uint16_t rnti, uint32_t lcid) override
  {
    return {};
  }
};

} // namespace isrenb

#endif // ISRENB_DUMMY_CLASSES_COMMON_H
