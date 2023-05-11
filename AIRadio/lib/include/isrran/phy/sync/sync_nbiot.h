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

/******************************************************************************
 *  File:         sync_nbiot.h
 *
 *  Description:  Time and frequency synchronization using the NPSS and NSSS signals.
 *
 *                The object is designed to work with signals sampled at 1.92 Mhz
 *                centered at the carrier frequency. Thus, downsampling is required
 *                if the signal is sampled at higher frequencies.
 *
 *                Correlation peak is detected comparing the maximum at the output
 *                of the correlator with a threshold.
 *
 *  Reference:    3GPP TS 36.211 version 13.2.0 Release 13
 *****************************************************************************/

#ifndef ISRRAN_SYNC_NBIOT_H
#define ISRRAN_SYNC_NBIOT_H

#include <math.h>
#include <stdbool.h>

#include "isrran/config.h"
#include "isrran/phy/sync/npss.h"
#include "isrran/phy/sync/nsss.h"
#include "isrran/phy/sync/sync.h"
#include "isrran/phy/ue/ue_sync.h"

#define MAX_NUM_CFO_CANDITATES 50

typedef struct ISRRAN_API {
  isrran_npss_synch_t npss;
  isrran_nsss_synch_t nsss;
  isrran_cp_synch_t   cp_synch;
  uint32_t            n_id_ncell;

  float        threshold;
  float        peak_value;
  uint32_t     fft_size;
  uint32_t     frame_size;
  uint32_t     max_frame_size;
  uint32_t     max_offset;
  bool         enable_cfo_estimation;
  bool         enable_cfo_cand_test;
  float        cfo_cand[MAX_NUM_CFO_CANDITATES];
  int          cfo_num_cand;
  int          cfo_cand_idx;
  float        mean_cfo;
  float        current_cfo_tol;
  cf_t*        shift_buffer;
  cf_t*        cfo_output;
  int          cfo_i;
  bool         find_cfo_i;
  bool         find_cfo_i_initiated;
  float        cfo_ema_alpha;
  uint32_t     nof_symbols;
  uint32_t     cp_len;
  isrran_cfo_t cfocorr;
  isrran_cp_t  cp;
} isrran_sync_nbiot_t;

ISRRAN_API int
isrran_sync_nbiot_init(isrran_sync_nbiot_t* q, uint32_t frame_size, uint32_t max_offset, uint32_t fft_size);

ISRRAN_API void isrran_sync_nbiot_free(isrran_sync_nbiot_t* q);

ISRRAN_API int
isrran_sync_nbiot_resize(isrran_sync_nbiot_t* q, uint32_t frame_size, uint32_t max_offset, uint32_t fft_size);

ISRRAN_API isrran_sync_find_ret_t isrran_sync_nbiot_find(isrran_sync_nbiot_t* q,
                                                         cf_t*                input,
                                                         uint32_t             find_offset,
                                                         uint32_t*            peak_position);

ISRRAN_API float cfo_estimate_nbiot(isrran_sync_nbiot_t* q, cf_t* input);

ISRRAN_API void isrran_sync_nbiot_set_threshold(isrran_sync_nbiot_t* q, float threshold);

ISRRAN_API void isrran_sync_nbiot_set_cfo_enable(isrran_sync_nbiot_t* q, bool enable);

ISRRAN_API void isrran_sync_nbiot_set_cfo_cand_test_enable(isrran_sync_nbiot_t* q, bool enable);

ISRRAN_API int isrran_sync_nbiot_set_cfo_cand(isrran_sync_nbiot_t* q, const float* cand, const int num);

ISRRAN_API void isrran_sync_nbiot_set_cfo_tol(isrran_sync_nbiot_t* q, float tol);

ISRRAN_API void isrran_sync_nbiot_set_cfo_ema_alpha(isrran_sync_nbiot_t* q, float alpha);

ISRRAN_API void isrran_sync_nbiot_set_npss_ema_alpha(isrran_sync_nbiot_t* q, float alpha);

ISRRAN_API int isrran_sync_nbiot_find_cell_id(isrran_sync_nbiot_t* q, cf_t* input);

ISRRAN_API int isrran_sync_nbiot_get_cell_id(isrran_sync_nbiot_t* q);

ISRRAN_API float isrran_sync_nbiot_get_cfo(isrran_sync_nbiot_t* q);

ISRRAN_API void isrran_sync_nbiot_set_cfo(isrran_sync_nbiot_t* q, float cfo);

ISRRAN_API bool isrran_sync_nbiot_nsss_detected(isrran_sync_nbiot_t* q);

ISRRAN_API float isrran_sync_nbiot_get_peak_value(isrran_sync_nbiot_t* q);

ISRRAN_API void isrran_sync_nbiot_reset(isrran_sync_nbiot_t* q);

#endif // ISRRAN_SYNC_NBIOT_H
