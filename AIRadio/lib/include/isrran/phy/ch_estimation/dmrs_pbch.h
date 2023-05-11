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

#ifndef ISRRAN_DMRS_PBCH_H
#define ISRRAN_DMRS_PBCH_H

#include "isrran/phy/common/phy_common_nr.h"

/**
 * @brief Describes the DeModulation Reference Signals (DMRS) for NR PBCH configuration
 */
typedef struct ISRRAN_API {
  uint32_t                    N_id;    ///< Physical cell identifier
  uint32_t                    n_hf;    ///< Number of half radio frame, 0 or 1
  uint32_t                    ssb_idx; ///< SSB candidate index, up to 3 LSB are significant
  uint32_t                    L_max;   ///< Number of SSB opportunities in half radio frame
  float                       beta;    ///< Power allocation specified in TS 38.213
  isrran_subcarrier_spacing_t scs;     ///< SSB configured subcarrier spacing
} isrran_dmrs_pbch_cfg_t;

/**
 * @brief Describes an NR PBCH DMRS based measurement
 */
typedef struct ISRRAN_API {
  float corr;         ///< Normalised correlation
  float epre;         ///< Linear energy per resource element
  float rsrp;         ///< Linear RSRP
  float cfo_hz;       ///< CFO in Hz
  float avg_delay_us; ///< Average delay in us
} isrran_dmrs_pbch_meas_t;

/**
 * @brief Put NR PBCH DMRS in the SSB resource grid
 * @param cfg PBCH DMRS configuration
 * @param[out] ssb_grid SSB resource grid
 * @return ISRRAN_SUCCESS if the inputs and configuration are valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_dmrs_pbch_put(const isrran_dmrs_pbch_cfg_t* cfg, cf_t ssb_grid[ISRRAN_SSB_NOF_RE]);

/**
 * @brief Measures NR PBCH DMRS
 * @param cfg PBCH DMRS configuration
 * @param ssb_grid SSB resource grid
 * @param[out] meas Measurement
 * @return ISRRAN_SUCCESS if the inputs and configuration are valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_dmrs_pbch_measure(const isrran_dmrs_pbch_cfg_t* cfg,
                                        const cf_t                    ssb_grid[ISRRAN_SSB_NOF_RE],
                                        isrran_dmrs_pbch_meas_t*      meas);

/**
 * @brief Estimates NR PBCH DMRS
 * @param cfg PBCH DMRS configuration
 * @param ssb_grid Demodulated SSB resource grid
 * @param[out] ce Estimated channel
 * @return ISRRAN_SUCCESS if the inputs and configuration are valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_dmrs_pbch_estimate(const isrran_dmrs_pbch_cfg_t* cfg,
                                         const cf_t                    ssb_grid[ISRRAN_SSB_NOF_RE],
                                         cf_t                          ce[ISRRAN_SSB_NOF_RE]);

#endif // ISRRAN_DMRS_PBCH_H
