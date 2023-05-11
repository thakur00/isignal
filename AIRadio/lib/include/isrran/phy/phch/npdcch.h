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

#ifndef ISRRAN_NPDCCH_H
#define ISRRAN_NPDCCH_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/fec/convolutional/convcoder.h"
#include "isrran/phy/fec/convolutional/rm_conv.h"
#include "isrran/phy/fec/convolutional/viterbi.h"
#include "isrran/phy/fec/crc.h"
#include "isrran/phy/mimo/layermap.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/dci.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/scrambling/scrambling.h"

#define ISRRAN_RARNTI_END_NBIOT 0x0100
#define ISRRAN_NBIOT_NUM_NRS_SYMS 8
#define ISRRAN_NPDCCH_MAX_RE (ISRRAN_NRE * ISRRAN_CP_NORM_SF_NSYMB - ISRRAN_NBIOT_NUM_NRS_SYMS)

#define ISRRAN_NBIOT_DCI_MAX_SIZE 23
#define ISRRAN_AL_REPETITION_USS 64 // Higher layer configured parameter al-Repetition-USS

typedef enum ISRRAN_API {
  ISRRAN_NPDCCH_FORMAT1 = 0,
  ISRRAN_NPDCCH_FORMAT0_LOWER_HALF,
  ISRRAN_NPDCCH_FORMAT0_UPPER_HALF,
  ISRRAN_NPDCCH_FORMAT_NITEMS
} isrran_npdcch_format_t;
static const char isrran_npdcch_format_text[ISRRAN_NPDCCH_FORMAT_NITEMS][30] = {"Format 1",
                                                                                "Format 0 (Lower Half)",
                                                                                "Format 0 (Upper Half)"};

/**
 * @brief Narrowband Physical downlink control channel (NPDCCH)
 *
 * Reference: 3GPP TS 36.211 version 13.2.0 Release 11 Sec. 6.8 and 10.2.5
 */
typedef struct ISRRAN_API {
  isrran_nbiot_cell_t cell;
  uint32_t            nof_cce;
  uint32_t            ncce_bits;
  uint32_t            max_bits;
  uint32_t            i_n_start;      /// start of the first OFDM symbol (signalled through NB-SIB1)
  uint32_t            nof_nbiot_refs; /// number of NRS symbols per OFDM symbol
  uint32_t            nof_lte_refs;   /// number of CRS symbols per OFDM symbol
  uint32_t            num_decoded_symbols;

  /* buffers */
  cf_t*    ce[ISRRAN_MAX_PORTS];
  cf_t*    symbols[ISRRAN_MAX_PORTS];
  cf_t*    x[ISRRAN_MAX_PORTS];
  cf_t*    d;
  uint8_t* e;
  float    rm_f[3 * (ISRRAN_DCI_MAX_BITS + 16)];
  float*   llr[2]; // Two layers of LLRs for Format0 and Format1 NPDCCH

  /* tx & rx objects */
  isrran_modem_table_t mod;
  isrran_sequence_t    seq[ISRRAN_NOF_SF_X_FRAME];
  isrran_viterbi_t     decoder;
  isrran_crc_t         crc;

} isrran_npdcch_t;

ISRRAN_API int isrran_npdcch_init(isrran_npdcch_t* q);

ISRRAN_API void isrran_npdcch_free(isrran_npdcch_t* q);

ISRRAN_API int isrran_npdcch_set_cell(isrran_npdcch_t* q, isrran_nbiot_cell_t cell);

/// Encoding function
ISRRAN_API int isrran_npdcch_encode(isrran_npdcch_t*      q,
                                    isrran_dci_msg_t*     msg,
                                    isrran_dci_location_t location,
                                    uint16_t              rnti,
                                    cf_t*                 sf_symbols[ISRRAN_MAX_PORTS],
                                    uint32_t              nsubframe);

/// Decoding functions: Extract the LLRs and save them in the isrran_npdcch_t object
ISRRAN_API int isrran_npdcch_extract_llr(isrran_npdcch_t* q,
                                         cf_t*            sf_symbols,
                                         cf_t*            ce[ISRRAN_MAX_PORTS],
                                         float            noise_estimate,
                                         uint32_t         sf_idx);

/// Decoding functions: Try to decode a DCI message after calling isrran_npdcch_extract_llr
ISRRAN_API int isrran_npdcch_decode_msg(isrran_npdcch_t*       q,
                                        isrran_dci_msg_t*      msg,
                                        isrran_dci_location_t* location,
                                        isrran_dci_format_t    format,
                                        uint16_t*              crc_rem);

ISRRAN_API int
isrran_npdcch_dci_decode(isrran_npdcch_t* q, float* e, uint8_t* data, uint32_t E, uint32_t nof_bits, uint16_t* crc);

ISRRAN_API int
isrran_npdcch_dci_encode(isrran_npdcch_t* q, uint8_t* data, uint8_t* e, uint32_t nof_bits, uint32_t E, uint16_t rnti);

ISRRAN_API void
isrran_npdcch_dci_encode_conv(isrran_npdcch_t* q, uint8_t* data, uint32_t nof_bits, uint8_t* coded_data, uint16_t rnti);

ISRRAN_API uint32_t isrran_npdcch_ue_locations(isrran_dci_location_t* c, uint32_t max_candidates);

ISRRAN_API uint32_t isrran_npdcch_common_locations(isrran_dci_location_t* c, uint32_t max_candidates);

int isrran_npdcch_cp(isrran_npdcch_t* q, cf_t* input, cf_t* output, bool put, isrran_npdcch_format_t format);
int isrran_npdcch_put(isrran_npdcch_t* q, cf_t* symbols, cf_t* sf_symbols, isrran_npdcch_format_t format);
int isrran_npdcch_get(isrran_npdcch_t* q, cf_t* symbols, cf_t* sf_symbols, isrran_npdcch_format_t format);

#endif // ISRRAN_NPDCCH_H
