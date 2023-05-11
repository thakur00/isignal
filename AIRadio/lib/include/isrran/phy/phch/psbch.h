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

#ifndef ISRRAN_PSBCH_H
#define ISRRAN_PSBCH_H

#include "isrran/phy/common/phy_common_sl.h"
#include "isrran/phy/dft/dft_precoding.h"
#include "isrran/phy/fec/convolutional/convcoder.h"
#include "isrran/phy/fec/convolutional/viterbi.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/modem_table.h"
#include "isrran/phy/scrambling/scrambling.h"

#define ISRRAN_SL_BCH_CRC_LEN 16

/**
 *  \brief Physical Sidelink broadcast channel.
 *
 *  Reference: 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.6
 */
typedef struct ISRRAN_API {
  uint32_t       N_sl_id;
  isrran_sl_tm_t tm;
  isrran_cp_t    cp;

  uint32_t nof_data_re; ///< Number of RE considered during the channel mapping
  uint32_t nof_tx_re;   ///< Number of RE actually transmitted over the air (without last OFDM symbol)
  uint32_t E;
  uint32_t Qm;
  uint32_t nof_prb;
  uint32_t nof_data_symbols;
  uint32_t nof_tx_symbols;
  uint32_t sl_bch_tb_len;
  uint32_t sl_bch_tb_crc_len;
  uint32_t sl_bch_encoded_len;
  float    precoding_scaling;

  // data
  uint8_t* c;

  // crc
  isrran_crc_t crc_mib_sl;
  uint8_t*     crc_temp;

  // channel coding
  isrran_viterbi_t   dec;
  isrran_convcoder_t encoder;
  uint8_t*           d;
  int16_t*           d_16;

  // rate matching
  uint8_t* e;
  uint8_t* e_bytes; ///< To pack bits to bytes
  int16_t* e_16;

  uint8_t* codeword;
  uint8_t* codeword_bytes;
  int16_t* llr;

  // interleaving
  uint32_t* interleaver_lut;

  // scrambling
  isrran_sequence_t seq;

  // modulation
  isrran_modem_table_t mod;
  cf_t*                mod_symbols;

  // dft precoding
  isrran_dft_precoding_t dft_precoder;
  isrran_dft_precoding_t idft_precoder;
  cf_t*                  scfdma_symbols;

} isrran_psbch_t;

ISRRAN_API int
isrran_psbch_init(isrran_psbch_t* q, uint32_t nof_prb, uint32_t N_sl_id, isrran_sl_tm_t tm, isrran_cp_t cp);

ISRRAN_API void isrran_psbch_free(isrran_psbch_t* q);

ISRRAN_API int isrran_psbch_encode(isrran_psbch_t* q, uint8_t* input, uint32_t input_len, cf_t* sf_buffer);

ISRRAN_API int isrran_psbch_decode(isrran_psbch_t* q, cf_t* scfdma_symbols, uint8_t* output, uint32_t max_output_len);

ISRRAN_API int isrran_psbch_reset(isrran_psbch_t* q, uint32_t N_sl_id);

ISRRAN_API int isrran_psbch_put(isrran_psbch_t* q, cf_t* symbols, cf_t* sf_buffer);

ISRRAN_API int isrran_psbch_get(isrran_psbch_t* q, cf_t* sf_buffer, cf_t* symbols);

#endif // ISRRAN_PSBCH_H
