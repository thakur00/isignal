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
 *  File:         cqi.h
 *
 *  Description:  Channel quality indicator message packing.
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10 Sec. 5.2.2.6, 5.2.3.3
 *****************************************************************************/

#ifndef ISRRAN_CQI_H
#define ISRRAN_CQI_H

#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"

#define ISRRAN_CQI_MAX_BITS 64
#define ISRRAN_DIF_CQI_MAX_BITS 3
#define ISRRAN_PMI_MAX_BITS 4
#define ISRRAN_CQI_STR_MAX_CHAR 64

typedef enum {
  ISRRAN_CQI_MODE_10,
  ISRRAN_CQI_MODE_11,
  ISRRAN_CQI_MODE_12,
  ISRRAN_CQI_MODE_20,
  ISRRAN_CQI_MODE_21,
  ISRRAN_CQI_MODE_22,
  ISRRAN_CQI_MODE_30,
  ISRRAN_CQI_MODE_31,
  ISRRAN_CQI_MODE_NA,
} isrran_cqi_report_mode_t;

typedef struct {
  bool                     periodic_configured;
  bool                     aperiodic_configured;
  uint16_t                 pmi_idx;
  uint32_t                 ri_idx;
  bool                     ri_idx_present;
  bool                     format_is_subband;
  uint8_t                  subband_wideband_ratio; ///< K value in TS 36.331. 0 for wideband reporting, (1..4) otherwise
  isrran_cqi_report_mode_t periodic_mode;
  isrran_cqi_report_mode_t aperiodic_mode;
} isrran_cqi_report_cfg_t;

/* Table 5.2.2.6.2-1: Fields for channel quality information feedback for higher layer configured subband
   CQI reports  (transmission mode 1, transmission mode 2, transmission mode 3, transmission mode 7 and
   transmission mode 8 configured without PMI/RI reporting). */

/* Table 5.2.2.6.2-2: Fields for channel quality information (CQI) feedback for higher layer configured subband CQI
   reports (transmission mode 4, transmission mode 5 and transmission mode 6). */

typedef struct ISRRAN_API {
  uint8_t  wideband_cqi_cw0;     // 4-bit width
  uint32_t subband_diff_cqi_cw0; // 2N-bit width
  uint8_t  wideband_cqi_cw1;     // if RI > 1 then 4-bit width otherwise 0-bit width
  uint32_t subband_diff_cqi_cw1; // if RI > 1 then 2N-bit width otherwise 0-bit width
  uint32_t pmi;                  // if RI > 1 then 2-bit width otherwise 1-bit width
} isrran_cqi_hl_subband_t;

/* Table 5.2.2.6.3-1: Fields for channel quality information feedback for UE selected subband CQI
reports
(transmission mode 1, transmission mode 2, transmission mode 3, transmission mode 7 and
transmission mode 8 configured without PMI/RI reporting). */
typedef struct ISRRAN_API {
  uint8_t  wideband_cqi;     // 4-bit width
  uint8_t  subband_diff_cqi; // 2-bit width
  uint32_t position_subband; // L-bit width
} isrran_cqi_ue_diff_subband_t;

/* Table 5.2.3.3.1-1: Fields for channel quality information feedback for wideband CQI reports
(transmission mode 1, transmission mode 2, transmission mode 3, transmission mode 7 and
transmission mode 8 configured without PMI/RI reporting).
This is for PUCCH Format 2 reports
*/

/* Table 5.2.3.3.1-2: UCI fields for channel quality and precoding information (CQI/PMI) feedback for
wideband reports (transmission mode 4, transmission mode 5 and transmission mode 6)
This is for PUCCH Format 2 reports
*/

typedef struct ISRRAN_API {
  uint8_t wideband_cqi;     // 4-bit width
  uint8_t spatial_diff_cqi; // If Rank==1 then it is 0-bit width otherwise it is 3-bit width
  uint8_t pmi;
} isrran_cqi_format2_wideband_t;

typedef struct ISRRAN_API {
  uint8_t subband_cqi;   // 4-bit width
  uint8_t subband_label; // 1- or 2-bit width
} isrran_cqi_ue_subband_t;

typedef enum {
  ISRRAN_CQI_TYPE_WIDEBAND = 0,
  ISRRAN_CQI_TYPE_SUBBAND_UE,
  ISRRAN_CQI_TYPE_SUBBAND_UE_DIFF,
  ISRRAN_CQI_TYPE_SUBBAND_HL
} isrran_cqi_type_t;

typedef struct ISRRAN_API {
  bool              data_enable;
  bool              pmi_present;
  bool              four_antenna_ports;   ///< If cell has 4 antenna ports then true otherwise false
  bool              rank_is_not_one;      ///< If rank > 1 then true otherwise false
  bool              subband_label_2_bits; ///< false, label=1-bit, true label=2-ack_value
  uint32_t          scell_index;          ///< Indicates the cell/carrier the measurement belongs, use 0 for PCell
  uint32_t          L;
  uint32_t          N;
  uint32_t          sb_idx;
  isrran_cqi_type_t type;
  uint32_t          ri_len;
} isrran_cqi_cfg_t;

typedef struct {
  union {
    isrran_cqi_format2_wideband_t wideband;
    isrran_cqi_ue_subband_t       subband_ue;
    isrran_cqi_ue_diff_subband_t  subband_ue_diff;
    isrran_cqi_hl_subband_t       subband_hl;
  };
  bool data_crc;
} isrran_cqi_value_t;

ISRRAN_API int isrran_cqi_size(isrran_cqi_cfg_t* cfg);

ISRRAN_API int
isrran_cqi_value_pack(isrran_cqi_cfg_t* cfg, isrran_cqi_value_t* value, uint8_t buff[ISRRAN_CQI_MAX_BITS]);

ISRRAN_API int
isrran_cqi_value_unpack(isrran_cqi_cfg_t* cfg, uint8_t buff[ISRRAN_CQI_MAX_BITS], isrran_cqi_value_t* value);

ISRRAN_API int
isrran_cqi_value_tostring(isrran_cqi_cfg_t* cfg, isrran_cqi_value_t* value, char* buff, uint32_t buff_len);

ISRRAN_API bool
isrran_cqi_periodic_send(const isrran_cqi_report_cfg_t* periodic_cfg, uint32_t tti, isrran_frame_type_t frame_type);

ISRRAN_API bool isrran_cqi_periodic_is_subband(const isrran_cqi_report_cfg_t* cfg,
                                               uint32_t                       tti,
                                               uint32_t                       nof_prb,
                                               isrran_frame_type_t            frame_type);

ISRRAN_API bool
isrran_cqi_periodic_ri_send(const isrran_cqi_report_cfg_t* periodic_cfg, uint32_t tti, isrran_frame_type_t frame_type);

ISRRAN_API uint32_t isrran_cqi_periodic_sb_bw_part_idx(const isrran_cqi_report_cfg_t* cfg,
                                                       uint32_t                       tti,
                                                       uint32_t                       nof_prb,
                                                       isrran_frame_type_t            frame_type);

ISRRAN_API int isrran_cqi_hl_get_no_subbands(int nof_prb);

/**
 * @brief Returns the number of bits to index a bandwidth part (L)
 *
 * @remark Implemented according to TS 38.213 section 7.2.2 Periodic CSI Reporting using PUCCH, paragraph that refers to
 * `L-bit label indexed in the order of increasing frequency, where L = ceil(log2(nof_prb/k/J))`
 *
 */
ISRRAN_API int isrran_cqi_hl_get_L(int nof_prb);

ISRRAN_API uint32_t isrran_cqi_get_sb_idx(uint32_t                       tti,
                                          uint32_t                       subband_label,
                                          const isrran_cqi_report_cfg_t* cqi_report_cfg,
                                          const isrran_cell_t*           cell);

ISRRAN_API uint8_t isrran_cqi_from_snr(float snr);

ISRRAN_API float isrran_cqi_to_coderate(uint32_t cqi, bool use_alt_table);

#endif // ISRRAN_CQI_H
