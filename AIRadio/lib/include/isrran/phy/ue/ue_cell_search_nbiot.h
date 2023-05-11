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

#ifndef ISRRAN_UE_CELL_SEARCH_NBIOT_H
#define ISRRAN_UE_CELL_SEARCH_NBIOT_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl_nbiot.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/npbch.h"
#include "isrran/phy/sync/cfo.h"
#include "isrran/phy/ue/ue_mib_nbiot.h"
#include "isrran/phy/ue/ue_sync_nbiot.h"

#define ISRRAN_CS_NOF_PRB 6
#define ISRRAN_CS_SAMP_FREQ 1920000.0

typedef struct ISRRAN_API {
  uint32_t n_id_ncell;
  float    peak;
  float    mode;
  float    psr;
  float    cfo;
} isrran_nbiot_ue_cellsearch_result_t;

/**
 * \brief Wrapper for the nbiot_ue_sync object.
 *
 * This object is a wrapper to the nbiot_ue_sync object. It receives
 * several synchronized frames and obtains the most common n_id_ncell.
 *
 * The I/O stream device sampling frequency must be set to 1.92 MHz
 * (ISRRAN_CS_SAMP_FREQ constant) before calling to
 * isrran_ue_cellsearch_nbiot_scan() functions.
 */
typedef struct ISRRAN_API {
  isrran_nbiot_ue_sync_t ue_sync;
  int32_t                sf_len;

  cf_t* rx_buffer[ISRRAN_MAX_CHANNELS];
  cf_t* nsss_buffer;
  int   nsss_sf_counter;

  uint32_t max_frames;
  uint32_t nof_valid_frames; // number of frames to scan
} isrran_ue_cellsearch_nbiot_t;

ISRRAN_API int
isrran_ue_cellsearch_nbiot_init(isrran_ue_cellsearch_nbiot_t* q,
                                uint32_t                      max_frames_total,
                                int(recv_callback)(void*, cf_t* [ISRRAN_MAX_CHANNELS], uint32_t, isrran_timestamp_t*),
                                void* stream_handler);

ISRRAN_API void isrran_ue_cellsearch_nbiot_free(isrran_ue_cellsearch_nbiot_t* q);

ISRRAN_API int isrran_ue_cellsearch_nbiot_scan(isrran_ue_cellsearch_nbiot_t* q);

ISRRAN_API int isrran_ue_cellsearch_nbiot_detect(isrran_ue_cellsearch_nbiot_t*        q,
                                                 isrran_nbiot_ue_cellsearch_result_t* found_cells);

ISRRAN_API int isrran_ue_cellsearch_nbiot_set_nof_valid_frames(isrran_ue_cellsearch_nbiot_t* q, uint32_t nof_frames);

#endif // ISRRAN_UE_CELL_SEARCH_NBIOT_H
