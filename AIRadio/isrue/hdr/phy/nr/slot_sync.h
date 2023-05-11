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

#ifndef ISRUE_SLOT_SYNC_H
#define ISRUE_SLOT_SYNC_H

#include "isrran/interfaces/radio_interfaces.h"
#include "isrran/interfaces/ue_nr_interfaces.h"
#include "isrran/radio/rf_buffer.h"
#include "isrran/radio/rf_timestamp.h"
#include "isrran/isrran.h"

namespace isrue {
namespace nr {
class slot_sync
{
public:
  struct args_t {
    double                      max_srate_hz    = 1.92e6;
    uint32_t                    nof_rx_channels = 1;
    isrran_subcarrier_spacing_t ssb_min_scs     = isrran_subcarrier_spacing_15kHz;
    bool                        disable_cfo     = false;
    float                       pbch_dmrs_thr   = 0.0f; ///< PBCH DMRS correlation detection threshold (0 means auto)
    float                       cfo_alpha       = 0.0f; ///< CFO averaging alpha (0 means auto)
    int                         thread_priority = -1;
  };

  slot_sync(isrlog::basic_logger& logger);
  ~slot_sync();

  bool init(const args_t& args, stack_interface_phy_nr* stack_, isrran::radio_interface_phy* radio_);

  int set_sync_cfg(const isrran_ue_sync_nr_cfg_t& cfg);

  int  recv_callback(isrran::rf_buffer_t& rf_buffer, isrran_timestamp_t* timestamp);
  bool run_sfn_sync();
  bool run_camping(isrran::rf_buffer_t& buffer, isrran::rf_timestamp_t& timestamp);
  void run_stack_tti();

  isrran_slot_cfg_t get_slot_cfg();

private:
  const static int             MIN_TTI_JUMP = 1;    ///< Time gap reported to stack after receiving subframe
  const static int             MAX_TTI_JUMP = 1000; ///< Maximum time gap tolerance in RF stream metadata
  isrlog::basic_logger&        logger;
  stack_interface_phy_nr*      stack = nullptr;
  isrran::radio_interface_phy* radio = nullptr;
  isrran::rf_timestamp_t       last_rx_time;
  isrran_ue_sync_nr_t          ue_sync_nr          = {};
  isrran_timestamp_t           stack_tti_ts_new    = {};
  isrran_timestamp_t           stack_tti_ts        = {};
  bool                         forced_rx_time_init = true; // Rx time sync after first receive from radio
  isrran::rf_buffer_t          sfn_sync_buff       = {};
  isrran_slot_cfg_t            slot_cfg            = {};
};
} // namespace nr
} // namespace isrue

#endif // ISRUE_SLOT_SYNC_H
