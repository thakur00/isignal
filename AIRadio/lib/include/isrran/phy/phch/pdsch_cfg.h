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
 *  File:         pdsch_cfg.h
 *
 *  Description:  Physical downlink shared channel configuration
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.4
 *****************************************************************************/

#ifndef ISRRAN_PDSCH_CFG_H
#define ISRRAN_PDSCH_CFG_H

#include "isrran/phy/fec/cbsegm.h"
#include "isrran/phy/fec/softbuffer.h"
#include "isrran/phy/phch/ra.h"

typedef struct ISRRAN_API {
  isrran_tx_scheme_t tx_scheme;
  uint32_t           pmi;
  bool               prb_idx[2][ISRRAN_MAX_PRB];
  uint32_t           nof_prb;
  uint32_t           nof_re;
  uint32_t           nof_symb_slot[2];
  isrran_ra_tb_t     tb[ISRRAN_MAX_CODEWORDS];
  int                last_tbs[ISRRAN_MAX_CODEWORDS];
  uint32_t           nof_tb;
  uint32_t           nof_layers;
} isrran_pdsch_grant_t;

typedef struct ISRRAN_API {
  isrran_pdsch_grant_t grant;

  uint16_t              rnti;
  uint32_t              max_nof_iterations;
  isrran_mimo_decoder_t decoder_type;
  float                 p_a;
  uint32_t              p_b;
  float                 rs_power;
  bool                  power_scale;
  bool                  csi_enable;
  bool                  use_tbs_index_alt;

  union {
    isrran_softbuffer_tx_t* tx[ISRRAN_MAX_CODEWORDS];
    isrran_softbuffer_rx_t* rx[ISRRAN_MAX_CODEWORDS];
  } softbuffers;

  bool     meas_evm_en;
  bool     meas_time_en;
  uint32_t meas_time_value;
} isrran_pdsch_cfg_t;

#endif // ISRRAN_PDSCH_CFG_H
