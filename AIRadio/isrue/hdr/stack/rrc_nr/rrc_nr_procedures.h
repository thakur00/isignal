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

#include "isrran/isrlog/isrlog.h"
#include "isrue/hdr/stack/rrc_nr/rrc_nr.h"

#ifndef ISRRAN_RRC_NR_PROCEDURES_H
#define ISRRAN_RRC_NR_PROCEDURES_H

namespace isrue {

/********************************
 *         Procedures
 *******************************/

class rrc_nr::cell_selection_proc
{
public:
  enum class state_t { phy_cell_search, phy_cell_select, sib_acquire };

  using cell_selection_complete_ev = isrran::proc_result_t<rrc_cell_search_result_t>;
  explicit cell_selection_proc(rrc_nr& parent_);
  isrran::proc_outcome_t init();
  isrran::proc_outcome_t step();
  isrran::proc_outcome_t react(const rrc_interface_phy_nr::cell_search_result_t& event);
  isrran::proc_outcome_t react(const rrc_interface_phy_nr::cell_select_result_t& event);
  isrran::proc_outcome_t react(const bool sib1_found);

  void then(const cell_selection_complete_ev& proc_result) const;

  rrc_cell_search_result_t get_result() const { return rrc_search_result; }
  static const char*       name() { return "Cell Selection"; }

private:
  isrran::proc_outcome_t handle_cell_search_result(const rrc_interface_phy_nr::cell_search_result_t& result);

  // conts
  rrc_nr&                       rrc_handle;

  // state vars
  rrc_interface_phy_nr::cell_search_result_t phy_search_result = {};
  rrc_cell_search_result_t                   rrc_search_result = {};
  state_t                                    state;
};

class rrc_nr::setup_request_proc
{
public:
  explicit setup_request_proc(rrc_nr& parent_);
  isrran::proc_outcome_t init(isrran::nr_establishment_cause_t cause_,
                              isrran::unique_byte_buffer_t     dedicated_info_nas_);
  isrran::proc_outcome_t step();
  void                   then(const isrran::proc_state_t& result);
  isrran::proc_outcome_t react(const cell_selection_proc::cell_selection_complete_ev& e);
  static const char*     name() { return "Setup Request"; }

private:
  // const
  rrc_nr&               rrc_handle;
  isrlog::basic_logger& logger;
  // args
  isrran::nr_establishment_cause_t cause;
  isrran::unique_byte_buffer_t     dedicated_info_nas;

  // state variables
  enum class state_t { cell_selection, config_serving_cell, wait_t300 } state;
  rrc_cell_search_result_t    cell_search_ret;
  isrran::proc_future_t<void> serv_cfg_fut;
};

class rrc_nr::connection_setup_proc
{
public:
  explicit connection_setup_proc(rrc_nr& parent_);
  isrran::proc_outcome_t init(const asn1::rrc_nr::radio_bearer_cfg_s& radio_bearer_cfg_,
                              const asn1::rrc_nr::cell_group_cfg_s&   cell_group_,
                              isrran::unique_byte_buffer_t            dedicated_info_nas_);
  isrran::proc_outcome_t step() { return isrran::proc_outcome_t::yield; }
  static const char*     name() { return "Connection Setup"; }
  isrran::proc_outcome_t react(const bool& config_complete);
  void                   then(const isrran::proc_state_t& result);

private:
  // const
  rrc_nr&               rrc_handle;
  isrlog::basic_logger& logger;
  // args
  isrran::unique_byte_buffer_t dedicated_info_nas;
};

class rrc_nr::connection_reconf_no_ho_proc
{
public:
  explicit connection_reconf_no_ho_proc(rrc_nr& parent_);
  isrran::proc_outcome_t init(const reconf_initiator_t         initiator_,
                              const bool                       endc_release_and_add_r15,
                              const asn1::rrc_nr::rrc_recfg_s& rrc_nr_reconf);
  isrran::proc_outcome_t step() { return isrran::proc_outcome_t::yield; }
  static const char*     name() { return "NR Connection Reconfiguration"; }
  isrran::proc_outcome_t react(const bool& config_complete);
  void                   then(const isrran::proc_state_t& result);

private:
  // const
  rrc_nr&                        rrc_handle;
  reconf_initiator_t             initiator;
  asn1::rrc_nr::rrc_recfg_s      rrc_recfg;
  asn1::rrc_nr::cell_group_cfg_s cell_group_cfg;
};

} // namespace isrue

#endif // ISRRAN_RRC_NR_PROCEDURES_H
