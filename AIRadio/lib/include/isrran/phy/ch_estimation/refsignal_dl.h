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
 *  File:         refsignal_dl.h
 *
 *  Description:  Object to manage downlink reference signals for channel estimation.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.10
 *********************************************************************************************/

#ifndef ISRRAN_REFSIGNAL_DL_H
#define ISRRAN_REFSIGNAL_DL_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"

// Number of references in a subframe: there are 2 symbols for port_id=0,1 x 2 slots x 2 refs per prb
#define ISRRAN_REFSIGNAL_NUM_SF_MBSFN(nof_prb, port_id) ((2 + 18) * (nof_prb))

#define ISRRAN_REFSIGNAL_MAX_NUM_SF(nof_prb) (8 * nof_prb)
#define ISRRAN_REFSIGNAL_MAX_NUM_SF_MBSFN(nof_prb) ISRRAN_REFSIGNAL_NUM_SF_MBSFN(nof_prb, 0)

#define ISRRAN_REFSIGNAL_PILOT_IDX(i, l, cell) (2 * cell.nof_prb * (l) + (i))

#define ISRRAN_REFSIGNAL_PILOT_IDX_MBSFN(i, l, cell) ((6 * cell.nof_prb * (l) + (i)))

/** Cell-Specific Reference Signal */
typedef struct ISRRAN_API {
  isrran_cell_t cell;
  cf_t*         pilots[2][ISRRAN_NOF_SF_X_FRAME]; // Saves the reference signal per subframe for ports 0,1 and ports 2,3
  isrran_sf_t   type;
  uint16_t      mbsfn_area_id;
} isrran_refsignal_t;

ISRRAN_API int isrran_refsignal_cs_init(isrran_refsignal_t* q, uint32_t max_prb);

ISRRAN_API int isrran_refsignal_cs_set_cell(isrran_refsignal_t* q, isrran_cell_t cell);

ISRRAN_API void isrran_refsignal_free(isrran_refsignal_t* q);

ISRRAN_API int
isrran_refsignal_cs_put_sf(isrran_refsignal_t* q, isrran_dl_sf_cfg_t* sf, uint32_t port_id, cf_t* sf_symbols);

ISRRAN_API int isrran_refsignal_cs_get_sf(isrran_refsignal_t* q,
                                          isrran_dl_sf_cfg_t* sf,
                                          uint32_t            port_id,
                                          cf_t*               sf_symbols,
                                          cf_t*               pilots);

ISRRAN_API uint32_t isrran_refsignal_cs_fidx(isrran_cell_t cell, uint32_t l, uint32_t port_id, uint32_t m);

ISRRAN_API uint32_t isrran_refsignal_cs_nsymbol(uint32_t l, isrran_cp_t cp, uint32_t port_id);

ISRRAN_API uint32_t isrran_refsignal_cs_v(uint32_t port_id, uint32_t ref_symbol_idx);

ISRRAN_API uint32_t isrran_refsignal_cs_nof_symbols(isrran_refsignal_t* q, isrran_dl_sf_cfg_t* sf, uint32_t port_id);

ISRRAN_API uint32_t isrran_refsignal_cs_nof_pilots_x_slot(uint32_t nof_ports);

ISRRAN_API uint32_t isrran_refsignal_cs_nof_re(isrran_refsignal_t* q, isrran_dl_sf_cfg_t* sf, uint32_t port_id);

ISRRAN_API int isrran_refsignal_mbsfn_init(isrran_refsignal_t* q, uint32_t max_prb);

ISRRAN_API int isrran_refsignal_mbsfn_set_cell(isrran_refsignal_t* q, isrran_cell_t cell, uint16_t mbsfn_area_id);

ISRRAN_API int isrran_refsignal_mbsfn_get_sf(isrran_cell_t cell, uint32_t port_id, cf_t* sf_symbols, cf_t* pilots);

ISRRAN_API uint32_t isrran_refsignal_mbsfn_nsymbol(uint32_t l);

ISRRAN_API uint32_t isrran_refsignal_mbsfn_fidx(uint32_t l);

ISRRAN_API uint32_t isrran_refsignal_mbsfn_nof_symbols();

ISRRAN_API int isrran_refsignal_mbsfn_put_sf(isrran_cell_t cell,
                                             uint32_t      port_id,
                                             cf_t*         cs_pilots,
                                             cf_t*         mbsfn_pilots,
                                             cf_t*         sf_symbols);

ISRRAN_API int isrran_refsignal_mbsfn_gen_seq(isrran_refsignal_t* q, isrran_cell_t cell, uint32_t N_mbsfn_id);

#endif // ISRRAN_REFSIGNAL_DL_H
