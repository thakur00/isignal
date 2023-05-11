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

#ifndef ISRENB_S1AP_METRICS_H
#define ISRENB_S1AP_METRICS_H

namespace isrenb {

typedef enum {
  S1AP_ATTACHING = 0, // Attempting to create S1 connection
  S1AP_READY,         // S1 connected
  S1AP_ERROR          // Failure
} S1AP_STATUS_ENUM;

struct s1ap_metrics_t {
  S1AP_STATUS_ENUM status;
};

} // namespace isrenb

#endif // ISRENB_S1AP_METRICS_H
