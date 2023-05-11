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

#ifndef ISRRAN_CHEST_SL_H
#define ISRRAN_CHEST_SL_H

#include <stdio.h>

#include "isrran/phy/common/phy_common_sl.h"
#include "isrran/phy/resampling/interp.h"

#define ISRRAN_SL_N_RU_SEQ (30)
#define ISRRAN_SL_MAX_DMRS_SYMB (4)
#define ISRRAN_SL_DEFAULT_NOF_DMRS_CYCLIC_SHIFTS (1)
#define ISRRAN_SL_MAX_PSCCH_NOF_DMRS_CYCLIC_SHIFTS (4)

// Base Sequence Number - always 0 for sidelink: 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 5.5.1.4
#define ISRRAN_SL_BASE_SEQUENCE_NUMBER 0
#define ISRRAN_SL_MAX_DMRS_PERIOD_LENGTH 320

typedef struct ISRRAN_API {
  uint32_t prb_start_idx; // PRB start idx to map RE from RIV
  uint32_t nof_prb;       // PSSCH nof_prb, Length of continuous PRB to map RE (in the pool) from RIV
  uint32_t N_x_id;
  uint32_t sf_idx; // PSSCH sf_idx
  uint32_t cyclic_shift;
} isrran_chest_sl_cfg_t;

typedef struct ISRRAN_API {
  isrran_sl_channels_t           channel;
  isrran_cell_sl_t               cell;
  isrran_sl_comm_resource_pool_t sl_comm_resource_pool;
  isrran_chest_sl_cfg_t          chest_sl_cfg;

  uint32_t sf_n_re;

  uint32_t M_sc_rs;
  int8_t   nof_dmrs_symbols;

  // Orthogonal Sequence (W) Transmission Mode 1, 2 and PSBCH
  int8_t w[ISRRAN_SL_MAX_DMRS_SYMB];

  // Cyclic Shift Values
  int8_t n_CS[ISRRAN_SL_MAX_DMRS_SYMB];

  // Reference Signal Cyclic Shift
  float alpha[ISRRAN_SL_MAX_DMRS_SYMB];

  // Group Hopping Flag
  uint32_t* f_gh_pattern;

  cf_t* r_sequence[ISRRAN_SL_MAX_DMRS_SYMB][ISRRAN_SL_MAX_PSCCH_NOF_DMRS_CYCLIC_SHIFTS];

  cf_t* r_sequence_rx[ISRRAN_SL_MAX_DMRS_SYMB];

  cf_t* ce;
  cf_t* ce_average;
  cf_t* noise_tmp;
  float noise_estimated;

  isrran_interp_linisrran_vec_t lin_vec_sl;

  bool  sync_error_enable;
  bool  rsrp_enable;
  float sync_err;
  float rsrp_corr;

} isrran_chest_sl_t;

ISRRAN_API int isrran_chest_sl_init(isrran_chest_sl_t*                    q,
                                    isrran_sl_channels_t                  channel,
                                    isrran_cell_sl_t                      cell,
                                    const isrran_sl_comm_resource_pool_t* sl_comm_resource_pool);

ISRRAN_API int isrran_chest_sl_set_cell(isrran_chest_sl_t* q, isrran_cell_sl_t cell);

ISRRAN_API int isrran_chest_sl_set_cfg(isrran_chest_sl_t* q, isrran_chest_sl_cfg_t chest_sl_cfg);

ISRRAN_API float isrran_chest_sl_get_sync_error(isrran_chest_sl_t* q);

ISRRAN_API float isrran_chest_sl_estimate_noise(isrran_chest_sl_t* q);

ISRRAN_API float isrran_chest_sl_get_rsrp(isrran_chest_sl_t* q);

ISRRAN_API int isrran_chest_sl_put_dmrs(isrran_chest_sl_t* q, cf_t* sf_buffer);

ISRRAN_API int isrran_chest_sl_get_dmrs(isrran_chest_sl_t* q, cf_t* sf_buffer, cf_t** dmrs_received);

ISRRAN_API void isrran_chest_sl_ls_estimate(isrran_chest_sl_t* q, cf_t* sf_buffer);

ISRRAN_API void isrran_chest_sl_ls_equalize(isrran_chest_sl_t* q, cf_t* sf_buffer, cf_t* equalized_sf_buffer);

ISRRAN_API void isrran_chest_sl_ls_estimate_equalize(isrran_chest_sl_t* q, cf_t* sf_buffer, cf_t* equalized_sf_buffer);

ISRRAN_API void isrran_chest_sl_free(isrran_chest_sl_t* q);

#endif