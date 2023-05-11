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

#ifndef ISRRAN_CSI_NR_H
#define ISRRAN_CSI_NR_H

#include "uci_cfg_nr.h"

/**
 * @brief Processes a new NZP-CSI-RS channel measurement, it maps the given measurement into the current measurements
 * applying an exponential moving average filter
 * @param csi_resources CSI Resource configuration, links NZP-CSI-RS resources with CSI Measurements
 * @param measurements Current CSI measurements
 * @param new_measure New NZP-CSI-RS channel measurement
 * @param nzp_csi_rs_id NZP-CSI-RS resource set identifier
 * @return ISRRAN_SUCCESS if the provided information is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int
isrran_csi_new_nzp_csi_rs_measurement(const isrran_csi_hl_resource_cfg_t csi_resources[ISRRAN_CSI_MAX_NOF_RESOURCES],
                                      isrran_csi_channel_measurements_t  measurements[ISRRAN_CSI_MAX_NOF_RESOURCES],
                                      const isrran_csi_channel_measurements_t* new_measure,
                                      uint32_t                                 nzp_csi_rs_id);

/**
 * @brief Generates CSI report configuration from the higher layer configuration for a given slot
 * @param cfg Higher layer report configuration
 * @param slot_cfg Current slot configuration
 * @param[out] report_cfg Report configuration for the given slot
 * @return The number CSI reports for transmission if the provided data is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_csi_reports_generate(const isrran_csi_hl_cfg_t* cfg,
                                           const isrran_slot_cfg_t*   slot_cfg,
                                           isrran_csi_report_cfg_t    report_cfg[ISRRAN_CSI_SLOT_MAX_NOF_REPORT]);

/**
 * @brief Quantifies a given set of CSI reports from the given set of measurements
 * @param reports Set of report configuration
 * @param measurements Set of measurements to quantify
 * @param report_value Set of report values
 * @return The number CSI reports for transmission if the provided data is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int
isrran_csi_reports_quantify(const isrran_csi_report_cfg_t           reports[ISRRAN_CSI_SLOT_MAX_NOF_REPORT],
                            const isrran_csi_channel_measurements_t measurements[ISRRAN_CSI_MAX_NOF_RESOURCES],
                            isrran_csi_report_value_t               report_value[ISRRAN_CSI_SLOT_MAX_NOF_REPORT]);

/**
 * @brief Compute number of CSI bits necessary to transmit all the CSI reports for a PUCCH transmission
 * @param report_list Provides the CSI report list
 * @param nof_reports Number of CSI reports in the list
 * @return The number of bits if the provided list is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_csi_part1_nof_bits(const isrran_csi_report_cfg_t* report_list, uint32_t nof_reports);

/**
 * @brief Compute number of CSI bits necessary to transmit all the CSI part 2 reports for a PUCCH transmission
 * @param report_list Provides the CSI report list
 * @param nof_reports Number of CSI reports in the list
 * @return The number of bits if the provided list is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_csi_part2_nof_bits(const isrran_csi_report_cfg_t* report_list, uint32_t nof_reports);

/**
 * @brief Checks if the report list contains part 2 CSI report
 * @param report_list Report list
 * @param nof_reports Number of reports in the list
 * @return True if at least one report contains part 2, false otherwise
 */
ISRRAN_API bool isrran_csi_has_part2(const isrran_csi_report_cfg_t* report_list, uint32_t nof_reports);

/**
 * @brief Pack CSI part 1 bits for a PUCCH or PUSCH transmission
 * @param report_list Provides the CSI report list
 * @param nof_reports Number of CSI reports in the list
 * @param o_csi1 CSI bits
 * @param max_o_csi1 Maximum number of CSI bits
 * @return number of packed bits if provided data is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_csi_part1_pack(const isrran_csi_report_cfg_t*   report_cfg,
                                     const isrran_csi_report_value_t* report_value,
                                     uint32_t                         nof_reports,
                                     uint8_t*                         o_csi1,
                                     uint32_t                         max_o_csi1);

/**
 *@brief Unpacks CSI part 1 bits for PUCCH or PUSCH transmission
 * @param report_list Provides the CSI report list
 * @param nof_reports Number of CSI reports in the list
 * @param o_csi1 CSI bits
 * @param max_o_csi1 Maximum number of CSI bits
 * @param report_value
 * @return ISRRAN_SUCCESS if provided data is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_csi_part1_unpack(const isrran_csi_report_cfg_t* report_cfg,
                                       uint32_t                       nof_reports,
                                       uint8_t*                       o_csi1,
                                       uint32_t                       max_o_csi1,
                                       isrran_csi_report_value_t*     report_value);

/**
 * @brief Converts to string a given list of CSI reports
 * @param report_cfg Report configuration list
 * @param report_value Report value list
 * @param nof_reports Number of reports
 * @param str String pointer
 * @param str_len Maximum string length
 * @return Number of used characters
 */
ISRRAN_API uint32_t isrran_csi_str(const isrran_csi_report_cfg_t*   report_cfg,
                                   const isrran_csi_report_value_t* report_value,
                                   uint32_t                         nof_reports,
                                   char*                            str,
                                   uint32_t                         str_len);

#endif // ISRRAN_CSI_NR_H
