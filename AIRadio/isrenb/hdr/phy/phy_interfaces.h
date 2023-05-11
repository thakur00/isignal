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

#ifndef ISRENB_PHY_INTERFACES_H_
#define ISRENB_PHY_INTERFACES_H_

#include "isrgnb/hdr/phy/phy_nr_interfaces.h"
#include "isrran/asn1/rrc/rr_common.h"
#include "isrran/common/interfaces_common.h"
#include "isrran/phy/channel/channel.h"
#include "isrran/isrran.h"
#include <inttypes.h>
#include <vector>

namespace isrenb {

struct phy_cell_cfg_t {
  isrran_cell_t cell;
  uint32_t      rf_port;
  uint32_t      cell_id;
  double        dl_freq_hz;
  double        ul_freq_hz;
  uint32_t      root_seq_idx;
  uint32_t      num_ra_preambles;
  float         gain_db;
  bool          dl_measure;
};

typedef std::vector<phy_cell_cfg_t> phy_cell_cfg_list_t;

struct cfr_args_t {
  bool              enable           = false;
  isrran_cfr_mode_t mode             = ISRRAN_CFR_THR_MANUAL;
  float             manual_thres     = 0.5f;
  float             strength         = 1.0f;
  float             auto_target_papr = 8.0f;
  float             ema_alpha        = 1.0f / (float)ISRRAN_CP_NORM_NSYMB;
};

struct phy_args_t {
  std::string            type;
  isrran::phy_log_args_t log;

  float                   rx_gain_offset      = 62;
  float                   max_prach_offset_us = 10;
  uint32_t                pusch_max_its       = 10;
  uint32_t                nr_pusch_max_its    = 10;
  bool                    pusch_8bit_decoder  = false;
  float                   tx_amplitude        = 1.0f;
  uint32_t                nof_phy_threads     = 1;
  std::string             equalizer_mode      = "mmse";
  float                   estimator_fil_w     = 1.0f;
  bool                    pusch_meas_epre     = true;
  bool                    pusch_meas_evm      = false;
  bool                    pusch_meas_ta       = true;
  bool                    pucch_meas_ta       = true;
  uint32_t                nof_prach_threads   = 1;
  bool                    extended_cp         = false;
  isrran::channel::args_t dl_channel_args;
  isrran::channel::args_t ul_channel_args;
  cfr_args_t              cfr_args;
};

struct phy_cfg_t {
  // Individual cell/sector configuration list
  phy_cell_cfg_list_t    phy_cell_cfg;
  phy_cell_cfg_list_nr_t phy_cell_cfg_nr;

  // Common configuration for all cells
  asn1::rrc::prach_cfg_sib_s     prach_cnfg;
  asn1::rrc::pdsch_cfg_common_s  pdsch_cnfg;
  asn1::rrc::pusch_cfg_common_s  pusch_cnfg;
  asn1::rrc::pucch_cfg_common_s  pucch_cnfg;
  asn1::rrc::isr_ul_cfg_common_c isr_ul_cnfg;

  isrran_cfr_cfg_t cfr_config;
};

} // namespace isrenb

#endif // ISRENB_PHY_INTERFACES_H_
