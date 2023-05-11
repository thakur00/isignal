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
 *  File:         dci.h
 *
 *  Description:  Downlink control information (DCI).
 *                Packing/Unpacking functions to convert between bit streams
 *                and packed DCI UL/DL grants defined in ra.h
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10 Sec. 5.3.3
 *****************************************************************************/

#ifndef ISRRAN_DCI_H
#define ISRRAN_DCI_H

#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/phch/ra.h"

#define ISRRAN_DCI_MAX_BITS 128
#define ISRRAN_RAR_GRANT_LEN 20

#define ISRRAN_DCI_IS_TB_EN(tb) (!(tb.mcs_idx == 0 && tb.rv == 1))
#define ISRRAN_DCI_TB_DISABLE(tb)                                                                                      \
  do {                                                                                                                 \
    tb.mcs_idx = 0;                                                                                                    \
    tb.rv      = 1;                                                                                                    \
  } while (0)
#define ISRRAN_DCI_HEXDEBUG 0

typedef struct {
  bool multiple_csi_request_enabled;
  bool cif_enabled;
  bool cif_present;
  bool isr_request_enabled;
  bool ra_format_enabled;
  bool is_not_ue_ss;
} isrran_dci_cfg_t;

typedef struct ISRRAN_API {
  uint32_t L;    // Aggregation level (logarithmic)
  uint32_t ncce; // Position of first CCE of the dci
} isrran_dci_location_t;

typedef struct ISRRAN_API {
  uint8_t               payload[ISRRAN_DCI_MAX_BITS];
  uint32_t              nof_bits;
  isrran_dci_location_t location;
  isrran_dci_format_t   format;
  uint16_t              rnti;
} isrran_dci_msg_t;

typedef struct ISRRAN_API {
  uint32_t mcs_idx;
  int      rv;
  bool     ndi;
  uint32_t cw_idx;
} isrran_dci_tb_t;

typedef struct ISRRAN_API {
  uint16_t              rnti;
  isrran_dci_format_t   format;
  isrran_dci_location_t location;
  uint32_t              ue_cc_idx;

  // Resource Allocation
  isrran_ra_type_t alloc_type;
  union {
    isrran_ra_type0_t type0_alloc;
    isrran_ra_type1_t type1_alloc;
    isrran_ra_type2_t type2_alloc;
  };

  // Codeword information
  isrran_dci_tb_t tb[ISRRAN_MAX_CODEWORDS];
  bool            tb_cw_swap;
  uint32_t        pinfo;

  // Power control
  bool    pconf;
  bool    power_offset;
  uint8_t tpc_pucch;

  // PDCCH order
  bool     is_pdcch_order;
  uint32_t preamble_idx;
  uint32_t prach_mask_idx;

  // Release 10
  uint32_t cif;
  bool     cif_present;
  bool     isr_request;
  bool     isr_request_present;

  // Other parameters
  uint32_t pid;
  uint32_t dai;
  bool     is_tdd;
  bool     is_dwpts;
  bool     sram_id;

  // For debugging purposes
#if ISRRAN_DCI_HEXDEBUG
  uint32_t nof_bits;
  char     hex_str[ISRRAN_DCI_MAX_BITS];
#endif
} isrran_dci_dl_t;

/** Unpacked DCI Format0 message */
typedef struct ISRRAN_API {
  uint16_t              rnti;
  isrran_dci_format_t   format;
  isrran_dci_location_t location;
  uint32_t              ue_cc_idx;

  isrran_ra_type2_t type2_alloc;
  /* 36.213 Table 8.4-2: ISRRAN_RA_PUSCH_HOP_HALF is 0 for < 10 Mhz and 10 for > 10 Mhz.
   * ISRRAN_RA_PUSCH_HOP_QUART is 00 for > 10 Mhz and ISRRAN_RA_PUSCH_HOP_QUART_NEG is 01 for > 10 Mhz.
   */
  enum {
    ISRRAN_RA_PUSCH_HOP_DISABLED  = -1,
    ISRRAN_RA_PUSCH_HOP_QUART     = 0,
    ISRRAN_RA_PUSCH_HOP_QUART_NEG = 1,
    ISRRAN_RA_PUSCH_HOP_HALF      = 2,
    ISRRAN_RA_PUSCH_HOP_TYPE2     = 3
  } freq_hop_fl;

  // Codeword information
  isrran_dci_tb_t tb;
  uint32_t        n_dmrs;
  bool            cqi_request;

  // TDD parametres
  uint32_t dai;
  uint32_t ul_idx;
  bool     is_tdd;

  // Power control
  uint8_t tpc_pusch;

  // Release 10
  uint32_t         cif;
  bool             cif_present;
  uint8_t          multiple_csi_request;
  bool             multiple_csi_request_present;
  bool             isr_request;
  bool             isr_request_present;
  isrran_ra_type_t ra_type;
  bool             ra_type_present;

  // For debugging purposes
#if ISRRAN_DCI_HEXDEBUG
  uint32_t nof_bits;
  char     hex_str[ISRRAN_DCI_MAX_BITS];
#endif /* ISRRAN_DCI_HEXDEBUG */

} isrran_dci_ul_t;

typedef struct ISRRAN_API {
  uint32_t rba;
  uint32_t trunc_mcs;
  uint32_t tpc_pusch;
  bool     ul_delay;
  bool     cqi_request;
  bool     hopping_flag;
} isrran_dci_rar_grant_t;

ISRRAN_API void isrran_dci_rar_unpack(uint8_t payload[ISRRAN_RAR_GRANT_LEN], isrran_dci_rar_grant_t* rar);

ISRRAN_API void isrran_dci_rar_pack(isrran_dci_rar_grant_t* rar, uint8_t payload[ISRRAN_RAR_GRANT_LEN]);

ISRRAN_API int isrran_dci_rar_to_ul_dci(isrran_cell_t* cell, isrran_dci_rar_grant_t* rar, isrran_dci_ul_t* dci_ul);

ISRRAN_API int isrran_dci_msg_pack_pusch(isrran_cell_t*      cell,
                                         isrran_dl_sf_cfg_t* sf,
                                         isrran_dci_cfg_t*   cfg,
                                         isrran_dci_ul_t*    dci,
                                         isrran_dci_msg_t*   msg);

ISRRAN_API int isrran_dci_msg_unpack_pusch(isrran_cell_t*      cell,
                                           isrran_dl_sf_cfg_t* sf,
                                           isrran_dci_cfg_t*   cfg,
                                           isrran_dci_msg_t*   msg,
                                           isrran_dci_ul_t*    dci);

ISRRAN_API int isrran_dci_msg_pack_pdsch(isrran_cell_t*      cell,
                                         isrran_dl_sf_cfg_t* sf,
                                         isrran_dci_cfg_t*   cfg,
                                         isrran_dci_dl_t*    dci,
                                         isrran_dci_msg_t*   msg);

ISRRAN_API int isrran_dci_msg_unpack_pdsch(isrran_cell_t*      cell,
                                           isrran_dl_sf_cfg_t* sf,
                                           isrran_dci_cfg_t*   cfg,
                                           isrran_dci_msg_t*   msg,
                                           isrran_dci_dl_t*    dci);

ISRRAN_API uint32_t isrran_dci_format_sizeof(const isrran_cell_t* cell,
                                             isrran_dl_sf_cfg_t*  sf,
                                             isrran_dci_cfg_t*    cfg,
                                             isrran_dci_format_t  format);

ISRRAN_API void isrran_dci_dl_fprint(FILE* f, isrran_dci_dl_t* dci, uint32_t nof_prb);

ISRRAN_API uint32_t isrran_dci_dl_info(const isrran_dci_dl_t* dci_dl, char* str, uint32_t str_len);

ISRRAN_API uint32_t isrran_dci_ul_info(isrran_dci_ul_t* dci_ul, char* info_str, uint32_t len);

ISRRAN_API isrran_dci_format_t isrran_dci_format_from_string(char* str);

ISRRAN_API char* isrran_dci_format_string(isrran_dci_format_t format);

ISRRAN_API char* isrran_dci_format_string_short(isrran_dci_format_t format);

ISRRAN_API bool
isrran_location_find(const isrran_dci_location_t* locations, uint32_t nof_locations, isrran_dci_location_t x);

ISRRAN_API bool isrran_location_find_location(const isrran_dci_location_t* locations,
                                              uint32_t                     nof_locations,
                                              const isrran_dci_location_t* location);

ISRRAN_API int isrran_dci_location_set(isrran_dci_location_t* c, uint32_t L, uint32_t nCCE);

ISRRAN_API bool isrran_dci_location_isvalid(isrran_dci_location_t* c);

ISRRAN_API void isrran_dci_cfg_set_common_ss(isrran_dci_cfg_t* cfg);

ISRRAN_API uint32_t isrran_dci_format_max_tb(isrran_dci_format_t format);

#endif // DCI_
