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

#ifndef ISRRAN_CHEST_DL_NBIOT_H
#define ISRRAN_CHEST_DL_NBIOT_H

#include <stdio.h>

#include "isrran/config.h"

#include "isrran/phy/ch_estimation/chest_common.h"
#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/ch_estimation/refsignal_dl_nbiot.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/resampling/interp.h"

/*! \brief Downlink channel estimation for NB-IoT
 *
 * Estimates the channel in the resource elements transmitting references and
 * interpolates for the rest of the resource grid.
 * The equalizer uses the channel estimates to produce an estimation of the
 * transmitted symbol.
 * This object depends on the isrran_refsignal_t object for creating the LTE CSR signal.
 */
typedef struct {
  isrran_nbiot_cell_t         cell;
  isrran_refsignal_dl_nbiot_t nrs_signal;

  cf_t* pilot_estimates;
  cf_t* pilot_estimates_average;
  cf_t* pilot_recv_signal;
  cf_t* tmp_noise;

  uint32_t smooth_filter_len;
  float    smooth_filter[ISRRAN_CHEST_MAX_SMOOTH_FIL_LEN];

  isrran_interp_linisrran_vec_t isrran_interp_linvec;
  isrran_interp_lin_t           isrran_interp_lin;

  float rssi[ISRRAN_MAX_PORTS];
  float rsrp[ISRRAN_MAX_PORTS];
  float noise_estimate[ISRRAN_MAX_PORTS];

  isrran_chest_dl_noise_alg_t noise_alg;

} isrran_chest_dl_nbiot_t;

ISRRAN_API int isrran_chest_dl_nbiot_init(isrran_chest_dl_nbiot_t* q, uint32_t max_prb);

ISRRAN_API void isrran_chest_dl_nbiot_free(isrran_chest_dl_nbiot_t* q);

ISRRAN_API int isrran_chest_dl_nbiot_set_cell(isrran_chest_dl_nbiot_t* q, isrran_nbiot_cell_t cell);

ISRRAN_API void isrran_chest_dl_nbiot_set_smooth_filter(isrran_chest_dl_nbiot_t* q, float* filter, uint32_t filter_len);

ISRRAN_API void isrran_chest_dl_nbiot_set_smooth_filter3_coeff(isrran_chest_dl_nbiot_t* q, float w);

ISRRAN_API void isrran_chest_dl_nbiot_set_noise_alg(isrran_chest_dl_nbiot_t*    q,
                                                    isrran_chest_dl_noise_alg_t noise_estimation_alg);

ISRRAN_API int isrran_chest_dl_nbiot_estimate(isrran_chest_dl_nbiot_t* q, cf_t* input, cf_t** ce, uint32_t sf_idx);

ISRRAN_API int isrran_chest_dl_nbiot_estimate_port(isrran_chest_dl_nbiot_t* q,
                                                   cf_t*                    input,
                                                   cf_t*                    ce,
                                                   uint32_t                 sf_idx,
                                                   uint32_t                 port_id);

ISRRAN_API float isrran_chest_dl_nbiot_get_noise_estimate(isrran_chest_dl_nbiot_t* q);

ISRRAN_API float isrran_chest_dl_nbiot_get_snr(isrran_chest_dl_nbiot_t* q);

ISRRAN_API float isrran_chest_dl_nbiot_get_rssi(isrran_chest_dl_nbiot_t* q);

ISRRAN_API float isrran_chest_dl_nbiot_get_rsrq(isrran_chest_dl_nbiot_t* q);

ISRRAN_API float isrran_chest_dl_nbiot_get_rsrp(isrran_chest_dl_nbiot_t* q);

#endif // ISRRAN_CHEST_DL_NBIOT_H
