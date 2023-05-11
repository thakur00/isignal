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

#ifndef ISRRAN_HARQ_ACK_H
#define ISRRAN_HARQ_ACK_H

#include "isrran/phy/phch/dci_nr.h"
#include "isrran/phy/phch/harq_ack_cfg.h"
#include "isrran/phy/phch/uci_cfg_nr.h"

ISRRAN_API int isrran_harq_ack_resource(const isrran_harq_ack_cfg_hl_t* cfg,
                                        const isrran_dci_dl_nr_t*       dci_dl,
                                        isrran_harq_ack_resource_t*     pdsch_ack_resource);

ISRRAN_API int isrran_harq_ack_gen_uci_cfg(const isrran_harq_ack_cfg_hl_t* cfg,
                                           const isrran_pdsch_ack_nr_t*    ack_info,
                                           isrran_uci_cfg_nr_t*            uci_cfg);

ISRRAN_API int isrran_harq_ack_pack(const isrran_harq_ack_cfg_hl_t* cfg,
                                    const isrran_pdsch_ack_nr_t*    ack_info,
                                    isrran_uci_data_nr_t*           uci_data);

ISRRAN_API int isrran_harq_ack_unpack(const isrran_harq_ack_cfg_hl_t* cfg,
                                      const isrran_uci_data_nr_t*     uci_data,
                                      isrran_pdsch_ack_nr_t*          ack_info);

ISRRAN_API int isrran_harq_ack_insert_m(isrran_pdsch_ack_nr_t* ack_info, const isrran_harq_ack_m_t* m);

ISRRAN_API uint32_t isrran_harq_ack_info(const isrran_pdsch_ack_nr_t* ack_info, char* str, uint32_t str_len);

#endif // ISRRAN_HARQ_ACK_H
