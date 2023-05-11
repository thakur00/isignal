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

#ifndef ISRRAN_PBCH_NR_H
#define ISRRAN_PBCH_NR_H

#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/fec/polar/polar_code.h"
#include "isrran/phy/fec/polar/polar_decoder.h"
#include "isrran/phy/fec/polar/polar_encoder.h"
#include "isrran/phy/fec/polar/polar_rm.h"
#include "isrran/phy/modem/modem_table.h"
#include "isrran/phy/phch/pbch_msg_nr.h"

/**
 * @brief Describes the NR PBCH object initialisation arguments
 */
typedef struct ISRRAN_API {
  bool enable_encode; ///< Enable encoder
  bool enable_decode; ///< Enable decoder
  bool disable_simd;  ///< Disable SIMD polar encoder/decoder
} isrran_pbch_nr_args_t;

/**
 * @brief Describes the NR PBCH configuration
 */
typedef struct ISRRAN_API {
  uint32_t N_id;    ///< Physical cell identifier
  uint32_t n_hf;    ///< Number of half radio frame, 0 or 1
  uint32_t ssb_idx; ///< SSB candidate index, up to 4 LSB significant
  uint32_t Lmax;    ///< Number of SSB opportunities, described in TS 38.213 4.1 ...
  float    beta;    ///< Scaling factor for PBCH symbols, set to zero for default
} isrran_pbch_nr_cfg_t;

/**
 * @brief Describes the NR PBCH object initialisation arguments
 */
typedef struct ISRRAN_API {
  isrran_polar_code_t    code;
  isrran_polar_encoder_t polar_encoder;
  isrran_polar_decoder_t polar_decoder;
  isrran_polar_rm_t      polar_rm_tx;
  isrran_polar_rm_t      polar_rm_rx;
  isrran_crc_t           crc;
  isrran_modem_table_t   qpsk;
} isrran_pbch_nr_t;

/**
 * @brief Initialises an NR PBCH object with the provided arguments
 * @param q NR PBCH object
 * @param args Arguments providing the desired configuration
 * @return ISRRAN_SUCCESS if initialization is successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pbch_nr_init(isrran_pbch_nr_t* q, const isrran_pbch_nr_args_t* args);

/**
 * @brief Deallocates an NR PBCH object
 * @param q NR PBCH object
 */
ISRRAN_API void isrran_pbch_nr_free(isrran_pbch_nr_t* q);

/**
 * @brief Encodes an NR PBCH message into a SSB resource grid
 * @param q NR PBCH object
 * @param cfg NR PBCH configuration
 * @param msg NR PBCH message to transmit
 * @param[out] ssb_grid SSB resource grid
 * @return ISRRAN_SUCCESS if encoding is successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pbch_nr_encode(isrran_pbch_nr_t*           q,
                                     const isrran_pbch_nr_cfg_t* cfg,
                                     const isrran_pbch_msg_nr_t* msg,
                                     cf_t                        ssb_grid[ISRRAN_SSB_NOF_RE]);

/**
 * @brief Decodes an NR PBCH message in the SSB resource grid
 * @param q NR PBCH object
 * @param cfg NR PBCH configuration
 * @param[in] ce Channel estimates for the SSB resource grid
 * @param msg NR PBCH message received
 * @return ISRRAN_SUCCESS if decoding is successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_pbch_nr_decode(isrran_pbch_nr_t*           q,
                                     const isrran_pbch_nr_cfg_t* cfg,
                                     const cf_t                  ssb_grid[ISRRAN_SSB_NOF_RE],
                                     const cf_t                  ce[ISRRAN_SSB_NOF_RE],
                                     isrran_pbch_msg_nr_t*       msg);

#endif // ISRRAN_PBCH_NR_H
