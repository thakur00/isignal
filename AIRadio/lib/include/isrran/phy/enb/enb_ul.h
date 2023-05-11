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
 *  File:         enb_ul.h
 *
 *  Description:  ENB uplink object.
 *
 *                This module is a frontend to all the uplink data and control
 *                channel processing modules for the ENB receiver side.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_ENB_UL_H
#define ISRRAN_ENB_UL_H

#include <stdbool.h>

#include "isrran/phy/ch_estimation/chest_ul.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/prach.h"
#include "isrran/phy/phch/pucch.h"
#include "isrran/phy/phch/pusch.h"
#include "isrran/phy/phch/pusch_cfg.h"
#include "isrran/phy/phch/ra.h"

#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

#include "isrran/config.h"

typedef struct ISRRAN_API {
  isrran_cell_t cell;

  cf_t*                 sf_symbols;
  cf_t*                 in_buffer;
  isrran_chest_ul_res_t chest_res;

  isrran_ofdm_t     fft;
  isrran_chest_ul_t chest;
  isrran_pusch_t    pusch;
  isrran_pucch_t    pucch;

} isrran_enb_ul_t;

/* This function shall be called just after the initial synchronization */
ISRRAN_API int isrran_enb_ul_init(isrran_enb_ul_t* q, cf_t* in_buffer, uint32_t max_prb);

ISRRAN_API void isrran_enb_ul_free(isrran_enb_ul_t* q);

ISRRAN_API int isrran_enb_ul_set_cell(isrran_enb_ul_t*                   q,
                                      isrran_cell_t                      cell,
                                      isrran_refsignal_dmrs_pusch_cfg_t* pusch_cfg,
                                      isrran_refsignal_isr_cfg_t*        isr_cfg);

ISRRAN_API void isrran_enb_ul_fft(isrran_enb_ul_t* q);

ISRRAN_API int isrran_enb_ul_get_pucch(isrran_enb_ul_t*    q,
                                       isrran_ul_sf_cfg_t* ul_sf,
                                       isrran_pucch_cfg_t* cfg,
                                       isrran_pucch_res_t* res);

ISRRAN_API int isrran_enb_ul_get_pusch(isrran_enb_ul_t*    q,
                                       isrran_ul_sf_cfg_t* ul_sf,
                                       isrran_pusch_cfg_t* cfg,
                                       isrran_pusch_res_t* res);

#endif // ISRRAN_ENB_UL_H
