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
 *  File:         resampler.h
 *
 *  Description:  Linear and vector interpolation
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_RESAMPLER_H
#define ISRRAN_RESAMPLER_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/dft/dft.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Resampler operating modes
 */
typedef enum {
  ISRRAN_RESAMPLER_MODE_INTERPOLATE = 0,
  ISRRAN_RESAMPLER_MODE_DECIMATE,
} isrran_resampler_mode_t;

/**
 * @brief Resampler internal buffers and subcomponents
 */
typedef struct {
  isrran_resampler_mode_t mode;       ///< Interpolate or decimate mode
  uint32_t                ratio;      ///< Decimation/Interpolation ratio
  uint32_t                window_sz;  ///< Maximum number of processed samples
  uint32_t                delay;      ///< Filter delay in samples
  isrran_dft_plan_t       fft;        ///< Forward DFT
  isrran_dft_plan_t       ifft;       ///< Backward DFT
  uint32_t                state_len;  ///< Number of acccumulated samples in the internal state
  cf_t*                   in_buffer;  ///< DFT input buffer
  cf_t*                   out_buffer; ///< DFT output buffer
  cf_t*                   state;      ///< Filter state
  cf_t*                   filter;     ///< Frequency domain filter
} isrran_resampler_fft_t;

/**
 * Initialise an FFT based resampler which can be configured as decimator or interpolator.
 * @param q Object pointer
 * @param mode Determines whether the operation mode is decimation or interpolation
 * @param ratio Operational ratio
 * @return ISRRAN_SUCCES if no error, otherwise an ISRRAN error code
 */
ISRRAN_API int isrran_resampler_fft_init(isrran_resampler_fft_t* q, isrran_resampler_mode_t mode, uint32_t ratio);

/**
 * @brief resets internal re-sampler state
 * @param q Object pointer
 */
ISRRAN_API void isrran_resampler_fft_reset_state(isrran_resampler_fft_t* q);

/**
 * Get delay from the FFT based resampler.
 * @param q Object pointer
 * @return the delay in number of samples
 */
ISRRAN_API uint32_t isrran_resampler_fft_get_delay(isrran_resampler_fft_t* q);

/**
 * @brief Run FFT based resampler in the initiated mode.
 *
 * @note Setting the input to NULL is equivalent of feeding zeroes
 * @note Setting the output to NULL is equivalent of dropping output samples
 *
 * @param q Object pointer, make sure it has been initialised
 * @param input Points at the input complex buffer
 * @param output Points at the output complex buffer
 * @param nsamples Number of samples to apply the processing
 */
ISRRAN_API void isrran_resampler_fft_run(isrran_resampler_fft_t* q, const cf_t* input, cf_t* output, uint32_t nsamples);

/**
 * Free FFT based resampler buffers and subcomponents
 * @param q  Object pointer
 */
ISRRAN_API void isrran_resampler_fft_free(isrran_resampler_fft_t* q);

#ifdef __cplusplus
}
#endif

#endif // ISRRAN_RESAMPLER_H
