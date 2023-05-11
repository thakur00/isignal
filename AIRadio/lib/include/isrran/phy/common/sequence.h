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
 *  File:         sequence.h
 *
 *  Description:  Pseudo Random Sequence generation. Sequences are defined by a length-31 Gold
 *                sequence.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 7.2
 *********************************************************************************************/

#ifndef ISRRAN_SEQUENCE_H
#define ISRRAN_SEQUENCE_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"

#define ISRRAN_SEQUENCE_MOD(X) ((uint32_t)((X) & (uint64_t)INT32_MAX))

typedef struct ISRRAN_API {
  uint32_t x1;
  uint32_t x2;
} isrran_sequence_state_t;

ISRRAN_API void isrran_sequence_state_init(isrran_sequence_state_t* s, uint32_t seed);

ISRRAN_API void isrran_sequence_state_gen_f(isrran_sequence_state_t* s, float value, float* out, uint32_t length);

ISRRAN_API void isrran_sequence_state_apply_f(isrran_sequence_state_t* s, const float* in, float* out, uint32_t length);

ISRRAN_API void
isrran_sequence_state_apply_c(isrran_sequence_state_t* s, const int8_t* in, int8_t* out, uint32_t length);

ISRRAN_API
void isrran_sequence_state_apply_bit(isrran_sequence_state_t* s, const uint8_t* in, uint8_t* out, uint32_t length);

ISRRAN_API void isrran_sequence_state_advance(isrran_sequence_state_t* s, uint32_t length);

typedef struct ISRRAN_API {
  uint8_t* c;
  uint8_t* c_bytes;
  float*   c_float;
  short*   c_short;
  int8_t*  c_char;
  uint32_t cur_len;
  uint32_t max_len;
} isrran_sequence_t;

ISRRAN_API int isrran_sequence_init(isrran_sequence_t* q, uint32_t len);

ISRRAN_API void isrran_sequence_free(isrran_sequence_t* q);

ISRRAN_API int isrran_sequence_LTE_pr(isrran_sequence_t* q, uint32_t len, uint32_t seed);

ISRRAN_API int isrran_sequence_set_LTE_pr(isrran_sequence_t* q, uint32_t len, uint32_t seed);

ISRRAN_API void isrran_sequence_apply_f(const float* in, float* out, uint32_t length, uint32_t seed);

ISRRAN_API void isrran_sequence_apply_s(const int16_t* in, int16_t* out, uint32_t length, uint32_t seed);

ISRRAN_API void isrran_sequence_apply_c(const int8_t* in, int8_t* out, uint32_t length, uint32_t seed);

ISRRAN_API void isrran_sequence_apply_packed(const uint8_t* in, uint8_t* out, uint32_t length, uint32_t seed);

ISRRAN_API void isrran_sequence_apply_bit(const uint8_t* in, uint8_t* out, uint32_t length, uint32_t seed);

ISRRAN_API int isrran_sequence_pbch(isrran_sequence_t* seq, isrran_cp_t cp, uint32_t cell_id);

ISRRAN_API int isrran_sequence_pcfich(isrran_sequence_t* seq, uint32_t nslot, uint32_t cell_id);

ISRRAN_API int isrran_sequence_phich(isrran_sequence_t* seq, uint32_t nslot, uint32_t cell_id);

ISRRAN_API int isrran_sequence_pdcch(isrran_sequence_t* seq, uint32_t nslot, uint32_t cell_id, uint32_t len);

ISRRAN_API int
isrran_sequence_pdsch(isrran_sequence_t* seq, uint16_t rnti, int q, uint32_t nslot, uint32_t cell_id, uint32_t len);

ISRRAN_API void isrran_sequence_pdsch_apply_pack(const uint8_t* in,
                                                 uint8_t*       out,
                                                 uint16_t       rnti,
                                                 int            q,
                                                 uint32_t       nslot,
                                                 uint32_t       cell_id,
                                                 uint32_t       len);

ISRRAN_API void isrran_sequence_pdsch_apply_f(const float* in,
                                              float*       out,
                                              uint16_t     rnti,
                                              int          q,
                                              uint32_t     nslot,
                                              uint32_t     cell_id,
                                              uint32_t     len);

ISRRAN_API void isrran_sequence_pdsch_apply_s(const int16_t* in,
                                              int16_t*       out,
                                              uint16_t       rnti,
                                              int            q,
                                              uint32_t       nslot,
                                              uint32_t       cell_id,
                                              uint32_t       len);

ISRRAN_API void isrran_sequence_pdsch_apply_c(const int8_t* in,
                                              int8_t*       out,
                                              uint16_t      rnti,
                                              int           q,
                                              uint32_t      nslot,
                                              uint32_t      cell_id,
                                              uint32_t      len);

ISRRAN_API int
isrran_sequence_pusch(isrran_sequence_t* seq, uint16_t rnti, uint32_t nslot, uint32_t cell_id, uint32_t len);

ISRRAN_API void isrran_sequence_pusch_apply_pack(const uint8_t* in,
                                                 uint8_t*       out,
                                                 uint16_t       rnti,
                                                 uint32_t       nslot,
                                                 uint32_t       cell_id,
                                                 uint32_t       len);

ISRRAN_API void isrran_sequence_pusch_apply_c(const int8_t* in,
                                              int8_t*       out,
                                              uint16_t      rnti,
                                              uint32_t      nslot,
                                              uint32_t      cell_id,
                                              uint32_t      len);

ISRRAN_API void isrran_sequence_pusch_apply_s(const int16_t* in,
                                              int16_t*       out,
                                              uint16_t       rnti,
                                              uint32_t       nslot,
                                              uint32_t       cell_id,
                                              uint32_t       len);

ISRRAN_API void
isrran_sequence_pusch_gen_unpack(uint8_t* out, uint16_t rnti, uint32_t nslot, uint32_t cell_id, uint32_t len);

ISRRAN_API int isrran_sequence_pucch(isrran_sequence_t* seq, uint16_t rnti, uint32_t nslot, uint32_t cell_id);

ISRRAN_API int isrran_sequence_pmch(isrran_sequence_t* seq, uint32_t nslot, uint32_t mbsfn_id, uint32_t len);

ISRRAN_API int isrran_sequence_npbch(isrran_sequence_t* seq, isrran_cp_t cp, uint32_t cell_id);

ISRRAN_API int isrran_sequence_npbch_r14(isrran_sequence_t* seq, uint32_t n_id_ncell, uint32_t nf);

ISRRAN_API int isrran_sequence_npdsch(isrran_sequence_t* seq,
                                      uint16_t           rnti,
                                      int                q,
                                      uint32_t           nf,
                                      uint32_t           nslot,
                                      uint32_t           cell_id,
                                      uint32_t           len);

ISRRAN_API int isrran_sequence_npdsch_bcch_r14(isrran_sequence_t* seq, uint32_t nf, uint32_t n_id_ncell, uint32_t len);

ISRRAN_API int isrran_sequence_npdcch(isrran_sequence_t* seq, uint32_t nslot, uint32_t cell_id, uint32_t len);

ISRRAN_API int isrran_sequence_npusch(isrran_sequence_t* seq,
                                      uint16_t           rnti,
                                      uint32_t           nf,
                                      uint32_t           nslot,
                                      uint32_t           cell_id,
                                      uint32_t           len);

ISRRAN_API int isrran_sequence_nprach(isrran_sequence_t* seq, uint32_t cell_id);

#endif // ISRRAN_SEQUENCE_H
