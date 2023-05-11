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
 *  File:         pdsch_cfg_nr.h
 *
 *  Description:  Physical downlink shared channel configuration
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_PHCH_CFG_NR_H
#define ISRRAN_PHCH_CFG_NR_H

#include "isrran/phy/ch_estimation/csi_rs_cfg.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/phch/sch_cfg_nr.h"
#include "isrran/phy/phch/uci_cfg_nr.h"
#include "isrran/phy/utils/re_pattern.h"

/**
 * @brief Specifies the maximum number of ZP-CSI-RS resources configured per slot. It is not implicitly specified in the
 * TS.
 */
#define ISRRAN_PHCH_CFG_MAX_NOF_ZP_CSI_RS_RES_PER_SLOT 16

/**
 * @brief PDSCH DMRS type
 */
typedef enum {
  isrran_dmrs_sch_type_1 = 0, // 1 pilot every 2 sub-carriers (default)
  isrran_dmrs_sch_type_2      // 2 consecutive pilots every 6 sub-carriers
} isrran_dmrs_sch_type_t;

/**
 * @brief PDSCH DMRS length in symbols
 */
typedef enum {
  isrran_dmrs_sch_len_1 = 0, // single, 1 symbol long (default)
  isrran_dmrs_sch_len_2      // double, 2 symbol long
} isrran_dmrs_sch_len_t;

/**
 * @brief Determines additional symbols if possible to be added
 */
typedef enum {
  isrran_dmrs_sch_add_pos_2 = 0,
  isrran_dmrs_sch_add_pos_0,
  isrran_dmrs_sch_add_pos_1,
  isrran_dmrs_sch_add_pos_3
} isrran_dmrs_sch_add_pos_t;

/**
 * @brief Provides PDSCH DMRS configuration
 * @remark Parameters described in TS 38.331 V15.10.0
 */
typedef struct {
  /// Parameters provided by IE DMRS-DownlinkConfig
  isrran_dmrs_sch_type_t    type;
  isrran_dmrs_sch_add_pos_t additional_pos;
  isrran_dmrs_sch_len_t     length;
  bool                      scrambling_id0_present;
  uint32_t                  scrambling_id0;
  bool                      scrambling_id1_present;
  uint32_t                  scrambling_id1;

  /// Parameters provided by ServingCellConfigCommon
  isrran_dmrs_sch_typeA_pos_t typeA_pos;
  bool                        lte_CRS_to_match_around;

  uint32_t reference_point_k_rb;

  /// Parameters provided by FeatureSetDownlink-v1540
  bool additional_DMRS_DL_Alt;

} isrran_dmrs_sch_cfg_t;

/**
 * @brief Common flatten PDSCH and PUSCH time domain allocation parameters
 * @remark Described in TS 38.331 V15.10.0 Section PDSCH-TimeDomainResourceAllocationList
 * @remark Described in TS 38.331 V15.10.0 Section PUSCH-TimeDomainResourceAllocationList
 */
typedef struct ISRRAN_API {
  /// For PDSCH Slot offset between DCI and its scheduled PDSCH
  /// For PUSCH parameter K2
  uint32_t k;

  /// SCH mapping type
  isrran_sch_mapping_type_t mapping_type;

  /// An index giving valid combinations of start symbol and length (jointly encoded) as start and length indicator
  /// (SLIV). The network configures the field so that the allocation does not cross the slot boundary
  uint32_t sliv;

} isrran_sch_time_ra_t;

/**
 * @brief PDSCH grant information provided by the Downlink Control Information (DCI)
 */
typedef struct ISRRAN_API {
  /// UE identifier
  uint16_t           rnti;
  isrran_rnti_type_t rnti_type;

  /// Time domain resources
  uint32_t                  k; // k0 for PDSCH, k2 for PUSCH
  uint32_t                  S;
  uint32_t                  L;
  isrran_sch_mapping_type_t mapping;

  /// Frequency domain resources
  bool     prb_idx[ISRRAN_MAX_PRB_NR];
  uint32_t nof_prb;

  /// Number of DMRS groups without data
  /// Described in TS 38.214 Section 5.1.6.2
  uint32_t nof_dmrs_cdm_groups_without_data;

  /// Linear DMRS power offset. Zero means unset and it is equivalent to one.
  /// For PUSCH, see TS 38.214 Table 6.2.2-1 for more information
  float beta_dmrs;

  /// Spatial resources
  uint32_t nof_layers;

  /// DMRS Scrambling sequence initialization (false: 0 or true: 1)
  bool n_scid;

  /// DCI information
  isrran_dci_format_nr_t     dci_format;
  isrran_search_space_type_t dci_search_space;

  /// Transport block
  uint32_t tb_scaling_field;
  /// ....

  isrran_sch_tb_t tb[ISRRAN_MAX_TB];
} isrran_sch_grant_nr_t;

/**
 * @brief Beta offset configuration provided from upper layers
 * @remark Configure according to TS 38.331 BetaOffsets
 */
typedef struct {
  uint32_t ack_index1;  ///< Use for up to 2 HARQ-ACK bits. Set to 11 if absent.
  uint32_t ack_index2;  ///< Use for up to 11 HARQ-ACK bits. Set to 11 if absent.
  uint32_t ack_index3;  ///< Use for more than 11 HARQ-ACK bits. Set to 11 if absent.
  uint32_t csi1_index1; ///< Use for up to 11 CSI bits. Set to 13 if absent.
  uint32_t csi1_index2; ///< Use for more than 11 CSI bits. Set to 13 if absent.
  uint32_t csi2_index1; ///< Use for up to 11 CSI bits. Set to 13 if absent.
  uint32_t csi2_index2; ///< Use for more than 11 CSI bits. Set to 13 if absent.

  /// Fix values for testing purposes
  float fix_ack; ///< Set to a non-zero value for fixing a beta offset value
  float fix_csi1;
  float fix_csi2;
} isrran_beta_offsets_t;

/**
 * @brief flatten SCH configuration parameters provided by higher layers
 * @remark Described in TS 38.331 V15.10.0 Section PDSCH-Config
 * @remark Described in TS 38.331 V15.10.0 Section PUSCH-Config
 */
typedef struct ISRRAN_API {
  // Serving cell parameters
  uint32_t                    scs_cfg; // Subcarrier spacing configuration
  isrran_dmrs_sch_typeA_pos_t typeA_pos;

  bool     scrambling_id_present;
  uint32_t scambling_id; // Identifier used to initialize data scrambling (0-1023)

  isrran_mcs_table_t     mcs_table;
  isrran_dmrs_sch_type_t dmrs_type;
  isrran_dmrs_sch_len_t  dmrs_max_length;
  struct {
    isrran_dmrs_sch_add_pos_t additional_pos;
    bool                      scrambling_id0_present;
    uint32_t                  scrambling_id0;
    bool                      scrambling_id1_present;
    uint32_t                  scrambling_id1;
    bool                      present;
  } dmrs_typeA;

  struct {
    isrran_dmrs_sch_add_pos_t additional_pos;
    bool                      scrambling_id0_present;
    uint32_t                  scrambling_id0;
    bool                      scrambling_id1_present;
    uint32_t                  scrambling_id1;
    bool                      present;
  } dmrs_typeB;

  isrran_sch_time_ra_t common_time_ra[ISRRAN_MAX_NOF_TIME_RA];
  uint32_t             nof_common_time_ra;

  isrran_sch_time_ra_t dedicated_time_ra[ISRRAN_MAX_NOF_TIME_RA];
  uint32_t             nof_dedicated_time_ra;

  bool                    rbg_size_cfg_1; ///< RBG size configuration (1 or 2)
  isrran_resource_alloc_t alloc;

  isrran_sch_cfg_t sch_cfg; ///< Common shared channel parameters

  /// PDSCH Periodic ZP-CSI-RS set
  isrran_csi_rs_zp_set_t p_zp_csi_rs_set;

  /// PDSCH Periodic NZP-CSI-RS set, indexed by nzp-CSI-ResourceSetId
  isrran_csi_rs_nzp_set_t nzp_csi_rs_sets[ISRRAN_PHCH_CFG_MAX_NOF_CSI_RS_SETS];

  /// PUSCH only
  isrran_beta_offsets_t beta_offsets;              /// Semi-static only.
  bool                  enable_transform_precoder; /// Enables transform precoding
  float scaling; /// Indicates a scaling factor to limit the number of resource elements assigned to UCI on PUSCH.
} isrran_sch_hl_cfg_nr_t;

/**
 * @brief Common NR-SCH (PDSCH and PUSCH for NR) configuration
 */
typedef struct ISRRAN_API {
  bool     scrambling_id_present;
  uint32_t scambling_id; ///< Identifier used to initialize data scrambling (0-1023)

  isrran_dmrs_sch_cfg_t    dmrs;    ///< DMRS configuration for this transmission
  isrran_sch_grant_nr_t    grant;   ///< Actual SCH grant
  isrran_sch_cfg_t         sch_cfg; ///< Common shared channel parameters
  isrran_re_pattern_list_t rvd_re;  ///< Reserved resource elements, as pattern

  /// PUSCH only parameters
  isrran_uci_cfg_nr_t uci; ///< Uplink Control Information configuration
  bool                enable_transform_precoder;
  bool                freq_hopping_enabled;
} isrran_sch_cfg_nr_t;

ISRRAN_API uint32_t isrran_sch_cfg_nr_nof_re(const isrran_sch_cfg_nr_t* sch_cfg);

ISRRAN_API uint32_t isrran_sch_cfg_nr_info(const isrran_sch_cfg_nr_t* sch_cfg, char* str, uint32_t str_len);

#endif // ISRRAN_PHCH_CFG_NR_H
