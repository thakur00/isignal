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

#ifndef ISRRAN_SCHED_NR_PDCCH_H
#define ISRRAN_SCHED_NR_PDCCH_H

#include "isrenb/hdr/stack/mac/sched_common.h"
#include "isrgnb/hdr/stack/mac/sched_nr_cfg.h"
#include "isrgnb/hdr/stack/mac/sched_ue/ue_cfg_manager.h"
#include "isrran/adt/bounded_bitset.h"
#include "isrran/adt/bounded_vector.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/phch/dci.h"

namespace isrenb {

namespace sched_nr_impl {

/// Helper function to fill DCI with BWP params
void fill_dci_from_cfg(const bwp_params_t& bwp_cfg, isrran_dci_dl_nr_t& dci);
void fill_dci_from_cfg(const bwp_params_t& bwp_cfg, isrran_dci_ul_nr_t& dci);

using coreset_bitmap = isrran::bounded_bitset<ISRRAN_CORESET_FREQ_DOMAIN_RES_SIZE * ISRRAN_CORESET_DURATION_MAX, true>;

class coreset_region
{
public:
  coreset_region(const bwp_params_t& bwp_cfg_, uint32_t coreset_id_, uint32_t slot_idx);
  void reset();

  bool alloc_pdcch(isrran_rnti_type_t         rnti_type,
                   bool                       is_dl,
                   uint32_t                   aggr_idx,
                   uint32_t                   search_space_id,
                   const ue_carrier_params_t* user,
                   isrran_dci_ctx_t&          dci);

  void rem_last_pdcch();

  uint32_t get_td_symbols() const { return coreset_cfg->duration; }
  uint32_t get_freq_resources() const { return nof_freq_res; }
  uint32_t nof_cces() const { return nof_freq_res * get_td_symbols(); }
  size_t   nof_allocs() const { return dfs_tree.size(); }

  void print_allocations(fmt::memory_buffer& fmtbuf) const;

private:
  const isrran_coreset_t* coreset_cfg;
  uint32_t                coreset_id;
  uint32_t                slot_idx;
  uint32_t                nof_freq_res = 0;

  const bwp_cce_pos_list&                          rar_cce_list;
  const isrran::optional_vector<bwp_cce_pos_list>& common_cce_list;

  // List of PDCCH grants
  struct alloc_record {
    uint32_t                   aggr_idx;
    uint32_t                   ss_id;
    isrran_dci_ctx_t*          dci;
    bool                       is_dl;
    const ue_carrier_params_t* ue;
  };
  isrran::bounded_vector<alloc_record, 2 * MAX_GRANTS> dci_list;

  // DFS decision tree of PDCCH grants
  struct tree_node {
    uint16_t              rnti        = ISRRAN_INVALID_RNTI;
    uint32_t              record_idx  = 0;
    uint32_t              dci_pos_idx = 0;
    isrran_dci_location_t dci_pos     = {0, 0};
    /// Accumulation of all PDCCH masks for the current solution (DFS path)
    coreset_bitmap total_mask, current_mask;
  };
  using alloc_tree_dfs_t = std::vector<tree_node>;
  alloc_tree_dfs_t dfs_tree, saved_dfs_tree;

  isrran::span<const uint32_t> get_cce_loc_table(const alloc_record& record) const;
  bool                         alloc_dfs_node(const alloc_record& record, uint32_t dci_idx);
  bool                         get_next_dfs();
};

using pdcch_dl_alloc_result = isrran::expected<pdcch_dl_t*, alloc_result>;
using pdcch_ul_alloc_result = isrran::expected<pdcch_ul_t*, alloc_result>;

/**
 * Class to handle the allocation of REs for a BWP PDCCH in a specific slot
 */
class bwp_pdcch_allocator
{
public:
  bwp_pdcch_allocator(const bwp_params_t& bwp_cfg_,
                      uint32_t            slot_idx,
                      pdcch_dl_list_t&    pdcch_dl_list,
                      pdcch_ul_list_t&    pdcch_ul_list);

  /**
   * Clear current slot allocations
   */
  void reset();

  /**
   * Allocates RE space for RAR DCI in PDCCH, avoiding in the process collisions with other PDCCH allocations
   * Fills DCI context with RAR PDCCH allocation information
   * @param ra_rnti RA-RNTI of RAR allocation
   * @param aggr_idx Aggregation level index (0..4)
   * @return PDCCH object with dci context filled if the allocation was successful. nullptr otherwise
   */
  pdcch_dl_alloc_result alloc_rar_pdcch(uint16_t ra_rnti, uint32_t aggr_idx);

  /**
   * Allocates RE space for SI DCI in PDCCH, avoiding in the process collisions with other PDCCH allocations
   * Fills DCI context with SI PDCCH allocation information
   * @param ss_id Search space ID
   * @param aggr_idx Aggregation level index (0..4)
   * @return PDCCH object with dci context filled if the allocation was successful. nullptr otherwise
   */
  pdcch_dl_alloc_result alloc_si_pdcch(uint32_t ss_id, uint32_t aggr_idx);

  /**
   * Allocates RE space for UE DL DCI in PDCCH, avoiding in the process collisions with other PDCCH allocations
   * Fills DCI context with PDCCH allocation information
   * @param rnti_type type of UE RNTI (e.g. C, TC)
   * @param ss_id Search space ID
   * @param aggr_idx Aggregation level index (0..4)
   * @param user UE object parameters
   * @return PDCCH object with dci context filled if the allocation was successful. nullptr otherwise
   */
  pdcch_dl_alloc_result
  alloc_dl_pdcch(isrran_rnti_type_t rnti_type, uint32_t ss_id, uint32_t aggr_idx, const ue_carrier_params_t& user);

  /**
   * @brief Allocates RE space for UL DCI in PDCCH, avoiding in the process collisions with other PDCCH allocations
   * Fills DCI context with PDCCH allocation information
   * @param ss_id Search space ID
   * @param aggr_idx Aggregation level index (0..4)
   * @param user UE object parameters
   * @return PDCCH object with dci context filled if the allocation was successful. nullptr otherwise
   */
  pdcch_ul_alloc_result alloc_ul_pdcch(uint32_t ss_id, uint32_t aggr_idx, const ue_carrier_params_t& user);

  /**
   * Cancel and remove last PDCCH allocation. It should only be called once after each alloc_dl_pdcch/alloc_ul_pdcch
   */
  void cancel_last_pdcch();

  /// Returns the number of PDCCH allocations made in the slot
  uint32_t nof_allocations() const;

  /// Number of CCEs in given coreset
  uint32_t nof_cces(uint32_t coreset_id) const;

  void        print_allocations(fmt::memory_buffer& fmtbuf) const;
  std::string print_allocations() const;

private:
  using slot_coreset_list = isrran::optional_array<coreset_region, ISRRAN_UE_DL_NR_MAX_NOF_CORESET>;

  pdcch_dl_alloc_result alloc_dl_pdcch_common(isrran_rnti_type_t         rnti_type,
                                              uint16_t                   rnti,
                                              uint32_t                   ss_id,
                                              uint32_t                   aggr_idx,
                                              isrran_dci_format_nr_t     dci_fmt,
                                              const ue_carrier_params_t* user = nullptr);

  /// Helper function to verify valid inputs
  alloc_result check_args_valid(isrran_rnti_type_t         rnti_type,
                                uint16_t                   rnti,
                                uint32_t                   ss_id,
                                uint32_t                   aggr_idx,
                                isrran_dci_format_nr_t     dci_fmt,
                                const ue_carrier_params_t* user,
                                bool                       is_dl) const;

  /// Fill DCI context of allocated PDCCH
  void fill_dci_ctx_common(isrran_dci_ctx_t&            dci,
                           isrran_rnti_type_t           rnti_type,
                           uint16_t                     rnti,
                           const isrran_search_space_t& ss,
                           isrran_dci_format_nr_t       dci_fmt,
                           const ue_carrier_params_t*   ue);

  // args
  const bwp_params_t&   bwp_cfg;
  isrlog::basic_logger& logger;
  const uint32_t        slot_idx;

  pdcch_dl_list_t&        pdcch_dl_list;
  pdcch_ul_list_t&        pdcch_ul_list;
  slot_coreset_list       coresets;
  const isrran_dci_ctx_t* pending_dci = nullptr; /// Saves last PDCCH allocation, in case it needs to be aborted
};

} // namespace sched_nr_impl

} // namespace isrenb

#endif // ISRRAN_SCHED_NR_PDCCH_H
