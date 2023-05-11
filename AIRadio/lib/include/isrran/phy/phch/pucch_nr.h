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

#ifndef ISRRAN_PUCCH_NR_H
#define ISRRAN_PUCCH_NR_H

#include "isrran/phy/ch_estimation/chest_ul.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/common/zc_sequence.h"
#include "isrran/phy/modem/modem_table.h"
#include "isrran/phy/phch/uci_nr.h"

/**
 * @brief Maximum number of symbols (without DMRS) that NR-PUCCH format 1 can transmit
 */
#define ISRRAN_PUCCH_NR_FORMAT1_N_MAX 7

typedef struct ISRRAN_API {
  isrran_uci_nr_args_t uci;
  uint32_t             max_nof_prb;
} isrran_pucch_nr_args_t;

typedef struct ISRRAN_API {
  float rsrp;
  float rsrp_dBfs;
  float epre;
  float epre_dBfs;
  float norm_corr;
} isrran_pucch_nr_measure_t;

/**
 * @brief NR-PUCCH encoder/decoder object
 */
typedef struct ISRRAN_API {
  uint32_t                 max_prb;
  isrran_carrier_nr_t      carrier;
  isrran_zc_sequence_lut_t r_uv_1prb;
  cf_t format1_w_i_m[ISRRAN_PUCCH_NR_FORMAT1_N_MAX][ISRRAN_PUCCH_NR_FORMAT1_N_MAX][ISRRAN_PUCCH_NR_FORMAT1_N_MAX];
  isrran_modem_table_t bpsk;
  isrran_modem_table_t qpsk;
  isrran_uci_nr_t      uci;
  uint8_t*             b;
  cf_t*                d;
  cf_t*                ce;
} isrran_pucch_nr_t;

/**
 * @brief Initialises an NR-PUCCH encoder/decoder object
 * @param q Object
 * @param args PUCCH configuration arguments
 * @return ISRRAN_SUCCESS if successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_init(isrran_pucch_nr_t* q, const isrran_pucch_nr_args_t* args);

/**
 * @brief Initialises an NR-PUCCH encoder/decoder object
 * @param q Object
 * @param carrier
 * @return ISRRAN_SUCCESS if successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_set_carrier(isrran_pucch_nr_t* q, const isrran_carrier_nr_t* carrier);

/**
 * @brief Deallocates an NR-PUCCH encoder/decoder object
 * @param q Object
 */
ISRRAN_API void isrran_pucch_nr_free(isrran_pucch_nr_t* q);

/**
 * @brief Computes the NR-PUCCH group sequence
 * @remark Implemented according to TS 38.211 clause 6.3.2.2.1 Group and sequence hopping
 * @param[in] carrier Serving cell and UL BWP configuration
 * @param[in] cfg PUCCH common configuration
 * @param[out] u_ Group sequence (u)
 * @param[out] v_ Base sequence (v)
 * @return ISRRAN_SUCCESS if provide arguments are right, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_group_sequence(const isrran_carrier_nr_t*          carrier,
                                              const isrran_pucch_nr_common_cfg_t* cfg,
                                              uint32_t*                           u_,
                                              uint32_t*                           v_);

/**
 * @brief Computes the NR alpha index (1-NRE)
 * @param[in] carrier Serving cell and UL BWP configuration
 * @param[in] cfg PUCCH common configuration
 * @param[in] slot slot configuration
 * @param[in] l OFDM Symbol, relative to the NR-PUCCH transmission start
 * @param[in] l_prime Initial OFDM symbol, relative to the transmission slot start
 * @param[in] m0 Initial cyclic shift
 * @param[in] m_cs Set to zero expect for format 0
 * @param[out] alpha_idx Computed alpha index
 * @return ISRRAN_SUCCESS if provide arguments are right, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_alpha_idx(const isrran_carrier_nr_t*          carrier,
                                         const isrran_pucch_nr_common_cfg_t* cfg,
                                         const isrran_slot_cfg_t*            slot,
                                         uint32_t                            l,
                                         uint32_t                            l_prime,
                                         uint32_t                            m0,
                                         uint32_t                            m_cs,
                                         uint32_t*                           alpha_idx);

/**
 * @brief Encode and writes NR-PUCCH format 0 in the resource grid
 * @remark Described in TS 38.211 clause 6.3.2.3 PUCCH format 0
 * @param[in,out] q NR-PUCCH encoder/decoder object
 * @param[in] cfg PUCCH common configuration
 * @param[in] slot slot configuration
 * @param[in] resource PUCCH format 0 resource
 * @param[in] m_cs Cyclic shift according to TS 38.213 clause 5
 * @param[out] slot_symbols Resource grid of the given slot
 * @return ISRRAN_SUCCESS if successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_format0_encode(const isrran_pucch_nr_t*            q,
                                              const isrran_pucch_nr_common_cfg_t* cfg,
                                              const isrran_slot_cfg_t*            slot,
                                              isrran_pucch_nr_resource_t*         resource,
                                              uint32_t                            m_cs,
                                              cf_t*                               slot_symbols);

/**
 * @brief Measures PUCCH format 0 in the resource grid
 * @param[in,out] q NR-PUCCH encoder/decoder object
 * @param[in] cfg PUCCH common configuration
 * @param[in] slot slot configuration
 * @param[in] resource PUCCH format 0 resource
 * @param[in] m_cs Cyclic shift according to TS 38.213 clause 5
 * @param[in] slot_symbols Resource grid of the given slot
 * @param[out] measure Measurement structure
 * @return ISRRAN_SUCCESS if successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_format0_measure(const isrran_pucch_nr_t*            q,
                                               const isrran_pucch_nr_common_cfg_t* cfg,
                                               const isrran_slot_cfg_t*            slot,
                                               isrran_pucch_nr_resource_t*         resource,
                                               uint32_t                            m_cs,
                                               const cf_t*                         slot_symbols,
                                               isrran_pucch_nr_measure_t*          measure);

/**
 * @brief Get NR-PUCCH orthogonal sequence w
 * @remark Defined by TS 38.211 Table 6.3.2.4.1-2: Orthogonal sequences ... for PUCCH format 1
 * @param[in,out] q NR-PUCCH encoder/decoder object
 * @param[in] n_pucch Number of PUCCH symbols
 * @param[in] i sequence index
 * @param m OFDM symbol index
 * @return Orthogonal sequence complex number if valid, NAN otherwise
 */
ISRRAN_API cf_t isrran_pucch_nr_format1_w(const isrran_pucch_nr_t* q, uint32_t n_pucch, uint32_t i, uint32_t m);

/**
 * @brief Encodes and puts NR-PUCCH format 1 in the resource grid
 * @remark Described in TS 38.211 clause 6.3.2.4 PUCCH format 1
 * @param[in,out] q NR-PUCCH encoder/decoder object
 * @param[in] cfg PUCCH common configuration
 * @param[in] slot slot configuration
 * @param[in] resource PUCCH format 1 resource
 * @param[in] b Bits to encode in the message
 * @param[in] nof_bits Number of bits to encode in the message
 * @param[out] slot_symbols Resource grid of the given slot
 * @return ISRRAN_SUCCESS if successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_format1_encode(const isrran_pucch_nr_t*            q,
                                              const isrran_pucch_nr_common_cfg_t* cfg,
                                              const isrran_slot_cfg_t*            slot,
                                              const isrran_pucch_nr_resource_t*   resource,
                                              uint8_t*                            b,
                                              uint32_t                            nof_bits,
                                              cf_t*                               slot_symbols);

/**
 * @brief Decodes NR-PUCCH format 1
 * @param[in,out] q NR-PUCCH encoder/decoder object
 * @param[in] cfg PUCCH common configuration
 * @param[in] slot slot configuration
 * @param[in] resource PUCCH format 2-4 resource
 * @param[in] chest_res Channel estimator result
 * @param[in] slot_symbols Resource grid of the given slot
 * @param[out] b Bits to decode
 * @param[in] nof_bits Number of bits to decode in the message
 * @param[out] norm_corr Normalised correlation
 * @return ISRRAN_SUCCESS if successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_format1_decode(isrran_pucch_nr_t*                  q,
                                              const isrran_pucch_nr_common_cfg_t* cfg,
                                              const isrran_slot_cfg_t*            slot,
                                              const isrran_pucch_nr_resource_t*   resource,
                                              isrran_chest_ul_res_t*              chest_res,
                                              cf_t*                               slot_symbols,
                                              uint8_t  b[ISRRAN_PUCCH_NR_FORMAT1_MAX_NOF_BITS],
                                              uint32_t nof_bits,
                                              float*   norm_corr);

/**
 * @brief Encoder NR-PUCCH formats 2, 3 and 4. The NR-PUCCH format is selected by resource->format.
 * @param[in,out] q NR-PUCCH encoder/decoder object
 * @param[in] cfg PUCCH common configuration
 * @param[in] slot slot configuration
 * @param[in] resource PUCCH format 1 resource
 * @param[in] uci_cfg Uplink Control Information configuration
 * @param[in] uci_value Uplink Control Information data
 * @param[out] slot_symbols Resource grid of the given slot
 * @return ISRRAN_SUCCESS if successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_format_2_3_4_encode(isrran_pucch_nr_t*                  q,
                                                   const isrran_pucch_nr_common_cfg_t* cfg,
                                                   const isrran_slot_cfg_t*            slot,
                                                   const isrran_pucch_nr_resource_t*   resource,
                                                   const isrran_uci_cfg_nr_t*          uci_cfg,
                                                   const isrran_uci_value_nr_t*        uci_value,
                                                   cf_t*                               slot_symbols);

/**
 * @brief Decode NR-PUCCH format 2, 3, and 4. The NR-PUCCH format is selected by resource->format.
 * @param q[in,out] q NR-PUCCH encoder/decoder
 * @param[in] cfg PUCCH common configuration
 * @param[in] slot slot configuration
 * @param[in] resource PUCCH format 2-4 resource
 * @param[in] uci_cfg Uplink Control Information configuration
 * @param[in] chest_res Channel estimator result
 * @param[in] slot_symbols Resource grid of the given slot
 * @param[out] uci_value Uplink Control Information data
 * @return ISRRAN_SUCCESS if successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pucch_nr_format_2_3_4_decode(isrran_pucch_nr_t*                  q,
                                                   const isrran_pucch_nr_common_cfg_t* cfg,
                                                   const isrran_slot_cfg_t*            slot,
                                                   const isrran_pucch_nr_resource_t*   resource,
                                                   const isrran_uci_cfg_nr_t*          uci_cfg,
                                                   isrran_chest_ul_res_t*              chest_res,
                                                   cf_t*                               slot_symbols,
                                                   isrran_uci_value_nr_t*              uci_value);

ISRRAN_API uint32_t isrran_pucch_nr_info(const isrran_pucch_nr_resource_t* resource,
                                         const isrran_uci_data_nr_t*       uci_data,
                                         char*                             str,
                                         uint32_t                          str_len);

#endif // ISRRAN_PUCCH_NR_H
