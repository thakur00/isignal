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

#ifndef ISRRAN_WIENER_DL_H_
#define ISRRAN_WIENER_DL_H_

#include <isrran/config.h>
#include <isrran/phy/common/phy_common.h>
#include <isrran/phy/dft/dft.h>
#include <isrran/phy/utils/random.h>

// Constant static parameters
#define ISRRAN_WIENER_DL_HLS_FIFO_SIZE (8U)
#define ISRRAN_WIENER_DL_MIN_PRB (4U)
#define ISRRAN_WIENER_DL_MIN_RE (ISRRAN_WIENER_DL_MIN_PRB * ISRRAN_NRE)
#define ISRRAN_WIENER_DL_MIN_REF (ISRRAN_WIENER_DL_MIN_PRB * 2U)
#define ISRRAN_WIENER_DL_TFIFO_SIZE (2U)
#define ISRRAN_WIENER_DL_XFIFO_SIZE (400U)
#define ISRRAN_WIENER_DL_TIMEFIFO_SIZE (32U)
#define ISRRAN_WIENER_DL_CXFIFO_SIZE (400U)

typedef struct {
  cf_t*    hls_fifo_1[ISRRAN_WIENER_DL_HLS_FIFO_SIZE]; // Least square channel estimates on odd pilots
  cf_t*    hls_fifo_2[ISRRAN_WIENER_DL_HLS_FIFO_SIZE]; // Least square channel estimates on even pilots
  cf_t*    tfifo[ISRRAN_WIENER_DL_TFIFO_SIZE];         // memory for time domain channel linear interpolation
  cf_t*    xfifo[ISRRAN_WIENER_DL_XFIFO_SIZE];         // fifo for averaging the frequency correlation vectors
  cf_t     cV[ISRRAN_WIENER_DL_MIN_RE];                // frequency correlation vector among all subcarriers
  float    deltan;                                     // step within time domain linear interpolation
  uint32_t nfifosamps;   // number of samples inside the fifo for averaging the correlation vectors
  float    invtpilotoff; // step for time domain linear interpolation
  cf_t*    timefifo;     // fifo for storing single frequency channel time domain evolution
  cf_t*    cxfifo[ISRRAN_WIENER_DL_CXFIFO_SIZE]; // fifo for averaging time domain channel correlation vector
  uint32_t sumlen; // length of dynamic average window for time domain channel correlation vector
  uint32_t skip;   // pilot OFDM symbols to skip when training Wiener matrices (skip = 1,..,4)
  uint32_t cnt;    // counter for skipping pilot OFDM symbols
} isrran_wiener_dl_state_t;

typedef struct {
  // Maximum allocated number of...
  uint32_t max_prb;      // Resource Blocks
  uint32_t max_ref;      // Reference signals
  uint32_t max_re;       // Resource Elements (equivalent to sub-carriers)
  uint32_t max_tx_ports; // Tx Ports
  uint32_t max_rx_ant;   // Rx Antennas

  // Configured number of...
  uint32_t nof_prb;      // Resource Blocks
  uint32_t nof_ref;      // Reference signals
  uint32_t nof_re;       // Resource Elements (equivalent to sub-carriers)
  uint32_t nof_tx_ports; // Tx Ports
  uint32_t nof_rx_ant;   // Rx Antennas

  // One state per possible channel (allocated in init)
  isrran_wiener_dl_state_t* state[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];

  // Wiener matrices
  cf_t wm1[ISRRAN_WIENER_DL_MIN_RE][ISRRAN_WIENER_DL_MIN_REF];
  cf_t wm2[ISRRAN_WIENER_DL_MIN_RE][ISRRAN_WIENER_DL_MIN_REF];
  bool wm_computed;
  bool ready;

  // Calculation support
  cf_t hlsv[ISRRAN_WIENER_DL_MIN_RE];
  cf_t hlsv_sum[ISRRAN_WIENER_DL_MIN_RE];
  cf_t acV[ISRRAN_WIENER_DL_MIN_RE];

  union {
    cf_t m[ISRRAN_WIENER_DL_MIN_REF][ISRRAN_WIENER_DL_MIN_REF];
    cf_t v[ISRRAN_WIENER_DL_MIN_REF * ISRRAN_WIENER_DL_MIN_REF];
  } RH;
  union {
    cf_t m[ISRRAN_WIENER_DL_MIN_REF][ISRRAN_WIENER_DL_MIN_REF];
    cf_t v[ISRRAN_WIENER_DL_MIN_REF * ISRRAN_WIENER_DL_MIN_REF];
  } invRH;
  cf_t hH1[ISRRAN_WIENER_DL_MIN_RE][ISRRAN_WIENER_DL_MIN_REF];
  cf_t hH2[ISRRAN_WIENER_DL_MIN_RE][ISRRAN_WIENER_DL_MIN_REF];

  // Temporal vector
  cf_t* tmp;

  // Random generator
  isrran_random_t random;

  // FFT/iFFT
  isrran_dft_plan_t fft;
  isrran_dft_plan_t ifft;
  cf_t              filter[ISRRAN_WIENER_DL_MIN_RE];

  // Matrix inverter
  void* matrix_inverter;
} isrran_wiener_dl_t;

ISRRAN_API int
isrran_wiener_dl_init(isrran_wiener_dl_t* q, uint32_t max_prb, uint32_t max_tx_ports, uint32_t max_rx_ant);

ISRRAN_API int isrran_wiener_dl_set_cell(isrran_wiener_dl_t* q, isrran_cell_t cell);

ISRRAN_API void isrran_wiener_dl_reset(isrran_wiener_dl_t* q);

ISRRAN_API int isrran_wiener_dl_run(isrran_wiener_dl_t* q,
                                    uint32_t            tx,
                                    uint32_t            rx,
                                    uint32_t            m,
                                    uint32_t            shift,
                                    cf_t*               pilots,
                                    cf_t*               estimated,
                                    float               snr_lin);

ISRRAN_API void isrran_wiener_dl_free(isrran_wiener_dl_t* q);

#endif // ISRRAN_WIENER_DL_H_
