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

#ifndef ISRUE_PRACH_H
#define ISRUE_PRACH_H

#include "isrran/interfaces/ue_phy_interfaces.h"
#include "isrran/radio/radio.h"
#include "isrran/isrlog/isrlog.h"
#include "isrran/isrran.h"
#include <bitset>
#include <mutex>

namespace isrue {

class prach
{
public:
  prach(isrlog::basic_logger& logger) : logger(logger) {}
  ~prach() { stop(); }

  void  init(uint32_t max_prb);
  void  stop();
  bool  set_cell(isrran_cell_t cell, isrran_prach_cfg_t prach_cfg);
  bool  prepare_to_send(uint32_t preamble_idx, int allowed_subframe = -1, float target_power_dbm = -1);
  bool  is_ready_to_send(uint32_t current_tti, uint32_t current_pci);
  bool  is_pending() const;
  cf_t* generate(float cfo, uint32_t* nof_sf, float* target_power = NULL);

  phy_interface_mac_lte::prach_info_t get_info() const;

private:
  bool generate_buffer(uint32_t f_idx);

private:
  static constexpr unsigned MAX_LEN_SF    = 3;
  static constexpr unsigned max_fs        = 12;
  static constexpr unsigned max_preambles = 64;

  isrlog::basic_logger& logger;
  isrran_prach_t        prach_obj        = {};
  isrran_cell_t         cell             = {};
  isrran_cfo_t          cfo_h            = {};
  isrran_prach_cfg_t    cfg              = {};
  cf_t*                 signal_buffer    = nullptr;
  int                   preamble_idx     = -1;
  uint32_t              len              = 0;
  int                   allowed_subframe = 0;
  int                   transmitted_tti  = 0;
  float                 target_power_dbm = 0;
  bool                  mem_initiated    = false;
  bool                  cell_initiated   = false;
  mutable std::mutex    mutex;
};

} // namespace isrue

#endif // ISRUE_PRACH_H
