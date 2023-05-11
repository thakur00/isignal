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
 *  File:         enb_dl.h
 *
 *  Description:  ENB downlink object.
 *
 *                This module is a frontend to all the downlink data and control
 *                channel processing modules for the ENB transmitter side.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_ENB_DL_H
#define ISRRAN_ENB_DL_H

#include <stdbool.h>

#include "isrran/phy/ch_estimation/refsignal_dl.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/ofdm.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/pbch.h"
#include "isrran/phy/phch/pcfich.h"
#include "isrran/phy/phch/pdcch.h"
#include "isrran/phy/phch/pdsch.h"
#include "isrran/phy/phch/pdsch_cfg.h"
#include "isrran/phy/phch/phich.h"
#include "isrran/phy/phch/pmch.h"
#include "isrran/phy/phch/ra.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/sync/pss.h"
#include "isrran/phy/sync/sss.h"

#include "isrran/phy/enb/enb_ul.h"
#include "isrran/phy/ue/ue_dl.h"

#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

#include "isrran/config.h"

typedef struct ISRRAN_API {
  isrran_cell_t cell;

  isrran_dl_sf_cfg_t dl_sf;

  isrran_cfr_cfg_t cfr_config;

  cf_t*         sf_symbols[ISRRAN_MAX_PORTS];
  cf_t*         out_buffer[ISRRAN_MAX_PORTS];
  isrran_ofdm_t ifft[ISRRAN_MAX_PORTS];
  isrran_ofdm_t ifft_mbsfn;

  isrran_pbch_t   pbch;
  isrran_pcfich_t pcfich;
  isrran_regs_t   regs;
  isrran_pdcch_t  pdcch;
  isrran_pdsch_t  pdsch;
  isrran_pmch_t   pmch;
  isrran_phich_t  phich;

  isrran_refsignal_t csr_signal;
  isrran_refsignal_t mbsfnr_signal;

  cf_t  pss_signal[ISRRAN_PSS_LEN];
  float sss_signal0[ISRRAN_SSS_LEN];
  float sss_signal5[ISRRAN_SSS_LEN];

  uint32_t              nof_common_locations[3];
  isrran_dci_location_t common_locations[3][ISRRAN_MAX_CANDIDATES_COM];

} isrran_enb_dl_t;

typedef struct {
  uint8_t  ack;
  uint32_t n_prb_lowest;
  uint32_t n_dmrs;
} isrran_enb_dl_phich_t;

/* This function shall be called just after the initial synchronization */
ISRRAN_API int isrran_enb_dl_init(isrran_enb_dl_t* q, cf_t* out_buffer[ISRRAN_MAX_PORTS], uint32_t max_prb);

ISRRAN_API void isrran_enb_dl_free(isrran_enb_dl_t* q);

ISRRAN_API int isrran_enb_dl_set_cell(isrran_enb_dl_t* q, isrran_cell_t cell);

ISRRAN_API int isrran_enb_dl_set_cfr(isrran_enb_dl_t* q, const isrran_cfr_cfg_t* cfr);

ISRRAN_API bool isrran_enb_dl_location_is_common_ncce(isrran_enb_dl_t* q, const isrran_dci_location_t* loc);

ISRRAN_API void isrran_enb_dl_put_base(isrran_enb_dl_t* q, isrran_dl_sf_cfg_t* dl_sf);

ISRRAN_API void isrran_enb_dl_put_phich(isrran_enb_dl_t* q, isrran_phich_grant_t* grant, bool ack);

ISRRAN_API int isrran_enb_dl_put_pdcch_dl(isrran_enb_dl_t* q, isrran_dci_cfg_t* dci_cfg, isrran_dci_dl_t* dci_dl);

ISRRAN_API int isrran_enb_dl_put_pdcch_ul(isrran_enb_dl_t* q, isrran_dci_cfg_t* dci_cfg, isrran_dci_ul_t* dci_ul);

ISRRAN_API int
isrran_enb_dl_put_pdsch(isrran_enb_dl_t* q, isrran_pdsch_cfg_t* pdsch, uint8_t* data[ISRRAN_MAX_CODEWORDS]);

ISRRAN_API int isrran_enb_dl_put_pmch(isrran_enb_dl_t* q, isrran_pmch_cfg_t* pmch_cfg, uint8_t* data);

ISRRAN_API void isrran_enb_dl_gen_signal(isrran_enb_dl_t* q);

ISRRAN_API bool isrran_enb_dl_gen_cqi_periodic(const isrran_cell_t*   cell,
                                               const isrran_dl_cfg_t* dl_cfg,
                                               uint32_t               tti,
                                               uint32_t               last_ri,
                                               isrran_cqi_cfg_t*      cqi_cfg);

ISRRAN_API bool isrran_enb_dl_gen_cqi_aperiodic(const isrran_cell_t*   cell,
                                                const isrran_dl_cfg_t* dl_cfg,
                                                uint32_t               ri,
                                                isrran_cqi_cfg_t*      cqi_cfg);

ISRRAN_API void isrran_enb_dl_save_signal(isrran_enb_dl_t* q);

/**
 * Generates the uplink control information configuration from the cell, subframe and HARQ ACK information. Note that
 * it expects the UCI configuration shall have been configured already with scheduling request and channel quality
 * information prior to this call.
 *
 * @param cell points to the physical layer cell parameters
 * @param sf points to the subframe configuration
 * @param ack_info is the HARQ-ACK information
 * @param uci_cfg the UCI configuration destination
 */
ISRRAN_API void isrran_enb_dl_gen_ack(const isrran_cell_t*      cell,
                                      const isrran_dl_sf_cfg_t* sf,
                                      const isrran_pdsch_ack_t* ack_info,
                                      isrran_uci_cfg_t*         uci_cfg);

/**
 * gets the HARQ-ACK values from the received Uplink Control Information configuration, the cell, and HARQ ACK
 * info itself. Note that it expects that the HARQ-ACK info has been set prior the UCI Data decoding.
 *
 * @param cell points to the physical layer cell parameters
 * @param uci_cfg points to the UCI configration
 * @param uci_value points to the received UCI values
 * @param ack_info is the HARQ-ACK information
 */
ISRRAN_API void isrran_enb_dl_get_ack(const isrran_cell_t*      cell,
                                      const isrran_uci_cfg_t*   uci_cfg,
                                      const isrran_uci_value_t* uci_value,
                                      isrran_pdsch_ack_t*       pdsch_ack);

/**
 * Gets the maximum signal power in decibels full scale. It is equivalent to the transmit power if all resource elements
 * were populated.
 * @return The maximum power
 */
ISRRAN_API float isrran_enb_dl_get_maximum_signal_power_dBfs(uint32_t nof_prb);

#endif // ISRRAN_ENB_DL_H
