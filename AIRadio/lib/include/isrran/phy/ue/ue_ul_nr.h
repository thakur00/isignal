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
 *  @file ue_dl_nr.h
 *
 *  Description:  NR UE uplink physical layer procedures for data
 *
 *                This module is a frontend to all the uplink data channel processing modules.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_UE_UL_DATA_H
#define ISRRAN_UE_UL_DATA_H

#include "isrran/phy/ch_estimation/dmrs_sch.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/phch_cfg_nr.h"
#include "isrran/phy/phch/pucch_cfg_nr.h"
#include "isrran/phy/phch/pucch_nr.h"
#include "isrran/phy/phch/pusch_nr.h"

typedef struct ISRRAN_API {
  isrran_pusch_nr_args_t pusch;
  isrran_pucch_nr_args_t pucch;
  uint32_t               nof_max_prb;
} isrran_ue_ul_nr_args_t;

typedef struct ISRRAN_API {
  uint32_t max_prb;

  isrran_carrier_nr_t carrier;

  isrran_ofdm_t ifft;

  cf_t*             sf_symbols[ISRRAN_MAX_PORTS];
  isrran_pusch_nr_t pusch;
  isrran_pucch_nr_t pucch;
  isrran_dmrs_sch_t dmrs;

  float freq_offset_hz;
} isrran_ue_ul_nr_t;

ISRRAN_API int isrran_ue_ul_nr_init(isrran_ue_ul_nr_t* q, cf_t* output, const isrran_ue_ul_nr_args_t* args);

ISRRAN_API int isrran_ue_ul_nr_set_carrier(isrran_ue_ul_nr_t* q, const isrran_carrier_nr_t* carrier);

ISRRAN_API void isrran_ue_ul_nr_set_freq_offset(isrran_ue_ul_nr_t* q, float freq_offset_hz);

ISRRAN_API int isrran_ue_ul_nr_encode_pusch(isrran_ue_ul_nr_t*            q,
                                            const isrran_slot_cfg_t*      slot_cfg,
                                            const isrran_sch_cfg_nr_t*    pusch_cfg,
                                            const isrran_pusch_data_nr_t* data);

ISRRAN_API int isrran_ue_ul_nr_encode_pucch(isrran_ue_ul_nr_t*                  q,
                                            const isrran_slot_cfg_t*            slot_cfg,
                                            const isrran_pucch_nr_common_cfg_t* cfg,
                                            const isrran_pucch_nr_resource_t*   resource,
                                            const isrran_uci_data_nr_t*         uci_data);

ISRRAN_API void isrran_ue_ul_nr_free(isrran_ue_ul_nr_t* q);

ISRRAN_API int isrran_ue_ul_nr_pusch_info(const isrran_ue_ul_nr_t*     q,
                                          const isrran_sch_cfg_nr_t*   cfg,
                                          const isrran_uci_value_nr_t* uci_value,
                                          char*                        str,
                                          uint32_t                     str_len);

ISRRAN_API int isrran_ue_ul_nr_pucch_info(const isrran_pucch_nr_resource_t* resource,
                                          const isrran_uci_data_nr_t*       uci_data,
                                          char*                             str,
                                          uint32_t                          str_len);

/**
 * @brief Decides whether the provided slot index within the radio frame is a SR transmission opportunity
 *
 * @remark Implemented according to TS 38.213 9.2.4 UE procedure for reporting SR
 *
 * @param sr_resources Provides the SR configuration from the upper layers
 * @param slot_idx Slot index in the radio frame
 * @param[out] sr_resource_id Optional SR resource index (or identifier)
 *
 * @return the number of SR opportunities if the provided slot index is a SR transmission opportunity, ISRRAN_ERROR code
 * if provided parameters are invalid
 */
ISRRAN_API int
isrran_ue_ul_nr_sr_send_slot(const isrran_pucch_nr_sr_resource_t sr_resources[ISRRAN_PUCCH_MAX_NOF_SR_RESOURCES],
                             uint32_t                            slot_idx,
                             uint32_t                            sr_resource_id[ISRRAN_PUCCH_MAX_NOF_SR_RESOURCES]);

#endif // ISRRAN_UE_UL_DATA_H
