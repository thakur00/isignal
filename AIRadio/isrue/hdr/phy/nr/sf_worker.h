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

#ifndef ISRUE_NR_PHCH_WORKER_H
#define ISRUE_NR_PHCH_WORKER_H

#include "cc_worker.h"
#include "isrran/common/thread_pool.h"
#include "isrran/interfaces/phy_common_interface.h"

namespace isrue {
namespace nr {

/**
 * The sf_worker class handles the PHY processing, UL and DL procedures associated with 1 subframe.
 * It contains multiple cc_worker objects, one for each component carrier which may be executed in
 * one or multiple threads.
 *
 * A sf_worker object is executed by a thread within the thread_pool.
 */

class sf_worker final : public isrran::thread_pool::worker
{
public:
  sf_worker(isrran::phy_common_interface& common_,
            state&                        phy_state_,
            const isrran::phy_cfg_nr_t&   cfg,
            isrlog::basic_logger&         logger);
  ~sf_worker() = default;

  /* Functions used by main PHY thread */
  cf_t*    get_buffer(uint32_t cc_idx, uint32_t antenna_idx);
  uint32_t get_buffer_len();
  void     set_context(const isrran::phy_common_interface::worker_context_t& w_ctx);
  int      read_pdsch_d(cf_t* pdsch_d);
  void     start_plot();
  void     set_cfg(const isrran::phy_cfg_nr_t& new_cfg)
  {
    for (unsigned i = 0, e = cc_workers.size(); i != e; ++i) {
      update_cfg(i, new_cfg);
    }
    sf_len = ISRRAN_SF_LEN_PRB_NR(new_cfg.carrier.nof_prb);
  }

  void set_prach(cf_t* prach_ptr, float prach_power);

private:
  /* Inherited from thread_pool::worker. Function called every subframe to run the DL/UL processing */
  void work_imp() override;

  void update_cfg(uint32_t cc_idx, const isrran::phy_cfg_nr_t& new_cfg);

private:
  std::vector<std::unique_ptr<cc_worker> > cc_workers;

  isrran::phy_common_interface&                  common;
  state&                                         phy_state;
  isrlog::basic_logger&                          logger;
  isrran::rf_timestamp_t                         tx_time     = {};
  cf_t*                                          prach_ptr   = nullptr;
  float                                          prach_power = 0;
  isrran::phy_common_interface::worker_context_t context     = {};
  uint32_t                                       sf_len      = 0;
};

} // namespace nr
} // namespace isrue

#endif // ISRUE_NR_PHCH_WORKER_H
