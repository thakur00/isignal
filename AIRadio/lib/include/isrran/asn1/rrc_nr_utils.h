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

#ifndef ISRRAN_RRC_NR_UTILS_H
#define ISRRAN_RRC_NR_UTILS_H

#include "isrran/common/phy_cfg_nr.h"
#include "isrran/interfaces/mac_interface_types.h"
#include "isrran/interfaces/pdcp_interface_types.h"
#include "isrran/interfaces/rlc_interface_types.h"

/************************
 * Forward declarations
 ***********************/
namespace asn1 {
namespace rrc_nr {

struct plmn_id_s;
struct sib1_s;
struct rlc_cfg_c;
struct pdcp_cfg_s;
struct lc_ch_cfg_s;
struct rach_cfg_common_s;
struct phr_cfg_s;

// Phy
struct tdd_ul_dl_cfg_common_s;
struct phys_cell_group_cfg_s;
struct search_space_s;
struct search_space_s;
struct csi_report_cfg_s;
struct ctrl_res_set_s;
struct pdsch_time_domain_res_alloc_s;
struct pusch_time_domain_res_alloc_s;
struct pucch_format_cfg_s;
struct pucch_res_s;
struct sched_request_res_cfg_s;
struct pusch_cfg_s;
struct pdsch_cfg_s;
struct dmrs_dl_cfg_s;
struct dmrs_ul_cfg_s;
struct beta_offsets_s;
struct uci_on_pusch_s;
struct zp_csi_rs_res_s;
struct nzp_csi_rs_res_s;
struct pdsch_serving_cell_cfg_s;
struct freq_info_dl_s;
struct serving_cell_cfg_common_s;
struct serving_cell_cfg_common_sib_s;
struct serving_cell_cfg_s;
struct pdcch_cfg_common_s;
struct pdcch_cfg_s;
struct pdsch_cfg_common_s;
struct pucch_cfg_common_s;
struct pucch_cfg_s;
struct pusch_cfg_common_s;
struct mib_s;

struct srb_to_add_mod_s;
struct drb_to_add_mod_s;

} // namespace rrc_nr
} // namespace asn1

/************************
 *  Conversion Helpers
 ***********************/
namespace isrran {

plmn_id_t make_plmn_id_t(const asn1::rrc_nr::plmn_id_s& asn1_type);
void      to_asn1(asn1::rrc_nr::plmn_id_s* asn1_type, const plmn_id_t& cfg);
/***************************
 *      PHY Config
 **************************/
bool make_phy_rach_cfg(const asn1::rrc_nr::rach_cfg_common_s& asn1_type,
                       isrran_duplex_mode_t                   duplex_mode,
                       isrran_prach_cfg_t*                    prach_cfg);
bool fill_rach_cfg_common(const isrran_prach_cfg_t& prach_cfg, asn1::rrc_nr::rach_cfg_common_s& asn1_type);

bool make_phy_tdd_cfg(const asn1::rrc_nr::tdd_ul_dl_cfg_common_s& tdd_ul_dl_cfg_common,
                      isrran_duplex_config_nr_t*                  isrran_duplex_config_nr);
bool make_phy_tdd_cfg(const isrran_duplex_config_nr_t&      isrran_duplex_config_nr,
                      isrran_subcarrier_spacing_t           scs,
                      asn1::rrc_nr::tdd_ul_dl_cfg_common_s* tdd_ul_dl_cfg_common);
bool make_phy_harq_ack_cfg(const asn1::rrc_nr::phys_cell_group_cfg_s& phys_cell_group_cfg,
                           isrran_harq_ack_cfg_hl_t*                  isrran_ue_dl_nr_harq_ack_cfg);
bool make_phy_coreset_cfg(const asn1::rrc_nr::ctrl_res_set_s& ctrl_res_set, isrran_coreset_t* isrran_coreset);
void make_phy_search_space0_cfg(isrran_search_space_t* in_isrran_search_space);
bool make_phy_search_space_cfg(const asn1::rrc_nr::search_space_s& search_space,
                               isrran_search_space_t*              isrran_search_space);
bool make_phy_csi_report(const asn1::rrc_nr::csi_report_cfg_s& csi_report_cfg,
                         isrran_csi_hl_report_cfg_t*           isrran_csi_hl_report_cfg);
bool make_phy_common_time_ra(const asn1::rrc_nr::pdsch_time_domain_res_alloc_s& pdsch_time_domain_res_alloc,
                             isrran_sch_time_ra_t*                              isrran_sch_time_ra);
bool make_phy_common_time_ra(const asn1::rrc_nr::pusch_time_domain_res_alloc_s& pusch_time_domain_res_allo,
                             isrran_sch_time_ra_t*                              isrran_sch_time_ra);
bool make_phy_max_code_rate(const asn1::rrc_nr::pucch_format_cfg_s& pucch_format_cfg, uint32_t* max_code_rate);
bool make_phy_res_config(const asn1::rrc_nr::pucch_res_s& pucch_res,
                         uint32_t                         format_2_max_code_rate,
                         isrran_pucch_nr_resource_t*      isrran_pucch_nr_resource);
bool make_phy_res_config(const isrran_pucch_nr_resource_t& in_pucch_res,
                         asn1::rrc_nr::pucch_res_s&        out_pucch_res,
                         uint32_t                          pucch_res_id);
bool make_phy_sr_resource(const asn1::rrc_nr::sched_request_res_cfg_s& sched_request_res_cfg,
                          isrran_pucch_nr_sr_resource_t*               isrran_pucch_nr_sr_resource);
bool make_phy_pusch_alloc_type(const asn1::rrc_nr::pusch_cfg_s& pusch_cfg,
                               isrran_resource_alloc_t*         in_isrran_resource_alloc);
bool make_phy_pdsch_alloc_type(const asn1::rrc_nr::pdsch_cfg_s& pdsch_cfg,
                               isrran_resource_alloc_t*         in_isrran_resource_alloc);
bool make_phy_dmrs_dl_additional_pos(const asn1::rrc_nr::dmrs_dl_cfg_s& dmrs_dl_cfg,
                                     isrran_dmrs_sch_add_pos_t*         in_isrran_dmrs_sch_add_pos);
bool make_phy_dmrs_ul_additional_pos(const asn1::rrc_nr::dmrs_ul_cfg_s& dmrs_ul_cfg,
                                     isrran_dmrs_sch_add_pos_t*         isrran_dmrs_sch_add_pos);
bool make_phy_beta_offsets(const asn1::rrc_nr::beta_offsets_s& beta_offsets,
                           isrran_beta_offsets_t*              isrran_beta_offsets);
bool make_phy_pusch_scaling(const asn1::rrc_nr::uci_on_pusch_s& uci_on_pusch, float* scaling);
bool make_phy_zp_csi_rs_resource(const asn1::rrc_nr::zp_csi_rs_res_s& zp_csi_rs_res,
                                 isrran_csi_rs_zp_resource_t*         zp_csi_rs_resource);
bool make_phy_nzp_csi_rs_resource(const asn1::rrc_nr::nzp_csi_rs_res_s& nzp_csi_rs_res,
                                  isrran_csi_rs_nzp_resource_t*         csi_rs_nzp_resource);
bool make_phy_carrier_cfg(const asn1::rrc_nr::freq_info_dl_s& freq_info_dl, isrran_carrier_nr_t* carrier_nr);
bool fill_phy_ssb_cfg(const isrran_carrier_nr_t&                         carrier,
                      const asn1::rrc_nr::serving_cell_cfg_common_sib_s& serv_cell_cfg,
                      phy_cfg_nr_t::ssb_cfg_t*                           out_ssb);
void fill_ssb_pos_in_burst(const asn1::rrc_nr::serving_cell_cfg_common_sib_s& ssb_pos,
                           phy_cfg_nr_t::ssb_cfg_t*                           out_ssb);
bool fill_ssb_pattern_scs(const isrran_carrier_nr_t&   carrier,
                          isrran_ssb_pattern_t*        pattern,
                          isrran_subcarrier_spacing_t* ssb_scs);
bool fill_phy_ssb_cfg(const isrran_carrier_nr_t&                         carrier,
                      const asn1::rrc_nr::serving_cell_cfg_common_sib_s& serv_cell_cfg,
                      isrran_ssb_cfg_t*                                  out_ssb);
bool make_phy_ssb_cfg(const isrran_carrier_nr_t&                     carrier,
                      const asn1::rrc_nr::serving_cell_cfg_common_s& serv_cell_cfg,
                      phy_cfg_nr_t::ssb_cfg_t*                       ssb);
bool make_phy_mib(const asn1::rrc_nr::mib_s& mib_cfg, isrran_mib_nr_t* mib);
bool make_pdsch_cfg_from_serv_cell(const asn1::rrc_nr::serving_cell_cfg_s& serv_cell, isrran_sch_hl_cfg_nr_t* sch_hl);
bool make_csi_cfg_from_serv_cell(const asn1::rrc_nr::serving_cell_cfg_s& serv_cell, isrran_csi_hl_cfg_t* csi_hl);
bool make_duplex_cfg_from_serv_cell(const asn1::rrc_nr::serving_cell_cfg_common_s& serv_cell,
                                    isrran_duplex_config_nr_t*                     duplex_cfg);
bool fill_phy_pdcch_cfg_common(const asn1::rrc_nr::pdcch_cfg_common_s& pdcch_cfg, isrran_pdcch_cfg_nr_t* pdcch);
bool fill_phy_pdcch_cfg(const asn1::rrc_nr::pdcch_cfg_s& pdcch_cfg, isrran_pdcch_cfg_nr_t* pdcch);
bool fill_phy_pdsch_cfg_common(const asn1::rrc_nr::pdsch_cfg_common_s& pdsch_cfg, isrran_sch_hl_cfg_nr_t* pdsch);
void fill_phy_pucch_cfg_common(const asn1::rrc_nr::pucch_cfg_common_s& pucch_cfg, isrran_pucch_nr_common_cfg_t* pucch);
bool fill_phy_pucch_cfg(const asn1::rrc_nr::pucch_cfg_s& pucch_cfg, isrran_pucch_nr_hl_cfg_t* pucch);
bool fill_phy_pucch_hl_cfg(const asn1::rrc_nr::pucch_cfg_s& pucch_cfg, isrran_pucch_nr_hl_cfg_t* pucch);
bool fill_phy_pusch_cfg_common(const asn1::rrc_nr::pusch_cfg_common_s& pusch_cfg, isrran_sch_hl_cfg_nr_t* pusch);
void fill_phy_carrier_cfg(const asn1::rrc_nr::serving_cell_cfg_common_sib_s& serv_cell_cfg,
                          isrran_carrier_nr_t*                               carrier_nr);
void fill_phy_ssb_cfg(const asn1::rrc_nr::serving_cell_cfg_common_sib_s& serv_cell_cfg, phy_cfg_nr_t::ssb_cfg_t* ssb);

/***************************
 *      MAC Config
 **************************/
logical_channel_config_t make_mac_logical_channel_cfg_t(uint8_t lcid, const asn1::rrc_nr::lc_ch_cfg_s& asn1_type);
void make_mac_rach_cfg(const asn1::rrc_nr::rach_cfg_common_s& asn1_type, rach_cfg_nr_t* rach_cfg_nr);
bool make_mac_phr_cfg_t(const asn1::rrc_nr::phr_cfg_s& asn1_type, phr_cfg_nr_t* phr_cfg_nr);
bool make_mac_dl_harq_cfg_nr_t(const asn1::rrc_nr::pdsch_serving_cell_cfg_s& asn1_type,
                               dl_harq_cfg_nr_t*                             out_dl_harq_cfg_nr);
/***************************
 *      RLC Config
 **************************/
int make_rlc_config_t(const asn1::rrc_nr::rlc_cfg_c& asn1_type, uint8_t bearer_id, rlc_config_t* rlc_config_out);

/***************************
 *      PDCP Config
 **************************/
pdcp_config_t make_srb_pdcp_config_t(const uint8_t bearer_id, bool is_ue);
pdcp_config_t make_nr_srb_pdcp_config_t(const uint8_t bearer_id, bool is_ue);
pdcp_config_t make_drb_pdcp_config_t(const uint8_t bearer_id, bool is_ue, const asn1::rrc_nr::pdcp_cfg_s& pdcp_cfg);

} // namespace isrran

/************************
 * ASN1 RRC extensions
 ***********************/
namespace asn1 {

namespace rrc_nr {
bool operator==(const srb_to_add_mod_s& lhs, const srb_to_add_mod_s& rhs);
bool operator==(const drb_to_add_mod_s& lhs, const drb_to_add_mod_s& rhs);
} // namespace rrc_nr

} // namespace asn1

#endif // ISRRAN_RRC_NR_UTILS_H
