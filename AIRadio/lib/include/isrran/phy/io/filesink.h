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
 *  File:         filesink.h
 *
 *  Description:  File sink.
 *                Supports writing floats, complex floats and complex shorts
 *                to file in text or binary formats.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_FILESINK_H
#define ISRRAN_FILESINK_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "isrran/config.h"
#include "isrran/phy/io/format.h"

/* Low-level API */
typedef struct ISRRAN_API {
  FILE*             f;
  isrran_datatype_t type;
} isrran_filesink_t;

ISRRAN_API int isrran_filesink_init(isrran_filesink_t* q, const char* filename, isrran_datatype_t type);

ISRRAN_API void isrran_filesink_free(isrran_filesink_t* q);

ISRRAN_API int isrran_filesink_write(isrran_filesink_t* q, void* buffer, int nsamples);

ISRRAN_API int isrran_filesink_write_multi(isrran_filesink_t* q, void** buffer, int nsamples, int nchannels);

#endif // ISRRAN_FILESINK_H
