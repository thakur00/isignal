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

#ifndef ISRUE_RRC_METRICS_H
#define ISRUE_RRC_METRICS_H

#include "isrran/interfaces/phy_interface_types.h"

namespace isrue {

// RRC states (3GPP 36.331 v10.0.0)
typedef enum {
  RRC_STATE_IDLE = 0,
  RRC_STATE_CONNECTED,
  RRC_STATE_N_ITEMS,
} rrc_state_t;
static const char rrc_state_text[RRC_STATE_N_ITEMS][100] = {"IDLE", "CONNECTED"};

struct rrc_metrics_t {
  rrc_state_t             state;
  std::vector<phy_meas_t> neighbour_cells;
};

} // namespace isrue

#endif // ISRUE_RRC_METRICS_H
