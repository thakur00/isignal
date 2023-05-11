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
 *  File:         pucch.h
 *
 *  Description:  Physical uplink control channel.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 5.4
 *****************************************************************************/

#ifndef ISRRAN_PUCCH_H
#define ISRRAN_PUCCH_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_ul.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/cqi.h"
#include "isrran/phy/phch/pucch_cfg.h"
#include "isrran/phy/phch/uci.h"

#define ISRRAN_PUCCH_N_SEQ ISRRAN_NRE
#define ISRRAN_PUCCH2_NOF_BITS ISRRAN_UCI_CQI_CODED_PUCCH_B
#define ISRRAN_PUCCH2_N_SF (5)
#define ISRRAN_PUCCH_1A_2A_NOF_ACK (1)
#define ISRRAN_PUCCH_1B_2B_NOF_ACK (2)
#define ISRRAN_PUCCH3_NOF_BITS (4 * ISRRAN_NRE)
#define ISRRAN_PUCCH_MAX_SYMBOLS (ISRRAN_PUCCH_N_SEQ * ISRRAN_PUCCH2_N_SF * ISRRAN_NOF_SLOTS_PER_SF)

// PUCCH Format 1B Channel selection
#define ISRRAN_PUCCH_CS_MAX_ACK 4
#define ISRRAN_PUCCH_CS_MAX_CARRIERS 2
#define ISRRAN_PUCCH_FORMAT3_MAX_CARRIERS 5

#define ISRRAN_PUCCH_DEFAULT_THRESHOLD_FORMAT1 (0.5f)
#define ISRRAN_PUCCH_DEFAULT_THRESHOLD_FORMAT1A (0.5f)
#define ISRRAN_PUCCH_DEFAULT_THRESHOLD_FORMAT2 (0.5f)
#define ISRRAN_PUCCH_DEFAULT_THRESHOLD_FORMAT3 (0.5f)
#define ISRRAN_PUCCH_DEFAULT_THRESHOLD_DMRS (0.4f)

/* PUCCH object */
typedef struct ISRRAN_API {
  isrran_cell_t        cell;
  isrran_modem_table_t mod;

  isrran_uci_cqi_pucch_t cqi;

  isrran_sequence_t seq_f2;
  bool              is_ue;

  int16_t  llr[ISRRAN_PUCCH3_NOF_BITS];
  uint8_t  bits_scram[ISRRAN_PUCCH_MAX_BITS];
  cf_t     d[ISRRAN_PUCCH_MAX_BITS / 2];
  uint32_t n_cs_cell[ISRRAN_NSLOTS_X_FRAME][ISRRAN_CP_NORM_NSYMB];
  uint32_t f_gh[ISRRAN_NSLOTS_X_FRAME];

  cf_t* z;
  cf_t* z_tmp;
  cf_t* ce;

} isrran_pucch_t;

typedef struct ISRRAN_API {
  isrran_uci_value_t uci_data;
  float              dmrs_correlation;
  float              snr_db;
  float              rssi_dbFs;
  float              ni_dbFs;
  float              correlation;
  bool               detected;

  // PUCCH Measurements
  bool  ta_valid;
  float ta_us;
} isrran_pucch_res_t;

ISRRAN_API int isrran_pucch_init_ue(isrran_pucch_t* q);

ISRRAN_API int isrran_pucch_init_enb(isrran_pucch_t* q);

ISRRAN_API void isrran_pucch_free(isrran_pucch_t* q);

/* These functions modify the state of the object and may take some time */
ISRRAN_API int isrran_pucch_set_cell(isrran_pucch_t* q, isrran_cell_t cell);

/* These functions do not modify the state and run in real-time */
ISRRAN_API void isrran_pucch_uci_gen_cfg(isrran_pucch_t* q, isrran_pucch_cfg_t* cfg, isrran_uci_data_t* uci_data);

ISRRAN_API int isrran_pucch_encode(isrran_pucch_t*     q,
                                   isrran_ul_sf_cfg_t* sf,
                                   isrran_pucch_cfg_t* cfg,
                                   isrran_uci_value_t* uci_data,
                                   cf_t*               sf_symbols);

ISRRAN_API int isrran_pucch_decode(isrran_pucch_t*        q,
                                   isrran_ul_sf_cfg_t*    sf,
                                   isrran_pucch_cfg_t*    cfg,
                                   isrran_chest_ul_res_t* channel,
                                   cf_t*                  sf_symbols,
                                   isrran_pucch_res_t*    data);

/* Other utilities. These functions do not modify the state and run in real-time */
ISRRAN_API float isrran_pucch_alpha_format1(const uint32_t n_cs_cell[ISRRAN_NSLOTS_X_FRAME][ISRRAN_CP_NORM_NSYMB],
                                            const isrran_pucch_cfg_t* cfg,
                                            isrran_cp_t               cp,
                                            bool                      is_dmrs,
                                            uint32_t                  ns,
                                            uint32_t                  l,
                                            uint32_t*                 n_oc,
                                            uint32_t*                 n_prime_ns);

ISRRAN_API float isrran_pucch_alpha_format2(const uint32_t n_cs_cell[ISRRAN_NSLOTS_X_FRAME][ISRRAN_CP_NORM_NSYMB],
                                            const isrran_pucch_cfg_t* cfg,
                                            uint32_t                  ns,
                                            uint32_t                  l);

ISRRAN_API int isrran_pucch_format2ab_mod_bits(isrran_pucch_format_t format, uint8_t bits[2], cf_t* d_10);

ISRRAN_API uint32_t isrran_pucch_m(const isrran_pucch_cfg_t* cfg, isrran_cp_t cp);

ISRRAN_API uint32_t isrran_pucch_n_prb(const isrran_cell_t* cell, const isrran_pucch_cfg_t* cfg, uint32_t ns);

ISRRAN_API int isrran_pucch_n_cs_cell(isrran_cell_t cell,
                                      uint32_t      n_cs_cell[ISRRAN_NSLOTS_X_FRAME][ISRRAN_CP_NORM_NSYMB]);

/**
 * Checks PUCCH collision from cell and two PUCCH configurations. The provided configurations shall provide format and
 * n_pucch resource prior to this call.
 *
 * @param cell cell parameters
 * @param cfg1 First PUCCH configuration
 * @param cfg2 Second PUCCH configuration
 * @return ISRRAN_SUCCESS if no collision, ISRRAN_ERROR if collision and otherwise ISRRAN_INVALID_INPUTS
 */
ISRRAN_API int
isrran_pucch_collision(const isrran_cell_t* cell, const isrran_pucch_cfg_t* cfg1, const isrran_pucch_cfg_t* cfg2);

/**
 * Checks PUCCH format 1b with channel selection collision configuration from a cell.
 *
 * @param cell cell parameters
 * @param cfg PUCCH configuration
 * @return ISRRAN_SUCCESS if no collision, ISRRAN_ERROR if collision and otherwise ISRRAN_INVALID_INPUTS
 */
ISRRAN_API int isrran_pucch_cfg_assert(const isrran_cell_t* cell, const isrran_pucch_cfg_t* cfg);

ISRRAN_API char* isrran_pucch_format_text(isrran_pucch_format_t format);

ISRRAN_API char* isrran_pucch_format_text_short(isrran_pucch_format_t format);

/**
 * Returns the number of ACK bits supported by a given PUCCH format
 * @param format PUCCH format
 * @return Returns the number of bits supported by the format
 */
ISRRAN_API uint32_t isrran_pucch_nof_ack_format(isrran_pucch_format_t format);

ISRRAN_API void
isrran_pucch_tx_info(isrran_pucch_cfg_t* cfg, isrran_uci_value_t* uci_data, char* str, uint32_t str_len);

ISRRAN_API void
isrran_pucch_rx_info(isrran_pucch_cfg_t* cfg, isrran_pucch_res_t* pucch_res, char* str, uint32_t str_len);

ISRRAN_API bool isrran_pucch_cfg_isvalid(isrran_pucch_cfg_t* cfg, uint32_t nof_prb);

#endif // ISRRAN_PUCCH_H
