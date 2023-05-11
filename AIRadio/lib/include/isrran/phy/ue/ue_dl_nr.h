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

#ifndef ISRRAN_UE_DL_NR_H
#define ISRRAN_UE_DL_NR_H

#include "isrran/phy/ch_estimation/csi_rs.h"
#include "isrran/phy/ch_estimation/dmrs_pdcch.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/csi.h"
#include "isrran/phy/phch/dci_nr.h"
#include "isrran/phy/phch/pdcch_cfg_nr.h"
#include "isrran/phy/phch/pdcch_nr.h"
#include "isrran/phy/phch/pdsch_nr.h"
#include "isrran/phy/phch/uci_cfg_nr.h"

/**
 * Maximum number of DCI messages to receive
 */
#define ISRRAN_MAX_DCI_MSG_NR 4

typedef struct ISRRAN_API {
  isrran_pdsch_nr_args_t pdsch;
  isrran_pdcch_nr_args_t pdcch;
  uint32_t               nof_rx_antennas;
  uint32_t               nof_max_prb;
  float                  pdcch_dmrs_corr_thr;
  float                  pdcch_dmrs_epre_thr;
} isrran_ue_dl_nr_args_t;

typedef struct ISRRAN_API {
  isrran_dci_ctx_t            dci_ctx;
  isrran_dmrs_pdcch_measure_t measure;
  isrran_pdcch_nr_res_t       result;
  uint32_t                    nof_bits;
} isrran_ue_dl_nr_pdcch_info_t;

typedef struct ISRRAN_API {
  uint32_t max_prb;
  uint32_t nof_rx_antennas;
  float    pdcch_dmrs_corr_thr;
  float    pdcch_dmrs_epre_thr;

  isrran_carrier_nr_t   carrier;
  isrran_pdcch_cfg_nr_t cfg;

  isrran_ofdm_t fft[ISRRAN_MAX_PORTS];

  cf_t*                 sf_symbols[ISRRAN_MAX_PORTS];
  isrran_chest_dl_res_t chest;
  isrran_pdsch_nr_t     pdsch;
  isrran_dmrs_sch_t     dmrs_pdsch;

  isrran_dmrs_pdcch_estimator_t dmrs_pdcch[ISRRAN_UE_DL_NR_MAX_NOF_CORESET];
  isrran_pdcch_nr_t             pdcch;
  isrran_dmrs_pdcch_ce_t*       pdcch_ce;

  /// Store Blind-search information from all possible candidate locations for debug purposes
  isrran_ue_dl_nr_pdcch_info_t pdcch_info[ISRRAN_MAX_NOF_CANDIDATES_SLOT_NR];
  uint32_t                     pdcch_info_count;

  /// DCI packing/unpacking object
  isrran_dci_nr_t dci;

  /// Temporally stores Found DCI messages from all SS
  isrran_dci_msg_nr_t dl_dci_msg[ISRRAN_MAX_DCI_MSG_NR];
  uint32_t            dl_dci_msg_count;

  isrran_dci_msg_nr_t ul_dci_msg[ISRRAN_MAX_DCI_MSG_NR];
  uint32_t            ul_dci_count;
} isrran_ue_dl_nr_t;

ISRRAN_API int
isrran_ue_dl_nr_init(isrran_ue_dl_nr_t* q, cf_t* input[ISRRAN_MAX_PORTS], const isrran_ue_dl_nr_args_t* args);

ISRRAN_API int isrran_ue_dl_nr_set_carrier(isrran_ue_dl_nr_t* q, const isrran_carrier_nr_t* carrier);

ISRRAN_API int isrran_ue_dl_nr_set_pdcch_config(isrran_ue_dl_nr_t*           q,
                                                const isrran_pdcch_cfg_nr_t* cfg,
                                                const isrran_dci_cfg_nr_t*   dci_cfg);

ISRRAN_API void isrran_ue_dl_nr_free(isrran_ue_dl_nr_t* q);

ISRRAN_API void isrran_ue_dl_nr_estimate_fft(isrran_ue_dl_nr_t* q, const isrran_slot_cfg_t* slot_cfg);

ISRRAN_API int isrran_ue_dl_nr_find_dl_dci(isrran_ue_dl_nr_t*       q,
                                           const isrran_slot_cfg_t* slot_cfg,
                                           uint16_t                 rnti,
                                           isrran_rnti_type_t       rnti_type,
                                           isrran_dci_dl_nr_t*      dci_dl_list,
                                           uint32_t                 nof_dci_msg);

ISRRAN_API int isrran_ue_dl_nr_find_ul_dci(isrran_ue_dl_nr_t*       q,
                                           const isrran_slot_cfg_t* slot_cfg,
                                           uint16_t                 rnti,
                                           isrran_rnti_type_t       rnti_type,
                                           isrran_dci_ul_nr_t*      dci_ul_list,
                                           uint32_t                 nof_dci_msg);

ISRRAN_API int isrran_ue_dl_nr_decode_pdsch(isrran_ue_dl_nr_t*         q,
                                            const isrran_slot_cfg_t*   slot,
                                            const isrran_sch_cfg_nr_t* cfg,
                                            isrran_pdsch_res_nr_t*     res);

ISRRAN_API uint32_t isrran_ue_dl_nr_pdsch_info(const isrran_ue_dl_nr_t*    q,
                                               const isrran_sch_cfg_nr_t*  cfg,
                                               const isrran_pdsch_res_nr_t res[ISRRAN_MAX_CODEWORDS],
                                               char*                       str,
                                               uint32_t                    str_len);

ISRRAN_API
int isrran_ue_dl_nr_csi_measure_trs(const isrran_ue_dl_nr_t*       q,
                                    const isrran_slot_cfg_t*       slot_cfg,
                                    const isrran_csi_rs_nzp_set_t* csi_rs_nzp_set,
                                    isrran_csi_trs_measurements_t* measurement);

ISRRAN_API
int isrran_ue_dl_nr_csi_measure_channel(const isrran_ue_dl_nr_t*           q,
                                        const isrran_slot_cfg_t*           slot_cfg,
                                        const isrran_csi_rs_nzp_set_t*     csi_rs_nzp_set,
                                        isrran_csi_channel_measurements_t* measurement);

#endif // ISRRAN_UE_DL_NR_H
