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
 *  File:         regs.h
 *
 *  Description:  Resource element mapping functions.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10
 *****************************************************************************/

#ifndef ISRRAN_REGS_H
#define ISRRAN_REGS_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include <stdbool.h>

#define REGS_PHICH_NSYM 12
#define REGS_PHICH_REGS_X_GROUP 3

#define REGS_PCFICH_NSYM 16
#define REGS_PCFICH_NREGS 4

#define REGS_RE_X_REG 4

typedef struct ISRRAN_API {
  uint32_t k[4];
  uint32_t k0;
  uint32_t l;
  bool     assigned;
} isrran_regs_reg_t;

typedef struct ISRRAN_API {
  uint32_t            nof_regs;
  isrran_regs_reg_t** regs;
} isrran_regs_ch_t;

typedef struct ISRRAN_API {
  isrran_cell_t cell;
  uint32_t      max_ctrl_symbols;
  uint32_t      ngroups_phich;
  uint32_t      ngroups_phich_m1;

  isrran_phich_r_t      phich_res;
  isrran_phich_length_t phich_len;

  isrran_regs_ch_t  pcfich;
  isrran_regs_ch_t* phich;    // there are several phich
  isrran_regs_ch_t  pdcch[3]; /* PDCCH indexing, permutation and interleaving is computed for
             the three possible CFI value */

  uint32_t           phich_mi;
  uint32_t           nof_regs;
  isrran_regs_reg_t* regs;
} isrran_regs_t;

ISRRAN_API int isrran_regs_init(isrran_regs_t* h, isrran_cell_t cell);

ISRRAN_API int isrran_regs_init_opts(isrran_regs_t* h, isrran_cell_t cell, uint32_t phich_mi, bool mbsfn_or_sf1_6_tdd);

ISRRAN_API void isrran_regs_free(isrran_regs_t* h);

ISRRAN_API int isrran_regs_pdcch_nregs(isrran_regs_t* h, uint32_t cfi);

ISRRAN_API int isrran_regs_pdcch_ncce(isrran_regs_t* h, uint32_t cfi);

ISRRAN_API int isrran_regs_pcfich_put(isrran_regs_t* h, cf_t symbols[REGS_PCFICH_NSYM], cf_t* slot_symbols);

ISRRAN_API int isrran_regs_pcfich_get(isrran_regs_t* h, cf_t* slot_symbols, cf_t symbols[REGS_PCFICH_NSYM]);

ISRRAN_API uint32_t isrran_regs_phich_nregs(isrran_regs_t* h);

ISRRAN_API int
isrran_regs_phich_add(isrran_regs_t* h, cf_t symbols[REGS_PHICH_NSYM], uint32_t ngroup, cf_t* slot_symbols);

ISRRAN_API int
isrran_regs_phich_get(isrran_regs_t* h, cf_t* slot_symbols, cf_t symbols[REGS_PHICH_NSYM], uint32_t ngroup);

ISRRAN_API uint32_t isrran_regs_phich_ngroups(isrran_regs_t* h);

ISRRAN_API uint32_t isrran_regs_phich_ngroups_m1(isrran_regs_t* h);

ISRRAN_API int isrran_regs_phich_reset(isrran_regs_t* h, cf_t* slot_symbols);

ISRRAN_API int isrran_regs_pdcch_put(isrran_regs_t* h, uint32_t cfi, cf_t* d, cf_t* slot_symbols);

ISRRAN_API int isrran_regs_pdcch_put_offset(isrran_regs_t* h,
                                            uint32_t       cfi,
                                            cf_t*          d,
                                            cf_t*          slot_symbols,
                                            uint32_t       start_reg,
                                            uint32_t       nof_regs);

ISRRAN_API int isrran_regs_pdcch_get(isrran_regs_t* h, uint32_t cfi, cf_t* slot_symbols, cf_t* d);

ISRRAN_API int isrran_regs_pdcch_get_offset(isrran_regs_t* h,
                                            uint32_t       cfi,
                                            cf_t*          slot_symbols,
                                            cf_t*          d,
                                            uint32_t       start_reg,
                                            uint32_t       nof_regs);

#endif // ISRRAN_REGS_H
