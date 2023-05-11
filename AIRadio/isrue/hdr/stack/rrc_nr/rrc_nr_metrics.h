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

#ifndef ISRUE_RRC_NR_METRICS_H
#define ISRUE_RRC_NR_METRICS_H

namespace isrue {

/// RRC states (3GPP 38.331 v15.5.1 Sec 4.2.1)
enum rrc_nr_state_t {
  RRC_NR_STATE_IDLE = 0,
  RRC_NR_STATE_CONNECTED,
  RRC_NR_STATE_CONNECTED_INACTIVE,
  RRC_NR_STATE_N_ITEMS,
};

struct rrc_nr_metrics_t {
  rrc_nr_state_t state;
};

} // namespace isrue

#endif // ISRUE_RRC_NR_METRICS_H
