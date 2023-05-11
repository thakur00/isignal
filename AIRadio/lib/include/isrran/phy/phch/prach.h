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
 *  File:         prach.h
 *
 *  Description:  Physical random access channel.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 5.7
 *****************************************************************************/

#ifndef ISRRAN_PRACH_H
#define ISRRAN_PRACH_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/dft/dft.h"
#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ISRRAN_PRACH_MAX_LEN (2 * 24576 + 21024) // Maximum Tcp + Tseq

// Long PRACH ZC sequence sequence length
#define ISRRAN_PRACH_N_ZC_LONG 839

// Short PRACH ZC sequence sequence length
#define ISRRAN_PRACH_N_ZC_SHORT 139

/** Generation and detection of RACH signals for uplink.
 *  Currently only supports preamble formats 0-3.
 *  Does not currently support high speed flag.
 *  Based on 3GPP TS 36.211 version 10.7.0 Release 10.
 */

typedef struct {
  int   idx;
  float factor;
  cf_t  phase_array[2 * ISRRAN_PRACH_N_ZC_LONG];
} isrran_prach_cancellation_t;

typedef struct ISRRAN_API {
  // Parameters from higher layers (extracted from SIB2)
  bool     is_nr;
  uint32_t config_idx;
  uint32_t f;            // preamble format
  uint32_t rsi;          // rootSequenceIndex
  bool     hs;           // highSpeedFlag
  uint32_t zczc;         // zeroCorrelationZoneConfig
  uint32_t N_ifft_ul;    // IFFT size for uplink
  uint32_t N_ifft_prach; // IFFT size for PRACH generation

  uint32_t max_N_ifft_ul;

  // Working parameters
  uint32_t N_zc;  // PRACH sequence length
  uint32_t N_cs;  // Cyclic shift size
  uint32_t N_seq; // Preamble length
  float    T_seq; // Preamble length in seconds
  float    T_tot; // Total sequence length in seconds
  uint32_t N_cp;  // Cyclic prefix length

  // Generated tables
  cf_t     seqs[64][ISRRAN_PRACH_N_ZC_LONG];     // Our set of 64 preamble sequences
  cf_t     dft_seqs[64][ISRRAN_PRACH_N_ZC_LONG]; // DFT-precoded seqs
  uint64_t dft_gen_bitmap;    // Bitmap where each bit Indicates if the dft has been generated for sequence i.
  uint32_t root_seqs_idx[64]; // Indices of root seqs in seqs table
  uint32_t N_roots;           // Number of root sequences used in this configuration
  cf_t*    td_signals[64];
  // Containers
  cf_t*  ifft_in;
  cf_t*  ifft_out;
  cf_t*  prach_bins;
  cf_t*  corr_spec;
  float* corr;

  // PRACH IFFT
  isrran_dft_plan_t fft;
  isrran_dft_plan_t ifft;

  // ZC-sequence FFT and IFFT
  isrran_dft_plan_t zc_fft;
  isrran_dft_plan_t zc_ifft;

  cf_t* signal_fft;
  float detect_factor;

  uint32_t                    deadzone;
  float                       peak_values[65];
  uint32_t                    peak_offsets[65];
  uint32_t                    num_ra_preambles;
  bool                        successive_cancellation;
  bool                        freq_domain_offset_calc;
  isrran_tdd_config_t         tdd_config;
  uint32_t                    current_prach_idx;
  cf_t*                       cross;
  cf_t*                       corr_freq;
  isrran_prach_cancellation_t prach_cancel;
  cf_t                        sub[839 * 2];
  float                       phase[839];

} isrran_prach_t;

typedef struct ISRRAN_API {
  int      nof_sf;
  uint32_t sf[5];
} isrran_prach_sf_config_t;

///@brief Maximum number of subframe number candidates for PRACH NR configuration
#define PRACH_NR_CFG_MAX_NOF_SF 10

/**
 * @brief PRACH configuration for NR as described in TS 38.211 Tables 6.3.3.2-2, 6.3.3.2-3 and 6.3.3.2-4
 */
typedef struct {
  uint32_t preamble_format;
  uint32_t x;
  uint32_t y;
  uint32_t subframe_number[PRACH_NR_CFG_MAX_NOF_SF];
  uint32_t nof_subframe_number;
  uint32_t starting_symbol; // subframe number
} prach_nr_config_t;

typedef enum ISRRAN_API {
  ISRRAN_PRACH_SFN_EVEN = 0,
  ISRRAN_PRACH_SFN_ANY,
} isrran_prach_sfn_t;

typedef struct {
  bool                is_nr; // Set to true if NR
  uint32_t            config_idx;
  uint32_t            root_seq_idx;
  uint32_t            zero_corr_zone;
  uint32_t            freq_offset;
  uint32_t            num_ra_preambles;
  bool                hs_flag;
  isrran_tdd_config_t tdd_config;
  bool                enable_successive_cancellation;
  bool                enable_freq_domain_offset_calc;
} isrran_prach_cfg_t;

typedef struct ISRRAN_API {
  uint32_t f;
  uint32_t t0;
  uint32_t t1;
  uint32_t t2;
} isrran_prach_tdd_loc_t;

typedef struct ISRRAN_API {
  uint32_t               nof_elems;
  isrran_prach_tdd_loc_t elems[6];
} isrran_prach_tdd_loc_table_t;

ISRRAN_API uint32_t isrran_prach_get_preamble_format(uint32_t config_idx);

ISRRAN_API isrran_prach_sfn_t isrran_prach_get_sfn(uint32_t config_idx);

ISRRAN_API bool isrran_prach_tti_opportunity(isrran_prach_t* p, uint32_t current_tti, int allowed_subframe);

ISRRAN_API bool
isrran_prach_tti_opportunity_config_fdd(uint32_t config_idx, uint32_t current_tti, int allowed_subframe);

ISRRAN_API bool isrran_prach_in_window_config_fdd(uint32_t config_idx, uint32_t current_tti, int allowed_subframe);

ISRRAN_API bool isrran_prach_tti_opportunity_config_tdd(uint32_t  config_idx,
                                                        uint32_t  tdd_ul_dl_config,
                                                        uint32_t  current_tti,
                                                        uint32_t* prach_idx);

ISRRAN_API const prach_nr_config_t* isrran_prach_nr_get_cfg_fr1_paired(uint32_t config_idx);

ISRRAN_API bool isrran_prach_nr_tti_opportunity_fr1_paired(uint32_t config_idx, uint32_t current_tti);

ISRRAN_API uint32_t isrran_prach_nr_start_symbol_fr1_paired(uint32_t config_idx);

ISRRAN_API const prach_nr_config_t* isrran_prach_nr_get_cfg_fr1_unpaired(uint32_t config_idx);

ISRRAN_API bool isrran_prach_nr_tti_opportunity_fr1_unpaired(uint32_t config_idx, uint32_t current_tti);

ISRRAN_API uint32_t isrran_prach_nr_start_symbol(uint32_t config_idx, isrran_duplex_mode_t duplex_mode);

ISRRAN_API uint32_t isrran_prach_f_ra_tdd(uint32_t config_idx,
                                          uint32_t tdd_ul_dl_config,
                                          uint32_t current_tti,
                                          uint32_t prach_idx,
                                          uint32_t prach_offset,
                                          uint32_t n_rb_ul);

ISRRAN_API uint32_t isrran_prach_f_id_tdd(uint32_t config_idx, uint32_t tdd_ul_dl_config, uint32_t prach_idx);

ISRRAN_API uint32_t isrran_prach_nof_f_idx_tdd(uint32_t config_idx, uint32_t tdd_ul_dl_config);

ISRRAN_API void isrran_prach_sf_config(uint32_t config_idx, isrran_prach_sf_config_t* sf_config);

ISRRAN_API int isrran_prach_init(isrran_prach_t* p, uint32_t max_N_ifft_ul);

ISRRAN_API int isrran_prach_set_cfg(isrran_prach_t* p, isrran_prach_cfg_t* cfg, uint32_t nof_prb);

ISRRAN_API int isrran_prach_gen(isrran_prach_t* p, uint32_t seq_index, uint32_t freq_offset, cf_t* signal);

ISRRAN_API int isrran_prach_detect(isrran_prach_t* p,
                                   uint32_t        freq_offset,
                                   cf_t*           signal,
                                   uint32_t        sig_len,
                                   uint32_t*       indices,
                                   uint32_t*       ind_len);

ISRRAN_API int isrran_prach_detect_offset(isrran_prach_t* p,
                                          uint32_t        freq_offset,
                                          cf_t*           signal,
                                          uint32_t        sig_len,
                                          uint32_t*       indices,
                                          float*          t_offsets,
                                          float*          peak_to_avg,
                                          uint32_t*       ind_len);

ISRRAN_API void isrran_prach_set_detect_factor(isrran_prach_t* p, float factor);

ISRRAN_API int isrran_prach_free(isrran_prach_t* p);

ISRRAN_API int isrran_prach_print_seqs(isrran_prach_t* p);

ISRRAN_API int isrran_prach_process(isrran_prach_t* p,
                                    cf_t*           signal,
                                    uint32_t*       indices,
                                    float*          t_offsets,
                                    float*          peak_to_avg,
                                    uint32_t*       n_indices,
                                    int             cancellation_idx,
                                    uint32_t        begin,
                                    uint32_t        sig_len);

#endif // ISRRAN_PRACH_H
