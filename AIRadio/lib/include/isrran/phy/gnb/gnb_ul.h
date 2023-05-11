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

#ifndef ISRRAN_GNB_UL_H
#define ISRRAN_GNB_UL_H

#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/pucch_nr.h"
#include "isrran/phy/phch/pusch_nr.h"

typedef struct ISRRAN_API {
  isrran_pusch_nr_args_t pusch;
  isrran_pucch_nr_args_t pucch;
  float                  pusch_min_snr_dB; ///< Minimum SNR threshold to decode PUSCH, set to 0 for default value
  uint32_t               nof_max_prb;
} isrran_gnb_ul_args_t;

typedef struct ISRRAN_API {
  uint32_t            max_prb;
  isrran_carrier_nr_t carrier;

  isrran_ofdm_t fft;

  cf_t*                 sf_symbols[ISRRAN_MAX_PORTS];
  isrran_pusch_nr_t     pusch;
  isrran_pucch_nr_t     pucch;
  isrran_dmrs_sch_t     dmrs;
  isrran_chest_dl_res_t chest_pusch;
  isrran_chest_ul_res_t chest_pucch;
  float                 pusch_min_snr_dB; ///< Minimum measured DMRS SNR, below this threshold PUSCH is not decoded
} isrran_gnb_ul_t;

ISRRAN_API int isrran_gnb_ul_init(isrran_gnb_ul_t* q, cf_t* input, const isrran_gnb_ul_args_t* args);

ISRRAN_API void isrran_gnb_ul_free(isrran_gnb_ul_t* q);

ISRRAN_API int isrran_gnb_ul_set_carrier(isrran_gnb_ul_t* q, const isrran_carrier_nr_t* carrier);

ISRRAN_API int isrran_gnb_ul_fft(isrran_gnb_ul_t* q);

ISRRAN_API int isrran_gnb_ul_get_pusch(isrran_gnb_ul_t*             q,
                                       const isrran_slot_cfg_t*     slot_cfg,
                                       const isrran_sch_cfg_nr_t*   cfg,
                                       const isrran_sch_grant_nr_t* grant,
                                       isrran_pusch_res_nr_t*       data);

ISRRAN_API int isrran_gnb_ul_get_pucch(isrran_gnb_ul_t*                    q,
                                       const isrran_slot_cfg_t*            slot_cfg,
                                       const isrran_pucch_nr_common_cfg_t* cfg,
                                       const isrran_pucch_nr_resource_t*   resource,
                                       const isrran_uci_cfg_nr_t*          uci_cfg,
                                       isrran_uci_value_nr_t*              uci_value,
                                       isrran_csi_trs_measurements_t*      meas);

ISRRAN_API uint32_t isrran_gnb_ul_pucch_info(isrran_gnb_ul_t*                     q,
                                             const isrran_pucch_nr_resource_t*    resource,
                                             const isrran_uci_data_nr_t*          uci_data,
                                             const isrran_csi_trs_measurements_t* csi,
                                             char*                                str,
                                             uint32_t                             str_len);

ISRRAN_API uint32_t isrran_gnb_ul_pusch_info(isrran_gnb_ul_t*             q,
                                             const isrran_sch_cfg_nr_t*   cfg,
                                             const isrran_pusch_res_nr_t* res,
                                             char*                        str,
                                             uint32_t                     str_len);

#endif // ISRRAN_GNB_UL_H
