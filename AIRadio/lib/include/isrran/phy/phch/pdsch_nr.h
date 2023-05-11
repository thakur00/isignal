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
 *  File:         pdsch_nr.h
 *
 *  Description:  Physical downlink shared channel for NR
 *
 *  Reference:    3GPP TS 38.211 V15.8.0 Sec. 7.3.1
 *****************************************************************************/

#ifndef ISRRAN_PDSCH_NR_H
#define ISRRAN_PDSCH_NR_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/dmrs_sch.h"
#include "isrran/phy/modem/evm.h"
#include "isrran/phy/modem/modem_table.h"
#include "isrran/phy/phch/phch_cfg_nr.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/phch/sch_nr.h"
#include "isrran/phy/scrambling/scrambling.h"

/**
 * @brief PDSCH encoder and decoder initialization arguments
 */
typedef struct ISRRAN_API {
  isrran_sch_nr_args_t sch;
  bool                 measure_evm;
  bool                 measure_time;
  uint32_t             max_prb;
  uint32_t             max_layers;
} isrran_pdsch_nr_args_t;

/**
 * @brief PDSCH NR object
 */
typedef struct ISRRAN_API {
  uint32_t             max_prb;                         ///< Maximum number of allocated prb
  uint32_t             max_layers;                      ///< Maximum number of allocated layers
  uint32_t             max_cw;                          ///< Maximum number of allocated code words
  isrran_carrier_nr_t  carrier;                         ///< NR carrier configuration
  isrran_sch_nr_t      sch;                             ///< SCH Encoder/Decoder Object
  uint8_t*             b[ISRRAN_MAX_CODEWORDS];         ///< SCH Encoded and scrambled data
  cf_t*                d[ISRRAN_MAX_CODEWORDS];         ///< PDSCH modulated bits
  cf_t*                x[ISRRAN_MAX_LAYERS_NR];         ///< PDSCH modulated bits
  isrran_modem_table_t modem_tables[ISRRAN_MOD_NITEMS]; ///< Modulator tables
  isrran_evm_buffer_t* evm_buffer;
  bool                 meas_time_en;
  uint32_t             meas_time_us;
  isrran_re_pattern_t  dmrs_re_pattern;
  uint32_t             nof_rvd_re;
} isrran_pdsch_nr_t;

/**
 * @brief Groups NR-PDSCH data for reception
 */
typedef struct {
  isrran_sch_tb_res_nr_t tb[ISRRAN_MAX_TB];         ///< SCH payload
  float                  evm[ISRRAN_MAX_CODEWORDS]; ///< EVM measurement if configured through arguments
} isrran_pdsch_res_nr_t;

ISRRAN_API int isrran_pdsch_nr_init_enb(isrran_pdsch_nr_t* q, const isrran_pdsch_nr_args_t* args);

ISRRAN_API int isrran_pdsch_nr_init_ue(isrran_pdsch_nr_t* q, const isrran_pdsch_nr_args_t* args);

ISRRAN_API void isrran_pdsch_nr_free(isrran_pdsch_nr_t* q);

ISRRAN_API int isrran_pdsch_nr_set_carrier(isrran_pdsch_nr_t* q, const isrran_carrier_nr_t* carrier);

ISRRAN_API int isrran_pdsch_nr_encode(isrran_pdsch_nr_t*           q,
                                      const isrran_sch_cfg_nr_t*   cfg,
                                      const isrran_sch_grant_nr_t* grant,
                                      uint8_t*                     data[ISRRAN_MAX_TB],
                                      cf_t*                        sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API int isrran_pdsch_nr_decode(isrran_pdsch_nr_t*           q,
                                      const isrran_sch_cfg_nr_t*   cfg,
                                      const isrran_sch_grant_nr_t* grant,
                                      isrran_chest_dl_res_t*       channel,
                                      cf_t*                        sf_symbols[ISRRAN_MAX_PORTS],
                                      isrran_pdsch_res_nr_t*       res);

ISRRAN_API uint32_t isrran_pdsch_nr_rx_info(const isrran_pdsch_nr_t*     q,
                                            const isrran_sch_cfg_nr_t*   cfg,
                                            const isrran_sch_grant_nr_t* grant,
                                            const isrran_pdsch_res_nr_t* res,
                                            char*                        str,
                                            uint32_t                     str_len);

ISRRAN_API uint32_t isrran_pdsch_nr_tx_info(const isrran_pdsch_nr_t*     q,
                                            const isrran_sch_cfg_nr_t*   cfg,
                                            const isrran_sch_grant_nr_t* grant,
                                            char*                        str,
                                            uint32_t                     str_len);

#endif // ISRRAN_PDSCH_NR_H
