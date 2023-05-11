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

#ifndef ISRRAN_DUMMY_UE_PHY_H
#define ISRRAN_DUMMY_UE_PHY_H

#include "isrue/hdr/phy/nr/worker_pool.h"

class ue_dummy_phy : public isrue::phy_interface_stack_nr
{
public:
  ue_dummy_phy(const char* logname, uint32_t max_workers) :
    logger(isrlog::fetch_basic_logger(logname)), workers(logger, max_workers)
  {}
  bool init(const isrue::phy_args_nr_t&    args_,
            isrran::phy_common_interface&  common,
            isrue::stack_interface_phy_nr* stack_,
            int                            prio)
  {
    return workers.init(args_, common, stack_, prio);
  }
  void                  stop() { workers.stop(); }
  isrue::nr::sf_worker* wait_worker(uint32_t tti) { return workers.wait_worker(tti); }
  void                  start_worker(isrue::nr::sf_worker* w) { workers.start_worker(w); }
  void                  get_metrics(isrue::phy_metrics_t& m) { return workers.get_metrics(m); }

  int set_rar_grant(uint32_t                                       rar_slot_idx,
                    std::array<uint8_t, ISRRAN_RAR_UL_GRANT_NBITS> packed_ul_grant,
                    uint16_t                                       rnti,
                    isrran_rnti_type_t                             rnti_type) override
  {
    return workers.set_rar_grant(rar_slot_idx, packed_ul_grant, rnti, rnti_type);
  }
  void send_prach(const uint32_t prach_occasion,
                  const int      preamble_index,
                  const float    preamble_received_target_power,
                  const float    ta_base_sec = 0.0f) override
  {}

  bool has_valid_sr_resource(uint32_t sr_id) override { return workers.has_valid_sr_resource(sr_id); }
  void clear_pending_grants() override { return workers.clear_pending_grants(); }

  bool           set_config(const isrran::phy_cfg_nr_t& cfg) override { return workers.set_config(cfg); }
  phy_nr_state_t get_state() override { return PHY_NR_STATE_IDLE; }
  void           reset_nr() override {}

  bool start_cell_search(const cell_search_args_t& req) override { return false; }

  bool start_cell_select(const cell_select_args_t& req) override { return false; }

private:
  isrlog::basic_logger&  logger;
  isrue::nr::worker_pool workers;
};

#endif // ISRRAN_DUMMY_UE_PHY_H
