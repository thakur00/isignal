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

#ifndef ISRRAN_SCHED_NR_COMMON_TEST_H
#define ISRRAN_SCHED_NR_COMMON_TEST_H

#include "isrgnb/hdr/stack/mac/sched_nr_pdcch.h"
#include "isrran/adt/span.h"

namespace isrenb {

/// Test DCI context consistency
void test_dci_ctx_consistency(const isrran_pdcch_cfg_nr_t& pdcch_cfg, const isrran_dci_ctx_t& dci);

/// Test PDCCH collisions
void test_pdcch_collisions(const isrran_pdcch_cfg_nr_t&                  pdcch_cfg,
                           isrran::const_span<sched_nr_impl::pdcch_dl_t> dl_pdcchs,
                           isrran::const_span<sched_nr_impl::pdcch_ul_t> ul_pddchs);

void test_dl_pdcch_consistency(const sched_nr_impl::cell_config_manager&     cell_cfg,
                               isrran::const_span<sched_nr_impl::pdcch_dl_t> dl_pdcch);
void test_pdsch_consistency(isrran::const_span<mac_interface_phy_nr::pdsch_t> dl_pdcch);
/// @brief Test whether the SSB grant gets scheduled with the correct periodicity.
void test_ssb_scheduled_grant(
    const isrran::slot_point&                                                                 sl_point,
    const sched_nr_impl::cell_config_manager&                                                 cell_cfg,
    const isrran::bounded_vector<mac_interface_phy_nr::ssb_t, mac_interface_phy_nr::MAX_SSB>& ssb_list);

} // namespace isrenb

#endif // ISRRAN_SCHED_NR_COMMON_TEST_H
