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

#include "isrran/common/test_common.h"
extern "C" {
#include "isrran/phy/phch/ra_nr.h"
}

void test_cqi_to_mcs()
{
  // Sample random CQI values and test the returned MCS, for different combinations of CQI and MCS tables
  // Set the parameters so that to select a given MCS table (1, 2, or 3)

  //  TEST CQI Table 1 - MCS table 1
  int mcs = isrran_ra_nr_cqi_to_mcs(/* cqi */ 3,
                                    /* cqi_table_idx */ ISRRAN_CSI_CQI_TABLE_1,
                                    /* mcs_table */ isrran_mcs_table_64qam,
                                    /* dci_format */ isrran_dci_format_nr_1_0,
                                    /* search_space_type*/ isrran_search_space_type_ue,
                                    /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(2, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(5,
                                ISRRAN_CSI_CQI_TABLE_1,
                                isrran_mcs_table_64qam,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(6, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(9,
                                ISRRAN_CSI_CQI_TABLE_1,
                                isrran_mcs_table_64qam,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(15, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(12,
                                ISRRAN_CSI_CQI_TABLE_1,
                                isrran_mcs_table_64qam,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(22, mcs);

  //  TEST CQI Table 2 - MCS table 2
  mcs = isrran_ra_nr_cqi_to_mcs(1,
                                ISRRAN_CSI_CQI_TABLE_2,
                                isrran_mcs_table_256qam,
                                isrran_dci_format_nr_1_1,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(4,
                                ISRRAN_CSI_CQI_TABLE_2,
                                isrran_mcs_table_256qam,
                                isrran_dci_format_nr_1_1,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(5, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(7,
                                ISRRAN_CSI_CQI_TABLE_2,
                                isrran_mcs_table_256qam,
                                isrran_dci_format_nr_1_1,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(11, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(11,
                                ISRRAN_CSI_CQI_TABLE_2,
                                isrran_mcs_table_256qam,
                                isrran_dci_format_nr_1_1,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(19, mcs);

  //  TEST CQI Table 3 - MCS table 3
  mcs = isrran_ra_nr_cqi_to_mcs(2,
                                ISRRAN_CSI_CQI_TABLE_3,
                                isrran_mcs_table_qam64LowSE,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(2, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(8,
                                ISRRAN_CSI_CQI_TABLE_3,
                                isrran_mcs_table_qam64LowSE,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(14, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(13,
                                ISRRAN_CSI_CQI_TABLE_3,
                                isrran_mcs_table_qam64LowSE,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(24, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(15,
                                ISRRAN_CSI_CQI_TABLE_3,
                                isrran_mcs_table_qam64LowSE,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(28, mcs);

  //  TEST CQI Table 1 - MCS table 2
  mcs = isrran_ra_nr_cqi_to_mcs(6,
                                ISRRAN_CSI_CQI_TABLE_1,
                                isrran_mcs_table_256qam,
                                isrran_dci_format_nr_1_1,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(4, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(14,
                                ISRRAN_CSI_CQI_TABLE_1,
                                isrran_mcs_table_256qam,
                                isrran_dci_format_nr_1_1,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(19, mcs);

  //  TEST CQI Table 1 - MCS table 3
  mcs = isrran_ra_nr_cqi_to_mcs(7,
                                ISRRAN_CSI_CQI_TABLE_1,
                                isrran_mcs_table_qam64LowSE,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(16, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(10,
                                ISRRAN_CSI_CQI_TABLE_1,
                                isrran_mcs_table_qam64LowSE,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(22, mcs);

  //  TEST CQI Table 2 - MCS table 1
  mcs = isrran_ra_nr_cqi_to_mcs(3,
                                ISRRAN_CSI_CQI_TABLE_2,
                                isrran_mcs_table_64qam,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(6, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(11,
                                ISRRAN_CSI_CQI_TABLE_2,
                                isrran_mcs_table_64qam,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(26, mcs);

  //  TEST CQI Table 2 - MCS table 3
  mcs = isrran_ra_nr_cqi_to_mcs(7,
                                ISRRAN_CSI_CQI_TABLE_2,
                                isrran_mcs_table_qam64LowSE,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(22, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(10,
                                ISRRAN_CSI_CQI_TABLE_2,
                                isrran_mcs_table_qam64LowSE,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(28, mcs);

  //  TEST CQI Table 3 - MCS table 1
  mcs = isrran_ra_nr_cqi_to_mcs(2,
                                ISRRAN_CSI_CQI_TABLE_3,
                                isrran_mcs_table_64qam,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(13,
                                ISRRAN_CSI_CQI_TABLE_3,
                                isrran_mcs_table_64qam,
                                isrran_dci_format_nr_1_0,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(20, mcs);

  //  TEST CQI Table 3 - MCS table 2
  mcs = isrran_ra_nr_cqi_to_mcs(5,
                                ISRRAN_CSI_CQI_TABLE_3,
                                isrran_mcs_table_256qam,
                                isrran_dci_format_nr_1_1,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(1, mcs);

  mcs = isrran_ra_nr_cqi_to_mcs(14,
                                ISRRAN_CSI_CQI_TABLE_3,
                                isrran_mcs_table_256qam,
                                isrran_dci_format_nr_1_1,
                                isrran_search_space_type_ue,
                                isrran_rnti_type_c);
  TESTASSERT_EQ(15, mcs);
}

void test_cqi_to_se()
{
  //  TEST CQI Table 1
  double se = isrran_ra_nr_cqi_to_se(2, ISRRAN_CSI_CQI_TABLE_1);
  TESTASSERT_EQ(0.2344, se);

  se = isrran_ra_nr_cqi_to_se(5, ISRRAN_CSI_CQI_TABLE_1);
  TESTASSERT_EQ(0.8770, se);

  se = isrran_ra_nr_cqi_to_se(8, ISRRAN_CSI_CQI_TABLE_1);
  TESTASSERT_EQ(1.9141, se);

  se = isrran_ra_nr_cqi_to_se(11, ISRRAN_CSI_CQI_TABLE_1);
  TESTASSERT_EQ(3.3223, se);

  se = isrran_ra_nr_cqi_to_se(13, ISRRAN_CSI_CQI_TABLE_1);
  TESTASSERT_EQ(4.5234, se);

  se = isrran_ra_nr_cqi_to_se(15, ISRRAN_CSI_CQI_TABLE_1);
  TESTASSERT_EQ(5.5547, se);

  //  TEST CQI Table 2
  se = isrran_ra_nr_cqi_to_se(1, ISRRAN_CSI_CQI_TABLE_2);
  TESTASSERT_EQ(0.1523, se);

  se = isrran_ra_nr_cqi_to_se(4, ISRRAN_CSI_CQI_TABLE_2);
  TESTASSERT_EQ(1.4766, se);

  se = isrran_ra_nr_cqi_to_se(7, ISRRAN_CSI_CQI_TABLE_2);
  TESTASSERT_EQ(2.7305, se);

  se = isrran_ra_nr_cqi_to_se(10, ISRRAN_CSI_CQI_TABLE_2);
  TESTASSERT_EQ(4.5234, se);

  se = isrran_ra_nr_cqi_to_se(12, ISRRAN_CSI_CQI_TABLE_2);
  TESTASSERT_EQ(5.5547, se);

  se = isrran_ra_nr_cqi_to_se(14, ISRRAN_CSI_CQI_TABLE_2);
  TESTASSERT_EQ(6.9141, se);

  //  TEST CQI Table 3
  se = isrran_ra_nr_cqi_to_se(1, ISRRAN_CSI_CQI_TABLE_3);
  TESTASSERT_EQ(0.0586, se);

  se = isrran_ra_nr_cqi_to_se(3, ISRRAN_CSI_CQI_TABLE_3);
  TESTASSERT_EQ(0.1523, se);

  se = isrran_ra_nr_cqi_to_se(7, ISRRAN_CSI_CQI_TABLE_3);
  TESTASSERT_EQ(0.8770, se);

  se = isrran_ra_nr_cqi_to_se(9, ISRRAN_CSI_CQI_TABLE_3);
  TESTASSERT_EQ(1.4766, se);

  se = isrran_ra_nr_cqi_to_se(12, ISRRAN_CSI_CQI_TABLE_3);
  TESTASSERT_EQ(2.7305, se);

  se = isrran_ra_nr_cqi_to_se(15, ISRRAN_CSI_CQI_TABLE_3);
  TESTASSERT_EQ(4.5234, se);
}

void test_se_to_mcs()
{
  //  MCS table 1
  int mcs = isrran_ra_nr_se_to_mcs(/* se */ .1,
                                   /* mcs_table */ isrran_mcs_table_64qam,
                                   /* dci_format */ isrran_dci_format_nr_1_0,
                                   /* search_space_type*/ isrran_search_space_type_ue,
                                   /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ .2344,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 1,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(6, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 1.0273,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(7, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 2.5,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(15, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 2.5703,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(16, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 2.57,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(16, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 2.5664,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(17, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 5.5,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(27, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 5.5574,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(28, mcs);

  mcs = isrran_ra_nr_se_to_mcs(/* se */ 6,
                               /* mcs_table */ isrran_mcs_table_64qam,
                               /* dci_format */ isrran_dci_format_nr_1_0,
                               /* search_space_type*/ isrran_search_space_type_ue,
                               /* rnti_type */ isrran_rnti_type_c);
  TESTASSERT_EQ(28, mcs);

  //  MCS table 2
  mcs = isrran_ra_nr_se_to_mcs(
      0.1, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      0.2344, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      0.24, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      1.47, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(4, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      1.4766, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(5, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      3.5, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(13, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      3.6094, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(14, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      5.2, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(19, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      5.3320, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(20, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      5.4, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(20, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      7.4063, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(27, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      7.5, isrran_mcs_table_256qam, isrran_dci_format_nr_1_1, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(27, mcs);

  //  MCS table 3
  mcs = isrran_ra_nr_se_to_mcs(
      0.05, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      0.0586, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      0.056, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(0, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      0.15, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(3, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      0.1523, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(4, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      0.49, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(8, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      0.4902, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(9, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      1.69, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(16, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      1.6953, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(17, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      4.52, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(27, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      4.5234, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(28, mcs);

  mcs = isrran_ra_nr_se_to_mcs(
      4.6, isrran_mcs_table_qam64LowSE, isrran_dci_format_nr_1_0, isrran_search_space_type_ue, isrran_rnti_type_c);
  TESTASSERT_EQ(28, mcs);
};

int main()
{
  test_cqi_to_mcs();
  test_cqi_to_se();
  test_se_to_mcs();
}