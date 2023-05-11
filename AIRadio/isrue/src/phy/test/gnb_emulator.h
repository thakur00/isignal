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

#ifndef ISRRAN_GNB_EMULATOR_H
#define ISRRAN_GNB_EMULATOR_H

#include <isrran/phy/channel/channel.h>
#include <isrran/radio/rf_timestamp.h>
#include <isrran/isrran.h>
#include <isrran/support/isrran_assert.h>

class gnb_emulator
{
private:
  uint32_t              sf_len  = 0;
  isrran_carrier_nr_t   carrier = {};
  isrran_ssb_t          ssb     = {};
  isrran::channel       channel;
  std::vector<cf_t>     buffer;
  isrlog::basic_logger& logger = isrlog::fetch_basic_logger("GNB-EMULATOR");

public:
  struct args_t {
    double                      srate_hz;
    isrran_carrier_nr_t         carrier;
    isrran_subcarrier_spacing_t ssb_scs;
    isrran_ssb_pattern_t        ssb_pattern;
    uint32_t                    ssb_periodicity_ms;
    isrran_duplex_mode_t        duplex_mode;
    isrran::channel::args_t     channel;
    std::string                 log_level = "warning";
  };

  gnb_emulator(const args_t& args) : channel(args.channel, 1, isrlog::fetch_basic_logger("GNB-EMULATOR"))
  {
    logger.set_level(isrlog::str_to_basic_level(args.log_level));

    isrran_assert(
        std::isnormal(args.srate_hz) and args.srate_hz > 0, "Invalid sampling rate (%.2f MHz)", args.srate_hz);

    // Initialise internals
    sf_len  = args.srate_hz / 1000;
    carrier = args.carrier;
    buffer.resize(sf_len);

    isrran_ssb_args_t ssb_args = {};
    ssb_args.enable_encode     = true;
    isrran_assert(isrran_ssb_init(&ssb, &ssb_args) == ISRRAN_SUCCESS, "SSB initialisation failed");

    isrran_ssb_cfg_t ssb_cfg = {};
    ssb_cfg.srate_hz         = args.srate_hz;
    ssb_cfg.center_freq_hz   = args.carrier.dl_center_frequency_hz;
    ssb_cfg.ssb_freq_hz      = args.carrier.ssb_center_freq_hz;
    ssb_cfg.scs              = args.ssb_scs;
    ssb_cfg.pattern          = args.ssb_pattern;
    ssb_cfg.duplex_mode      = args.duplex_mode;
    ssb_cfg.periodicity_ms   = args.ssb_periodicity_ms;
    isrran_assert(isrran_ssb_set_cfg(&ssb, &ssb_cfg) == ISRRAN_SUCCESS, "SSB set config failed");

    // Configure channel
    channel.set_srate((uint32_t)args.srate_hz);
  }

  int work(uint32_t sf_idx, cf_t* baseband_buffer, const isrran::rf_timestamp_t& ts)
  {
    logger.set_context(sf_idx);

    // Zero buffer
    isrran_vec_cf_zero(buffer.data(), sf_len);

    // Check if SSB needs to be sent
    if (isrran_ssb_send(&ssb, sf_idx)) {
      // Prepare PBCH message
      isrran_pbch_msg_nr_t msg = {};

      // Add SSB
      if (isrran_ssb_add(&ssb, carrier.pci, &msg, buffer.data(), buffer.data()) < ISRRAN_SUCCESS) {
        logger.error("Error adding SSB");
        return ISRRAN_ERROR;
      }
    }

    // Run channel
    cf_t* in[ISRRAN_MAX_CHANNELS]  = {};
    cf_t* out[ISRRAN_MAX_CHANNELS] = {};
    in[0]                          = buffer.data();
    out[0]                         = buffer.data();
    channel.run(in, out, sf_len, ts.get(0));

    // Add buffer to baseband buffer
    isrran_vec_sum_ccc(baseband_buffer, buffer.data(), baseband_buffer, sf_len);

    return ISRRAN_SUCCESS;
  }

  ~gnb_emulator() { isrran_ssb_free(&ssb); }
};

#endif // ISRRAN_GNB_EMULATOR_H
