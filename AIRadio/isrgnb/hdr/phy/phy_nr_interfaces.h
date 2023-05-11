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

#ifndef ISRRAN_PHY_NR_INTERFACES_H
#define ISRRAN_PHY_NR_INTERFACES_H

#include "isrran/isrran.h"
#include <vector>

namespace isrenb {

struct phy_cell_cfg_nr_t {
  isrran_carrier_nr_t carrier;
  uint32_t            rf_port;
  uint32_t            cell_id;
  float               gain_db;
  bool                  dl_measure;
};

using phy_cell_cfg_list_nr_t = std::vector<phy_cell_cfg_nr_t>;

} // namespace isrenb

#endif // ISRRAN_PHY_NR_INTERFACES_H
