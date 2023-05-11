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
 *  File:         netsource.h
 *
 *  Description:  Network socket source.
 *                Supports reading binary data from a TCP or UDP socket.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_NETSOURCE_H
#define ISRRAN_NETSOURCE_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "isrran/config.h"

typedef enum { ISRRAN_NETSOURCE_UDP, ISRRAN_NETSOURCE_TCP } isrran_netsource_type_t;

/* Low-level API */
typedef struct ISRRAN_API {
  int                     sockfd;
  int                     connfd;
  struct sockaddr_in      servaddr;
  isrran_netsource_type_t type;
  struct sockaddr_in      cliaddr;
} isrran_netsource_t;

ISRRAN_API int
isrran_netsource_init(isrran_netsource_t* q, const char* address, uint16_t port, isrran_netsource_type_t type);

ISRRAN_API void isrran_netsource_free(isrran_netsource_t* q);

ISRRAN_API int isrran_netsource_set_nonblocking(isrran_netsource_t* q);

ISRRAN_API int isrran_netsource_read(isrran_netsource_t* q, void* buffer, int nof_bytes);

ISRRAN_API int isrran_netsource_write(isrran_netsource_t* q, void* buffer, int nbytes);

ISRRAN_API int isrran_netsource_set_timeout(isrran_netsource_t* q, uint32_t microseconds);

#endif // ISRRAN_NETSOURCE_H
