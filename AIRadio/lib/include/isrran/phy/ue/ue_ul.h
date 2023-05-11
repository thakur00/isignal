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
 *  File:         ue_ul.h
 *
 *  Description:  UE uplink object.
 *
 *                This module is a frontend to all the uplink data and control
 *                channel processing modules.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_UE_UL_H
#define ISRRAN_UE_UL_H

#include "isrran/phy/ch_estimation/chest_dl.h"
#include "isrran/phy/ch_estimation/refsignal_ul.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/pusch.h"
#include "isrran/phy/phch/ra.h"
#include "isrran/phy/sync/cfo.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

#include "isrran/config.h"

/* UE UL power control */
typedef struct {
  // Common configuration
  float p0_nominal_pusch;
  float alpha;
  float p0_nominal_pucch;
  float delta_f_pucch[5];
  float delta_preamble_msg3;

  // Dedicated configuration
  float p0_ue_pusch;
  bool  delta_mcs_based;
  bool  acc_enabled;
  float p0_ue_pucch;
  float p_isr_offset;
} isrran_ue_ul_powerctrl_t;

typedef struct ISRRAN_API {
  // Uplink config (includes common and dedicated variables)
  isrran_pucch_cfg_t                pucch;
  isrran_pusch_cfg_t                pusch;
  isrran_pusch_hopping_cfg_t        hopping;
  isrran_ue_ul_powerctrl_t          power_ctrl;
  isrran_refsignal_dmrs_pusch_cfg_t dmrs;
  isrran_refsignal_isr_cfg_t        isr;
} isrran_ul_cfg_t;

typedef enum {
  ISRRAN_UE_UL_NORMALIZE_MODE_AUTO = 0,
  ISRRAN_UE_UL_NORMALIZE_MODE_FORCE_AMPLITUDE
} isrran_ue_ul_normalize_mode_t;

typedef struct ISRRAN_API {
  isrran_ul_cfg_t ul_cfg;
  bool            grant_available;
  uint32_t        cc_idx;

  isrran_ue_ul_normalize_mode_t normalize_mode;
  float                         force_peak_amplitude;
  bool                          cfo_en;
  float                         cfo_tol;
  float                         cfo_value;

} isrran_ue_ul_cfg_t;

typedef struct ISRRAN_API {
  isrran_cell_t cell;

  bool signals_pregenerated;

  isrran_ofdm_t fft;
  isrran_cfo_t  cfo;

  isrran_refsignal_ul_t             signals;
  isrran_refsignal_ul_dmrs_pregen_t pregen_dmrs;
  isrran_refsignal_isr_pregen_t     pregen_isr;

  isrran_pusch_t pusch;
  isrran_pucch_t pucch;

  isrran_ra_ul_pusch_hopping_t hopping;

  isrran_cfr_cfg_t cfr_config;

  cf_t* out_buffer;
  cf_t* refsignal;
  cf_t* isr_signal;
  cf_t* sf_symbols;

} isrran_ue_ul_t;

ISRRAN_API int isrran_ue_ul_init(isrran_ue_ul_t* q, cf_t* out_buffer, uint32_t max_prb);

ISRRAN_API void isrran_ue_ul_free(isrran_ue_ul_t* q);

ISRRAN_API int isrran_ue_ul_set_cell(isrran_ue_ul_t* q, isrran_cell_t cell);

ISRRAN_API int isrran_ue_ul_set_cfr(isrran_ue_ul_t* q, const isrran_cfr_cfg_t* cfr);

ISRRAN_API int isrran_ue_ul_pregen_signals(isrran_ue_ul_t* q, isrran_ue_ul_cfg_t* cfg);

ISRRAN_API int isrran_ue_ul_dci_to_pusch_grant(isrran_ue_ul_t*       q,
                                               isrran_ul_sf_cfg_t*   sf,
                                               isrran_ue_ul_cfg_t*   cfg,
                                               isrran_dci_ul_t*      dci,
                                               isrran_pusch_grant_t* grant);

ISRRAN_API void isrran_ue_ul_pusch_hopping(isrran_ue_ul_t*       q,
                                           isrran_ul_sf_cfg_t*   sf,
                                           isrran_ue_ul_cfg_t*   cfg,
                                           isrran_pusch_grant_t* grant);

ISRRAN_API int
isrran_ue_ul_encode(isrran_ue_ul_t* q, isrran_ul_sf_cfg_t* sf, isrran_ue_ul_cfg_t* cfg, isrran_pusch_data_t* data);

ISRRAN_API int isrran_ue_ul_sr_send_tti(const isrran_pucch_cfg_t* cfg, uint32_t current_tti);

ISRRAN_API bool
isrran_ue_ul_gen_sr(isrran_ue_ul_cfg_t* cfg, isrran_ul_sf_cfg_t* sf, isrran_uci_data_t* uci_data, bool sr_request);

/**
 * Determines the PUCCH resource selection according to 3GPP 36.213 R10 Section 10.1. The PUCCH format and resource are
 * saved in cfg->format and cfg->n_pucch. Also, HARQ-ACK
 *
 * @param cell Cell parameter, non-modifiable
 * @param cfg PUCCH configuration and contains function results
 * @param uci_cfg UCI configuration
 * @param uci_data UCI data
 * @param b Modified bits after applying HARQ-ACK feedback mode "encoding"
 */
ISRRAN_API void isrran_ue_ul_pucch_resource_selection(const isrran_cell_t*      cell,
                                                      isrran_pucch_cfg_t*       cfg,
                                                      const isrran_uci_cfg_t*   uci_cfg,
                                                      const isrran_uci_value_t* uci_value,
                                                      uint8_t                   b[ISRRAN_UCI_MAX_ACK_BITS]);

ISRRAN_API bool isrran_ue_ul_info(isrran_ue_ul_cfg_t* cfg,
                                  isrran_ul_sf_cfg_t* sf,
                                  isrran_uci_value_t* uci_data,
                                  char*               str,
                                  uint32_t            str_len);

#endif // ISRRAN_UE_UL_H
