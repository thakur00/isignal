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

#ifndef ISRRAN_REFSIGNAL_DL_SYNC_H_
#define ISRRAN_REFSIGNAL_DL_SYNC_H_

#include <isrran/phy/ch_estimation/refsignal_dl.h>
#include <isrran/phy/dft/ofdm.h>
#include <isrran/phy/utils/convolution.h>

typedef struct {
  isrran_refsignal_t   refsignal;
  isrran_ofdm_t        ifft;
  cf_t*                ifft_buffer_in;
  cf_t*                ifft_buffer_out;
  cf_t*                sequences[ISRRAN_NOF_SF_X_FRAME];
  cf_t*                correlation;
  isrran_conv_fft_cc_t conv_fft_cc;

  // Results
  bool     found;
  float    rsrp_dBfs;
  float    rssi_dBfs;
  float    rsrq_dB;
  float    cfo_Hz;
  uint32_t peak_index;
} isrran_refsignal_dl_sync_t;

ISRRAN_API int isrran_refsignal_dl_sync_init(isrran_refsignal_dl_sync_t* q, isrran_cp_t cp);

ISRRAN_API int isrran_refsignal_dl_sync_set_cell(isrran_refsignal_dl_sync_t* q, isrran_cell_t cell);

ISRRAN_API void isrran_refsignal_dl_sync_free(isrran_refsignal_dl_sync_t* q);

ISRRAN_API int isrran_refsignal_dl_sync_run(isrran_refsignal_dl_sync_t* q, cf_t* buffer, uint32_t nsamples);

ISRRAN_API void isrran_refsignal_dl_sync_measure_sf(isrran_refsignal_dl_sync_t* q,
                                                    cf_t*                       buffer,
                                                    uint32_t                    sf_idx,
                                                    float*                      rsrp,
                                                    float*                      rssi,
                                                    float*                      cfo);

#endif // ISRRAN_REFSIGNAL_DL_SYNC_H_
