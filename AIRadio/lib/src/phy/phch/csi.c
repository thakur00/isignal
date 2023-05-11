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
#include "isrran/phy/phch/csi.h"
#include "isrran/phy/utils/bit.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <math.h>

#define CSI_WIDEBAND_CSI_NOF_BITS 4

#define CSI_DEFAULT_ALPHA 0.5f

/// Implements SNRI to CQI conversion
uint32_t csi_snri_db_to_cqi(isrran_csi_cqi_table_t table, float snri_db)
{
  return 15;
}

// Implements CSI report triggers
static bool csi_report_trigger(const isrran_csi_hl_report_cfg_t* cfg, uint32_t slot_idx)
{
  switch (cfg->type) {
    case ISRRAN_CSI_REPORT_TYPE_PERIODIC:
      return (slot_idx + cfg->periodic.period - cfg->periodic.offset) % cfg->periodic.period == 0;
    default:; // Do nothing
  }
  return false;
}

static void csi_wideband_cri_ri_pmi_cqi_quantify(const isrran_csi_hl_report_cfg_t*        cfg,
                                                 const isrran_csi_channel_measurements_t* channel_meas,
                                                 const isrran_csi_channel_measurements_t* interf_meas,
                                                 isrran_csi_report_value_t*               report_value)
{
  // Take SNR by default
  float wideband_sinr_db = channel_meas->wideband_snr_db;

  // If interference is provided, use the channel RSRP and interference EPRE to calculate the SINR
  if (interf_meas != NULL) {
    wideband_sinr_db = channel_meas->wideband_rsrp_dBm - interf_meas->wideband_epre_dBm;
  }

  // Fill quantified values
  report_value->wideband_cri_ri_pmi_cqi.cqi = csi_snri_db_to_cqi(cfg->cqi_table, wideband_sinr_db);
  report_value->wideband_cri_ri_pmi_cqi.ri  = 0;
  report_value->wideband_cri_ri_pmi_cqi.pmi = 0;
}

static uint32_t csi_wideband_cri_ri_pmi_cqi_nof_bits(const isrran_csi_report_cfg_t* cfg)
{
  // Compute number of bits for CRI
  uint32_t nof_bits_cri = 0;
  if (cfg->K_csi_rs > 0) {
    nof_bits_cri = (uint32_t)ceilf(log2f((float)cfg->K_csi_rs));
  }

  switch (cfg->nof_ports) {
    case 1:
      return CSI_WIDEBAND_CSI_NOF_BITS + nof_bits_cri;
    default:
      ERROR("Invalid or not implemented number of ports (%d)", cfg->nof_ports);
  }
  return 0;
}

static uint32_t csi_wideband_cri_ri_pmi_cqi_pack(const isrran_csi_report_cfg_t*   cfg,
                                                 const isrran_csi_report_value_t* value,
                                                 uint8_t*                         o_csi1)
{
  // Compute number of bits for CRI
  uint32_t nof_bits_cri = 0;
  if (cfg->K_csi_rs > 0) {
    nof_bits_cri = (uint32_t)ceilf(log2f((float)cfg->K_csi_rs));
  }

  // Write wideband CQI
  isrran_bit_unpack(value->wideband_cri_ri_pmi_cqi.cqi, &o_csi1, CSI_WIDEBAND_CSI_NOF_BITS);

  // Compute number of bits for CRI and write
  isrran_bit_unpack(value->cri, &o_csi1, nof_bits_cri);

  return nof_bits_cri + CSI_WIDEBAND_CSI_NOF_BITS;
}

static uint32_t csi_wideband_cri_ri_pmi_cqi_unpack(const isrran_csi_report_cfg_t* cfg,
                                                   uint8_t*                       o_csi1,
                                                   isrran_csi_report_value_t*     value)
{
  // Compute number of bits for CRI
  uint32_t nof_bits_cri = 0;
  if (cfg->K_csi_rs > 0) {
    nof_bits_cri = (uint32_t)ceilf(log2f((float)cfg->K_csi_rs));
  }

  // Write wideband CQI
  value->wideband_cri_ri_pmi_cqi.cqi = isrran_bit_pack(&o_csi1, CSI_WIDEBAND_CSI_NOF_BITS);

  // Compute number of bits for CRI and write
  value->cri = isrran_bit_pack(&o_csi1, nof_bits_cri);

  return nof_bits_cri + CSI_WIDEBAND_CSI_NOF_BITS;
}

static uint32_t csi_none_nof_bits(const isrran_csi_report_cfg_t* cfg)
{
  return cfg->K_csi_rs;
}

static uint32_t
csi_none_pack(const isrran_csi_report_cfg_t* cfg, const isrran_csi_report_value_t* value, uint8_t* o_csi1)
{
  isrran_vec_u8_copy(o_csi1, (uint8_t*)value->none, cfg->K_csi_rs);

  return cfg->K_csi_rs;
}

static uint32_t
csi_none_unpack(const isrran_csi_report_cfg_t* cfg, const uint8_t* o_csi1, isrran_csi_report_value_t* value)
{
  isrran_vec_u8_copy((uint8_t*)value->none, o_csi1, cfg->K_csi_rs);

  return cfg->K_csi_rs;
}

int isrran_csi_new_nzp_csi_rs_measurement(
    const isrran_csi_hl_resource_cfg_t       csi_resources[ISRRAN_CSI_MAX_NOF_RESOURCES],
    isrran_csi_channel_measurements_t        measurements[ISRRAN_CSI_MAX_NOF_RESOURCES],
    const isrran_csi_channel_measurements_t* new_measure,
    uint32_t                                 nzp_csi_rs_id)
{
  if (csi_resources == NULL || measurements == NULL || new_measure == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Iterate all resources
  for (uint32_t res_idx = 0; res_idx < ISRRAN_CSI_MAX_NOF_RESOURCES; res_idx++) {
    // Skip resource is set to none
    if (csi_resources[res_idx].type != ISRRAN_CSI_HL_RESOURCE_CFG_TYPE_NZP_CSI_RS_SSB) {
      continue;
    }

    // Look for the NZP-CSI reource set
    bool present = false;
    for (uint32_t i = 0; i < ISRRAN_CSI_MAX_NOF_NZP_CSI_RS_RESOURCE_SETS_X_CONFIG && !present; i++) {
      if (csi_resources[res_idx].nzp_csi_rs_ssb.nzp_csi_rs_resource_set_id_list[i] == nzp_csi_rs_id) {
        present = true;
      }
    }

    // Skip Resource if it does not contain the NZP-CSI-RS set
    if (!present) {
      continue;
    }

    // Filter measurements
    measurements[res_idx].wideband_rsrp_dBm =
        ISRRAN_VEC_SAFE_EMA(new_measure->wideband_rsrp_dBm, measurements[res_idx].wideband_rsrp_dBm, CSI_DEFAULT_ALPHA);
    measurements[res_idx].wideband_epre_dBm =
        ISRRAN_VEC_SAFE_EMA(new_measure->wideband_epre_dBm, measurements[res_idx].wideband_epre_dBm, CSI_DEFAULT_ALPHA);
    measurements[res_idx].wideband_snr_db =
        ISRRAN_VEC_SAFE_EMA(new_measure->wideband_snr_db, measurements[res_idx].wideband_snr_db, CSI_DEFAULT_ALPHA);

    // Force rest
    measurements[res_idx].cri      = new_measure->cri;
    measurements[res_idx].K_csi_rs = new_measure->K_csi_rs;
  }

  return ISRRAN_SUCCESS;
}

int isrran_csi_reports_generate(const isrran_csi_hl_cfg_t* cfg,
                                const isrran_slot_cfg_t*   slot_cfg,
                                isrran_csi_report_cfg_t    report_cfg[ISRRAN_CSI_SLOT_MAX_NOF_REPORT])
{
  uint32_t count = 0;

  // Check inputs
  if (cfg == NULL || report_cfg == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Make sure report configuration is initialised to zero
  ISRRAN_MEM_ZERO(report_cfg, isrran_csi_report_cfg_t, ISRRAN_CSI_SLOT_MAX_NOF_REPORT);

  // Iterate every possible configured CSI report
  for (uint32_t i = 0; i < ISRRAN_CSI_MAX_NOF_REPORT; i++) {
    // Skip if report is not configured or triggered
    if (!csi_report_trigger(&cfg->reports[i], slot_cfg->idx)) {
      continue;
    }

    if (count >= ISRRAN_CSI_SLOT_MAX_NOF_REPORT) {
      ERROR("The number of CSI reports in the slot (%d) exceeds the maximum (%d)",
            count++,
            ISRRAN_CSI_SLOT_MAX_NOF_REPORT);
      return ISRRAN_ERROR;
    }

    // Configure report
    report_cfg[count].cfg       = cfg->reports[i];
    report_cfg[count].nof_ports = 1;
    report_cfg[count].K_csi_rs  = 1;
    report_cfg[count].has_part2 = false;
    count++;
  }

  return (int)count;
}

int isrran_csi_reports_quantify(const isrran_csi_report_cfg_t           reports[ISRRAN_CSI_SLOT_MAX_NOF_REPORT],
                                const isrran_csi_channel_measurements_t measurements[ISRRAN_CSI_MAX_NOF_RESOURCES],
                                isrran_csi_report_value_t               report_value[ISRRAN_CSI_SLOT_MAX_NOF_REPORT])
{
  uint32_t count = 0;

  // Check inputs
  if (reports == NULL || measurements == NULL || report_value == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Iterate every possible configured CSI report
  for (uint32_t i = 0; i < ISRRAN_CSI_SLOT_MAX_NOF_REPORT; i++) {
    // If the report is the last one, break
    if (reports->cfg.type == ISRRAN_CSI_REPORT_TYPE_NONE) {
      break;
    }

    // Select channel measurement
    uint32_t channel_meas_id = reports[i].cfg.channel_meas_id;
    if (channel_meas_id >= ISRRAN_CSI_MAX_NOF_RESOURCES) {
      ERROR("Channel measurement ID (%d) is out of range", channel_meas_id);
      return ISRRAN_ERROR;
    }
    const isrran_csi_channel_measurements_t* channel_meas = &measurements[channel_meas_id];

    // Select interference measurement
    const isrran_csi_channel_measurements_t* interf_meas = NULL;
    if (reports[i].cfg.interf_meas_present) {
      uint32_t interf_meas_id = reports[i].cfg.interf_meas_id;
      if (interf_meas_id >= ISRRAN_CSI_MAX_NOF_RESOURCES) {
        ERROR("Interference measurement ID (%d) is out of range", interf_meas_id);
        return ISRRAN_ERROR;
      }
      interf_meas = &measurements[interf_meas_id];
    }

    // Quantify measurements according to frequency and quantity configuration
    if (reports[i].cfg.freq_cfg == ISRRAN_CSI_REPORT_FREQ_WIDEBAND &&
        reports[i].cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_CRI_RI_PMI_CQI) {
      csi_wideband_cri_ri_pmi_cqi_quantify(&reports[i].cfg, channel_meas, interf_meas, &report_value[count]);
      count++;
    } else {
      ; // Ignore other types
    }
  }

  return (int)count;
}

int isrran_csi_part1_nof_bits(const isrran_csi_report_cfg_t* report_list, uint32_t nof_reports)
{
  uint32_t count = 0;

  // Check input pointer
  if (report_list == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Iterate all report configurations
  for (uint32_t i = 0; i < nof_reports; i++) {
    const isrran_csi_report_cfg_t* report = &report_list[i];
    if (report->cfg.quantity && report->cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_CRI_RI_PMI_CQI) {
      count += csi_wideband_cri_ri_pmi_cqi_nof_bits(report);
    } else if (report->cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_NONE) {
      count += csi_none_nof_bits(report);
    }
  }

  return (int)count;
}

int isrran_csi_part2_nof_bits(const isrran_csi_report_cfg_t* report_list, uint32_t nof_reports)
{
  uint32_t count = 0;

  // Check input pointer
  if (report_list == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  return (int)count;
}

bool isrran_csi_has_part2(const isrran_csi_report_cfg_t* report_list, uint32_t nof_reports)
{
  if (report_list == NULL || nof_reports == 0) {
    return false;
  }

  for (uint32_t i = 0; i < nof_reports; i++) {
    if (report_list[i].has_part2) {
      return true;
    }
  }
  return false;
}

int isrran_csi_part1_pack(const isrran_csi_report_cfg_t*   report_cfg,
                          const isrran_csi_report_value_t* report_value,
                          uint32_t                         nof_reports,
                          uint8_t*                         o_csi1,
                          uint32_t                         max_o_csi1)
{
  uint32_t count = 0;

  if (report_cfg == NULL || report_value == NULL || o_csi1 == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  int n = isrran_csi_part1_nof_bits(report_cfg, nof_reports);
  if (n > (int)max_o_csi1) {
    ERROR("The maximum number of CSI bits (%d) is not enough to accommodate %d bits", max_o_csi1, n);
    return ISRRAN_ERROR;
  }

  for (uint32_t i = 0; i < nof_reports && count < max_o_csi1; i++) {
    if (report_cfg[i].cfg.freq_cfg == ISRRAN_CSI_REPORT_FREQ_WIDEBAND &&
        report_cfg[i].cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_CRI_RI_PMI_CQI) {
      count += csi_wideband_cri_ri_pmi_cqi_pack(&report_cfg[i], &report_value[i], &o_csi1[count]);
    } else if (report_cfg[i].cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_NONE) {
      count += csi_none_pack(&report_cfg[i], &report_value[i], &o_csi1[count]);
    } else {
      ERROR("CSI frequency (%d) and quantity (%d) combination is not implemented",
            report_cfg[i].cfg.freq_cfg,
            report_cfg[i].cfg.quantity);
    }
  }

  return (int)count;
}

int isrran_csi_part1_unpack(const isrran_csi_report_cfg_t* report_cfg,
                            uint32_t                       nof_reports,
                            uint8_t*                       o_csi1,
                            uint32_t                       max_o_csi1,
                            isrran_csi_report_value_t*     report_value)
{
  uint32_t count = 0;

  if (report_cfg == NULL || report_value == NULL || o_csi1 == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  int n = isrran_csi_part1_nof_bits(report_cfg, nof_reports);
  if (n > (int)max_o_csi1) {
    ERROR("The maximum number of CSI bits (%d) is not enough to accommodate %d bits", max_o_csi1, n);
    return ISRRAN_ERROR;
  }

  for (uint32_t i = 0; i < nof_reports && count < max_o_csi1; i++) {
    if (report_cfg[i].cfg.freq_cfg == ISRRAN_CSI_REPORT_FREQ_WIDEBAND &&
        report_cfg[i].cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_CRI_RI_PMI_CQI) {
      count += csi_wideband_cri_ri_pmi_cqi_unpack(&report_cfg[i], &o_csi1[count], &report_value[i]);
    } else if (report_cfg[i].cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_NONE) {
      count += csi_none_unpack(&report_cfg[i], &o_csi1[count], &report_value[i]);
    } else {
      ERROR("CSI frequency (%d) and quantity (%d) combination is not implemented",
            report_cfg[i].cfg.freq_cfg,
            report_cfg[i].cfg.quantity);
    }
  }

  return (int)count;
}

uint32_t isrran_csi_str(const isrran_csi_report_cfg_t*   report_cfg,
                        const isrran_csi_report_value_t* report_value,
                        uint32_t                         nof_reports,
                        char*                            str,
                        uint32_t                         str_len)
{
  uint32_t len = 0;
  for (uint32_t i = 0; i < nof_reports; i++) {
    if (report_cfg[i].cfg.freq_cfg == ISRRAN_CSI_REPORT_FREQ_WIDEBAND &&
        report_cfg[i].cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_CRI_RI_PMI_CQI) {
      len = isrran_print_check(str, str_len, len, "cqi=%d ", report_value[i].wideband_cri_ri_pmi_cqi.cqi);
    } else if (report_cfg[i].cfg.quantity == ISRRAN_CSI_REPORT_QUANTITY_NONE) {
      char tmp[20] = {};
      isrran_vec_sprint_bin(tmp, sizeof(tmp), report_value[i].none, report_cfg->K_csi_rs);
      len = isrran_print_check(str, str_len, len, "csi=%s ", tmp);
    }
  }
  return len;
}
