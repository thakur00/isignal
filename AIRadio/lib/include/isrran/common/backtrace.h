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
 *  File:         backtrace.h
 *
 *  Description:  print backtrace in runtime.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_BACKTRACE_H
#define ISRRAN_BACKTRACE_H

#include "isrran/config.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

ISRRAN_API void isrran_backtrace_print(FILE* f);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ISRRAN_BACKTRACE_H
