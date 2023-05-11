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

/**********************************************************************************************
 *  File:         convcoder.h
 *
 *  Description:  Cyclic Redundancy Check
 *                LTE requires CRC lengths 8, 16, 24A and 24B, each with it's own generator
 *                polynomial.
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10 Sec. 5.1.1
 *********************************************************************************************/

#ifndef ISRRAN_CRC_H
#define ISRRAN_CRC_H

#include "isrran/config.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct ISRRAN_API {
  uint64_t table[256];
  int      polynom;
  int      order;
  uint64_t crcinit;
  uint64_t crcmask;
  uint64_t crchighbit;
  uint32_t isrran_crc_out;
} isrran_crc_t;

ISRRAN_API int isrran_crc_init(isrran_crc_t* h, uint32_t isrran_crc_poly, int isrran_crc_order);

ISRRAN_API int isrran_crc_set_init(isrran_crc_t* h, uint64_t init_value);

ISRRAN_API uint32_t isrran_crc_attach(isrran_crc_t* h, uint8_t* data, int len);

ISRRAN_API uint32_t isrran_crc_attach_byte(isrran_crc_t* h, uint8_t* data, int len);

static inline void isrran_crc_checksum_put_byte(isrran_crc_t* h, uint8_t byte)
{
  uint64_t crc = h->crcinit;

  uint32_t idx;
  if (h->order > 8) {
    // For more than 8 bits
    uint32_t ord = h->order - 8U;
    idx          = ((crc >> (ord)) & 0xffU) ^ byte;
  } else {
    // For 8 bits or less
    uint32_t ord = 8U - h->order;
    idx          = ((crc << (ord)) & 0xffU) ^ byte;
  }

  crc        = (crc << 8U) ^ h->table[idx];
  h->crcinit = crc;
}

static inline uint64_t isrran_crc_checksum_get(isrran_crc_t* h)
{
  return (h->crcinit & h->crcmask);
}

ISRRAN_API uint32_t isrran_crc_checksum_byte(isrran_crc_t* h, const uint8_t* data, int len);

ISRRAN_API uint32_t isrran_crc_checksum(isrran_crc_t* h, uint8_t* data, int len);

ISRRAN_API bool isrran_crc_match_byte(isrran_crc_t* h, uint8_t* data, int len);

ISRRAN_API bool isrran_crc_match(isrran_crc_t* h, uint8_t* data, int len);

#endif // ISRRAN_CRC_H
