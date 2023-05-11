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

#ifndef ISRRAN_SCHED_NR_INTERFACE_HELPERS_H
#define ISRRAN_SCHED_NR_INTERFACE_HELPERS_H

#include "sched_nr_interface.h"
#include "isrran/adt/optional_array.h"

namespace isrenb {

//////////////////////////////////// Search Space Helpers ////////////////////////////////////////////

/// Get a range of active search spaces in a PDCCH configuration
inline isrran::split_optional_span<isrran_search_space_t> view_active_search_spaces(isrran_pdcch_cfg_nr_t& pdcch)
{
  return isrran::split_optional_span<isrran_search_space_t>{pdcch.search_space, pdcch.search_space_present};
}
inline isrran::split_optional_span<const isrran_search_space_t>
view_active_search_spaces(const isrran_pdcch_cfg_nr_t& pdcch)
{
  return isrran::split_optional_span<const isrran_search_space_t>{pdcch.search_space, pdcch.search_space_present};
}

inline bool contains_dci_format(const isrran_search_space_t& ss, isrran_dci_format_nr_t dci_fmt)
{
  auto is_dci_fmt = [dci_fmt](const isrran_dci_format_nr_t& f) { return f == dci_fmt; };
  return std::any_of(&ss.formats[0], &ss.formats[ss.nof_formats], is_dci_fmt);
}

//////////////////////////////////// CORESET Helpers ////////////////////////////////////////////

/// Get a range of active coresets in a PDCCH configuration
inline isrran::split_optional_span<isrran_coreset_t> view_active_coresets(isrran_pdcch_cfg_nr_t& pdcch)
{
  return isrran::split_optional_span<isrran_coreset_t>{pdcch.coreset, pdcch.coreset_present};
}
inline isrran::split_optional_span<const isrran_coreset_t> view_active_coresets(const isrran_pdcch_cfg_nr_t& pdcch)
{
  return isrran::split_optional_span<const isrran_coreset_t>{pdcch.coreset, pdcch.coreset_present};
}

/// Get number of CCEs available in CORESET for PDCCH
uint32_t coreset_nof_cces(const isrran_coreset_t& coreset);

//////////////////////////////////// Sched Output Helpers ////////////////////////////////////////////

inline bool operator==(isrran_dci_location_t lhs, isrran_dci_location_t rhs)
{
  return lhs.ncce == rhs.ncce and lhs.L == rhs.L;
}

//////////////////////////////////// UE configuration Helpers ////////////////////////////////////////////

void                 make_mib_cfg(const sched_nr_cell_cfg_t& cfg, isrran_mib_nr_t* mib);
void                 make_ssb_cfg(const sched_nr_cell_cfg_t& cfg, isrran::phy_cfg_nr_t::ssb_cfg_t* ssb);
isrran::phy_cfg_nr_t get_common_ue_phy_cfg(const sched_nr_cell_cfg_t& cfg);

} // namespace isrenb

#endif // ISRRAN_SCHED_NR_INTERFACE_HELPERS_H
