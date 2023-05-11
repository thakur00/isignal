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

#ifndef ISRRAN_RA_UL_NR_H
#define ISRRAN_RA_UL_NR_H

#include "dci_nr.h"
#include "isrran/config.h"
#include "isrran/phy/phch/phch_cfg_nr.h"
#include "isrran/phy/phch/pucch_cfg_nr.h"
#include "uci_cfg_nr.h"

/**
 * @brief Calculates the PUSCH time resource allocation and stores it in the provided PUSCH NR grant.
 *
 * @remark Defined by TS 38.214 V15.10.0 section 6.1.2.1.1 Determination of the resource allocation table to be used for
 * PUSCH
 *
 * @param cfg Flattened PUSCH configuration provided from higher layers
 * @param rnti_type Type of the RNTI of the corresponding DCI
 * @param ss_type Type of the SS for PDCCH
 * @param coreset_id CORESET identifier associated with the PDCCH transmission
 * @param m Time domain resource assignment field value m provided in DCI
 * @param[out] Provides grant pointer to fill
 * @return Returns ISRRAN_SUCCESS if the provided allocation is valid, otherwise it returns ISRRAN_ERROR code
 */
ISRRAN_API int isrran_ra_ul_nr_time(const isrran_sch_hl_cfg_nr_t*    cfg,
                                    const isrran_rnti_type_t         rnti_type,
                                    const isrran_search_space_type_t ss_type,
                                    const uint32_t                   coreset_id,
                                    const uint8_t                    m,
                                    isrran_sch_grant_nr_t*           grant);

/**
 * @brief Calculates the PUSCH time resource default A and stores it in the provided PUSCH NR grant.
 *
 * @remark Defined by TS 38.214 V15.10.0 Table 6.1.2.1.1-2: Default PUSCH time domain resource allocation A for normal
 * CP
 *
 * @param scs_cfg Sub-carrier spacing configuration for PUSCH (μ)
 * @param m Time domain resource assignment field value m of the DCI
 * @param[out] grant PUSCH grant
 * @return Returns ISRRAN_SUCCESS if the provided allocation is valid, otherwise it returns ISRRAN_ERROR code
 */
ISRRAN_API int
isrran_ra_ul_nr_pusch_time_resource_default_A(uint32_t scs_cfg, uint32_t m, isrran_sch_grant_nr_t* grant);

/**
 * @brief Calculates the number of front load symbols
 *
 * @param cfg PUSCH NR configuration by upper layers
 * @param dci Provides PUSCH NR DCI
 * @param[out] dmrs_duration
 * @return ISRRAN_SUCCESS if provided arguments are valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ra_ul_nr_nof_front_load_symbols(const isrran_sch_hl_cfg_nr_t* cfg,
                                                      const isrran_dci_ul_nr_t*     dci,
                                                      isrran_dmrs_sch_len_t*        dmrs_duration);

/**
 * @brief Calculates the number of DMRS CDM groups without data
 *
 * @remark Defined by TS 38.214 V15.10.0 6.2.2 UE DM-RS transmission procedure
 *
 * @param cfg PUSCH NR configuration by upper layers
 * @param dci Provides PUSCH NR DCI
 * @return The number of DMRS CDM groups without data if the provided data is valid, otherwise it returns ISRRAN_ERROR
 * code
 */
ISRRAN_API int isrran_ra_ul_nr_nof_dmrs_cdm_groups_without_data(const isrran_sch_hl_cfg_nr_t* cfg,
                                                                const isrran_dci_ul_nr_t*     dci,
                                                                uint32_t                      L);

/**
 * @brief Calculates the ratio of PUSCH EPRE to DM-RS EPRE
 *
 * @remark Defined by TS 38.214 V15.10.0 Table 6.2.2-1: The ratio of PUSCH EPRE to DM-RS EPRE
 *
 * @param[out] grant Provides grant pointer to fill
 * @return Returns ISRRAN_SUCCESS if the provided data is valid, otherwise it returns ISRRAN_ERROR code
 */
ISRRAN_API int isrran_ra_ul_nr_dmrs_power_offset(isrran_sch_grant_nr_t* grant);

/**
 * @brief Calculates the minimum number of PRB required for transmitting NR-PUCCH Format 2, 3 or 4
 * @remark Based in TS 38.213 9.2.5.1 UE procedure for multiplexing HARQ-ACK or CSI and SR in a PUCCH
 * @return The number of PRB if the provided configuration is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ra_ul_nr_pucch_format_2_3_min_prb(const isrran_pucch_nr_resource_t* resource,
                                                        const isrran_uci_cfg_nr_t*        uci_cfg);

/**
 * @brief Calculates the PUSCH frequency resource allocation and stores it in the provided PUSCH NR grant.
 *
 * @remark Defined by TS 38.214 V15.10.0 section 6.1.2.2 Resource allocation in frequency domain (for DCI formats 0_0
 * and 0_1)
 * @remark Defined by TS 38.213 V15.10.0 section 8.3 for PUSCH scheduled by RAR UL grant
 * @param carrier Carrier information
 * @param cfg PDSCH NR configuration by upper layers
 * @param dci_dl Unpacked DCI used to schedule the PDSCH grant
 * @param[out] grant Provides grant pointer to fill
 * @return ISRRAN_SUCCESS if the provided data is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ra_ul_nr_freq(const isrran_carrier_nr_t*    carrier,
                                    const isrran_sch_hl_cfg_nr_t* cfg,
                                    const isrran_dci_ul_nr_t*     dci_ul,
                                    isrran_sch_grant_nr_t*        grant);

/**
 * @brief Selects a valid PUCCH resource for transmission
 * @param pucch_cfg PUCCH configuration from upper layers
 * @param uci_cfg Uplink Control information configuration (and PDCCH context)
 * @param N_bwp_sz Uplink BWP size in nof_prb
 * @param[out] resource Selected resource for transmitting PUCCH
 * @return ISRRAN_SUCCESS if provided configuration is valid, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ra_ul_nr_pucch_resource(const isrran_pucch_nr_hl_cfg_t* pucch_cfg,
                                              const isrran_uci_cfg_nr_t*      uci_cfg,
                                              uint32_t                        N_bwp_sz,
                                              isrran_pucch_nr_resource_t*     resource);

/**
 * @brief Computes the number of SR bits
 * @param K Number of SR transmission opportunities, including negative
 * @return The number of bits according to the number of SRs
 */
ISRRAN_API uint32_t isrran_ra_ul_nr_nof_sr_bits(uint32_t K);

#endif // ISRRAN_RA_UL_NR_H
