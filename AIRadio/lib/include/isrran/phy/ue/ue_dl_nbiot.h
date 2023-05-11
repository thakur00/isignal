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

#ifndef ISRRAN_UE_DL_NBIOT_H
#define ISRRAN_UE_DL_NBIOT_H

#include <stdbool.h>

#include "isrran/phy/ch_estimation/chest_dl_nbiot.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/ofdm.h"

#include "isrran/phy/phch/dci_nbiot.h"
#include "isrran/phy/phch/npdcch.h"
#include "isrran/phy/phch/npdsch.h"
#include "isrran/phy/phch/npdsch_cfg.h"
#include "isrran/phy/phch/ra_nbiot.h"

#include "isrran/phy/sync/cfo.h"

#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

#include "isrran/config.h"

#define ISRRAN_NBIOT_EXPECT_MORE_SF -3  // returned when expecting more subframes for successful decoding
#define ISRRAN_NBIOT_UE_DL_FOUND_DCI -4 // returned when DCI for given RNTI was found
#define ISRRAN_NBIOT_UE_DL_SKIP_SF -5

/*
 * @brief Narrowband UE downlink object.
 *
 * This module is a frontend to all the downlink data and control
 * channel processing modules.
 */
typedef struct ISRRAN_API {
  isrran_npdcch_t         npdcch;
  isrran_npdsch_t         npdsch;
  isrran_ofdm_t           fft;
  isrran_chest_dl_nbiot_t chest;

  isrran_cfo_t sfo_correct;

  isrran_softbuffer_rx_t softbuffer;
  isrran_nbiot_cell_t    cell;
  isrran_mib_nb_t        mib;
  bool                   mib_set;

  int    nof_re;     // Number of RE per subframe
  cf_t*  sf_symbols; // this buffer holds the symbols of the current subframe
  cf_t*  sf_buffer;  // this buffer holds multiple subframes
  cf_t*  ce[ISRRAN_MAX_PORTS];
  cf_t*  ce_buffer[ISRRAN_MAX_PORTS];
  float* llr; // Buffer to hold soft-bits for later combining repetitions

  uint32_t pkt_errors;
  uint32_t pkts_total;
  uint32_t pkts_ok;
  uint32_t nof_detected;
  uint32_t bits_total;

  // DL configuration for "normal" transmissions
  bool                has_dl_grant;
  isrran_npdsch_cfg_t npdsch_cfg;

  // DL configuration for SIB1 transmissions
  uint32_t                 sib1_sfn[4 * SIB1_NB_MAX_REP]; // there are 4 SIB1 TTIs in each hyper frame
  isrran_nbiot_si_params_t si_params[ISRRAN_NBIOT_SI_TYPE_NITEMS];
  bool                     si_tti[10240]; // We trade memory consumption for speed

  uint16_t              current_rnti;
  uint32_t              last_n_cce;
  isrran_dci_location_t last_location;

  float sample_offset;
} isrran_nbiot_ue_dl_t;

// This function shall be called just after the initial synchronization
ISRRAN_API int isrran_nbiot_ue_dl_init(isrran_nbiot_ue_dl_t* q,
                                       cf_t*                 in_buffer[ISRRAN_MAX_PORTS],
                                       uint32_t              max_prb,
                                       uint32_t              nof_rx_antennas);

ISRRAN_API void isrran_nbiot_ue_dl_free(isrran_nbiot_ue_dl_t* q);

ISRRAN_API int isrran_nbiot_ue_dl_set_cell(isrran_nbiot_ue_dl_t* q, isrran_nbiot_cell_t cell);

ISRRAN_API int isrran_nbiot_ue_dl_decode_fft_estimate(isrran_nbiot_ue_dl_t* q, uint32_t sf_idx, bool is_dl_sf);

ISRRAN_API int isrran_nbiot_ue_dl_decode_estimate(isrran_nbiot_ue_dl_t* q, uint32_t sf_idx);

ISRRAN_API int
isrran_nbiot_ue_dl_cfg_grant(isrran_nbiot_ue_dl_t* q, isrran_ra_nbiot_dl_grant_t* grant, uint32_t sf_idx);

ISRRAN_API int
isrran_nbiot_ue_dl_find_dl_dci(isrran_nbiot_ue_dl_t* q, uint32_t sf_idx, uint16_t rnti, isrran_dci_msg_t* dci_msg);

int isrran_nbiot_ue_dl_find_dl_dci_type_siprarnti(isrran_nbiot_ue_dl_t* q, uint16_t rnti, isrran_dci_msg_t* dci_msg);

int isrran_nbiot_ue_dl_find_dl_dci_type_crnti(isrran_nbiot_ue_dl_t* q,
                                              uint32_t              sf_idx,
                                              uint16_t              rnti,
                                              isrran_dci_msg_t*     dci_msg);

ISRRAN_API int
isrran_nbiot_ue_dl_find_ul_dci(isrran_nbiot_ue_dl_t* q, uint32_t tti, uint32_t rnti, isrran_dci_msg_t* dci_msg);

ISRRAN_API uint32_t isrran_nbiot_ue_dl_get_ncce(isrran_nbiot_ue_dl_t* q);

ISRRAN_API void isrran_nbiot_ue_dl_set_sample_offset(isrran_nbiot_ue_dl_t* q, float sample_offset);

ISRRAN_API int
isrran_nbiot_ue_dl_decode(isrran_nbiot_ue_dl_t* q, cf_t* input, uint8_t* data, uint32_t sfn, uint32_t sf_idx);

ISRRAN_API int isrran_nbiot_ue_dl_decode_npdcch(isrran_nbiot_ue_dl_t* q,
                                                cf_t*                 input,
                                                uint32_t              sfn,
                                                uint32_t              sf_idx,
                                                uint16_t              rnti,
                                                isrran_dci_msg_t*     dci_msg);

ISRRAN_API int isrran_nbiot_ue_dl_decode_npdsch(isrran_nbiot_ue_dl_t* q,
                                                cf_t*                 input,
                                                uint8_t*              data,
                                                uint32_t              sfn,
                                                uint32_t              sf_idx,
                                                uint16_t              rnti);

int isrran_nbiot_ue_dl_decode_npdsch_bcch(isrran_nbiot_ue_dl_t* q, uint8_t* data, uint32_t tti);

int isrran_nbiot_ue_dl_decode_npdsch_no_bcch(isrran_nbiot_ue_dl_t* q, uint8_t* data, uint32_t tti, uint16_t rnti);

void isrran_nbiot_ue_dl_tb_decoded(isrran_nbiot_ue_dl_t* q, uint8_t* data);

ISRRAN_API void isrran_nbiot_ue_dl_reset(isrran_nbiot_ue_dl_t* q);

ISRRAN_API void isrran_nbiot_ue_dl_set_rnti(isrran_nbiot_ue_dl_t* q, uint16_t rnti);

ISRRAN_API void isrran_nbiot_ue_dl_set_mib(isrran_nbiot_ue_dl_t* q, isrran_mib_nb_t mib);

ISRRAN_API void isrran_nbiot_ue_dl_decode_sib1(isrran_nbiot_ue_dl_t* q, uint32_t current_sfn);

ISRRAN_API void
isrran_nbiot_ue_dl_get_sib1_grant(isrran_nbiot_ue_dl_t* q, uint32_t sfn, isrran_ra_nbiot_dl_grant_t* grant);

ISRRAN_API void isrran_nbiot_ue_dl_decode_sib(isrran_nbiot_ue_dl_t*    q,
                                              uint32_t                 hfn,
                                              uint32_t                 sfn,
                                              isrran_nbiot_si_type_t   type,
                                              isrran_nbiot_si_params_t params);

ISRRAN_API void isrran_nbiot_ue_dl_get_next_si_sfn(uint32_t                 current_hfn,
                                                   uint32_t                 current_sfn,
                                                   isrran_nbiot_si_params_t params,
                                                   uint32_t*                si_hfn,
                                                   uint32_t*                si_sfn);

ISRRAN_API void
isrran_nbiot_ue_dl_set_si_params(isrran_nbiot_ue_dl_t* q, isrran_nbiot_si_type_t type, isrran_nbiot_si_params_t params);

ISRRAN_API bool isrran_nbiot_ue_dl_is_sib1_sf(isrran_nbiot_ue_dl_t* q, uint32_t sfn, uint32_t sf_idx);

ISRRAN_API void isrran_nbiot_ue_dl_set_grant(isrran_nbiot_ue_dl_t* q, isrran_ra_nbiot_dl_grant_t* grant);

ISRRAN_API void isrran_nbiot_ue_dl_flush_grant(isrran_nbiot_ue_dl_t* q);

ISRRAN_API bool isrran_nbiot_ue_dl_has_grant(isrran_nbiot_ue_dl_t* q);

ISRRAN_API void isrran_nbiot_ue_dl_check_grant(isrran_nbiot_ue_dl_t* q, isrran_ra_nbiot_dl_grant_t* grant);

ISRRAN_API void isrran_nbiot_ue_dl_save_signal(isrran_nbiot_ue_dl_t* q, cf_t* input, uint32_t tti, uint32_t sf_idx);

#endif // ISRRAN_UE_DL_NBIOT_H
