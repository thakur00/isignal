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
 *  File:         agc.h
 *
 *  Description:  Automatic gain control
 *                This module is not currently used
 *
 *  Reference:
 *********************************************************************************************/

#ifndef ISRRAN_AGC_H
#define ISRRAN_AGC_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"

#define ISRRAN_AGC_CALLBACK(NAME) void(NAME)(void* h, float gain_db)
#define ISRRAN_AGC_DEFAULT_TARGET (0.3f)  /* Average RMS target or maximum peak target*/
#define ISRRAN_AGC_DEFAULT_BW (0.3f)      /* Moving average coefficient */
#define ISRRAN_AGC_HOLD_COUNT (20)        /* Number of frames to wait after setting the gain before start measuring */
#define ISRRAN_AGC_MIN_MEASUREMENTS (10)  /* Minimum number of measurements  */
#define ISRRAN_AGC_MIN_GAIN_OFFSET (2.0f) /* Mimum of gain offset to set the radio gain */

typedef enum ISRRAN_API { ISRRAN_AGC_MODE_ENERGY = 0, ISRRAN_AGC_MODE_PEAK_AMPLITUDE } isrran_agc_mode_t;

/*
 * The AGC has been implemented using 3 states:
 *  - init: it simply starts the process of measuring
 *  - measure: performs a minimum of ISRRAN_AGC_MIN_MEASUREMENTS and does not set the gain until it needs
 *             ISRRAN_AGC_MIN_GAIN_OFFSET dB more of gain. The gain is set in the enter hold transition.
 *  - hold: waits for ISRRAN_AGC_HOLD_COUNT frames as a Rx gain transition period. After this period, it enters measure
 *          state.
 *
 * FSM abstraction:
 *
 * +------+   Enter measure  +---------+   Enter hold   +------+
 * | init | ---------------->| Measure |--------------->| Hold |
 * +------+                  +---------+                +------+
 *                                ^      Enter measure      |
 *                                +-------------------------+
 */

typedef enum { ISRRAN_AGC_STATE_INIT = 0, ISRRAN_AGC_STATE_MEASURE, ISRRAN_AGC_STATE_HOLD } isrran_agc_state_t;

typedef struct ISRRAN_API {
  float bandwidth;
  float gain_db;
  float gain_offset_db;
  float min_gain_db;
  float max_gain_db;
  float default_gain_db;
  float y_out;
  bool  isfirst;
  void* uhd_handler;
  ISRRAN_AGC_CALLBACK(*set_gain_callback);
  isrran_agc_mode_t  mode;
  float              target;
  uint32_t           nof_frames;
  uint32_t           frame_cnt;
  uint32_t           hold_cnt;
  float*             y_tmp;
  isrran_agc_state_t state;
} isrran_agc_t;

ISRRAN_API int isrran_agc_init_acc(isrran_agc_t* q, isrran_agc_mode_t mode, uint32_t nof_frames);

ISRRAN_API int isrran_agc_init_uhd(isrran_agc_t*     q,
                                   isrran_agc_mode_t mode,
                                   uint32_t          nof_frames,
                                   ISRRAN_AGC_CALLBACK(set_gain_callback),
                                   void* uhd_handler);

ISRRAN_API void isrran_agc_free(isrran_agc_t* q);

ISRRAN_API void isrran_agc_reset(isrran_agc_t* q);

ISRRAN_API void isrran_agc_set_gain_range(isrran_agc_t* q, float min_gain_db, float max_gain_db);

ISRRAN_API float isrran_agc_get_gain(isrran_agc_t* q);

ISRRAN_API void isrran_agc_set_gain(isrran_agc_t* q, float init_gain_value_db);

ISRRAN_API void isrran_agc_process(isrran_agc_t* q, cf_t* signal, uint32_t len);

#endif // ISRRAN_AGC_H
