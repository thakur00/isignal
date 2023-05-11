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
 *  File:         rm_conv.h
 *
 *  Description:  Rate matching for convolutionally coded transport channels and control
 *                information.
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10 Sec. 5.1.4.2
 *********************************************************************************************/

#ifndef ISRRAN_RM_CONV_H
#define ISRRAN_RM_CONV_H

#include "isrran/config.h"

#ifndef ISRRAN_RX_NULL
#define ISRRAN_RX_NULL 10000
#endif

#ifndef ISRRAN_TX_NULL
#define ISRRAN_TX_NULL 100
#endif
ISRRAN_API int isrran_rm_conv_tx(uint8_t* input, uint32_t in_len, uint8_t* output, uint32_t out_len);

ISRRAN_API int isrran_rm_conv_rx(float* input, uint32_t in_len, float* output, uint32_t out_len);

/************* FIX THIS. MOVE ALL PROCESSING TO INT16 AND HAVE ONLY 1 IMPLEMENTATION ******/

ISRRAN_API int isrran_rm_conv_rx_s(int16_t* input, uint32_t in_len, int16_t* output, uint32_t out_len);

#endif // ISRRAN_RM_CONV_H
