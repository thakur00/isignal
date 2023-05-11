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

#ifndef ISRRAN_PSCCH_H
#define ISRRAN_PSCCH_H

#include <stdint.h>

#include "isrran/phy/common/phy_common_sl.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/dft/dft_precoding.h"
#include "isrran/phy/fec/convolutional/convcoder.h"
#include "isrran/phy/fec/convolutional/viterbi.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/modem/modem_table.h"

/**
 *  \brief Physical Sidelink control channel.
 *
 *  Reference: 3GPP TS 36.211 version 15.6.0 Release 15 Section 9.4
 */

typedef struct ISRRAN_API {

  uint32_t         max_prb;
  isrran_cell_sl_t cell;

  uint32_t sci_len;
  uint32_t nof_tx_re;

  uint32_t pscch_nof_prb;

  // crc
  uint8_t*     c;
  isrran_crc_t crc;
  uint8_t*     sci_crc;

  // channel coding
  isrran_viterbi_t   dec;
  isrran_convcoder_t encoder;
  uint8_t*           d;
  int16_t*           d_16;

  // rate matching
  uint32_t E;
  uint8_t* e;
  int16_t* e_16;

  uint8_t* e_bytes; ///< To pack bits to bytes
  uint32_t nof_symbols;

  // interleaving
  uint32_t* interleaver_lut;
  uint8_t*  codeword;
  uint8_t*  codeword_bytes;

  // scrambling
  isrran_sequence_t seq;

  // modulation
  isrran_modem_table_t mod;
  cf_t*                mod_symbols;
  int16_t*             llr;

  // dft precoding
  isrran_dft_precoding_t dft_precoder;
  isrran_dft_precoding_t idft_precoder;

  cf_t* scfdma_symbols;

} isrran_pscch_t;

ISRRAN_API int  isrran_pscch_init(isrran_pscch_t* q, uint32_t max_prb);
ISRRAN_API int  isrran_pscch_set_cell(isrran_pscch_t* q, isrran_cell_sl_t cell);
ISRRAN_API int  isrran_pscch_encode(isrran_pscch_t* q, uint8_t* sci, cf_t* sf_buffer, uint32_t prb_start_idx);
ISRRAN_API int  isrran_pscch_decode(isrran_pscch_t* q, cf_t* equalized_sf_syms, uint8_t* sci, uint32_t prb_start_idx);
ISRRAN_API int  isrran_pscch_put(isrran_pscch_t* q, cf_t* sf_buffer, uint32_t prb_start_idx);
ISRRAN_API int  isrran_pscch_get(isrran_pscch_t* q, cf_t* sf_buffer, uint32_t prb_start_idx);
ISRRAN_API void isrran_pscch_free(isrran_pscch_t* q);

#endif // ISRRAN_PSCCH_H
