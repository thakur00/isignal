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

#include "isrran/phy/ue/ue_mib_nbiot.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>

int isrran_ue_mib_nbiot_init(isrran_ue_mib_nbiot_t* q, cf_t** in_buffer, uint32_t max_prb)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL) {
    ret = ISRRAN_ERROR;
    bzero(q, sizeof(isrran_ue_mib_nbiot_t));

    if (isrran_npbch_init(&q->npbch)) {
      fprintf(stderr, "Error initiating NPBCH\n");
      goto clean_exit;
    }

    q->sf_symbols = isrran_vec_cf_malloc(ISRRAN_SF_LEN_RE(max_prb, ISRRAN_CP_NORM));
    if (!q->sf_symbols) {
      perror("malloc");
      goto clean_exit;
    }

    for (uint32_t i = 0; i < ISRRAN_MAX_PORTS; i++) {
      q->ce[i] = isrran_vec_cf_malloc(ISRRAN_SF_LEN_RE(max_prb, ISRRAN_CP_NORM));
      if (!q->ce[i]) {
        perror("malloc");
        goto clean_exit;
      }
    }

    if (isrran_ofdm_rx_init(&q->fft, ISRRAN_CP_NORM, in_buffer[0], q->sf_symbols, max_prb)) {
      fprintf(stderr, "Error initializing FFT\n");
      goto clean_exit;
    }
    isrran_ofdm_set_freq_shift(&q->fft, ISRRAN_NBIOT_FREQ_SHIFT_FACTOR);

    if (isrran_chest_dl_nbiot_init(&q->chest, max_prb)) {
      fprintf(stderr, "Error initializing reference signal\n");
      goto clean_exit;
    }
    isrran_ue_mib_nbiot_reset(q);

    ret = ISRRAN_SUCCESS;
  }

clean_exit:
  if (ret == ISRRAN_ERROR) {
    isrran_ue_mib_nbiot_free(q);
  }
  return ret;
}

void isrran_ue_mib_nbiot_free(isrran_ue_mib_nbiot_t* q)
{
  if (q->sf_symbols) {
    free(q->sf_symbols);
  }
  for (int i = 0; i < ISRRAN_MAX_PORTS; i++) {
    if (q->ce[i]) {
      free(q->ce[i]);
    }
  }
  isrran_sync_nbiot_free(&q->sfind);
  isrran_chest_dl_nbiot_free(&q->chest);
  isrran_npbch_free(&q->npbch);
  isrran_ofdm_rx_free(&q->fft);

  bzero(q, sizeof(isrran_ue_mib_nbiot_t));
}

int isrran_ue_mib_nbiot_set_cell(isrran_ue_mib_nbiot_t* q, isrran_nbiot_cell_t cell)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && cell.nof_ports <= ISRRAN_NBIOT_MAX_PORTS) {
    ret = ISRRAN_ERROR;

    if (isrran_npbch_set_cell(&q->npbch, cell)) {
      fprintf(stderr, "Error setting cell in NPBCH\n");
      goto clean_exit;
    }

    if (cell.nof_ports == 0) {
      cell.nof_ports = ISRRAN_NBIOT_MAX_PORTS;
    }

    if (isrran_chest_dl_nbiot_set_cell(&q->chest, cell)) {
      fprintf(stderr, "Error initializing reference signal\n");
      goto clean_exit;
    }
    isrran_ue_mib_nbiot_reset(q);

    ret = ISRRAN_SUCCESS;
  }

clean_exit:
  if (ret == ISRRAN_ERROR) {
    isrran_ue_mib_nbiot_free(q);
  }
  return ret;
}

void isrran_ue_mib_nbiot_reset(isrran_ue_mib_nbiot_t* q)
{
  q->frame_cnt = 0;
  isrran_npbch_decode_reset(&q->npbch);
}

int isrran_ue_mib_nbiot_decode(isrran_ue_mib_nbiot_t* q,
                               cf_t*                  input,
                               uint8_t*               bch_payload,
                               uint32_t*              nof_tx_ports,
                               int*                   sfn_offset)
{
  // Run FFT for the symbols
  isrran_ofdm_rx_sf(&q->fft);

  // Get channel estimates of sf idx #0 for each port
  if (isrran_chest_dl_nbiot_estimate(&q->chest, q->sf_symbols, q->ce, 0) < 0) {
    return ISRRAN_ERROR;
  }

  // Reset decoder if we missed a NPBCH TTI
  if (q->frame_cnt > ISRRAN_NPBCH_NUM_FRAMES) {
    INFO("Resetting NPBCH decoder after %d frames", q->frame_cnt);
    isrran_ue_mib_nbiot_reset(q);
    return ISRRAN_UE_MIB_NBIOT_NOTFOUND;
  }

  // Decode NPBCH
  if (ISRRAN_SUCCESS == isrran_npbch_decode_nf(&q->npbch,
                                               q->sf_symbols,
                                               q->ce,
                                               isrran_chest_dl_nbiot_get_noise_estimate(&q->chest),
                                               bch_payload,
                                               nof_tx_ports,
                                               sfn_offset,
                                               0)) {
    DEBUG("BCH decoded ok with offset %d", *sfn_offset);
    if (memcmp(bch_payload, q->last_bch_payload, ISRRAN_MIB_NB_LEN) == 0) {
      DEBUG("BCH content equals last BCH, new counter %d", q->frame_cnt);
    } else {
      // new BCH transmitted
      if (q->frame_cnt != 0) {
        INFO("MIB-NB decoded: %u with offset %d", q->frame_cnt, *sfn_offset);
        if (*sfn_offset != 0) {
          INFO("New BCH was decoded at block offset %d. SFN may be corrupted.", *sfn_offset);
        }
        isrran_ue_mib_nbiot_reset(q);
        return ISRRAN_UE_MIB_NBIOT_FOUND;
      } else {
        // store new BCH
        DEBUG("New BCH transmitted after %d frames", q->frame_cnt);
        memcpy(q->last_bch_payload, bch_payload, ISRRAN_MIB_NB_LEN);
      }
    }
    q->frame_cnt++;
  }
  return ISRRAN_UE_MIB_NBIOT_NOTFOUND;
}

int isrran_ue_mib_sync_nbiot_init_multi(
    isrran_ue_mib_sync_nbiot_t* q,
    int(recv_callback)(void*, cf_t* [ISRRAN_MAX_PORTS], uint32_t, isrran_timestamp_t*),
    uint32_t nof_rx_antennas,
    void*    stream_handler)
{
  // Allocate memory for time re-alignment and MIB detection
  for (int i = 0; i < ISRRAN_MAX_PORTS; i++) {
    q->sf_buffer[i] = NULL;
  }
  for (int i = 0; i < nof_rx_antennas; i++) {
    q->sf_buffer[i] = isrran_vec_cf_malloc(ISRRAN_SF_LEN_PRB_NBIOT * ISRRAN_NOF_SF_X_FRAME);
  }
  q->nof_rx_antennas = nof_rx_antennas;

  if (isrran_ue_mib_nbiot_init(&q->ue_mib, q->sf_buffer, ISRRAN_NBIOT_MAX_PRB)) {
    fprintf(stderr, "Error initiating ue_mib\n");
    return ISRRAN_ERROR;
  }
  if (isrran_ue_sync_nbiot_init_multi(
          &q->ue_sync, ISRRAN_NBIOT_MAX_PRB, recv_callback, q->nof_rx_antennas, stream_handler)) {
    fprintf(stderr, "Error initiating ue_sync\n");
    isrran_ue_mib_nbiot_free(&q->ue_mib);
    return ISRRAN_ERROR;
  }
  return ISRRAN_SUCCESS;
}

int isrran_ue_mib_sync_nbiot_set_cell(isrran_ue_mib_sync_nbiot_t* q, isrran_nbiot_cell_t cell)
{
  if (isrran_ue_mib_nbiot_set_cell(&q->ue_mib, cell)) {
    fprintf(stderr, "Error initiating ue_mib\n");
    return ISRRAN_ERROR;
  }
  if (isrran_ue_sync_nbiot_set_cell(&q->ue_sync, cell)) {
    fprintf(stderr, "Error initiating ue_sync\n");
    isrran_ue_mib_nbiot_free(&q->ue_mib);
    return ISRRAN_ERROR;
  }
  return ISRRAN_SUCCESS;
}

void isrran_ue_mib_sync_nbiot_free(isrran_ue_mib_sync_nbiot_t* q)
{
  isrran_ue_mib_nbiot_free(&q->ue_mib);
  isrran_ue_sync_nbiot_free(&q->ue_sync);
}

void isrran_ue_mib_sync_nbiot_reset(isrran_ue_mib_sync_nbiot_t* q)
{
  isrran_ue_mib_nbiot_reset(&q->ue_mib);
  isrran_ue_sync_nbiot_reset(&q->ue_sync);
}

int isrran_ue_mib_sync_nbiot_decode(isrran_ue_mib_sync_nbiot_t* q,
                                    uint32_t                    max_frames_timeout,
                                    uint8_t*                    bch_payload,
                                    uint32_t*                   nof_tx_ports,
                                    int*                        sfn_offset)
{
  int      ret        = ISRRAN_ERROR_INVALID_INPUTS;
  uint32_t nof_frames = 0;
  int      mib_ret    = ISRRAN_UE_MIB_NBIOT_NOTFOUND;

  if (q == NULL) {
    return ret;
  }

  do {
    mib_ret = ISRRAN_UE_MIB_NBIOT_NOTFOUND;
    ret     = isrran_ue_sync_nbiot_zerocopy_multi(&q->ue_sync, q->sf_buffer);
    if (ret < 0) {
      fprintf(stderr, "Error calling isrran_ue_sync_nbiot_zerocopy_multi()\n");
      break;
    }

    if (isrran_ue_sync_nbiot_get_sfidx(&q->ue_sync) == 0) {
      mib_ret = isrran_ue_mib_nbiot_decode(&q->ue_mib, NULL, bch_payload, nof_tx_ports, sfn_offset);
      if (mib_ret < 0) {
        DEBUG("Resetting NPBCH decoder after %d frames", q->ue_mib.frame_cnt);
        isrran_ue_mib_nbiot_reset(&q->ue_mib);
      }
      nof_frames++;
    }
  } while (mib_ret == ISRRAN_UE_MIB_NBIOT_NOTFOUND && nof_frames < max_frames_timeout);

  return mib_ret;
}
