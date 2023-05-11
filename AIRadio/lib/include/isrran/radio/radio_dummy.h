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

#include "channel_mapping.h"
#include "radio_metrics.h"
#include "rf_buffer.h"
#include "rf_timestamp.h"
#include "isrran/common/interfaces_common.h"
#include "isrran/interfaces/radio_interfaces.h"
#include "isrran/phy/resampling/resampler.h"
#include "isrran/phy/rf/rf.h"
#include "isrran/radio/radio_base.h"
#include "isrran/isrlog/isrlog.h"
#include "isrran/isrran.h"

#include <condition_variable>
#include <list>
#include <string>

#ifndef ISRRAN_RADIO_DUMMY_H
#define ISRRAN_RADIO_DUMMY_H

namespace isrran {

/**
 * Implementation of radio dummy for the PHY testing
 *
 * It uses ringbuffers from isrRAN library to emulate baseband transmission and reception. The current implementation
 * does not support dynamic sampling rates, gains and frequencies.
 */
class radio_dummy : public isrran::radio_base, public isrran::radio_interface_phy
{
private:
  static const uint32_t            TEMP_BUFFER_SZ = ISRRAN_SF_LEN_MAX * ISRRAN_NOF_SF_X_FRAME;
  isrlog::basic_logger&            logger;
  std::vector<isrran_ringbuffer_t> rx_ring_buffers;
  std::vector<isrran_ringbuffer_t> tx_ring_buffers;
  std::mutex                       tx_mutex;
  std::atomic<double>              srate_hz       = {0.0f};
  std::atomic<float>               rx_gain        = {1.0f};
  std::atomic<float>               tx_gain        = {1.0f};
  cf_t*                            temp_buffer    = nullptr;
  uint64_t                         rx_timestamp   = 0;
  uint64_t                         tx_timestamp   = 0;
  isrran_rf_info_t                 rf_info        = {};
  std::atomic<bool>                is_initialised = {false};
  std::atomic<bool>                quit           = {false};

  void write_ring_buffers(std::vector<isrran_ringbuffer_t>& buffers, cf_t** buffer, uint32_t nsamples)
  {
    for (uint32_t i = 0; i < buffers.size(); i++) {
      int ret = ISRRAN_SUCCESS;
      do {
        if (ret != ISRRAN_SUCCESS) {
          logger.error("Ring buffer write failed (full). Trying again.");
        }
        ret = isrran_ringbuffer_write_timed(&buffers[i], buffer[i], (int)(sizeof(cf_t) * nsamples), 1000);
      } while (ret == ISRRAN_ERROR_TIMEOUT and not quit);
    }
  }

  void read_ring_buffers(std::vector<isrran_ringbuffer_t>& buffers, cf_t** buffer, uint32_t nsamples)
  {
    for (uint32_t i = 0; i < buffers.size(); i++) {
      int ret = ISRRAN_SUCCESS;
      do {
        if (ret != ISRRAN_SUCCESS) {
          logger.error("Ring buffer read failed. Trying again.");
        }
        ret = isrran_ringbuffer_read_timed(&buffers[i], buffer[i], (int)(sizeof(cf_t) * nsamples), 1000);
      } while (ret == ISRRAN_ERROR_TIMEOUT and not quit);
    }
  }

  void write_zeros_ring_buffers(std::vector<isrran_ringbuffer_t>& buffers, uint32_t nsamples)
  {
    uint32_t n = ISRRAN_MIN(nsamples, TEMP_BUFFER_SZ);
    isrran_vec_cf_zero(temp_buffer, n);

    std::array<cf_t*, ISRRAN_MAX_CHANNELS> zero_buffer_pointers = {};
    for (cf_t*& ptr : zero_buffer_pointers) {
      ptr = temp_buffer;
    }

    while (nsamples > 0) {
      // Get new number of samples
      n = ISRRAN_MIN(nsamples, TEMP_BUFFER_SZ);

      // Write zeros in the buffers
      write_ring_buffers(buffers, zero_buffer_pointers.data(), n);

      nsamples -= n;
    }
  }

  void advance_tx_timestamp(uint64_t ts, bool round_sf = false)
  {
    std::lock_guard<std::mutex> lock(tx_mutex);

    // Make sure new timestamp has not passed
    if (ts < tx_timestamp) {
      return;
    }

    // Calculate transmission gap
    uint32_t tx_gap = (uint32_t)(ts - tx_timestamp);

    // Round gap to subframe size
    if (round_sf) {
      uint64_t sf_sz = (uint64_t)(srate_hz / 1e3);
      tx_gap         = sf_sz * ISRRAN_CEIL(tx_gap, sf_sz);
    }

    // Skip zeros if there is no gap
    if (tx_gap == 0) {
      return;
    }

    // Write zeros in tx ring buffer
    write_zeros_ring_buffers(tx_ring_buffers, tx_gap);

    // Update new transmit timestamp
    tx_timestamp += tx_gap;
  }

public:
  radio_dummy() : logger(isrlog::fetch_basic_logger("RF", false)) {}

  ~radio_dummy()
  {
    for (auto& rb : rx_ring_buffers) {
      isrran_ringbuffer_free(&rb);
    }
    for (auto& rb : tx_ring_buffers) {
      isrran_ringbuffer_free(&rb);
    }
    if (temp_buffer) {
      free(temp_buffer);
    }
  }

  std::string get_type() override { return "dummy"; }
  int         init(const rf_args_t& args_, phy_interface_radio* phy_) override
  {
    // Set logger level
    logger.set_level(isrlog::str_to_basic_level(args_.log_level));

    // Get base sampling rate and assert the value is valid
    srate_hz = args_.srate_hz;
    if (not std::isnormal(srate_hz)) {
      logger.error("A valid sampling rate is missing");
      return ISRRAN_ERROR;
    }

    // Create receiver ring buffers
    rx_ring_buffers.resize(args_.nof_carriers * (size_t)args_.nof_antennas);
    for (auto& rb : rx_ring_buffers) {
      if (isrran_ringbuffer_init(&rb, (int)sizeof(cf_t) * TEMP_BUFFER_SZ) != ISRRAN_SUCCESS) {
        perror("init softbuffer");
      }
    }

    // Create transmitter ring buffers
    tx_ring_buffers.resize(args_.nof_carriers * (size_t)args_.nof_antennas);
    for (auto& rb : tx_ring_buffers) {
      if (isrran_ringbuffer_init(&rb, (int)sizeof(cf_t) * TEMP_BUFFER_SZ) != ISRRAN_SUCCESS) {
        perror("init softbuffer");
      }
    }

    // Create temporal buffer
    temp_buffer = isrran_vec_cf_malloc(TEMP_BUFFER_SZ);
    if (!temp_buffer) {
      perror("malloc");
    }

    // Set RF Info (in dB)
    rf_info.min_rx_gain = 0.0f;
    rf_info.max_rx_gain = 90.0f;
    rf_info.min_tx_gain = 0.0f;
    rf_info.max_tx_gain = 90.0f;

    // Finally, the radio is initialised
    is_initialised = true;

    return ISRRAN_SUCCESS;
  }
  void stop() override { quit = true; }
  bool get_metrics(rf_metrics_t* metrics) override { return false; }

  void set_loglevel(std::string& str) { logger.set_level(isrlog::str_to_basic_level(str)); }

  void write_rx(cf_t** buffer, uint32_t nsamples) { write_ring_buffers(rx_ring_buffers, buffer, nsamples); }

  void read_tx(cf_t** buffer, uint32_t nsamples) { read_ring_buffers(tx_ring_buffers, buffer, nsamples); }

  bool tx(isrran::rf_buffer_interface& buffer, const isrran::rf_timestamp_interface& tx_time) override
  {
    bool ret = true;

    // Convert timestamp to samples
    uint64_t tx_time_n = isrran_timestamp_uint64(&tx_time.get(0), srate_hz);

    // Check if the transmission is in the past
    {
      std::lock_guard<std::mutex> lock(tx_mutex);
      if (tx_time_n < tx_timestamp) {
        logger.error("Error transmission in the past for %d samples", (int)(tx_timestamp - tx_time_n));
        return false;
      }
    }

    // Advance TX to timestamp
    advance_tx_timestamp(tx_time_n);

    // From now on, protect buffers
    std::lock_guard<std::mutex> lock(tx_mutex);

    // Write transmission buffers into the ring buffer
    write_ring_buffers(tx_ring_buffers, buffer.to_cf_t(), buffer.get_nof_samples());

    // Increment transmit timestamp
    tx_timestamp += buffer.get_nof_samples();

    return ret;
  }
  void release_freq(const uint32_t& carrier_idx) override{};
  void tx_end() override {}
  bool rx_now(isrran::rf_buffer_interface& buffer, isrran::rf_timestamp_interface& rxd_time) override
  {
    // Advance Tx buffer
    advance_tx_timestamp(rx_timestamp + buffer.get_nof_samples(), true);

    // Read samples
    read_ring_buffers(rx_ring_buffers, buffer.to_cf_t(), buffer.get_nof_samples());

    // Apply Rx gain
    for (uint32_t i = 0; i < rx_ring_buffers.size(); i++) {
      cf_t* ptr = buffer.get(i);
      isrran_vec_sc_prod_cfc(ptr, rx_gain, ptr, buffer.get_nof_samples());
    }

    // Set Rx timestamp
    isrran_timestamp_init_uint64(rxd_time.get_ptr(0), rx_timestamp, (double)srate_hz);

    // Advance timestamp
    rx_timestamp += buffer.get_nof_samples();

    return true;
  }
  void set_tx_freq(const uint32_t& channel_idx, const double& freq) override
  {
    logger.info("Set Tx freq to %+.0f MHz.", freq * 1.0e-6);
  }
  void set_rx_freq(const uint32_t& channel_idx, const double& freq) override
  {
    logger.info("Set Rx freq to %+.0f MHz.", freq * 1.0e-6);
  }
  void set_rx_gain_th(const float& gain) override
  {
    rx_gain = isrran_convert_dB_to_amplitude(gain);
    logger.info("Set Rx gain-th to %+.1f dB (%.6f).", gain, rx_gain.load());
  }
  void set_tx_gain(const float& gain) override
  {
    tx_gain = isrran_convert_dB_to_amplitude(gain);
    logger.info("Set Tx gain to %+.1f dB (%.6f).", gain, tx_gain.load());
  }
  void set_rx_gain(const float& gain) override
  {
    rx_gain = isrran_convert_dB_to_amplitude(gain);
    logger.info("Set Rx gain to %+.1f dB (%.6f).", gain, rx_gain.load());
  }
  void set_tx_srate(const double& srate) override { logger.info("Set Tx sampling rate to %+.3f MHz.", srate * 1.0e-6); }
  void set_rx_srate(const double& srate) override { logger.info("Set Rx sampling rate to %+.3f MHz.", srate * 1.0e-6); }
  void set_channel_rx_offset(uint32_t ch, int32_t offset_samples) override{};
  float             get_rx_gain() override { return isrran_convert_amplitude_to_dB(rx_gain); }
  double            get_freq_offset() override { return 0; }
  bool              is_continuous_tx() override { return false; }
  bool              get_is_start_of_burst() override { return false; }
  bool              is_init() override { return is_initialised; }
  void              reset() override {}
  isrran_rf_info_t* get_info() override { return &rf_info; }
};

} // namespace isrran

#endif // ISRRAN_RADIO_DUMMY_H
