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
 *  File:         tc_interl.h
 *
 *  Description:  Turbo code interleaver.
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10 Sec. 5.1.3.2.3
 *********************************************************************************************/

#ifndef ISRRAN_TC_INTERL_H
#define ISRRAN_TC_INTERL_H

#include "isrran/config.h"
#include <stdint.h>

typedef struct ISRRAN_API {
  uint16_t* forward;
  uint16_t* reverse;
  uint32_t  max_long_cb;
} isrran_tc_interl_t;

ISRRAN_API int isrran_tc_interl_LTE_gen(isrran_tc_interl_t* h, uint32_t long_cb);

ISRRAN_API int isrran_tc_interl_LTE_gen_interl(isrran_tc_interl_t* h, uint32_t long_cb, uint32_t interl_win);

ISRRAN_API int isrran_tc_interl_init(isrran_tc_interl_t* h, uint32_t max_long_cb);

ISRRAN_API void isrran_tc_interl_free(isrran_tc_interl_t* h);

#endif // ISRRAN_TC_INTERL_H
