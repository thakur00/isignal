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

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "isrran/phy/io/netsink.h"

int isrran_netsink_init(isrran_netsink_t* q, const char* address, uint16_t port, isrran_netsink_type_t type)
{
  bzero(q, sizeof(isrran_netsink_t));

  q->sockfd = socket(AF_INET, type == ISRRAN_NETSINK_TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
  if (q->sockfd < 0) {
    perror("socket");
    return ISRRAN_ERROR;
  }

  int enable = 1;
#if defined(SO_REUSEADDR)
  if (setsockopt(q->sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");
#endif
#if defined(SO_REUSEPORT)
  if (setsockopt(q->sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEPORT) failed");
#endif

  q->servaddr.sin_family      = AF_INET;
  if (inet_pton(q->servaddr.sin_family, address, &q->servaddr.sin_addr) != 1) {
    perror("inet_pton");
    return ISRRAN_ERROR;
  }
  q->servaddr.sin_port        = htons(port);
  q->connected                = false;
  q->type                     = type;

  return ISRRAN_SUCCESS;
}

void isrran_netsink_free(isrran_netsink_t* q)
{
  if (q->sockfd) {
    close(q->sockfd);
  }
  bzero(q, sizeof(isrran_netsink_t));
}

int isrran_netsink_set_nonblocking(isrran_netsink_t* q)
{
  if (fcntl(q->sockfd, F_SETFL, O_NONBLOCK)) {
    perror("fcntl");
    return ISRRAN_ERROR;
  }
  return ISRRAN_SUCCESS;
}

int isrran_netsink_write(isrran_netsink_t* q, void* buffer, int nof_bytes)
{
  if (!q->connected) {
    if (connect(q->sockfd, &q->servaddr, sizeof(q->servaddr)) < 0) {
      if (errno == ECONNREFUSED || errno == EINPROGRESS) {
        return ISRRAN_SUCCESS;
      } else {
        perror("connect");
        exit(-1);
        return ISRRAN_ERROR;
      }
    } else {
      q->connected = true;
    }
  }
  int n = 0;
  if (q->connected) {
    n = write(q->sockfd, buffer, nof_bytes);
    if (n < 0) {
      if (errno == ECONNRESET) {
        close(q->sockfd);
        q->sockfd = socket(AF_INET, q->type == ISRRAN_NETSINK_TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
        if (q->sockfd < 0) {
          perror("socket");
          return ISRRAN_ERROR;
        }
        q->connected = false;
        return ISRRAN_SUCCESS;
      }
    }
  }
  return n;
}
