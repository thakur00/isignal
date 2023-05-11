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

#ifndef ISRUE_PHY_H
#define ISRUE_PHY_H

#include "phy_common.h"
#include "phy_metrics.h"
#include "isrran/common/block_queue.h"
#include "isrran/common/threads.h"
#include "isrran/common/trace.h"
#include "isrran/interfaces/phy_interface_types.h"
#include "isrran/interfaces/radio_interfaces.h"
#include "isrran/radio/radio.h"
#include "isrran/isrlog/isrlog.h"
#include "isrran/isrran.h"
#include "isrue/hdr/phy/lte/worker_pool.h"
#include "isrue/hdr/phy/nr/worker_pool.h"
#include "isrue/hdr/phy/ue_phy_base.h"
#include "sync.h"

namespace isrue {

typedef _Complex float cf_t;

class phy final : public ue_phy_base,
                  public phy_interface_stack_lte,
                  public phy_interface_stack_nr,
                  public isrran::phy_interface_radio,
                  public isrran::thread
{
public:
  explicit phy() :
    logger_phy(isrlog::fetch_basic_logger("PHY")),
    logger_phy_lib(isrlog::fetch_basic_logger("PHY_LIB")),
    lte_workers(MAX_WORKERS),
    nr_workers(logger_phy, MAX_WORKERS),
    common(logger_phy),
    sfsync(logger_phy, logger_phy_lib),
    prach_buffer(logger_phy),
    thread("PHY")
  {}

  ~phy() final { stop(); }

  // Init for LTE PHYs
  int init(const phy_args_t& args_, stack_interface_phy_lte* stack_, isrran::radio_interface_phy* radio_);

  void stop() final;

  void wait_initialize() final;
  bool is_initialized() final;

  void get_metrics(const isrran::isrran_rat_t& rat, phy_metrics_t* m) final;
  void isrran_phy_logger(phy_logger_level_t log_level, char* str);

  void radio_overflow() final;
  void radio_failure() final;

  /********** RRC INTERFACE ********************/

  bool cell_search(int earfcn) final;
  bool cell_select(phy_cell_t cell) final;

  // Sets the new PHY configuration for the given CC. The configuration is applied in the background. The notify()
  // function will be called when the reconfiguration is completed. Unless the PRACH configuration has changed, the
  // reconfiguration will not take more than 3 ms
  bool set_config(const isrran::phy_cfg_t& config, uint32_t cc_idx) final;

  // Adds or modifies the cell configuration for a given CC. If the EARFCN has changed w.r.t. the previous value, or if
  // the cell is new, this function might take a few hundred ms to complete, depending on the radio
  bool set_scell(isrran_cell_t cell_info, uint32_t cc_idx, uint32_t earfcn) final;

  // Applies a TDD configuration in the background. This function will take less than 3 ms to execute.
  void set_config_tdd(isrran_tdd_config_t& tdd_config) final;

  // Todo
  void set_config_mbsfn_sib2(isrran::mbsfn_sf_cfg_t* cfg_list, uint32_t nof_cfgs) final;
  void set_config_mbsfn_sib13(const isrran::sib13_t& sib13) final;
  void set_config_mbsfn_mcch(const isrran::mcch_msg_t& mcch) final;

  // This function applies the new configuration immediately
  void set_cells_to_meas(uint32_t earfcn, const std::set<uint32_t>& pci) final;

  // This function applies the new configuration immediately
  void meas_stop() final;

  // also MAC interface
  bool cell_is_camping() final;

  /********** MAC INTERFACE ********************/
  /* Transmits PRACH in the next opportunity */
  void         prach_send(uint32_t preamble_idx, int allowed_subframe, float target_power_dbm, float ta_base_sec) final;
  prach_info_t prach_get_info() final;

  /* Indicates the transmission of a SR signal in the next opportunity */
  void sr_send() final;
  int  sr_last_tx_tti() final;

  // Time advance commands
  void set_timeadv_rar(uint32_t tti, uint32_t ta_cmd) final;
  void set_timeadv(uint32_t tti, uint32_t ta_cmd) final;

  /* Activate / Disactivate SCell*/
  void deactivate_scells() final;
  void set_activation_deactivation_scell(uint32_t cmd, uint32_t tti) final;

  /* Sets RAR dci payload */
  void set_rar_grant(uint8_t grant_payload[ISRRAN_RAR_GRANT_LEN], uint16_t rnti) final;

  /*Set MAC->PHY MCH period  stopping point*/
  void set_mch_period_stop(uint32_t stop) final;

  float get_phr() final;
  float get_pathloss_db() final;

  uint32_t get_current_tti() final;

  void start_plot() final;

  const static int MAX_WORKERS     = 4;
  const static int DEFAULT_WORKERS = 4;

  std::string get_type() final { return "lte_soft"; }

  /********** NR INTERFACE ********************/
  int            init(const phy_args_nr_t& args_, stack_interface_phy_nr* stack_, isrran::radio_interface_phy* radio_);
  bool           set_config(const isrran::phy_cfg_nr_t& cfg) final;
  void           send_prach(const uint32_t prach_occasion,
                            const int      preamble_index,
                            const float    preamble_received_target_power,
                            const float    ta_base_sec = 0.0f) final;
  void           set_earfcn(std::vector<uint32_t> earfcns);
  bool           has_valid_sr_resource(uint32_t sr_id) final;
  void           clear_pending_grants() final;
  int            set_rar_grant(uint32_t                                       rar_slot_idx,
                               std::array<uint8_t, ISRRAN_RAR_UL_GRANT_NBITS> packed_ul_grant,
                               uint16_t                                       rnti,
                               isrran_rnti_type_t                             rnti_type) final;
  phy_nr_state_t get_state() override { return PHY_NR_STATE_IDLE; };
  void           reset_nr() override{};
  bool           start_cell_search(const cell_search_args_t& req) override { return false; };
  bool           start_cell_select(const cell_select_args_t& req) override { return false; };

private:
  void run_thread() final;
  void configure_prach_params();
  void reset();
  bool set_scell(isrran_cell_t cell_info, uint32_t cc_idx, uint32_t earfcn, bool run_in_background);
  void set_scell_cmd(isrran_cell_t cell_info, uint32_t cc_idx, uint32_t earfcn, bool earfcn_is_different);

  std::mutex              config_mutex;
  std::condition_variable config_cond;
  std::atomic<bool>       is_configured = {false};

  const static int SF_RECV_THREAD_PRIO = 0;
  const static int WORKERS_THREAD_PRIO = 2;

  isrran::radio_interface_phy* radio = nullptr;

  isrlog::basic_logger&           logger_phy;
  isrlog::basic_logger&           logger_phy_lib;
  isrue::stack_interface_phy_lte* stack    = nullptr;
  isrue::stack_interface_phy_nr*  stack_nr = nullptr;

  lte::worker_pool lte_workers;
  nr::worker_pool  nr_workers;
  phy_common       common;
  sync             sfsync;
  prach            prach_buffer;

  isrran_prach_cfg_t  prach_cfg  = {};
  isrran_tdd_config_t tdd_config = {};

  isrran::phy_cfg_t    config    = {};
  isrran::phy_cfg_nr_t config_nr = {};
  phy_args_t           args      = {};

  // Since cell_search/cell_select operations take a lot of time, we use another queue to process the other commands
  // in parallel and avoid accumulating in the queue
  phy_cmd_proc cmd_worker_cell, cmd_worker;

  // Tracks the current selected cell (last call to cell_select)
  isrran_cell_t selected_cell = {};

  // Tracks the current selected EARFCN (last call to cell_select)
  uint32_t selected_earfcn = 0;

  static void set_default_args(phy_args_t& args);
  bool        check_args(const phy_args_t& args);
};

} // namespace isrue

#endif // ISRUE_PHY_H
