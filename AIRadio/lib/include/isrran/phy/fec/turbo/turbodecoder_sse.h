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

#ifndef ISRRAN_TURBODECODER_SSE_H
#define ISRRAN_TURBODECODER_SSE_H

#include "isrran/config.h"

typedef struct ISRRAN_API {
  uint32_t max_long_cb;
  int16_t* alpha;
  int16_t* branch;
} tdec_sse_t;

int  tdec_sse_init(void** h, uint32_t max_long_cb);
void tdec_sse_free(void* h);
void tdec_sse_dec(void* h, int16_t* input, int16_t* app, int16_t* parity, int16_t* output, uint32_t long_cb);
void tdec_sse_extract_input(int16_t* input,
                            int16_t* syst,
                            int16_t* parity0,
                            int16_t* parity1,
                            int16_t* app2,
                            uint32_t long_cb);
void tdec_sse_decision_byte(int16_t* app1, uint8_t* output, uint32_t long_cb);

#endif // ISRRAN_TURBODECODER_SSE_H
