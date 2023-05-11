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

#ifndef ISRRAN_RINGBUFFER_H
#define ISRRAN_RINGBUFFER_H

#include "isrran/config.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t*        buffer;
  bool            active;
  int             capacity;
  int             count;
  int             wpm;
  int             rpm;
  pthread_mutex_t mutex;
  pthread_cond_t  write_cvar;
  pthread_cond_t  read_cvar;
} isrran_ringbuffer_t;

#ifdef __cplusplus
extern "C" {
#endif

ISRRAN_API int isrran_ringbuffer_init(isrran_ringbuffer_t* q, int capacity);

ISRRAN_API void isrran_ringbuffer_free(isrran_ringbuffer_t* q);

ISRRAN_API void isrran_ringbuffer_reset(isrran_ringbuffer_t* q);

ISRRAN_API int isrran_ringbuffer_status(isrran_ringbuffer_t* q);

ISRRAN_API int isrran_ringbuffer_space(isrran_ringbuffer_t* q);

ISRRAN_API int isrran_ringbuffer_resize(isrran_ringbuffer_t* q, int capacity);

// write to the buffer immediately, if there isnt enough space it will overflow
ISRRAN_API int isrran_ringbuffer_write(isrran_ringbuffer_t* q, void* ptr, int nof_bytes);

// block forever until there is enough space then write to buffer
ISRRAN_API int isrran_ringbuffer_write_block(isrran_ringbuffer_t* q, void* ptr, int nof_bytes);

// block for timeout_ms milliseconds, then either  write to buffer if there is space or return an error without writing
ISRRAN_API int isrran_ringbuffer_write_timed(isrran_ringbuffer_t* q, void* ptr, int nof_bytes, int32_t timeout_ms);

ISRRAN_API int
isrran_ringbuffer_write_timed_block(isrran_ringbuffer_t* q, void* ptr, int nof_bytes, int32_t timeout_ms);

// read from buffer, blocking until there is enough samples
ISRRAN_API int isrran_ringbuffer_read(isrran_ringbuffer_t* q, void* ptr, int nof_bytes);

// read from buffer, blocking for timeout_ms milliseconds until there is enough samples or return an error
ISRRAN_API int isrran_ringbuffer_read_timed(isrran_ringbuffer_t* q, void* p, int nof_bytes, int32_t timeout_ms);

ISRRAN_API int isrran_ringbuffer_read_timed_block(isrran_ringbuffer_t* q, void* p, int nof_bytes, int32_t timeout_ms);

// read samples from the buffer, convert them from uint16_t to cplx float and get the conjugate
ISRRAN_API int isrran_ringbuffer_read_convert_conj(isrran_ringbuffer_t* q, cf_t* dst_ptr, float norm, int nof_samples);

ISRRAN_API int isrran_ringbuffer_read_block(isrran_ringbuffer_t* q, void** p, int nof_bytes, int32_t timeout_ms);

ISRRAN_API void isrran_ringbuffer_stop(isrran_ringbuffer_t* q);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_RINGBUFFER_H
