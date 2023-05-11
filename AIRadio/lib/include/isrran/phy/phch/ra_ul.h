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
 *  File:         ra_ul.h
 *
 *  Description:  Implements Resource allocation Procedures for UL defined in Sections 8
 *
 *  Reference:    3GPP TS 36.213 version 10.0.1 Release 10
 *****************************************************************************/

#ifndef ISRRAN_RA_UL_H
#define ISRRAN_RA_UL_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/pusch_cfg.h"

// Structure for PUSCH frequency hopping procedure
typedef struct ISRRAN_API {
  bool              initialized;
  isrran_cell_t     cell;
  isrran_sequence_t seq_type2_fo;
} isrran_ra_ul_pusch_hopping_t;

ISRRAN_API int isrran_ra_ul_pusch_hopping_init(isrran_ra_ul_pusch_hopping_t* q, isrran_cell_t cell);

ISRRAN_API void isrran_ra_ul_pusch_hopping_free(isrran_ra_ul_pusch_hopping_t* q);

ISRRAN_API void isrran_ra_ul_pusch_hopping(isrran_ra_ul_pusch_hopping_t* q,
                                           isrran_ul_sf_cfg_t*           sf,
                                           isrran_pusch_hopping_cfg_t*   hopping_cfg,
                                           isrran_pusch_grant_t*         grant);

/** Functions to generate a grant from a received DCI */
ISRRAN_API int isrran_ra_ul_dci_to_grant(isrran_cell_t*              cell,
                                         isrran_ul_sf_cfg_t*         sf,
                                         isrran_pusch_hopping_cfg_t* hopping_cfg,
                                         isrran_dci_ul_t*            dci,
                                         isrran_pusch_grant_t*       grant);

ISRRAN_API void isrran_ra_ul_compute_nof_re(isrran_pusch_grant_t* grant, isrran_cp_t cp, uint32_t N_isr);

/** Others */
ISRRAN_API uint32_t isrran_ra_ul_info(const isrran_pusch_grant_t* grant, char* info_str, uint32_t len);

#endif // ISRRAN_RA_UL_H
