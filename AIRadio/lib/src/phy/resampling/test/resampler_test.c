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

#include "isrran/phy/channel/ch_awgn.h"
#include "isrran/phy/resampling/resampler.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <getopt.h>
#include <stdlib.h>
#include <sys/time.h>

static uint32_t buffer_size = 1920;
static uint32_t factor      = 2;
static uint32_t repetitions = 2;
static enum {
  WAVE_SINE = 0,
  WAVE_DELTA,
  WAVE_STEP,
  WAVE_GAUSS,
} wave = WAVE_SINE;

static void usage(char* prog)
{
  printf("Usage: %s [sfr]\n", prog);
  printf("\t-s Buffer size [Default %d]\n", buffer_size);
  printf("\t-f Interpolation/Decimation factor [Default %d]\n", factor);
  printf("\t-w Wave type: sine, step, delta [Default sine]\n");
  printf("\t-f r [Default %d]\n", repetitions);
}

static void parse_args(int argc, char** argv)
{
  int opt;

  while ((opt = getopt(argc, argv, "sfrvw")) != -1) {
    switch (opt) {
      case 's':
        buffer_size = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'f':
        factor = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'r':
        repetitions = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'v':
        increase_isrran_verbose_level();
        break;
      case 'w':
        if (strcmp(argv[optind], "sine") == 0) {
          wave = WAVE_SINE;
          break;
        }

        if (strcmp(argv[optind], "delta") == 0) {
          wave = WAVE_DELTA;
          break;
        }

        if (strcmp(argv[optind], "step") == 0) {
          wave = WAVE_STEP;
          break;
        }

        if (strcmp(argv[optind], "gauss") == 0) {
          wave = WAVE_GAUSS;
          break;
        }

        printf("Invalid wave '%s'\n", argv[optind]);
        usage(argv[0]);
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
}

int main(int argc, char** argv)
{
  struct timeval         t[3]   = {};
  isrran_resampler_fft_t interp = {};
  isrran_resampler_fft_t decim  = {};
  isrran_channel_awgn_t  awgn   = {};

  parse_args(argc, argv);

  cf_t* src          = isrran_vec_cf_malloc(buffer_size);
  cf_t* interpolated = isrran_vec_cf_malloc(buffer_size * factor);
  cf_t* decimated    = isrran_vec_cf_malloc(buffer_size);

  if (isrran_resampler_fft_init(&interp, ISRRAN_RESAMPLER_MODE_INTERPOLATE, factor)) {
    return ISRRAN_ERROR;
  }

  if (isrran_resampler_fft_init(&decim, ISRRAN_RESAMPLER_MODE_DECIMATE, factor)) {
    return ISRRAN_ERROR;
  }

  isrran_vec_cf_zero(src, buffer_size);

  switch (wave) {
    case WAVE_SINE:
      isrran_vec_gen_sine(1.0f, 0.01f, src, buffer_size / 2);
      break;
    case WAVE_DELTA:
      src[0] = 1.0f;
      break;
    case WAVE_STEP:
      for (uint32_t i = 0; i < buffer_size; i++) {
        src[i] = 1.0f;
      }
      break;
    case WAVE_GAUSS:
      isrran_channel_awgn_init(&awgn, 0);
      isrran_channel_awgn_set_n0(&awgn, 0);
      isrran_channel_awgn_run_c(&awgn, src, src, buffer_size);
      isrran_channel_awgn_free(&awgn);
      break;
  }

  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    printf("signal=");
    isrran_vec_fprint_c(stdout, src, buffer_size);
  }

  gettimeofday(&t[1], NULL);
  for (uint32_t r = 0; r < repetitions; r++) {
    isrran_resampler_fft_run(&interp, src, interpolated, buffer_size);
    isrran_resampler_fft_run(&decim, interpolated, decimated, buffer_size * factor);
  }
  gettimeofday(&t[2], NULL);
  get_time_interval(t);
  uint64_t duration_us = (uint64_t)(t[0].tv_sec * 1000000UL + t[0].tv_usec);

  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    printf("interp=");
    isrran_vec_fprint_c(stdout, interpolated, buffer_size * factor);
    printf("decim=");
    isrran_vec_fprint_c(stdout, decimated, buffer_size);
  }

  // Check error
  uint32_t delay    = (isrran_resampler_fft_get_delay(&decim) + isrran_resampler_fft_get_delay(&interp)) / factor;
  uint32_t nsamples = buffer_size - delay;
  isrran_vec_sub_ccc(src, &decimated[delay], interpolated, nsamples);
  float mse = sqrtf(isrran_vec_avg_power_cf(interpolated, nsamples));

  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    printf("recovered=");
    isrran_vec_fprint_c(stdout, &decimated[delay], nsamples);
  }

  printf("Done %.1f Msps; MSE: %.6f\n", factor * buffer_size * repetitions / (double)duration_us, mse);

  isrran_resampler_fft_free(&interp);
  isrran_resampler_fft_free(&decim);
  free(src);
  free(interpolated);
  free(decimated);

  return (mse < 0.1f) ? ISRRAN_SUCCESS : ISRRAN_ERROR;
}