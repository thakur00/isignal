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

/**********************************************************************************************
 *  File:         chest_ul.h
 *
 *  Description:  3GPP LTE Uplink channel estimator and equalizer.
 *                Estimates the channel in the resource elements transmitting references and
 *                interpolates for the rest of the resource grid.
 *                The equalizer uses the channel estimates to produce an estimation of the
 *                transmitted symbol.
 *
 *  Reference:
 *********************************************************************************************/

#ifndef ISRRAN_CHEST_UL_H
#define ISRRAN_CHEST_UL_H

#include <stdio.h>

#include "isrran/config.h"

#include "isrran/phy/ch_estimation/chest_common.h"
#include "isrran/phy/ch_estimation/refsignal_ul.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/phch/pucch_cfg.h"
#include "isrran/phy/phch/pusch_cfg.h"
#include "isrran/phy/resampling/interp.h"

typedef struct ISRRAN_API {
  cf_t*    ce;
  uint32_t nof_re;
  float    noise_estimate;
  float    noise_estimate_dbFs;
  float    rsrp;
  float    rsrp_dBfs;
  float    epre;
  float    epre_dBfs;
  float    snr;
  float    snr_db;
  float    cfo_hz;
  float    ta_us;
} isrran_chest_ul_res_t;

typedef struct {
  isrran_cell_t cell;

  isrran_refsignal_ul_t             dmrs_signal;
  isrran_refsignal_ul_dmrs_pregen_t dmrs_pregen;
  bool                              dmrs_signal_configured;

  isrran_refsignal_isr_pregen_t isr_pregen;
  bool                          isr_signal_configured;

  cf_t* pilot_estimates;
  cf_t* pilot_estimates_tmp[4];
  cf_t* pilot_recv_signal;
  cf_t* pilot_known_signal;
  cf_t* tmp_noise;

#ifdef FREQ_SEL_SNR
  float snr_vector[12000];
  float pilot_power[12000];
#endif
  uint32_t smooth_filter_len;
  float    smooth_filter[ISRRAN_CHEST_MAX_SMOOTH_FIL_LEN];

  isrran_interp_linisrran_vec_t isrran_interp_linvec;

} isrran_chest_ul_t;

ISRRAN_API int isrran_chest_ul_init(isrran_chest_ul_t* q, uint32_t max_prb);

ISRRAN_API void isrran_chest_ul_free(isrran_chest_ul_t* q);

ISRRAN_API int isrran_chest_ul_res_init(isrran_chest_ul_res_t* q, uint32_t max_prb);

ISRRAN_API void isrran_chest_ul_res_set_identity(isrran_chest_ul_res_t* q);

ISRRAN_API void isrran_chest_ul_res_free(isrran_chest_ul_res_t* q);

ISRRAN_API int isrran_chest_ul_set_cell(isrran_chest_ul_t* q, isrran_cell_t cell);

ISRRAN_API void isrran_chest_ul_pregen(isrran_chest_ul_t*                 q,
                                       isrran_refsignal_dmrs_pusch_cfg_t* cfg,
                                       isrran_refsignal_isr_cfg_t*        isr_cfg);

ISRRAN_API int isrran_chest_ul_estimate_pusch(isrran_chest_ul_t*     q,
                                              isrran_ul_sf_cfg_t*    sf,
                                              isrran_pusch_cfg_t*    cfg,
                                              cf_t*                  input,
                                              isrran_chest_ul_res_t* res);

ISRRAN_API int isrran_chest_ul_estimate_pucch(isrran_chest_ul_t*     q,
                                              isrran_ul_sf_cfg_t*    sf,
                                              isrran_pucch_cfg_t*    cfg,
                                              cf_t*                  input,
                                              isrran_chest_ul_res_t* res);

ISRRAN_API int isrran_chest_ul_estimate_isr(isrran_chest_ul_t*                 q,
                                            isrran_ul_sf_cfg_t*                sf,
                                            isrran_refsignal_isr_cfg_t*        cfg,
                                            isrran_refsignal_dmrs_pusch_cfg_t* pusch_cfg,
                                            cf_t*                              input,
                                            isrran_chest_ul_res_t*             res);

#endif // ISRRAN_CHEST_UL_H
