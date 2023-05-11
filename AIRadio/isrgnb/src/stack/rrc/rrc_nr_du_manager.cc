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

#include "isrgnb/hdr/stack/rrc/rrc_nr_du_manager.h"
#include "isrgnb/hdr/stack/rrc/cell_asn1_config.h"
#include "isrran/asn1/rrc_nr_utils.h"
#include "isrran/common/string_helpers.h"

using namespace asn1::rrc_nr;

namespace isrenb {

du_config_manager::du_config_manager(const rrc_nr_cfg_t& cfg_) : cfg(cfg_), logger(isrlog::fetch_basic_logger("RRC-NR"))
{}

du_config_manager::~du_config_manager() {}

int du_config_manager::add_cell()
{
  // add cell
  std::unique_ptr<du_cell_config> obj  = std::make_unique<du_cell_config>();
  du_cell_config&                 cell = *obj;
  cell.cc                              = cells.size();

  // Fill general cell params
  cell.pci = cfg.cell_list[cell.cc].phy_cell.carrier.pci;

  // fill MIB ASN.1
  if (fill_mib_from_enb_cfg(cfg.cell_list[cell.cc], cell.mib) != ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Pack MIB
  cell.packed_mib = isrran::make_byte_buffer();
  if (cell.packed_mib == nullptr) {
    logger.error("Couldn't allocate PDU in %s().", __FUNCTION__);
    return ISRRAN_ERROR;
  }
  {
    asn1::bit_ref  bref(cell.packed_mib->msg, cell.packed_mib->get_tailroom());
    bcch_bch_msg_s bch_msg;
    bch_msg.msg.set_mib() = cell.mib;
    if (bch_msg.pack(bref) != asn1::ISRASN_SUCCESS) {
      logger.error("Couldn't pack mib msg");
      return ISRRAN_ERROR;
    }
    cell.packed_mib->N_bytes = bref.distance_bytes();
  }
  logger.info(
      cell.packed_mib->data(), cell.packed_mib->size(), "BCCH-BCH Message (with MIB) (%d B)", cell.packed_mib->size());
  asn1::json_writer js;
  cell.mib.to_json(js);
  logger.info("MIB content: %s", js.to_string().c_str());

  // fill SIB1 ASN.1
  if (fill_sib1_from_enb_cfg(cfg, cell.cc, cell.sib1) != ISRRAN_SUCCESS) {
    logger.error("Couldn't generate SIB1");
    return ISRRAN_ERROR;
  }

  // Pack SIB1
  cell.packed_sib1 = isrran::make_byte_buffer();
  if (cell.packed_sib1 == nullptr) {
    logger.error("Couldn't allocate PDU in %s().", __FUNCTION__);
    return ISRRAN_ERROR;
  }
  {
    asn1::bit_ref     bref(cell.packed_sib1->msg, cell.packed_sib1->get_tailroom());
    bcch_dl_sch_msg_s bcch_msg;
    bcch_msg.msg.set_c1().set_sib_type1() = cell.sib1;
    if (bcch_msg.pack(bref) != asn1::ISRASN_SUCCESS) {
      logger.error("Couldn't pack SIB1 msg");
      return ISRRAN_ERROR;
    }
    cell.packed_sib1->N_bytes = bref.distance_bytes();
  }
  if (cfg.is_standalone) {
    logger.info(cell.packed_sib1->data(),
                cell.packed_sib1->size(),
                "BCCH-DL-SCH-Message (with SIB1) (%d B)",
                cell.packed_sib1->size());
    cell.sib1.to_json(js);
    logger.info("SIB1 content: %s", js.to_string().c_str());
  }

  // Generate SSB SCS
  isrran_subcarrier_spacing_t ssb_scs;
  if (not isrran::fill_ssb_pattern_scs(cfg.cell_list[cell.cc].phy_cell.carrier, &cell.ssb_pattern, &ssb_scs)) {
    return ISRRAN_ERROR;
  }
  cell.ssb_scs.value      = (subcarrier_spacing_e::options)ssb_scs;
  cell.ssb_center_freq_hz = cfg.cell_list[cell.cc].ssb_freq_hz;
  cell.dl_freq_hz         = cfg.cell_list[cell.cc].phy_cell.carrier.dl_center_frequency_hz;
  cell.is_standalone      = cfg.is_standalone;

  cells.push_back(std::move(obj));
  return ISRRAN_SUCCESS;
}

void fill_phy_pdcch_cfg_common(const du_cell_config& cell, isrran_pdcch_cfg_nr_t* pdcch)
{
  const serving_cell_cfg_common_sib_s& serv_cell    = cell.serv_cell_cfg_common();
  const pdcch_cfg_common_s&            pdcch_common = serv_cell.dl_cfg_common.init_dl_bwp.pdcch_cfg_common.setup();

  bool    is_sa        = cell.is_standalone;
  uint8_t coreset0_idx = cell.mib.pdcch_cfg_sib1.ctrl_res_set_zero;
  auto scs = (isrran_subcarrier_spacing_t)serv_cell.dl_cfg_common.init_dl_bwp.generic_params.subcarrier_spacing.value;
  auto ssb_scs     = (isrran_subcarrier_spacing_t)cell.ssb_scs.value;
  uint32_t nof_prb = serv_cell.dl_cfg_common.freq_info_dl.scs_specific_carrier_list[0].carrier_bw;

  if (is_sa) {
    // Generate CORESET#0
    pdcch->coreset_present[0] = true;
    // Get pointA and SSB absolute frequencies
    double pointA_abs_freq_Hz = cell.dl_freq_hz - nof_prb * ISRRAN_NRE * ISRRAN_SUBC_SPACING_NR(scs) / 2;
    double ssb_abs_freq_Hz    = cell.ssb_center_freq_hz;
    // Calculate integer SSB to pointA frequency offset in Hz
    uint32_t ssb_pointA_freq_offset_Hz =
        (ssb_abs_freq_Hz > pointA_abs_freq_Hz) ? (uint32_t)(ssb_abs_freq_Hz - pointA_abs_freq_Hz) : 0;
    int ret = isrran_coreset_zero(cell.pci, ssb_pointA_freq_offset_Hz, ssb_scs, scs, coreset0_idx, &pdcch->coreset[0]);
    isrran_assert(ret == ISRRAN_SUCCESS, "Failed to generate CORESET#0");

    // Generate SearchSpace#0
    pdcch->search_space_present[0]           = true;
    pdcch->search_space[0].id                = 0;
    pdcch->search_space[0].coreset_id        = 0;
    pdcch->search_space[0].type              = isrran_search_space_type_common_0;
    pdcch->search_space[0].nof_candidates[0] = 1;
    pdcch->search_space[0].nof_candidates[1] = 1;
    pdcch->search_space[0].nof_candidates[2] = 1;
    pdcch->search_space[0].nof_candidates[3] = 0;
    pdcch->search_space[0].nof_candidates[4] = 0;
    pdcch->search_space[0].nof_formats       = 1;
    pdcch->search_space[0].formats[0]        = isrran_dci_format_nr_1_0;
    pdcch->search_space[0].duration          = 1;
  }

  // Generate Common CORESETs and Search Spaces
  bool ret = isrran::fill_phy_pdcch_cfg_common(pdcch_common, pdcch);
  isrran_assert(ret, "PDCCH Config Common");
}

} // namespace isrenb
