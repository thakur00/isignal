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

#ifndef ISRRAN_PSSCH_H
#define ISRRAN_PSSCH_H

#include "isrran/phy/common/phy_common_sl.h"
#include "isrran/phy/dft/dft_precoding.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/fec/turbo/turbocoder.h"
#include "isrran/phy/fec/turbo/turbodecoder.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/scrambling/scrambling.h"

/**
 *  \brief Physical Sidelink shared channel.
 *
 *  Reference: 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.3
 */

// Redundancy version
static const uint8_t isrran_pssch_rv[4] = {0, 2, 3, 1};

typedef struct ISRRAN_API {
  uint32_t prb_start_idx; // PRB start idx to map RE from RIV
  uint32_t nof_prb;       // PSSCH nof_prbs, Length of continuous PRB to map RE (in the pool) from RIV
  uint32_t N_x_id;
  uint32_t mcs_idx;
  uint32_t rv_idx;
  uint32_t sf_idx; // PSSCH sf_idx
} isrran_pssch_cfg_t;

typedef struct ISRRAN_API {
  isrran_cell_sl_t               cell;
  isrran_sl_comm_resource_pool_t sl_comm_resource_pool;
  isrran_pssch_cfg_t             pssch_cfg;

  isrran_cbsegm_t cb_segm;

  uint32_t G;
  uint32_t E;
  uint32_t Qm;
  uint32_t nof_data_symbols;
  uint32_t nof_tx_symbols;

  uint32_t nof_data_re; // Number of RE considered during the channel mapping
  uint32_t nof_tx_re;   // Number of RE actually transmitted over the air (without last OFDM symbol)
  uint32_t sl_sch_tb_len;
  uint32_t scfdma_symbols_len;

  // data
  uint8_t* b;

  // crc
  uint8_t*     c_r;
  uint8_t*     c_r_bytes;
  uint8_t*     tb_crc_temp;
  isrran_crc_t tb_crc;
  uint8_t*     cb_crc_temp;
  isrran_crc_t cb_crc;

  // channel coding
  uint8_t*      d_r;
  int16_t*      d_r_16;
  isrran_tcod_t tcod;
  isrran_tdec_t tdec;

  // rate matching
  uint8_t* e_r;
  int16_t* e_r_16;
  uint8_t* buff_b;

  uint8_t* codeword;
  uint8_t* codeword_bytes;
  int16_t* llr;

  // interleaving
  uint8_t*  f;
  uint8_t*  f_bytes;
  int16_t*  f_16;
  uint32_t* interleaver_lut;

  // scrambling
  isrran_sequence_t scrambling_seq;

  // modulation
  isrran_mod_t         mod_idx;
  isrran_modem_table_t mod[ISRRAN_MOD_NITEMS];
  cf_t*                symbols;
  uint8_t*             bits_after_demod;
  uint8_t*             bytes_after_demod;

  // dft precoding
  isrran_dft_precoding_t dft_precoder;
  cf_t*                  scfdma_symbols;
  isrran_dft_precoding_t idft_precoder;

} isrran_pssch_t;

ISRRAN_API int  isrran_pssch_init(isrran_pssch_t*                       q,
                                  const isrran_cell_sl_t*               cell,
                                  const isrran_sl_comm_resource_pool_t* sl_comm_resource_pool);
ISRRAN_API int  isrran_pssch_set_cfg(isrran_pssch_t* q, isrran_pssch_cfg_t pssch_cfg);
ISRRAN_API int  isrran_pssch_encode(isrran_pssch_t* q, uint8_t* input, uint32_t input_len, cf_t* sf_buffer);
ISRRAN_API int  isrran_pssch_decode(isrran_pssch_t* q, cf_t* equalized_sf_syms, uint8_t* output, uint32_t output_len);
ISRRAN_API int  isrran_pssch_put(isrran_pssch_t* q, cf_t* sf_buffer, cf_t* symbols);
ISRRAN_API int  isrran_pssch_get(isrran_pssch_t* q, cf_t* sf_buffer, cf_t* symbols);
ISRRAN_API void isrran_pssch_free(isrran_pssch_t* q);

#endif // ISRRAN_PSSCH_H