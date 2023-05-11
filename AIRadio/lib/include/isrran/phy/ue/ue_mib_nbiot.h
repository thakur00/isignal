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

#ifndef ISRRAN_UE_MIB_NBIOT_H
#define ISRRAN_UE_MIB_NBIOT_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl_nbiot.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/npbch.h"
#include "isrran/phy/sync/cfo.h"
#include "isrran/phy/ue/ue_sync_nbiot.h"

#define ISRRAN_UE_MIB_NBIOT_NOF_PRB 6
#define ISRRAN_UE_MIB_NBIOT_FOUND 1
#define ISRRAN_UE_MIB_NBIOT_NOTFOUND 0

/**
 * \brief This object decodes the MIB-NB from the NPBCH of an NB-IoT LTE signal.
 *
 * The function isrran_ue_mib_nbiot_decode() shall be called multiple
 * times, each passing a number of samples multiple of 19200,
 * sampled at 1.92 MHz (that is, 10 ms of samples).
 *
 * The function uses the sync_t object to find the NPSS sequence and
 * decode the NPBCH to obtain the MIB.
 *
 * The function returns 0 until the MIB is decoded.
 */

typedef struct ISRRAN_API {
  isrran_sync_nbiot_t sfind;

  cf_t* sf_symbols;
  cf_t* ce[ISRRAN_MAX_PORTS];

  isrran_ofdm_t           fft;
  isrran_chest_dl_nbiot_t chest;
  isrran_npbch_t          npbch;

  uint8_t  last_bch_payload[ISRRAN_MIB_NB_LEN];
  uint32_t nof_tx_ports;
  uint32_t nof_rx_antennas;
  uint32_t sfn_offset;

  uint32_t frame_cnt;
} isrran_ue_mib_nbiot_t;

ISRRAN_API int isrran_ue_mib_nbiot_init(isrran_ue_mib_nbiot_t* q, cf_t** in_buffer, uint32_t max_prb);

ISRRAN_API int isrran_ue_mib_nbiot_set_cell(isrran_ue_mib_nbiot_t* q, isrran_nbiot_cell_t cell);

ISRRAN_API void isrran_ue_mib_nbiot_free(isrran_ue_mib_nbiot_t* q);

ISRRAN_API void isrran_ue_mib_nbiot_reset(isrran_ue_mib_nbiot_t* q);

ISRRAN_API int isrran_ue_mib_nbiot_decode(isrran_ue_mib_nbiot_t* q,
                                          cf_t*                  input,
                                          uint8_t*               bch_payload,
                                          uint32_t*              nof_tx_ports,
                                          int*                   sfn_offset);

/* This interface uses ue_mib and ue_sync to first get synchronized subframes
 * and then decode MIB
 *
 * This object calls the pbch object with nof_ports=0 for blind nof_ports determination
 */
typedef struct {
  isrran_ue_mib_nbiot_t  ue_mib;
  isrran_nbiot_ue_sync_t ue_sync;
  cf_t*                  sf_buffer[ISRRAN_MAX_PORTS];
  uint32_t               nof_rx_antennas;
} isrran_ue_mib_sync_nbiot_t;

ISRRAN_API int
isrran_ue_mib_sync_nbiot_init_multi(isrran_ue_mib_sync_nbiot_t* q,
                                    int(recv_callback)(void*, cf_t* [ISRRAN_MAX_PORTS], uint32_t, isrran_timestamp_t*),
                                    uint32_t nof_rx_antennas,
                                    void*    stream_handler);

ISRRAN_API void isrran_ue_mib_sync_nbiot_free(isrran_ue_mib_sync_nbiot_t* q);

ISRRAN_API int isrran_ue_mib_sync_nbiot_set_cell(isrran_ue_mib_sync_nbiot_t* q, isrran_nbiot_cell_t cell);

ISRRAN_API void isrran_ue_mib_sync_nbiot_reset(isrran_ue_mib_sync_nbiot_t* q);

ISRRAN_API int isrran_ue_mib_sync_nbiot_decode(isrran_ue_mib_sync_nbiot_t* q,
                                               uint32_t                    max_frames_timeout,
                                               uint8_t*                    bch_payload,
                                               uint32_t*                   nof_tx_ports,
                                               int*                        sfn_offset);

#endif // ISRRAN_UE_MIB_NBIOT_H
