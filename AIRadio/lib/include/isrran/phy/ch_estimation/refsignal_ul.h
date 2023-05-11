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
 *  File:         refsignal_ul.h
 *
 *  Description:  Object to manage uplink reference signals for channel estimation.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 5.5
 *********************************************************************************************/

#ifndef ISRRAN_REFSIGNAL_UL_H
#define ISRRAN_REFSIGNAL_UL_H

#include "isrran/config.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/phch/pucch_cfg.h"
#include "isrran/phy/phch/pusch_cfg.h"

#define ISRRAN_NOF_GROUPS_U 30
#define ISRRAN_NOF_SEQUENCES_U 2
#define ISRRAN_NOF_DELTA_SS 30
#define ISRRAN_NOF_CSHIFT 8

#define ISRRAN_REFSIGNAL_UL_L(ns_idx, cp) ((ns_idx + 1) * ISRRAN_CP_NSYMB(cp) - 4)

/* PUSCH DMRS common configuration (received in SIB2) */
typedef struct ISRRAN_API {
  uint32_t cyclic_shift;
  uint32_t delta_ss;
  bool     group_hopping_en;
  bool     sequence_hopping_en;
} isrran_refsignal_dmrs_pusch_cfg_t;

typedef struct ISRRAN_API {

  // Common Configuration
  uint32_t subframe_config;
  uint32_t bw_cfg;
  bool     simul_ack;

  // Dedicated configuration
  uint32_t B;
  uint32_t b_hop;
  uint32_t n_isr;
  uint32_t I_isr;
  uint32_t k_tc;
  uint32_t n_rrc;
  bool     dedicated_enabled;
  bool     common_enabled;
  bool     configured;
} isrran_refsignal_isr_cfg_t;

/** Uplink DeModulation Reference Signal (DMRS) */
typedef struct ISRRAN_API {
  isrran_cell_t cell;

  uint32_t n_cs_cell[ISRRAN_NSLOTS_X_FRAME][ISRRAN_CP_NORM_NSYMB];
  uint32_t n_prs_pusch[ISRRAN_NOF_DELTA_SS][ISRRAN_NSLOTS_X_FRAME]; // We precompute n_prs needed for cyclic shift alpha
                                                                    // at isrran_refsignal_dl_init()
  uint32_t f_gh[ISRRAN_NSLOTS_X_FRAME];
  uint32_t u_pucch[ISRRAN_NSLOTS_X_FRAME];
  uint32_t v_pusch[ISRRAN_NSLOTS_X_FRAME][ISRRAN_NOF_DELTA_SS];
} isrran_refsignal_ul_t;

typedef struct {
  uint32_t max_prb;
  cf_t** r[ISRRAN_NOF_CSHIFT][ISRRAN_NOF_SF_X_FRAME];
} isrran_refsignal_ul_dmrs_pregen_t;

typedef struct {
  cf_t* r[ISRRAN_NOF_SF_X_FRAME];
} isrran_refsignal_isr_pregen_t;

ISRRAN_API int isrran_refsignal_ul_set_cell(isrran_refsignal_ul_t* q, isrran_cell_t cell);

ISRRAN_API uint32_t isrran_refsignal_dmrs_N_rs(isrran_pucch_format_t format, isrran_cp_t cp);

ISRRAN_API uint32_t isrran_refsignal_dmrs_pucch_symbol(uint32_t m, isrran_pucch_format_t format, isrran_cp_t cp);

ISRRAN_API int isrran_refsignal_dmrs_pusch_pregen_init(isrran_refsignal_ul_dmrs_pregen_t* pregen, uint32_t max_prb);

ISRRAN_API int isrran_refsignal_dmrs_pusch_pregen(isrran_refsignal_ul_t*             q,
                                                  isrran_refsignal_ul_dmrs_pregen_t* pregen,
                                                  isrran_refsignal_dmrs_pusch_cfg_t* cfg);

ISRRAN_API void isrran_refsignal_dmrs_pusch_pregen_free(isrran_refsignal_ul_t*             q,
                                                        isrran_refsignal_ul_dmrs_pregen_t* pregen);

ISRRAN_API int isrran_refsignal_dmrs_pusch_pregen_put(isrran_refsignal_ul_t*             q,
                                                      isrran_ul_sf_cfg_t*                sf_cfg,
                                                      isrran_refsignal_ul_dmrs_pregen_t* pregen,
                                                      isrran_pusch_cfg_t*                pusch_cfg,
                                                      cf_t*                              sf_symbols);

ISRRAN_API int isrran_refsignal_dmrs_pusch_gen(isrran_refsignal_ul_t*             q,
                                               isrran_refsignal_dmrs_pusch_cfg_t* cfg,
                                               uint32_t                           nof_prb,
                                               uint32_t                           sf_idx,
                                               uint32_t                           cyclic_shift_for_dmrs,
                                               cf_t*                              r_pusch);

ISRRAN_API void isrran_refsignal_dmrs_pusch_put(isrran_refsignal_ul_t* q,
                                                isrran_pusch_cfg_t*    pusch_cfg,
                                                cf_t*                  r_pusch,
                                                cf_t*                  sf_symbols);

ISRRAN_API void isrran_refsignal_dmrs_pusch_get(isrran_refsignal_ul_t* q,
                                                isrran_pusch_cfg_t*    pusch_cfg,
                                                cf_t*                  sf_symbols,
                                                cf_t*                  r_pusch);

ISRRAN_API int isrran_refsignal_dmrs_pucch_gen(isrran_refsignal_ul_t* q,
                                               isrran_ul_sf_cfg_t*    sf,
                                               isrran_pucch_cfg_t*    cfg,
                                               cf_t*                  r_pucch);

ISRRAN_API int
isrran_refsignal_dmrs_pucch_put(isrran_refsignal_ul_t* q, isrran_pucch_cfg_t* cfg, cf_t* r_pucch, cf_t* output);

ISRRAN_API int
isrran_refsignal_dmrs_pucch_get(isrran_refsignal_ul_t* q, isrran_pucch_cfg_t* cfg, cf_t* input, cf_t* r_pucch);

ISRRAN_API int isrran_refsignal_isr_pregen(isrran_refsignal_ul_t*             q,
                                           isrran_refsignal_isr_pregen_t*     pregen,
                                           isrran_refsignal_isr_cfg_t*        cfg,
                                           isrran_refsignal_dmrs_pusch_cfg_t* dmrs);

ISRRAN_API int isrran_refsignal_isr_pregen_put(isrran_refsignal_ul_t*         q,
                                               isrran_refsignal_isr_pregen_t* pregen,
                                               isrran_refsignal_isr_cfg_t*    cfg,
                                               uint32_t                       tti,
                                               cf_t*                          sf_symbols);

ISRRAN_API void isrran_refsignal_isr_pregen_free(isrran_refsignal_ul_t* q, isrran_refsignal_isr_pregen_t* pregen);

ISRRAN_API int isrran_refsignal_isr_gen(isrran_refsignal_ul_t*             q,
                                        isrran_refsignal_isr_cfg_t*        cfg,
                                        isrran_refsignal_dmrs_pusch_cfg_t* pusch_cfg,
                                        uint32_t                           sf_idx,
                                        cf_t*                              r_isr);

ISRRAN_API int isrran_refsignal_isr_put(isrran_refsignal_ul_t*      q,
                                        isrran_refsignal_isr_cfg_t* cfg,
                                        uint32_t                    tti,
                                        cf_t*                       r_isr,
                                        cf_t*                       sf_symbols);

ISRRAN_API int isrran_refsignal_isr_get(isrran_refsignal_ul_t*      q,
                                        isrran_refsignal_isr_cfg_t* cfg,
                                        uint32_t                    tti,
                                        cf_t*                       r_isr,
                                        cf_t*                       sf_symbols);

ISRRAN_API void isrran_refsignal_isr_pusch_shortened(isrran_refsignal_ul_t*      q,
                                                     isrran_ul_sf_cfg_t*         sf,
                                                     isrran_refsignal_isr_cfg_t* isr_cfg,
                                                     isrran_pusch_cfg_t*         pusch_cfg);

ISRRAN_API void isrran_refsignal_isr_pucch_shortened(isrran_refsignal_ul_t*      q,
                                                     isrran_ul_sf_cfg_t*         sf,
                                                     isrran_refsignal_isr_cfg_t* isr_cfg,
                                                     isrran_pucch_cfg_t*         pucch_cfg);

ISRRAN_API int isrran_refsignal_isr_send_cs(uint32_t subframe_config, uint32_t sf_idx);

ISRRAN_API int isrran_refsignal_isr_send_ue(uint32_t I_isr, uint32_t tti);

ISRRAN_API uint32_t isrran_refsignal_isr_rb_start_cs(uint32_t bw_cfg, uint32_t nof_prb);

ISRRAN_API uint32_t isrran_refsignal_isr_rb_L_cs(uint32_t bw_cfg, uint32_t nof_prb);

ISRRAN_API uint32_t isrran_refsignal_isr_M_sc(isrran_refsignal_ul_t* q, isrran_refsignal_isr_cfg_t* cfg);

#endif // ISRRAN_REFSIGNAL_UL_H
