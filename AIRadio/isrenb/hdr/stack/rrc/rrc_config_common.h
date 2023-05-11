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

#ifndef ISRRAN_RRC_CONFIG_COMMON_H
#define ISRRAN_RRC_CONFIG_COMMON_H

#include <cstdint>

namespace isrenb {

enum rrc_cfg_cqi_mode_t { RRC_CFG_CQI_MODE_PERIODIC = 0, RRC_CFG_CQI_MODE_APERIODIC, RRC_CFG_CQI_MODE_N_ITEMS };
static const char* rrc_cfg_cqi_mode_text[] = {"periodic", "aperiodic"};

inline const char* to_string(rrc_cfg_cqi_mode_t mode)
{
  return mode < RRC_CFG_CQI_MODE_N_ITEMS ? rrc_cfg_cqi_mode_text[mode] : "invalid CQI mode";
}

struct rrc_cfg_cqi_t {
  uint32_t           sf_mapping[80];
  uint32_t           nof_subframes;
  uint32_t           period;
  uint32_t           m_ri;
  bool               is_subband_enabled;
  uint32_t           subband_k;
  bool               simultaneousAckCQI;
  rrc_cfg_cqi_mode_t mode;
};

} // namespace isrenb

#endif // ISRRAN_RRC_CONFIG_COMMON_H
