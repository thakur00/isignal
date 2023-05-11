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

#ifndef ISRRAN_NR_CC_WORKER_H
#define ISRRAN_NR_CC_WORKER_H

#include "isrue/hdr/phy/phy_common.h"
#include "state.h"

namespace isrue {
namespace nr {

class cc_worker
{
public:
  cc_worker(uint32_t cc_idx, isrlog::basic_logger& log, state& phy_state_, const isrran::phy_cfg_nr_t& cfg);
  ~cc_worker();

  void update_cfg(const isrran::phy_cfg_nr_t& new_config);
  void set_tti(uint32_t tti);

  cf_t*    get_rx_buffer(uint32_t antenna_idx);
  cf_t*    get_tx_buffer(uint32_t antenna_idx);
  uint32_t get_buffer_len();

  bool work_dl();
  bool work_ul();

  int read_pdsch_d(cf_t* pdsch_d);

private:
  // PHY lib temporal logger types
  typedef std::array<char, 512>  str_info_t;
  typedef std::array<char, 2048> str_extra_t;

  bool                                configured  = false;
  isrran_slot_cfg_t                   dl_slot_cfg = {};
  isrran_slot_cfg_t                   ul_slot_cfg = {};
  uint32_t                            cc_idx      = 0;
  std::array<cf_t*, ISRRAN_MAX_PORTS> rx_buffer   = {};
  std::array<cf_t*, ISRRAN_MAX_PORTS> tx_buffer   = {};
  uint32_t                            buffer_sz   = 0;
  state&                              phy;
  isrran::phy_cfg_nr_t                cfg;
  isrran_ssb_t                        ssb   = {};
  isrran_ue_dl_nr_t                   ue_dl = {};
  isrran_ue_ul_nr_t                   ue_ul = {};
  isrlog::basic_logger&               logger;

  // Methods for DCI blind search
  void decode_pdcch_ul();
  void decode_pdcch_dl();

  /**
   * @brief Decodes PDSCH in the current processing slot
   * @return true if current configuration is valid and no error occur, false otherwise
   */
  bool decode_pdsch_dl();

  /**
   * @brief Performs Channel State Information (CSI) measurements
   * @return true if current configuration is valid and no error occur, false otherwise
   */
  bool measure_csi();
};

} // namespace nr
} // namespace isrue

#endif // ISRRAN_NR_CC_WORKER_H
