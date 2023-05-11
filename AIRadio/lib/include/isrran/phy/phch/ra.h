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
 *  File:         ra.h
 *
 *  Description:  Implements Resource allocation Procedures common in for DL and UL
 *
 *  Reference:    3GPP TS 36.213 version 10.0.1 Release 10
 *****************************************************************************/

#ifndef ISRRAN_RA_H
#define ISRRAN_RA_H

#include <stdbool.h>
#include <stdint.h>

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"

/**************************************************
 * Common structures used for Resource Allocation
 **************************************************/

typedef struct ISRRAN_API {
  isrran_mod_t mod;
  int          tbs;
  int          rv;
  uint32_t     nof_bits;
  uint32_t     cw_idx;
  bool         enabled;

  // this is for debugging and metrics purposes
  uint32_t mcs_idx;
} isrran_ra_tb_t;

typedef enum ISRRAN_API {
  ISRRAN_RA_ALLOC_TYPE0 = 0,
  ISRRAN_RA_ALLOC_TYPE1 = 1,
  ISRRAN_RA_ALLOC_TYPE2 = 2
} isrran_ra_type_t;

typedef struct ISRRAN_API {
  uint32_t rbg_bitmask;
} isrran_ra_type0_t;

typedef struct ISRRAN_API {
  uint32_t vrb_bitmask;
  uint32_t rbg_subset;
  bool     shift;
} isrran_ra_type1_t;

typedef struct ISRRAN_API {
  uint32_t riv; // if L_crb==0, DCI message packer will take this value directly
  enum { ISRRAN_RA_TYPE2_NPRB1A_2 = 0, ISRRAN_RA_TYPE2_NPRB1A_3 = 1 } n_prb1a;
  enum { ISRRAN_RA_TYPE2_NG1 = 0, ISRRAN_RA_TYPE2_NG2 = 1 } n_gap;
  enum { ISRRAN_RA_TYPE2_LOC = 0, ISRRAN_RA_TYPE2_DIST = 1 } mode;
} isrran_ra_type2_t;

#define ISRRAN_RA_NOF_TBS_IDX 34

ISRRAN_API uint32_t isrran_ra_type0_P(uint32_t nof_prb);

ISRRAN_API uint32_t isrran_ra_type2_n_vrb_dl(uint32_t nof_prb, bool ngap_is_1);

ISRRAN_API uint32_t isrran_ra_type2_n_rb_step(uint32_t nof_prb);

ISRRAN_API uint32_t isrran_ra_type2_ngap(uint32_t nof_prb, bool ngap_is_1);

ISRRAN_API uint32_t isrran_ra_type1_N_rb(uint32_t nof_prb);

ISRRAN_API uint32_t isrran_ra_type2_to_riv(uint32_t L_crb, uint32_t RB_start, uint32_t nof_prb);

ISRRAN_API void
isrran_ra_type2_from_riv(uint32_t riv, uint32_t* L_crb, uint32_t* RB_start, uint32_t nof_prb, uint32_t nof_vrb);

ISRRAN_API int isrran_ra_tbs_idx_from_mcs(uint32_t mcs, bool use_tbs_index_alt, bool is_ul);

ISRRAN_API isrran_mod_t isrran_ra_dl_mod_from_mcs(uint32_t mcs, bool use_tbs_index_alt);

ISRRAN_API isrran_mod_t isrran_ra_ul_mod_from_mcs(uint32_t mcs);

ISRRAN_API int isrran_ra_mcs_from_tbs_idx(uint32_t tbs_idx, bool use_tbs_index_alt, bool is_ul);

ISRRAN_API int isrran_ra_tbs_from_idx(uint32_t tbs_idx, uint32_t n_prb);

ISRRAN_API int isrran_ra_tbs_to_table_idx(uint32_t tbs, uint32_t n_prb, uint32_t max_tbs_idx);

#endif // ISRRAN_RA_H
