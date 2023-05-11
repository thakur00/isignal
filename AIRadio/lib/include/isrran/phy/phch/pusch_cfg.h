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

#ifndef ISRRAN_PUSCH_CFG_H
#define ISRRAN_PUSCH_CFG_H

#include "isrran/phy/fec/softbuffer.h"
#include "isrran/phy/phch/ra.h"
#include "isrran/phy/phch/uci_cfg.h"

typedef struct ISRRAN_API {
  uint32_t I_offset_cqi;
  uint32_t I_offset_ri;
  uint32_t I_offset_ack;
} isrran_uci_offset_cfg_t;

typedef struct {
  enum { ISRRAN_PUSCH_HOP_MODE_INTER_SF = 1, ISRRAN_PUSCH_HOP_MODE_INTRA_SF = 0 } hop_mode;
  uint32_t hopping_offset;
  uint32_t n_sb;
  uint32_t n_rb_ho;
  uint32_t current_tx_nb;
  bool     hopping_enabled;
} isrran_pusch_hopping_cfg_t;

typedef struct ISRRAN_API {

  uint32_t       L_prb;
  uint32_t       n_prb[2];       // rb_start after frequency hopping
  uint32_t       n_prb_tilde[2]; // rb_start after frequency hopping per retx
  uint32_t       freq_hopping;
  uint32_t       nof_re;
  uint32_t       nof_symb;
  isrran_ra_tb_t tb;
  isrran_ra_tb_t last_tb;
  uint32_t       n_dmrs;
  bool           is_rar;

} isrran_pusch_grant_t;

typedef struct ISRRAN_API {

  uint16_t rnti;

  isrran_uci_cfg_t        uci_cfg;
  isrran_uci_offset_cfg_t uci_offset;
  isrran_pusch_grant_t    grant;

  uint32_t max_nof_iterations;
  uint32_t last_O_cqi;
  uint32_t K_segm;
  uint32_t current_tx_nb;
  bool     csi_enable;
  bool     enable_64qam;

  union {
    isrran_softbuffer_tx_t* tx;
    isrran_softbuffer_rx_t* rx;
  } softbuffers;

  bool     meas_time_en;
  uint32_t meas_time_value;

  bool meas_epre_en;
  bool meas_ta_en;
  bool meas_evm_en;

} isrran_pusch_cfg_t;

#endif // ISRRAN_PUSCH_CFG_H
