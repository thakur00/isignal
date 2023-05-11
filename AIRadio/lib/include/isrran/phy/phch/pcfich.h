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
 *  File:         pcfich.h
 *
 *  Description:  Physical control format indicator channel
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.7
 *****************************************************************************/

#ifndef ISRRAN_PCFICH_H
#define ISRRAN_PCFICH_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/scrambling/scrambling.h"

#define PCFICH_CFI_LEN 32
#define PCFICH_RE PCFICH_CFI_LEN / 2

/* PCFICH object */
typedef struct ISRRAN_API {
  isrran_cell_t cell;
  int           nof_symbols;

  uint32_t nof_rx_antennas;

  /* handler to REGs resource mapper */
  isrran_regs_t* regs;

  /* buffers */
  cf_t ce[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS][PCFICH_RE];
  cf_t symbols[ISRRAN_MAX_PORTS][PCFICH_RE];
  cf_t x[ISRRAN_MAX_PORTS][PCFICH_RE];
  cf_t d[PCFICH_RE];

  // cfi table in floats
  float cfi_table_float[3][PCFICH_CFI_LEN];

  /* bit message */
  uint8_t data[PCFICH_CFI_LEN];

  /* received soft bits */
  float data_f[PCFICH_CFI_LEN];

  /* tx & rx objects */
  isrran_modem_table_t mod;
  isrran_sequence_t    seq[ISRRAN_NOF_SF_X_FRAME];

} isrran_pcfich_t;

ISRRAN_API int isrran_pcfich_init(isrran_pcfich_t* q, uint32_t nof_rx_antennas);

ISRRAN_API int isrran_pcfich_set_cell(isrran_pcfich_t* q, isrran_regs_t* regs, isrran_cell_t cell);

ISRRAN_API void isrran_pcfich_free(isrran_pcfich_t* q);

ISRRAN_API int isrran_pcfich_decode(isrran_pcfich_t*       q,
                                    isrran_dl_sf_cfg_t*    sf,
                                    isrran_chest_dl_res_t* channel,
                                    cf_t*                  sf_symbols[ISRRAN_MAX_PORTS],
                                    float*                 corr_result);

ISRRAN_API int isrran_pcfich_encode(isrran_pcfich_t* q, isrran_dl_sf_cfg_t* sf, cf_t* sf_symbols[ISRRAN_MAX_PORTS]);

#endif // ISRRAN_PCFICH_H
