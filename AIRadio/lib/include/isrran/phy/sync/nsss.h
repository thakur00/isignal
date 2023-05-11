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
 *  File:         nsss.h
 *
 *  Description:  Narrowband secondary synchronization signal (NSSS)
 *                generation and detection.
 *
 *
 *  Reference:    3GPP TS 36.211 version 13.2.0 Release 13 Sec. 10.2.7.2
 *****************************************************************************/

#ifndef ISRRAN_NSSS_H
#define ISRRAN_NSSS_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/dft/dft.h"
#include "isrran/phy/utils/convolution.h"

#define ISRRAN_NSSS_NSYMB 11
#define ISRRAN_NSSS_NSC 12
#define ISRRAN_NSSS_LEN (ISRRAN_NSSS_NSYMB * ISRRAN_NSSS_NSC)
#define ISRRAN_NSSS_NUM_SEQ 4
#define ISRRAN_NSSS_TOT_LEN (ISRRAN_NSSS_NUM_SEQ * ISRRAN_NSSS_LEN)

#define ISRRAN_NSSS_CORR_FILTER_LEN 1508
#define ISRRAN_NSSS_CORR_OFFSET 412

#define ISRRAN_NSSS_PERIOD 2
#define ISRRAN_NSSS_NUM_SF_DETECT (ISRRAN_NSSS_PERIOD)

// b_q_m table from 3GPP TS 36.211 v13.2.0 table 10.2.7.2.1-1
static const int b_q_m[ISRRAN_NSSS_NUM_SEQ][128] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1,
     1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1,
     1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1,
     1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1},
    {1,  -1, -1, 1,  -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1,
     -1, 1,  -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,
     -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1,
     -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1,
     1,  -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,  -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1},
    {1,  -1, -1, 1,  -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1,
     -1, 1,  -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,
     -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1, -1, 1,  -1, 1,
     1,  -1, 1,  -1, -1, 1,  -1, 1,  1,  -1, -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1,  -1,
     -1, 1,  1,  -1, 1,  -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1,  -1}};

/* Low-level API */
typedef struct ISRRAN_API {
  uint32_t             input_size;
  uint32_t             subframe_sz;
  uint32_t             fft_size, max_fft_size;
  isrran_conv_fft_cc_t conv_fft;

  cf_t*  nsss_signal_time[ISRRAN_NUM_PCI];
  cf_t*  tmp_input;
  cf_t*  conv_output;
  float* conv_output_abs;
  float  peak_values[ISRRAN_NUM_PCI];
  float  corr_peak_threshold;
} isrran_nsss_synch_t;

ISRRAN_API int isrran_nsss_synch_init(isrran_nsss_synch_t* q, uint32_t input_size, uint32_t fft_size);

ISRRAN_API void isrran_nsss_synch_free(isrran_nsss_synch_t* q);

ISRRAN_API int isrran_nsss_synch_resize(isrran_nsss_synch_t* q, uint32_t fft_size);

ISRRAN_API int isrran_nsss_sync_find(isrran_nsss_synch_t* q,
                                     cf_t*                input,
                                     float*               corr_peak_value,
                                     uint32_t*            cell_id,
                                     uint32_t*            sfn_partial);

void isrran_nsss_sync_find_pci(isrran_nsss_synch_t* q, cf_t* input, uint32_t cell_id);

ISRRAN_API int isrran_nsss_corr_init(isrran_nsss_synch_t* q);

ISRRAN_API void isrran_nsss_generate(cf_t* signal, uint32_t cell_id);

ISRRAN_API void isrran_nsss_put_subframe(isrran_nsss_synch_t* q,
                                         cf_t*                nsss,
                                         cf_t*                subframe,
                                         const int            nf,
                                         const uint32_t       nof_prb,
                                         const uint32_t       nbiot_prb_offset);

#endif // ISRRAN_NSSS_H