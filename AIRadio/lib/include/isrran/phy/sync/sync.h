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
 *  File:         sync.h
 *
 *  Description:  Time and frequency synchronization using the PSS and SSS signals.
 *
 *                The object is designed to work with signals sampled at 1.92 Mhz
 *                centered at the carrier frequency. Thus, downsampling is required
 *                if the signal is sampled at higher frequencies.
 *
 *                Correlation peak is detected comparing the maximum at the output
 *                of the correlator with a threshold. The comparison accepts two
 *                modes: absolute value or peak-to-mean ratio, which are configured
 *                with the functions sync_pss_det_absolute() and sync_pss_det_peakmean().
 *
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 6.11.1, 6.11.2
 *****************************************************************************/

#ifndef ISRRAN_SYNC_H
#define ISRRAN_SYNC_H

#include <math.h>
#include <stdbool.h>

#include "isrran/config.h"
#include "isrran/phy/sync/cfo.h"
#include "isrran/phy/sync/cp.h"
#include "isrran/phy/sync/pss.h"
#include "isrran/phy/sync/sss.h"

#define ISRRAN_SYNC_FFT_SZ_MIN 64
#define ISRRAN_SYNC_FFT_SZ_MAX 2048

typedef enum { SSS_DIFF = 0, SSS_PARTIAL_3 = 2, SSS_FULL = 1 } sss_alg_t;

typedef struct ISRRAN_API {
  isrran_pss_t      pss;
  isrran_pss_t      pss_i[2];
  isrran_sss_t      sss;
  isrran_cp_synch_t cp_synch;
  cf_t*             cfo_i_corr[2];
  int               decimate;
  float             threshold;
  float             peak_value;
  uint32_t          N_id_2;
  uint32_t          N_id_1;
  uint32_t          sf_idx;
  uint32_t          fft_size;
  uint32_t          frame_size;
  uint32_t          max_offset;
  uint32_t          nof_symbols;
  uint32_t          cp_len;
  float             current_cfo_tol;
  sss_alg_t         sss_alg;
  bool              detect_cp;
  bool              sss_en;
  isrran_cp_t       cp;
  uint32_t          m0;
  uint32_t          m1;
  float             m0_value;
  float             m1_value;
  float             M_norm_avg;
  float             M_ext_avg;
  cf_t*             temp;

  uint32_t max_frame_size;

  isrran_frame_type_t frame_type;
  bool                detect_frame_type;

  // variables for various CFO estimation methods
  bool cfo_cp_enable;
  bool cfo_pss_enable;
  bool cfo_i_enable;

  bool cfo_cp_is_set;
  bool cfo_pss_is_set;
  bool cfo_i_initiated;

  float cfo_cp_mean;
  float cfo_pss;
  float cfo_pss_mean;
  int   cfo_i_value;

  float cfo_ema_alpha;

  uint32_t cfo_cp_nsymbols;

  isrran_cfo_t cfo_corr_frame;
  isrran_cfo_t cfo_corr_symbol;

  bool sss_channel_equalize;
  bool pss_filtering_enabled;
  cf_t sss_filt[ISRRAN_SYMBOL_SZ_MAX];
  cf_t pss_filt[ISRRAN_SYMBOL_SZ_MAX];

  bool              sss_generated;
  bool              sss_detected;
  bool              sss_available;
  float             sss_corr;
  isrran_dft_plan_t idftp_sss;
  cf_t              sss_recv[ISRRAN_SYMBOL_SZ_MAX];
  cf_t              sss_signal[2][ISRRAN_SYMBOL_SZ_MAX];

} isrran_sync_t;

typedef enum {
  ISRRAN_SYNC_FOUND         = 1,
  ISRRAN_SYNC_FOUND_NOSPACE = 2,
  ISRRAN_SYNC_NOFOUND       = 0,
  ISRRAN_SYNC_ERROR         = -1
} isrran_sync_find_ret_t;

ISRRAN_API int isrran_sync_init(isrran_sync_t* q, uint32_t frame_size, uint32_t max_offset, uint32_t fft_size);

ISRRAN_API int
isrran_sync_init_decim(isrran_sync_t* q, uint32_t frame_size, uint32_t max_offset, uint32_t fft_size, int decimate);

ISRRAN_API void isrran_sync_free(isrran_sync_t* q);

ISRRAN_API int isrran_sync_resize(isrran_sync_t* q, uint32_t frame_size, uint32_t max_offset, uint32_t fft_size);

ISRRAN_API void isrran_sync_reset(isrran_sync_t* q);

/* Finds a correlation peak in the input signal around position find_offset */
ISRRAN_API isrran_sync_find_ret_t isrran_sync_find(isrran_sync_t* q,
                                                   const cf_t*    input,
                                                   uint32_t       find_offset,
                                                   uint32_t*      peak_position);

/* Estimates the CP length */
ISRRAN_API isrran_cp_t isrran_sync_detect_cp(isrran_sync_t* q, const cf_t* input, uint32_t peak_pos);

/* Sets the threshold for peak comparison */
ISRRAN_API void isrran_sync_set_threshold(isrran_sync_t* q, float threshold);

/* Gets the subframe idx (0 or 5) */
ISRRAN_API uint32_t isrran_sync_get_sf_idx(isrran_sync_t* q);

/* Gets the peak value */
ISRRAN_API float isrran_sync_get_peak_value(isrran_sync_t* q);

/* Choose SSS detection algorithm */
ISRRAN_API void isrran_sync_set_sss_algorithm(isrran_sync_t* q, sss_alg_t alg);

/* Sets PSS exponential averaging alpha weight */
ISRRAN_API void isrran_sync_set_em_alpha(isrran_sync_t* q, float alpha);

/* Sets the N_id_2 to search for */
ISRRAN_API int isrran_sync_set_N_id_2(isrran_sync_t* q, uint32_t N_id_2);

ISRRAN_API int isrran_sync_set_N_id_1(isrran_sync_t* q, uint32_t N_id_1);

/* Gets the Physical CellId from the last call to synch_run() */
ISRRAN_API int isrran_sync_get_cell_id(isrran_sync_t* q);

/* Enables/disables filtering of the central PRBs before PSS CFO estimation or SSS correlation*/
ISRRAN_API void isrran_sync_set_pss_filt_enable(isrran_sync_t* q, bool enable);

ISRRAN_API void isrran_sync_set_sss_eq_enable(isrran_sync_t* q, bool enable);

/* Gets the CFO estimation from the last call to synch_run() */
ISRRAN_API float isrran_sync_get_cfo(isrran_sync_t* q);

/* Resets internal CFO state */
ISRRAN_API void isrran_sync_cfo_reset(isrran_sync_t* q, float cfo_Hz);

/* Copies CFO internal state from another object to avoid long transients */
ISRRAN_API void isrran_sync_copy_cfo(isrran_sync_t* q, isrran_sync_t* src_obj);

/* Enable different CFO estimation stages */
ISRRAN_API void isrran_sync_set_cfo_i_enable(isrran_sync_t* q, bool enable);
ISRRAN_API void isrran_sync_set_cfo_cp_enable(isrran_sync_t* q, bool enable, uint32_t nof_symbols);

ISRRAN_API void isrran_sync_set_cfo_pss_enable(isrran_sync_t* q, bool enable);

/* Sets CFO correctors tolerance (in Hz) */
ISRRAN_API void isrran_sync_set_cfo_tol(isrran_sync_t* q, float tol);

ISRRAN_API void isrran_sync_set_frame_type(isrran_sync_t* q, isrran_frame_type_t frame_type);

/* Sets the exponential moving average coefficient for CFO averaging */
ISRRAN_API void isrran_sync_set_cfo_ema_alpha(isrran_sync_t* q, float alpha);

/* Gets the CP length estimation from the last call to synch_run() */
ISRRAN_API isrran_cp_t isrran_sync_get_cp(isrran_sync_t* q);

/* Sets the CP length estimation (must do it if disabled) */
ISRRAN_API void isrran_sync_set_cp(isrran_sync_t* q, isrran_cp_t cp);

/* Enables/Disables SSS detection  */
ISRRAN_API void isrran_sync_sss_en(isrran_sync_t* q, bool enabled);

ISRRAN_API isrran_pss_t* isrran_sync_get_cur_pss_obj(isrran_sync_t* q);

ISRRAN_API bool isrran_sync_sss_detected(isrran_sync_t* q);

ISRRAN_API float isrran_sync_sss_correlation_peak(isrran_sync_t* q);

ISRRAN_API bool isrran_sync_sss_available(isrran_sync_t* q);

/* Enables/Disables CP detection  */
ISRRAN_API void isrran_sync_cp_en(isrran_sync_t* q, bool enabled);

#endif // ISRRAN_SYNC_H
