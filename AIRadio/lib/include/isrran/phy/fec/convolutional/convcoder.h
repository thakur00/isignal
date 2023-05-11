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
 *  File:         convcoder.h
 *
 *  Description:  Convolutional encoder.
 *                LTE uses a tail biting convolutional code with constraint length 7
 *                and coding rate 1/3.
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10 Sec. 5.1.3.1
 *********************************************************************************************/

#ifndef ISRRAN_CONVCODER_H
#define ISRRAN_CONVCODER_H

#include "isrran/config.h"
#include <stdbool.h>

typedef struct ISRRAN_API {
  uint32_t R;
  uint32_t K;
  int      poly[3];
  bool     tail_biting;
} isrran_convcoder_t;

ISRRAN_API int isrran_convcoder_encode(isrran_convcoder_t* q, uint8_t* input, uint8_t* output, uint32_t frame_length);

#endif // ISRRAN_CONVCODER_H
