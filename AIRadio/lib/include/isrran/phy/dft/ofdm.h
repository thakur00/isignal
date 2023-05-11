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

#ifndef ISRRAN_OFDM_H
#define ISRRAN_OFDM_H

/**********************************************************************************************
 *  File:         ofdm.h
 *
 *  Description:  OFDM modulation object.
 *                Used in generation of downlink OFDM signals.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6
 *********************************************************************************************/

#include <strings.h>

#include "isrran/config.h"
#include "isrran/phy/cfr/cfr.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/dft.h"

/**
 * @struct isrran_ofdm_cfg_t
 * Contains the generic OFDM modulator configuration. The structure must be initialised to all zeros before being
 * filled. Only compulsory parameters need to be filled prior initialization.
 *
 * This structure must be used with init functions isrran_ofdm_rx_init_cfg and isrran_ofdm_tx_init_cfg. These provide
 * more flexible options.
 */
typedef struct ISRRAN_API {
  // Compulsory parameters
  uint32_t    nof_prb;    ///< Number of Resource Block
  cf_t*       in_buffer;  ///< Input buffer pointer
  cf_t*       out_buffer; ///< Output buffer pointer
  isrran_cp_t cp;         ///< Cyclic prefix type

  // Optional parameters
  isrran_sf_t      sf_type;          ///< Subframe type, normal or MBSFN
  bool             normalize;        ///< Normalization flag, it divides the output by square root of the symbol size
  float            freq_shift_f;     ///< Frequency shift, normalised by sampling rate (used in UL)
  float            rx_window_offset; ///< DFT Window offset in CP portion (0-1), RX only
  uint32_t         symbol_sz;        ///< Symbol size, forces a given symbol size for the number of PRB
  bool             keep_dc;          ///< If true, it does not remove the DC
  double           phase_compensation_hz; ///< Carrier frequency in Hz for phase compensation, set to 0 to disable
  isrran_cfr_cfg_t cfr_tx_cfg;            ///< Tx CFR configuration
} isrran_ofdm_cfg_t;

/**
 * @struct isrran_ofdm_t
 * OFDM object, common for Tx and Rx
 */
typedef struct ISRRAN_API {
  isrran_ofdm_cfg_t cfg;
  isrran_dft_plan_t fft_plan;
  isrran_dft_plan_t fft_plan_sf[2];
  uint32_t          max_prb;
  uint32_t          nof_symbols;
  uint32_t          nof_guards;
  uint32_t          nof_re;
  uint32_t          slot_sz;
  uint32_t          sf_sz;
  cf_t*             tmp; // for removing zero padding
  bool              mbsfn_subframe;
  uint32_t          mbsfn_guard_len;
  uint32_t          nof_symbols_mbsfn;
  uint8_t           non_mbsfn_region;
  uint32_t          window_offset_n;
  cf_t*             shift_buffer;
  cf_t*             window_offset_buffer;
  cf_t              phase_compensation[ISRRAN_MAX_NSYMB * ISRRAN_NOF_SLOTS_PER_SF];
  isrran_cfr_t      tx_cfr; ///< Tx CFR object
} isrran_ofdm_t;

/**
 * @brief Initialises or reconfigures OFDM receiver
 *
 * @note The reconfiguration of the OFDM object considers only CP, number of PRB and optionally the FFT size
 * @attention The OFDM object must be zeroed externally prior calling the initialization for first time
 *
 * @param q OFDM object
 * @param cfg OFDM configuration
 * @return ISRRAN_SUCCESS if the initialization/reconfiguration is successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ofdm_rx_init_cfg(isrran_ofdm_t* q, isrran_ofdm_cfg_t* cfg);

/**
 * @brief Initialises or reconfigures OFDM transmitter
 *
 * @note The reconfiguration of the OFDM object considers only CP, number of PRB and optionally the FFT size
 * @attention The OFDM object must be zeroed externally prior calling the initialization for first time
 *
 * @param q OFDM object
 * @param cfg OFDM configuration
 * @return ISRRAN_SUCCESS if the initialization/reconfiguration is successful, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ofdm_tx_init_cfg(isrran_ofdm_t* q, isrran_ofdm_cfg_t* cfg);

ISRRAN_API int
isrran_ofdm_rx_init_mbsfn(isrran_ofdm_t* q, isrran_cp_t cp_type, cf_t* in_buffer, cf_t* out_buffer, uint32_t max_prb);

ISRRAN_API int
isrran_ofdm_rx_init(isrran_ofdm_t* q, isrran_cp_t cp_type, cf_t* in_buffer, cf_t* out_buffer, uint32_t max_prb);

ISRRAN_API int isrran_ofdm_tx_set_prb(isrran_ofdm_t* q, isrran_cp_t cp, uint32_t nof_prb);

ISRRAN_API int isrran_ofdm_rx_set_prb(isrran_ofdm_t* q, isrran_cp_t cp, uint32_t nof_prb);

ISRRAN_API void isrran_ofdm_rx_free(isrran_ofdm_t* q);

ISRRAN_API void isrran_ofdm_rx_sf(isrran_ofdm_t* q);

ISRRAN_API void isrran_ofdm_rx_sf_ng(isrran_ofdm_t* q, cf_t* input, cf_t* output);

ISRRAN_API int
isrran_ofdm_tx_init(isrran_ofdm_t* q, isrran_cp_t cp_type, cf_t* in_buffer, cf_t* out_buffer, uint32_t nof_prb);

ISRRAN_API int
isrran_ofdm_tx_init_mbsfn(isrran_ofdm_t* q, isrran_cp_t cp, cf_t* in_buffer, cf_t* out_buffer, uint32_t nof_prb);

ISRRAN_API void isrran_ofdm_tx_free(isrran_ofdm_t* q);

ISRRAN_API void isrran_ofdm_tx_sf(isrran_ofdm_t* q);

ISRRAN_API int isrran_ofdm_set_freq_shift(isrran_ofdm_t* q, float freq_shift);

ISRRAN_API void isrran_ofdm_set_normalize(isrran_ofdm_t* q, bool normalize_enable);

ISRRAN_API int isrran_ofdm_set_phase_compensation(isrran_ofdm_t* q, double center_freq_hz);

ISRRAN_API void isrran_ofdm_set_non_mbsfn_region(isrran_ofdm_t* q, uint8_t non_mbsfn_region);

ISRRAN_API int isrran_ofdm_set_cfr(isrran_ofdm_t* q, isrran_cfr_cfg_t* cfr);

#endif // ISRRAN_OFDM_H
