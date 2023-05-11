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
 *  File:         turbodecoder.h
 *
 *  Description:  Turbo Decoder.
 *                Parallel Concatenated Convolutional Code (PCCC) with two 8-state constituent
 *                encoders and one turbo code internal interleaver. The coding rate of turbo
 *                encoder is 1/3.
 *                MAP_GEN is the MAX-LOG-MAP generic implementation of the decoder.
 *
 *  Reference:    3GPP TS 36.212 version 10.0.0 Release 10 Sec. 5.1.3.2
 *********************************************************************************************/

#ifndef ISRRAN_TURBODECODER_H
#define ISRRAN_TURBODECODER_H

#include "isrran/config.h"
#include "isrran/phy/fec/cbsegm.h"
#include "isrran/phy/fec/turbo/tc_interl.h"

#define ISRRAN_TCOD_RATE 3
#define ISRRAN_TCOD_TOTALTAIL 12

#define ISRRAN_TCOD_MAX_LEN_CB 6144

// Expect the input to be aligned for sub-block window processing.
#define ISRRAN_TDEC_EXPECT_INPUT_SB 1

// Include interfaces for 8 and 16 bit decoder implementations
#define LLR_IS_8BIT
#include "isrran/phy/fec/turbo/turbodecoder_impl.h"
#undef LLR_IS_8BIT

#define LLR_IS_16BIT
#include "isrran/phy/fec/turbo/turbodecoder_impl.h"
#undef LLR_IS_16BIT

#define ISRRAN_TDEC_NOF_AUTO_MODES_8 2
#define ISRRAN_TDEC_NOF_AUTO_MODES_16 3

typedef enum { ISRRAN_TDEC_8, ISRRAN_TDEC_16 } isrran_tdec_llr_type_t;

typedef struct ISRRAN_API {
  uint32_t max_long_cb;

  void*                     dec8_hdlr[ISRRAN_TDEC_NOF_AUTO_MODES_8];
  void*                     dec16_hdlr[ISRRAN_TDEC_NOF_AUTO_MODES_16];
  isrran_tdec_8bit_impl_t*  dec8[ISRRAN_TDEC_NOF_AUTO_MODES_8];
  isrran_tdec_16bit_impl_t* dec16[ISRRAN_TDEC_NOF_AUTO_MODES_16];
  int                       nof_blocks8[ISRRAN_TDEC_NOF_AUTO_MODES_8];
  int                       nof_blocks16[ISRRAN_TDEC_NOF_AUTO_MODES_16];

  // Declare as void types as can be int8 or int16
  void* app1;
  void* app2;
  void* ext1;
  void* ext2;
  void* syst0;
  void* parity0;
  void* parity1;

  void* input_conv;

  bool force_not_sb;

  isrran_tdec_impl_type_t dec_type;

  isrran_tdec_llr_type_t current_llr_type;
  uint32_t               current_dec;
  uint32_t               current_long_cb;
  uint32_t               current_inter_idx;
  int                    current_cbidx;
  isrran_tc_interl_t     interleaver[4][ISRRAN_NOF_TC_CB_SIZES];
  int                    n_iter;
} isrran_tdec_t;

ISRRAN_API int isrran_tdec_init(isrran_tdec_t* h, uint32_t max_long_cb);

ISRRAN_API int isrran_tdec_init_manual(isrran_tdec_t* h, uint32_t max_long_cb, isrran_tdec_impl_type_t dec_type);

ISRRAN_API void isrran_tdec_free(isrran_tdec_t* h);

ISRRAN_API void isrran_tdec_force_not_sb(isrran_tdec_t* h);

ISRRAN_API int isrran_tdec_new_cb(isrran_tdec_t* h, uint32_t long_cb);

ISRRAN_API int isrran_tdec_get_nof_iterations(isrran_tdec_t* h);

ISRRAN_API uint32_t isrran_tdec_autoimp_get_subblocks(uint32_t long_cb);

ISRRAN_API uint32_t isrran_tdec_autoimp_get_subblocks_8bit(uint32_t long_cb);

ISRRAN_API void isrran_tdec_iteration(isrran_tdec_t* h, int16_t* input, uint8_t* output);

ISRRAN_API int
isrran_tdec_run_all(isrran_tdec_t* h, int16_t* input, uint8_t* output, uint32_t nof_iterations, uint32_t long_cb);

ISRRAN_API void isrran_tdec_iteration_8bit(isrran_tdec_t* h, int8_t* input, uint8_t* output);

ISRRAN_API int
isrran_tdec_run_all_8bit(isrran_tdec_t* h, int8_t* input, uint8_t* output, uint32_t nof_iterations, uint32_t long_cb);

#endif // ISRRAN_TURBODECODER_H
