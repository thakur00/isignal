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

#ifndef ISRRAN_RRC_NR_CONFIG_H
#define ISRRAN_RRC_NR_CONFIG_H

#include "isrenb/hdr/stack/rrc/rrc_config_common.h"
#include "isrgnb/hdr/phy/phy_nr_interfaces.h"
#include "isrran/asn1/rrc_nr.h"
#include "isrran/common/security.h"
#include "isrran/interfaces/gnb_rrc_nr_interfaces.h"

namespace isrenb {

// Cell/Sector configuration for NR cells
struct rrc_cell_cfg_nr_t {
  phy_cell_cfg_nr_t                phy_cell; // already contains all PHY-related parameters (i.e. RF port, PCI, etc.)
  uint32_t                         tac;      // Tracking area code
  uint32_t                         dl_arfcn; // DL freq already included in carrier
  uint32_t                         ul_arfcn; // UL freq also in carrier
  uint32_t                         dl_absolute_freq_point_a; // derived from DL ARFCN
  uint32_t                         ul_absolute_freq_point_a; // derived from UL ARFCN
  uint32_t                         band;
  uint32_t                         prach_root_seq_idx;
  uint32_t                         num_ra_preambles;
  uint32_t                         coreset0_idx; // Table 13-{1,...15} row index
  isrran_duplex_mode_t             duplex_mode;
  double                           ssb_freq_hz;
  uint32_t                         ssb_absolute_freq_point; // derived from DL ARFCN (SSB arfcn)
  uint32_t                         ssb_offset;
  isrran_subcarrier_spacing_t      ssb_scs;
  isrran_ssb_pattern_t             ssb_pattern;
  asn1::rrc_nr::pdcch_cfg_common_s pdcch_cfg_common;
  asn1::rrc_nr::pdcch_cfg_s        pdcch_cfg_ded;
  int8_t                           pdsch_rs_power;
};

typedef std::vector<rrc_cell_cfg_nr_t> rrc_cell_list_nr_t;

struct srb_5g_cfg_t {
  bool                    present = false;
  asn1::rrc_nr::rlc_cfg_c rlc_cfg;
};

struct rrc_nr_cfg_five_qi_t {
  bool                     configured = false;
  asn1::rrc_nr::pdcp_cfg_s pdcp_cfg;
  asn1::rrc_nr::rlc_cfg_c  rlc_cfg;
};

struct rrc_nr_cfg_t {
  rrc_cell_list_nr_t cell_list;
  uint32_t           inactivity_timeout_ms = 100000;
  uint32_t           enb_id;
  uint16_t           mcc;
  uint16_t           mnc;
  bool               is_standalone;

  srb_5g_cfg_t srb1_cfg;
  srb_5g_cfg_t srb2_cfg;

  std::map<uint32_t, rrc_nr_cfg_five_qi_t> five_qi_cfg;

  std::array<isrran::CIPHERING_ALGORITHM_ID_NR_ENUM, isrran::CIPHERING_ALGORITHM_ID_NR_N_ITEMS> nea_preference_list;
  std::array<isrran::INTEGRITY_ALGORITHM_ID_NR_ENUM, isrran::INTEGRITY_ALGORITHM_ID_NR_N_ITEMS> nia_preference_list;

  std::string log_name = "RRC-NR";
  std::string log_level;
  uint32_t    log_hex_limit;
};

} // namespace isrenb

#endif // ISRRAN_RRC_NR_CONFIG_H
