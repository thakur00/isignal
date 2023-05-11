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

#ifndef ISRENB_PHY_H
#define ISRENB_PHY_H

#include "lte/sf_worker.h"
#include "phy_common.h"
#include "isrenb/hdr/phy/enb_phy_base.h"
#include "isrran/common/trace.h"
#include "isrran/interfaces/enb_metrics_interface.h"
#include "isrran/interfaces/enb_time_interface.h"
#include "isrran/interfaces/radio_interfaces.h"
#include "isrran/radio/radio.h"
#include "isrran/isrlog/isrlog.h"
#include "txrx.h"

namespace isrenb {

class phy final : public enb_phy_base,
                  public phy_interface_stack_lte,
                  public phy_interface_stack_nr,
                  public isrran::phy_interface_radio
{
public:
  phy(isrlog::sink& log_sink);
  ~phy();

  int  init(const phy_args_t&            args,
            const phy_cfg_t&             cfg,
            isrran::radio_interface_phy* radio_,
            stack_interface_phy_lte*     stack_lte_,
            stack_interface_phy_nr&      stack_nr_,
            enb_time_interface*          enb_);
  int  init(const phy_args_t&            args,
            const phy_cfg_t&             cfg,
            isrran::radio_interface_phy* radio_,
            stack_interface_phy_lte*     stack_,
            enb_time_interface*          enb_);
  void stop() override;

  std::string get_type() override { return "lte"; };

  /* MAC->PHY interface */
  void rem_rnti(uint16_t rnti) final;
  void set_mch_period_stop(uint32_t stop) final;
  void set_activation_deactivation_scell(uint16_t                                     rnti,
                                         const std::array<bool, ISRRAN_MAX_CARRIERS>& activation) override;

  /*RRC-PHY interface*/
  void configure_mbsfn(isrran::sib2_mbms_t* sib2, isrran::sib13_t* sib13, const isrran::mcch_msg_t& mcch) override;

  void start_plot() override;
  void set_config(uint16_t rnti, const phy_rrc_cfg_list_t& phy_cfg_list) override;
  void complete_config(uint16_t rnti) override;

  void get_metrics(std::vector<phy_metrics_t>& metrics) override;

  void cmd_cell_gain(uint32_t cell_id, float gain_db) override;
  void cmd_cell_measure() override;

  void radio_overflow() override{};
  void radio_failure() override{};

  void isrran_phy_logger(phy_logger_level_t log_level, char* str);

  int set_common_cfg(const common_cfg_t& common_cfg) override;

private:
  isrran::phy_cfg_mbsfn_t mbsfn_config = {};
  uint32_t                nof_workers  = 0;

  const static int MAX_WORKERS = 4;

  const static int PRACH_WORKER_THREAD_PRIO = 5;
  const static int SF_RECV_THREAD_PRIO      = 1;
  const static int WORKERS_THREAD_PRIO      = 2;

  isrran::radio_interface_phy* radio = nullptr;

  isrlog::sink&         log_sink;
  isrlog::basic_logger& phy_log;
  isrlog::basic_logger& phy_lib_log;

  lte::worker_pool                 lte_workers;
  std::unique_ptr<nr::worker_pool> nr_workers;
  phy_common                       workers_common;
  prach_worker_pool                prach;
  txrx                             tx_rx;

  bool initialized = false;

  isrran_prach_cfg_t prach_cfg  = {};
  common_cfg_t       common_cfg = {};

  void parse_common_config(const phy_cfg_t& cfg);
  int  init_lte(const phy_args_t&            args,
                const phy_cfg_t&             cfg,
                isrran::radio_interface_phy* radio_,
                stack_interface_phy_lte*     stack_,
                enb_time_interface*          enb_);
  int  init_nr(const phy_args_t& args, const phy_cfg_t& cfg, stack_interface_phy_nr& stack);
};

} // namespace isrenb

#endif // ISRENB_PHY_H
