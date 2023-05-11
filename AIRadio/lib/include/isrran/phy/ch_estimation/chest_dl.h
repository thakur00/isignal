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
 *  File:         chest_dl.h
 *
 *  Description:  3GPP LTE Downlink channel estimator and equalizer.
 *                Estimates the channel in the resource elements transmitting references and
 *                interpolates for the rest of the resource grid.
 *                The equalizer uses the channel estimates to produce an estimation of the
 *                transmitted symbol.
 *                This object depends on the isrran_refsignal_t object for creating the LTE
 *                CSR signal.
 *
 *  Reference:
 *********************************************************************************************/

#ifndef ISRRAN_CHEST_DL_H
#define ISRRAN_CHEST_DL_H

#include <stdio.h>

#include "isrran/config.h"

#include "isrran/phy/ch_estimation/chest_common.h"
#include "isrran/phy/ch_estimation/refsignal_dl.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/resampling/interp.h"
#include "isrran/phy/sync/pss.h"
#include "wiener_dl.h"

typedef struct ISRRAN_API {
  cf_t*    ce[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  uint32_t nof_re;
  float    noise_estimate;
  float    noise_estimate_dbm;
  float    snr_db;
  float    snr_ant_port_db[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  float    rsrp;
  float    rsrp_dbm;
  float    rsrp_neigh;
  float    rsrp_port_dbm[ISRRAN_MAX_PORTS];
  float    rsrp_ant_port_dbm[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  float    rsrq;
  float    rsrq_db;
  float    rsrq_ant_port_db[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  float    rssi_dbm;
  float    cfo;
  float    sync_error;
} isrran_chest_dl_res_t;

// Noise estimation algorithm
typedef enum ISRRAN_API {
  ISRRAN_NOISE_ALG_REFS = 0,
  ISRRAN_NOISE_ALG_PSS,
  ISRRAN_NOISE_ALG_EMPTY,
} isrran_chest_dl_noise_alg_t;

// Channel estimator algorithm
typedef enum ISRRAN_API {
  ISRRAN_ESTIMATOR_ALG_AVERAGE = 0,
  ISRRAN_ESTIMATOR_ALG_INTERPOLATE,
  ISRRAN_ESTIMATOR_ALG_WIENER,
} isrran_chest_dl_estimator_alg_t;

typedef struct ISRRAN_API {
  isrran_cell_t cell;
  uint32_t      nof_rx_antennas;

  isrran_refsignal_t   csr_refs;
  isrran_refsignal_t** mbsfn_refs;

  isrran_wiener_dl_t* wiener_dl;

  cf_t* pilot_estimates;
  cf_t* pilot_estimates_average;
  cf_t* pilot_recv_signal;
  cf_t* tmp_noise;
  cf_t* tmp_cfo_estimate;

#ifdef FREQ_SEL_SNR
  float snr_vector[12000];
  float pilot_power[12000];
#endif

  isrran_interp_linisrran_vec_t isrran_interp_linvec;
  isrran_interp_lin_t           isrran_interp_lin;
  isrran_interp_lin_t           isrran_interp_lin_3;
  isrran_interp_lin_t           isrran_interp_lin_mbsfn;

  float rssi[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  float rsrp[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  float rsrp_corr[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  float noise_estimate[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  float sync_err[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  float cfo;

  /* Use PSS for noise estimation in LS linear interpolation mode */
  cf_t pss_signal[ISRRAN_PSS_LEN];
  cf_t tmp_pss[ISRRAN_PSS_LEN];
  cf_t tmp_pss_noisy[ISRRAN_PSS_LEN];

} isrran_chest_dl_t;

typedef struct ISRRAN_API {
  isrran_chest_dl_estimator_alg_t estimator_alg;
  isrran_chest_dl_noise_alg_t     noise_alg;

  isrran_chest_filter_t       filter_type;
  float                       filter_coef[2];

  uint16_t mbsfn_area_id;
  bool     rsrp_neighbour;
  bool     cfo_estimate_enable;
  uint32_t cfo_estimate_sf_mask;
  bool     sync_error_enable;

} isrran_chest_dl_cfg_t;

ISRRAN_API int isrran_chest_dl_init(isrran_chest_dl_t* q, uint32_t max_prb, uint32_t nof_rx_antennas);

ISRRAN_API void isrran_chest_dl_free(isrran_chest_dl_t* q);

ISRRAN_API int isrran_chest_dl_res_init(isrran_chest_dl_res_t* q, uint32_t max_prb);
ISRRAN_API int isrran_chest_dl_res_init_re(isrran_chest_dl_res_t* q, uint32_t nof_re);

ISRRAN_API void isrran_chest_dl_res_set_identity(isrran_chest_dl_res_t* q);

ISRRAN_API void isrran_chest_dl_res_set_ones(isrran_chest_dl_res_t* q);

ISRRAN_API void isrran_chest_dl_res_free(isrran_chest_dl_res_t* q);

/* These functions change the internal object state */

ISRRAN_API int isrran_chest_dl_set_mbsfn_area_id(isrran_chest_dl_t* q, uint16_t mbsfn_area_id);

ISRRAN_API int isrran_chest_dl_set_cell(isrran_chest_dl_t* q, isrran_cell_t cell);

/* These functions do not change the internal state */

ISRRAN_API int isrran_chest_dl_estimate(isrran_chest_dl_t*     q,
                                        isrran_dl_sf_cfg_t*    sf,
                                        cf_t*                  input[ISRRAN_MAX_PORTS],
                                        isrran_chest_dl_res_t* res);

ISRRAN_API int isrran_chest_dl_estimate_cfg(isrran_chest_dl_t*     q,
                                            isrran_dl_sf_cfg_t*    sf,
                                            isrran_chest_dl_cfg_t* cfg,
                                            cf_t*                  input[ISRRAN_MAX_PORTS],
                                            isrran_chest_dl_res_t* res);

ISRRAN_API isrran_chest_dl_estimator_alg_t isrran_chest_dl_str2estimator_alg(const char* str);

#endif // ISRRAN_CHEST_DL_H
