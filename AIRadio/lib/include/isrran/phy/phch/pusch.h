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
 *  File:         pusch.h
 *
 *  Description:  Physical uplink shared channel.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 5.3
 *****************************************************************************/

#ifndef ISRRAN_PUSCH_H
#define ISRRAN_PUSCH_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/refsignal_ul.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/dft_precoding.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/evm.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/pusch_cfg.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/phch/sch.h"
#include "isrran/phy/scrambling/scrambling.h"

/* PUSCH object */
typedef struct ISRRAN_API {
  isrran_cell_t cell;

  bool     is_ue;
  uint16_t ue_rnti;
  uint32_t max_re;

  bool llr_is_8bit;

  isrran_dft_precoding_t dft_precoding;

  /* buffers */
  // void buffers are shared for tx and rx
  cf_t* ce;
  cf_t* z;
  cf_t* d;

  void* q;
  void* g;

  /* tx & rx objects */
  isrran_modem_table_t mod[ISRRAN_MOD_NITEMS];
  isrran_sch_t         ul_sch;

  // EVM buffer
  isrran_evm_buffer_t* evm_buffer;

} isrran_pusch_t;

typedef struct ISRRAN_API {
  uint8_t*           ptr;
  isrran_uci_value_t uci;
} isrran_pusch_data_t;

typedef struct ISRRAN_API {
  uint8_t*           data;
  isrran_uci_value_t uci;
  bool               crc;
  float              avg_iterations_block;
  float              evm;
  float              epre_dbfs;
} isrran_pusch_res_t;

ISRRAN_API int isrran_pusch_init_ue(isrran_pusch_t* q, uint32_t max_prb);

ISRRAN_API int isrran_pusch_init_enb(isrran_pusch_t* q, uint32_t max_prb);

ISRRAN_API void isrran_pusch_free(isrran_pusch_t* q);

/* These functions modify the state of the object and may take some time */
ISRRAN_API int isrran_pusch_set_cell(isrran_pusch_t* q, isrran_cell_t cell);

/**
 * Asserts PUSCH grant attributes are in range
 * @param grant Pointer to PUSCH grant
 * @return it returns ISRRAN_SUCCESS if the grant is correct, otherwise it returns a ISRRAN_ERROR code
 */
ISRRAN_API int isrran_pusch_assert_grant(const isrran_pusch_grant_t* grant);

/* These functions do not modify the state and run in real-time */
ISRRAN_API int isrran_pusch_encode(isrran_pusch_t*      q,
                                   isrran_ul_sf_cfg_t*  sf,
                                   isrran_pusch_cfg_t*  cfg,
                                   isrran_pusch_data_t* data,
                                   cf_t*                sf_symbols);

ISRRAN_API int isrran_pusch_decode(isrran_pusch_t*        q,
                                   isrran_ul_sf_cfg_t*    sf,
                                   isrran_pusch_cfg_t*    cfg,
                                   isrran_chest_ul_res_t* channel,
                                   cf_t*                  sf_symbols,
                                   isrran_pusch_res_t*    data);

ISRRAN_API uint32_t isrran_pusch_grant_tx_info(isrran_pusch_grant_t* grant,
                                               isrran_uci_cfg_t*     uci_cfg,
                                               isrran_uci_value_t*   uci_data,
                                               char*                 str,
                                               uint32_t              str_len);

ISRRAN_API uint32_t isrran_pusch_tx_info(isrran_pusch_cfg_t* cfg,
                                         isrran_uci_value_t* uci_data,
                                         char*               str,
                                         uint32_t            str_len);

ISRRAN_API uint32_t isrran_pusch_rx_info(isrran_pusch_cfg_t*    cfg,
                                         isrran_pusch_res_t*    res,
                                         isrran_chest_ul_res_t* chest_res,
                                         char*                  str,
                                         uint32_t               str_len);

#endif // ISRRAN_PUSCH_H
