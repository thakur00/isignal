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
 *  File:         ue_cell_search.h
 *
 *  Description:  Wrapper for the ue_sync object.
 *
 *                This object is a wrapper to the ue_sync object. It receives
 *                several synchronized frames and obtains the most common cell_id
 *                and cp length.
 *
 *                The I/O stream device sampling frequency must be set to 1.92 MHz
 *                (ISRRAN_CS_SAMP_FREQ constant) before calling to
 *                isrran_ue_cellsearch_scan() functions.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_UE_CELL_SEARCH_H
#define ISRRAN_UE_CELL_SEARCH_H

#include <stdbool.h>

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/pbch.h"
#include "isrran/phy/sync/cfo.h"
#include "isrran/phy/ue/ue_mib.h"
#include "isrran/phy/ue/ue_sync.h"

#define ISRRAN_CS_NOF_PRB      6
#define ISRRAN_CS_SAMP_FREQ    1920000.0

typedef struct ISRRAN_API {
  uint32_t cell_id;
  isrran_cp_t         cp;
  isrran_frame_type_t frame_type;
  float peak; 
  float mode; 
  float psr;
  float               cfo;
} isrran_ue_cellsearch_result_t;

typedef struct ISRRAN_API {
  isrran_ue_sync_t ue_sync;

  cf_t*    sf_buffer[ISRRAN_MAX_CHANNELS];
  uint32_t nof_rx_antennas;

  uint32_t max_frames;
  uint32_t nof_valid_frames;  // number of 5 ms frames to scan 
    
  uint32_t *mode_ntimes;
  uint8_t*  mode_counted;

  isrran_ue_cellsearch_result_t* candidates;
} isrran_ue_cellsearch_t;

ISRRAN_API int isrran_ue_cellsearch_init(isrran_ue_cellsearch_t* q,
                                         uint32_t                max_frames_total,
                                         int(recv_callback)(void*, void*, uint32_t, isrran_timestamp_t*),
                                         void* stream_handler);

ISRRAN_API int
isrran_ue_cellsearch_init_multi(isrran_ue_cellsearch_t* q,
                                uint32_t                max_frames_total,
                                int(recv_callback)(void*, cf_t* [ISRRAN_MAX_CHANNELS], uint32_t, isrran_timestamp_t*),
                                uint32_t nof_rx_antennas,
                                void*    stream_handler);

ISRRAN_API void isrran_ue_cellsearch_free(isrran_ue_cellsearch_t* q);

ISRRAN_API int
isrran_ue_cellsearch_scan_N_id_2(isrran_ue_cellsearch_t* q, uint32_t N_id_2, isrran_ue_cellsearch_result_t* found_cell);

ISRRAN_API int isrran_ue_cellsearch_scan(isrran_ue_cellsearch_t*       q,
                                         isrran_ue_cellsearch_result_t found_cells[3],
                                         uint32_t*                     max_N_id_2);

ISRRAN_API int isrran_ue_cellsearch_set_nof_valid_frames(isrran_ue_cellsearch_t* q, uint32_t nof_frames);

ISRRAN_API void isrran_set_detect_cp(isrran_ue_cellsearch_t* q, bool enable);

#endif // ISRRAN_UE_CELL_SEARCH_H

