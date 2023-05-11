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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/fec/softbuffer.h"
#include "isrran/phy/fec/turbo/turbodecoder_gen.h"
#include "isrran/phy/phch/ra.h"
#include "isrran/phy/utils/vector.h"

#define MAX_PDSCH_RE(cp) (2 * ISRRAN_CP_NSYMB(cp) * 12)

int isrran_softbuffer_rx_init(isrran_softbuffer_rx_t* q, uint32_t nof_prb)
{
  int ret = isrran_ra_tbs_from_idx(ISRRAN_RA_NOF_TBS_IDX - 1, nof_prb);

  if (ret == ISRRAN_ERROR) {
    return ISRRAN_ERROR;
  }
  uint32_t max_cb      = (uint32_t)ret / (ISRRAN_TCOD_MAX_LEN_CB - 24) + 1;
  uint32_t max_cb_size = SOFTBUFFER_SIZE;

  return isrran_softbuffer_rx_init_guru(q, max_cb, max_cb_size);
}

int isrran_softbuffer_rx_init_guru(isrran_softbuffer_rx_t* q, uint32_t max_cb, uint32_t max_cb_size)
{
  int ret = ISRRAN_ERROR;

  // Protect pointer
  if (!q) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Initialise object
  ISRRAN_MEM_ZERO(q, isrran_softbuffer_rx_t, 1);

  // Set internal attributes
  q->max_cb      = max_cb;
  q->max_cb_size = max_cb_size;

  q->buffer_f = ISRRAN_MEM_ALLOC(int16_t*, q->max_cb);
  if (!q->buffer_f) {
    perror("malloc");
    goto clean_exit;
  }
  ISRRAN_MEM_ZERO(q->buffer_f, int16_t*, q->max_cb);

  q->data = ISRRAN_MEM_ALLOC(uint8_t*, q->max_cb);
  if (!q->data) {
    perror("malloc");
    goto clean_exit;
  }
  ISRRAN_MEM_ZERO(q->data, uint8_t*, q->max_cb);

  q->cb_crc = ISRRAN_MEM_ALLOC(bool, q->max_cb);
  if (!q->cb_crc) {
    perror("malloc");
    goto clean_exit;
  }

  for (uint32_t i = 0; i < q->max_cb; i++) {
    q->buffer_f[i] = isrran_vec_i16_malloc(q->max_cb_size);
    if (!q->buffer_f[i]) {
      perror("malloc");
      goto clean_exit;
    }

    q->data[i] = isrran_vec_u8_malloc(q->max_cb_size / 8);
    if (!q->data[i]) {
      perror("malloc");
      goto clean_exit;
    }
  }

  isrran_softbuffer_rx_reset(q);

  // Consider success
  ret = ISRRAN_SUCCESS;

clean_exit:
  if (ret) {
    isrran_softbuffer_rx_free(q);
  }

  return ret;
}

void isrran_softbuffer_rx_free(isrran_softbuffer_rx_t* q)
{
  if (q) {
    if (q->buffer_f) {
      for (uint32_t i = 0; i < q->max_cb; i++) {
        if (q->buffer_f[i]) {
          free(q->buffer_f[i]);
        }
      }
      free(q->buffer_f);
    }
    if (q->data) {
      for (uint32_t i = 0; i < q->max_cb; i++) {
        if (q->data[i]) {
          free(q->data[i]);
        }
      }
      free(q->data);
    }
    if (q->cb_crc) {
      free(q->cb_crc);
    }

    ISRRAN_MEM_ZERO(q, isrran_softbuffer_rx_t, 1);
  }
}

void isrran_softbuffer_rx_reset_tbs(isrran_softbuffer_rx_t* q, uint32_t tbs)
{
  uint32_t nof_cb = (tbs + 24) / (ISRRAN_TCOD_MAX_LEN_CB - 24) + 1;
  isrran_softbuffer_rx_reset_cb(q, ISRRAN_MIN(nof_cb, q->max_cb));
}

void isrran_softbuffer_rx_reset(isrran_softbuffer_rx_t* q)
{
  isrran_softbuffer_rx_reset_cb(q, q->max_cb);
}

void isrran_softbuffer_rx_reset_cb(isrran_softbuffer_rx_t* q, uint32_t nof_cb)
{
  if (q->buffer_f) {
    if (nof_cb > q->max_cb) {
      nof_cb = q->max_cb;
    }
    for (uint32_t i = 0; i < nof_cb; i++) {
      if (q->buffer_f[i]) {
        isrran_vec_i16_zero(q->buffer_f[i], q->max_cb_size);
      }
      if (q->data[i]) {
        isrran_vec_u8_zero(q->data[i], q->max_cb_size / 8);
      }
    }
  }
  if (q->cb_crc) {
    ISRRAN_MEM_ZERO(q->cb_crc, bool, q->max_cb);
  }
  q->tb_crc = false;
}

void isrran_softbuffer_rx_reset_cb_crc(isrran_softbuffer_rx_t* q, uint32_t nof_cb)
{
  if (q == NULL || nof_cb == 0) {
    return;
  }

  ISRRAN_MEM_ZERO(q->cb_crc, bool, ISRRAN_MIN(q->max_cb, nof_cb));
}

int isrran_softbuffer_tx_init(isrran_softbuffer_tx_t* q, uint32_t nof_prb)
{
  int ret = isrran_ra_tbs_from_idx(ISRRAN_RA_NOF_TBS_IDX - 1, nof_prb);
  if (ret == ISRRAN_ERROR) {
    return ISRRAN_ERROR;
  }
  uint32_t max_cb      = (uint32_t)ret / (ISRRAN_TCOD_MAX_LEN_CB - 24) + 1;
  uint32_t max_cb_size = SOFTBUFFER_SIZE;

  return isrran_softbuffer_tx_init_guru(q, max_cb, max_cb_size);
}

int isrran_softbuffer_tx_init_guru(isrran_softbuffer_tx_t* q, uint32_t max_cb, uint32_t max_cb_size)
{
  // Protect pointer
  if (!q) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Initialise object
  ISRRAN_MEM_ZERO(q, isrran_softbuffer_tx_t, 1);

  // Set internal attributes
  q->max_cb      = max_cb;
  q->max_cb_size = max_cb_size;

  q->buffer_b = ISRRAN_MEM_ALLOC(uint8_t*, q->max_cb);
  if (!q->buffer_b) {
    perror("malloc");
    return ISRRAN_ERROR;
  }
  ISRRAN_MEM_ZERO(q->buffer_b, uint8_t*, q->max_cb);

  // TODO: Use HARQ buffer limitation based on UE category
  for (uint32_t i = 0; i < q->max_cb; i++) {
    q->buffer_b[i] = isrran_vec_u8_malloc(q->max_cb_size);
    if (!q->buffer_b[i]) {
      perror("malloc");
      return ISRRAN_ERROR;
    }
  }

  isrran_softbuffer_tx_reset(q);

  return ISRRAN_SUCCESS;
}

void isrran_softbuffer_tx_free(isrran_softbuffer_tx_t* q)
{
  if (q) {
    if (q->buffer_b) {
      for (uint32_t i = 0; i < q->max_cb; i++) {
        if (q->buffer_b[i]) {
          free(q->buffer_b[i]);
        }
      }
      free(q->buffer_b);
    }
    ISRRAN_MEM_ZERO(q, isrran_softbuffer_tx_t, 1);
  }
}

void isrran_softbuffer_tx_reset_tbs(isrran_softbuffer_tx_t* q, uint32_t tbs)
{
  uint32_t nof_cb = (tbs + 24) / (ISRRAN_TCOD_MAX_LEN_CB - 24) + 1;
  isrran_softbuffer_tx_reset_cb(q, nof_cb);
}

void isrran_softbuffer_tx_reset(isrran_softbuffer_tx_t* q)
{
  isrran_softbuffer_tx_reset_cb(q, q->max_cb);
}

void isrran_softbuffer_tx_reset_cb(isrran_softbuffer_tx_t* q, uint32_t nof_cb)
{
  if (q->buffer_b) {
    if (nof_cb > q->max_cb) {
      nof_cb = q->max_cb;
    }
    for (uint32_t i = 0; i < nof_cb; i++) {
      if (q->buffer_b[i]) {
        isrran_vec_u8_zero(q->buffer_b[i], q->max_cb_size);
      }
    }
  }
}
