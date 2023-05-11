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

#ifndef ISRRAN_RF_FILE_IMP_H
#define ISRRAN_RF_FILE_IMP_H

#include <inttypes.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/rf/rf.h"

#define DEVNAME_FILE "file"
#define PARAM_LEN (128)
#define PARAM_LEN_SHORT (PARAM_LEN / 2)

extern rf_dev_t isrran_rf_dev_file;

ISRRAN_API const char* rf_file_devname(void* h);

ISRRAN_API int rf_file_start_rx_stream(void* h, bool now);

// ISRRAN_API int rf_file_start_rx_stream_nsamples(void* h, uint32_t nsamples);

ISRRAN_API int rf_file_stop_rx_stream(void* h);

ISRRAN_API void rf_file_flush_buffer(void* h);

ISRRAN_API bool rf_file_has_rssi(void* h);

ISRRAN_API float rf_file_get_rssi(void* h);

ISRRAN_API void rf_file_suppress_stdout(void* h);

ISRRAN_API void rf_file_register_error_handler(void* h, isrran_rf_error_handler_t error_handler, void* arg);

/**
 * @brief This function is not supported for file-based RF abstraction
 *
 * Use @c rf_file_open_file() to open this device
 *
 * @param args not used
 * @param h not used
 * @return ISRRAN_ERROR_INVALID_COMMAND
 */
ISRRAN_API int rf_file_open(char* args, void** h);

/**
 * @brief This function is not supported for file-based RF abstraction
 *
 * Use @c rf_file_open_file() to open this device
 *
 * @param args not used
 * @param h not used
 * @param nof_channels not used
 * @return ISRRAN_ERROR_INVALID_COMMAND
 */
ISRRAN_API int rf_file_open_multi(char* args, void** h, uint32_t nof_channels);

ISRRAN_API int rf_file_close(void* h);

ISRRAN_API double rf_file_set_rx_srate(void* h, double srate);

ISRRAN_API int rf_file_set_rx_gain(void* h, double gain);

ISRRAN_API int rf_file_set_rx_gain_ch(void* h, uint32_t ch, double gain);

ISRRAN_API int rf_file_set_tx_gain(void* h, double gain);

ISRRAN_API int rf_file_set_tx_gain_ch(void* h, uint32_t ch, double gain);

ISRRAN_API double rf_file_get_rx_gain(void* h);

ISRRAN_API double rf_file_get_tx_gain(void* h);

ISRRAN_API isrran_rf_info_t* rf_file_get_info(void* h);

ISRRAN_API double rf_file_set_rx_freq(void* h, uint32_t ch, double freq);

ISRRAN_API double rf_file_set_tx_srate(void* h, double srate);

ISRRAN_API double rf_file_set_tx_freq(void* h, uint32_t ch, double freq);

ISRRAN_API void rf_file_get_time(void* h, time_t* secs, double* frac_secs);

// isrran_rf_sync_pps

ISRRAN_API int
rf_file_recv_with_time(void* h, void* data, uint32_t nsamples, bool blocking, time_t* secs, double* frac_secs);

ISRRAN_API int
rf_file_recv_with_time_multi(void* h, void** data, uint32_t nsamples, bool blocking, time_t* secs, double* frac_secs);

ISRRAN_API int rf_file_send_timed(void*  h,
                                  void*  data,
                                  int    nsamples,
                                  time_t secs,
                                  double frac_secs,
                                  bool   has_time_spec,
                                  bool   blocking,
                                  bool   is_start_of_burst,
                                  bool   is_end_of_burst);

ISRRAN_API int rf_file_send_timed_multi(void*  h,
                                        void*  data[4],
                                        int    nsamples,
                                        time_t secs,
                                        double frac_secs,
                                        bool   has_time_spec,
                                        bool   blocking,
                                        bool   is_start_of_burst,
                                        bool   is_end_of_burst);

/**
 * @brief Dedicated function to open a file-based RF abstraction
 * @param[out] h Resulting object handle
 * @param[in] rx_files List of pre-opened FILE* for each RX channel; NULL to disable
 * @param[in] tx_files List of pre-opened FILE* for each TX channel; NULL to disable
 * @param[in] nof_channels Number of channels per direction
 * @param[in] base_srate Sample rate of RX and TX files
 * @return ISRRAN_SUCCESS on success, otherwise error code
 */
ISRRAN_API int
rf_file_open_file(void** h, FILE** rx_files, FILE** tx_files, uint32_t nof_channels, uint32_t base_srate);

#endif // ISRRAN_RF_FILE_IMP_H
