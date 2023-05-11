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
 *  File:         uci.h
 *
 *  Description:  Uplink control information. Only 1-bit ACK for UCI on PUSCCH is supported
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10 Sec. 5.2.3, 5.2.4
 *****************************************************************************/

#ifndef ISRRAN_UCI_H
#define ISRRAN_UCI_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/fec/convolutional/viterbi.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/phch/pusch_cfg.h"
#include "isrran/phy/phch/uci_cfg.h"

#define ISRRAN_UCI_MAX_CQI_LEN_PUSCH 512
#define ISRRAN_UCI_MAX_CQI_LEN_PUCCH 13
#define ISRRAN_UCI_CQI_CODED_PUCCH_B 20
#define ISRRAN_UCI_STR_MAX_CHAR 32

typedef struct ISRRAN_API {
  isrran_crc_t     crc;
  isrran_viterbi_t viterbi;
  uint8_t          tmp_cqi[ISRRAN_UCI_MAX_CQI_LEN_PUSCH];
  uint8_t          encoded_cqi[3 * ISRRAN_UCI_MAX_CQI_LEN_PUSCH];
  int16_t          encoded_cqi_s[3 * ISRRAN_UCI_MAX_CQI_LEN_PUSCH];
} isrran_uci_cqi_pusch_t;

typedef struct ISRRAN_API {
  uint8_t** cqi_table;
  int16_t** cqi_table_s;
} isrran_uci_cqi_pucch_t;

ISRRAN_API void isrran_uci_cqi_pucch_init(isrran_uci_cqi_pucch_t* q);

ISRRAN_API void isrran_uci_cqi_pucch_free(isrran_uci_cqi_pucch_t* q);

ISRRAN_API int
isrran_uci_encode_cqi_pucch(uint8_t* cqi_data, uint32_t cqi_len, uint8_t b_bits[ISRRAN_UCI_CQI_CODED_PUCCH_B]);

ISRRAN_API int isrran_uci_encode_cqi_pucch_from_table(isrran_uci_cqi_pucch_t* q,
                                                      uint8_t*                cqi_data,
                                                      uint32_t                cqi_len,
                                                      uint8_t                 b_bits[ISRRAN_UCI_CQI_CODED_PUCCH_B]);

ISRRAN_API int16_t isrran_uci_decode_cqi_pucch(isrran_uci_cqi_pucch_t* q,
                                               int16_t                 b_bits[ISRRAN_CQI_MAX_BITS], // aligned for simd
                                               uint8_t*                cqi_data,
                                               uint32_t                cqi_len);

ISRRAN_API int isrran_uci_cqi_init(isrran_uci_cqi_pusch_t* q);

ISRRAN_API void isrran_uci_cqi_free(isrran_uci_cqi_pusch_t* q);

ISRRAN_API int isrran_uci_encode_cqi_pusch(isrran_uci_cqi_pusch_t* q,
                                           isrran_pusch_cfg_t*     cfg,
                                           uint8_t*                cqi_data,
                                           uint32_t                cqi_len,
                                           float                   beta,
                                           uint32_t                Q_prime_ri,
                                           uint8_t*                q_bits);

ISRRAN_API int isrran_uci_decode_cqi_pusch(isrran_uci_cqi_pusch_t* q,
                                           isrran_pusch_cfg_t*     cfg,
                                           int16_t*                q_bits,
                                           float                   beta,
                                           uint32_t                Q_prime_ri,
                                           uint32_t                cqi_len,
                                           uint8_t*                cqi_data,
                                           bool*                   cqi_ack);

ISRRAN_API int isrran_uci_encode_ack(isrran_pusch_cfg_t* cfg,
                                     uint8_t             acks[2],
                                     uint32_t            nof_acks,
                                     uint32_t            O_cqi,
                                     float               beta,
                                     uint32_t            H_prime_total,
                                     isrran_uci_bit_t*   ri_bits);

ISRRAN_API int isrran_uci_encode_ack_ri(isrran_pusch_cfg_t* cfg,
                                        uint8_t*            data,
                                        uint32_t            O_ack,
                                        uint32_t            O_cqi,
                                        float               beta,
                                        uint32_t            H_prime_total,
                                        bool                input_is_ri,
                                        uint32_t            N_bundle,
                                        isrran_uci_bit_t*   ri_bits);

ISRRAN_API int isrran_uci_decode_ack_ri(isrran_pusch_cfg_t* cfg,
                                        int16_t*            q_bits,
                                        uint8_t*            c_seq,
                                        float               beta,
                                        uint32_t            H_prime_total,
                                        uint32_t            O_cqi,
                                        isrran_uci_bit_t*   ack_ri_bits,
                                        uint8_t*            data,
                                        bool*               valid,
                                        uint32_t            nof_bits,
                                        bool                is_ri);

/**
 * Calculates the maximum number of coded symbols used by CQI-UCI over PUSCH
 */
ISRRAN_API uint32_t isrran_qprime_cqi_ext(uint32_t L_prb, uint32_t nof_symbols, uint32_t tbs, float beta);

/**
 * Calculates the maximum number of coded symbols used by ACK/RI over PUSCH
 */
ISRRAN_API uint32_t
isrran_qprime_ack_ext(uint32_t L_prb, uint32_t nof_symbols, uint32_t tbs, uint32_t nof_ack, float beta);

/**
 * Calculates the number of acknowledgements carried by the Uplink Control Information (UCI) deduced from the number of
 * transport blocks indicated in the UCI's configuration.
 *
 * @param uci_cfg is the UCI configuration
 * @return the number of acknowledgements
 */
ISRRAN_API uint32_t isrran_uci_cfg_total_ack(const isrran_uci_cfg_t* uci_cfg);

ISRRAN_API void isrran_uci_data_reset(isrran_uci_data_t* uci_data);

ISRRAN_API int
isrran_uci_data_info(isrran_uci_cfg_t* uci_cfg, isrran_uci_value_t* uci_data, char* str, uint32_t maxlen);

#endif // ISRRAN_UCI_H
