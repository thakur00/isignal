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

#ifndef ISRRAN_RRC_NR_SECURITY_CONTEXT_H
#define ISRRAN_RRC_NR_SECURITY_CONTEXT_H

#include "isrgnb/hdr/stack/rrc/rrc_nr_config.h"
#include "isrran/asn1/ngap.h"
#include "isrran/interfaces/gnb_interfaces.h"
#include "isrran/isrlog/isrlog.h"

namespace isrgnb {

class nr_security_context
{
public:
  explicit nr_security_context(const isrenb::rrc_nr_cfg_t& cfg_) :
    cfg(cfg_), logger(isrlog::fetch_basic_logger("RRC-NR"))
  {}

  nr_security_context(const nr_security_context& other) : cfg(other.cfg), logger(isrlog::fetch_basic_logger("RRC-NR"))
  {
    k_gnb_present         = other.k_gnb_present;
    security_capabilities = other.security_capabilities;
    std::copy(other.k_gnb, other.k_gnb + 32, k_gnb);
    sec_cfg = other.sec_cfg;
    ncc     = other.ncc;
  }

  nr_security_context& operator=(const nr_security_context& other)
  {
    k_gnb_present         = other.k_gnb_present;
    security_capabilities = other.security_capabilities;
    std::copy(other.k_gnb, other.k_gnb + 32, k_gnb);
    sec_cfg = other.sec_cfg;
    ncc     = other.ncc;
    return *this;
  }

  bool set_security_capabilities(const asn1::ngap::ue_security_cap_s& caps);
  void set_security_key(const asn1::fixed_bitstring<256, false, true>& key);
  void set_ncc(uint8_t ncc_) { ncc = ncc_; }

  asn1::rrc_nr::security_algorithm_cfg_s get_security_algorithm_cfg() const;
  const isrran::nr_as_security_config_t& get_as_sec_cfg() const { return sec_cfg; }
  uint8_t                                get_ncc() const { return ncc; }
  bool                                   is_as_sec_cfg_valid() const { return k_gnb_present; }

  void regenerate_keys_handover(uint32_t new_pci, uint32_t new_dl_earfcn);

private:
  void generate_as_keys();

  isrlog::basic_logger&           logger;
  const isrenb::rrc_nr_cfg_t&     cfg;
  bool                            k_gnb_present         = false;
  asn1::ngap::ue_security_cap_s   security_capabilities = {};
  uint8_t                         k_gnb[32]             = {}; // Provided by MME
  isrran::nr_as_security_config_t sec_cfg               = {};
  uint8_t                         ncc                   = 0;
};
} // namespace isrgnb
#endif
