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

#ifndef ISRRAN_PUCCH_CFG_H
#define ISRRAN_PUCCH_CFG_H

#include "isrran/phy/phch/cqi.h"
#include "isrran/phy/phch/ra.h"
#include "isrran/phy/phch/uci_cfg.h"

#define ISRRAN_PUCCH_SIZE_AN_CS 4
#define ISRRAN_PUCCH_SIZE_AN_N3 4
#define ISRRAN_PUCCH_NOF_AN_CS 2
#define ISRRAN_PUCCH_MAX_BITS ISRRAN_CQI_MAX_BITS

typedef enum ISRRAN_API {
  ISRRAN_PUCCH_FORMAT_1 = 0,
  ISRRAN_PUCCH_FORMAT_1A,
  ISRRAN_PUCCH_FORMAT_1B,
  ISRRAN_PUCCH_FORMAT_2,
  ISRRAN_PUCCH_FORMAT_2A,
  ISRRAN_PUCCH_FORMAT_2B,
  ISRRAN_PUCCH_FORMAT_3,
  ISRRAN_PUCCH_FORMAT_ERROR,
} isrran_pucch_format_t;

typedef struct ISRRAN_API {
  // Input configuration for this subframe
  uint16_t rnti;

  // UCI configuration
  isrran_uci_cfg_t uci_cfg;

  // Common configuration
  uint32_t delta_pucch_shift;
  uint32_t n_rb_2;
  uint32_t N_cs;
  uint32_t N_pucch_1;
  bool     group_hopping_en; // common pusch config

  // Dedicated PUCCH configuration
  uint32_t I_sr;
  bool     sr_configured;
  uint32_t n_pucch_1[4]; // 4 n_pucch resources specified by RRC
  uint32_t n_pucch_2;
  uint32_t n_pucch_sr;
  bool     simul_cqi_ack;
  bool     tdd_ack_multiplex; // if false, bundle
  bool     sps_enabled;

  // Release 10 CA specific
  isrran_ack_nack_feedback_mode_t ack_nack_feedback_mode;
  uint32_t                        n1_pucch_an_cs[ISRRAN_PUCCH_SIZE_AN_CS][ISRRAN_PUCCH_NOF_AN_CS];
  uint32_t                        n3_pucch_an_list[ISRRAN_PUCCH_SIZE_AN_N3];

  // Other configuration
  float threshold_format1;
  float threshold_data_valid_format1a;
  float threshold_data_valid_format2;
  float threshold_data_valid_format3;
  float threshold_dmrs_detection;
  bool  meas_ta_en;

  // PUCCH configuration generated during a call to encode/decode
  isrran_pucch_format_t format;
  uint16_t              n_pucch;
  uint8_t               pucch2_drs_bits[ISRRAN_PUCCH_MAX_BITS];

} isrran_pucch_cfg_t;

#endif // ISRRAN_PUCCH_CFG_H
