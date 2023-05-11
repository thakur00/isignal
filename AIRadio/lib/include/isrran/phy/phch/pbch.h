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
 *  File:         pbch.h
 *
 *  Description:  Physical broadcast channel. If cell.nof_ports = 0, the number
 *                of ports is blindly determined using the CRC of the received
 *                codeword for 1, 2 and 4 ports
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.6
 *****************************************************************************/

#ifndef ISRRAN_PBCH_H
#define ISRRAN_PBCH_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/fec/convolutional/convcoder.h"
#include "isrran/phy/fec/convolutional/rm_conv.h"
#include "isrran/phy/fec/convolutional/viterbi.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/scrambling/scrambling.h"

#define ISRRAN_BCH_PAYLOAD_LEN 24
#define ISRRAN_BCH_PAYLOADCRC_LEN (ISRRAN_BCH_PAYLOAD_LEN + 16)
#define ISRRAN_BCH_ENCODED_LEN 3 * (ISRRAN_BCH_PAYLOADCRC_LEN)

#define ISRRAN_PBCH_MAX_RE 256 // make it avx2-aligned

/* PBCH object */
typedef struct ISRRAN_API {
  isrran_cell_t cell;

  uint32_t nof_symbols;

  /* buffers */
  cf_t*    ce[ISRRAN_MAX_PORTS];
  cf_t*    symbols[ISRRAN_MAX_PORTS];
  cf_t*    x[ISRRAN_MAX_PORTS];
  cf_t*    d;
  float*   llr;
  float*   temp;
  float    rm_f[ISRRAN_BCH_ENCODED_LEN];
  uint8_t* rm_b;
  uint8_t  data[ISRRAN_BCH_PAYLOADCRC_LEN];
  uint8_t  data_enc[ISRRAN_BCH_ENCODED_LEN];

  uint32_t frame_idx;

  /* tx & rx objects */
  isrran_modem_table_t mod;
  isrran_sequence_t    seq;
  isrran_viterbi_t     decoder;
  isrran_crc_t         crc;
  isrran_convcoder_t   encoder;
  bool                 search_all_ports;

} isrran_pbch_t;

ISRRAN_API int isrran_pbch_init(isrran_pbch_t* q);

ISRRAN_API void isrran_pbch_free(isrran_pbch_t* q);

ISRRAN_API int isrran_pbch_set_cell(isrran_pbch_t* q, isrran_cell_t cell);

ISRRAN_API int isrran_pbch_decode(isrran_pbch_t*         q,
                                  isrran_chest_dl_res_t* channel,
                                  cf_t*                  sf_symbols[ISRRAN_MAX_PORTS],
                                  uint8_t                bch_payload[ISRRAN_BCH_PAYLOAD_LEN],
                                  uint32_t*              nof_tx_ports,
                                  int*                   sfn_offset);

ISRRAN_API int isrran_pbch_encode(isrran_pbch_t* q,
                                  uint8_t        bch_payload[ISRRAN_BCH_PAYLOAD_LEN],
                                  cf_t*          sf_symbols[ISRRAN_MAX_PORTS],
                                  uint32_t       frame_idx);

ISRRAN_API void isrran_pbch_decode_reset(isrran_pbch_t* q);

ISRRAN_API void isrran_pbch_mib_unpack(uint8_t* msg, isrran_cell_t* cell, uint32_t* sfn);

ISRRAN_API void isrran_pbch_mib_pack(isrran_cell_t* cell, uint32_t sfn, uint8_t* msg);

#endif // ISRRAN_PBCH_H
