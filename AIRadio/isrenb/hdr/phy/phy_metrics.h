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

#ifndef ISRENB_PHY_METRICS_H
#define ISRENB_PHY_METRICS_H

#include <limits>

namespace isrenb {

// PHY metrics per user

struct ul_metrics_t {
  float   n;
  float   pusch_sinr;
  float   pusch_rssi;
  int64_t pusch_tpc;
  float   pucch_sinr;
  float   pucch_rssi;
  float   pucch_ni;
  float   turbo_iters;
  float   mcs;
  int     n_samples;
  int     n_samples_pucch;
};

struct dl_metrics_t {
  float mcs;
  int64_t pucch_tpc;
  int     n_samples;
};

struct phy_metrics_t {
  dl_metrics_t dl;
  ul_metrics_t ul;
};

} // namespace isrenb

#endif // ISRENB_PHY_METRICS_H
