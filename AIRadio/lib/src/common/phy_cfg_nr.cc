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

#include "isrran/common/phy_cfg_nr.h"
#include "isrran/common/band_helper.h"
#include "isrran/isrran.h"

namespace isrran {

isrran_dci_cfg_nr_t phy_cfg_nr_t::get_dci_cfg() const
{
  isrran_dci_cfg_nr_t dci_cfg = {};

  // Assume BWP bandwidth equals full channel bandwidth
  dci_cfg.coreset0_bw       = pdcch.coreset_present[0] ? isrran_coreset_get_bw(&pdcch.coreset[0]) : 0;
  dci_cfg.bwp_dl_initial_bw = carrier.nof_prb;
  dci_cfg.bwp_dl_active_bw  = carrier.nof_prb;
  dci_cfg.bwp_ul_initial_bw = carrier.nof_prb;
  dci_cfg.bwp_ul_active_bw  = carrier.nof_prb;

  // Iterate over all SS to select monitoring options
  for (uint32_t i = 0; i < ISRRAN_UE_DL_NR_MAX_NOF_SEARCH_SPACE; i++) {
    // Skip not configured SS
    if (not pdcch.search_space_present[i]) {
      continue;
    }

    // Iterate all configured formats
    for (uint32_t j = 0; j < pdcch.search_space[i].nof_formats; j++) {
      if ((pdcch.search_space[i].type == isrran_search_space_type_common_3 or
           pdcch.search_space[i].type == isrran_search_space_type_common_1) &&
          pdcch.search_space[i].formats[j] == isrran_dci_format_nr_0_0) {
        dci_cfg.monitor_common_0_0 = true;
      } else if (pdcch.search_space[i].type == isrran_search_space_type_ue &&
                 pdcch.search_space[i].formats[j] == isrran_dci_format_nr_0_0) {
        dci_cfg.monitor_0_0_and_1_0 = true;
      } else if (pdcch.search_space[i].type == isrran_search_space_type_ue &&
                 pdcch.search_space[i].formats[j] == isrran_dci_format_nr_0_1) {
        dci_cfg.monitor_0_1_and_1_1 = true;
      }
    }
  }

  // Set PUSCH parameters
  dci_cfg.enable_sul     = false;
  dci_cfg.enable_hopping = false;

  // Set Format 0_1 and 1_1 parameters
  dci_cfg.carrier_indicator_size = 0;
  dci_cfg.harq_ack_codebok       = harq_ack.harq_ack_codebook;
  dci_cfg.nof_rb_groups          = 0;

  // Format 0_1 specific configuration (for PUSCH only)
  dci_cfg.nof_ul_bwp      = 0;
  dci_cfg.nof_ul_time_res = (pusch.nof_dedicated_time_ra > 0)
                                ? pusch.nof_dedicated_time_ra
                                : (pusch.nof_common_time_ra > 0) ? pusch.nof_common_time_ra : ISRRAN_MAX_NOF_TIME_RA;
  dci_cfg.nof_isr                        = 1;
  dci_cfg.nof_ul_layers                  = 1;
  dci_cfg.pusch_nof_cbg                  = 0;
  dci_cfg.report_trigger_size            = 0;
  dci_cfg.enable_transform_precoding     = false;
  dci_cfg.dynamic_dual_harq_ack_codebook = false;
  dci_cfg.pusch_tx_config_non_codebook   = false;
  dci_cfg.pusch_ptrs                     = false;
  dci_cfg.pusch_dynamic_betas            = false;
  dci_cfg.pusch_alloc_type               = pusch.alloc;
  dci_cfg.pusch_dmrs_type                = pusch.dmrs_type;
  dci_cfg.pusch_dmrs_max_len             = pusch.dmrs_max_length;

  // Format 1_1 specific configuration (for PDSCH only)
  dci_cfg.nof_dl_bwp      = 0;
  dci_cfg.nof_dl_time_res = (pdsch.nof_dedicated_time_ra > 0)
                                ? pdsch.nof_dedicated_time_ra
                                : (pdsch.nof_common_time_ra > 0) ? pdsch.nof_common_time_ra : ISRRAN_MAX_NOF_TIME_RA;
  dci_cfg.nof_aperiodic_zp       = 0;
  dci_cfg.pdsch_nof_cbg          = 0;
  dci_cfg.nof_dl_to_ul_ack       = harq_ack.nof_dl_data_to_ul_ack;
  dci_cfg.pdsch_inter_prb_to_prb = false;
  dci_cfg.pdsch_rm_pattern1      = false;
  dci_cfg.pdsch_rm_pattern2      = false;
  dci_cfg.pdsch_2cw              = false;
  dci_cfg.multiple_scell         = false;
  dci_cfg.pdsch_tci              = false;
  dci_cfg.pdsch_cbg_flush        = false;
  dci_cfg.pdsch_dynamic_bundling = false;
  dci_cfg.pdsch_alloc_type       = pdsch.alloc;
  dci_cfg.pdsch_dmrs_type        = pdsch.dmrs_type;
  dci_cfg.pdsch_dmrs_max_len     = pdsch.dmrs_max_length;

  return dci_cfg;
}

bool phy_cfg_nr_t::assert_ss_id(uint32_t ss_id) const
{
  // Make sure SS access if bounded
  if (ss_id >= ISRRAN_UE_DL_NR_MAX_NOF_SEARCH_SPACE) {
    return false;
  }

  // Check SS is present
  if (not pdcch.search_space_present[ss_id]) {
    return false;
  }

  // Extract CORESET id
  uint32_t coreset_id = pdcch.search_space[ss_id].coreset_id;

  // Make sure CORESET id is bounded
  if (coreset_id >= ISRRAN_UE_DL_NR_MAX_NOF_CORESET) {
    return false;
  }

  // Check CORESET is present
  if (not pdcch.coreset_present[coreset_id]) {
    return false;
  }

  return true;
}

bool phy_cfg_nr_t::get_dci_locations(
    const uint32_t&                                                                           slot_idx,
    const uint16_t&                                                                           rnti,
    const uint32_t&                                                                           ss_id,
    const uint32_t&                                                                           L,
    isrran::bounded_vector<isrran_dci_location_t, ISRRAN_SEARCH_SPACE_MAX_NOF_CANDIDATES_NR>& locations) const
{
  // Assert search space
  if (not assert_ss_id(ss_id)) {
    return ISRRAN_ERROR;
  }

  // Select SS and CORESET
  const isrran_search_space_t& ss      = pdcch.search_space[ss_id];
  const isrran_coreset_t&      coreset = pdcch.coreset[ss.coreset_id];

  // Compute NCCE
  std::array<uint32_t, ISRRAN_SEARCH_SPACE_MAX_NOF_CANDIDATES_NR> ncce = {};
  int n = isrran_pdcch_nr_locations_coreset(&coreset, &ss, rnti, L, slot_idx, ncce.data());
  if (n < ISRRAN_SUCCESS) {
    return false;
  }

  // Push locations
  for (uint32_t i = 0; i < (uint32_t)n; i++) {
    locations.push_back({L, ncce[i]});
  }

  return true;
}

isrran_dci_format_nr_t phy_cfg_nr_t::get_dci_format_pdsch(uint32_t ss_id) const
{
  // Assert search space
  if (not assert_ss_id(ss_id)) {
    return ISRRAN_DCI_FORMAT_NR_COUNT;
  }

  // Select SS
  const isrran_search_space_t& ss = pdcch.search_space[ss_id];

  // Extract number of formats
  uint32_t nof_formats = ISRRAN_MIN(ss.nof_formats, ISRRAN_DCI_FORMAT_NR_COUNT);

  // Select DCI formats
  for (uint32_t i = 0; i < nof_formats; i++) {
    // Select DL format
    if (ss.formats[i] == isrran_dci_format_nr_1_0 or ss.formats[i] == isrran_dci_format_nr_1_1) {
      return ss.formats[i];
    }
  }

  // If reached here, no valid DCI format is available
  return ISRRAN_DCI_FORMAT_NR_COUNT;
}

isrran_dci_format_nr_t phy_cfg_nr_t::get_dci_format_pusch(uint32_t ss_id) const
{
  // Assert search space
  if (not assert_ss_id(ss_id)) {
    return ISRRAN_DCI_FORMAT_NR_COUNT;
  }

  // Select SS
  const isrran_search_space_t& ss = pdcch.search_space[ss_id];

  // Extract number of formats
  uint32_t nof_formats = ISRRAN_MIN(ss.nof_formats, ISRRAN_DCI_FORMAT_NR_COUNT);

  // Select DCI formats
  for (uint32_t i = 0; i < nof_formats; i++) {
    // Select DL format
    if (ss.formats[i] == isrran_dci_format_nr_0_0 or ss.formats[i] == isrran_dci_format_nr_0_1) {
      return ss.formats[i];
    }
  }

  // If reached here, no valid DCI format is available
  return ISRRAN_DCI_FORMAT_NR_COUNT;
}

bool phy_cfg_nr_t::get_dci_ctx_pdsch_rnti_c(uint32_t                     ss_id,
                                            const isrran_dci_location_t& location,
                                            const uint16_t&              rnti,
                                            isrran_dci_ctx_t&            ctx) const
{
  // Get DCI format, includes SS Id assertion
  isrran_dci_format_nr_t format = get_dci_format_pdsch(ss_id);
  if (format == ISRRAN_DCI_FORMAT_NR_COUNT) {
    return false;
  }

  // Select search space
  const isrran_search_space_t& ss = pdcch.search_space[ss_id];

  // Fill context
  ctx.location         = location;
  ctx.ss_type          = ss.type;
  ctx.coreset_id       = ss.coreset_id;
  ctx.coreset_start_rb = isrran_coreset_start_rb(&pdcch.coreset[ss.coreset_id]);
  ctx.rnti_type        = isrran_rnti_type_c;
  ctx.format           = format;
  ctx.rnti             = rnti;

  return true;
}

bool phy_cfg_nr_t::get_dci_ctx_pusch_rnti_c(uint32_t                     ss_id,
                                            const isrran_dci_location_t& location,
                                            const uint16_t&              rnti,
                                            isrran_dci_ctx_t&            ctx) const
{
  // Get DCI format, includes SS Id assertion
  isrran_dci_format_nr_t format = get_dci_format_pusch(ss_id);
  if (format == ISRRAN_DCI_FORMAT_NR_COUNT) {
    return false;
  }

  // Select search space
  const isrran_search_space_t& ss = pdcch.search_space[ss_id];

  // Fill context
  ctx.location         = location;
  ctx.ss_type          = ss.type;
  ctx.coreset_id       = ss.coreset_id;
  ctx.coreset_start_rb = isrran_coreset_start_rb(&pdcch.coreset[ss.coreset_id]);
  ctx.rnti_type        = isrran_rnti_type_c;
  ctx.format           = format;
  ctx.rnti             = rnti;

  return true;
}

bool phy_cfg_nr_t::get_pdsch_cfg(const isrran_slot_cfg_t&  slot_cfg,
                                 const isrran_dci_dl_nr_t& dci,
                                 isrran_sch_cfg_nr_t&      pdsch_cfg) const
{
  return isrran_ra_dl_dci_to_grant_nr(&carrier, &slot_cfg, &pdsch, &dci, &pdsch_cfg, &pdsch_cfg.grant) ==
         ISRRAN_SUCCESS;
}

bool phy_cfg_nr_t::get_pusch_cfg(const isrran_slot_cfg_t&  slot_cfg,
                                 const isrran_dci_ul_nr_t& dci,
                                 isrran_sch_cfg_nr_t&      pusch_cfg) const
{
  return isrran_ra_ul_dci_to_grant_nr(&carrier, &slot_cfg, &pusch, &dci, &pusch_cfg, &pusch_cfg.grant) ==
         ISRRAN_SUCCESS;
}

bool phy_cfg_nr_t::get_pdsch_ack_resource(const isrran_dci_dl_nr_t&   dci_dl,
                                          isrran_harq_ack_resource_t& ack_resource) const
{
  return isrran_harq_ack_resource(&harq_ack, &dci_dl, &ack_resource) == ISRRAN_SUCCESS;
}

bool phy_cfg_nr_t::get_uci_cfg(const isrran_slot_cfg_t&     slot_cfg,
                               const isrran_pdsch_ack_nr_t& pdsch_ack,
                               isrran_uci_cfg_nr_t&         uci_cfg) const
{
  // Generate configuration for HARQ feedback
  if (isrran_harq_ack_gen_uci_cfg(&harq_ack, &pdsch_ack, &uci_cfg) < ISRRAN_SUCCESS) {
    return false;
  }

  // Generate configuration for SR
  uint32_t sr_resource_id[ISRRAN_PUCCH_MAX_NOF_SR_RESOURCES] = {};
  int      n = isrran_ue_ul_nr_sr_send_slot(pucch.sr_resources, slot_cfg.idx, sr_resource_id);
  if (n < ISRRAN_SUCCESS) {
    ERROR("Calculating SR opportunities");
    return false;
  }

  if (n > 0) {
    uci_cfg.pucch.sr_resource_id = sr_resource_id[0];
    uci_cfg.o_sr                 = isrran_ra_ul_nr_nof_sr_bits((uint32_t)n);
    uci_cfg.sr_positive_present  = true;
  }

  // Generate configuration for CSI reports
  n = isrran_csi_reports_generate(&csi, &slot_cfg, uci_cfg.csi);
  if (n > ISRRAN_SUCCESS) {
    uci_cfg.nof_csi = (uint32_t)n;
  }

  return true;
}

bool phy_cfg_nr_t::get_pucch_uci_cfg(const isrran_slot_cfg_t&      slot_cfg,
                                     const isrran_uci_cfg_nr_t&    uci_cfg,
                                     isrran_pucch_nr_common_cfg_t& cfg,
                                     isrran_pucch_nr_resource_t&   resource) const
{
  // Select PUCCH resource
  if (isrran_ra_ul_nr_pucch_resource(&pucch, &uci_cfg, carrier.nof_prb, &resource) < ISRRAN_SUCCESS) {
    ERROR("Selecting PUCCH resource");
    return false;
  }

  return true;
}

bool phy_cfg_nr_t::get_pusch_uci_cfg(const isrran_slot_cfg_t&   slot_cfg,
                                     const isrran_uci_cfg_nr_t& uci_cfg,
                                     isrran_sch_cfg_nr_t&       pusch_cfg) const
{
  // Generate configuration for PUSCH
  if (isrran_ra_ul_set_grant_uci_nr(&carrier, &pusch, &uci_cfg, &pusch_cfg) < ISRRAN_SUCCESS) {
    return false;
  }

  return true;
}

isrran_ssb_cfg_t phy_cfg_nr_t::get_ssb_cfg() const
{
  // Retrieve band
  isrran::isrran_band_helper bh   = isrran::isrran_band_helper();
  uint16_t                   band = bh.get_band_from_dl_freq_Hz(carrier.dl_center_frequency_hz);
  isrran_assert(band != UINT16_MAX,
                "DL frequency %f MHz does not belong to any valid band",
                carrier.dl_center_frequency_hz / 1e6);

  // Make SSB configuration
  isrran_ssb_cfg_t ssb_cfg = {};
  ssb_cfg.center_freq_hz   = carrier.dl_center_frequency_hz;
  ssb_cfg.ssb_freq_hz      = carrier.ssb_center_freq_hz;
  ssb_cfg.scs              = ssb.scs;
  ssb_cfg.pattern          = bh.get_ssb_pattern(band, ssb.scs);
  ssb_cfg.duplex_mode      = duplex.mode;
  ssb_cfg.periodicity_ms   = ssb.periodicity_ms;

  isrran_assert(ssb_cfg.pattern != ISRRAN_SSB_PATTERN_INVALID,
                "Invalid SSB pattern for band %d and SSB subcarrier spacing %s",
                band,
                isrran_subcarrier_spacing_to_str(ssb.scs));

  return ssb_cfg;
}

} // namespace isrran
