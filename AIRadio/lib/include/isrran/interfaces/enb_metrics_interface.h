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

#ifndef ISRRAN_ENB_METRICS_INTERFACE_H
#define ISRRAN_ENB_METRICS_INTERFACE_H

#include <stdint.h>

#include "isrenb/hdr/common/common_enb.h"
#include "isrenb/hdr/phy/phy_metrics.h"
#include "isrenb/hdr/stack/mac/common/mac_metrics.h"
#include "isrenb/hdr/stack/rrc/rrc_metrics.h"
#include "isrenb/hdr/stack/s1ap/s1ap_metrics.h"
#include "isrran/common/metrics_hub.h"
#include "isrran/radio/radio_metrics.h"
#include "isrran/rlc/rlc_metrics.h"
#include "isrran/system/sys_metrics.h"
#include "isrran/upper/pdcp_metrics.h"
#include "isrue/hdr/stack/upper/gw_metrics.h"

namespace isrenb {

struct rlc_metrics_t {
  std::vector<isrran::rlc_metrics_t> ues;
};

struct pdcp_metrics_t {
  std::vector<isrran::pdcp_metrics_t> ues;
};

struct stack_metrics_t {
  mac_metrics_t  mac;
  rrc_metrics_t  rrc;
  rlc_metrics_t  rlc;
  pdcp_metrics_t pdcp;
  s1ap_metrics_t s1ap;
};

struct enb_metrics_t {
  isrran::rf_metrics_t       rf;
  std::vector<phy_metrics_t> phy;
  stack_metrics_t            stack;
  stack_metrics_t            nr_stack;
  isrran::sys_metrics_t      sys;
  bool                       running;
};

// ENB interface
class enb_metrics_interface : public isrran::metrics_interface<enb_metrics_t>
{
public:
  virtual bool get_metrics(enb_metrics_t* m) = 0;
};

} // namespace isrenb

#endif // ISRRAN_ENB_METRICS_INTERFACE_H
