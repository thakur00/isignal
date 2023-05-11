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

#ifndef ISRRAN_UE_SYNC_NBIOT_H
#define ISRRAN_UE_SYNC_NBIOT_H

#include <stdbool.h>

#include "isrran/config.h"
#include "isrran/phy/agc/agc.h"
#include "isrran/phy/ch_estimation/chest_dl_nbiot.h"
#include "isrran/phy/common/timestamp.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/io/filesource.h"
#include "isrran/phy/phch/npbch.h"
#include "isrran/phy/sync/cfo.h"
#include "isrran/phy/sync/sync_nbiot.h"

typedef isrran_ue_sync_state_t isrran_nbiot_ue_sync_state_t;

//#define MEASURE_EXEC_TIME

typedef struct ISRRAN_API {
  isrran_sync_nbiot_t sfind;
  isrran_sync_nbiot_t strack;

  isrran_agc_t agc;
  bool         do_agc;
  uint32_t     agc_period;

  void* stream;
  void* stream_single;
  int (*recv_callback)(void*, cf_t* [ISRRAN_MAX_PORTS], uint32_t, isrran_timestamp_t*);
  int (*recv_callback_single)(void*, void*, uint32_t, isrran_timestamp_t*);
  isrran_timestamp_t last_timestamp;

  isrran_filesource_t file_source;
  bool                file_mode;
  float               file_cfo;
  isrran_cfo_t        file_cfo_correct;

  isrran_nbiot_ue_sync_state_t state;

  uint32_t nof_rx_antennas;
  uint32_t frame_len;
  uint32_t fft_size;
  uint32_t nof_recv_sf; // Number of subframes received each call to isrran_ue_sync_get_buffer
  uint32_t nof_avg_find_frames;
  uint32_t frame_find_cnt;
  uint32_t sf_len;

  /* These count half frames (5ms) */
  uint64_t frame_ok_cnt;
  uint32_t frame_no_cnt;
  uint32_t frame_total_cnt;

  /* this is the system frame number (SFN) */
  uint32_t frame_number;

  isrran_nbiot_cell_t cell;
  uint32_t            sf_idx;
  bool                correct_cfo;

  uint32_t peak_idx;
  int      next_rf_sample_offset;
  int      last_sample_offset;
  float    mean_sample_offset;
  float    mean_sfo;
  uint32_t sample_offset_correct_period;
  float    sfo_ema;
  uint32_t max_prb;

#ifdef MEASURE_EXEC_TIME
  float mean_exec_time;
#endif
} isrran_nbiot_ue_sync_t;

ISRRAN_API int isrran_ue_sync_nbiot_init(isrran_nbiot_ue_sync_t* q,
                                         isrran_nbiot_cell_t     cell,
                                         int(recv_callback)(void*, void*, uint32_t, isrran_timestamp_t*),
                                         void* stream_handler);

ISRRAN_API int
isrran_ue_sync_nbiot_init_multi(isrran_nbiot_ue_sync_t* q,
                                uint32_t                max_prb,
                                int(recv_callback)(void*, cf_t* [ISRRAN_MAX_PORTS], uint32_t, isrran_timestamp_t*),
                                uint32_t nof_rx_antennas,
                                void*    stream_handler);

ISRRAN_API int isrran_ue_sync_nbiot_init_file(isrran_nbiot_ue_sync_t* q,
                                              isrran_nbiot_cell_t     cell,
                                              char*                   file_name,
                                              int                     offset_time,
                                              float                   offset_freq);

ISRRAN_API int isrran_ue_sync_nbiot_init_file_multi(isrran_nbiot_ue_sync_t* q,
                                                    isrran_nbiot_cell_t     cell,
                                                    char*                   file_name,
                                                    int                     offset_time,
                                                    float                   offset_freq,
                                                    uint32_t                nof_rx_ant);

ISRRAN_API void isrran_ue_sync_nbiot_free(isrran_nbiot_ue_sync_t* q);

ISRRAN_API int isrran_ue_sync_nbiot_set_cell(isrran_nbiot_ue_sync_t* q, isrran_nbiot_cell_t cell);

ISRRAN_API int isrran_ue_sync_nbiot_start_agc(isrran_nbiot_ue_sync_t* q,
                                              ISRRAN_AGC_CALLBACK(set_gain_callback),
                                              float init_gain_value);

ISRRAN_API uint32_t isrran_ue_sync_nbiot_sf_len(isrran_nbiot_ue_sync_t* q);

ISRRAN_API int isrran_nbiot_ue_sync_get_buffer(isrran_nbiot_ue_sync_t* q, cf_t** sf_symbols);

ISRRAN_API void isrran_ue_sync_nbiot_set_agc_period(isrran_nbiot_ue_sync_t* q, uint32_t period);

/* CAUTION: input_buffer MUST have space for 2 subframes */
ISRRAN_API int isrran_ue_sync_nbiot_zerocopy(isrran_nbiot_ue_sync_t* q, cf_t* input_buffer);

ISRRAN_API int isrran_ue_sync_nbiot_zerocopy_multi(isrran_nbiot_ue_sync_t* q, cf_t** input_buffer);

ISRRAN_API void isrran_ue_sync_nbiot_set_cfo(isrran_nbiot_ue_sync_t* q, float cfo);

ISRRAN_API void isrran_ue_sync_nbiot_reset(isrran_nbiot_ue_sync_t* q);

ISRRAN_API isrran_nbiot_ue_sync_state_t isrran_ue_sync_nbiot_get_state(isrran_nbiot_ue_sync_t* q);

ISRRAN_API uint32_t isrran_ue_sync_nbiot_get_sfidx(isrran_nbiot_ue_sync_t* q);

ISRRAN_API void isrran_ue_sync_nbiot_set_cfo_enable(isrran_nbiot_ue_sync_t* q, bool enable);

ISRRAN_API float isrran_ue_sync_nbiot_get_cfo(isrran_nbiot_ue_sync_t* q);

ISRRAN_API float isrran_ue_sync_nbiot_get_sfo(isrran_nbiot_ue_sync_t* q);

ISRRAN_API void isrran_ue_sync_nbiot_set_cfo_ema(isrran_nbiot_ue_sync_t* q, float ema);

ISRRAN_API void isrran_ue_sync_nbiot_set_cfo_tol(isrran_nbiot_ue_sync_t* q, float cfo_tol);

ISRRAN_API int isrran_ue_sync_nbiot_get_last_sample_offset(isrran_nbiot_ue_sync_t* q);

ISRRAN_API void isrran_ue_sync_nbiot_set_sample_offset_correct_period(isrran_nbiot_ue_sync_t* q,
                                                                      uint32_t                nof_subframes);

ISRRAN_API void isrran_ue_sync_nbiot_get_last_timestamp(isrran_nbiot_ue_sync_t* q, isrran_timestamp_t* timestamp);

#endif // ISRRAN_UE_SYNC_NBIOT_H