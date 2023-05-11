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
#include "isrran/phy/phch/phch_cfg_nr.h"
#include "isrran/phy/ch_estimation/dmrs_sch.h"
#include "isrran/phy/phch/csi.h"
#include "isrran/phy/phch/uci_nr.h"

static const char* dmrs_sch_type_to_str(isrran_dmrs_sch_type_t type)
{
  switch (type) {
    case isrran_dmrs_sch_type_1:
      return "1";
    case isrran_dmrs_sch_type_2:
      return "2";
    default:; // Do nothing
  }
  return "invalid";
}

static const char* dmrs_sch_add_pos_to_str(isrran_dmrs_sch_add_pos_t add_pos)
{
  switch (add_pos) {
    case isrran_dmrs_sch_add_pos_2:
      return "2";
    case isrran_dmrs_sch_add_pos_0:
      return "0";
    case isrran_dmrs_sch_add_pos_1:
      return "1";
    case isrran_dmrs_sch_add_pos_3:
      return "3";
    default:; // Do nothing
  }
  return "invalid";
}

static const char* dmrs_sch_len_to_str(isrran_dmrs_sch_len_t len)
{
  switch (len) {
    case isrran_dmrs_sch_len_1:
      return "single";
    case isrran_dmrs_sch_len_2:
      return "double";
    default:; // Do nothing
  }
  return "invalid";
}

static const char* dmrs_sch_typeApos_to_str(isrran_dmrs_sch_typeA_pos_t typeA_pos)
{
  switch (typeA_pos) {
    case isrran_dmrs_sch_typeA_pos_2:
      return "2";
    case isrran_dmrs_sch_typeA_pos_3:
      return "3";
    default:; // Do nothing
  }
  return "invalid";
}

static uint32_t phch_cfg_nr_dmrs_to_str(const isrran_dmrs_sch_cfg_t* dmrs,
                                        const isrran_sch_grant_nr_t* grant,
                                        char*                        str,
                                        uint32_t                     str_len)
{
  uint32_t len = 0;

  len = isrran_print_check(str, str_len, len, "  DMRS:\n");
  len = isrran_print_check(str, str_len, len, "    type=%s\n", dmrs_sch_type_to_str(dmrs->type));
  len = isrran_print_check(str, str_len, len, "    add_pos=%s\n", dmrs_sch_add_pos_to_str(dmrs->additional_pos));
  len = isrran_print_check(str, str_len, len, "    len=%s\n", dmrs_sch_len_to_str(dmrs->length));
  len = isrran_print_check(str, str_len, len, "    typeA_pos=%s\n", dmrs_sch_typeApos_to_str(dmrs->typeA_pos));

  if (dmrs->scrambling_id0_present) {
    len = isrran_print_check(str, str_len, len, "    scrambling_id_0=%03x\n", dmrs->scrambling_id0);
  }

  if (dmrs->scrambling_id1_present) {
    len = isrran_print_check(str, str_len, len, "    scrambling_id_1=%03x\n", dmrs->scrambling_id1);
  }

  if (dmrs->lte_CRS_to_match_around) {
    len = isrran_print_check(str, str_len, len, "    lte_CRS_to_match_around=y\n");
  }

  if (dmrs->additional_DMRS_DL_Alt) {
    len = isrran_print_check(str, str_len, len, "    additional_DMRS_DL_Alt=y\n");
  }

  isrran_re_pattern_t pattern = {};
  if (isrran_dmrs_sch_rvd_re_pattern(dmrs, grant, &pattern) == ISRRAN_SUCCESS) {
    len = isrran_print_check(str, str_len, len, "    rvd_pattern: ");
    len += isrran_re_pattern_info(&pattern, &str[len], str_len - len);
    len = isrran_print_check(str, str_len, len, "\n");
  }

  return len;
}

static const char* sch_mapping_to_str(isrran_sch_mapping_type_t mapping)
{
  switch (mapping) {
    case isrran_sch_mapping_type_A:
      return "A";
    case isrran_sch_mapping_type_B:
      return "B";
    default:; // Do nothing
  }
  return "invalid";
}

static const char* sch_xoverhead_to_str(isrran_xoverhead_t xoverhead)
{
  switch (xoverhead) {
    case isrran_xoverhead_0:
      return "0";
    case isrran_xoverhead_6:
      return "6";
    case isrran_xoverhead_12:
      return "12";
    case isrran_xoverhead_18:
      return "18";
    default:; // Do nothing
  }
  return "invalid";
}

static uint32_t phch_cfg_tb_to_str(const isrran_sch_tb_t* tb, char* str, uint32_t str_len)
{
  uint32_t len = 0;

  if (!tb->enabled) {
    return len;
  }

  len = isrran_print_check(str, str_len, len, "    CW%d:\n", tb->cw_idx);
  len = isrran_print_check(str, str_len, len, "      mod=%s\n", isrran_mod_string(tb->mod));
  len = isrran_print_check(str, str_len, len, "      nof_layers=%d\n", tb->N_L);
  len = isrran_print_check(str, str_len, len, "      mcs=%d\n", tb->mcs);
  len = isrran_print_check(str, str_len, len, "      tbs=%d\n", tb->tbs);
  len = isrran_print_check(str, str_len, len, "      R=%.3f\n", tb->R);
  len = isrran_print_check(str, str_len, len, "      rv=%d\n", tb->rv);
  len = isrran_print_check(str, str_len, len, "      ndi=%d\n", tb->ndi);
  len = isrran_print_check(str, str_len, len, "      nof_re=%d\n", tb->nof_re);
  len = isrran_print_check(str, str_len, len, "      nof_bits=%d\n", tb->nof_bits);

  return len;
}

static uint32_t phch_cfg_grant_to_str(const isrran_sch_grant_nr_t* grant, char* str, uint32_t str_len)
{
  uint32_t len = 0;

  uint32_t first_prb = ISRRAN_MAX_PRB_NR;
  for (uint32_t i = 0; i < ISRRAN_MAX_PRB_NR && first_prb == ISRRAN_MAX_PRB_NR; i++) {
    if (grant->prb_idx[i]) {
      first_prb = i;
    }
  }

  len = isrran_print_check(str, str_len, len, "  Grant:\n");
  len = isrran_print_check(str, str_len, len, "    rnti=0x%x\n", grant->rnti);
  len = isrran_print_check(str, str_len, len, "    rnti_type=%s\n", isrran_rnti_type_str(grant->rnti_type));
  len = isrran_print_check(str, str_len, len, "    k=%d\n", grant->k);
  len = isrran_print_check(str, str_len, len, "    mapping=%s\n", sch_mapping_to_str(grant->mapping));
  len = isrran_print_check(str, str_len, len, "    t_alloc=%d:%d\n", grant->S, grant->L);
  len = isrran_print_check(str, str_len, len, "    f_alloc=%d:%d\n", first_prb, grant->nof_prb);
  len = isrran_print_check(str, str_len, len, "    nof_dmrs_cdm_grps=%d\n", grant->nof_dmrs_cdm_groups_without_data);
  len = isrran_print_check(str, str_len, len, "    beta_dmrs=%f\n", grant->beta_dmrs);
  len = isrran_print_check(str, str_len, len, "    nof_layers=%d\n", grant->nof_layers);
  len = isrran_print_check(str, str_len, len, "    n_scid=%d\n", grant->n_scid);
  len = isrran_print_check(str, str_len, len, "    tb_scaling_field=%d\n", grant->tb_scaling_field);

  for (uint32_t i = 0; i < ISRRAN_MAX_TB; i++) {
    len += phch_cfg_tb_to_str(&grant->tb[i], &str[len], str_len - len);
  }

  return len;
}

static uint32_t phch_cfg_sch_to_str(const isrran_sch_cfg_t* sch, char* str, uint32_t str_len)
{
  uint32_t len = 0;

  len = isrran_print_check(str, str_len, len, "  SCH:\n");
  len = isrran_print_check(str, str_len, len, "    mcs_table=%s\n", isrran_mcs_table_to_str(sch->mcs_table));
  len = isrran_print_check(str, str_len, len, "    xoverhead=%s\n", sch_xoverhead_to_str(sch->xoverhead));

  return len;
}

static uint32_t phch_cfg_rvd_to_str(const isrran_re_pattern_list_t* pattern_list, char* str, uint32_t str_len)
{
  uint32_t len = 0;

  if (pattern_list->count == 0) {
    return len;
  }

  len = isrran_print_check(str, str_len, len, "  Reserved:\n");
  for (uint32_t i = 0; i < pattern_list->count; i++) {
    len = isrran_print_check(str, str_len, len, "    %d: ", i);
    len += isrran_re_pattern_info(&pattern_list->data[i], &str[len], str_len - len);
    len = isrran_print_check(str, str_len, len, "\n");
  }

  return len;
}

static uint32_t phch_cfg_uci_to_str(const isrran_uci_cfg_nr_t* uci, char* str, uint32_t str_len)
{
  uint32_t len = 0;

  if (isrran_uci_nr_total_bits(uci) == 0) {
    return len;
  }

  len = isrran_print_check(str, str_len, len, "  UCI:\n");
  len = isrran_print_check(str, str_len, len, "    alpha=%.2f\n", uci->pusch.alpha);
  len = isrran_print_check(str, str_len, len, "    beta_harq_ack_offset=%.2f\n", uci->pusch.beta_harq_ack_offset);
  len = isrran_print_check(str, str_len, len, "    beta_csi_part1_offset=%.2f\n", uci->pusch.beta_csi1_offset);
  len = isrran_print_check(str, str_len, len, "    beta_csi_part2_offset=%.2f\n", uci->pusch.beta_csi1_offset);
  len = isrran_print_check(str, str_len, len, "    o_csi1=%d\n", isrran_csi_part1_nof_bits(uci->csi, uci->nof_csi));
  len = isrran_print_check(str, str_len, len, "    o_ack=%d\n", uci->ack.count);

  return len;
}

uint32_t isrran_sch_cfg_nr_info(const isrran_sch_cfg_nr_t* sch_cfg, char* str, uint32_t str_len)
{
  uint32_t len = 0;

  if (sch_cfg->scambling_id) {
    len = isrran_print_check(str, str_len, len, "  scrambling_id=0x%03x\n", sch_cfg->scrambling_id_present);
  }

  // Append DMRS information
  len += phch_cfg_nr_dmrs_to_str(&sch_cfg->dmrs, &sch_cfg->grant, &str[len], str_len - len);

  // Append grant information
  len += phch_cfg_grant_to_str(&sch_cfg->grant, &str[len], str_len - len);

  // Append SCH information
  len += phch_cfg_sch_to_str(&sch_cfg->sch_cfg, &str[len], str_len - len);

  // Append reserved RE information
  len += phch_cfg_rvd_to_str(&sch_cfg->rvd_re, &str[len], str_len - len);

  // UCI configuration
  len += phch_cfg_uci_to_str(&sch_cfg->uci, &str[len], str_len - len);

  return len;
}
