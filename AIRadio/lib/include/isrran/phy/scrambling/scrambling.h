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
 *  File:         scrambling.h
 *
 *  Description:  Generic scrambling functions used by UL and DL channels.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 5.3.1, 6.3.1
 *****************************************************************************/

#ifndef ISRRAN_SCRAMBLING_H
#define ISRRAN_SCRAMBLING_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/common/sequence.h"

/* Scrambling has no state */
ISRRAN_API void isrran_scrambling_b(isrran_sequence_t* s, uint8_t* data);

ISRRAN_API void isrran_scrambling_b_offset(isrran_sequence_t* s, uint8_t* data, int offset, int len);

ISRRAN_API void isrran_scrambling_bytes(isrran_sequence_t* s, uint8_t* data, int len);

ISRRAN_API void isrran_scrambling_f(isrran_sequence_t* s, float* data);

ISRRAN_API void isrran_scrambling_f_offset(isrran_sequence_t* s, float* data, int offset, int len);

ISRRAN_API void isrran_scrambling_s(isrran_sequence_t* s, short* data);

ISRRAN_API void isrran_scrambling_s_offset(isrran_sequence_t* s, short* data, int offset, int len);

ISRRAN_API void isrran_scrambling_sb_offset(isrran_sequence_t* s, int8_t* data, int offset, int len);

ISRRAN_API void isrran_scrambling_c(isrran_sequence_t* s, cf_t* data);

ISRRAN_API void isrran_scrambling_c_offset(isrran_sequence_t* s, cf_t* data, int offset, int len);

#endif // ISRRAN_SCRAMBLING_H
