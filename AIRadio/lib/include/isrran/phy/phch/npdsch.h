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

#ifndef ISRRAN_NPDSCH_H
#define ISRRAN_NPDSCH_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/fec/convolutional/convcoder.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/npdsch_cfg.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/phch/sch.h"
#include "isrran/phy/scrambling/scrambling.h"

#define ISRRAN_NPDSCH_MAX_RE                                                                                           \
  (ISRRAN_CP_NORM_SF_NSYMB * ISRRAN_NRE - 8) ///< Full PRB minus 8 RE for NRS (one antenna port)
#define ISRRAN_NPDSCH_MAX_TBS 680            ///< Max TBS in Rel13 NB-IoT
#define ISRRAN_NPDSCH_CRC_LEN (24)
#define ISRRAN_NPDSCH_MAX_TBS_CRC (ISRRAN_NPDSCH_MAX_TBS + ISRRAN_NPDSCH_CRC_LEN)
#define ISRRAN_NPDSCH_MAX_TBS_ENC (3 * ISRRAN_NPDSCH_MAX_TBS_CRC)
#define ISRRAN_NPDSCH_MAX_NOF_SF 10
#define ISRRAN_NPDSCH_NUM_SEQ (2 * ISRRAN_NOF_SF_X_FRAME) ///< for even and odd numbered SFNs

/* @brief Narrowband Physical Downlink shared channel (NPDSCH)
 *
 * Reference:    3GPP TS 36.211 version 13.2.0 Release 13 Sec. 10.2.3
 */
typedef struct ISRRAN_API {
  isrran_nbiot_cell_t cell;
  uint32_t            max_re;
  bool                rnti_is_set;
  uint16_t            rnti;

  // buffers
  uint8_t data[ISRRAN_NPDSCH_MAX_TBS_CRC];
  uint8_t data_enc[ISRRAN_NPDSCH_MAX_TBS_ENC];
  float   rm_f[ISRRAN_NPDSCH_MAX_TBS_ENC];
  cf_t*   ce[ISRRAN_MAX_PORTS];
  cf_t*   symbols[ISRRAN_MAX_PORTS];
  cf_t*   sib_symbols[ISRRAN_MAX_PORTS]; // extra buffer for SIB1 symbols as they may be interleaved with other NPDSCH
  cf_t*   tx_syms[ISRRAN_MAX_PORTS];     // pointer to either symbols or sib1_symbols
  cf_t*   x[ISRRAN_MAX_PORTS];
  cf_t*   d;

  float*   llr;
  uint8_t* temp;
  uint8_t* rm_b;

  // tx & rx objects
  isrran_modem_table_t mod;
  isrran_viterbi_t     decoder;
  isrran_sequence_t    seq[ISRRAN_NPDSCH_NUM_SEQ];
  isrran_crc_t         crc;
  isrran_convcoder_t   encoder;
} isrran_npdsch_t;

typedef struct {
  uint16_t hyper_sfn;
  // TODO: add all other fields
} isrran_sys_info_block_type_1_nb_t;

ISRRAN_API int isrran_npdsch_init(isrran_npdsch_t* q);

ISRRAN_API void isrran_npdsch_free(isrran_npdsch_t* q);

ISRRAN_API int isrran_npdsch_set_cell(isrran_npdsch_t* q, isrran_nbiot_cell_t cell);

ISRRAN_API int isrran_npdsch_set_rnti(isrran_npdsch_t* q, uint16_t rnti);

ISRRAN_API int isrran_npdsch_cfg(isrran_npdsch_cfg_t*        cfg,
                                 isrran_nbiot_cell_t         cell,
                                 isrran_ra_nbiot_dl_grant_t* grant,
                                 uint32_t                    sf_idx);

ISRRAN_API int isrran_npdsch_encode(isrran_npdsch_t*        q,
                                    isrran_npdsch_cfg_t*    cfg,
                                    isrran_softbuffer_tx_t* softbuffer,
                                    uint8_t*                data,
                                    cf_t*                   sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API int isrran_npdsch_encode_rnti_idx(isrran_npdsch_t*        q,
                                             isrran_npdsch_cfg_t*    cfg,
                                             isrran_softbuffer_tx_t* softbuffer,
                                             uint8_t*                data,
                                             uint32_t                rnti_idx,
                                             cf_t*                   sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API int isrran_npdsch_encode_rnti(isrran_npdsch_t*        q,
                                         isrran_npdsch_cfg_t*    cfg,
                                         isrran_softbuffer_tx_t* softbuffer,
                                         uint8_t*                data,
                                         uint16_t                rnti,
                                         cf_t*                   sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API int isrran_npdsch_encode_seq(isrran_npdsch_t*        q,
                                        isrran_npdsch_cfg_t*    cfg,
                                        isrran_softbuffer_tx_t* softbuffer,
                                        uint8_t*                data,
                                        isrran_sequence_t*      seq,
                                        cf_t*                   sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API int isrran_npdsch_decode(isrran_npdsch_t*        q,
                                    isrran_npdsch_cfg_t*    cfg,
                                    isrran_softbuffer_rx_t* softbuffer,
                                    cf_t*                   sf_symbols,
                                    cf_t*                   ce[ISRRAN_MAX_PORTS],
                                    float                   noise_estimate,
                                    uint32_t                sfn,
                                    uint8_t*                data);

ISRRAN_API int isrran_npdsch_decode_rnti(isrran_npdsch_t*        q,
                                         isrran_npdsch_cfg_t*    cfg,
                                         isrran_softbuffer_rx_t* softbuffer,
                                         cf_t*                   sf_symbols,
                                         cf_t*                   ce[ISRRAN_MAX_PORTS],
                                         float                   noise_estimate,
                                         uint16_t                rnti,
                                         uint32_t                sfn,
                                         uint8_t*                data,
                                         uint32_t                rep_counter);

ISRRAN_API int
isrran_npdsch_rm_and_decode(isrran_npdsch_t* q, isrran_npdsch_cfg_t* cfg, float* softbits, uint8_t* data);

ISRRAN_API int
isrran_npdsch_cp(isrran_npdsch_t* q, cf_t* input, cf_t* output, isrran_ra_nbiot_dl_grant_t* grant, bool put);

ISRRAN_API float isrran_npdsch_average_noi(isrran_npdsch_t* q);

ISRRAN_API uint32_t isrran_npdsch_last_noi(isrran_npdsch_t* q);

ISRRAN_API void isrran_npdsch_sib1_pack(isrran_cell_t* cell, isrran_sys_info_block_type_1_nb_t* sib, uint8_t* payload);

ISRRAN_API void isrran_npdsch_sib1_unpack(uint8_t* const msg, isrran_sys_info_block_type_1_nb_t* sib);

#endif // ISRRAN_NPDSCH_H
