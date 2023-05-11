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
 *  File:         sch.h
 *
 *  Description:  Common UL and DL shared channel encode/decode functions.
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10
 *****************************************************************************/

#ifndef ISRRAN_SCH_H
#define ISRRAN_SCH_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/fec/turbo/rm_turbo.h"
#include "isrran/phy/fec/turbo/turbocoder.h"
#include "isrran/phy/fec/turbo/turbodecoder.h"
#include "isrran/phy/phch/pdsch_cfg.h"
#include "isrran/phy/phch/pusch_cfg.h"
#include "isrran/phy/phch/uci.h"

#ifndef ISRRAN_RX_NULL
#define ISRRAN_RX_NULL 10000
#endif

#ifndef ISRRAN_TX_NULL
#define ISRRAN_TX_NULL 100
#endif

/* DL-SCH AND UL-SCH common functions */
typedef struct ISRRAN_API {

  uint32_t max_iterations;
  float    avg_iterations;

  bool llr_is_8bit;

  /* buffers */
  uint8_t*         cb_in;
  uint8_t*         parity_bits;
  void*            e;
  uint8_t*         temp_g_bits;
  uint32_t*        ul_interleaver;
  isrran_uci_bit_t ack_ri_bits[57600]; // 4*M_sc*Qm_max for RI and ACK

  isrran_tcod_t encoder;
  isrran_tdec_t decoder;
  isrran_crc_t  crc_tb;
  isrran_crc_t  crc_cb;

  isrran_uci_cqi_pusch_t uci_cqi;

} isrran_sch_t;

ISRRAN_API int isrran_sch_init(isrran_sch_t* q);

ISRRAN_API void isrran_sch_free(isrran_sch_t* q);

ISRRAN_API void isrran_sch_set_max_noi(isrran_sch_t* q, uint32_t max_iterations);

ISRRAN_API float isrran_sch_last_noi(isrran_sch_t* q);

ISRRAN_API int isrran_dlsch_encode(isrran_sch_t* q, isrran_pdsch_cfg_t* cfg, uint8_t* data, uint8_t* e_bits);

ISRRAN_API int isrran_dlsch_encode2(isrran_sch_t*       q,
                                    isrran_pdsch_cfg_t* cfg,
                                    uint8_t*            data,
                                    uint8_t*            e_bits,
                                    int                 codeword_idx,
                                    uint32_t            nof_layers);

ISRRAN_API int isrran_dlsch_decode(isrran_sch_t* q, isrran_pdsch_cfg_t* cfg, int16_t* e_bits, uint8_t* data);

ISRRAN_API int isrran_dlsch_decode2(isrran_sch_t*       q,
                                    isrran_pdsch_cfg_t* cfg,
                                    int16_t*            e_bits,
                                    uint8_t*            data,
                                    int                 codeword_idx,
                                    uint32_t            nof_layers);

ISRRAN_API int isrran_ulsch_encode(isrran_sch_t*       q,
                                   isrran_pusch_cfg_t* cfg,
                                   uint8_t*            data,
                                   isrran_uci_value_t* uci_data,
                                   uint8_t*            g_bits,
                                   uint8_t*            q_bits);

ISRRAN_API int isrran_ulsch_decode(isrran_sch_t*       q,
                                   isrran_pusch_cfg_t* cfg,
                                   int16_t*            q_bits,
                                   int16_t*            g_bits,
                                   uint8_t*            c_seq,
                                   uint8_t*            data,
                                   isrran_uci_value_t* uci_data);

ISRRAN_API float isrran_sch_beta_cqi(uint32_t I_cqi);

ISRRAN_API float isrran_sch_beta_ack(uint32_t I_harq);

ISRRAN_API uint32_t isrran_sch_find_Ioffset_ack(float beta);

ISRRAN_API uint32_t isrran_sch_find_Ioffset_cqi(float beta);

ISRRAN_API uint32_t isrran_sch_find_Ioffset_ri(float beta);

///< Sidelink uses PUSCH Interleaver in all channels
ISRRAN_API void isrran_sl_ulsch_interleave(uint8_t* g_bits,
                                           uint32_t Qm,
                                           uint32_t H_prime_total,
                                           uint32_t N_pusch_symbs,
                                           uint8_t* q_bits);

///< Sidelink uses PUSCH Deinterleaver in all channels
ISRRAN_API void isrran_sl_ulsch_deinterleave(int16_t*  q_bits,
                                             uint32_t  Qm,
                                             uint32_t  H_prime_total,
                                             uint32_t  N_pusch_symbs,
                                             int16_t*  g_bits,
                                             uint32_t* inteleaver_lut);

#endif // ISRRAN_SCH_H