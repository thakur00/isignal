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
 *  File:         pdsch.h
 *
 *  Description:  Physical downlink shared channel
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.4
 *****************************************************************************/

#ifndef ISRRAN_PDSCH_H
#define ISRRAN_PDSCH_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/evm.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/pdsch_cfg.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/phch/sch.h"
#include "isrran/phy/scrambling/scrambling.h"

/* PDSCH object */
typedef struct ISRRAN_API {
  isrran_cell_t cell;

  uint32_t nof_rx_antennas;
  uint32_t max_re;

  bool is_ue;

  bool llr_is_8bit;

  /* buffers */
  // void buffers are shared for tx and rx
  cf_t* ce[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS]; /* Channel estimation (Rx only) */
  cf_t* symbols[ISRRAN_MAX_PORTS];              /* PDSCH Encoded/Decoded Symbols */
  cf_t* x[ISRRAN_MAX_LAYERS];                   /* Layer mapped */
  cf_t* d[ISRRAN_MAX_CODEWORDS];                /* Modulated/Demodulated codewords */
  void* e[ISRRAN_MAX_CODEWORDS];

  float* csi[ISRRAN_MAX_CODEWORDS]; /* Channel Strengh Indicator */

  /* tx & rx objects */
  isrran_modem_table_t mod[ISRRAN_MOD_NITEMS];

  // EVM buffers, one for each codeword (avoid concurrency issue with coworker)
  isrran_evm_buffer_t* evm_buffer[ISRRAN_MAX_CODEWORDS];
  float                avg_evm;

  isrran_sch_t dl_sch;

  void* coworker_ptr;

} isrran_pdsch_t;

typedef struct {
  uint8_t* payload;
  bool     crc;
  float    avg_iterations_block;
  float    evm;
} isrran_pdsch_res_t;

ISRRAN_API int isrran_pdsch_init_ue(isrran_pdsch_t* q, uint32_t max_prb, uint32_t nof_rx_antennas);

ISRRAN_API int isrran_pdsch_init_enb(isrran_pdsch_t* q, uint32_t max_prb);

ISRRAN_API void isrran_pdsch_free(isrran_pdsch_t* q);

/* These functions modify the state of the object and may take some time */
ISRRAN_API int isrran_pdsch_enable_coworker(isrran_pdsch_t* q);

ISRRAN_API int isrran_pdsch_set_cell(isrran_pdsch_t* q, isrran_cell_t cell);

/* These functions do not modify the state and run in real-time */
ISRRAN_API int isrran_pdsch_encode(isrran_pdsch_t*     q,
                                   isrran_dl_sf_cfg_t* sf,
                                   isrran_pdsch_cfg_t* cfg,
                                   uint8_t*            data[ISRRAN_MAX_CODEWORDS],
                                   cf_t*               sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API int isrran_pdsch_decode(isrran_pdsch_t*        q,
                                   isrran_dl_sf_cfg_t*    sf,
                                   isrran_pdsch_cfg_t*    cfg,
                                   isrran_chest_dl_res_t* channel,
                                   cf_t*                  sf_symbols[ISRRAN_MAX_PORTS],
                                   isrran_pdsch_res_t     data[ISRRAN_MAX_CODEWORDS]);

ISRRAN_API int isrran_pdsch_select_pmi(isrran_pdsch_t*        q,
                                       isrran_chest_dl_res_t* channel,
                                       uint32_t               nof_layers,
                                       uint32_t*              best_pmi,
                                       float                  sinr[ISRRAN_MAX_CODEBOOKS]);

ISRRAN_API int isrran_pdsch_compute_cn(isrran_pdsch_t* q, isrran_chest_dl_res_t* channel, float* cn);

ISRRAN_API uint32_t isrran_pdsch_grant_rx_info(isrran_pdsch_grant_t* grant,
                                               isrran_pdsch_res_t    res[ISRRAN_MAX_CODEWORDS],
                                               char*                 str,
                                               uint32_t              str_len);

ISRRAN_API uint32_t isrran_pdsch_rx_info(isrran_pdsch_cfg_t* cfg,
                                         isrran_pdsch_res_t  res[ISRRAN_MAX_CODEWORDS],
                                         char*               str,
                                         uint32_t            str_len);

ISRRAN_API uint32_t isrran_pdsch_tx_info(isrran_pdsch_cfg_t* cfg, char* str, uint32_t str_len);

#endif // ISRRAN_PDSCH_H
