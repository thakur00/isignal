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
 *  File:         netsink.h
 *
 *  Description:  Network socket sink.
 *                Supports writing binary data to a TCP or UDP socket.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_NETSINK_H
#define ISRRAN_NETSINK_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "isrran/config.h"

typedef enum { ISRRAN_NETSINK_UDP, ISRRAN_NETSINK_TCP } isrran_netsink_type_t;

/* Low-level API */
typedef struct ISRRAN_API {
  int                   sockfd;
  bool                  connected;
  isrran_netsink_type_t type;
  struct sockaddr_in    servaddr;
} isrran_netsink_t;

ISRRAN_API int isrran_netsink_init(isrran_netsink_t* q, const char* address, uint16_t port, isrran_netsink_type_t type);

ISRRAN_API void isrran_netsink_free(isrran_netsink_t* q);

ISRRAN_API int isrran_netsink_write(isrran_netsink_t* q, void* buffer, int nof_bytes);

ISRRAN_API int isrran_netsink_set_nonblocking(isrran_netsink_t* q);

#endif // ISRRAN_NETSINK_H
