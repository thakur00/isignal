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

/**********************************************************************************************
 *  File:         phy_common.h
 *
 *  Description:  Common parameters and lookup functions for PHY
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10
 *********************************************************************************************/

#ifndef ISRRAN_PHY_COMMON_H
#define ISRRAN_PHY_COMMON_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "isrran/config.h"

#define ISRRAN_NOF_SF_X_FRAME 10
#define ISRRAN_NOF_SLOTS_PER_SF 2
#define ISRRAN_NSLOTS_X_FRAME (ISRRAN_NOF_SLOTS_PER_SF * ISRRAN_NOF_SF_X_FRAME)

#define ISRRAN_NSOFT_BITS 250368 // Soft buffer size for Category 1 UE

#define ISRRAN_PC_MAX 23 // Maximum TX power for Category 1 UE (in dBm)

#define ISRRAN_NOF_NID_1 (168)
#define ISRRAN_NOF_NID_2 (3)
#define ISRRAN_NUM_PCI (ISRRAN_NOF_NID_1 * ISRRAN_NOF_NID_2)

#define ISRRAN_MAX_CARRIERS 5 // Maximum number of supported simultaneous carriers
#define ISRRAN_MAX_PORTS 4
#define ISRRAN_MAX_CHANNELS (ISRRAN_MAX_CARRIERS * ISRRAN_MAX_PORTS)
#define ISRRAN_MAX_LAYERS 4
#define ISRRAN_MAX_CODEWORDS 2
#define ISRRAN_MAX_TB ISRRAN_MAX_CODEWORDS
#define ISRRAN_MAX_QM 8

#define ISRRAN_MAX_CODEBLOCKS 32

#define ISRRAN_MAX_CODEBOOKS 4

#define ISRRAN_NOF_CFI 3
#define ISRRAN_CFI_ISVALID(x) ((x >= 1 && x <= 3))
#define ISRRAN_CFI_IDX(x) ((x - 1) % ISRRAN_NOF_CFI)

#define ISRRAN_LTE_CRC24A 0x1864CFB
#define ISRRAN_LTE_CRC24B 0X1800063
#define ISRRAN_LTE_CRC24C 0X1B2B117
#define ISRRAN_LTE_CRC16 0x11021
#define ISRRAN_LTE_CRC11 0xE21
#define ISRRAN_LTE_CRC8 0x19B
#define ISRRAN_LTE_CRC6 0x61

#define ISRRAN_MAX_MBSFN_AREA_IDS 256
#define ISRRAN_PMCH_RV 0

typedef enum { ISRRAN_CP_NORM = 0, ISRRAN_CP_EXT } isrran_cp_t;
typedef enum { ISRRAN_SF_NORM = 0, ISRRAN_SF_MBSFN } isrran_sf_t;

#define ISRRAN_INVALID_RNTI 0x0 // TS 36.321 - Table 7.1-1 RNTI 0x0 isn't a valid DL RNTI
#define ISRRAN_CRNTI_START 0x000B
#define ISRRAN_CRNTI_END 0xFFF3
#define ISRRAN_RARNTI_START 0x0001
#define ISRRAN_RARNTI_END 0x000A
#define ISRRAN_SIRNTI 0xFFFF
#define ISRRAN_PRNTI 0xFFFE
#define ISRRAN_MRNTI 0xFFFD

#define ISRRAN_RNTI_ISRAR(rnti) (rnti >= ISRRAN_RARNTI_START && rnti <= ISRRAN_RARNTI_END)
#define ISRRAN_RNTI_ISUSER(rnti) (rnti >= ISRRAN_CRNTI_START && rnti <= ISRRAN_CRNTI_END)
#define ISRRAN_RNTI_ISSI(rnti) (rnti == ISRRAN_SIRNTI)
#define ISRRAN_RNTI_ISPA(rnti) (rnti == ISRRAN_PRNTI)
#define ISRRAN_RNTI_ISMBSFN(rnti) (rnti == ISRRAN_MRNTI)
#define ISRRAN_RNTI_ISSIRAPA(rnti) (ISRRAN_RNTI_ISSI(rnti) || ISRRAN_RNTI_ISRAR(rnti) || ISRRAN_RNTI_ISPA(rnti))

#define ISRRAN_CELL_ID_UNKNOWN 1000

#define ISRRAN_MAX_NSYMB 7

#define ISRRAN_MAX_PRB 110
#define ISRRAN_NRE 12

#define ISRRAN_SYMBOL_SZ_MAX 2048

#define ISRRAN_CP_NORM_NSYMB 7
#define ISRRAN_CP_NORM_SF_NSYMB (2 * ISRRAN_CP_NORM_NSYMB)
#define ISRRAN_CP_NORM_0_LEN 160
#define ISRRAN_CP_NORM_LEN 144

#define ISRRAN_CP_EXT_NSYMB 6
#define ISRRAN_CP_EXT_SF_NSYMB (2 * ISRRAN_CP_EXT_NSYMB)
#define ISRRAN_CP_EXT_LEN 512
#define ISRRAN_CP_EXT_7_5_LEN 1024

#define ISRRAN_CP_ISNORM(cp) (cp == ISRRAN_CP_NORM)
#define ISRRAN_CP_ISEXT(cp) (cp == ISRRAN_CP_EXT)
#define ISRRAN_CP_NSYMB(cp) (ISRRAN_CP_ISNORM(cp) ? ISRRAN_CP_NORM_NSYMB : ISRRAN_CP_EXT_NSYMB)

#define ISRRAN_CP_LEN(symbol_sz, c) ((int)ceilf((((float)(c) * (symbol_sz)) / 2048.0f)))
#define ISRRAN_CP_LEN_NORM(symbol, symbol_sz)                                                                          \
  (((symbol) == 0) ? ISRRAN_CP_LEN((symbol_sz), ISRRAN_CP_NORM_0_LEN) : ISRRAN_CP_LEN((symbol_sz), ISRRAN_CP_NORM_LEN))
#define ISRRAN_CP_LEN_EXT(symbol_sz) (ISRRAN_CP_LEN((symbol_sz), ISRRAN_CP_EXT_LEN))

#define ISRRAN_CP_SZ(symbol_sz, cp)                                                                                    \
  (ISRRAN_CP_LEN(symbol_sz, (ISRRAN_CP_ISNORM(cp) ? ISRRAN_CP_NORM_LEN : ISRRAN_CP_EXT_LEN)))
#define ISRRAN_SYMBOL_SZ(symbol_sz, cp) (symbol_sz + ISRRAN_CP_SZ(symbol_sz, cp))
#define ISRRAN_SLOT_LEN(symbol_sz) (symbol_sz * 15 / 2)
#define ISRRAN_SF_LEN(symbol_sz) (symbol_sz * 15)
#define ISRRAN_SF_LEN_MAX (ISRRAN_SF_LEN(ISRRAN_SYMBOL_SZ_MAX))

#define ISRRAN_SLOT_LEN_PRB(nof_prb) (ISRRAN_SLOT_LEN(isrran_symbol_sz(nof_prb)))
#define ISRRAN_SF_LEN_PRB(nof_prb) ((uint32_t)ISRRAN_SF_LEN(isrran_symbol_sz(nof_prb)))

#define ISRRAN_SLOT_LEN_RE(nof_prb, cp) (nof_prb * ISRRAN_NRE * ISRRAN_CP_NSYMB(cp))
#define ISRRAN_SF_LEN_RE(nof_prb, cp) (2 * ISRRAN_SLOT_LEN_RE(nof_prb, cp))
#define ISRRAN_NOF_RE(cell) (2 * ISRRAN_SLOT_LEN_RE(cell.nof_prb, cell.cp))

#define ISRRAN_TA_OFFSET (10e-6)

#define ISRRAN_LTE_TS (1.0f / (15000.0f * 2048.0f))

#define ISRRAN_SLOT_IDX_CPNORM(symbol_idx, symbol_sz)                                                                  \
  (symbol_idx == 0 ? 0                                                                                                 \
                   : (symbol_sz + ISRRAN_CP_LEN(symbol_sz, ISRRAN_CP_NORM_0_LEN) +                                     \
                      (symbol_idx - 1) * (symbol_sz + ISRRAN_CP_LEN(symbol_sz, ISRRAN_CP_NORM_LEN))))
#define ISRRAN_SLOT_IDX_CPEXT(idx, symbol_sz) (idx * (symbol_sz + ISRRAN_CP(symbol_sz, ISRRAN_CP_EXT_LEN)))

#define ISRRAN_RE_IDX(nof_prb, symbol_idx, sample_idx) ((symbol_idx) * (nof_prb) * (ISRRAN_NRE) + sample_idx)

#define ISRRAN_RS_VSHIFT(cell_id) (cell_id % 6)

#define ISRRAN_GUARD_RE(nof_prb) ((isrran_symbol_sz(nof_prb) - nof_prb * ISRRAN_NRE) / 2)

#define ISRRAN_SYMBOL_HAS_REF(l, cp, nof_ports) ((l == 1 && nof_ports == 4) || l == 0 || l == ISRRAN_CP_NSYMB(cp) - 3)

#define ISRRAN_NOF_CTRL_SYMBOLS(cell, cfi) (cfi + (cell.nof_prb < 10 ? 1 : 0))

#define ISRRAN_SYMBOL_HAS_REF_MBSFN(l, s) ((l == 2 && s == 0) || (l == 0 && s == 1) || (l == 4 && s == 1))

#define ISRRAN_NON_MBSFN_REGION_GUARD_LENGTH(non_mbsfn_region, symbol_sz)                                              \
  ((non_mbsfn_region == 1)                                                                                             \
       ? (ISRRAN_CP_LEN_EXT(symbol_sz) - ISRRAN_CP_LEN_NORM(0, symbol_sz))                                             \
       : (2 * ISRRAN_CP_LEN_EXT(symbol_sz) - ISRRAN_CP_LEN_NORM(0, symbol_sz) - ISRRAN_CP_LEN_NORM(1, symbol_sz)))

#define ISRRAN_FDD_NOF_HARQ (FDD_HARQ_DELAY_DL_MS + FDD_HARQ_DELAY_UL_MS)
#define ISRRAN_MAX_HARQ_PROC 15

#define ISRRAN_NOF_LTE_BANDS 58

#define ISRRAN_DEFAULT_MAX_FRAMES_PBCH 500
#define ISRRAN_DEFAULT_MAX_FRAMES_PSS 10
#define ISRRAN_DEFAULT_NOF_VALID_PSS_FRAMES 10

#define ZERO_OBJECT(x) memset(&(x), 0x0, sizeof((x)))

typedef enum ISRRAN_API { ISRRAN_PHICH_NORM = 0, ISRRAN_PHICH_EXT } isrran_phich_length_t;

typedef enum ISRRAN_API {
  ISRRAN_PHICH_R_1_6 = 0,
  ISRRAN_PHICH_R_1_2,
  ISRRAN_PHICH_R_1,
  ISRRAN_PHICH_R_2
} isrran_phich_r_t;

/// LTE duplex modes.
typedef enum ISRRAN_API {
  /// FDD uses frame structure type 1.
  ISRRAN_FDD = 0,
  /// TDD uses frame structure type 2.
  ISRRAN_TDD = 1
} isrran_frame_type_t;

/// Maximum number of TDD special subframe configurations.
#define ISRRAN_MAX_TDD_SS_CONFIGS (10u)

/// Maximum number of TDD uplink-downlink subframe configurations.
#define ISRRAN_MAX_TDD_SF_CONFIGS (7u)

/// Configuration fields for operating in TDD mode.
typedef struct ISRRAN_API {
  /// Uplink-downlink configuration, valid range is [0,ISRRAN_MAX_TDD_SF_CONFIGS[.
  /// TS 36.211 v8.9.0 Table 4.2-2.
  uint32_t sf_config;
  /// Special subframe symbol length configuration, valid range is [0,ISRRAN_MAX_TDD_SS_CONFIGS[.
  /// TS 36.211 v13.13.0 Table 4.2-1.
  uint32_t ss_config;
  /// Set to true when the fields have been configured, otherwise false.
  bool configured;
} isrran_tdd_config_t;

/// TDD uplink-downlink subframe types.
typedef enum ISRRAN_API {
  /// Subframe is reserved for downlink transmissions.
  ISRRAN_TDD_SF_D = 0,
  /// Subframe is reserved for uplink transmissions.
  ISRRAN_TDD_SF_U = 1,
  /// Special subframe.
  ISRRAN_TDD_SF_S = 2,
} isrran_tdd_sf_t;

typedef struct {
  uint8_t mbsfn_area_id;
  uint8_t non_mbsfn_region_length;
  uint8_t mbsfn_mcs;
  bool    enable;
  bool    is_mcch;
} isrran_mbsfn_cfg_t;

// Common cell constant properties that require object reconfiguration
typedef struct ISRRAN_API {
  uint32_t              nof_prb;
  uint32_t              nof_ports;
  uint32_t              id;
  isrran_cp_t           cp;
  isrran_phich_length_t phich_length;
  isrran_phich_r_t      phich_resources;
  isrran_frame_type_t   frame_type;
} isrran_cell_t;

// Common downlink properties that may change every subframe
typedef struct ISRRAN_API {
  isrran_tdd_config_t tdd_config;
  uint32_t            tti;
  uint32_t            cfi;
  isrran_sf_t         sf_type;
  uint32_t            non_mbsfn_region;
} isrran_dl_sf_cfg_t;

typedef struct ISRRAN_API {
  isrran_tdd_config_t tdd_config;
  uint32_t            tti;
  bool                shortened;
} isrran_ul_sf_cfg_t;

typedef enum ISRRAN_API {
  ISRRAN_TM1 = 0,
  ISRRAN_TM2,
  ISRRAN_TM3,
  ISRRAN_TM4,
  ISRRAN_TM5,
  ISRRAN_TM6,
  ISRRAN_TM7,
  ISRRAN_TM8,
  ISRRAN_TMINV // Invalid Transmission Mode
} isrran_tm_t;

typedef enum ISRRAN_API {
  ISRRAN_TXSCHEME_PORT0,
  ISRRAN_TXSCHEME_DIVERSITY,
  ISRRAN_TXSCHEME_SPATIALMUX,
  ISRRAN_TXSCHEME_CDD
} isrran_tx_scheme_t;

typedef enum ISRRAN_API { ISRRAN_MIMO_DECODER_ZF, ISRRAN_MIMO_DECODER_MMSE } isrran_mimo_decoder_t;

/*!
 * \brief Types of modulations and associated modulation order.
 */
typedef enum ISRRAN_API {
  ISRRAN_MOD_BPSK = 0, /*!< \brief BPSK. */
  ISRRAN_MOD_QPSK,     /*!< \brief QPSK. */
  ISRRAN_MOD_16QAM,    /*!< \brief QAM16. */
  ISRRAN_MOD_64QAM,    /*!< \brief QAM64. */
  ISRRAN_MOD_256QAM,   /*!< \brief QAM256. */
  ISRRAN_MOD_NITEMS
} isrran_mod_t;

typedef enum {
  ISRRAN_DCI_FORMAT0 = 0,
  ISRRAN_DCI_FORMAT1,
  ISRRAN_DCI_FORMAT1A,
  ISRRAN_DCI_FORMAT1B,
  ISRRAN_DCI_FORMAT1C,
  ISRRAN_DCI_FORMAT1D,
  ISRRAN_DCI_FORMAT2,
  ISRRAN_DCI_FORMAT2A,
  ISRRAN_DCI_FORMAT2B,
  // ISRRAN_DCI_FORMAT3,
  // ISRRAN_DCI_FORMAT3A,
  ISRRAN_DCI_FORMATN0,
  ISRRAN_DCI_FORMATN1,
  ISRRAN_DCI_FORMATN2,
  ISRRAN_DCI_FORMAT_RAR, // Not a real LTE format. Used internally to indicate RAR grant
  ISRRAN_DCI_NOF_FORMATS
} isrran_dci_format_t;

typedef enum {
  ISRRAN_PUCCH_ACK_NACK_FEEDBACK_MODE_NORMAL = 0, /* No cell selection no pucch3 */
  ISRRAN_PUCCH_ACK_NACK_FEEDBACK_MODE_CS,
  ISRRAN_PUCCH_ACK_NACK_FEEDBACK_MODE_PUCCH3,
  ISRRAN_PUCCH_ACK_NACK_FEEDBACK_MODE_ERROR,
} isrran_ack_nack_feedback_mode_t;

typedef struct ISRRAN_API {
  int   id;
  float fd;
} isrran_earfcn_t;

enum band_geographical_area {
  ISRRAN_BAND_GEO_AREA_ALL,
  ISRRAN_BAND_GEO_AREA_NAR,
  ISRRAN_BAND_GEO_AREA_APAC,
  ISRRAN_BAND_GEO_AREA_EMEA,
  ISRRAN_BAND_GEO_AREA_JAPAN,
  ISRRAN_BAND_GEO_AREA_CALA,
  ISRRAN_BAND_GEO_AREA_NA
};

///< NB-IoT specific structs
typedef enum {
  ISRRAN_NBIOT_MODE_INBAND_SAME_PCI = 0,
  ISRRAN_NBIOT_MODE_INBAND_DIFFERENT_PCI,
  ISRRAN_NBIOT_MODE_GUARDBAND,
  ISRRAN_NBIOT_MODE_STANDALONE,
  ISRRAN_NBIOT_MODE_N_ITEMS,
} isrran_nbiot_mode_t;

typedef struct ISRRAN_API {
  isrran_cell_t       base;      // the umbrella or super cell
  uint32_t            nbiot_prb; // the index of the NB-IoT PRB within the cell
  uint32_t            n_id_ncell;
  uint32_t            nof_ports; // The number of antenna ports for NB-IoT
  bool                is_r14;    // Whether the cell is a R14 cell
  isrran_nbiot_mode_t mode;
} isrran_nbiot_cell_t;

#define ISRRAN_NBIOT_MAX_PORTS 2
#define ISRRAN_NBIOT_MAX_CODEWORDS ISRRAN_MAX_CODEWORDS

#define ISRRAN_SF_LEN_PRB_NBIOT (ISRRAN_SF_LEN_PRB(1))

#define ISRRAN_SF_LEN_RE_NBIOT (ISRRAN_SF_LEN_RE(1, ISRRAN_CP_NORM))

#define ISRRAN_NBIOT_FFT_SIZE 128
#define ISRRAN_NBIOT_FREQ_SHIFT_FACTOR ((float)-0.5)
#define ISRRAN_NBIOT_NUM_RX_ANTENNAS 1
#define ISRRAN_NBIOT_MAX_PRB 1

#define ISRRAN_NBIOT_DEFAULT_NUM_PRB_BASECELL 1
#define ISRRAN_NBIOT_DEFAULT_PRB_OFFSET 0

#define ISRRAN_DEFAULT_MAX_FRAMES_NPBCH 500
#define ISRRAN_DEFAULT_MAX_FRAMES_NPSS 20
#define ISRRAN_DEFAULT_NOF_VALID_NPSS_FRAMES 20

#define ISRRAN_NBIOT_NPBCH_NOF_TOTAL_BITS (1600) ///< Number of bits for the entire NPBCH (See 36.211 Sec 10.2.4.1)
#define ISRRAN_NBIOT_NPBCH_NOF_BITS_SF                                                                                 \
  (ISRRAN_NBIOT_NPBCH_NOF_TOTAL_BITS / 8) ///< The NPBCH is transmitted in 8 blocks (See 36.211 Sec 10.2.4.4)

#define ISRRAN_MAX_DL_BITS_CAT_NB1 (680) ///< TS 36.306 v15.4.0 Table 4.1C-1

///< PHY common function declarations

ISRRAN_API bool isrran_cell_isvalid(isrran_cell_t* cell);

ISRRAN_API void isrran_cell_fprint(FILE* stream, isrran_cell_t* cell, uint32_t sfn);

ISRRAN_API bool isrran_cellid_isvalid(uint32_t cell_id);

ISRRAN_API bool isrran_nofprb_isvalid(uint32_t nof_prb);

/**
 * Returns the subframe type for a given subframe number and a TDD configuration.
 * Check TS 36.211 v8.9.0 Table 4.2-2.
 *
 * @param tdd_config TDD configuration.
 * @param sf_idx Subframe number, must be in range [0,ISRRAN_NOF_SF_X_FRAME[.
 * @return Returns the subframe type.
 */
ISRRAN_API isrran_tdd_sf_t isrran_sfidx_tdd_type(isrran_tdd_config_t tdd_config, uint32_t sf_idx);

/**
 * Returns the number of UpPTS symbols in a subframe.
 * Check TS 36.211 v13.13.0 Table 4.2-1.
 *
 * @param tdd_config TDD configuration.
 * @return Returns the number of UpPTS symbols.
 */
ISRRAN_API uint32_t isrran_sfidx_tdd_nof_up(isrran_tdd_config_t tdd_config);

/**
 * Returns the number of GP symbols in a subframe.
 * Check TS 36.211 v13.13.0 Table 4.2-1.
 *
 * @param tdd_config TDD configuration.
 * @return Returns the number of GP symbols.
 */
ISRRAN_API uint32_t isrran_sfidx_tdd_nof_gp(isrran_tdd_config_t tdd_config);

/**
 * Returns the number of DwPTS symbols in a subframe.
 * Check TS 36.211 v13.13.0 Table 4.2-1.
 *
 * @param tdd_config TDD configuration.
 * @return Returns the number of DwPTS symbols.
 */
ISRRAN_API uint32_t isrran_sfidx_tdd_nof_dw(isrran_tdd_config_t tdd_config);

ISRRAN_API uint32_t isrran_tdd_nof_harq(isrran_tdd_config_t tdd_config);

ISRRAN_API uint32_t isrran_sfidx_tdd_nof_dw_slot(isrran_tdd_config_t tdd_config, uint32_t slot, isrran_cp_t cp);

ISRRAN_API bool isrran_sfidx_isvalid(uint32_t sf_idx);

ISRRAN_API bool isrran_portid_isvalid(uint32_t port_id);

ISRRAN_API bool isrran_N_id_2_isvalid(uint32_t N_id_2);

ISRRAN_API bool isrran_N_id_1_isvalid(uint32_t N_id_1);

ISRRAN_API bool isrran_symbol_sz_isvalid(uint32_t symbol_sz);

ISRRAN_API int isrran_symbol_sz(uint32_t nof_prb);

ISRRAN_API int isrran_symbol_sz_power2(uint32_t nof_prb);

ISRRAN_API int isrran_nof_prb(uint32_t symbol_sz);

ISRRAN_API uint32_t isrran_max_cce(uint32_t nof_prb);

ISRRAN_API int isrran_sampling_freq_hz(uint32_t nof_prb);

ISRRAN_API void isrran_use_standard_symbol_size(bool enabled);

ISRRAN_API bool isrran_symbol_size_is_standard();

ISRRAN_API uint32_t isrran_re_x_prb(uint32_t ns, uint32_t symbol, uint32_t nof_ports, uint32_t nof_symbols);

ISRRAN_API uint32_t isrran_voffset(uint32_t symbol_id, uint32_t cell_id, uint32_t nof_ports);

ISRRAN_API int isrran_group_hopping_f_gh(uint32_t f_gh[ISRRAN_NSLOTS_X_FRAME], uint32_t cell_id);

ISRRAN_API uint32_t isrran_N_ta_new_rar(uint32_t ta);

ISRRAN_API uint32_t isrran_N_ta_new(uint32_t N_ta_old, uint32_t ta);

ISRRAN_API float isrran_coderate(uint32_t tbs, uint32_t nof_re);

ISRRAN_API char* isrran_cp_string(isrran_cp_t cp);

ISRRAN_API isrran_mod_t isrran_str2mod(const char* mod_str);

ISRRAN_API char* isrran_mod_string(isrran_mod_t mod);

ISRRAN_API uint32_t isrran_mod_bits_x_symbol(isrran_mod_t mod);

ISRRAN_API uint8_t isrran_band_get_band(uint32_t dl_earfcn);

ISRRAN_API bool isrran_band_is_tdd(uint32_t band);

ISRRAN_API double isrran_band_fd(uint32_t dl_earfcn);

ISRRAN_API double isrran_band_fu(uint32_t ul_earfcn);

ISRRAN_API uint32_t isrran_band_ul_earfcn(uint32_t dl_earfcn);

ISRRAN_API int
isrran_band_get_fd_band(uint32_t band, isrran_earfcn_t* earfcn, int earfcn_start, int earfcn_end, uint32_t max_elems);

ISRRAN_API int isrran_band_get_fd_band_all(uint32_t band, isrran_earfcn_t* earfcn, uint32_t max_nelems);

ISRRAN_API int
isrran_band_get_fd_region(enum band_geographical_area region, isrran_earfcn_t* earfcn, uint32_t max_elems);

ISRRAN_API int isrran_str2mimotype(char* mimo_type_str, isrran_tx_scheme_t* type);

ISRRAN_API char* isrran_mimotype2str(isrran_tx_scheme_t mimo_type);

/* Returns the interval tti1-tti2 mod 10240 */
ISRRAN_API uint32_t isrran_tti_interval(uint32_t tti1, uint32_t tti2);

ISRRAN_API uint32_t isrran_print_check(char* s, size_t max_len, uint32_t cur_len, const char* format, ...);

ISRRAN_API bool  isrran_nbiot_cell_isvalid(isrran_nbiot_cell_t* cell);
ISRRAN_API bool  isrran_nbiot_portid_isvalid(uint32_t port_id);
ISRRAN_API float isrran_band_fu_nbiot(uint32_t ul_earfcn, const float m_ul);

ISRRAN_API char* isrran_nbiot_mode_string(isrran_nbiot_mode_t mode);

/**
 * Returns a constant string pointer with the ACK/NACK feedback mode
 *
 * @param ack_nack_feedback_mode Mode
 * @return Returns constant pointer with the text of the mode if successful, `error` otherwise
 */
ISRRAN_API const char* isrran_ack_nack_feedback_mode_string(isrran_ack_nack_feedback_mode_t ack_nack_feedback_mode);

/**
 * Returns a constant string pointer with the ACK/NACK feedback mode
 *
 * @param ack_nack_feedback_mode Mode
 * @return Returns constant pointer with the text of the mode if successful, `error` otherwise
 */
ISRRAN_API isrran_ack_nack_feedback_mode_t isrran_string_ack_nack_feedback_mode(const char* str);

/**
 * Returns the number of bits for Rank Indicador reporting depending on the cell
 *
 * @param cell
 */
ISRRAN_API uint32_t isrran_ri_nof_bits(const isrran_cell_t* cell);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_PHY_COMMON_H
