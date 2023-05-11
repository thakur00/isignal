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

#ifndef ISRRAN_CHANNEL_H
#define ISRRAN_CHANNEL_H

#include "ch_awgn.h"
#include "delay.h"
#include "fading.h"
#include "hst.h"
#include "rlf.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/isrlog/isrlog.h"
#include <memory>
#include <string>

namespace isrran {

class channel
{
public:
  struct args_t {
    // General
    bool enable = false;

    // AWGN options
    bool  awgn_enable            = false;
    float awgn_signal_power_dBfs = 0.0f;
    float awgn_snr_dB            = 30.0f;

    // Fading options
    bool        fading_enable = false;
    std::string fading_model  = "none";

    // High Speed Train options
    bool  hst_enable      = false;
    float hst_fd_hz       = 750.0f;
    float hst_period_s    = 7.2f;
    float hst_init_time_s = 0.0f;

    // Delay options
    bool  delay_enable      = false;
    float delay_min_us      = 10;
    float delay_max_us      = 100;
    float delay_period_s    = 3600;
    float delay_init_time_s = 0;

    // RLF options
    bool     rlf_enable   = false;
    uint32_t rlf_t_on_ms  = 10000;
    uint32_t rlf_t_off_ms = 2000;
  };

  channel(const args_t& channel_args, uint32_t _nof_channels, isrlog::basic_logger& logger);
  ~channel();
  void set_srate(uint32_t srate);
  void set_signal_power_dBfs(float power_dBfs);
  void run(cf_t* in[ISRRAN_MAX_CHANNELS], cf_t* out[ISRRAN_MAX_CHANNELS], uint32_t len, const isrran_timestamp_t& t);

private:
  isrlog::basic_logger&    logger;
  float                    hst_init_phase              = 0.0f;
  isrran_channel_fading_t* fading[ISRRAN_MAX_CHANNELS] = {};
  isrran_channel_delay_t*  delay[ISRRAN_MAX_CHANNELS]  = {};
  isrran_channel_awgn_t*   awgn                        = nullptr;
  isrran_channel_hst_t*    hst                         = nullptr;
  isrran_channel_rlf_t*    rlf                         = nullptr;
  cf_t*                    buffer_in                   = nullptr;
  cf_t*                    buffer_out                  = nullptr;
  uint32_t                 nof_channels                = 0;
  uint32_t                 current_srate               = 0;
  args_t                   args                        = {};
};

typedef std::unique_ptr<channel> channel_ptr;

} // namespace isrran

#endif // ISRRAN_CHANNEL_H
