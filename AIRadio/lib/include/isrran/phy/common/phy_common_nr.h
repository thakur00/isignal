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

#ifndef ISRRAN_PHY_COMMON_NR_H
#define ISRRAN_PHY_COMMON_NR_H

#include "phy_common.h"
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the number of symbols per slot. Defined by TS 38.211 v15.8.0 Table 4.3.2-1.
 */
#define ISRRAN_NSYMB_PER_SLOT_NR 14U

/**
 * @brief Defines the resource grid size in physical resource elements (frequency and time domain)
 */
#define ISRRAN_SLOT_LEN_RE_NR(nof_prb) (nof_prb * ISRRAN_NRE * ISRRAN_NSYMB_PER_SLOT_NR)

/**
 * @brief Minimum subframe length in samples for a given number of PRB
 */
#define ISRRAN_SF_LEN_PRB_NR(nof_prb) (isrran_min_symbol_sz_rb(nof_prb) * 15)

#define ISRRAN_SLOT_MAX_LEN_RE_NR (ISRRAN_SLOT_LEN_RE_NR(ISRRAN_MAX_PRB_NR))
#define ISRRAN_MAX_LAYERS_NR 8

/**
 * @brief Defines the maximum numerology supported. Defined by TS 38.211 v15.8.0 Table 4.3.2-1.
 */
#define ISRRAN_NR_MAX_NUMEROLOGY 4U

/**
 * @brief Defines the symbol duration, including cyclic prefix
 */
#define ISRRAN_SUBC_SPACING_NR(NUM) (15000U << (uint32_t)(NUM))

/**
 * @brief Defines the number of slots per SF. Defined by TS 38.211 v15.8.0 Table 4.3.2-1.
 */
#define ISRRAN_NSLOTS_PER_SF_NR(NUM) (1U << (NUM))

/**
 * @brief Defines the number of slots per frame. Defined by TS 38.211 v15.8.0 Table 4.3.2-1.
 */
#define ISRRAN_NSLOTS_PER_FRAME_NR(NUM) (ISRRAN_NSLOTS_PER_SF_NR(NUM) * ISRRAN_NOF_SF_X_FRAME)

/**
 * @brief Bounds slot index into the frame
 */
#define ISRRAN_SLOT_NR_MOD(NUM, N) ((N) % ISRRAN_NSLOTS_PER_FRAME_NR(NUM))

/**
 * @brief Maximum Carrier identification value. Defined by TS 38.331 v15.10.0 as PhysCellId from 0 to 1007.
 */
#define ISRRAN_MAX_ID_NR 1007

/**
 * @brief Maximum number of physical resource blocks (PRB) that a 5G NR can have. This is defined by TS 38.331 v15.10.0
 * as maxNrofPhysicalResourceBlocks
 */
#define ISRRAN_MAX_PRB_NR 275

/**
 * @brief Maximum start sub-carrier index for the carrier relative point
 */
#define ISRRAN_MAX_START_NR 2199

/**
 * @brief defines the maximum number of Aggregation levels: 1, 2, 4, 8 and 16
 */
#define ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR 5

/**
 * @brief defines the maximum number of RE
 */
#define ISRRAN_PDCCH_MAX_RE ((ISRRAN_NRE - 3U) * (1U << (ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR - 1U)) * 6U)

/**
 * @brief defines the maximum number of candidates for a given search-space and aggregation level according to TS 38.331
 * SearchSpace sequence
 */
#define ISRRAN_SEARCH_SPACE_MAX_NOF_CANDIDATES_NR 8

/**
 * @brief defines the maximum number of monitored PDCCH candidates per slot and per serving cell according to TS 38.213
 * Table 10.1-2
 */
#define ISRRAN_MAX_NOF_CANDIDATES_SLOT_NR 44

/**
 * @brief defines the maximum number of resource elements per PRB
 * @remark Defined in TS 38.214 V15.10.0 5.1.3.2 Transport block size determination, point 1, second bullet
 */
#define ISRRAN_MAX_NRE_NR 156

/**
 * @brief defines the maximum number of resource elements in a PDSCH transmission
 * @remark deduced from in TS 36.214 V15.10.0 5.1.3.2 Transport block size determination
 */
#define ISRRAN_PDSCH_MAX_RE_NR (ISRRAN_MAX_NRE_NR * ISRRAN_MAX_PRB_NR)

/**
 * @brief defines the maximum number of bits that can be transmitted in a slot
 */
#define ISRRAN_SLOT_MAX_NOF_BITS_NR (ISRRAN_PDSCH_MAX_RE_NR * ISRRAN_MAX_QM)

/**
 * @brief Maximum number of PDSCH time domain resource allocations. This is defined by TS 38.331 v15.10.0
 * as maxNrofDL-Allocations
 */
#define ISRRAN_MAX_NOF_TIME_RA 16

/**
 * @brief Maximum dl-DataToUL-ACK value. This is defined by TS 38.331 v15.10.1 in PUCCH-Config
 */
#define ISRRAN_MAX_NOF_DL_DATA_TO_UL 8

/**
 * @brief Maximum number of HARQ processes in the DL, signaled through RRC (PDSCH-ServingCellConfig)
 */
#define ISRRAN_MAX_HARQ_PROC_DL_NR 16 // 3GPP TS 38.214 version 15.3.0 Sec. 5.1 or nrofHARQ-ProcessesForPDSCH

/**
 * @brief Default number of HARQ processes in the DL, if config is absent.
 */
#define ISRRAN_DEFAULT_HARQ_PROC_DL_NR 8

/**
 * @brief Maximum number of HARQ processes in the UL, signaled through RRC (ConfiguredGrantConfig)
 */
#define ISRRAN_MAX_HARQ_PROC_UL_NR 16 // 3GPP TS 38.214 version 15.3.0 Sec. 6.1

/**
 * @brief SSB bandwidth in subcarriers, described in TS 38.211 section 7.4.3.1 Time-frequency structure of an SS/PBCH
 * block
 */
#define ISRRAN_SSB_BW_SUBC 240

/**
 * @brief SSB duration in symbols, described in TS 38.211 section 7.4.3.1 Time-frequency structure of an SS/PBCH block
 */
#define ISRRAN_SSB_DURATION_NSYMB 4

/**
 * @brief Number of NR N_id_1 Physical Cell Identifier (PCI) as described in TS 38.211 section 7.4.2.1 Physical-layer
 * cell identities
 */
#define ISRRAN_NOF_NID_1_NR 336

/**
 * @brief Number of NR N_id_2 Physical Cell Identifier (PCI) as described in TS 38.211 section 7.4.2.1 Physical-layer
 * cell identities
 */
#define ISRRAN_NOF_NID_2_NR 3

/**
 * @brief Number of NR N_id Physical Cell Identifier (PCI) as described in TS 38.211 section 7.4.2.1 Physical-layer
 * cell identities
 */
#define ISRRAN_NOF_NID_NR (ISRRAN_NOF_NID_1_NR * ISRRAN_NOF_NID_2_NR)

/**
 * @brief Compute N_id_1 from the Physical Cell Identifier (PCI) as described in TS 38.211 section 7.4.2.1
 * Physical-layer cell identities
 */
#define ISRRAN_NID_1_NR(N_ID) ((N_ID) / ISRRAN_NOF_NID_2_NR)

/**
 * @brief Compute N_id_2 from the Physical Cell Identifier (PCI) as described in TS 38.211 section 7.4.2.1
 * Physical-layer cell identities
 */
#define ISRRAN_NID_2_NR(N_ID) ((N_ID) % ISRRAN_NOF_NID_2_NR)

/**
 * @brief Compute Physical Cell Identifier (PCI) N_id from N_id_1 and N_id_2
 */
#define ISRRAN_NID_NR(NID_1, NID_2) (ISRRAN_NOF_NID_2_NR * (NID_1) + (NID_2))

/**
 * @brief SSB number of resource elements, described in TS 38.211 section 7.4.3.1 Time-frequency structure of an SS/PBCH
 * block
 */
#define ISRRAN_SSB_NOF_RE (ISRRAN_SSB_BW_SUBC * ISRRAN_SSB_DURATION_NSYMB)

/**
 * @brief Symbol index with extended CP
 */
#define ISRRAN_EXT_CP_SYMBOL(SCS) (7U << (uint32_t)(SCS))

typedef enum ISRRAN_API {
  isrran_coreset_mapping_type_non_interleaved = 0,
  isrran_coreset_mapping_type_interleaved,
} isrran_coreset_mapping_type_t;

typedef enum ISRRAN_API {
  isrran_coreset_bundle_size_n2 = 0,
  isrran_coreset_bundle_size_n3,
  isrran_coreset_bundle_size_n6,
} isrran_coreset_bundle_size_t;

uint32_t pdcch_nr_bundle_size(isrran_coreset_bundle_size_t x);

typedef enum ISRRAN_API {
  isrran_coreset_precoder_granularity_contiguous = 0,
  isrran_coreset_precoder_granularity_reg_bundle
} isrran_coreset_precoder_granularity_t;

/**
 * @brief PDSCH mapping type
 * @remark Described in TS 38.331 V15.10.0 Section PDSCH-TimeDomainResourceAllocationList
 */
typedef enum ISRRAN_API { isrran_sch_mapping_type_A = 0, isrran_sch_mapping_type_B } isrran_sch_mapping_type_t;

/**
 * @brief Search Space (SS) type
 * @remark Described in TS 38.213 V15.10.0 Section 10.1 UE procedure for determining physical downlink control channel
 * assignment
 */
typedef enum ISRRAN_API {
  isrran_search_space_type_common_0 = 0, ///< configured by pdcch-ConfigSIB1 in MIB or by searchSpaceSIB1 in
                                         ///< PDCCH-ConfigCommon or by searchSpaceZero in PDCCH-ConfigCommon
  isrran_search_space_type_common_0A,    ///< configured by searchSpaceOtherSystemInformation in PDCCH-ConfigCommon
  isrran_search_space_type_common_1,     ///< configured by ra-SearchSpace in PDCCH-ConfigCommon
  isrran_search_space_type_common_2,     ///< configured by pagingSearchSpace in PDCCH-ConfigCommon
  isrran_search_space_type_common_3,     ///< configured by SearchSpace in PDCCH-Config with searchSpaceType = common
  isrran_search_space_type_ue,  ///< configured by SearchSpace in PDCCH-Config with searchSpaceType = ue-Specific
  isrran_search_space_type_rar, ///< Indicates that a grant was given by MAC RAR as described in TS 38.213 clause 8.2
  isrran_search_space_type_cg   ///< Indicates that a grant was given by Configured Grant from the upper layers
} isrran_search_space_type_t;

/**
 * @brief Helper macro to get if a search space type is common or not
 */
#define ISRRAN_SEARCH_SPACE_IS_COMMON(SS_TYPE) ((SS_TYPE) < isrran_search_space_type_ue)

/**
 * @brief RAR content length in bits (see TS 38.321 Sec 6.2.3)
 */
#define ISRRAN_RAR_UL_GRANT_NBITS (27)

/**
 * @brief Indicates the MCS table the UE shall use for PDSCH and/or PUSCH without transform precoding
 */
typedef enum ISRRAN_API {
  isrran_mcs_table_64qam = 0,
  isrran_mcs_table_256qam,
  isrran_mcs_table_qam64LowSE,
  isrran_mcs_table_N
} isrran_mcs_table_t;

/**
 * @brief RNTI types
 * @remark Usage described in TS 38.321 Table 7.1-2: RNTI usage.
 */
typedef enum ISRRAN_API {
  isrran_rnti_type_c = 0,
  isrran_rnti_type_p,      ///< @brief Paging and System Information change notification (PCH)
  isrran_rnti_type_si,     ///< @brief Broadcast of System Information (DL-SCH)
  isrran_rnti_type_ra,     ///< @brief Random Access Response (DL-SCH)
  isrran_rnti_type_tc,     ///< @brief Contention Resolution (when no valid C-RNTI is available) (DL-SCH)
  isrran_rnti_type_cs,     ///< @brief Configured scheduled unicast transmission (DL-SCH, UL-SCH)
  isrran_rnti_type_sp_csi, ///< @brief Activation of Semi-persistent CSI reporting on PUSCH
  isrran_rnti_type_mcs_c,  ///< @brief Dynamically scheduled unicast transmission (DL-SCH)
} isrran_rnti_type_t;

/**
 * @brief DCI formats
 * @remark Described in TS 38.212 V15.9.0 Section 7.3.1 DCI formats
 */
typedef enum ISRRAN_API {
  isrran_dci_format_nr_0_0 = 0, ///< @brief Scheduling of PUSCH in one cell
  isrran_dci_format_nr_0_1,     ///< @brief Scheduling of PUSCH in one cell
  isrran_dci_format_nr_1_0,     ///< @brief Scheduling of PDSCH in one cell
  isrran_dci_format_nr_1_1,     ///< @brief Scheduling of PDSCH in one cell
  isrran_dci_format_nr_2_0,     ///< @brief Notifying a group of UEs of the slot format
  isrran_dci_format_nr_2_1, ///< @brief Notifying a group of UEs of the PRB(s) and OFDM symbol(s) where UE may assume no
                            ///< transmission is intended for the UE
  isrran_dci_format_nr_2_2, ///< @brief Transmission of TPC commands for PUCCH and PUSCH
  isrran_dci_format_nr_2_3, ///< @brief Transmission of a group of TPC commands for ISR transmissions by one or more UEs
  isrran_dci_format_nr_rar, ///< @brief Scheduling a transmission of PUSCH from RAR
  isrran_dci_format_nr_cg,  ///< @brief Scheduling of PUSCH using a configured grant
  ISRRAN_DCI_FORMAT_NR_COUNT ///< @brief Number of DCI formats
} isrran_dci_format_nr_t;

/**
 * @brief Overhead configuration provided by higher layers
 * @remark Described in TS 38.331 V15.10.0 PDSCH-ServingCellConfig
 * @remark Described in TS 38.331 V15.10.0 PUSCH-ServingCellConfig
 */
typedef enum ISRRAN_API {
  isrran_xoverhead_0 = 0,
  isrran_xoverhead_6,
  isrran_xoverhead_12,
  isrran_xoverhead_18
} isrran_xoverhead_t;

/**
 * @brief PDSCH HARQ ACK codebook configuration
 * @remark Described in TS 38.331 V15.10.0 PhysicalCellGroupConfig
 */
typedef enum ISRRAN_API {
  isrran_pdsch_harq_ack_codebook_none = 0,
  isrran_pdsch_harq_ack_codebook_semi_static,
  isrran_pdsch_harq_ack_codebook_dynamic,
} isrran_harq_ack_codebook_t;

/**
 * @brief PDSCH/PUSCH Resource allocation configuration
 * @remark Described in TS 38.331 V15.10.0 PhysicalCellGroupConfig
 */
typedef enum ISRRAN_API {
  isrran_resource_alloc_type0 = 0,
  isrran_resource_alloc_type1,
  isrran_resource_alloc_dynamic,
} isrran_resource_alloc_t;

/**
 * @brief Subcarrier spacing 15 or 30 kHz <6GHz and 60 or 120 kHz >6GHz
 * @remark Described in TS 38.331 V15.10.0 subcarrier spacing
 */

typedef enum ISRRAN_API {
  isrran_subcarrier_spacing_15kHz = 0,
  isrran_subcarrier_spacing_30kHz,
  isrran_subcarrier_spacing_60kHz,
  isrran_subcarrier_spacing_120kHz,
  isrran_subcarrier_spacing_240kHz,
  isrran_subcarrier_spacing_invalid
} isrran_subcarrier_spacing_t;

typedef enum ISRRAN_API {
  ISRRAN_SSB_PATTERN_A = 0, // FR1, 15 kHz SCS
  ISRRAN_SSB_PATTERN_B,     // FR1, 30 kHz SCS
  ISRRAN_SSB_PATTERN_C,     // FR1, 30 kHz SCS
  ISRRAN_SSB_PATTERN_D,     // FR2, 120 kHz SCS
  ISRRAN_SSB_PATTERN_E,     // FR2, 240 kHz SCS
  ISRRAN_SSB_PATTERN_INVALID,
} isrran_ssb_pattern_t;

typedef enum ISRRAN_API {
  ISRRAN_DUPLEX_MODE_FDD = 0, // Paired
  ISRRAN_DUPLEX_MODE_TDD,     // Unpaired
  ISRRAN_DUPLEX_MODE_SDL,     // Supplementary DownLink
  ISRRAN_DUPLEX_MODE_SUL,     // Supplementary UpLink
  ISRRAN_DUPLEX_MODE_INVALID
} isrran_duplex_mode_t;

/**
 * @brief Determines whether the first DMRS goes into symbol index 2 or 3
 */
typedef enum {
  isrran_dmrs_sch_typeA_pos_2 = 0, // Start in slot symbol index 2 (default)
  isrran_dmrs_sch_typeA_pos_3      // Start in slot symbol index 3
} isrran_dmrs_sch_typeA_pos_t;

/**
 * @brief NR carrier parameters. It is a combination of fixed cell and bandwidth-part (BWP)
 */
typedef struct ISRRAN_API {
  uint32_t                    pci;
  double                      dl_center_frequency_hz; ///< Absolute baseband center frequency in Hz for DL grid
  double                      ul_center_frequency_hz; ///< Absolute baseband center frequency in Hz for UL grid
  double                      ssb_center_freq_hz;     ///< SS/PBCH Block center frequency in Hz. Set to 0 if not present
  uint32_t                    offset_to_carrier; ///< Offset between point A and the lowest subcarrier of the lowest RB
  isrran_subcarrier_spacing_t scs;
  uint32_t                    nof_prb; ///< @brief See TS 38.101-1 Table 5.3.2-1 for more details
  uint32_t                    start;
  uint32_t max_mimo_layers; ///< @brief DL: Indicates the maximum number of MIMO layers to be used for PDSCH in all BWPs
                            ///< of this serving cell. (see TS 38.212 [17], clause 5.4.2.1). UL: Indicates the maximum
                            ///< MIMO layer to be used for PUSCH in all BWPs of the normal UL of this serving cell (see
                            ///< TS 38.212 [17], clause 5.4.2.1)
} isrran_carrier_nr_t;

#define ISRRAN_DEFAULT_CARRIER_NR                                                                                      \
  {                                                                                                                    \
    .pci = 500, .dl_center_frequency_hz = 117000 * 30e3, .ul_center_frequency_hz = 117000 * 30e3,                      \
    .ssb_center_freq_hz = 3.5e9, .offset_to_carrier = 0, .scs = isrran_subcarrier_spacing_15kHz, .nof_prb = 52,        \
    .start = 0, .max_mimo_layers = 1                                                                                   \
  }

/**
 * @brief NR Slot parameters. It contains parameters that change in a slot basis.
 */
typedef struct ISRRAN_API {
  /// Slot index in the radio frame
  uint32_t idx;

  /// Left for future parameters
  /// ...
} isrran_slot_cfg_t;

/**
 * @brief Min number of OFDM symbols in a control resource set.
 */
#define ISRRAN_CORESET_DURATION_MIN 1

/**
 * @brief Max number of OFDM symbols in a control resource set. Specified in TS 38.331 V15.10.0 as maxCoReSetDuration
 */
#define ISRRAN_CORESET_DURATION_MAX 3

/**
 * @brief Number of possible CORESET frequency resources.
 */
#define ISRRAN_CORESET_FREQ_DOMAIN_RES_SIZE 45

/**
 * @brief Max value for shift index
 */
#define ISRRAN_CORESET_SHIFT_INDEX_MAX (ISRRAN_CORESET_NOF_PRB_MAX - 1)

/**
 * @brief CORESET parameters as defined in TS 38.331 V15.10.0 - ControlResourceSet
 */
typedef struct ISRRAN_API {
  uint32_t                              id;
  isrran_coreset_mapping_type_t         mapping_type;
  uint32_t                              duration;
  bool                                  freq_resources[ISRRAN_CORESET_FREQ_DOMAIN_RES_SIZE];
  bool                                  dmrs_scrambling_id_present;
  uint32_t                              dmrs_scrambling_id;
  isrran_coreset_precoder_granularity_t precoder_granularity;
  isrran_coreset_bundle_size_t          interleaver_size; ///< Referenced in TS 38.211 section 7.3.2.2 as R
  isrran_coreset_bundle_size_t          reg_bundle_size;  ///< Referenced in TS 38.211 section 7.3.2.2 as L
  uint32_t                              shift_index;
  uint32_t offset_rb; ///< Integer offset in resource blocks from the pointA (lowest subcarrier of resource grid) to the
                      ///< lowest resource block of the CORESET region (used by CORESET Zero only)

  /** Missing TCI parameters */
} isrran_coreset_t;

/**
 * @brief SearchSpace parameters as defined in TS 38.331 v15.10.0 SearchSpace sequence
 */
typedef struct ISRRAN_API {
  uint32_t                   id;
  uint32_t                   coreset_id;
  uint32_t                   duration; ///< SS duration length in slots
  isrran_search_space_type_t type;     ///< Sets the SS type, common (multiple types) or UE specific
  isrran_dci_format_nr_t     formats[ISRRAN_DCI_FORMAT_NR_COUNT]; ///< Specifies the DCI formats that shall be searched
  uint32_t                   nof_formats;
  uint32_t                   nof_candidates[ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR];
} isrran_search_space_t;

/**
 * @brief TDD pattern configuration
 */
typedef struct ISRRAN_API {
  uint32_t period_ms;      ///< Period in milliseconds, set to 0 if not present
  uint32_t nof_dl_slots;   ///< Number of consecutive full DL slots at the beginning of each DL-UL pattern
  uint32_t nof_dl_symbols; ///< Number of consecutive DL symbols in the beginning of the slot following the last DL slot
  uint32_t nof_ul_slots;   ///< Number of consecutive full UL slots at the end of each DL-UL pattern
  uint32_t nof_ul_symbols; ///< Number of consecutive UL symbols in the end of the slot preceding the first full UL slot
} isrran_tdd_pattern_t;

/**
 * @brief TDD configuration as described in TS 38.331 v15.10.0 TDD-UL-DL-ConfigCommon
 */
typedef struct ISRRAN_API {
  isrran_tdd_pattern_t pattern1;
  isrran_tdd_pattern_t pattern2;
} isrran_tdd_config_nr_t;

/**
 * @brief Describes duplex configuration
 */
typedef struct ISRRAN_API {
  isrran_duplex_mode_t mode;
  union {
    isrran_tdd_config_nr_t tdd; ///< TDD configuration
    // ... add here other mode parameters
  };
} isrran_duplex_config_nr_t;

/**
 * @brief Describes a measurement based on NZP-CSI-RS or SSB-CSI
 * @note Used for tracking RSRP, SNR, CFO, SFO, and so on
 * @note isrran_csi_channel_measurements_t is used for CSI report generation
 */
typedef struct ISRRAN_API {
  float    rsrp;       ///< Linear scale RSRP
  float    rsrp_dB;    ///< Logarithm scale RSRP relative to full-scale
  float    epre;       ///< Linear scale EPRE
  float    epre_dB;    ///< Logarithm scale EPRE relative to full-scale
  float    n0;         ///< Linear noise level
  float    n0_dB;      ///< Logarithm scale noise level relative to full-scale
  float    snr_dB;     ///< Signal to noise ratio in decibels
  float    cfo_hz;     ///< Carrier frequency offset in Hz. Only set if more than 2 symbols are available in a TRS set
  float    cfo_hz_max; ///< Maximum CFO in Hz that can be measured. It is set to 0 if CFO cannot be estimated
  float    delay_us;   ///< Average measured delay in microseconds
  uint32_t nof_re;     ///< Number of available RE for the measurement, it can be used for weighting among different
  ///< measurements
} isrran_csi_trs_measurements_t;

/**
 * @brief Get the RNTI type name for NR
 * @param rnti_type RNTI type name
 * @return Constant string with the RNTI type name
 */
ISRRAN_API const char* isrran_rnti_type_str(isrran_rnti_type_t rnti_type);

/**
 * @brief Get the short RNTI type name for NR
 * @param rnti_type RNTI type name
 * @return Constant string with the short RNTI type name
 */
ISRRAN_API const char* isrran_rnti_type_str_short(isrran_rnti_type_t rnti_type);

/**
 * @brief Get the Search Space Type string for a given type
 * @param ss_type The given Search Space Type
 * @return The string describing the SS Type
 */
ISRRAN_API const char* isrran_ss_type_str(isrran_search_space_type_t ss_type);

/**
 * @brief Get the RNTI type name for NR
 * @param rnti_type RNTI type name
 * @return Constant string with the RNTI type name
 */
ISRRAN_API const char* isrran_dci_format_nr_string(isrran_dci_format_nr_t format);

/**
 * @brief Calculates the bandwidth of a given CORESET in physical resource blocks (PRB) . This function uses the
 * frequency domain resources bit-map for counting the number of PRB.
 *
 * @attention Prior to this function call, the frequency domain resources bit-map shall be zeroed beyond the
 * carrier.nof_prb / 6 index, otherwise the CORESET bandwidth might be greater than the carrier.
 *
 * @param coreset provides the given CORESET configuration
 * @return The number of PRB that the CORESET takes in frequency domain
 */
ISRRAN_API uint32_t isrran_coreset_get_bw(const isrran_coreset_t* coreset);

/**
 * @brief Calculates the number of Physical Resource Elements (time and frequency domain) that a given CORESET uses.
 * This function uses the frequency domain resources bit-map and duration to compute the number of symbols.
 *
 * @attention Prior to this function call, the frequency domain resources bit-map shall be zeroed beyond the
 * carrier.nof_prb / 6 index, otherwise the CORESET bandwidth might be greater than the carrier.
 *
 * @param coreset provides the given CORESET configuration
 * @return The number of resource elements that compose the coreset
 */
ISRRAN_API uint32_t isrran_coreset_get_sz(const isrran_coreset_t* coreset);

/**
 * @brief Calculates the starting resource block index in the resource grid
 *
 * @remark Intended to be used for common search space as specifies the lat clause in TS 38.214 section 5.1.2.2 Resource
 * allocation in frequency domain
 *
 * @param coreset provides the given CORESET configuration
 * @return The index of the lowest resource block in the resource grid used by the given CORESET if the CORESET
 * configuration is valid; Otherwise, 0.
 */
ISRRAN_API uint32_t isrran_coreset_start_rb(const isrran_coreset_t* coreset);

/**
 * @brief Get the NR PDSCH mapping type in string
 * @param mapping_type Mapping type
 * @return Constant string with PDSCH mapping type
 */
ISRRAN_API const char* isrran_sch_mapping_type_to_str(isrran_sch_mapping_type_t mapping_type);

/**
 * @brief Get the MCS table string
 * @param mcs_table MCS table value
 * @return Constant string with the MCS table name
 */
ISRRAN_API const char* isrran_mcs_table_to_str(isrran_mcs_table_t mcs_table);

/**
 * @brief Get the MCS table value from a string
 * @param str Points to a given string
 * @return The MCS table value
 */
ISRRAN_API isrran_mcs_table_t isrran_mcs_table_from_str(const char* str);

/**
 * @brief Computes the minimum valid symbol size for a given amount of PRB
 * @attention The valid FFT sizes are radix 2 and radix 3 between 128 to 4096 points.
 * @param nof_prb Number of PRB
 * @return The minimum valid FFT size if the number of PRB is in range, 0 otherwise
 */
ISRRAN_API uint32_t isrran_min_symbol_sz_rb(uint32_t nof_prb);

/**
 * @brief Computes the minimum valid symbol size for a given amount of PRB
 * @attention The valid FFT sizes are radix 2 and radix 3 between 128 to 4096 points.
 * @param nof_prb Number of PRB
 * @return The minimum valid FFT size if the number of PRB is in range, 0 otherwise
 */
ISRRAN_API int isrran_symbol_sz_from_srate(double srate_hz, isrran_subcarrier_spacing_t scs);

/**
 * @brief Computes the time in seconds between the beginning of the slot and the given symbol
 * @remark All symbol size reference and values are taken from TS 38.211 section 5.3 OFDM baseband signal generation
 * @param l Given symbol index
 * @param scs Subcarrier spacing
 * @return Returns the symbol time offset in seconds
 */
ISRRAN_API float isrran_symbol_offset_s(uint32_t l, isrran_subcarrier_spacing_t scs);

/**
 * @brief Computes the time in seconds between two symbols in a slot
 * @note l0 is expected to be smaller than l1
 * @remark All symbol size reference and values are taken from TS 38.211 section 5.3 OFDM baseband signal generation
 * @param l0 First symbol index within the slot
 * @param l1 Second symbol index within the slot
 * @param scs Subcarrier spacing
 * @return Returns the time in seconds between the two symbols if the condition above is satisfied, 0 seconds otherwise
 */
ISRRAN_API float isrran_symbol_distance_s(uint32_t l0, uint32_t l1, isrran_subcarrier_spacing_t scs);

/**
 * @brief Decides whether a given slot is configured as Downlink
 * @param cfg Provides the carrier duplex configuration
 * @param numerology Provides BWP numerology
 * @param slot_idx Slot index in the frame for the given numerology
 * @return true if the provided slot index is configured for Downlink
 */
ISRRAN_API bool isrran_duplex_nr_is_dl(const isrran_duplex_config_nr_t* cfg, uint32_t numerology, uint32_t slot_idx);

/**
 * @brief Decides whether a given slot is configured as Uplink
 * @param cfg Provides the carrier duplex configuration
 * @param numerology Provides BWP numerology
 * @param slot_idx Slot index in the frame for the given numerology
 * @return true if the provided slot index is configured for Uplink
 */
ISRRAN_API bool isrran_duplex_nr_is_ul(const isrran_duplex_config_nr_t* cfg, uint32_t numerology, uint32_t slot_idx);

ISRRAN_API int isrran_carrier_to_cell(const isrran_carrier_nr_t* carrier, isrran_cell_t* cell);

/**
 * @brief Writes detailed Channel State Information measurement into a string
 * @param meas Provides the measurement
 * @param str Provides string
 * @param str_len Maximum string length
 * @return The number of writen characters
 */
ISRRAN_API uint32_t isrran_csi_meas_info(const isrran_csi_trs_measurements_t* meas, char* str, uint32_t str_len);

/**
 * @brief Writes short Channel State Information measurement into a string
 * @param meas Provides the measurement
 * @param str Provides string
 * @param str_len Maximum string length
 * @return The number of writen characters
 */
ISRRAN_API uint32_t isrran_csi_meas_info_short(const isrran_csi_trs_measurements_t* meas, char* str, uint32_t str_len);

/**
 * @brief Converts a given string into a subcarrier spacing
 * @param str Provides the string
 * @return A valid subcarrier if the string is valid, isrran_subcarrier_spacing_invalid otherwise
 */
ISRRAN_API isrran_subcarrier_spacing_t isrran_subcarrier_spacing_from_str(const char* str);

/**
 * @brief Converts a given subcarrier spacing to string
 * @param scs Subcarrier spacing
 * @return A constant string pointer
 */
ISRRAN_API const char* isrran_subcarrier_spacing_to_str(isrran_subcarrier_spacing_t scs);

/**
 * @brief Combine Channel State Information from Tracking Reference Signals (CSI-TRS)
 * @param[in] a CSI-RS measurement
 * @param[in] b CSI-RS measurement
 * @param[out] dst Destination of the combined
 */
ISRRAN_API void isrran_combine_csi_trs_measurements(const isrran_csi_trs_measurements_t* a,
                                                    const isrran_csi_trs_measurements_t* b,
                                                    isrran_csi_trs_measurements_t*       dst);

/**
 * @brief Setup CORESET Zero from a configuration index
 * @remark Defined by TS 38.213 tables 13-1, 13-2, 13-3, 13-4, 13-5, 13-6,  13-7,  13-8,  13-9,  13-10
 * @param n_cell_id Physical Cell identifier
 * @param ssb_pointA_freq_offset_Hz Integer frequency offset in Hz between the SS/PBCH block center and pointA
 * @param ssb_scs SS/PBCH block subcarrier spacing
 * @param pdcch_scs PDCCH subcarrier spacing
 * @param idx CORESET Zero configuration index
 * @param[out] coreset Points to the resultant CORESET
 * @return ISRRAN_SUCCESS if the given inputs lead to a valid CORESET configuration, otherise ISRRAN_ERROR code
 */
ISRRAN_API int isrran_coreset_zero(uint32_t                    n_cell_id,
                                   uint32_t                    ssb_pointA_freq_offset_Hz,
                                   isrran_subcarrier_spacing_t ssb_scs,
                                   isrran_subcarrier_spacing_t pdcch_scs,
                                   uint32_t                    idx,
                                   isrran_coreset_t*           coreset);

/**
 * @brief Obtain offset in RBs between CoresetZero and SSB. See TS 38.213, Tables 13-{1,...,10}
 * @param idx Index of 13-{1,...10} table
 * @param ssb_scs SS/PBCH block subcarrier spacing
 * @param pdcch_scs PDCCH subcarrier spacing
 * @return offset in RBs, or -1 in case of invalid inputs
 */
ISRRAN_API int
isrran_coreset0_ssb_offset(uint32_t idx, isrran_subcarrier_spacing_t ssb_scs, isrran_subcarrier_spacing_t pdcch_scs);

/**
 * @brief Convert Coreset to string
 *
 * @param coreset The coreset structure as input
 * @param str The string to write to
 * @param str_len Maximum string length
 * @return ISRRAN_API
 */
ISRRAN_API int isrran_coreset_to_str(isrran_coreset_t* coreset, char* str, uint32_t str_len);

/**
 * @brief Convert SSB pattern to string
 * @param pattern
 * @return a string describing the SSB pattern
 */
ISRRAN_API const char* isrran_ssb_pattern_to_str(isrran_ssb_pattern_t pattern);

/**
 * @brief Convert string to SSB pattern
 * @param str String to convert
 * @return The pattern, ISRRAN_SSB_PATTERN_INVALID if string is invalid
 */
ISRRAN_API isrran_ssb_pattern_t isrran_ssb_pattern_fom_str(const char* str);

/**
 * @brief Compares if two NR carrier structures are equal
 * @param a First carrier to compare
 * @param b Second carrier to compare
 * @return True if all the carrier structure fields are equal, otherwise false
 */
ISRRAN_API bool isrran_carrier_nr_equal(const isrran_carrier_nr_t* a, const isrran_carrier_nr_t* b);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_PHY_COMMON_NR_H
