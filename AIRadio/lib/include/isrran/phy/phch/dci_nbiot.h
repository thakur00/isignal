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

/**
 *
 * @file dci_nbiot.h
 *
 * @brief Downlink control information (DCI) for NB-IoT.
 *
 * Packing/Unpacking functions to convert between bit streams
 * and packed DCI UL/DL grants defined in ra_nbiot.h
 *
 * Reference: 3GPP TS 36.212 version 13.2.0 Release 13 Sec. 6.4.3
 *
 */

#ifndef ISRRAN_DCI_NBIOT_H
#define ISRRAN_DCI_NBIOT_H

#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/ra_nbiot.h"

#define ISRRAN_DCI_MAX_BITS 128
#define ISRRAN_NBIOT_RAR_GRANT_LEN 15

ISRRAN_API void isrran_nbiot_dci_rar_grant_unpack(isrran_nbiot_dci_rar_grant_t* rar,
                                                  const uint8_t                 grant[ISRRAN_NBIOT_RAR_GRANT_LEN]);

ISRRAN_API int isrran_nbiot_dci_msg_to_dl_grant(const isrran_dci_msg_t*     msg,
                                                const uint16_t              msg_rnti,
                                                isrran_ra_nbiot_dl_dci_t*   dl_dci,
                                                isrran_ra_nbiot_dl_grant_t* grant,
                                                const uint32_t              sfn,
                                                const uint32_t              sf_idx,
                                                const uint32_t              r_max,
                                                const isrran_nbiot_mode_t   mode);

ISRRAN_API int isrran_nbiot_dci_msg_to_ul_grant(const isrran_dci_msg_t*          msg,
                                                isrran_ra_nbiot_ul_dci_t*        ul_dci,
                                                isrran_ra_nbiot_ul_grant_t*      grant,
                                                const uint32_t                   rx_tti,
                                                const isrran_npusch_sc_spacing_t spacing);

ISRRAN_API int
isrran_nbiot_dci_rar_to_ul_grant(isrran_nbiot_dci_rar_grant_t* rar, isrran_ra_nbiot_ul_grant_t* grant, uint32_t rx_tti);

ISRRAN_API bool isrran_nbiot_dci_location_isvalid(const isrran_dci_location_t* c);

ISRRAN_API int isrran_dci_msg_pack_npdsch(const isrran_ra_nbiot_dl_dci_t* data,
                                          const isrran_dci_format_t       format,
                                          isrran_dci_msg_t*               msg,
                                          const bool                      crc_is_crnti);

ISRRAN_API int
isrran_dci_msg_unpack_npdsch(const isrran_dci_msg_t* msg, isrran_ra_nbiot_dl_dci_t* data, const bool crc_is_crnti);

ISRRAN_API int isrran_dci_msg_unpack_npusch(const isrran_dci_msg_t* msg, isrran_ra_nbiot_ul_dci_t* data);

ISRRAN_API uint32_t isrran_dci_nbiot_format_sizeof(isrran_dci_format_t format);

#endif // ISRRAN_DCI_NBIOT_H
