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
 *  File:         pss.h
 *
 *  Description:  Primary synchronization signal (PSS) generation and detection.
 *
 *                The isrran_pss_t object provides functions for fast
 *                computation of the crosscorrelation between the PSS and received
 *                signal and CFO estimation. Also, the function isrran_pss_tperiodic()
 *                is designed to be called periodically every subframe, taking
 *                care of the correct data alignment with respect to the PSS sequence.
 *
 *                The object is designed to work with signals sampled at 1.92 Mhz
 *                centered at the carrier frequency. Thus, downsampling is required
 *                if the signal is sampled at higher frequencies.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.11.1
 *****************************************************************************/

#ifndef ISRRAN_PSS_H
#define ISRRAN_PSS_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/utils/convolution.h"
#include "isrran/phy/utils/filter.h"

#define CONVOLUTION_FFT

#define ISRRAN_PSS_LEN 62
#define ISRRAN_PSS_RE (6 * 12)

/* PSS processing options */

#define ISRRAN_PSS_ACCUMULATE_ABS // If enabled, accumulates the correlation absolute value on consecutive calls to
                                  // isrran_pss_find_pss

#define ISRRAN_PSS_RETURN_PSR // If enabled returns peak to side-lobe ratio, otherwise returns absolute peak value

/* Low-level API */
typedef struct ISRRAN_API {

#ifdef CONVOLUTION_FFT
  isrran_conv_fft_cc_t conv_fft;
  isrran_filt_cc_t     filter;

#endif
  int decimate;

  uint32_t max_frame_size;
  uint32_t max_fft_size;

  uint32_t frame_size;
  uint32_t N_id_2;
  uint32_t fft_size;
  cf_t*    pss_signal_freq_full[3];

  cf_t* pss_signal_time[3];
  cf_t* pss_signal_time_scale[3];

  cf_t   pss_signal_freq[3][ISRRAN_PSS_LEN]; // One sequence for each N_id_2
  cf_t*  tmp_input;
  cf_t*  conv_output;
  float* conv_output_abs;
  float  ema_alpha;
  float* conv_output_avg;
  float  peak_value;

  bool              filter_pss_enable;
  isrran_dft_plan_t dftp_input;
  isrran_dft_plan_t idftp_input;
  cf_t              tmp_fft[ISRRAN_SYMBOL_SZ_MAX];
  cf_t              tmp_fft2[ISRRAN_SYMBOL_SZ_MAX];

  cf_t tmp_ce[ISRRAN_PSS_LEN];

  bool chest_on_filter;

} isrran_pss_t;

typedef enum { PSS_TX, PSS_RX } pss_direction_t;

/* Basic functionality */
ISRRAN_API int isrran_pss_init_fft(isrran_pss_t* q, uint32_t frame_size, uint32_t fft_size);

ISRRAN_API int isrran_pss_init_fft_offset(isrran_pss_t* q, uint32_t frame_size, uint32_t fft_size, int cfo_i);

ISRRAN_API int
isrran_pss_init_fft_offset_decim(isrran_pss_t* q, uint32_t frame_size, uint32_t fft_size, int cfo_i, int decimate);

ISRRAN_API int isrran_pss_resize(isrran_pss_t* q, uint32_t frame_size, uint32_t fft_size, int offset);

ISRRAN_API int isrran_pss_init(isrran_pss_t* q, uint32_t frame_size);

ISRRAN_API void isrran_pss_free(isrran_pss_t* q);

ISRRAN_API void isrran_pss_reset(isrran_pss_t* q);

ISRRAN_API void isrran_pss_filter_enable(isrran_pss_t* q, bool enable);

ISRRAN_API void isrran_pss_sic(isrran_pss_t* q, cf_t* input);

ISRRAN_API void isrran_pss_filter(isrran_pss_t* q, const cf_t* input, cf_t* output);

ISRRAN_API int isrran_pss_generate(cf_t* signal, uint32_t N_id_2);

ISRRAN_API void isrran_pss_get_slot(cf_t* slot, cf_t* pss_signal, uint32_t nof_prb, isrran_cp_t cp);

ISRRAN_API void isrran_pss_put_slot(cf_t* pss_signal, cf_t* slot, uint32_t nof_prb, isrran_cp_t cp);

ISRRAN_API void isrran_pss_set_ema_alpha(isrran_pss_t* q, float alpha);

ISRRAN_API int isrran_pss_set_N_id_2(isrran_pss_t* q, uint32_t N_id_2);

ISRRAN_API int isrran_pss_find_pss(isrran_pss_t* q, const cf_t* input, float* corr_peak_value);

ISRRAN_API int isrran_pss_chest(isrran_pss_t* q, const cf_t* input, cf_t ce[ISRRAN_PSS_LEN]);

ISRRAN_API float isrran_pss_cfo_compute(isrran_pss_t* q, const cf_t* pss_recv);

#endif // ISRRAN_PSS_H
