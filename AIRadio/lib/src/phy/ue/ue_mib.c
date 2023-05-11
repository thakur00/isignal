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

#include "isrran/isrran.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "isrran/phy/ue/ue_mib.h"

#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

#define MIB_BUFFER_MAX_SAMPLES (3 * ISRRAN_SF_LEN_PRB(ISRRAN_UE_MIB_NOF_PRB))

int isrran_ue_mib_init(isrran_ue_mib_t* q, cf_t* in_buffer, uint32_t max_prb)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL) {
    ret = ISRRAN_ERROR;
    bzero(q, sizeof(isrran_ue_mib_t));

    if (isrran_pbch_init(&q->pbch)) {
      ERROR("Error initiating PBCH");
      goto clean_exit;
    }

    q->sf_symbols = isrran_vec_cf_malloc(ISRRAN_SF_LEN_RE(max_prb, ISRRAN_CP_NORM));
    if (!q->sf_symbols) {
      perror("malloc");
      goto clean_exit;
    }

    if (isrran_ofdm_rx_init(&q->fft, ISRRAN_CP_NORM, in_buffer, q->sf_symbols, max_prb)) {
      ERROR("Error initializing FFT");
      goto clean_exit;
    }
    if (isrran_chest_dl_init(&q->chest, max_prb, 1)) {
      ERROR("Error initializing reference signal");
      goto clean_exit;
    }
    if (isrran_chest_dl_res_init(&q->chest_res, max_prb)) {
      ERROR("Error initializing reference signal");
      goto clean_exit;
    }
    isrran_ue_mib_reset(q);

    ret = ISRRAN_SUCCESS;
  }

clean_exit:
  if (ret == ISRRAN_ERROR) {
    isrran_ue_mib_free(q);
  }
  return ret;
}

void isrran_ue_mib_free(isrran_ue_mib_t* q)
{
  if (q->sf_symbols) {
    free(q->sf_symbols);
  }
  isrran_sync_free(&q->sfind);
  isrran_chest_dl_res_free(&q->chest_res);
  isrran_chest_dl_free(&q->chest);
  isrran_pbch_free(&q->pbch);
  isrran_ofdm_rx_free(&q->fft);

  bzero(q, sizeof(isrran_ue_mib_t));
}

int isrran_ue_mib_set_cell(isrran_ue_mib_t* q, isrran_cell_t cell)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && cell.nof_ports <= ISRRAN_MAX_PORTS) {
    if (isrran_pbch_set_cell(&q->pbch, cell)) {
      ERROR("Error initiating PBCH");
      return ISRRAN_ERROR;
    }
    if (isrran_ofdm_rx_set_prb(&q->fft, cell.cp, cell.nof_prb)) {
      ERROR("Error initializing FFT");
      return ISRRAN_ERROR;
    }

    if (cell.nof_ports == 0) {
      cell.nof_ports = ISRRAN_MAX_PORTS;
    }

    if (isrran_chest_dl_set_cell(&q->chest, cell)) {
      ERROR("Error initializing reference signal");
      return ISRRAN_ERROR;
    }
    isrran_ue_mib_reset(q);

    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

void isrran_ue_mib_reset(isrran_ue_mib_t* q)
{
  q->frame_cnt = 0;
  isrran_pbch_decode_reset(&q->pbch);
}

int isrran_ue_mib_decode(isrran_ue_mib_t* q,
                         uint8_t          bch_payload[ISRRAN_BCH_PAYLOAD_LEN],
                         uint32_t*        nof_tx_ports,
                         int*             sfn_offset)
{
  int ret = ISRRAN_SUCCESS;

  /* Run FFT for the slot symbols */
  isrran_ofdm_rx_sf(&q->fft);

  // sf_idx is always 0 in MIB
  isrran_dl_sf_cfg_t sf_cfg;
  ZERO_OBJECT(sf_cfg);

  // Current MIB decoder implementation uses a single antenna
  cf_t* sf_buffer[ISRRAN_MAX_PORTS] = {};
  sf_buffer[0]                      = q->sf_symbols;

  /* Get channel estimates of sf idx #0 for each port */
  ret = isrran_chest_dl_estimate(&q->chest, &sf_cfg, sf_buffer, &q->chest_res);
  if (ret < 0) {
    return ISRRAN_ERROR;
  }
  /* Reset decoder if we missed a frame */
  if (q->frame_cnt > 8) {
    INFO("Resetting PBCH decoder after %d frames", q->frame_cnt);
    isrran_ue_mib_reset(q);
  }

  /* Decode PBCH */
  ret = isrran_pbch_decode(&q->pbch, &q->chest_res, sf_buffer, bch_payload, nof_tx_ports, sfn_offset);
  if (ret < 0) {
    ERROR("Error decoding PBCH (%d)", ret);
  } else if (ret == 1) {
    INFO("MIB decoded: %u, snr=%.1f dB", q->frame_cnt, q->chest_res.snr_db);
    isrran_ue_mib_reset(q);
    ret = ISRRAN_UE_MIB_FOUND;
  } else {
    ret = ISRRAN_UE_MIB_NOTFOUND;
    INFO("MIB not decoded: %u, snr=%.1f dB", q->frame_cnt, q->chest_res.snr_db);
    q->frame_cnt++;
  }

  return ret;
}

int isrran_ue_mib_sync_init_multi(isrran_ue_mib_sync_t* q,
                                  int(recv_callback)(void*, cf_t* [ISRRAN_MAX_CHANNELS], uint32_t, isrran_timestamp_t*),
                                  uint32_t nof_rx_channels,
                                  void*    stream_handler)
{
  for (int i = 0; i < nof_rx_channels; i++) {
    q->sf_buffer[i] = isrran_vec_cf_malloc(MIB_BUFFER_MAX_SAMPLES);
  }
  q->nof_rx_channels = nof_rx_channels;

  // Use 1st RF channel only to receive MIB
  if (isrran_ue_mib_init(&q->ue_mib, q->sf_buffer[0], ISRRAN_UE_MIB_NOF_PRB)) {
    ERROR("Error initiating ue_mib");
    return ISRRAN_ERROR;
  }
  // Configure ue_sync to receive all channels
  if (isrran_ue_sync_init_multi(
          &q->ue_sync, ISRRAN_UE_MIB_NOF_PRB, false, recv_callback, nof_rx_channels, stream_handler)) {
    fprintf(stderr, "Error initiating ue_sync\n");
    isrran_ue_mib_free(&q->ue_mib);
    return ISRRAN_ERROR;
  }
  return ISRRAN_SUCCESS;
}

int isrran_ue_mib_sync_set_cell(isrran_ue_mib_sync_t* q, isrran_cell_t cell)
{
  // If the ports are set to 0, ue_mib goes through 1, 2 and 4 ports to blindly detect nof_ports
  cell.nof_ports = 0;

  // MIB search is done at 6 PRB
  cell.nof_prb = ISRRAN_UE_MIB_NOF_PRB;

  if (isrran_ue_mib_set_cell(&q->ue_mib, cell)) {
    ERROR("Error initiating ue_mib");
    return ISRRAN_ERROR;
  }
  if (isrran_ue_sync_set_cell(&q->ue_sync, cell)) {
    ERROR("Error initiating ue_sync");
    isrran_ue_mib_free(&q->ue_mib);
    return ISRRAN_ERROR;
  }
  return ISRRAN_SUCCESS;
}

void isrran_ue_mib_sync_free(isrran_ue_mib_sync_t* q)
{
  for (int i = 0; i < q->nof_rx_channels; i++) {
    if (q->sf_buffer[i]) {
      free(q->sf_buffer[i]);
    }
  }
  isrran_ue_mib_free(&q->ue_mib);
  isrran_ue_sync_free(&q->ue_sync);
}

void isrran_ue_mib_sync_reset(isrran_ue_mib_sync_t* q)
{
  isrran_ue_mib_reset(&q->ue_mib);
  isrran_ue_sync_reset(&q->ue_sync);
}

int isrran_ue_mib_sync_decode(isrran_ue_mib_sync_t* q,
                              uint32_t              max_frames_timeout,
                              uint8_t               bch_payload[ISRRAN_BCH_PAYLOAD_LEN],
                              uint32_t*             nof_tx_ports,
                              int*                  sfn_offset)
{
  int      ret        = ISRRAN_ERROR_INVALID_INPUTS;
  uint32_t nof_frames = 0;
  int      mib_ret    = ISRRAN_UE_MIB_NOTFOUND;

  if (q == NULL) {
    return ret;
  }

  isrran_ue_mib_sync_reset(q);

  do {
    mib_ret = ISRRAN_UE_MIB_NOTFOUND;
    ret     = isrran_ue_sync_zerocopy(&q->ue_sync, q->sf_buffer, MIB_BUFFER_MAX_SAMPLES);
    if (ret < 0) {
      ERROR("Error calling isrran_ue_sync_work()");
      return -1;
    }

    if (isrran_ue_sync_get_sfidx(&q->ue_sync) == 0) {
      if (ret == 1) {
        mib_ret = isrran_ue_mib_decode(&q->ue_mib, bch_payload, nof_tx_ports, sfn_offset);
      } else {
        DEBUG("Resetting PBCH decoder after %d frames", q->ue_mib.frame_cnt);
        isrran_ue_mib_reset(&q->ue_mib);
      }
      nof_frames++;
    }
  } while (mib_ret == ISRRAN_UE_MIB_NOTFOUND && nof_frames < max_frames_timeout);

  return mib_ret;
}
