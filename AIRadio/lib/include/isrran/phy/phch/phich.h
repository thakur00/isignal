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
 *  File:         phich.h
 *
 *  Description:  Physical Hybrid ARQ indicator channel.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.9
 *****************************************************************************/

#ifndef ISRRAN_PHICH_H
#define ISRRAN_PHICH_H

#include "regs.h"
#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/scrambling/scrambling.h"

#define ISRRAN_PHICH_NORM_NSEQUENCES 8
#define ISRRAN_PHICH_EXT_NSEQUENCES 4
#define ISRRAN_PHICH_NBITS 3

#define ISRRAN_PHICH_NORM_MSYMB ISRRAN_PHICH_NBITS * 4
#define ISRRAN_PHICH_EXT_MSYMB ISRRAN_PHICH_NBITS * 2
#define ISRRAN_PHICH_MAX_NSYMB ISRRAN_PHICH_NORM_MSYMB
#define ISRRAN_PHICH_NORM_C 1
#define ISRRAN_PHICH_EXT_C 2
#define ISRRAN_PHICH_NORM_NSF 4
#define ISRRAN_PHICH_EXT_NSF 2

/* phich object */
typedef struct ISRRAN_API {
  isrran_cell_t cell;

  uint32_t nof_rx_antennas;

  /* handler to REGs resource mapper */
  isrran_regs_t* regs;

  /* buffers */
  cf_t ce[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS][ISRRAN_PHICH_MAX_NSYMB];
  cf_t sf_symbols[ISRRAN_MAX_PORTS][ISRRAN_PHICH_MAX_NSYMB];
  cf_t x[ISRRAN_MAX_PORTS][ISRRAN_PHICH_MAX_NSYMB];
  cf_t d[ISRRAN_PHICH_MAX_NSYMB];
  cf_t d0[ISRRAN_PHICH_MAX_NSYMB];
  cf_t z[ISRRAN_PHICH_NBITS];

  /* bit message */
  uint8_t data[ISRRAN_PHICH_NBITS];
  float   data_rx[ISRRAN_PHICH_NBITS];

  /* tx & rx objects */
  isrran_modem_table_t mod;
  isrran_sequence_t    seq[ISRRAN_NOF_SF_X_FRAME];

} isrran_phich_t;

typedef struct ISRRAN_API {
  uint32_t ngroup;
  uint32_t nseq;
} isrran_phich_resource_t;

typedef struct ISRRAN_API {
  uint32_t n_prb_lowest;
  uint32_t n_dmrs;
  uint32_t I_phich;
} isrran_phich_grant_t;

typedef struct ISRRAN_API {
  bool  ack_value;
  float distance;
} isrran_phich_res_t;

ISRRAN_API int isrran_phich_init(isrran_phich_t* q, uint32_t nof_rx_antennas);

ISRRAN_API void isrran_phich_free(isrran_phich_t* q);

ISRRAN_API int isrran_phich_set_cell(isrran_phich_t* q, isrran_regs_t* regs, isrran_cell_t cell);

ISRRAN_API void isrran_phich_set_regs(isrran_phich_t* q, isrran_regs_t* regs);

ISRRAN_API void isrran_phich_calc(isrran_phich_t* q, isrran_phich_grant_t* grant, isrran_phich_resource_t* n_phich);

ISRRAN_API int isrran_phich_decode(isrran_phich_t*         q,
                                   isrran_dl_sf_cfg_t*     sf,
                                   isrran_chest_dl_res_t*  channel,
                                   isrran_phich_resource_t n_phich,
                                   cf_t*                   sf_symbols[ISRRAN_MAX_PORTS],
                                   isrran_phich_res_t*     result);

ISRRAN_API int isrran_phich_encode(isrran_phich_t*         q,
                                   isrran_dl_sf_cfg_t*     sf,
                                   isrran_phich_resource_t n_phich,
                                   uint8_t                 ack,
                                   cf_t*                   sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API void isrran_phich_reset(isrran_phich_t* q, cf_t* slot_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API uint32_t isrran_phich_ngroups(isrran_phich_t* q);

ISRRAN_API uint32_t isrran_phich_nsf(isrran_phich_t* q);

#endif // ISRRAN_PHICH_H
