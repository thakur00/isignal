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
 *  File:         ue_mib.h
 *
 *  Description:  This object decodes the MIB from the PBCH of an LTE signal.
 *
 *                The function isrran_ue_mib_decode() shall be called multiple
 *                times, each passing a number of samples multiple of 19200,
 *                sampled at 1.92 MHz (that is, 10 ms of samples).
 *
 *                The function uses the sync_t object to find the PSS sequence and
 *                decode the PBCH to obtain the MIB.
 *
 *                The function returns 0 until the MIB is decoded.
 *
 *                See ue_cell_detect.c for an example.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_UE_MIB_H
#define ISRRAN_UE_MIB_H

#include <stdbool.h>

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/pbch.h"
#include "isrran/phy/sync/cfo.h"
#include "isrran/phy/ue/ue_sync.h"

#define ISRRAN_UE_MIB_NOF_PRB              6

#define ISRRAN_UE_MIB_FOUND                1
#define ISRRAN_UE_MIB_NOTFOUND             0

typedef struct ISRRAN_API {
  isrran_sync_t sfind;

  cf_t* sf_symbols;

  isrran_ofdm_t fft;
  isrran_pbch_t pbch;

  isrran_chest_dl_t     chest;
  isrran_chest_dl_res_t chest_res;

  uint8_t  bch_payload[ISRRAN_BCH_PAYLOAD_LEN];
  uint32_t nof_tx_ports; 
  uint32_t sfn_offset;

  uint32_t frame_cnt;
} isrran_ue_mib_t;

ISRRAN_API int isrran_ue_mib_init(isrran_ue_mib_t* q, cf_t* in_buffer, uint32_t max_prb);

ISRRAN_API void isrran_ue_mib_free(isrran_ue_mib_t* q);

ISRRAN_API int isrran_ue_mib_set_cell(isrran_ue_mib_t* q, isrran_cell_t cell);

ISRRAN_API void isrran_ue_mib_reset(isrran_ue_mib_t* q);

ISRRAN_API int isrran_ue_mib_decode(isrran_ue_mib_t* q,
                                    uint8_t          bch_payload[ISRRAN_BCH_PAYLOAD_LEN],
                                    uint32_t*        nof_tx_ports,
                                    int*             sfn_offset);

/* This interface uses ue_mib and ue_sync to first get synchronized subframes 
 * and then decode MIB
 * 
 * This object calls the pbch object with nof_ports=0 for blind nof_ports determination 
*/
typedef struct {
  isrran_ue_mib_t  ue_mib;
  isrran_ue_sync_t ue_sync;
  cf_t*            sf_buffer[ISRRAN_MAX_CHANNELS];
  uint32_t         nof_rx_channels;
} isrran_ue_mib_sync_t;

ISRRAN_API int
isrran_ue_mib_sync_init_multi(isrran_ue_mib_sync_t* q,
                              int(recv_callback)(void*, cf_t* [ISRRAN_MAX_CHANNELS], uint32_t, isrran_timestamp_t*),
                              uint32_t nof_rx_channels,
                              void*    stream_handler);

ISRRAN_API void isrran_ue_mib_sync_free(isrran_ue_mib_sync_t* q);

ISRRAN_API int isrran_ue_mib_sync_set_cell(isrran_ue_mib_sync_t* q, isrran_cell_t cell);

ISRRAN_API void isrran_ue_mib_sync_reset(isrran_ue_mib_sync_t* q);

ISRRAN_API int isrran_ue_mib_sync_decode(isrran_ue_mib_sync_t* q,
                                         uint32_t              max_frames_timeout,
                                         uint8_t               bch_payload[ISRRAN_BCH_PAYLOAD_LEN],
                                         uint32_t*             nof_tx_ports,
                                         int*                  sfn_offset);

#endif // ISRRAN_UE_MIB_H

