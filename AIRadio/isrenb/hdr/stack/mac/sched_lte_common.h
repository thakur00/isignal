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

#ifndef ISRRAN_SCHED_LTE_COMMON_H
#define ISRRAN_SCHED_LTE_COMMON_H

#include "sched_interface.h"
#include "isrran/adt/bounded_bitset.h"
#include "isrran/common/tti_point.h"

namespace isrenb {

/***********************
 *     Constants
 **********************/

constexpr float    tti_duration_ms = 1;
constexpr uint32_t NOF_AGGR_LEVEL  = 4;

/***********************
 *   Helper Types
 **********************/

/// List of CCE start positions in PDCCH
using cce_position_list = isrran::bounded_vector<uint32_t, 6>;

/// Map {L} -> list of CCE positions
using cce_cfi_position_table = std::array<cce_position_list, NOF_AGGR_LEVEL>;

/// Map {cfi, L} -> list of CCE positions
using cce_sf_position_table = std::array<std::array<cce_position_list, NOF_AGGR_LEVEL>, ISRRAN_NOF_CFI>;

/// Map {sf, cfi, L} -> list of CCE positions
using cce_frame_position_table = std::array<cce_sf_position_table, ISRRAN_NOF_SF_X_FRAME>;

/// structs to bundle together all the sched arguments, and share them with all the sched sub-components
class sched_cell_params_t
{
  struct regs_deleter {
    void operator()(isrran_regs_t* p);
  };

public:
  bool set_cfg(uint32_t                             enb_cc_idx_,
               const sched_interface::cell_cfg_t&   cfg_,
               const sched_interface::sched_args_t& sched_args);
  // convenience getters
  uint32_t nof_prbs_to_rbgs(uint32_t nof_prbs) const { return isrran::ceil_div(nof_prbs, P); }
  uint32_t nof_prb() const { return cfg.cell.nof_prb; }
  uint32_t get_dl_lb_nof_re(tti_point tti_tx_dl, uint32_t nof_prbs_alloc) const;
  uint32_t get_dl_nof_res(isrran::tti_point tti_tx_dl, const isrran_dci_dl_t& dci, uint32_t cfi) const;

  uint32_t                                     enb_cc_idx       = 0;
  sched_interface::cell_cfg_t                  cfg              = {};
  isrran_pucch_cfg_t                           pucch_cfg_common = {};
  const sched_interface::sched_args_t*         sched_cfg        = nullptr;
  std::unique_ptr<isrran_regs_t, regs_deleter> regs;
  cce_sf_position_table                        common_locations = {};
  cce_frame_position_table                     rar_locations    = {};
  std::array<uint32_t, ISRRAN_NOF_CFI>         nof_cce_table    = {}; ///< map cfix -> nof cces in PDCCH
  uint32_t                                     P                = 0;
  uint32_t                                     nof_rbgs         = 0;

  using dl_nof_re_table = isrran::bounded_vector<
      std::array<std::array<std::array<uint32_t, ISRRAN_NOF_CFI>, ISRRAN_NOF_SLOTS_PER_SF>, ISRRAN_NOF_SF_X_FRAME>,
      ISRRAN_MAX_PRB>;
  using dl_lb_nof_re_table = std::array<isrran::bounded_vector<uint32_t, ISRRAN_MAX_PRB>, ISRRAN_NOF_SF_X_FRAME>;

  /// Table of nof REs
  dl_nof_re_table nof_re_table;
  /// Cached computation of Lower bound of nof REs
  dl_lb_nof_re_table nof_re_lb_table;
};

/// Type of Allocation stored in PDSCH/PUSCH
enum class alloc_type_t { DL_BC, DL_PCCH, DL_RAR, DL_PDCCH_ORDER, DL_DATA, UL_DATA };
inline bool is_dl_ctrl_alloc(alloc_type_t a)
{
  return a == alloc_type_t::DL_BC or a == alloc_type_t::DL_PCCH or a == alloc_type_t::DL_RAR or
         a == alloc_type_t::DL_PDCCH_ORDER;
}

} // namespace isrenb

#endif // ISRRAN_SCHED_LTE_COMMON_H
