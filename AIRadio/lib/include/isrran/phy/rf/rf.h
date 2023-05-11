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

#ifndef ISRRAN_RF_H
#define ISRRAN_RF_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "isrran/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RF_PARAM_LEN (256)

typedef struct {
  void* handler;
  void* dev;

  // The following variables are for threaded RX gain control
  bool            thread_gain_run;
  pthread_t       thread_gain;
  pthread_cond_t  cond;
  pthread_mutex_t mutex;
  double          cur_rx_gain;
  double          new_rx_gain;
  bool            tx_gain_same_rx;
  float           tx_rx_gain_offset;
} isrran_rf_t;

typedef struct {
  double min_tx_gain;
  double max_tx_gain;
  double min_rx_gain;
  double max_rx_gain;
} isrran_rf_info_t;

typedef struct {
  enum {
    ISRRAN_RF_ERROR_LATE,
    ISRRAN_RF_ERROR_UNDERFLOW,
    ISRRAN_RF_ERROR_OVERFLOW,
    ISRRAN_RF_ERROR_RX,
    ISRRAN_RF_ERROR_OTHER
  } type;
  int         opt;
  const char* msg;
} isrran_rf_error_t;

typedef void (*isrran_rf_error_handler_t)(void* arg, isrran_rf_error_t error);

/* RF frontend API */
typedef struct {
  const char* name;
  const char* (*isrran_rf_devname)(void* h);
  int (*isrran_rf_start_rx_stream)(void* h, bool now);
  int (*isrran_rf_stop_rx_stream)(void* h);
  void (*isrran_rf_flush_buffer)(void* h);
  bool (*isrran_rf_has_rssi)(void* h);
  float (*isrran_rf_get_rssi)(void* h);
  void (*isrran_rf_suppress_stdout)(void* h);
  void (*isrran_rf_register_error_handler)(void* h, isrran_rf_error_handler_t error_handler, void* arg);
  int (*isrran_rf_open)(char* args, void** h);
  int (*isrran_rf_open_multi)(char* args, void** h, uint32_t nof_channels);
  int (*isrran_rf_close)(void* h);
  double (*isrran_rf_set_rx_srate)(void* h, double freq);
  int (*isrran_rf_set_rx_gain)(void* h, double gain);
  int (*isrran_rf_set_rx_gain_ch)(void* h, uint32_t ch, double gain);
  int (*isrran_rf_set_tx_gain)(void* h, double gain);
  int (*isrran_rf_set_tx_gain_ch)(void* h, uint32_t ch, double gain);
  double (*isrran_rf_get_rx_gain)(void* h);
  double (*isrran_rf_get_tx_gain)(void* h);
  isrran_rf_info_t* (*isrran_rf_get_info)(void* h);
  double (*isrran_rf_set_rx_freq)(void* h, uint32_t ch, double freq);
  double (*isrran_rf_set_tx_srate)(void* h, double freq);
  double (*isrran_rf_set_tx_freq)(void* h, uint32_t ch, double freq);
  void (*isrran_rf_get_time)(void* h, time_t* secs, double* frac_secs);
  void (*isrran_rf_sync_pps)(void* h);
  int (*isrran_rf_recv_with_time)(void*    h,
                                  void*    data,
                                  uint32_t nsamples,
                                  bool     blocking,
                                  time_t*  secs,
                                  double*  frac_secs);
  int (*isrran_rf_recv_with_time_multi)(void*    h,
                                        void**   data,
                                        uint32_t nsamples,
                                        bool     blocking,
                                        time_t*  secs,
                                        double*  frac_secs);
  int (*isrran_rf_send_timed)(void*  h,
                              void*  data,
                              int    nsamples,
                              time_t secs,
                              double frac_secs,
                              bool   has_time_spec,
                              bool   blocking,
                              bool   is_start_of_burst,
                              bool   is_end_of_burst);
  int (*isrran_rf_send_timed_multi)(void*  h,
                                    void** data,
                                    int    nsamples,
                                    time_t secs,
                                    double frac_secs,
                                    bool   has_time_spec,
                                    bool   blocking,
                                    bool   is_start_of_burst,
                                    bool   is_end_of_burst);
} rf_dev_t;

typedef struct {
  const char* plugin_name;
  void*       dl_handle;
  rf_dev_t*   rf_api;
} isrran_rf_plugin_t;

ISRRAN_API int isrran_rf_load_plugins();

ISRRAN_API int isrran_rf_open(isrran_rf_t* h, char* args);

ISRRAN_API int isrran_rf_open_multi(isrran_rf_t* h, char* args, uint32_t nof_channels);

ISRRAN_API int isrran_rf_open_devname(isrran_rf_t* h, const char* devname, char* args, uint32_t nof_channels);

/**
 * @brief Opens a file-based RF abstraction
 * @param[out] rf Device handle
 * @param[in] rx_files List of pre-opened FILE* for each RX channel; NULL to disable
 * @param[in] tx_files List of pre-opened FILE* for each TX channel; NULL to disable
 * @param[in] nof_channels Number of channels per direction
 * @param[in] base_srate Sample rate of RX and TX files
 * @return ISRRAN_SUCCESS on success, otherwise error code
 */
ISRRAN_API int
isrran_rf_open_file(isrran_rf_t* rf, FILE** rx_files, FILE** tx_files, uint32_t nof_channels, uint32_t base_srate);

ISRRAN_API const char* isrran_rf_name(isrran_rf_t* h);

ISRRAN_API int isrran_rf_start_gain_thread(isrran_rf_t* rf, bool tx_gain_same_rx);

ISRRAN_API int isrran_rf_close(isrran_rf_t* h);

ISRRAN_API int isrran_rf_start_rx_stream(isrran_rf_t* h, bool now);

ISRRAN_API int isrran_rf_stop_rx_stream(isrran_rf_t* h);

ISRRAN_API void isrran_rf_flush_buffer(isrran_rf_t* h);

ISRRAN_API bool isrran_rf_has_rssi(isrran_rf_t* h);

ISRRAN_API float isrran_rf_get_rssi(isrran_rf_t* h);

ISRRAN_API double isrran_rf_set_rx_srate(isrran_rf_t* h, double freq);

ISRRAN_API int isrran_rf_set_rx_gain(isrran_rf_t* h, double gain);

ISRRAN_API int isrran_rf_set_rx_gain_ch(isrran_rf_t* h, uint32_t ch, double gain);

ISRRAN_API void isrran_rf_set_tx_rx_gain_offset(isrran_rf_t* h, double offset);

ISRRAN_API int isrran_rf_set_rx_gain_th(isrran_rf_t* h, double gain);

ISRRAN_API double isrran_rf_get_rx_gain(isrran_rf_t* h);

ISRRAN_API double isrran_rf_get_tx_gain(isrran_rf_t* h);

ISRRAN_API isrran_rf_info_t* isrran_rf_get_info(isrran_rf_t* h);

ISRRAN_API void isrran_rf_suppress_stdout(isrran_rf_t* h);

ISRRAN_API void isrran_rf_register_error_handler(isrran_rf_t* h, isrran_rf_error_handler_t error_handler, void* arg);

ISRRAN_API double isrran_rf_set_rx_freq(isrran_rf_t* h, uint32_t ch, double freq);

ISRRAN_API int isrran_rf_recv(isrran_rf_t* h, void* data, uint32_t nsamples, bool blocking);

ISRRAN_API int
isrran_rf_recv_with_time(isrran_rf_t* h, void* data, uint32_t nsamples, bool blocking, time_t* secs, double* frac_secs);

ISRRAN_API int isrran_rf_recv_with_time_multi(isrran_rf_t* h,
                                              void**       data,
                                              uint32_t     nsamples,
                                              bool         blocking,
                                              time_t*      secs,
                                              double*      frac_secs);

ISRRAN_API double isrran_rf_set_tx_srate(isrran_rf_t* h, double freq);

ISRRAN_API int isrran_rf_set_tx_gain(isrran_rf_t* h, double gain);

ISRRAN_API int isrran_rf_set_tx_gain_ch(isrran_rf_t* h, uint32_t ch, double gain);

ISRRAN_API double isrran_rf_set_tx_freq(isrran_rf_t* h, uint32_t ch, double freq);

ISRRAN_API void isrran_rf_get_time(isrran_rf_t* h, time_t* secs, double* frac_secs);

ISRRAN_API int isrran_rf_sync(isrran_rf_t* rf);

ISRRAN_API int isrran_rf_send(isrran_rf_t* h, void* data, uint32_t nsamples, bool blocking);

ISRRAN_API int
isrran_rf_send2(isrran_rf_t* h, void* data, uint32_t nsamples, bool blocking, bool start_of_burst, bool end_of_burst);

ISRRAN_API int isrran_rf_send(isrran_rf_t* h, void* data, uint32_t nsamples, bool blocking);

ISRRAN_API int isrran_rf_send_timed(isrran_rf_t* h, void* data, int nsamples, time_t secs, double frac_secs);

ISRRAN_API int isrran_rf_send_timed2(isrran_rf_t* h,
                                     void*        data,
                                     int          nsamples,
                                     time_t       secs,
                                     double       frac_secs,
                                     bool         is_start_of_burst,
                                     bool         is_end_of_burst);

ISRRAN_API int isrran_rf_send_timed_multi(isrran_rf_t* rf,
                                          void**       data,
                                          int          nsamples,
                                          time_t       secs,
                                          double       frac_secs,
                                          bool         blocking,
                                          bool         is_start_of_burst,
                                          bool         is_end_of_burst);

ISRRAN_API int isrran_rf_send_multi(isrran_rf_t* rf,
                                    void**       data,
                                    int          nsamples,
                                    bool         blocking,
                                    bool         is_start_of_burst,
                                    bool         is_end_of_burst);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_RF_H
