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

#ifndef ISRRAN_GNB_DL_H
#define ISRRAN_GNB_DL_H

#include "isrran/phy/ch_estimation/csi_rs.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/pdcch_cfg_nr.h"
#include "isrran/phy/phch/pdcch_nr.h"
#include "isrran/phy/phch/pdsch_nr.h"
#include "isrran/phy/sync/ssb.h"

typedef struct ISRRAN_API {
  isrran_pdsch_nr_args_t      pdsch;
  isrran_pdcch_nr_args_t      pdcch;
  uint32_t                    nof_tx_antennas;
  uint32_t                    nof_max_prb; ///< Maximum number of allocated RB
  double                      srate_hz;    ///< Fix sampling rate, set to 0 for minimum to fit nof_max_prb
  isrran_subcarrier_spacing_t scs;
} isrran_gnb_dl_args_t;

typedef struct ISRRAN_API {
  float                 srate_hz;
  uint32_t              symbol_sz;
  uint32_t              max_prb;
  uint32_t              nof_tx_antennas;
  isrran_carrier_nr_t   carrier;
  isrran_pdcch_cfg_nr_t pdcch_cfg;

  isrran_ofdm_t fft[ISRRAN_MAX_PORTS];

  cf_t*             sf_symbols[ISRRAN_MAX_PORTS];
  isrran_pdsch_nr_t pdsch;
  isrran_dmrs_sch_t dmrs;

  isrran_dci_nr_t   dci; ///< Stores DCI configuration
  isrran_pdcch_nr_t pdcch;
  isrran_ssb_t      ssb;
} isrran_gnb_dl_t;

ISRRAN_API int isrran_gnb_dl_init(isrran_gnb_dl_t* q, cf_t* output[ISRRAN_MAX_PORTS], const isrran_gnb_dl_args_t* args);

ISRRAN_API int isrran_gnb_dl_set_carrier(isrran_gnb_dl_t* q, const isrran_carrier_nr_t* carrier);

ISRRAN_API int isrran_gnb_dl_set_ssb_config(isrran_gnb_dl_t* q, const isrran_ssb_cfg_t* ssb);

ISRRAN_API int isrran_gnb_dl_set_pdcch_config(isrran_gnb_dl_t*             q,
                                              const isrran_pdcch_cfg_nr_t* cfg,
                                              const isrran_dci_cfg_nr_t*   dci_cfg);

ISRRAN_API void isrran_gnb_dl_free(isrran_gnb_dl_t* q);

ISRRAN_API int isrran_gnb_dl_base_zero(isrran_gnb_dl_t* q);

ISRRAN_API void isrran_gnb_dl_gen_signal(isrran_gnb_dl_t* q);

ISRRAN_API int isrran_gnb_dl_add_ssb(isrran_gnb_dl_t* q, const isrran_pbch_msg_nr_t* pbch_msg, uint32_t sf_idx);

ISRRAN_API int
isrran_gnb_dl_pdcch_put_dl(isrran_gnb_dl_t* q, const isrran_slot_cfg_t* slot_cfg, const isrran_dci_dl_nr_t* dci_dl);

ISRRAN_API int
isrran_gnb_dl_pdcch_put_ul(isrran_gnb_dl_t* q, const isrran_slot_cfg_t* slot_cfg, const isrran_dci_ul_nr_t* dci_ul);

ISRRAN_API int isrran_gnb_dl_pdsch_put(isrran_gnb_dl_t*           q,
                                       const isrran_slot_cfg_t*   slot,
                                       const isrran_sch_cfg_nr_t* cfg,
                                       uint8_t*                   data[ISRRAN_MAX_TB]);

ISRRAN_API float isrran_gnb_dl_get_maximum_signal_power_dBfs(uint32_t nof_prb);

ISRRAN_API int
isrran_gnb_dl_pdsch_info(const isrran_gnb_dl_t* q, const isrran_sch_cfg_nr_t* cfg, char* str, uint32_t str_len);

ISRRAN_API int
isrran_gnb_dl_pdcch_dl_info(const isrran_gnb_dl_t* q, const isrran_dci_dl_nr_t* dci, char* str, uint32_t str_len);

ISRRAN_API int
isrran_gnb_dl_pdcch_ul_info(const isrran_gnb_dl_t* q, const isrran_dci_ul_nr_t* dci, char* str, uint32_t str_len);

ISRRAN_API int isrran_gnb_dl_nzp_csi_rs_put(isrran_gnb_dl_t*                    q,
                                            const isrran_slot_cfg_t*            slot_cfg,
                                            const isrran_csi_rs_nzp_resource_t* resource);

#endif // ISRRAN_GNB_DL_H
