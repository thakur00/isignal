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
 *  File:         sch_nr.h
 *
 *  Description:  Common UL and DL shared channel encode/decode functions for NR.
 *
 *  Reference:    3GPP TS 38.212 V15.9.0
 *****************************************************************************/

#ifndef ISRRAN_SCH_NR_H
#define ISRRAN_SCH_NR_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/fec/ldpc/ldpc_decoder.h"
#include "isrran/phy/fec/ldpc/ldpc_encoder.h"
#include "isrran/phy/fec/ldpc/ldpc_rm.h"
#include "isrran/phy/phch/phch_cfg_nr.h"

/**
 * @brief Maximum number of codeblocks for a NR shared channel transmission. It assumes a rate of 1.0 for the maximum
 * amount of bits a resource grid can fit
 */
#define ISRRAN_SCH_NR_MAX_NOF_CB_LDPC                                                                                  \
  ((ISRRAN_SLOT_MAX_NOF_BITS_NR + (ISRRAN_LDPC_MAX_LEN_CB - 1)) / ISRRAN_LDPC_MAX_LEN_CB)

/**
 * @brief Groups NR-PUSCH data for reception
 */
typedef struct {
  uint8_t* payload;  ///< SCH payload
  bool     crc;      ///< CRC match
  float    avg_iter; ///< Average iterations
} isrran_sch_tb_res_nr_t;

typedef struct ISRRAN_API {
  isrran_carrier_nr_t carrier;

  /// Temporal data buffers
  uint8_t* temp_cb;

  /// CRC generators
  isrran_crc_t crc_tb_24;
  isrran_crc_t crc_tb_16;
  isrran_crc_t crc_cb;

  /// LDPC encoders
  isrran_ldpc_encoder_t* encoder_bg1[MAX_LIFTSIZE + 1];
  isrran_ldpc_encoder_t* encoder_bg2[MAX_LIFTSIZE + 1];

  /// LDPC decoders
  isrran_ldpc_decoder_t* decoder_bg1[MAX_LIFTSIZE + 1];
  isrran_ldpc_decoder_t* decoder_bg2[MAX_LIFTSIZE + 1];

  /// LDPC Rate matcher
  isrran_ldpc_rm_t tx_rm;
  isrran_ldpc_rm_t rx_rm;
} isrran_sch_nr_t;

/**
 * @brief SCH encoder and decoder initialization arguments
 */
typedef struct ISRRAN_API {
  bool     disable_simd;
  bool     decoder_use_flooded;
  float    decoder_scaling_factor;
  uint32_t max_nof_iter; ///< Maximum number of LDPC iterations
} isrran_sch_nr_args_t;

/**
 * @brief Common SCH configuration
 */
typedef struct {
  isrran_basegraph_t bg;   ///< @brief Base graph
  uint32_t           Qm;   ///< @brief Modulation order
  uint32_t           G;    ///< Number of available bits
  uint32_t           A;    ///< @brief Payload size, TBS
  uint32_t           L_tb; ///< @brief the number of the transport block parity bits (16 or 24 bits)
  uint32_t           L_cb; ///< @brief the number of the code block parity bits (0 or 24 bits)
  uint32_t           B;    ///< @brief the number of bits in the transport block including TB CRC
  uint32_t           Bp;   ///< @brief the number of bits in the transport block including CB and TB CRCs
  uint32_t           Kp;   ///< @brief Number of payload bits of the code block including CB CRC
  uint32_t           Kr;   ///< @brief Number of payload bits of the code block including CB CRC and filler bits
  uint32_t           F;    ///< @brief Number of filler bits
  uint32_t           Nref; ///< @brief N_ref parameter described in TS 38.212 V15.9.0 5.4.2.1
  uint32_t           Z;    ///< @brief LDPC lifting size
  uint32_t           Nl;   ///< @brief Number of transmission layers that the transport block is mapped onto
  bool               mask[ISRRAN_SCH_NR_MAX_NOF_CB_LDPC]; ///< Indicates what codeblocks shall be encoded/decoded
  uint32_t           C;                                   ///< Number of codeblocks
  uint32_t           Cp;                                  ///< Number of codeblocks that are actually transmitted
} isrran_sch_nr_tb_info_t;

/**
 * @brief Base graph selection from a provided transport block size and target rate
 *
 * @remark Defined for DL-SCH in TS 38.212 V15.9.0 section 7.2.2 LDPC base graph selection
 * @remark Defined for UL-SCH in TS 38.212 V15.9.0 section 6.2.2 LDPC base graph selection
 *
 * @param tbs the payload size as described in Clause 7.2.1 for DL-SCH and 6.2.1 for UL-SCH.
 * @param R Target rate
 * @return it returns the selected base graph
 */
ISRRAN_API isrran_basegraph_t isrran_sch_nr_select_basegraph(uint32_t tbs, double R);

/**
 * @brief Calculates all the parameters required for performing TS 38.212 V15.9.0 5.4 General procedures for LDPC
 * @param sch_cfg Provides higher layers configuration
 * @param tb Provides transport block configuration
 * @param cfg SCH object
 * @return
 */
ISRRAN_API int isrran_sch_nr_fill_tb_info(const isrran_carrier_nr_t* carrier,
                                          const isrran_sch_cfg_t*    sch_cfg,
                                          const isrran_sch_tb_t*     tb,
                                          isrran_sch_nr_tb_info_t*   cfg);

/**
 * @brief Initialises an SCH object as transmitter
 * @param q Points ats the SCH object
 * @param args Provides static configuration arguments
 * @return ISRRAN_SUCCESS if the initialization is successful, ISRRAN_ERROR otherwise
 */
ISRRAN_API int isrran_sch_nr_init_tx(isrran_sch_nr_t* q, const isrran_sch_nr_args_t* args);

/**
 * @brief Initialises an SCH object as receiver
 * @param q Points ats the SCH object
 * @param args Provides static configuration arguments
 * @return ISRRAN_SUCCESS if the initialization is successful, ISRRAN_ERROR otherwise
 */
ISRRAN_API int isrran_sch_nr_init_rx(isrran_sch_nr_t* q, const isrran_sch_nr_args_t* args);

/**
 * @brief Sets SCH object carrier attribute
 * @param q Points ats the SCH object
 * @param carrier Provides the NR carrier object
 * @return ISRRAN_SUCCESS if the setting is successful, ISRRAN_ERROR otherwise
 */
ISRRAN_API int isrran_sch_nr_set_carrier(isrran_sch_nr_t* q, const isrran_carrier_nr_t* carrier);

/**
 * @brief Free allocated resources used by an SCH intance
 * @param q Points ats the SCH object
 */
ISRRAN_API void isrran_sch_nr_free(isrran_sch_nr_t* q);

ISRRAN_API int isrran_dlsch_nr_encode(isrran_sch_nr_t*        q,
                                      const isrran_sch_cfg_t* cfg,
                                      const isrran_sch_tb_t*  tb,
                                      const uint8_t*          data,
                                      uint8_t*                e_bits);

ISRRAN_API int isrran_dlsch_nr_decode(isrran_sch_nr_t*        q,
                                      const isrran_sch_cfg_t* sch_cfg,
                                      const isrran_sch_tb_t*  tb,
                                      int8_t*                 e_bits,
                                      isrran_sch_tb_res_nr_t* res);

ISRRAN_API int isrran_ulsch_nr_encode(isrran_sch_nr_t*        q,
                                      const isrran_sch_cfg_t* cfg,
                                      const isrran_sch_tb_t*  tb,
                                      const uint8_t*          data,
                                      uint8_t*                e_bits);

ISRRAN_API int isrran_ulsch_nr_decode(isrran_sch_nr_t*        q,
                                      const isrran_sch_cfg_t* sch_cfg,
                                      const isrran_sch_tb_t*  tb,
                                      int8_t*                 e_bits,
                                      isrran_sch_tb_res_nr_t* res);

ISRRAN_API int
isrran_sch_nr_tb_info(const isrran_sch_tb_t* tb, const isrran_sch_tb_res_nr_t* res, char* str, uint32_t str_len);

#endif // ISRRAN_SCH_NR_H