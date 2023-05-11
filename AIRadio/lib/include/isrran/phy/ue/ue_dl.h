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
 *  File:         ue_dl.h
 *
 *  Description:  UE downlink object.
 *
 *                This module is a frontend to all the downlink data and control
 *                channel processing modules.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_UE_DL_H
#define ISRRAN_UE_DL_H

#include <stdbool.h>

#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/ofdm.h"

#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/pcfich.h"
#include "isrran/phy/phch/pdcch.h"
#include "isrran/phy/phch/pdsch.h"
#include "isrran/phy/phch/pdsch_cfg.h"
#include "isrran/phy/phch/phich.h"
#include "isrran/phy/phch/pmch.h"
#include "isrran/phy/phch/ra.h"
#include "isrran/phy/phch/regs.h"

#include "isrran/phy/sync/cfo.h"

#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

#include "isrran/config.h"

#define ISRRAN_MAX_CANDIDATES_UE 16 // From 36.213 Table 9.1.1-1
#define ISRRAN_MAX_CANDIDATES_COM 6 // From 36.213 Table 9.1.1-1
#define ISRRAN_MAX_CANDIDATES (ISRRAN_MAX_CANDIDATES_UE + ISRRAN_MAX_CANDIDATES_COM)

#define ISRRAN_MAX_FORMATS 4

#define ISRRAN_MI_NOF_REGS ((q->cell.frame_type == ISRRAN_FDD) ? 1 : 6)
#define ISRRAN_MI_MAX_REGS 6

#define ISRRAN_MAX_DCI_MSG ISRRAN_MAX_CARRIERS

typedef struct ISRRAN_API {
  isrran_dci_format_t   formats[ISRRAN_MAX_FORMATS];
  isrran_dci_location_t loc[ISRRAN_MAX_CANDIDATES];
  uint32_t              nof_locations;
  uint32_t              nof_formats;
} dci_blind_search_t;

typedef struct ISRRAN_API {
  // Cell configuration
  isrran_cell_t cell;
  uint32_t      nof_rx_antennas;
  uint16_t      current_mbsfn_area_id;

  // Objects for all DL Physical Channels
  isrran_pcfich_t pcfich;
  isrran_pdcch_t  pdcch;
  isrran_pdsch_t  pdsch;
  isrran_pmch_t   pmch;
  isrran_phich_t  phich;

  // Control region
  isrran_regs_t regs[ISRRAN_MI_MAX_REGS];
  uint32_t      mi_manual_index;
  bool          mi_auto;

  // Channel estimation and OFDM demodulation
  isrran_chest_dl_t     chest;
  isrran_chest_dl_res_t chest_res;
  isrran_ofdm_t         fft[ISRRAN_MAX_PORTS];
  isrran_ofdm_t         fft_mbsfn;

  // Buffers to store channel symbols after demodulation
  cf_t*              sf_symbols[ISRRAN_MAX_PORTS];
  dci_blind_search_t current_ss_common;

  isrran_dci_msg_t pending_ul_dci_msg[ISRRAN_MAX_DCI_MSG];
  uint32_t         pending_ul_dci_count;

  isrran_dci_location_t allocated_locations[ISRRAN_MAX_DCI_MSG];
  uint32_t              nof_allocated_locations;
} isrran_ue_dl_t;

// Downlink config (includes common and dedicated variables)
typedef struct ISRRAN_API {
  isrran_cqi_report_cfg_t cqi_report;
  isrran_pdsch_cfg_t      pdsch;
  isrran_dci_cfg_t        dci;
  isrran_tm_t             tm;
  bool                    dci_common_ss;
} isrran_dl_cfg_t;

typedef struct ISRRAN_API {
  isrran_dl_cfg_t       cfg;
  isrran_chest_dl_cfg_t chest_cfg;
  uint32_t              last_ri;
  float                 snr_to_cqi_offset;
} isrran_ue_dl_cfg_t;

typedef struct {
  uint32_t v_dai_dl;
  uint32_t n_cce;
  uint32_t grant_cc_idx;
  uint32_t tpc_for_pucch;
} isrran_pdsch_ack_resource_t;

typedef struct {
  isrran_pdsch_ack_resource_t resource;
  uint32_t                    k;
  uint8_t                     value[ISRRAN_MAX_CODEWORDS]; // 0/1 or 2 for DTX
  bool                        present;
} isrran_pdsch_ack_m_t;

typedef struct {
  uint32_t             M;
  isrran_pdsch_ack_m_t m[ISRRAN_UCI_MAX_M];
} isrran_pdsch_ack_cc_t;

typedef struct {
  isrran_pdsch_ack_cc_t           cc[ISRRAN_MAX_CARRIERS];
  uint32_t                        nof_cc;
  uint32_t                        V_dai_ul;
  isrran_tm_t                     transmission_mode;
  isrran_ack_nack_feedback_mode_t ack_nack_feedback_mode;
  bool                            is_grant_available;
  bool                            is_pusch_available;
  bool                            tdd_ack_multiplex;
  bool                            simul_cqi_ack;
  bool                            simul_cqi_ack_pucch3;
} isrran_pdsch_ack_t;

ISRRAN_API int
isrran_ue_dl_init(isrran_ue_dl_t* q, cf_t* input[ISRRAN_MAX_PORTS], uint32_t max_prb, uint32_t nof_rx_antennas);

ISRRAN_API void isrran_ue_dl_free(isrran_ue_dl_t* q);

ISRRAN_API int isrran_ue_dl_set_cell(isrran_ue_dl_t* q, isrran_cell_t cell);

ISRRAN_API int isrran_ue_dl_set_mbsfn_area_id(isrran_ue_dl_t* q, uint16_t mbsfn_area_id);

ISRRAN_API void isrran_ue_dl_set_non_mbsfn_region(isrran_ue_dl_t* q, uint8_t non_mbsfn_region_length);

ISRRAN_API void isrran_ue_dl_set_mi_manual(isrran_ue_dl_t* q, uint32_t mi_idx);

ISRRAN_API void isrran_ue_dl_set_mi_auto(isrran_ue_dl_t* q);

/* Perform signal demodulation and channel estimation and store signals in the object */
ISRRAN_API int isrran_ue_dl_decode_fft_estimate(isrran_ue_dl_t* q, isrran_dl_sf_cfg_t* sf, isrran_ue_dl_cfg_t* cfg);

ISRRAN_API int isrran_ue_dl_decode_fft_estimate_noguru(isrran_ue_dl_t*     q,
                                                       isrran_dl_sf_cfg_t* sf,
                                                       isrran_ue_dl_cfg_t* cfg,
                                                       cf_t*               input[ISRRAN_MAX_PORTS]);

/* Finds UL/DL DCI in the signal processed in a previous call to decode_fft_estimate() */
ISRRAN_API int isrran_ue_dl_find_ul_dci(isrran_ue_dl_t*     q,
                                        isrran_dl_sf_cfg_t* sf,
                                        isrran_ue_dl_cfg_t* dl_cfg,
                                        uint16_t            rnti,
                                        isrran_dci_ul_t     dci_msg[ISRRAN_MAX_DCI_MSG]);

ISRRAN_API int isrran_ue_dl_find_dl_dci(isrran_ue_dl_t*     q,
                                        isrran_dl_sf_cfg_t* sf,
                                        isrran_ue_dl_cfg_t* dl_cfg,
                                        uint16_t            rnti,
                                        isrran_dci_dl_t     dci_msg[ISRRAN_MAX_DCI_MSG]);

ISRRAN_API int isrran_ue_dl_dci_to_pdsch_grant(isrran_ue_dl_t*       q,
                                               isrran_dl_sf_cfg_t*   sf,
                                               isrran_ue_dl_cfg_t*   cfg,
                                               isrran_dci_dl_t*      dci,
                                               isrran_pdsch_grant_t* grant);

/* Decodes PDSCH and PHICH in the signal processed in a previous call to decode_fft_estimate() */
ISRRAN_API int isrran_ue_dl_decode_pdsch(isrran_ue_dl_t*     q,
                                         isrran_dl_sf_cfg_t* sf,
                                         isrran_pdsch_cfg_t* pdsch_cfg,
                                         isrran_pdsch_res_t  data[ISRRAN_MAX_CODEWORDS]);

ISRRAN_API int isrran_ue_dl_decode_pmch(isrran_ue_dl_t*     q,
                                        isrran_dl_sf_cfg_t* sf,
                                        isrran_pmch_cfg_t*  pmch_cfg,
                                        isrran_pdsch_res_t  data[ISRRAN_MAX_CODEWORDS]);

ISRRAN_API int isrran_ue_dl_decode_phich(isrran_ue_dl_t*       q,
                                         isrran_dl_sf_cfg_t*   sf,
                                         isrran_ue_dl_cfg_t*   cfg,
                                         isrran_phich_grant_t* grant,
                                         isrran_phich_res_t*   result);

ISRRAN_API int isrran_ue_dl_select_ri(isrran_ue_dl_t* q, uint32_t* ri, float* cn);

ISRRAN_API void isrran_ue_dl_gen_cqi_periodic(isrran_ue_dl_t*     q,
                                              isrran_ue_dl_cfg_t* cfg,
                                              uint32_t            wideband_value,
                                              uint32_t            tti,
                                              isrran_uci_data_t*  uci_data);

ISRRAN_API void isrran_ue_dl_gen_cqi_aperiodic(isrran_ue_dl_t*     q,
                                               isrran_ue_dl_cfg_t* cfg,
                                               uint32_t            wideband_value,
                                               isrran_uci_data_t*  uci_data);

ISRRAN_API void isrran_ue_dl_gen_ack(const isrran_cell_t*      cell,
                                     const isrran_dl_sf_cfg_t* sf,
                                     const isrran_pdsch_ack_t* ack_info,
                                     isrran_uci_data_t*        uci_data);

/* Functions used for testing purposes */
ISRRAN_API int isrran_ue_dl_find_and_decode(isrran_ue_dl_t*     q,
                                            isrran_dl_sf_cfg_t* sf,
                                            isrran_ue_dl_cfg_t* cfg,
                                            isrran_pdsch_cfg_t* pdsch_cfg,
                                            uint8_t*            data[ISRRAN_MAX_CODEWORDS],
                                            bool                acks[ISRRAN_MAX_CODEWORDS]);

ISRRAN_API void isrran_ue_dl_save_signal(isrran_ue_dl_t* q, isrran_dl_sf_cfg_t* sf, isrran_pdsch_cfg_t* pdsch_cfg);

#endif // ISRRAN_UE_DL_H
