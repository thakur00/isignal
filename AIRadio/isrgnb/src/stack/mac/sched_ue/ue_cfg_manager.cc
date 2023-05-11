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

#include "isrgnb/hdr/stack/mac/sched_ue/ue_cfg_manager.h"
#include "isrgnb/hdr/stack/mac/sched_nr_helpers.h"
#include "isrran/asn1/rrc_nr_utils.h"

namespace isrenb {
namespace sched_nr_impl {

ue_cfg_manager::ue_cfg_manager(uint32_t enb_cc_idx) : carriers(1)
{
  carriers[enb_cc_idx].active = true;
  carriers[enb_cc_idx].cc     = enb_cc_idx;
  ue_bearers[0].direction     = mac_lc_ch_cfg_t::BOTH;
}

ue_cfg_manager::ue_cfg_manager(const sched_nr_ue_cfg_t& cfg_req) : ue_cfg_manager()
{
  apply_config_request(cfg_req);
}

int ue_cfg_manager::apply_config_request(const sched_nr_ue_cfg_t& cfg_req)
{
  maxharq_tx = cfg_req.maxharq_tx;
  carriers   = cfg_req.carriers;
  phy_cfg    = cfg_req.phy_cfg;

  if (cfg_req.sp_cell_cfg.is_present()) {
    isrran::make_pdsch_cfg_from_serv_cell(cfg_req.sp_cell_cfg->sp_cell_cfg_ded, &phy_cfg.pdsch);
    isrran::make_csi_cfg_from_serv_cell(cfg_req.sp_cell_cfg->sp_cell_cfg_ded, &phy_cfg.csi);
  }
  for (uint32_t lcid : cfg_req.lc_ch_to_rem) {
    assert(lcid > 0 && "LCID=0 cannot be removed");
    ue_bearers[lcid] = {};
  }
  for (const sched_nr_ue_lc_ch_cfg_t& lc_ch : cfg_req.lc_ch_to_add) {
    assert(lc_ch.lcid > 0 && "LCID=0 cannot be configured");
    ue_bearers[lc_ch.lcid] = lc_ch.cfg;
  }

  return ISRRAN_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ue_carrier_params_t::ue_carrier_params_t(uint16_t rnti_, const bwp_params_t& bwp_cfg_, const ue_cfg_manager& uecfg_) :
  rnti(rnti_), cc(bwp_cfg_.cc), cfg_(&uecfg_), bwp_cfg(&bwp_cfg_), cached_dci_cfg(uecfg_.phy_cfg.get_dci_cfg())
{
  std::fill(ss_id_to_cce_idx.begin(), ss_id_to_cce_idx.end(), ISRRAN_UE_DL_NR_MAX_NOF_SEARCH_SPACE);
  const auto& pdcch        = phy().pdcch;
  auto        ss_view      = isrran::make_optional_span(pdcch.search_space, pdcch.search_space_present);
  auto        coreset_view = isrran::make_optional_span(pdcch.coreset, pdcch.coreset_present);
  for (const auto& ss : ss_view) {
    isrran_assert(coreset_view.contains(ss.coreset_id),
                  "Invalid mapping search space id=%d to coreset id=%d",
                  ss.id,
                  ss.coreset_id);
    cce_positions_list.emplace_back();
    get_dci_locs(coreset_view[ss.coreset_id], ss, rnti, cce_positions_list.back());
    ss_id_to_cce_idx[ss.id] = cce_positions_list.size() - 1;
  }
}

int ue_carrier_params_t::find_ss_id(isrran_dci_format_nr_t dci_fmt) const
{
  static const uint32_t           aggr_idx  = 2;                  // TODO: Make it dynamic
  static const isrran_rnti_type_t rnti_type = isrran_rnti_type_c; // TODO: Use TC-RNTI for Msg4

  auto active_ss_lst = view_active_search_spaces(phy().pdcch);

  for (const isrran_search_space_t& ss : active_ss_lst) {
    // Prioritize UE-dedicated SearchSpaces
    if (ss.type == isrran_search_space_type_ue and ss.nof_candidates[aggr_idx] > 0 and
        contains_dci_format(ss, dci_fmt) and is_rnti_type_valid_in_search_space(rnti_type, ss.type)) {
      return ss.id;
    }
  }
  // Search Common SearchSpaces
  for (const isrran_search_space_t& ss : active_ss_lst) {
    if (ISRRAN_SEARCH_SPACE_IS_COMMON(ss.type) and ss.nof_candidates[aggr_idx] > 0 and
        contains_dci_format(ss, dci_fmt) and is_rnti_type_valid_in_search_space(rnti_type, ss.type)) {
      return ss.id;
    }
  }
  return -1;
}

} // namespace sched_nr_impl
} // namespace isrenb
