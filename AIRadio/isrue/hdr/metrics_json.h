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
 * File:        metrics_json.h
 * Description: Metrics class printing to a json file.
 *****************************************************************************/

#ifndef ISRUE_METRICS_JSON_H
#define ISRUE_METRICS_JSON_H

#include "isrran/isrlog/log_channel.h"
#include "ue_metrics_interface.h"

namespace isrue {

class metrics_json : public isrran::metrics_listener<ue_metrics_t>
{
public:
  explicit metrics_json(isrlog::log_channel& c) : log_c(c) {}

  void set_metrics(const ue_metrics_t& m, const uint32_t period_usec) override;
  void set_ue_handle(ue_metrics_interface* ue_);
  void stop() override {}

private:
  isrlog::log_channel&  log_c;
  ue_metrics_interface* ue = nullptr;

  std::mutex mutex = {};
};

} // namespace isrue

#endif // ISRUE_METRICS_JSON_H
