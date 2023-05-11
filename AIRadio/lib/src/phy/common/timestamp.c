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

#include "isrran/phy/common/timestamp.h"
#include "math.h"

int isrran_timestamp_init(isrran_timestamp_t* t, time_t full_secs, double frac_secs)
{
  int ret = ISRRAN_ERROR;
  if (t != NULL && frac_secs >= 0.0) {
    t->full_secs = full_secs;
    t->frac_secs = frac_secs;
    ret          = ISRRAN_SUCCESS;
  }
  return ret;
}

void isrran_timestamp_init_uint64(isrran_timestamp_t* ts_time, uint64_t ts_count, double base_srate)
{
  uint64_t seconds      = ts_count / (uint64_t)base_srate;
  uint64_t frac_samples = (uint64_t)(seconds * (uint64_t)base_srate);
  double   frac_seconds = (double)(ts_count - frac_samples) / base_srate;

  if (ts_time) {
    isrran_timestamp_init(ts_time, seconds, frac_seconds);
  }
}

int isrran_timestamp_copy(isrran_timestamp_t* dest, isrran_timestamp_t* src)
{
  int ret = ISRRAN_ERROR;
  if (dest != NULL && src != NULL) {
    dest->full_secs = src->full_secs;
    dest->frac_secs = src->frac_secs;
    ret             = ISRRAN_SUCCESS;
  }
  return ret;
}

int isrran_timestamp_compare(const isrran_timestamp_t* a, const isrran_timestamp_t* b)
{
  int ret = 0;

  if (a->full_secs > b->full_secs) {
    ret = +1;
  } else if (a->full_secs < b->full_secs) {
    ret = -1;
  } else if (a->frac_secs > b->frac_secs) {
    ret = +1;
  } else if (a->frac_secs < b->frac_secs) {
    ret = -1;
  }

  return ret;
}

int isrran_timestamp_add(isrran_timestamp_t* t, time_t full_secs, double frac_secs)
{
  int ret = ISRRAN_ERROR;
  if (t != NULL && frac_secs >= 0.0) {
    t->frac_secs += frac_secs;
    t->full_secs += full_secs;
    double r = floor(t->frac_secs);
    t->full_secs += r;
    t->frac_secs -= r;
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

int isrran_timestamp_sub(isrran_timestamp_t* t, time_t full_secs, double frac_secs)
{
  int ret = ISRRAN_ERROR;
  if (t != NULL && frac_secs >= 0.0) {
    t->frac_secs -= frac_secs;
    t->full_secs -= full_secs;
    if (t->frac_secs < 0) {
      t->frac_secs = t->frac_secs + 1;
      t->full_secs--;
    }
    if (t->full_secs < 0)
      return ISRRAN_ERROR;
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

double isrran_timestamp_real(const isrran_timestamp_t* t)
{
  return t->frac_secs + t->full_secs;
}

bool isrran_timestamp_iszero(const isrran_timestamp_t* t)
{
  return ((t->full_secs == 0) && (t->frac_secs == 0));
}

uint32_t isrran_timestamp_uint32(isrran_timestamp_t* t)
{
  uint32_t x = t->full_secs * 1e6 + (uint32_t)(t->frac_secs * 1e6);
  return x;
}

uint64_t isrran_timestamp_uint64(const isrran_timestamp_t* t, double srate)
{
  return (uint64_t)(t->full_secs * (uint64_t)srate) + (uint64_t)round(t->frac_secs * srate);
}
