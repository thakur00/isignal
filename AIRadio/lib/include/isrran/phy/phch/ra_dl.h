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
 *  File:         ra_dl.h
 *
 *  Description:  Implements Resource allocation Procedures for DL defined in Section 7
 *
 *  Reference:    3GPP TS 36.213 version 10.0.1 Release 10
 *****************************************************************************/

#ifndef ISRRAN_RA_DL_H
#define ISRRAN_RA_DL_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/pdsch_cfg.h"
#include "isrran/phy/phch/ra.h"

/**************************************************
 * Structures used for Downlink Resource Allocation
 **************************************************/

/** Functions to generate a grant from a received DCI */
ISRRAN_API int isrran_ra_dl_dci_to_grant(const isrran_cell_t*   cell,
                                         isrran_dl_sf_cfg_t*    sf,
                                         isrran_tm_t            tm,
                                         bool                   pdsch_use_tbs_index_alt,
                                         const isrran_dci_dl_t* dci,
                                         isrran_pdsch_grant_t*  grant);

ISRRAN_API int
isrran_ra_dl_grant_to_grant_prb_allocation(const isrran_dci_dl_t* dci, isrran_pdsch_grant_t* grant, uint32_t nof_prb);

/** Functions used by the eNodeB scheduler */
ISRRAN_API uint32_t isrran_ra_dl_approx_nof_re(const isrran_cell_t* cell, uint32_t nof_prb, uint32_t nof_ctrl_symbols);

ISRRAN_API uint32_t ra_re_x_prb(const isrran_cell_t* cell, isrran_dl_sf_cfg_t* sf, uint32_t slot, uint32_t prb_idx);

ISRRAN_API uint32_t isrran_ra_dl_grant_nof_re(const isrran_cell_t*  cell,
                                              isrran_dl_sf_cfg_t*   sf,
                                              isrran_pdsch_grant_t* grant);

/** Others */
ISRRAN_API int isrran_dl_fill_ra_mcs(isrran_ra_tb_t* tb, int last_tbs, uint32_t nprb, bool pdsch_use_tbs_index_alt);

ISRRAN_API void
isrran_ra_dl_compute_nof_re(const isrran_cell_t* cell, isrran_dl_sf_cfg_t* sf, isrran_pdsch_grant_t* grant);

ISRRAN_API uint32_t isrran_ra_dl_info(isrran_pdsch_grant_t* grant, char* info_str, uint32_t len);

#endif // ISRRAN_RA_DL_H
