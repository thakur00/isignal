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

#ifndef ISRUE_SEARCH_H
#define ISRUE_SEARCH_H

#include "isrran/radio/radio.h"
#include "isrran/isrlog/isrlog.h"
#include "isrran/isrran.h"

namespace isrue {

class search_callback
{
public:
  virtual int                          radio_recv_fnc(isrran::rf_buffer_t&, isrran_timestamp_t* rx_time) = 0;
  virtual void                         set_ue_sync_opts(isrran_ue_sync_t* q, float cfo)                  = 0;
  virtual isrran::radio_interface_phy* get_radio()                                                       = 0;
  virtual void                         set_rx_gain(float gain)                                           = 0;
};

// Class to run cell search
class search
{
public:
  typedef enum { CELL_NOT_FOUND, CELL_FOUND, ERROR, TIMEOUT } ret_code;

  explicit search(isrlog::basic_logger& logger) : logger(logger) {}
  ~search();
  void     init(isrran::rf_buffer_t& buffer_, uint32_t nof_rx_channels, search_callback* parent, int force_N_id_2_, int force_N_id_1_);
  void     reset();
  float    get_last_cfo();
  void     set_agc_enable(bool enable);
  ret_code run(isrran_cell_t* cell, std::array<uint8_t, ISRRAN_BCH_PAYLOAD_LEN>& bch_payload);
  void     set_cp_en(bool enable);

private:
  search_callback*       p = nullptr;
  isrlog::basic_logger&  logger;
  isrran::rf_buffer_t    buffer       = {};
  isrran_ue_cellsearch_t cs           = {};
  isrran_ue_mib_sync_t   ue_mib_sync  = {};
  int                    force_N_id_2 = 0;
  int                    force_N_id_1 = 0;
};

}; // namespace isrue

#endif // ISRUE_SEARCH_H
