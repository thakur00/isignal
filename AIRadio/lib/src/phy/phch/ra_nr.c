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

#include "isrran/phy/phch/ra_nr.h"
#include "isrran/phy/ch_estimation/csi_rs.h"
#include "isrran/phy/fec/cbsegm.h"
#include "isrran/phy/phch/csi.h"
#include "isrran/phy/phch/pdsch_nr.h"
#include "isrran/phy/phch/ra_dl_nr.h"
#include "isrran/phy/phch/ra_ul_nr.h"
#include "isrran/phy/phch/uci_nr.h"
#include "isrran/phy/utils/debug.h"
#include <math.h> /* floor */

typedef struct {
  isrran_mod_t modulation;
  double       R; // Target code Rate R x [1024]
  double       S; // Spectral efficiency
} mcs_entry_t;

#define RA_NR_MCS_SIZE_TABLE1 29
#define RA_NR_MCS_SIZE_TABLE2 28
#define RA_NR_MCS_SIZE_TABLE3 29
#define RA_NR_TBS_SIZE_TABLE 93
#define RA_NR_CQI_TABLE_SIZE 16
#define RA_NR_BETA_OFFSET_HARQACK_SIZE 32
#define RA_NR_BETA_OFFSET_CSI_SIZE 32

#define RA_NR_READ_TABLE(N)                                                                                            \
  static double isrran_ra_nr_R_from_mcs_table##N(uint32_t mcs_idx)                                                     \
  {                                                                                                                    \
    if (mcs_idx >= RA_NR_MCS_SIZE_TABLE##N) {                                                                          \
      return NAN;                                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
    return ra_nr_table##N[mcs_idx].R;                                                                                  \
  }                                                                                                                    \
                                                                                                                       \
  static isrran_mod_t isrran_ra_nr_modulation_from_mcs_table##N(uint32_t mcs_idx)                                      \
  {                                                                                                                    \
    if (mcs_idx >= RA_NR_MCS_SIZE_TABLE##N) {                                                                          \
      return ISRRAN_MOD_NITEMS;                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    return ra_nr_table##N[mcs_idx].modulation;                                                                         \
  }

/**
 * TS 38.214 V15.10.0 Table 5.1.3.1-1: MCS index table 1 for PDSCH
 */
static const mcs_entry_t ra_nr_table1[RA_NR_MCS_SIZE_TABLE1] = {
    {ISRRAN_MOD_QPSK, 120, 0.2344},  {ISRRAN_MOD_QPSK, 157, 0.3066},  {ISRRAN_MOD_QPSK, 193, 0.3770},
    {ISRRAN_MOD_QPSK, 251, 0.4902},  {ISRRAN_MOD_QPSK, 308, 0.6016},  {ISRRAN_MOD_QPSK, 379, 0.7402},
    {ISRRAN_MOD_QPSK, 449, 0.8770},  {ISRRAN_MOD_QPSK, 526, 1.0273},  {ISRRAN_MOD_QPSK, 602, 1.1758},
    {ISRRAN_MOD_QPSK, 679, 1.3262},  {ISRRAN_MOD_16QAM, 340, 1.3281}, {ISRRAN_MOD_16QAM, 378, 1.4766},
    {ISRRAN_MOD_16QAM, 434, 1.6953}, {ISRRAN_MOD_16QAM, 490, 1.9141}, {ISRRAN_MOD_16QAM, 553, 2.1602},
    {ISRRAN_MOD_16QAM, 616, 2.4063}, {ISRRAN_MOD_16QAM, 658, 2.5703}, {ISRRAN_MOD_64QAM, 438, 2.5664},
    {ISRRAN_MOD_64QAM, 466, 2.7305}, {ISRRAN_MOD_64QAM, 517, 3.0293}, {ISRRAN_MOD_64QAM, 567, 3.3223},
    {ISRRAN_MOD_64QAM, 616, 3.6094}, {ISRRAN_MOD_64QAM, 666, 3.9023}, {ISRRAN_MOD_64QAM, 719, 4.2129},
    {ISRRAN_MOD_64QAM, 772, 4.5234}, {ISRRAN_MOD_64QAM, 822, 4.8164}, {ISRRAN_MOD_64QAM, 873, 5.1152},
    {ISRRAN_MOD_64QAM, 910, 5.3320}, {ISRRAN_MOD_64QAM, 948, 5.5547}};

/**
 * TS 38.214 V15.10.0 Table 5.1.3.1-2: MCS index table 2 for PDSCH
 */
static const mcs_entry_t ra_nr_table2[RA_NR_MCS_SIZE_TABLE2] = {
    {ISRRAN_MOD_QPSK, 120, 0.2344},   {ISRRAN_MOD_QPSK, 193, 0.3770},   {ISRRAN_MOD_QPSK, 308, 0.6016},
    {ISRRAN_MOD_QPSK, 449, 0.8770},   {ISRRAN_MOD_QPSK, 602, 1.1758},   {ISRRAN_MOD_16QAM, 378, 1.4766},
    {ISRRAN_MOD_16QAM, 434, 1.6953},  {ISRRAN_MOD_16QAM, 490, 1.9141},  {ISRRAN_MOD_16QAM, 553, 2.1602},
    {ISRRAN_MOD_16QAM, 616, 2.4063},  {ISRRAN_MOD_16QAM, 658, 2.5703},  {ISRRAN_MOD_64QAM, 466, 2.7305},
    {ISRRAN_MOD_64QAM, 517, 3.0293},  {ISRRAN_MOD_64QAM, 567, 3.3223},  {ISRRAN_MOD_64QAM, 616, 3.6094},
    {ISRRAN_MOD_64QAM, 666, 3.9023},  {ISRRAN_MOD_64QAM, 719, 4.2129},  {ISRRAN_MOD_64QAM, 772, 4.5234},
    {ISRRAN_MOD_64QAM, 822, 4.8164},  {ISRRAN_MOD_64QAM, 873, 5.1152},  {ISRRAN_MOD_256QAM, 682.5, 5.3320},
    {ISRRAN_MOD_256QAM, 711, 5.5547}, {ISRRAN_MOD_256QAM, 754, 5.8906}, {ISRRAN_MOD_256QAM, 797, 6.2266},
    {ISRRAN_MOD_256QAM, 841, 6.5703}, {ISRRAN_MOD_256QAM, 885, 6.9141}, {ISRRAN_MOD_256QAM, 916.5, 7.1602},
    {ISRRAN_MOD_256QAM, 948, 7.4063}};

/**
 * TS 38.214 V15.10.0 Table 5.1.3.1-3: MCS index table 3 for PDSCH
 */
static const mcs_entry_t ra_nr_table3[RA_NR_MCS_SIZE_TABLE3] = {
    {ISRRAN_MOD_QPSK, 30, 0.0586},   {ISRRAN_MOD_QPSK, 40, 0.0781},   {ISRRAN_MOD_QPSK, 50, 0.0977},
    {ISRRAN_MOD_QPSK, 64, 0.1250},   {ISRRAN_MOD_QPSK, 78, 0.1523},   {ISRRAN_MOD_QPSK, 99, 0.1934},
    {ISRRAN_MOD_QPSK, 120, 0.2344},  {ISRRAN_MOD_QPSK, 157, 0.3066},  {ISRRAN_MOD_QPSK, 193, 0.3770},
    {ISRRAN_MOD_QPSK, 251, 0.4902},  {ISRRAN_MOD_QPSK, 308, 0.6016},  {ISRRAN_MOD_QPSK, 379, 0.7402},
    {ISRRAN_MOD_QPSK, 449, 0.8770},  {ISRRAN_MOD_QPSK, 526, 1.0273},  {ISRRAN_MOD_QPSK, 602, 1.1758},
    {ISRRAN_MOD_16QAM, 340, 1.3281}, {ISRRAN_MOD_16QAM, 378, 1.4766}, {ISRRAN_MOD_16QAM, 434, 1.6953},
    {ISRRAN_MOD_16QAM, 490, 1.9141}, {ISRRAN_MOD_16QAM, 553, 2.1602}, {ISRRAN_MOD_16QAM, 616, 2.4063},
    {ISRRAN_MOD_64QAM, 438, 2.5664}, {ISRRAN_MOD_64QAM, 466, 2.7305}, {ISRRAN_MOD_64QAM, 517, 3.0293},
    {ISRRAN_MOD_64QAM, 567, 3.3223}, {ISRRAN_MOD_64QAM, 616, 3.6094}, {ISRRAN_MOD_64QAM, 666, 3.9023},
    {ISRRAN_MOD_64QAM, 719, 4.2129}, {ISRRAN_MOD_64QAM, 772, 4.5234}};

/**
 * Generate MCS table access functions
 */
RA_NR_READ_TABLE(1)
RA_NR_READ_TABLE(2)
RA_NR_READ_TABLE(3)

/**
 * TS 38.214 V15.10.0 Table 5.1.3.2-1: TBS for N info ≤ 3824
 */
static const uint32_t ra_nr_tbs_table[RA_NR_TBS_SIZE_TABLE] = {
    24,   32,   40,   48,   56,   64,   72,   80,   88,   96,   104,  112,  120,  128,  136,  144,  152,  160,  168,
    176,  184,  192,  208,  224,  240,  256,  272,  288,  304,  320,  336,  352,  368,  384,  408,  432,  456,  480,
    504,  528,  552,  576,  608,  640,  672,  704,  736,  768,  808,  848,  888,  928,  984,  1032, 1064, 1128, 1160,
    1192, 1224, 1256, 1288, 1320, 1352, 1416, 1480, 1544, 1608, 1672, 1736, 1800, 1864, 1928, 2024, 2088, 2152, 2216,
    2280, 2408, 2472, 2536, 2600, 2664, 2728, 2792, 2856, 2976, 3104, 3240, 3368, 3496, 3624, 3752, 3824};

/**
 * TS 38.213 V15.10.0 Table 9.3-1: Mapping of beta_offset values for HARQ-ACK information and the index signalled by
 * higher layers
 */
static const float ra_nr_beta_offset_ack_table[RA_NR_BETA_OFFSET_HARQACK_SIZE] = {
    1.000f,  2.000f,  2.500f,  3.125f,  4.000f,   5.000f, 6.250f, 8.000f, 10.000f, 12.625f, 15.875f,
    20.000f, 31.000f, 50.000f, 80.000f, 126.000f, NAN,    NAN,    NAN,    NAN,     NAN,     NAN,
    NAN,     NAN,     NAN,     NAN,     NAN,      NAN,    NAN,    NAN,    NAN,     NAN};

/**
 * TS 38.213 V15.10.0 Table 9.3-2: Mapping of beta_offset values for CSI and the index signalled by higher layers
 */
static const float ra_nr_beta_offset_csi_table[RA_NR_BETA_OFFSET_CSI_SIZE] = {
    1.125f, 1.250f, 1.375f, 1.625f, 1.750f,  2.000f,  2.250f,  2.500f,  2.875f, 3.125f, 3.500f,
    4.000f, 5.000f, 6.250f, 8.000f, 10.000f, 12.625f, 15.875f, 20.000f, NAN,    NAN,    NAN,
    NAN,    NAN,    NAN,    NAN,    NAN,     NAN,     NAN,     NAN,     NAN,    NAN};

typedef enum { ra_nr_table_idx_1 = 0, ra_nr_table_idx_2, ra_nr_table_idx_3 } ra_nr_table_idx_t;

/**
 * The table below performs the mapping of the CQI into the closest MCS, based on the corresponding spectral efficiency.
 * The mapping works as follows:
 * - select spectral efficiency from the CQI from tables Table 5.2.2.1-2, Table 5.2.2.1-3, or Table 5.2.2.1-4,
 *   TS 38.214 V15.14.0
 * - select MCS corresponding to same spectral efficiency from Table 5.1.3.1-1, Table 5.1.3.1-2, or Table 5.1.3.1-3,
 *   TS 38.214 V15.14.0
 *
 * The array ra_nr_cqi_to_mcs_table[CQI_table_idx][MCS_table_idx][CQI] contains the MCS corresponding to CQI, based on
 * the given CQI_table_idx and MCS_table_idx tables
 * CQI_table_idx: 1 -> Table 5.2.2.1-2; 2 -> Table 5.2.2.1-3, 3 -> Table 5.2.2.1-4
 * MCS_table_idx: 1 -> Table 5.1.3.1-1; 2 -> Table 5.1.3.1-2; 3 -> Table 5.1.3.1-3
 */

static const int ra_nr_cqi_to_mcs_table[3][3][RA_NR_CQI_TABLE_SIZE] = {
    /* ROW 1 - CQI Table 1 */
    {/* MCS Table 1 */ {-1, 0, 0, 2, 4, 6, 8, 11, 13, 15, 18, 20, 22, 24, 26, 28},
     /* MCS Table 2 */ {-1, 0, 0, 1, 2, 3, 4, 5, 7, 9, 11, 13, 15, 17, 19, 21},
     /* MCS Table 3 */ {-1, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 28, 28}},
    /* ROW 2 - CQI Table 2 */
    {/* MCS Table 1 */ {-1, 0, 2, 6, 11, 13, 15, 18, 20, 22, 24, 26, 28, 28, 28, 28},
     /* MCS Table 2 */ {-1, 0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27},
     /* MCS Table 3 */ {-1, 4, 8, 12, 16, 18, 20, 22, 24, 26, 28, 28, 28, 28, 28, 28}},
    /* ROW 3 - CQI Table 3 */
    {/* MCS Table 1 */ {-1, 0, 0, 0, 0, 2, 4, 6, 8, 11, 13, 15, 18, 20, 22, 24},
     /* MCS Table 2 */ {-1, 0, 0, 0, 0, 1, 2, 3, 4, 5, 7, 9, 11, 13, 15, 17},
     /* MCS Table 3 */ {-1, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28}}};

/**
 * The CQI to Spectral efficiency table.
 * The array ra_nr_cqi_to_se_table[CQI_table_idx][CQI] contains the Spectral Efficiency corresponding to CQI, based on
 * the given CQI_table_idx:
 * CQI_table_idx: 1 -> Table 5.2.2.1-2; 2 -> Table 5.2.2.1-3, 3 -> Table 5.2.2.1-4
 */
static const double ra_nr_cqi_to_se_table[3][RA_NR_CQI_TABLE_SIZE] = {
    /* ROW 1 - CQI Table 1 */
    {-1,
     0.1523,
     0.2344,
     0.3770,
     0.6016,
     0.8770,
     1.1758,
     1.4766,
     1.9141,
     2.4063,
     2.7305,
     3.3223,
     3.9023,
     4.5234,
     5.1152,
     5.5547},
    /* ROW 2 - CQI Table 2 */
    {-1,
     0.1523,
     0.3770,
     0.8770,
     1.4766,
     1.9141,
     2.4063,
     2.7305,
     3.3223,
     3.9023,
     4.5234,
     5.1152,
     5.5547,
     6.2266,
     6.9141,
     7.4063},
    /* ROW 3 - CQI Table 3 */
    {-1,
     0.0586,
     0.0977,
     0.1523,
     0.2344,
     0.3770,
     0.6016,
     0.8770,
     1.1758,
     1.4766,
     1.9141,
     2.4063,
     2.7305,
     3.3223,
     3.9023,
     4.5234}};

static ra_nr_table_idx_t ra_nr_select_table_pusch_noprecoding(isrran_mcs_table_t         mcs_table,
                                                              isrran_dci_format_nr_t     dci_format,
                                                              isrran_search_space_type_t search_space_type,
                                                              isrran_rnti_type_t         rnti_type)
{
  // Non-implemented parameters
  bool mcs_c_rnti = false;

  // - if mcs-Table in pusch-Config is set to 'qam256', and
  // - PUSCH is scheduled by a PDCCH with DCI format 0_1 with
  // - CRC scrambled by C-RNTI or SP-CSI-RNTI,
  if (mcs_table == isrran_mcs_table_256qam && dci_format == isrran_dci_format_nr_0_1 &&
      (rnti_type == isrran_rnti_type_c || rnti_type == isrran_rnti_type_sp_csi)) {
    return ra_nr_table_idx_2;
  }

  // - the UE is not configured with MCS-C-RNTI,
  // - mcs-Table in pusch-Config is set to 'qam64LowSE', and the
  // - PUSCH is scheduled by a PDCCH in a UE-specific search space with
  // - CRC scrambled by C-RNTI or SP-CSI-RNTI,
  if (!mcs_c_rnti && mcs_table == isrran_mcs_table_qam64LowSE && dci_format != isrran_dci_format_nr_rar &&
      search_space_type == isrran_search_space_type_ue &&
      (rnti_type == isrran_rnti_type_c || rnti_type == isrran_rnti_type_sp_csi)) {
    return ra_nr_table_idx_3;
  }

  // - the UE is configured with MCS-C-RNTI, and
  // - the PUSCH is scheduled by a PDCCH with
  // - CRC scrambled by MCS-C-RNTI,
  //  if (mcs_c_rnti && dci_format != isrran_dci_format_nr_rar && rnti_type == isrran_rnti_type_mcs_c) {
  //    return ra_nr_table_idx_3;
  //  }

  // - mcs-Table in configuredGrantConfig is set to 'qam256',
  //   - if PUSCH is scheduled by a PDCCH with CRC scrambled by CS-RNTI or
  //   - if PUSCH is transmitted with configured grant
  //  if (configured_grant_table == isrran_mcs_table_256qam &&
  //      (rnti_type == isrran_rnti_type_cs || dci_format == isrran_dci_format_nr_cg)) {
  //    return ra_nr_table_idx_2;
  //  }

  // - mcs-Table in configuredGrantConfig is set to 'qam64LowSE'
  //   - if PUSCH is scheduled by a PDCCH with CRC scrambled by CS-RNTI or
  //   - if PUSCH is transmitted with configured grant,
  //  if (configured_grant_table == isrran_mcs_table_qam64LowSE &&
  //      (rnti_type == isrran_rnti_type_cs || dci_format == isrran_dci_format_nr_cg)) {
  //    return ra_nr_table_idx_3;
  //  }

  return ra_nr_table_idx_1;
}

static ra_nr_table_idx_t ra_nr_select_table_pdsch(isrran_mcs_table_t         mcs_table,
                                                  isrran_dci_format_nr_t     dci_format,
                                                  isrran_search_space_type_t search_space_type,
                                                  isrran_rnti_type_t         rnti_type)
{
  // - the higher layer parameter mcs-Table given by PDSCH-Config is set to 'qam256', and
  // - the PDSCH is scheduled by a PDCCH with DCI format 1_1 with
  // - CRC scrambled by C-RNTI
  if (mcs_table == isrran_mcs_table_256qam && dci_format == isrran_dci_format_nr_1_1 &&
      rnti_type == isrran_rnti_type_c) {
    return ra_nr_table_idx_2;
  }

  // the UE is not configured with MCS-C-RNTI,
  // the higher layer parameter mcs-Table given by PDSCH-Config is set to 'qam64LowSE', and
  // the PDSCH is scheduled by a PDCCH in a UE-specific search space with
  // CRC scrambled by C - RNTI
  if (mcs_table == isrran_mcs_table_qam64LowSE && search_space_type == isrran_search_space_type_ue &&
      rnti_type == isrran_rnti_type_c) {
    return ra_nr_table_idx_3;
  }

  // - the UE is not configured with the higher layer parameter mcs-Table given by SPS-Config,
  // - the higher layer parameter mcs-Table given by PDSCH-Config is set to 'qam256',
  //   - if the PDSCH is scheduled by a PDCCH with DCI format 1_1 with CRC scrambled by CS-RNTI or
  //   - if the PDSCH is scheduled without corresponding PDCCH transmission using SPS-Config,
  //  if (!sps_config_mcs_table_present && mcs_table == isrran_mcs_table_256qam &&
  //      ((dci_format == isrran_dci_format_nr_1_1 && rnti_type == isrran_rnti_type_cs) || (!is_pdcch_sps))) {
  //    return ra_nr_table_idx_2;
  //  }

  // - the UE is configured with the higher layer parameter mcs-Table given by SPS-Config set to 'qam64LowSE'
  //   - if the PDSCH is scheduled by a PDCCH with CRC scrambled by CS-RNTI or
  //   - if the PDSCH is scheduled without corresponding PDCCH transmission using SPS-Config,
  //  if (sps_config_mcs_table_present && sps_config_mcs_table == isrran_mcs_table_qam64LowSE &&
  //      (rnti_type == isrran_rnti_type_cs || is_pdcch_sps)) {
  //    return ra_nr_table_idx_3;
  //  }

  // else
  return ra_nr_table_idx_1;
}

static ra_nr_table_idx_t ra_nr_select_table(isrran_mcs_table_t         mcs_table,
                                            isrran_dci_format_nr_t     dci_format,
                                            isrran_search_space_type_t search_space_type,
                                            isrran_rnti_type_t         rnti_type)
{
  // Check if it is a PUSCH transmission
  if (dci_format == isrran_dci_format_nr_0_0 || dci_format == isrran_dci_format_nr_0_1 ||
      dci_format == isrran_dci_format_nr_rar || dci_format == isrran_dci_format_nr_cg) {
    return ra_nr_select_table_pusch_noprecoding(mcs_table, dci_format, search_space_type, rnti_type);
  }

  return ra_nr_select_table_pdsch(mcs_table, dci_format, search_space_type, rnti_type);
}

static int ra_nr_dmrs_power_offset(isrran_sch_grant_nr_t* grant)
{
  if (grant == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Defined by TS 38.214 V15.10.0:
  // - Table 4.1-1: The ratio of PDSCH EPRE to DM-RS EPRE
  // - Table 6.2.2-1: The ratio of PUSCH EPRE to DM-RS EPRE
  float ratio_dB[3] = {0, -3, -4.77};

  if (grant->nof_dmrs_cdm_groups_without_data < 1 || grant->nof_dmrs_cdm_groups_without_data > 3) {
    ERROR("Invalid number of DMRS CDM groups without data (%d)", grant->nof_dmrs_cdm_groups_without_data);
    return ISRRAN_ERROR;
  }

  grant->beta_dmrs = isrran_convert_dB_to_amplitude(-ratio_dB[grant->nof_dmrs_cdm_groups_without_data - 1]);

  return ISRRAN_SUCCESS;
}

double isrran_ra_nr_R_from_mcs(isrran_mcs_table_t         mcs_table,
                               isrran_dci_format_nr_t     dci_format,
                               isrran_search_space_type_t search_space_type,
                               isrran_rnti_type_t         rnti_type,
                               uint32_t                   mcs_idx)
{
  ra_nr_table_idx_t table = ra_nr_select_table(mcs_table, dci_format, search_space_type, rnti_type);

  switch (table) {
    case ra_nr_table_idx_1:
      return isrran_ra_nr_R_from_mcs_table1(mcs_idx) / 1024.0;
    case ra_nr_table_idx_2:
      return isrran_ra_nr_R_from_mcs_table2(mcs_idx) / 1024.0;
    case ra_nr_table_idx_3:
      return isrran_ra_nr_R_from_mcs_table3(mcs_idx) / 1024.0;
    default:
      ERROR("Invalid table %d", table);
  }

  return NAN;
}

isrran_mod_t isrran_ra_nr_mod_from_mcs(isrran_mcs_table_t         mcs_table,
                                       isrran_dci_format_nr_t     dci_format,
                                       isrran_search_space_type_t search_space_type,
                                       isrran_rnti_type_t         rnti_type,
                                       uint32_t                   mcs_idx)
{
  ra_nr_table_idx_t table = ra_nr_select_table(mcs_table, dci_format, search_space_type, rnti_type);

  switch (table) {
    case ra_nr_table_idx_1:
      return isrran_ra_nr_modulation_from_mcs_table1(mcs_idx);
    case ra_nr_table_idx_2:
      return isrran_ra_nr_modulation_from_mcs_table2(mcs_idx);
    case ra_nr_table_idx_3:
      return isrran_ra_nr_modulation_from_mcs_table3(mcs_idx);
    default:
      ERROR("Invalid table %d", table);
  }

  return ISRRAN_MOD_NITEMS;
}

int isrran_ra_dl_nr_slot_nof_re(const isrran_sch_cfg_nr_t* pdsch_cfg, const isrran_sch_grant_nr_t* grant)
{
  // the number of symbols of the PDSCH allocation within the slot
  int n_sh_symb = grant->L;

  // the number of REs for DM-RS per PRB in the scheduled duration
  int n_prb_dmrs = isrran_dmrs_sch_get_N_prb(&pdsch_cfg->dmrs, grant);
  if (n_prb_dmrs < ISRRAN_SUCCESS) {
    ERROR("Invalid number of DMRS RE");
    return ISRRAN_ERROR;
  }

  // the overhead configured by higher layer parameter xOverhead in PDSCH-ServingCellConfig
  uint32_t n_prb_oh = 0;
  switch (pdsch_cfg->sch_cfg.xoverhead) {
    case isrran_xoverhead_0:
      n_prb_oh = 0;
      break;
    case isrran_xoverhead_6:
      n_prb_oh = 6;
      break;
    case isrran_xoverhead_12:
      n_prb_oh = 12;
      break;
    case isrran_xoverhead_18:
      n_prb_oh = 18;
      break;
  }

  // Compute total number of n_re used for PDSCH
  uint32_t n_re_prime = ISRRAN_NRE * n_sh_symb - n_prb_dmrs - n_prb_oh;

  uint32_t n_prb = 0;
  for (uint32_t i = 0; i < ISRRAN_MAX_PRB_NR; i++) {
    n_prb += (uint32_t)grant->prb_idx[i];
  }

  // Return the number of resource elements for PDSCH
  return ISRRAN_MIN(ISRRAN_MAX_NRE_NR, n_re_prime) * n_prb;
}

#define POW2(N) (1U << (N))

static uint32_t ra_nr_tbs_from_n_info3(uint32_t n_info)
{
  // quantized intermediate number of information bits
  uint32_t n            = (uint32_t)ISRRAN_MAX(3.0, floor(log2(n_info)) - 6.0);
  uint32_t n_info_prime = ISRRAN_MAX(ra_nr_tbs_table[0], POW2(n) * ISRRAN_FLOOR(n_info, POW2(n)));

  // use Table 5.1.3.2-1 find the closest TBS that is not less than n_info_prime
  for (uint32_t i = 0; i < RA_NR_TBS_SIZE_TABLE; i++) {
    if (n_info_prime <= ra_nr_tbs_table[i]) {
      return ra_nr_tbs_table[i];
    }
  }

  return ra_nr_tbs_table[RA_NR_TBS_SIZE_TABLE - 1];
}

static uint32_t ra_nr_tbs_from_n_info4(uint32_t n_info, double R)
{
  // quantized intermediate number of information bits
  uint32_t n            = (uint32_t)(floor(log2(n_info - 24.0)) - 5.0);
  uint32_t n_info_prime = ISRRAN_MAX(3840, POW2(n) * ISRRAN_ROUND(n_info - 24.0, POW2(n)));

  if (R <= 0.25) {
    uint32_t C = ISRRAN_CEIL(n_info_prime + 24U, 3816U);
    return 8U * C * ISRRAN_CEIL(n_info_prime + 24U, 8U * C) - 24U;
  }

  if (n_info_prime > 8424) {
    uint32_t C = ISRRAN_CEIL(n_info_prime + 24U, 8424U);
    return 8U * C * ISRRAN_CEIL(n_info_prime + 24U, 8U * C) - 24U;
  }

  return 8U * ISRRAN_CEIL(n_info_prime + 24U, 8U) - 24U;
}

/**
 * @brief Implements TS 38.214 V15.10.0 Table 5.1.3.2-2 Scaling factor of N_info for P-RNTI and RA-RNTI
 * @param tb_scaling_field provided by the grant
 * @return It returns the value if the field is in range, otherwise it returns NAN
 */
static double ra_nr_get_scaling(uint32_t tb_scaling_field)
{
  static const double tb_scaling[4] = {1.0, 0.5, 0.25, NAN};

  if (tb_scaling_field < 4) {
    return tb_scaling[tb_scaling_field];
  }

  return NAN;
}

uint32_t isrran_ra_nr_tbs(uint32_t N_re, double S, double R, uint32_t Qm, uint32_t nof_layers)
{
  if (!isnormal(S)) {
    S = 1.0;
  }

  if (nof_layers == 0) {
    ERROR("Incorrect number of layers (%d). Setting to 1.", nof_layers);
    nof_layers = 1;
  }

  // 2) Intermediate number of information bits (N info ) is obtained by N inf o = N RE · R · Q m · υ .
  uint32_t n_info = (uint32_t)(N_re * S * R * Qm * nof_layers);

  // 3) When n_info ≤ 3824
  if (n_info <= 3824) {
    return ra_nr_tbs_from_n_info3(n_info);
  }
  // 4) When n_info > 3824
  return ra_nr_tbs_from_n_info4(n_info, R);
}

static int ra_nr_assert_csi_rs_dmrs_collision(const isrran_sch_cfg_nr_t* pdsch_cfg)
{
  // Generate DMRS pattern
  isrran_re_pattern_t dmrs_re_pattern = {};
  if (isrran_dmrs_sch_rvd_re_pattern(&pdsch_cfg->dmrs, &pdsch_cfg->grant, &dmrs_re_pattern) < ISRRAN_SUCCESS) {
    ERROR("Error computing DMRS pattern");
    return ISRRAN_ERROR;
  }

  // Check for collision
  if (isrran_re_pattern_check_collision(&pdsch_cfg->rvd_re, &dmrs_re_pattern) < ISRRAN_SUCCESS) {
    // Create reserved info string
    char str_rvd[512] = {};
    isrran_re_pattern_list_info(&pdsch_cfg->rvd_re, str_rvd, (uint32_t)sizeof(str_rvd));

    // Create DMRS info string
    char str_dmrs[512] = {};
    isrran_re_pattern_info(&dmrs_re_pattern, str_dmrs, (uint32_t)sizeof(str_dmrs));

    ERROR("Error. The UE is not expected to receive CSI-RS (%s) and DM-RS (%s) on the same resource elements.",
          str_rvd,
          str_dmrs);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

uint32_t ra_nr_nof_crc_bits(uint32_t tbs, double R)
{
  isrran_cbsegm_t    cbsegm = {};
  isrran_basegraph_t bg     = isrran_sch_nr_select_basegraph(tbs, R);

  if (bg == BG1) {
    if (isrran_cbsegm_ldpc_bg1(&cbsegm, tbs) != ISRRAN_SUCCESS) {
      // This should never fail
      ERROR("Error: calculating LDPC BG1 code block segmentation for tbs=%d", tbs);
      return 0;
    }
  } else {
    if (isrran_cbsegm_ldpc_bg2(&cbsegm, tbs) != ISRRAN_SUCCESS) {
      // This should never fail
      ERROR("Error: calculating LDPC BG1 code block segmentation for tbs=%d", tbs);
      return 0;
    }
  }

  return cbsegm.C * cbsegm.L_cb + cbsegm.L_tb;
}

int isrran_ra_nr_fill_tb(const isrran_sch_cfg_nr_t*   pdsch_cfg,
                         const isrran_sch_grant_nr_t* grant,
                         uint32_t                     mcs_idx,
                         isrran_sch_tb_t*             tb)
{
  // Get target Rate
  double R = isrran_ra_nr_R_from_mcs(
      pdsch_cfg->sch_cfg.mcs_table, grant->dci_format, grant->dci_search_space, grant->rnti_type, mcs_idx);
  if (!isnormal(R)) {
    return ISRRAN_ERROR;
  }

  // Get modulation
  isrran_mod_t m = isrran_ra_nr_mod_from_mcs(
      pdsch_cfg->sch_cfg.mcs_table, grant->dci_format, grant->dci_search_space, grant->rnti_type, mcs_idx);
  if (m >= ISRRAN_MOD_NITEMS) {
    return ISRRAN_ERROR;
  }

  // Get modulation order
  uint32_t Qm = isrran_mod_bits_x_symbol(m);
  if (Qm == 0) {
    return ISRRAN_ERROR;
  }

  // For the PDSCH assigned by a
  // - PDCCH with DCI format 1_0 with
  // - CRC scrambled by P-RNTI, or RA-RNTI,
  double S = 1.0;
  if ((ISRRAN_RNTI_ISRAR(grant->rnti) || ISRRAN_RNTI_ISPA(grant->rnti)) &&
      grant->dci_format == isrran_dci_format_nr_1_0) {
    // where the scaling factor S is determined based on the TB scaling
    //  field in the DCI as in Table 5.1.3.2-2.
    S = ra_nr_get_scaling(grant->tb_scaling_field);

    // If the scaling is invalid return error
    if (!isnormal(S)) {
      return ISRRAN_ERROR;
    }
  }

  // 1) The UE shall first determine the number of REs (N RE ) within the slot.
  int N_re = isrran_ra_dl_nr_slot_nof_re(pdsch_cfg, grant);
  if (N_re <= ISRRAN_SUCCESS) {
    ERROR("Invalid number of RE (%d)", N_re);
    return ISRRAN_ERROR;
  }

  // Calculate number of layers accordingly, assumes first codeword only
  uint32_t nof_cw         = grant->nof_layers < 5 ? 1 : 2;
  uint32_t nof_layers_cw1 = grant->nof_layers / nof_cw;
  tb->N_L                 = nof_layers_cw1;

  // Check DMRS and CSI-RS collision according to TS 38.211 7.4.1.5.3 Mapping to physical resources
  // If there was a collision, the number of RE in the grant would be wrong
  if (ra_nr_assert_csi_rs_dmrs_collision(pdsch_cfg) < ISRRAN_SUCCESS) {
    ERROR("Error: CSI-RS and DMRS collision detected");
    return ISRRAN_ERROR;
  }

  // Calculate reserved RE
  uint32_t N_re_rvd = isrran_re_pattern_list_count(&pdsch_cfg->rvd_re, grant->S, grant->S + grant->L, grant->prb_idx);

  // Steps 2,3,4
  tb->mcs      = mcs_idx;
  tb->tbs      = (int)isrran_ra_nr_tbs(N_re, S, R, Qm, tb->N_L);
  tb->R        = R;
  tb->mod      = m;
  tb->nof_re   = (N_re - N_re_rvd) * grant->nof_layers;
  tb->nof_bits = tb->nof_re * Qm;
  tb->enabled  = true;

  // Calculate actual rate
  tb->R_prime = 0.0;
  if (tb->nof_re != 0) {
    tb->R_prime = (double)(tb->tbs + ra_nr_nof_crc_bits(tb->tbs, tb->R)) / (double)tb->nof_bits;
  }

  return ISRRAN_SUCCESS;
}

static int ra_dl_dmrs(const isrran_sch_hl_cfg_nr_t* hl_cfg, const isrran_dci_dl_nr_t* dci, isrran_sch_cfg_nr_t* cfg)
{
  const bool dedicated_dmrs_present =
      (cfg->grant.mapping == isrran_sch_mapping_type_A) ? hl_cfg->dmrs_typeA.present : hl_cfg->dmrs_typeB.present;

  if (dci->ctx.format == isrran_dci_format_nr_1_0 || !dedicated_dmrs_present) {
    // The reference point for k is
    // - for PDSCH transmission carrying SIB1, subcarrier 0 of the lowest-numbered common resource block in the
    //  CORESET configured by the PBCH
    //- otherwise, subcarrier 0 in common resource block 0
    if (dci->ctx.rnti_type == isrran_rnti_type_si) {
      cfg->dmrs.reference_point_k_rb = dci->ctx.coreset_start_rb;
    }

    if (cfg->grant.mapping == isrran_sch_mapping_type_A) {
      // Absent default values are defined is TS 38.331 - DMRS-DownlinkConfig
      cfg->dmrs.additional_pos         = isrran_dmrs_sch_add_pos_2;
      cfg->dmrs.type                   = isrran_dmrs_sch_type_1;
      cfg->dmrs.length                 = isrran_dmrs_sch_len_1;
      cfg->dmrs.scrambling_id0_present = false;
      cfg->dmrs.scrambling_id1_present = false;
    } else {
      ERROR("Unsupported configuration");
      return ISRRAN_ERROR;
    }
  } else {
    // Load DMRS duration
    if (isrran_ra_dl_nr_nof_front_load_symbols(hl_cfg, dci, &cfg->dmrs.length) < ISRRAN_SUCCESS) {
      ERROR("Loading number of front-load symbols");
      return ISRRAN_ERROR;
    }

    // DMRS Type
    cfg->dmrs.type = hl_cfg->dmrs_type;

    // Other DMRS configuration
    if (cfg->grant.mapping == isrran_sch_mapping_type_A) {
      cfg->dmrs.additional_pos         = hl_cfg->dmrs_typeA.additional_pos;
      cfg->dmrs.scrambling_id0_present = false;
      cfg->dmrs.scrambling_id1_present = false;
    } else {
      cfg->dmrs.additional_pos         = hl_cfg->dmrs_typeB.additional_pos;
      cfg->dmrs.scrambling_id0_present = false;
      cfg->dmrs.scrambling_id1_present = false;
    }
  }

  // Set number of DMRS CDM groups without data
  int n = isrran_ra_dl_nr_nof_dmrs_cdm_groups_without_data(hl_cfg, dci, cfg->grant.L);
  if (n < ISRRAN_SUCCESS) {
    ERROR("Error loading number of DMRS CDM groups");
    return ISRRAN_ERROR;
  }
  cfg->grant.nof_dmrs_cdm_groups_without_data = (uint32_t)n;

  // Set DMRS power offset Table 6.2.2-1: The ratio of PUSCH EPRE to DM-RS EPRE
  if (ra_nr_dmrs_power_offset(&cfg->grant) < ISRRAN_SUCCESS) {
    ERROR("Error setting DMRS power offset");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int ra_dl_resource_mapping(const isrran_carrier_nr_t*    carrier,
                                  const isrran_slot_cfg_t*      slot,
                                  const isrran_sch_hl_cfg_nr_t* pdsch_hl_cfg,
                                  isrran_sch_cfg_nr_t*          pdsch_cfg)
{
  // SS/PBCH block transmission resources not available for PDSCH
  // ... Not implemented

  // 5.1.4.1 PDSCH resource mapping with RB symbol level granularity
  // rateMatchPatternToAddModList ... Not implemented

  // 5.1.4.2 PDSCH resource mapping with RE level granularity
  // RateMatchingPatternLTE-CRS ... Not implemented

  // Append periodic ZP-CSI-RS
  for (uint32_t i = 0; i < pdsch_hl_cfg->p_zp_csi_rs_set.count; i++) {
    // Select resource
    const isrran_csi_rs_zp_resource_t* resource = &pdsch_hl_cfg->p_zp_csi_rs_set.data[i];

    // Check if the periodic ZP-CSI is transmitted
    if (isrran_csi_rs_send(&resource->periodicity, slot)) {
      INFO("Tx/Rx ZP-CSI-RS @slot=%d\n", slot->idx);
      if (isrran_csi_rs_append_resource_to_pattern(carrier, &resource->resource_mapping, &pdsch_cfg->rvd_re)) {
        ERROR("Error appending ZP-CSI-RS as RE pattern");
        return ISRRAN_ERROR;
      }
    }
  }

  // Append semi-persistent ZP-CSI-RS here
  // ... not implemented

  // Append aperiodic ZP-CSI-RS here
  // ... not implemented

  // Append periodic NZP-CSI-RS according to TS 38.211 clause 7.3.1.5 Mapping to virtual resource blocks
  // Only aplicable if CRC is scrambled by C-RNTI, MCS-C-RNTI, CS-RNTI, or PDSCH with SPS
  bool nzp_rvd_valid = pdsch_cfg->grant.rnti_type == isrran_rnti_type_c ||
                       pdsch_cfg->grant.rnti_type == isrran_rnti_type_mcs_c ||
                       pdsch_cfg->grant.rnti_type == isrran_rnti_type_cs;
  for (uint32_t set_id = 0; set_id < ISRRAN_PHCH_CFG_MAX_NOF_CSI_RS_SETS && nzp_rvd_valid; set_id++) {
    for (uint32_t res_id = 0; res_id < pdsch_hl_cfg->nzp_csi_rs_sets[set_id].count; res_id++) {
      // Select resource
      const isrran_csi_rs_nzp_resource_t* resource = &pdsch_hl_cfg->nzp_csi_rs_sets[set_id].data[res_id];

      // Check if the periodic ZP-CSI is transmitted
      if (isrran_csi_rs_send(&resource->periodicity, slot)) {
        INFO("Tx/Rx NZP-CSI-RS set_id=%d; res=%d; @slot=%d\n", set_id, res_id, slot->idx);
        if (isrran_csi_rs_append_resource_to_pattern(carrier, &resource->resource_mapping, &pdsch_cfg->rvd_re)) {
          ERROR("Error appending ZP-CSI-RS as RE pattern");
          return ISRRAN_ERROR;
        }
      }
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_ra_dl_dci_to_grant_nr(const isrran_carrier_nr_t*    carrier,
                                 const isrran_slot_cfg_t*      slot,
                                 const isrran_sch_hl_cfg_nr_t* pdsch_hl_cfg,
                                 const isrran_dci_dl_nr_t*     dci_dl,
                                 isrran_sch_cfg_nr_t*          pdsch_cfg,
                                 isrran_sch_grant_nr_t*        pdsch_grant)
{
  // 5.2.1.1 Resource allocation in time domain
  if (isrran_ra_dl_nr_time(pdsch_hl_cfg,
                           dci_dl->ctx.rnti_type,
                           dci_dl->ctx.ss_type,
                           dci_dl->ctx.coreset_id,
                           dci_dl->time_domain_assigment,
                           pdsch_grant) < ISRRAN_SUCCESS) {
    ERROR("Error computing time domain resource allocation");
    return ISRRAN_ERROR;
  }

  // 5.1.2.2 Resource allocation in frequency domain
  if (isrran_ra_dl_nr_freq(carrier, pdsch_hl_cfg, dci_dl, pdsch_grant) < ISRRAN_SUCCESS) {
    ERROR("Error computing frequency domain resource allocation");
    return ISRRAN_ERROR;
  }

  // 5.1.2.3 Physical resource block (PRB) bundling
  // ...

  pdsch_grant->nof_layers      = 1;
  pdsch_grant->dci_format      = dci_dl->ctx.format;
  pdsch_grant->rnti            = dci_dl->ctx.rnti;
  pdsch_grant->rnti_type       = dci_dl->ctx.rnti_type;
  pdsch_grant->tb[0].rv        = dci_dl->rv;
  pdsch_grant->tb[0].mcs       = dci_dl->mcs;
  pdsch_grant->tb[0].ndi       = dci_dl->ndi;
  pdsch_cfg->sch_cfg.mcs_table = pdsch_hl_cfg->mcs_table;

  // 5.1.4 PDSCH resource mapping
  if (ra_dl_resource_mapping(carrier, slot, pdsch_hl_cfg, pdsch_cfg) < ISRRAN_SUCCESS) {
    ERROR("Error in resource mapping");
    return ISRRAN_ERROR;
  }

  // 5.1.6.2 DM-RS reception procedure
  if (ra_dl_dmrs(pdsch_hl_cfg, dci_dl, pdsch_cfg) < ISRRAN_SUCCESS) {
    ERROR("Error selecting DMRS configuration");
    return ISRRAN_ERROR;
  }

  // 5.1.3 Modulation order, target code rate, redundancy version and transport block size determination
  if (isrran_ra_nr_fill_tb(pdsch_cfg, pdsch_grant, dci_dl->mcs, &pdsch_grant->tb[0]) < ISRRAN_SUCCESS) {
    ERROR("Error filing tb");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int
ra_ul_dmrs(const isrran_sch_hl_cfg_nr_t* pusch_hl_cfg, const isrran_dci_ul_nr_t* dci, isrran_sch_cfg_nr_t* cfg)
{
  const bool dedicated_dmrs_present = (cfg->grant.mapping == isrran_sch_mapping_type_A)
                                          ? pusch_hl_cfg->dmrs_typeA.present
                                          : pusch_hl_cfg->dmrs_typeB.present;

  if (dci->ctx.format == isrran_dci_format_nr_0_0 || dci->ctx.format == isrran_dci_format_nr_rar ||
      !dedicated_dmrs_present) {
    if (cfg->grant.mapping == isrran_sch_mapping_type_A) {
      // Absent default values are defined is TS 38.331 - DMRS-DownlinkConfig
      cfg->dmrs.additional_pos         = isrran_dmrs_sch_add_pos_2;
      cfg->dmrs.type                   = isrran_dmrs_sch_type_1;
      cfg->dmrs.length                 = isrran_dmrs_sch_len_1;
      cfg->dmrs.scrambling_id0_present = false;
      cfg->dmrs.scrambling_id1_present = false;
    } else {
      ERROR("Unsupported configuration");
      return ISRRAN_ERROR;
    }
  } else {
    // DMRS duration
    if (isrran_ra_ul_nr_nof_front_load_symbols(pusch_hl_cfg, dci, &cfg->dmrs.length) < ISRRAN_SUCCESS) {
      ERROR("Loading number of front-load symbols");
      return ISRRAN_ERROR;
    }

    // DMRS type
    cfg->dmrs.type = pusch_hl_cfg->dmrs_type;

    if (cfg->grant.mapping == isrran_sch_mapping_type_A) {
      cfg->dmrs.additional_pos         = pusch_hl_cfg->dmrs_typeA.additional_pos;
      cfg->dmrs.scrambling_id0_present = false;
      cfg->dmrs.scrambling_id1_present = false;
    } else {
      cfg->dmrs.additional_pos         = pusch_hl_cfg->dmrs_typeB.additional_pos;
      cfg->dmrs.scrambling_id0_present = false;
      cfg->dmrs.scrambling_id1_present = false;
    }
  }

  // Set number of DMRS CDM groups without data
  int n = isrran_ra_ul_nr_nof_dmrs_cdm_groups_without_data(pusch_hl_cfg, dci, cfg->grant.L);
  if (n < ISRRAN_SUCCESS) {
    ERROR("Error getting number of DMRS CDM groups without data");
    return ISRRAN_ERROR;
  }
  cfg->grant.nof_dmrs_cdm_groups_without_data = (uint32_t)n;

  // Set DMRS power offset Table 6.2.2-1: The ratio of PUSCH EPRE to DM-RS EPRE
  if (ra_nr_dmrs_power_offset(&cfg->grant) < ISRRAN_SUCCESS) {
    ERROR("Error setting DMRS power offset");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_ra_ul_dci_to_grant_nr(const isrran_carrier_nr_t*    carrier,
                                 const isrran_slot_cfg_t*      slot_cfg,
                                 const isrran_sch_hl_cfg_nr_t* pusch_hl_cfg,
                                 const isrran_dci_ul_nr_t*     dci_ul,
                                 isrran_sch_cfg_nr_t*          pusch_cfg,
                                 isrran_sch_grant_nr_t*        pusch_grant)
{
  // 5.2.1.1 Resource allocation in time domain
  if (isrran_ra_ul_nr_time(pusch_hl_cfg,
                           dci_ul->ctx.rnti_type,
                           dci_ul->ctx.ss_type,
                           dci_ul->ctx.coreset_id,
                           dci_ul->time_domain_assigment,
                           pusch_grant) < ISRRAN_SUCCESS) {
    ERROR("Error computing time domain resource allocation");
    return ISRRAN_ERROR;
  }

  // 5.1.2.2 Resource allocation in frequency domain
  if (isrran_ra_ul_nr_freq(carrier, pusch_hl_cfg, dci_ul, pusch_grant) < ISRRAN_SUCCESS) {
    ERROR("Error computing frequency domain resource allocation");
    return ISRRAN_ERROR;
  }

  // 5.1.2.3 Physical resource block (PRB) bundling
  // ...

  pusch_grant->nof_layers      = 1;
  pusch_grant->dci_format      = dci_ul->ctx.format;
  pusch_grant->rnti            = dci_ul->ctx.rnti;
  pusch_grant->rnti_type       = dci_ul->ctx.rnti_type;
  pusch_grant->tb[0].rv        = dci_ul->rv;
  pusch_grant->tb[0].mcs       = dci_ul->mcs;
  pusch_grant->tb[0].ndi       = dci_ul->ndi;
  pusch_cfg->sch_cfg.mcs_table = pusch_hl_cfg->mcs_table;

  // 5.1.6.2 DM-RS reception procedure
  if (ra_ul_dmrs(pusch_hl_cfg, dci_ul, pusch_cfg) < ISRRAN_SUCCESS) {
    ERROR("Error selecting DMRS configuration");
    return ISRRAN_ERROR;
  }

  // 5.1.3 Modulation order, target code rate, redundancy version and transport block size determination
  if (isrran_ra_nr_fill_tb(pusch_cfg, pusch_grant, dci_ul->mcs, &pusch_grant->tb[0]) < ISRRAN_SUCCESS) {
    ERROR("Error filing tb");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

/*
 * Implements clauses related to HARQ-ACK beta offset selection from the section `9.3 UCI reporting in physical uplink
 * shared channel`
 */
static float ra_ul_beta_offset_ack_semistatic(const isrran_beta_offsets_t* beta_offsets,
                                              const isrran_uci_cfg_nr_t*   uci_cfg)
{
  if (isnormal(beta_offsets->fix_ack)) {
    return beta_offsets->fix_ack;
  }

  // Select Beta Offset index from the number of HARQ-ACK bits
  uint32_t beta_offset_index = beta_offsets->ack_index1;
  if (uci_cfg->ack.count > 11) {
    beta_offset_index = beta_offsets->ack_index3;
  } else if (uci_cfg->ack.count > 2) {
    beta_offset_index = beta_offsets->ack_index2;
  }

  // Protect table boundary
  if (beta_offset_index >= RA_NR_BETA_OFFSET_HARQACK_SIZE) {
    ERROR("Beta offset index for HARQ-ACK (%d) for O_ack=%d exceeds table size (%d)",
          beta_offset_index,
          uci_cfg->ack.count,
          RA_NR_BETA_OFFSET_HARQACK_SIZE);
    return NAN;
  }

  // Select beta offset from Table 9.3-1
  return ra_nr_beta_offset_ack_table[beta_offset_index];
}

/*
 * Implements clauses related to HARQ-ACK beta offset selection from the section `9.3 UCI reporting in physical uplink
 * shared channel`
 */
static float ra_ul_beta_offset_csi_semistatic(const isrran_beta_offsets_t* beta_offsets,
                                              const isrran_uci_cfg_nr_t*   uci_cfg,
                                              bool                         part2)
{
  float fix_beta_offset = part2 ? beta_offsets->fix_csi2 : beta_offsets->fix_csi1;
  if (isnormal(fix_beta_offset)) {
    return fix_beta_offset;
  }

  int O_csi1 = isrran_csi_part1_nof_bits(uci_cfg->csi, uci_cfg->nof_csi);
  int O_csi2 = isrran_csi_part2_nof_bits(uci_cfg->csi, uci_cfg->nof_csi);
  if (O_csi1 < ISRRAN_SUCCESS || O_csi2 < ISRRAN_SUCCESS) {
    ERROR("Invalid O_csi1 (%d) or O_csi2(%d)", O_csi1, O_csi2);
    return NAN;
  }

  // Calculate number of CSI bits; CSI part 2 is not supported.
  uint32_t O_csi = (uint32_t)(part2 ? O_csi2 : O_csi1);

  // Select Beta Offset index from the number of HARQ-ACK bits
  uint32_t beta_offset_index = part2 ? beta_offsets->csi2_index1 : beta_offsets->csi1_index1;
  if (O_csi > 11) {
    beta_offset_index = part2 ? beta_offsets->csi2_index2 : beta_offsets->csi1_index2;
  }

  // Protect table boundary
  if (beta_offset_index >= RA_NR_BETA_OFFSET_CSI_SIZE) {
    ERROR("Beta offset index for CSI (%d) for O_csi=%d exceeds table size (%d)",
          beta_offset_index,
          O_csi,
          RA_NR_BETA_OFFSET_CSI_SIZE);
    return NAN;
  }

  // Select beta offset from Table 9.3-1
  return ra_nr_beta_offset_csi_table[beta_offset_index];
}

int isrran_ra_ul_set_grant_uci_nr(const isrran_carrier_nr_t*    carrier,
                                  const isrran_sch_hl_cfg_nr_t* pusch_hl_cfg,
                                  const isrran_uci_cfg_nr_t*    uci_cfg,
                                  isrran_sch_cfg_nr_t*          pusch_cfg)
{
  if (pusch_cfg->grant.nof_prb == 0) {
    ERROR("Invalid number of PRB (%d)", pusch_cfg->grant.nof_prb);
    return ISRRAN_ERROR;
  }

  // Initially, copy all fields
  pusch_cfg->uci = *uci_cfg;

  // Reset UCI PUSCH configuration
  ISRRAN_MEM_ZERO(&pusch_cfg->uci.pusch, isrran_uci_nr_pusch_cfg_t, 1);

  // Get DMRS symbol indexes
  uint32_t nof_dmrs_l                          = 0;
  uint32_t dmrs_l[ISRRAN_DMRS_SCH_MAX_SYMBOLS] = {};
  int      n = isrran_dmrs_sch_get_symbols_idx(&pusch_cfg->dmrs, &pusch_cfg->grant, dmrs_l);
  if (n < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }
  nof_dmrs_l = (uint32_t)n;

  // Find OFDM symbol index of the first OFDM symbol after the first set of consecutive OFDM symbol(s) carrying DMRS
  // Starts at first OFDM symbol carrying DMRS
  for (uint32_t l = dmrs_l[0], dmrs_l_idx = 0; l < pusch_cfg->grant.S + pusch_cfg->grant.L; l++) {
    // Check if it is not carrying DMRS...
    if (l != dmrs_l[dmrs_l_idx]) {
      // Set value and stop iterating
      pusch_cfg->uci.pusch.l0 = l;
      break;
    }

    // Move to the next DMRS OFDM symbol index
    if (dmrs_l_idx < nof_dmrs_l) {
      dmrs_l_idx++;
    }
  }

  // Find OFDM symbol index of the first OFDM symbol that does not carry DMRS
  // Starts at first OFDM symbol of the PUSCH transmission
  for (uint32_t l = pusch_cfg->grant.S, dmrs_l_idx = 0; l < pusch_cfg->grant.S + pusch_cfg->grant.L; l++) {
    // Check if it is not carrying DMRS...
    if (l != dmrs_l[dmrs_l_idx]) {
      pusch_cfg->uci.pusch.l1 = l;
      break;
    }

    // Move to the next DMRS OFDM symbol index
    if (dmrs_l_idx < nof_dmrs_l) {
      dmrs_l_idx++;
    }
  }

  // Number of DMRS per PRB
  uint32_t n_sc_dmrs = ISRRAN_DMRS_SCH_SC(pusch_cfg->grant.nof_dmrs_cdm_groups_without_data, pusch_cfg->dmrs.type);

  // Set UCI RE number of candidates per OFDM symbol according to TS 38.312 6.3.2.4.2.1
  for (uint32_t l = 0, dmrs_l_idx = 0; l < ISRRAN_NSYMB_PER_SLOT_NR; l++) {
    // Skip if OFDM symbol is outside of the PUSCH transmission
    if (l < pusch_cfg->grant.S || l >= (pusch_cfg->grant.S + pusch_cfg->grant.L)) {
      pusch_cfg->uci.pusch.M_pusch_sc[l] = 0;
      pusch_cfg->uci.pusch.M_uci_sc[l]   = 0;
      continue;
    }

    // OFDM symbol carries DMRS
    if (l == dmrs_l[dmrs_l_idx]) {
      // Calculate PUSCH RE candidates
      pusch_cfg->uci.pusch.M_pusch_sc[l] = pusch_cfg->grant.nof_prb * (ISRRAN_NRE - n_sc_dmrs);

      // The Number of RE candidates for UCI are 0
      pusch_cfg->uci.pusch.M_uci_sc[l] = 0;

      // Advance DMRS symbol index
      dmrs_l_idx++;

      // Skip to next symbol
      continue;
    }

    // Number of RE for Phase Tracking Reference Signals (PT-RS)
    uint32_t M_ptrs_sc = 0; // Not implemented yet

    // Number of RE given by the grant
    pusch_cfg->uci.pusch.M_pusch_sc[l] = pusch_cfg->grant.nof_prb * ISRRAN_NRE;

    // Calculate the number of UCI candidates
    pusch_cfg->uci.pusch.M_uci_sc[l] = pusch_cfg->uci.pusch.M_pusch_sc[l] - M_ptrs_sc;
  }

  // Generate SCH Transport block information
  isrran_sch_nr_tb_info_t sch_tb_info = {};
  if (isrran_sch_nr_fill_tb_info(carrier, &pusch_cfg->sch_cfg, &pusch_cfg->grant.tb[0], &sch_tb_info) <
      ISRRAN_SUCCESS) {
    ERROR("Generating TB info");
    return ISRRAN_ERROR;
  }

  // Calculate the sum of codeblock sizes
  for (uint32_t i = 0; i < sch_tb_info.C; i++) {
    // Accumulate codeblock size if mask is enabled
    pusch_cfg->uci.pusch.K_sum += (sch_tb_info.mask[i]) ? sch_tb_info.Kr : 0;
  }

  // Set other PUSCH parameters
  pusch_cfg->uci.pusch.modulation = pusch_cfg->grant.tb[0].mod;
  pusch_cfg->uci.pusch.nof_layers = pusch_cfg->grant.nof_layers;
  pusch_cfg->uci.pusch.R          = (float)pusch_cfg->grant.tb[0].R;
  pusch_cfg->uci.pusch.nof_re     = pusch_cfg->grant.tb[0].nof_re;

  // Select beta offsets
  pusch_cfg->uci.pusch.beta_harq_ack_offset = ra_ul_beta_offset_ack_semistatic(&pusch_hl_cfg->beta_offsets, uci_cfg);
  if (!isnormal(pusch_cfg->uci.pusch.beta_harq_ack_offset)) {
    return ISRRAN_ERROR;
  }

  pusch_cfg->uci.pusch.beta_csi1_offset = ra_ul_beta_offset_csi_semistatic(&pusch_hl_cfg->beta_offsets, uci_cfg, false);
  if (!isnormal(pusch_cfg->uci.pusch.beta_csi1_offset)) {
    return ISRRAN_ERROR;
  }

  pusch_cfg->uci.pusch.beta_csi2_offset = ra_ul_beta_offset_csi_semistatic(&pusch_hl_cfg->beta_offsets, uci_cfg, true);
  if (!isnormal(pusch_cfg->uci.pusch.beta_csi2_offset)) {
    return ISRRAN_ERROR;
  }

  pusch_cfg->uci.pusch.alpha = pusch_hl_cfg->scaling;
  if (!isnormal(pusch_cfg->uci.pusch.alpha)) {
    ERROR("Invalid Scaling (%f)", pusch_cfg->uci.pusch.alpha);
    return ISRRAN_ERROR;
  }

  // Calculate number of UCI encoded bits
  int Gack = 0;
  if (pusch_cfg->uci.ack.count > 2) {
    Gack = isrran_uci_nr_pusch_ack_nof_bits(&pusch_cfg->uci.pusch, pusch_cfg->uci.ack.count);
    if (Gack < ISRRAN_SUCCESS) {
      ERROR("Error calculating Qdack");
      return ISRRAN_ERROR;
    }
  }
  int Gcsi1 = isrran_uci_nr_pusch_csi1_nof_bits(&pusch_cfg->uci);
  if (Gcsi1 < ISRRAN_SUCCESS) {
    ERROR("Error calculating Qdack");
    return ISRRAN_ERROR;
  }
  int Gcsi2 = 0; // NOT supported

  // Update Number of TB encoded bits
  for (uint32_t i = 0; i < ISRRAN_MAX_TB; i++) {
    pusch_cfg->grant.tb[i].nof_bits =
        pusch_cfg->grant.tb[i].nof_re * isrran_mod_bits_x_symbol(pusch_cfg->grant.tb[i].mod) - Gack - Gcsi1 - Gcsi2;

    if (pusch_cfg->grant.tb[i].nof_bits > 0) {
      pusch_cfg->grant.tb[i].R_prime =
          (double)(pusch_cfg->grant.tb[i].tbs +
                   ra_nr_nof_crc_bits(pusch_cfg->grant.tb[i].tbs, pusch_cfg->grant.tb[i].R)) /
          (double)pusch_cfg->grant.tb[i].nof_bits;
    } else {
      pusch_cfg->grant.tb[i].R_prime = NAN;
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_ra_nr_cqi_to_mcs(uint8_t                    cqi,
                            isrran_csi_cqi_table_t     cqi_table_idx,
                            isrran_mcs_table_t         mcs_table,
                            isrran_dci_format_nr_t     dci_format,
                            isrran_search_space_type_t search_space_type,
                            isrran_rnti_type_t         rnti_type)
{
  if (cqi >= RA_NR_CQI_TABLE_SIZE) {
    ERROR("Invalid CQI (%u)", cqi);
    return -1;
  }

  ra_nr_table_idx_t mcs_table_idx = ra_nr_select_table_pdsch(mcs_table, dci_format, search_space_type, rnti_type);

  return ra_nr_cqi_to_mcs_table[cqi_table_idx][mcs_table_idx][cqi];
}

double isrran_ra_nr_cqi_to_se(uint8_t cqi, isrran_csi_cqi_table_t cqi_table_idx)
{
  if (cqi >= RA_NR_CQI_TABLE_SIZE) {
    ERROR("Invalid CQI (%u)", cqi);
    return -1;
  }

  return ra_nr_cqi_to_se_table[cqi_table_idx][cqi];
}

int isrran_ra_nr_se_to_mcs(double                     se_target,
                           isrran_mcs_table_t         mcs_table,
                           isrran_dci_format_nr_t     dci_format,
                           isrran_search_space_type_t search_space_type,
                           isrran_rnti_type_t         rnti_type)
{
  // Get MCS table index to be used
  ra_nr_table_idx_t mcs_table_idx = ra_nr_select_table_pdsch(mcs_table, dci_format, search_space_type, rnti_type);

  // Get MCS table and size based on mcs_table_idx
  const mcs_entry_t* mcs_se_table;
  size_t             mcs_table_size;
  switch (mcs_table_idx) {
    case ra_nr_table_idx_1:
      mcs_se_table   = ra_nr_table1;
      mcs_table_size = RA_NR_MCS_SIZE_TABLE1;
      break;
    case ra_nr_table_idx_2:
      mcs_se_table   = ra_nr_table2;
      mcs_table_size = RA_NR_MCS_SIZE_TABLE2;
      break;
    case ra_nr_table_idx_3:
      mcs_se_table   = ra_nr_table3;
      mcs_table_size = RA_NR_MCS_SIZE_TABLE3;
      break;
    default:
      ERROR("Invalid MCS table index (%u)", mcs_table_idx);
      return -1;
  }

  // if SE is lower than min possible value, return min MCS
  if (se_target <= mcs_se_table[0].S) {
    return 0;
  }
  // if SE is greater than max possible value, return max MCS
  else if (se_target >= mcs_se_table[mcs_table_size - 1].S) {
    return mcs_table_size - 1;
  }

  // handle monotonicity oddity between MCS 16 and 17 for MCS table 1
  if (mcs_table_idx == ra_nr_table_idx_1) {
    if (se_target == mcs_se_table[17].S) {
      return 17;
    } else if (se_target <= mcs_se_table[16].S && se_target > mcs_se_table[17].S) {
      return 16;
    }
  }

  /* In the following, we search for the greatest MCS value such that MCS(SE) <= target SE, where the target SE is the
   * value provided as an input argument. The MCS is the vector index, the content of the vector is the SE.
   * The search is performed by means of a binary-search like algorithm. At each iteration, we look for the SE in the
   * left or right half of the vector, depending on the target SE.
   * We stop when the lower-bound (lb) and upper-bound (ub) are two consecutive MCS values and we return the
   * lower-bound, which approximates the greatest MCS value such that MCS(SE) <= target SE
   * */
  size_t lb = 0;                  // lower-bound of MCS-to-SE vector where to perform binary search
  size_t ub = mcs_table_size - 1; // upper-bound of MCS-to-SE vector where to perform binary search
  while (ub > lb + 1) {
    size_t mid_point = (size_t)floor(((double)(lb + ub)) / 2);
    // break out of loop is there is an exact match
    if (mcs_se_table[mid_point].S == se_target) {
      return (int)mid_point;
    }
    // restrict the search to the left half of the vector
    else if (se_target < mcs_se_table[mid_point].S) {
      ub = mid_point;
      // handle monotonicity oddity between MCS 16 and 17 for MCS table 1
      if (mcs_table_idx == ra_nr_table_idx_1 && ub == 17) {
        ub = 16;
      }
    }
    // restrict the search to the right half of the vector
    else { /* se_target > mcs_se_table[mid_point].S ) */
      lb = mid_point;
      // handle monotonicity oddity between MCS 16 and 17 for MCS table 1
      if (mcs_table_idx == ra_nr_table_idx_1 && lb == 16) {
        lb = 17;
      }
    }
  }

  return (int)lb;
}
