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

#ifndef ISRRAN_PUSCH_NR_H
#define ISRRAN_PUSCH_NR_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/dmrs_sch.h"
#include "isrran/phy/modem/evm.h"
#include "isrran/phy/modem/modem_table.h"
#include "isrran/phy/phch/phch_cfg_nr.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/phch/sch_nr.h"
#include "isrran/phy/phch/uci_nr.h"
#include "isrran/phy/scrambling/scrambling.h"

/**
 * @brief PUSCH encoder and decoder initialization arguments
 */
typedef struct ISRRAN_API {
  isrran_sch_nr_args_t sch;
  isrran_uci_nr_args_t uci;
  bool                 measure_evm;
  bool                 measure_time;
  uint32_t             max_layers;
  uint32_t             max_prb;
} isrran_pusch_nr_args_t;

/**
 * @brief PDSCH NR object
 */
typedef struct ISRRAN_API {
  uint32_t             max_prb;                         ///< Maximum number of allocated prb
  uint32_t             max_layers;                      ///< Maximum number of allocated layers
  uint32_t             max_cw;                          ///< Maximum number of allocated code words
  isrran_carrier_nr_t  carrier;                         ///< NR carrier configuration
  isrran_sch_nr_t      sch;                             ///< SCH Encoder/Decoder Object
  isrran_uci_nr_t      uci;                             ///< UCI Encoder/Decoder Object
  uint8_t*             b[ISRRAN_MAX_CODEWORDS];         ///< SCH Encoded and scrambled data
  cf_t*                d[ISRRAN_MAX_CODEWORDS];         ///< PDSCH modulated bits
  cf_t*                x[ISRRAN_MAX_LAYERS_NR];         ///< PDSCH modulated bits
  isrran_modem_table_t modem_tables[ISRRAN_MOD_NITEMS]; ///< Modulator tables
  isrran_evm_buffer_t* evm_buffer;
  bool                 meas_time_en;
  uint32_t             meas_time_us;
  isrran_re_pattern_t  dmrs_re_pattern;
  uint8_t*             g_ulsch;   ///< Temporal Encoded UL-SCH data
  uint8_t*             g_ack;     ///< Temporal Encoded HARQ-ACK bits
  uint8_t*             g_csi1;    ///< Temporal Encoded CSI part 1 bits
  uint8_t*             g_csi2;    ///< Temporal Encoded CSI part 2 bits
  uint32_t*            pos_ulsch; ///< Reserved resource elements for HARQ-ACK multiplexing position
  uint32_t*            pos_ack;   ///< Reserved resource elements for HARQ-ACK multiplexing position
  uint32_t*            pos_csi1;  ///< Reserved resource elements for CSI part 1 multiplexing position
  uint32_t*            pos_csi2;  ///< Reserved resource elements for CSI part 1 multiplexing position
  bool                 uci_mux;   ///< Set to true if PUSCH needs to multiplex UCI
  uint32_t             G_ack;     ///< Number of encoded HARQ-ACK bits
  uint32_t             G_csi1;    ///< Number of encoded CSI part 1 bits
  uint32_t             G_csi2;    ///< Number of encoded CSI part 2 bits
  uint32_t             G_ulsch;   ///< Number of encoded shared channel
} isrran_pusch_nr_t;

/**
 * @brief Groups NR-PUSCH data for transmission
 */
typedef struct {
  uint8_t*              payload[ISRRAN_MAX_TB]; ///< SCH payload
  isrran_uci_value_nr_t uci;                    ///< UCI payload
} isrran_pusch_data_nr_t;

/**
 * @brief Groups NR-PUSCH data for reception
 */
typedef struct {
  isrran_sch_tb_res_nr_t tb[ISRRAN_MAX_TB];         ///< SCH payload
  isrran_uci_value_nr_t  uci;                       ///< UCI payload
  float                  evm[ISRRAN_MAX_CODEWORDS]; ///< EVM measurement if configured through arguments
} isrran_pusch_res_nr_t;

ISRRAN_API int isrran_pusch_nr_init_gnb(isrran_pusch_nr_t* q, const isrran_pusch_nr_args_t* args);

ISRRAN_API int isrran_pusch_nr_init_ue(isrran_pusch_nr_t* q, const isrran_pusch_nr_args_t* args);

ISRRAN_API void isrran_pusch_nr_free(isrran_pusch_nr_t* q);

ISRRAN_API int isrran_pusch_nr_set_carrier(isrran_pusch_nr_t* q, const isrran_carrier_nr_t* carrier);

ISRRAN_API int isrran_pusch_nr_encode(isrran_pusch_nr_t*            q,
                                      const isrran_sch_cfg_nr_t*    cfg,
                                      const isrran_sch_grant_nr_t*  grant,
                                      const isrran_pusch_data_nr_t* data,
                                      cf_t*                         sf_symbols[ISRRAN_MAX_PORTS]);

ISRRAN_API int isrran_pusch_nr_decode(isrran_pusch_nr_t*           q,
                                      const isrran_sch_cfg_nr_t*   cfg,
                                      const isrran_sch_grant_nr_t* grant,
                                      isrran_chest_dl_res_t*       channel,
                                      cf_t*                        sf_symbols[ISRRAN_MAX_PORTS],
                                      isrran_pusch_res_nr_t*       data);

ISRRAN_API uint32_t isrran_pusch_nr_rx_info(const isrran_pusch_nr_t*     q,
                                            const isrran_sch_cfg_nr_t*   cfg,
                                            const isrran_sch_grant_nr_t* grant,
                                            const isrran_pusch_res_nr_t* res,
                                            char*                        str,
                                            uint32_t                     str_len);

ISRRAN_API uint32_t isrran_pusch_nr_tx_info(const isrran_pusch_nr_t*     q,
                                            const isrran_sch_cfg_nr_t*   cfg,
                                            const isrran_sch_grant_nr_t* grant,
                                            const isrran_uci_value_nr_t* uci_value,
                                            char*                        str,
                                            uint32_t                     str_len);

#endif // ISRRAN_PUSCH_NR_H
