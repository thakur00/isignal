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
 *  File:         dft_precoding.h
 *
 *  Description:  DFT-based transform precoding object.
 *                Used in generation of uplink SCFDMA signals.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 5.3.3
 *********************************************************************************************/

#ifndef ISRRAN_DFT_PRECODING_H
#define ISRRAN_DFT_PRECODING_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/dft.h"

/* DFT-based Transform Precoding object */
typedef struct ISRRAN_API {

  uint32_t          max_prb;
  isrran_dft_plan_t dft_plan[ISRRAN_MAX_PRB + 1];

} isrran_dft_precoding_t;

ISRRAN_API int isrran_dft_precoding_init(isrran_dft_precoding_t* q, uint32_t max_prb, bool is_tx);

ISRRAN_API int isrran_dft_precoding_init_tx(isrran_dft_precoding_t* q, uint32_t max_prb);

ISRRAN_API int isrran_dft_precoding_init_rx(isrran_dft_precoding_t* q, uint32_t max_prb);

ISRRAN_API void isrran_dft_precoding_free(isrran_dft_precoding_t* q);

ISRRAN_API bool isrran_dft_precoding_valid_prb(uint32_t nof_prb);

ISRRAN_API uint32_t isrran_dft_precoding_get_valid_prb(uint32_t nof_prb);

ISRRAN_API int
isrran_dft_precoding(isrran_dft_precoding_t* q, cf_t* input, cf_t* output, uint32_t nof_prb, uint32_t nof_symbols);

#endif // ISRRAN_DFT_PRECODING_H
