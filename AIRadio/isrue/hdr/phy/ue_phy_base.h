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

/******************************************************************************
 * File:        ue_phy_base.h
 * Description: Base class for all UE PHYs.
 *****************************************************************************/

#ifndef ISRUE_UE_PHY_BASE_H
#define ISRUE_UE_PHY_BASE_H

#include "isrran/interfaces/ue_phy_interfaces.h"
#include "isrue/hdr/phy/phy_metrics.h"

namespace isrue {

class ue_phy_base
{
public:
  ue_phy_base(){};
  virtual ~ue_phy_base(){};

  virtual std::string get_type() = 0;

  virtual void stop() = 0;

  virtual void wait_initialize() = 0;
  virtual bool is_initialized()  = 0;
  virtual void start_plot()      = 0;

  virtual void get_metrics(const isrran::isrran_rat_t& rat, phy_metrics_t* m) = 0;
};

} // namespace isrue

#endif // ISRUE_UE_PHY_BASE_H
