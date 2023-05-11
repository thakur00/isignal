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
 *  File:         softbuffer.h
 *
 *  Description:  Buffer for RX and TX soft bits. This should be provided by MAC.
 *                Provided here basically for the examples.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_SOFTBUFFER_H
#define ISRRAN_SOFTBUFFER_H

#include "isrran/config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ISRRAN_API {
  uint32_t  max_cb;
  uint32_t  max_cb_size;
  int16_t** buffer_f;
  uint8_t** data;
  bool*     cb_crc;
  bool      tb_crc;
} isrran_softbuffer_rx_t;

typedef struct ISRRAN_API {
  uint32_t  max_cb;
  uint32_t  max_cb_size;
  uint8_t** buffer_b;
} isrran_softbuffer_tx_t;

#define SOFTBUFFER_SIZE 18600

ISRRAN_API int isrran_softbuffer_rx_init(isrran_softbuffer_rx_t* q, uint32_t nof_prb);

/**
 * @brief Initialises Rx soft-buffer for a number of code blocks and their size
 * @param q The Rx soft-buffer pointer
 * @param max_cb The maximum number of code blocks to allocate
 * @param max_cb_size The code block size to allocate
 * @return It returns ISRRAN_SUCCESS if it allocates the soft-buffer successfully, otherwise it returns ISRRAN_ERROR
 * code
 */
ISRRAN_API int isrran_softbuffer_rx_init_guru(isrran_softbuffer_rx_t* q, uint32_t max_cb, uint32_t max_cb_size);

ISRRAN_API void isrran_softbuffer_rx_reset(isrran_softbuffer_rx_t* p);

ISRRAN_API void isrran_softbuffer_rx_reset_tbs(isrran_softbuffer_rx_t* q, uint32_t tbs);

ISRRAN_API void isrran_softbuffer_rx_reset_cb(isrran_softbuffer_rx_t* q, uint32_t nof_cb);

ISRRAN_API void isrran_softbuffer_rx_free(isrran_softbuffer_rx_t* p);

/**
 * @brief Resets a number of CB CRCs
 * @note This function is intended to be used if all CB CRC have matched but the TB CRC failed. In this case, all CB
 * should be decoded again
 * @param q Rx soft-buffer object
 * @param nof_cb Number of CB to reset
 */
ISRRAN_API void isrran_softbuffer_rx_reset_cb_crc(isrran_softbuffer_rx_t* q, uint32_t nof_cb);

ISRRAN_API int isrran_softbuffer_tx_init(isrran_softbuffer_tx_t* q, uint32_t nof_prb);

/**
 * @brief Initialises Tx soft-buffer for a number of code blocks and their size
 * @param q The Tx soft-buffer pointer
 * @param max_cb The maximum number of code blocks to allocate
 * @param max_cb_size The code block size to allocate
 * @return It returns ISRRAN_SUCCESS if it allocates the soft-buffer successfully, otherwise it returns ISRRAN_ERROR
 * code
 */
ISRRAN_API int isrran_softbuffer_tx_init_guru(isrran_softbuffer_tx_t* q, uint32_t max_cb, uint32_t max_cb_size);

ISRRAN_API void isrran_softbuffer_tx_reset(isrran_softbuffer_tx_t* p);

ISRRAN_API void isrran_softbuffer_tx_reset_tbs(isrran_softbuffer_tx_t* q, uint32_t tbs);

ISRRAN_API void isrran_softbuffer_tx_reset_cb(isrran_softbuffer_tx_t* q, uint32_t nof_cb);

ISRRAN_API void isrran_softbuffer_tx_free(isrran_softbuffer_tx_t* p);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_SOFTBUFFER_H
