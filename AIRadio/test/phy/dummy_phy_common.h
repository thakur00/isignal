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

#ifndef ISRRAN_DUMMY_PHY_COMMON_H
#define ISRRAN_DUMMY_PHY_COMMON_H

#include <isrran/common/tti_sempahore.h>
#include <isrran/interfaces/phy_common_interface.h>
#include <isrran/isrlog/isrlog.h>
#include <isrran/isrran.h>
#include <vector>

class phy_common : public isrran::phy_common_interface
{
private:
  const uint32_t                   RINGBUFFER_TIMEOUT_MS = 10;
  std::atomic<bool>                quit                  = {false};
  isrlog::basic_logger&            logger;
  double                           srate_hz;
  uint64_t                         write_ts = 0;
  uint64_t                         read_ts  = 0;
  std::vector<cf_t>                zero_buffer; ///< Zero buffer for Tx
  std::vector<cf_t>                sink_buffer; ///< Dummy buffer for Rx
  std::mutex                       ringbuffers_mutex;
  std::vector<isrran_ringbuffer_t> ringbuffers;
  isrran::tti_semaphore<void*>     semaphore;

  void write_zero_padding(uint32_t nof_zeros)
  {
    // Skip if no pading is required
    if (nof_zeros == 0) {
      return;
    }

    logger.debug("Padding %d zeros", nof_zeros);

    // For each ringbuffer, padd zero
    int nof_bytes = (int)(nof_zeros * sizeof(cf_t));
    for (isrran_ringbuffer_t& rb : ringbuffers) {
      // If quit is flagged, return instantly
      if (quit) {
        return;
      }

      // Actual write
      int err = ISRRAN_SUCCESS;
      do {
        err = isrran_ringbuffer_write_timed(&rb, zero_buffer.data(), nof_bytes, RINGBUFFER_TIMEOUT_MS);
        if (err < ISRRAN_SUCCESS and err != ISRRAN_ERROR_TIMEOUT) {
          logger.error("Error writing zeros in ringbuffer");
        }
      } while (err < ISRRAN_SUCCESS and not quit);
    }

    // Increment write timestamp
    write_ts += nof_zeros;
  }

  void write_baseband(isrran::rf_buffer_t& buffer)
  {
    // skip if baseband is not available
    if (buffer.get_nof_samples() == 0) {
      return;
    }

    // For each ringbuffer, write
    int      nof_bytes   = (int)(buffer.get_nof_samples() * sizeof(cf_t));
    uint32_t channel_idx = 0;
    for (isrran_ringbuffer_t& rb : ringbuffers) {
      // If quit is flagged, return instantly
      if (quit) {
        return;
      }

      // Extract channel buffer pointer
      cf_t* channel_buffer = buffer.get(channel_idx);

      // If the pointer is not set, use the zero buffer
      if (channel_buffer == nullptr) {
        channel_buffer = zero_buffer.data();
      }

      // Actual write
      int err = ISRRAN_SUCCESS;
      do {
        err = isrran_ringbuffer_write_timed(&rb, channel_buffer, nof_bytes, RINGBUFFER_TIMEOUT_MS);
        if (err < ISRRAN_SUCCESS and err != ISRRAN_ERROR_TIMEOUT) {
          logger.error("Error writing zeros in ringbuffer");
        }
      } while (err < ISRRAN_SUCCESS and not quit);

      // Increment channel counter
      channel_idx++;
    }

    // Increment write timestamp
    write_ts += buffer.get_nof_samples();
  }

  void read_baseband(std::vector<cf_t*>& buffers, uint32_t nof_samples)
  {
    // For each ringbuffer, read
    int      nof_bytes   = (int)(nof_samples * sizeof(cf_t));
    uint32_t channel_idx = 0;
    for (isrran_ringbuffer_t& rb : ringbuffers) {
      // If quit is flagged, return instantly
      if (quit) {
        return;
      }

      // Extract channel buffer pointer
      cf_t* channel_buffer = buffers[channel_idx];

      // If the pointer is not set, use the zero buffer
      if (channel_buffer == nullptr) {
        channel_buffer = sink_buffer.data();
      }

      // Actual write
      int err = ISRRAN_SUCCESS;
      do {
        err = isrran_ringbuffer_read_timed(&rb, channel_buffer, nof_bytes, RINGBUFFER_TIMEOUT_MS);
        if (err < ISRRAN_SUCCESS and err != ISRRAN_ERROR_TIMEOUT) {
          logger.error("Error reading zeros in ringbuffer");
        }
      } while (err < ISRRAN_SUCCESS and not quit);

      // Increment channel counter
      channel_idx++;
    }
  }

public:
  struct args_t {
    double   srate_hz     = 11.52e6;
    uint32_t buffer_sz_ms = 10; ///< Buffer size in milliseconds
    uint32_t nof_channels = 1;

    args_t(double srate_hz_, uint32_t buffer_sz_ms_, uint32_t nof_channels_) :
      srate_hz(srate_hz_), buffer_sz_ms(buffer_sz_ms_), nof_channels(nof_channels_)
    {}
  };

  phy_common(const args_t& args, isrlog::basic_logger& logger_) : srate_hz(args.srate_hz), logger(logger_)
  {
    uint32_t buffer_sz       = std::ceil((double)args.buffer_sz_ms * srate_hz * 1e-3);
    uint32_t buffer_sz_bytes = sizeof(cf_t) * buffer_sz;

    // Allocate data buffer
    zero_buffer.resize(buffer_sz);
    sink_buffer.resize(buffer_sz);

    // Allocate ring buffers
    ringbuffers.resize(args.nof_channels);

    // Initialise buffers
    for (isrran_ringbuffer_t& rb : ringbuffers) {
      if (isrran_ringbuffer_init(&rb, buffer_sz_bytes) < ISRRAN_SUCCESS) {
        logger.error("Error ringbuffer init");
      }
    }
  }

  ~phy_common()
  {
    for (isrran_ringbuffer_t& rb : ringbuffers) {
      isrran_ringbuffer_free(&rb);
    }
  }

  void push_semaphore(void* worker_ptr) { semaphore.push(worker_ptr); }

  void worker_end(const worker_context_t& w_ctx, const bool& tx_enable, isrran::rf_buffer_t& buffer) override
  {
    // Synchronize worker
    semaphore.wait(w_ctx.worker_ptr);

    // Protect internal buffers and states
    std::unique_lock<std::mutex> lock(ringbuffers_mutex);

    uint64_t tx_ts = isrran_timestamp_uint64(&w_ctx.tx_time.get(0), srate_hz);

    // Check transmit timestamp is not in the past
    if (tx_ts < write_ts) {
      logger.error("Tx time (%f) is %d samples in the past",
                   isrran_timestamp_real(&w_ctx.tx_time.get(0)),
                   (uint32_t)(write_ts - tx_ts));
      semaphore.release();
      return;
    }

    // Write zero padding if necessary
    write_zero_padding((uint32_t)(tx_ts - write_ts));

    // Write baseband
    if (tx_enable) {
      write_baseband(buffer);
    } else {
      write_zero_padding(buffer.get_nof_samples());
    }

    // Release semaphore, so next worker can be used
    semaphore.release();
  }

  void read(std::vector<cf_t*>& buffers, uint32_t nof_samples, isrran::rf_timestamp_t& timestamp)
  {
    // Protect internal buffers and states
    std::unique_lock<std::mutex> lock(ringbuffers_mutex);

    // Detect if zero padding is necessary
    if (read_ts + nof_samples > write_ts) {
      uint32_t nof_zero_pading = (uint32_t)((read_ts + nof_samples) - write_ts);
      write_zero_padding(nof_zero_pading);
    }

    // Actual baseband read
    read_baseband(buffers, nof_samples);

    // Write timestamp
    isrran_timestamp_init_uint64(timestamp.get_ptr(0), read_ts, srate_hz);

    // Increment Rx timestamp
    read_ts += nof_samples;
  }

  void stop() { quit = true; }
};

#endif // ISRRAN_DUMMY_PHY_COMMON_H
