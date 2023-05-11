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
 *  File:         pdcch.h
 *
 *  Description:  Physical downlink control channel.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.8
 *****************************************************************************/

#ifndef ISRRAN_PDCCH_H
#define ISRRAN_PDCCH_H

#include "isrran/config.h"
#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/fec/convolutional/convcoder.h"
#include "isrran/phy/fec/convolutional/rm_conv.h"
#include "isrran/phy/fec/convolutional/viterbi.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/scrambling/scrambling.h"

typedef enum ISRRAN_API { SEARCH_UE, SEARCH_COMMON } isrran_pdcch_search_mode_t;

/* PDCCH object */
typedef struct ISRRAN_API {
  isrran_cell_t cell;
  uint32_t      nof_regs[3];
  uint32_t      nof_cce[3];
  uint32_t      max_bits;
  uint32_t      nof_rx_antennas;
  bool          is_ue;

  isrran_regs_t* regs;

  /* buffers */
  cf_t*    ce[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];
  cf_t*    symbols[ISRRAN_MAX_PORTS];
  cf_t*    x[ISRRAN_MAX_PORTS];
  cf_t*    d;
  uint8_t* e;
  float    rm_f[3 * (ISRRAN_DCI_MAX_BITS + 16)];
  float*   llr;

  /* tx & rx objects */
  isrran_modem_table_t mod;
  isrran_sequence_t    seq[ISRRAN_NOF_SF_X_FRAME];
  isrran_viterbi_t     decoder;
  isrran_crc_t         crc;

} isrran_pdcch_t;

ISRRAN_API int isrran_pdcch_init_ue(isrran_pdcch_t* q, uint32_t max_prb, uint32_t nof_rx_antennas);

ISRRAN_API int isrran_pdcch_init_enb(isrran_pdcch_t* q, uint32_t max_prb);

ISRRAN_API int isrran_pdcch_set_cell(isrran_pdcch_t* q, isrran_regs_t* regs, isrran_cell_t cell);

ISRRAN_API void isrran_pdcch_set_regs(isrran_pdcch_t* q, isrran_regs_t* regs);

ISRRAN_API void isrran_pdcch_free(isrran_pdcch_t* q);

ISRRAN_API float isrran_pdcch_coderate(uint32_t nof_bits, uint32_t l);

/* Encoding function */
ISRRAN_API int isrran_pdcch_encode(isrran_pdcch_t*     q,
                                   isrran_dl_sf_cfg_t* sf,
                                   isrran_dci_msg_t*   msg,
                                   cf_t*               sf_symbols[ISRRAN_MAX_PORTS]);

/* Decoding functions: Extract the LLRs and save them in the isrran_pdcch_t object */

ISRRAN_API int isrran_pdcch_extract_llr(isrran_pdcch_t*        q,
                                        isrran_dl_sf_cfg_t*    sf,
                                        isrran_chest_dl_res_t* channel,
                                        cf_t*                  sf_symbols[ISRRAN_MAX_PORTS]);

/* Decoding functions: Try to decode a DCI message after calling isrran_pdcch_extract_llr */
ISRRAN_API int
isrran_pdcch_decode_msg(isrran_pdcch_t* q, isrran_dl_sf_cfg_t* sf, isrran_dci_cfg_t* dci_cfg, isrran_dci_msg_t* msg);

/**
 * @brief Computes decoded DCI correlation. It encodes the given DCI message and compares it with the received LLRs
 * @param q PDCCH object
 * @param msg Previously decoded DCI message
 * @return The normalized correlation between the restored symbols and the received LLRs
 */
ISRRAN_API float isrran_pdcch_msg_corr(isrran_pdcch_t* q, isrran_dci_msg_t* msg);

ISRRAN_API int
isrran_pdcch_dci_decode(isrran_pdcch_t* q, float* e, uint8_t* data, uint32_t E, uint32_t nof_bits, uint16_t* crc);

ISRRAN_API int
isrran_pdcch_dci_encode(isrran_pdcch_t* q, uint8_t* data, uint8_t* e, uint32_t nof_bits, uint32_t E, uint16_t rnti);

ISRRAN_API void
isrran_pdcch_dci_encode_conv(isrran_pdcch_t* q, uint8_t* data, uint32_t nof_bits, uint8_t* coded_data, uint16_t rnti);

/* Function for generation of UE-specific search space DCI locations */
ISRRAN_API uint32_t isrran_pdcch_ue_locations(isrran_pdcch_t*        q,
                                              isrran_dl_sf_cfg_t*    sf,
                                              isrran_dci_location_t* locations,
                                              uint32_t               max_locations,
                                              uint16_t               rnti);

ISRRAN_API uint32_t isrran_pdcch_ue_locations_ncce(uint32_t               nof_cce,
                                                   isrran_dci_location_t* c,
                                                   uint32_t               max_candidates,
                                                   uint32_t               sf_idx,
                                                   uint16_t               rnti);

ISRRAN_API uint32_t isrran_pdcch_ue_locations_ncce_L(uint32_t               nof_cce,
                                                     isrran_dci_location_t* c,
                                                     uint32_t               max_candidates,
                                                     uint32_t               sf_idx,
                                                     uint16_t               rnti,
                                                     int                    L);

/* Function for generation of common search space DCI locations */
ISRRAN_API uint32_t isrran_pdcch_common_locations(isrran_pdcch_t*        q,
                                                  isrran_dci_location_t* locations,
                                                  uint32_t               max_locations,
                                                  uint32_t               cfi);

ISRRAN_API uint32_t isrran_pdcch_common_locations_ncce(uint32_t               nof_cce,
                                                       isrran_dci_location_t* c,
                                                       uint32_t               max_candidates);

#endif // ISRRAN_PDCCH_H
