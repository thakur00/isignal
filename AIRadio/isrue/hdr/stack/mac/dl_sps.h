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

#ifndef ISRUE_DL_SPS_H
#define ISRUE_DL_SPS_H

#include "isrran/common/timers.h"

/* Downlink Semi-Persistent schedulign (Section 5.10.1) */

namespace isrue {

class dl_sps
{
public:
  void clear() {}
  void reset() {}
  bool get_pending_grant(uint32_t tti, mac_interface_phy_lte::mac_grant_dl_t* grant) { return false; }

private:
};

} // namespace isrue

#endif // ISRUE_DL_SPS_H
