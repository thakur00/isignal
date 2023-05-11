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
 *  File:         pmch.h
 *
 *  Description:  Physical multicast channel
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.5
 *****************************************************************************/

#ifndef ISRRAN_PMCH_H
#define ISRRAN_PMCH_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/pdsch.h"
#include "isrran/phy/phch/ra_dl.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/phch/sch.h"
#include "isrran/phy/scrambling/scrambling.h"
typedef struct {
  isrran_sequence_t seq[ISRRAN_NOF_SF_X_FRAME];
} isrran_pmch_seq_t;

typedef struct ISRRAN_API {
  isrran_pdsch_cfg_t pdsch_cfg;
  uint16_t           area_id;
} isrran_pmch_cfg_t;

/* PMCH object */
typedef struct ISRRAN_API {
  isrran_cell_t cell;

  uint32_t nof_rx_antennas;

  uint32_t max_re;

  /* buffers */
  // void buffers are shared for tx and rx
  cf_t* ce[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  cf_t* symbols[ISRRAN_MAX_PORTS];
  cf_t* x[ISRRAN_MAX_PORTS];
  cf_t* d;
  void* e;

  /* tx & rx objects */
  isrran_modem_table_t mod[4];

  // This is to generate the scrambling seq for multiple MBSFN Area IDs
  isrran_pmch_seq_t** seqs;

  isrran_sch_t dl_sch;

} isrran_pmch_t;

ISRRAN_API int isrran_pmch_init(isrran_pmch_t* q, uint32_t max_prb, uint32_t nof_rx_antennas);

ISRRAN_API void isrran_pmch_free(isrran_pmch_t* q);

ISRRAN_API int isrran_pmch_set_cell(isrran_pmch_t* q, isrran_cell_t cell);

ISRRAN_API int isrran_pmch_set_area_id(isrran_pmch_t* q, uint16_t area_id);

ISRRAN_API void isrran_pmch_free_area_id(isrran_pmch_t* q, uint16_t area_id);

ISRRAN_API void isrran_configure_pmch(isrran_pmch_cfg_t* pmch_cfg, isrran_cell_t* cell, isrran_mbsfn_cfg_t* mbsfn_cfg);

ISRRAN_API int isrran_pmch_encode(isrran_pmch_t*      q,
                                  isrran_dl_sf_cfg_t* sf,
                                  isrran_pmch_cfg_t*  cfg,
                                  uint8_t*            data,
                                  cf_t*               sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API int isrran_pmch_decode(isrran_pmch_t*         q,
                                  isrran_dl_sf_cfg_t*    sf,
                                  isrran_pmch_cfg_t*     cfg,
                                  isrran_chest_dl_res_t* channel,
                                  cf_t*                  sf_symbols[ISRRAN_MAX_PORTS],
                                  isrran_pdsch_res_t*    data);

#endif // ISRRAN_PMCH_H
