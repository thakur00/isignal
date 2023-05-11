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

#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/dft.h"
#include "isrran/phy/io/filesink.h"
#include "isrran/phy/sync/npss.h"
#include "isrran/phy/utils/convolution.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

#define PRINT_ERR(err) fprintf(stderr, "%s(): %s", __PRETTY_FUNCTION__, err)

#define DUMP_SIGNALS 0
#define DO_FREQ_SHIFT 1

const float factor_lut[ISRRAN_NPSS_LEN] = {1, 1, 1, 1, -1, -1, 1, 1, 1, -1, 1};

/* Initializes the NPSS synchronization object.
 *
 * It correlates a signal of frame_size samples with the NPSS sequence in the time domain
 * The NPSS sequence is transformed using 11 * fft_size samples plus cyclic prefix.
 */
int isrran_npss_synch_init(isrran_npss_synch_t* q, uint32_t frame_size, uint32_t fft_size)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL) {
    ret = ISRRAN_ERROR;

    memset(q, 0, sizeof(isrran_npss_synch_t));

    q->fft_size = q->max_fft_size = fft_size;
    q->frame_size = q->max_frame_size = frame_size;
    q->ema_alpha                      = 0.2;

    uint32_t buffer_size = ISRRAN_NPSS_CORR_FILTER_LEN + frame_size + 1;

    q->tmp_input = isrran_vec_cf_malloc(buffer_size);
    if (!q->tmp_input) {
      PRINT_ERR("Error allocating memory\n");
      goto clean_and_exit;
    }
    memset(q->tmp_input, 0, buffer_size * sizeof(cf_t));

    q->conv_output = isrran_vec_cf_malloc(buffer_size);
    if (!q->conv_output) {
      fprintf(stderr, "Error allocating memory\n");
      goto clean_and_exit;
    }
    memset(q->conv_output, 0, sizeof(cf_t) * buffer_size);
    q->conv_output_avg = isrran_vec_f_malloc(buffer_size);
    if (!q->conv_output_avg) {
      fprintf(stderr, "Error allocating memory\n");
      goto clean_and_exit;
    }
    memset(q->conv_output_avg, 0, sizeof(float) * buffer_size);
#ifdef ISRRAN_NPSS_ACCUMULATE_ABS
    q->conv_output_abs = isrran_vec_f_malloc(buffer_size);
    if (!q->conv_output_abs) {
      fprintf(stderr, "Error allocating memory\n");
      goto clean_and_exit;
    }
    memset(q->conv_output_abs, 0, sizeof(float) * buffer_size);
#endif

    q->npss_signal_time = isrran_vec_cf_malloc(buffer_size);
    if (!q->npss_signal_time) {
      fprintf(stderr, "Error allocating memory\n");
      goto clean_and_exit;
    }
    memset(q->npss_signal_time, 0, sizeof(cf_t) * buffer_size);

    // The NPSS is translated into the time domain
    if (isrran_npss_corr_init(q->npss_signal_time, fft_size, q->frame_size)) {
      fprintf(stderr, "Error initiating NPSS detector for fft_size=%d\n", fft_size);
      goto clean_and_exit;
    }

#ifdef CONVOLUTION_FFT
    if (isrran_conv_fft_cc_init(&q->conv_fft, frame_size, ISRRAN_NPSS_CORR_FILTER_LEN)) {
      fprintf(stderr, "Error initiating convolution FFT\n");
      goto clean_and_exit;
    }
    // run convolution once to compute filter
    isrran_corr_fft_cc_run(&q->conv_fft, q->tmp_input, q->npss_signal_time, q->conv_output);
#endif

    isrran_npss_synch_reset(q);

    ret = ISRRAN_SUCCESS;
  }

clean_and_exit:
  if (ret == ISRRAN_ERROR) {
    isrran_npss_synch_free(q);
  }
  return ret;
}

int isrran_npss_synch_resize(isrran_npss_synch_t* q, uint32_t frame_size, uint32_t fft_size)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q != NULL) {
    ret = ISRRAN_ERROR;

    if (fft_size > q->max_fft_size || frame_size > q->max_frame_size) {
      PRINT_ERR("fft_size and frame_size must be lower than initialized\n");
      return ISRRAN_ERROR;
    }

    q->ema_alpha  = 0.2;
    q->fft_size   = fft_size;
    q->frame_size = frame_size;

    uint32_t buffer_size = ISRRAN_NPSS_CORR_FILTER_LEN + frame_size + 1;
    memset(q->tmp_input, 0, buffer_size * sizeof(cf_t));
    memset(q->conv_output, 0, sizeof(cf_t) * buffer_size);
    memset(q->conv_output_avg, 0, sizeof(float) * buffer_size);

#ifdef ISRRAN_NPSS_ACCUMULATE_ABS
    memset(q->conv_output_abs, 0, sizeof(float) * buffer_size);
#endif

    // Re-generate NPSS sequences for this FFT size
    if (isrran_npss_corr_init(q->npss_signal_time, fft_size, q->frame_size)) {
      fprintf(stderr, "Error initiating NPSS detector for fft_size=%d\n", fft_size);
      return ISRRAN_ERROR;
    }

#ifdef CONVOLUTION_FFT
    if (isrran_conv_fft_cc_replan(&q->conv_fft, frame_size, ISRRAN_NPSS_CORR_FILTER_LEN)) {
      fprintf(stderr, "Error initiating convolution FFT\n");
      return ISRRAN_ERROR;
    }
#endif

    isrran_npss_synch_reset(q);

    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

int isrran_npss_corr_init(cf_t* npss_signal_time, const uint32_t fft_size, const uint32_t frame_size)
{
  isrran_dft_plan_t plan;
  _Complex float    npss_signal_pad[fft_size];
  _Complex float    npss_signal[ISRRAN_NPSS_TOT_LEN] = {};

  // generate correlation sequence
  isrran_npss_generate(npss_signal);
#if DUMP_SIGNALS
  // isrran_vec_save_file("npss_corr_seq_freq.bin", npss_signal, ISRRAN_NPSS_TOT_LEN*sizeof(cf_t));
#endif

  // zero buffers
  memset(npss_signal_time, 0, (fft_size + frame_size) * sizeof(cf_t));
  memset(npss_signal_pad, 0, fft_size * sizeof(cf_t));

  // construct dft plan and convert signal into the time domain
  if (isrran_dft_plan(&plan, fft_size, ISRRAN_DFT_BACKWARD, ISRRAN_DFT_COMPLEX)) {
    return ISRRAN_ERROR;
  }
  isrran_dft_plan_set_mirror(&plan, true);
  isrran_dft_plan_set_dc(&plan, false);
  isrran_dft_plan_set_norm(&plan, true);

  // one symbol at a time
  cf_t* output     = npss_signal_time;
  int   output_len = 0;
  for (int i = 0; i < ISRRAN_NPSS_NUM_OFDM_SYMS; i++) {
    // zero buffer, copy NPSS symbol to appr. pos and transform to time-domain
    memset(npss_signal_pad, 0, fft_size * sizeof(cf_t));

    // 5th NPSS symbol has CP length of 10 symbols
    int cp_len = (i != 4) ? ISRRAN_CP_LEN_NORM(1, ISRRAN_NBIOT_FFT_SIZE) : ISRRAN_CP_LEN_NORM(0, ISRRAN_NBIOT_FFT_SIZE);
    int k      = (fft_size - ISRRAN_NRE) / 2; // place NPSS in the centre

    memcpy(&npss_signal_pad[k], &npss_signal[i * ISRRAN_NPSS_LEN], ISRRAN_NPSS_LEN * sizeof(cf_t));
    isrran_dft_run_c(&plan, npss_signal_pad, &output[cp_len]);

    // add CP
    memcpy(output, &output[fft_size], cp_len * sizeof(cf_t));

    // prepare next iteration
    output += fft_size + cp_len;
    output_len += fft_size + cp_len;
  }

  assert(output_len == ISRRAN_NPSS_CORR_FILTER_LEN);

#if DO_FREQ_SHIFT
  // shift entire signal in frequency domain by half a subcarrier
  cf_t  shift_buffer[ISRRAN_SF_LEN(fft_size)];
  cf_t* ptr = shift_buffer;
  for (uint32_t n = 0; n < 2; n++) {
    for (uint32_t i = 0; i < 7; i++) {
      uint32_t cplen = ISRRAN_CP_LEN_NORM(i, fft_size);
      for (uint32_t t = 0; t < fft_size + cplen; t++) {
        ptr[t] = cexpf(I * 2 * M_PI * ((float)t - (float)cplen) * -ISRRAN_NBIOT_FREQ_SHIFT_FACTOR / fft_size);
      }
      ptr += fft_size + cplen;
    }
  }
  isrran_vec_prod_ccc(
      npss_signal_time, &shift_buffer[ISRRAN_NPSS_CORR_OFFSET], npss_signal_time, ISRRAN_NPSS_CORR_FILTER_LEN);
  isrran_vec_sc_prod_cfc(npss_signal_time, 1.0 / 3, npss_signal_time, output_len);
#endif

  isrran_dft_plan_free(&plan);

  return ISRRAN_SUCCESS;
}

/** Performs time-domain NPSS correlation.
 * Returns the index of the NPSS correlation peak in a subframe.
 * The frame starts at corr_peak_pos-ISRRAN_NPSS_CORR_OFFSET+frame_size/2.
 * The value of the correlation is stored in corr_peak_value.
 *
 * Input buffer must be subframe_size long.
 */
int isrran_npss_sync_find(isrran_npss_synch_t* q, cf_t* input, float* corr_peak_value)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && input != NULL) {
    uint32_t corr_peak_pos;
    uint32_t conv_output_len;

    // Correlate input with NPSS sequence
    if (q->frame_size >= q->fft_size) {
#ifdef CONVOLUTION_FFT
      memcpy(q->tmp_input, input, q->frame_size * sizeof(cf_t));
      conv_output_len = isrran_corr_fft_cc_run_opt(&q->conv_fft, q->tmp_input, q->npss_signal_time, q->conv_output);
#else
      conv_output_len =
          isrran_conv_cc(input, q->pss_signal_time[q->N_id_2], q->conv_output, q->frame_size, q->fft_size);
#endif
    } else {
      for (int i = 0; i < q->frame_size; i++) {
        q->conv_output[i] = isrran_vec_dot_prod_ccc(q->npss_signal_time, &input[i], q->fft_size);
      }
      conv_output_len = q->frame_size;
    }

#ifdef ISRRAN_NPSS_ABS_SQUARE
    isrran_vec_abs_square_cf(q->conv_output, q->conv_output_abs, conv_output_len - 1);
#else
    isrran_vec_abs_cf(q->conv_output, q->conv_output_abs, conv_output_len - 1);
#endif

    if (q->ema_alpha < 1.0 && q->ema_alpha > 0.0) {
      isrran_vec_sc_prod_fff(q->conv_output_abs, q->ema_alpha, q->conv_output_abs, conv_output_len - 1);
      isrran_vec_sc_prod_fff(q->conv_output_avg, 1 - q->ema_alpha, q->conv_output_avg, conv_output_len - 1);

      isrran_vec_sum_fff(q->conv_output_abs, q->conv_output_avg, q->conv_output_avg, conv_output_len - 1);
    } else {
      memcpy(q->conv_output_avg, q->conv_output_abs, sizeof(float) * (conv_output_len - 1));
    }

    // Find maximum of the absolute value of the correlation
    corr_peak_pos = isrran_vec_max_fi(q->conv_output_avg, conv_output_len - 1);

#if DUMP_SIGNALS
    printf("Dumping debug signals.\n");
    isrran_vec_save_file("npss_find_input.bin", input, q->frame_size * sizeof(cf_t));
    isrran_vec_save_file("npss_corr_seq_time.bin", q->npss_signal_time, ISRRAN_NPSS_CORR_FILTER_LEN * sizeof(cf_t));
    isrran_vec_save_file("npss_find_conv_output_abs.bin", q->conv_output_abs, conv_output_len * sizeof(float));
    isrran_vec_save_file("npss_find_conv_output_avg.bin", q->conv_output_avg, conv_output_len * sizeof(float));
#endif

    // save absolute value
    q->peak_value = q->conv_output_avg[corr_peak_pos];

#ifdef ISRRAN_NPSS_RETURN_PSR
    // Find second side lobe

    // Find end of peak lobe to the right
    int pl_ub = corr_peak_pos + 1;
    while (q->conv_output_avg[pl_ub + 1] <= q->conv_output_avg[pl_ub] && pl_ub < conv_output_len) {
      pl_ub++;
    }
    // Find end of peak lobe to the left
    int pl_lb;
    if (corr_peak_pos > 2) {
      pl_lb = corr_peak_pos - 1;
      while (q->conv_output_avg[pl_lb - 1] <= q->conv_output_avg[pl_lb] && pl_lb > 1) {
        pl_lb--;
      }
    } else {
      pl_lb = 0;
    }

    int sl_distance_right = conv_output_len - 1 - pl_ub;
    if (sl_distance_right < 0) {
      sl_distance_right = 0;
    }
    int sl_distance_left = pl_lb;

    int   sl_right        = pl_ub + isrran_vec_max_fi(&q->conv_output_avg[pl_ub], sl_distance_right);
    int   sl_left         = isrran_vec_max_fi(q->conv_output_avg, sl_distance_left);
    float side_lobe_value = ISRRAN_MAX(q->conv_output_avg[sl_right], q->conv_output_avg[sl_left]);
    if (corr_peak_value) {
      *corr_peak_value = q->conv_output_avg[corr_peak_pos] / side_lobe_value;

      if (*corr_peak_value < 10) {
        DEBUG("peak_pos=%2d, pl_ub=%2d, pl_lb=%2d, sl_right: %2d, sl_left: %2d, PSR: %.2f/%.2f=%.2f",
              corr_peak_pos,
              pl_ub,
              pl_lb,
              sl_right,
              sl_left,
              q->conv_output_avg[corr_peak_pos],
              side_lobe_value,
              *corr_peak_value);
      }
    }
#else
    if (corr_peak_value) {
      *corr_peak_value = q->conv_output_avg[corr_peak_pos];
    }
#endif

    ret = (int)corr_peak_pos;
  }
  return ret;
}

void isrran_npss_synch_set_ema_alpha(isrran_npss_synch_t* q, float alpha)
{
  q->ema_alpha = alpha;
}

void isrran_npss_synch_free(isrran_npss_synch_t* q)
{
  if (q) {
    if (q->npss_signal_time) {
      free(q->npss_signal_time);
    }
#ifdef CONVOLUTION_FFT
    isrran_conv_fft_cc_free(&q->conv_fft);
#endif
    if (q->tmp_input) {
      free(q->tmp_input);
    }
    if (q->conv_output) {
      free(q->conv_output);
    }
    if (q->conv_output_abs) {
      free(q->conv_output_abs);
    }
    if (q->conv_output_avg) {
      free(q->conv_output_avg);
    }
  }
}

void isrran_npss_synch_reset(isrran_npss_synch_t* q)
{
  if (q->conv_output_avg) {
    uint32_t buffer_size = ISRRAN_NPSS_CORR_FILTER_LEN + q->max_frame_size + 1;
    memset(q->conv_output_avg, 0, sizeof(float) * buffer_size);
  }
}

/**
 * This function calculates the Zadoff-Chu sequence.
 * 36.211 13.2.0 section 10.2.7.1.1
 *
 * It produces ISRRAN_NPSS_LEN * ISRRAN_NPSS_NUM_SC = 11 * 11 = 121 samples.
 * @param signal Output array.
 */
int isrran_npss_generate(cf_t* signal)
{
  float       arg;
  const float root_value = 5.0;

  int sign = -1;
  int l    = 0;
  int n    = 0;

  // iterate over symbol indices
  for (l = 0; l < ISRRAN_CP_NORM_SF_NSYMB - 3; l++) {
    // iterate over subcarriers, leave out last one
    for (n = 0; n < ISRRAN_NRE - 1; n++) {
      arg                                      = (float)sign * M_PI * root_value * ((float)n * ((float)n + 1.0)) / 11.0;
      __real__ signal[l * ISRRAN_NPSS_LEN + n] = cosf(arg);
      __imag__ signal[l * ISRRAN_NPSS_LEN + n] = sinf(arg);

      signal[l * ISRRAN_NPSS_LEN + n] *= factor_lut[l];
    }
  }

  return 0;
}

/** 36.211 10.3 section 6.11.1.2
 */
void isrran_npss_put_subframe(isrran_npss_synch_t* q,
                              cf_t*                npss_signal,
                              cf_t*                sf,
                              const uint32_t       nof_prb,
                              const uint32_t       nbiot_prb_offset)
{
  // skip first 3 OFDM symbols over all PRBs completely
  uint32_t k = 3 * nof_prb * ISRRAN_NRE + nbiot_prb_offset * ISRRAN_NRE;

  // put NPSS in each of the 11 symbols of the subframe
  for (int l = 0; l < ISRRAN_CP_NORM_SF_NSYMB - 3; l++) {
    memcpy(&sf[k + ISRRAN_NPSS_LEN * l], &npss_signal[ISRRAN_NPSS_LEN * l], ISRRAN_NPSS_LEN * sizeof(cf_t));
    k += (nof_prb - 1) * ISRRAN_NRE + 1; // last SC of the PRB is also null
  }
}
