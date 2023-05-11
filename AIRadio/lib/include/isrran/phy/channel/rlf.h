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

#ifndef ISRRAN_RLF_H
#define ISRRAN_RLF_H

#include "isrran/config.h"
#include "isrran/phy/common/timestamp.h"
#include <stdint.h>

typedef struct {
  uint32_t t_on_ms;
  uint32_t t_off_ms;
} isrran_channel_rlf_t;

#ifdef __cplusplus
extern "C" {
#endif

ISRRAN_API void isrran_channel_rlf_init(isrran_channel_rlf_t* q, uint32_t t_on_ms, uint32_t t_off_ms);

ISRRAN_API void isrran_channel_rlf_execute(isrran_channel_rlf_t*     q,
                                           const cf_t*               in,
                                           cf_t*                     out,
                                           uint32_t                  nsamples,
                                           const isrran_timestamp_t* ts);

ISRRAN_API void isrran_channel_rlf_free(isrran_channel_rlf_t* q);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_RLF_H
