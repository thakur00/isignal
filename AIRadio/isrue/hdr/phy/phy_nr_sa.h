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

#ifndef ISRUE_PHY_NR_SA_H
#define ISRUE_PHY_NR_SA_H

#include "phy_common.h"
#include "isrran/interfaces/ue_nr_interfaces.h"
#include "isrue/hdr/phy/nr/sync_sa.h"
#include "isrue/hdr/phy/ue_phy_base.h"

namespace isrue {

/**
 * @brief NR Standalone PHY
 */
class phy_nr_sa final : public ue_phy_base, public phy_interface_stack_nr, public isrran::phy_interface_radio
{
public:
  phy_nr_sa(const char* logname);
  ~phy_nr_sa() final { stop(); }

  int  init(const phy_args_nr_t& args_, stack_interface_phy_nr* stack_, isrran::radio_interface_phy* radio_);
  void wait_initialize() final;
  bool is_initialized() final;
  void stop() final;
  void reset_nr() final;

  void start_plot() final {}

  void radio_overflow() final {}
  void radio_failure() final {}

  std::string get_type() final { return "nr_soft"; }

  int  set_rar_grant(uint32_t                                       rar_slot_idx,
                     std::array<uint8_t, ISRRAN_RAR_UL_GRANT_NBITS> packed_ul_grant,
                     uint16_t                                       rnti,
                     isrran_rnti_type_t                             rnti_type) final;
  void send_prach(const uint32_t prach_occasion,
                  const int      preamble_index,
                  const float    preamble_received_target_power,
                  const float    ta_base_sec) final;
  void set_timeadv_rar(uint32_t tti, uint32_t ta_cmd) final;
  void set_timeadv(uint32_t tti, uint32_t ta_cmd) final;

  void set_earfcn(std::vector<uint32_t> earfcns);
  bool has_valid_sr_resource(uint32_t sr_id) final;
  void clear_pending_grants() final;
  bool set_config(const isrran::phy_cfg_nr_t& cfg) final;

  phy_nr_state_t get_state() final;
  bool           start_cell_search(const cell_search_args_t& req) final;
  bool           start_cell_select(const cell_select_args_t& req) final;

  void get_metrics(const isrran::isrran_rat_t& rat, phy_metrics_t* m) final
  {
    if (rat == isrran::isrran_rat_t::nr) {
      return workers.get_metrics(*m);
    }
  };
  void isrran_phy_logger(phy_logger_level_t log_level, char* str);

private:
  isrlog::basic_logger& logger;
  isrlog::basic_logger& logger_phy_lib;

  nr::worker_pool workers;
  nr::sync_sa     sync;

  isrran::phy_cfg_nr_t config_nr     = {};
  phy_args_nr_t        args          = {};
  isrran_carrier_nr_t  selected_cell = {};

  isrran::radio_interface_phy* radio = nullptr;
  stack_interface_phy_nr*      stack = nullptr;

  std::atomic<bool> is_configured = {false};

  // Since cell_search/cell_select operations take a lot of time, we use another queue to process the other commands
  // in parallel and avoid accumulating in the queue
  phy_cmd_proc cmd_worker_cell, cmd_worker;

  // Run initialization functions in the background
  void        init_background();
  std::thread init_thread;

  const static int MAX_WORKERS     = 4;
  const static int DEFAULT_WORKERS = 4;

  static void set_default_args(phy_args_nr_t& args);
  bool        check_args(const phy_args_nr_t& args);
};

} // namespace isrue
#endif // ISRUE_PHY_NR_SA_H
