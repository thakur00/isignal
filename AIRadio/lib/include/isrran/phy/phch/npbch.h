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

#ifndef ISRRAN_NPBCH_H
#define ISRRAN_NPBCH_H

#include "isrran/config.h"
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

#define ISRRAN_MIB_NB_LEN 34
#define ISRRAN_MIB_NB_CRC_LEN (ISRRAN_MIB_NB_LEN + 16)
#define ISRRAN_MIB_NB_ENC_LEN (3 * ISRRAN_MIB_NB_CRC_LEN)

#define ISRRAN_NPBCH_NUM_RE (12 * 11 - 4 * 8) // 100 RE, entire PRB minus 3 symbols minus 4 times NRS=CRS REs
#define ISRRAN_NPBCH_NUM_BLOCKS 8             // MIB-NB is split across 8 blocks
#define ISRRAN_NPBCH_NUM_REP 8                // number of repetitions per block
#define ISRRAN_NPBCH_NUM_FRAMES (ISRRAN_NPBCH_NUM_BLOCKS * ISRRAN_NPBCH_NUM_REP)

typedef struct {
  uint16_t            sfn;
  uint16_t            hfn;
  uint8_t             sched_info_sib1;
  uint8_t             sys_info_tag;
  bool                ac_barring;
  isrran_nbiot_mode_t mode;
} isrran_mib_nb_t;

/**
 * \brief Narrowband Physical broadcast channel (NPBCH)
 *
 *  Reference: 3GPP TS 36.211 version 13.2.0 Release 13 Sec. 10.2.4
 */
typedef struct ISRRAN_API {
  isrran_nbiot_cell_t cell;

  uint32_t frame_idx;
  uint32_t nof_symbols;

  // buffers
  cf_t*    ce[ISRRAN_MAX_PORTS];
  cf_t*    symbols[ISRRAN_MAX_PORTS];
  cf_t*    x[ISRRAN_MAX_PORTS];
  cf_t*    d;
  float*   llr;
  float*   temp;
  float    rm_f[ISRRAN_MIB_NB_ENC_LEN];
  uint8_t* rm_b;
  uint8_t  data[ISRRAN_MIB_NB_CRC_LEN];
  uint8_t  data_enc[ISRRAN_MIB_NB_ENC_LEN];

  isrran_nbiot_mode_t op_mode;

  // tx & rx objects
  isrran_modem_table_t mod;
  isrran_sequence_t    seq;
  isrran_sequence_t    seq_r14[ISRRAN_NPBCH_NUM_BLOCKS];
  isrran_viterbi_t     decoder;
  isrran_crc_t         crc;
  isrran_convcoder_t   encoder;
  bool                 search_all_ports;
} isrran_npbch_t;

ISRRAN_API int isrran_npbch_init(isrran_npbch_t* q);

ISRRAN_API void isrran_npbch_free(isrran_npbch_t* q);

ISRRAN_API int isrran_npbch_set_cell(isrran_npbch_t* q, isrran_nbiot_cell_t cell);

ISRRAN_API void isrran_npbch_mib_pack(uint32_t sfn, uint32_t hfn, isrran_mib_nb_t mib, uint8_t* msg);

ISRRAN_API void isrran_npbch_mib_unpack(uint8_t* msg, isrran_mib_nb_t* mib);

ISRRAN_API void isrran_mib_nb_printf(FILE* stream, isrran_nbiot_cell_t cell, isrran_mib_nb_t* mib);

ISRRAN_API int isrran_npbch_put_subframe(isrran_npbch_t* q,
                                         uint8_t         bch_payload[ISRRAN_MIB_NB_LEN],
                                         cf_t*           sf[ISRRAN_MAX_PORTS],
                                         uint32_t        frame_idx);

ISRRAN_API int isrran_npbch_encode(isrran_npbch_t* q,
                                   uint8_t         bch_payload[ISRRAN_MIB_NB_LEN],
                                   cf_t*           sf[ISRRAN_MAX_PORTS],
                                   uint32_t        frame_idx);

int isrran_npbch_rotate(isrran_npbch_t* q,
                        uint32_t        nf,
                        cf_t*           input_signal,
                        cf_t*           output_signal,
                        int             num_samples,
                        bool            back);

ISRRAN_API int isrran_npbch_decode(isrran_npbch_t* q,
                                   cf_t*           slot1_symbols,
                                   cf_t*           ce[ISRRAN_MAX_PORTS],
                                   float           noise_estimate,
                                   uint8_t         bch_payload[ISRRAN_MIB_NB_LEN],
                                   uint32_t*       nof_tx_ports,
                                   int*            sfn_offset);

ISRRAN_API int isrran_npbch_decode_nf(isrran_npbch_t* q,
                                      cf_t*           slot1_symbols,
                                      cf_t*           ce[ISRRAN_MAX_PORTS],
                                      float           noise_estimate,
                                      uint8_t         bch_payload[ISRRAN_MIB_NB_LEN],
                                      uint32_t*       nof_tx_ports,
                                      int*            sfn_offset,
                                      int             nf);

ISRRAN_API void isrran_npbch_decode_reset(isrran_npbch_t* q);

ISRRAN_API int isrran_npbch_decode_frame(isrran_npbch_t* q,
                                         uint32_t        src,
                                         uint32_t        dst,
                                         uint32_t        n,
                                         uint32_t        nof_bits,
                                         uint32_t        nof_ports);

ISRRAN_API uint32_t isrran_npbch_crc_check(isrran_npbch_t* q, uint8_t* bits, uint32_t nof_ports);

ISRRAN_API void isrran_npbch_crc_set_mask(uint8_t* data, int nof_ports);

ISRRAN_API int isrran_npbch_cp(cf_t* input, cf_t* output, isrran_nbiot_cell_t cell, bool put);

#endif // ISRRAN_NPBCH_H
