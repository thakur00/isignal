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
 *  File:         npss.h
 *
 *  Description:  Narrowband Primary synchronization signal (NPSS) generation and detection.
 *
 *                The isrran_npss_synch_t object provides functions for fast
 *                computation of the crosscorrelation between the NPSS and received
 *                signal and CFO estimation. Also, the function isrran_npss_synch_tperiodic()
 *                is designed to be called periodically every subframe, taking
 *                care of the correct data alignment with respect to the NPSS sequence.
 *
 *                The object is designed to work with signals sampled at ?.? Mhz
 *                centered at the carrier frequency. Thus, downsampling is required
 *                if the signal is sampled at higher frequencies.
 *
 *  Reference:    3GPP TS 36.211 version 13.2.0 Release 13 Sec. 10.x.x
 *****************************************************************************/

#ifndef ISRRAN_NPSS_H
#define ISRRAN_NPSS_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/utils/convolution.h"

#define CONVOLUTION_FFT

#define ISRRAN_NPSS_RETURN_PSR

#define ISRRAN_NPSS_LEN 11
#define ISRRAN_NPSS_NUM_OFDM_SYMS 11
#define ISRRAN_NPSS_TOT_LEN (ISRRAN_NPSS_LEN * ISRRAN_NPSS_NUM_OFDM_SYMS)

#define ISRRAN_NPSS_CORR_FILTER_LEN                                                                                    \
  ((ISRRAN_NPSS_NUM_OFDM_SYMS * ISRRAN_NBIOT_FFT_SIZE) +                                                               \
   (ISRRAN_NPSS_NUM_OFDM_SYMS - 1) * ISRRAN_CP_LEN_NORM(1, ISRRAN_NBIOT_FFT_SIZE) +                                    \
   ISRRAN_CP_LEN_NORM(0, ISRRAN_NBIOT_FFT_SIZE))

// The below value corresponds to the time-domain representation of the first
// three OFDM-symbols plus cyclic prefix that are not transmitted in the sub-frame
// carrying the NPSS
#define ISRRAN_NPSS_CORR_OFFSET (ISRRAN_SF_LEN(ISRRAN_NBIOT_FFT_SIZE) - ISRRAN_NPSS_CORR_FILTER_LEN)

// CFO estimation based on the NPSS is done using the second slot of the sub-frame
#define ISRRAN_NPSS_CFO_OFFSET (ISRRAN_SF_LEN(ISRRAN_NBIOT_FFT_SIZE) / 2 - ISRRAN_NPSS_CORR_OFFSET)
#define ISRRAN_NPSS_CFO_NUM_SYMS 6 // number of symbols for CFO estimation
#define ISRRAN_NPSS_CFO_NUM_SAMPS                                                                                      \
  ((ISRRAN_NPSS_CFO_NUM_SYMS * ISRRAN_NBIOT_FFT_SIZE) +                                                                \
   (ISRRAN_NPSS_CFO_NUM_SYMS - 1) * ISRRAN_CP_LEN_NORM(1, ISRRAN_NBIOT_FFT_SIZE) +                                     \
   ISRRAN_CP_LEN_NORM(0, ISRRAN_NBIOT_FFT_SIZE)) // resulting number of samples

// NPSS processing options
#define ISRRAN_NPSS_ACCUMULATE_ABS // If enabled, accumulates the correlation absolute value on consecutive calls to
                                   // isrran_pss_synch_find_pss

#define ISRRAN_NPSS_ABS_SQUARE // If enabled, compute abs square, otherwise computes absolute value only

#define ISRRAN_NPSS_RETURN_PSR // If enabled returns peak to side-lobe ratio, otherwise returns absolute peak value

/* Low-level API */
typedef struct ISRRAN_API {
#ifdef CONVOLUTION_FFT
  isrran_conv_fft_cc_t conv_fft;
#endif

  uint32_t frame_size, max_frame_size;
  uint32_t fft_size, max_fft_size;

  cf_t*  npss_signal_time;
  cf_t*  tmp_input;
  cf_t*  conv_output;
  float* conv_output_abs;
  float  ema_alpha;
  float* conv_output_avg;
  float  peak_value;
} isrran_npss_synch_t;

// Basic functionality
ISRRAN_API int isrran_npss_synch_init(isrran_npss_synch_t* q, uint32_t frame_size, uint32_t fft_size);

ISRRAN_API void isrran_npss_synch_reset(isrran_npss_synch_t* q);

ISRRAN_API int isrran_npss_synch_resize(isrran_npss_synch_t* q, uint32_t frame_size, uint32_t fft_size);

ISRRAN_API void isrran_npss_synch_set_ema_alpha(isrran_npss_synch_t* q, float alpha);

ISRRAN_API void isrran_npss_synch_free(isrran_npss_synch_t* q);

ISRRAN_API int isrran_npss_sync_find(isrran_npss_synch_t* q, cf_t* input, float* corr_peak_value);

// Internal functions
ISRRAN_API int isrran_npss_corr_init(cf_t* npss_signal_time, uint32_t fft_size, uint32_t frame_size);

ISRRAN_API int isrran_npss_generate(cf_t* signal);

ISRRAN_API void isrran_npss_put_subframe(isrran_npss_synch_t* q,
                                         cf_t*                npss_signal,
                                         cf_t*                sf,
                                         const uint32_t       nof_prb,
                                         const uint32_t       nbiot_prb_offset);

#endif // ISRRAN_NPSS_H
