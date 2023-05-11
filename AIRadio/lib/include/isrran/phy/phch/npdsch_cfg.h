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

#ifndef ISRRAN_NPDSCH_CFG_H
#define ISRRAN_NPDSCH_CFG_H

#include "isrran/phy/phch/ra_nbiot.h"

/*
 * @brief Narrowband Physical downlink shared channel configuration
 *
 * Reference: 3GPP TS 36.211 version 13.2.0 Release 13 Sec. 10.2.3
 */
typedef struct ISRRAN_API {
  isrran_ra_nbiot_dl_grant_t grant;
  isrran_ra_nbits_t          nbits;
  bool                       is_encoded;
  bool                       has_bcch; // Whether this NPDSCH is carrying the BCCH
  uint32_t                   sf_idx;   // The current idx within the entire NPDSCH
  uint32_t                   rep_idx;  // The current repetion within this NPDSCH
  uint32_t                   num_sf;   // Total number of subframes tx'ed in this NPDSCH
} isrran_npdsch_cfg_t;

#endif // ISRRAN_NPDSCH_CFG_H
