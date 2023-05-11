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

#include "isrran/phy/enb/enb_ul.h"

#include "isrran/isrran.h"
#include <complex.h>
#include <math.h>
#include <string.h>

int isrran_enb_ul_init(isrran_enb_ul_t* q, cf_t* in_buffer, uint32_t max_prb)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL) {
    ret = ISRRAN_ERROR;

    bzero(q, sizeof(isrran_enb_ul_t));

    q->sf_symbols = isrran_vec_cf_malloc(ISRRAN_SF_LEN_RE(max_prb, ISRRAN_CP_NORM));
    if (!q->sf_symbols) {
      perror("malloc");
      goto clean_exit;
    }

    q->chest_res.ce = isrran_vec_cf_malloc(ISRRAN_SF_LEN_RE(max_prb, ISRRAN_CP_NORM));
    if (!q->chest_res.ce) {
      perror("malloc");
      goto clean_exit;
    }
    q->in_buffer = in_buffer;

    if (isrran_pucch_init_enb(&q->pucch)) {
      ERROR("Error creating PUCCH object");
      goto clean_exit;
    }

    if (isrran_pusch_init_enb(&q->pusch, max_prb)) {
      ERROR("Error creating PUSCH object");
      goto clean_exit;
    }

    if (isrran_chest_ul_init(&q->chest, max_prb)) {
      ERROR("Error initiating channel estimator");
      goto clean_exit;
    }

    ret = ISRRAN_SUCCESS;

  } else {
    ERROR("Invalid parameters");
  }

clean_exit:
  if (ret == ISRRAN_ERROR) {
    isrran_enb_ul_free(q);
  }
  return ret;
}

void isrran_enb_ul_free(isrran_enb_ul_t* q)
{
  if (q) {
    isrran_ofdm_rx_free(&q->fft);
    isrran_pucch_free(&q->pucch);
    isrran_pusch_free(&q->pusch);
    isrran_chest_ul_free(&q->chest);

    if (q->sf_symbols) {
      free(q->sf_symbols);
    }
    if (q->chest_res.ce) {
      free(q->chest_res.ce);
    }
    bzero(q, sizeof(isrran_enb_ul_t));
  }
}

int isrran_enb_ul_set_cell(isrran_enb_ul_t*                   q,
                           isrran_cell_t                      cell,
                           isrran_refsignal_dmrs_pusch_cfg_t* pusch_cfg,
                           isrran_refsignal_isr_cfg_t*        isr_cfg)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && isrran_cell_isvalid(&cell)) {
    if (cell.id != q->cell.id || q->cell.nof_prb == 0) {
      q->cell = cell;

      isrran_ofdm_cfg_t ofdm_cfg = {};
      ofdm_cfg.nof_prb           = q->cell.nof_prb;
      ofdm_cfg.in_buffer         = q->in_buffer;
      ofdm_cfg.out_buffer        = q->sf_symbols;
      ofdm_cfg.cp                = q->cell.cp;
      ofdm_cfg.freq_shift_f      = -0.5f;
      ofdm_cfg.normalize         = false;
      ofdm_cfg.rx_window_offset  = 0.5f;
      if (isrran_ofdm_rx_init_cfg(&q->fft, &ofdm_cfg)) {
        ERROR("Error initiating FFT");
        return ISRRAN_ERROR;
      }
      if (isrran_ofdm_rx_set_prb(&q->fft, q->cell.cp, q->cell.nof_prb)) {
        ERROR("Error initiating FFT");
        return ISRRAN_ERROR;
      }

      if (isrran_pucch_set_cell(&q->pucch, q->cell)) {
        ERROR("Error creating PUCCH object");
        return ISRRAN_ERROR;
      }

      if (isrran_pusch_set_cell(&q->pusch, q->cell)) {
        ERROR("Error creating PUSCH object");
        return ISRRAN_ERROR;
      }

      if (isrran_chest_ul_set_cell(&q->chest, cell)) {
        ERROR("Error initiating channel estimator");
        return ISRRAN_ERROR;
      }

      // ISR is a dedicated configuration
      isrran_chest_ul_pregen(&q->chest, pusch_cfg, isr_cfg);

      ret = ISRRAN_SUCCESS;
    }
  } else {
    ERROR("Invalid cell properties: Id=%d, Ports=%d, PRBs=%d", cell.id, cell.nof_ports, cell.nof_prb);
  }
  return ret;
}

void isrran_enb_ul_fft(isrran_enb_ul_t* q)
{
  isrran_ofdm_rx_sf(&q->fft);
}

static int get_pucch(isrran_enb_ul_t* q, isrran_ul_sf_cfg_t* ul_sf, isrran_pucch_cfg_t* cfg, isrran_pucch_res_t* res)
{
  int      ret                               = ISRRAN_SUCCESS;
  uint32_t n_pucch_i[ISRRAN_PUCCH_MAX_ALLOC] = {};
  uint32_t uci_cfg_total_ack                 = isrran_uci_cfg_total_ack(&cfg->uci_cfg);

  // Drop CQI if there is collision with ACK
  if (!cfg->simul_cqi_ack && uci_cfg_total_ack > 0 && cfg->uci_cfg.cqi.data_enable) {
    cfg->uci_cfg.cqi.data_enable = false;
  }

  // Select format
  cfg->format = isrran_pucch_proc_select_format(&q->cell, cfg, &cfg->uci_cfg, NULL);
  if (cfg->format == ISRRAN_PUCCH_FORMAT_ERROR) {
    ERROR("Returned Error while selecting PUCCH format");
    return ISRRAN_ERROR;
  }

  // Get possible resources
  int nof_resources = isrran_pucch_proc_get_resources(&q->cell, cfg, &cfg->uci_cfg, NULL, n_pucch_i);
  if (nof_resources < 1 || nof_resources > ISRRAN_PUCCH_CS_MAX_ACK) {
    ERROR("No PUCCH resource could be calculated (%d)", nof_resources);
    return ISRRAN_ERROR;
  }

  // Initialise minimum correlation
  res->correlation = 0.0f;

  // Iterate possible resources and select the one with higher correlation
  for (int i = 0; i < nof_resources && ret == ISRRAN_SUCCESS; i++) {
    isrran_pucch_res_t pucch_res = {};

    // Configure resource
    cfg->n_pucch = n_pucch_i[i];

    // Prepare configuration
    if (isrran_chest_ul_estimate_pucch(&q->chest, ul_sf, cfg, q->sf_symbols, &q->chest_res)) {
      ERROR("Error estimating PUCCH DMRS");
      return ISRRAN_ERROR;
    }
    pucch_res.snr_db    = q->chest_res.snr_db;
    pucch_res.rssi_dbFs = q->chest_res.epre_dBfs;
    pucch_res.ni_dbFs   = q->chest_res.noise_estimate_dbFs;

    ret = isrran_pucch_decode(&q->pucch, ul_sf, cfg, &q->chest_res, q->sf_symbols, &pucch_res);
    if (ret < ISRRAN_SUCCESS) {
      ERROR("Error decoding PUCCH");
    } else {
      // Get PUCCH Format 1b with channel selection if:
      // - At least one ACK bit needs to be received; and
      // - PUCCH Format 1b was used; and
      // - HARQ feedback mode is set to PUCCH Format1b with Channel Selection (CS); and
      // - No scheduling request is expected; and
      // - Data is valid (invalid data does not make sense to decode).
      if (uci_cfg_total_ack > 0 && cfg->format == ISRRAN_PUCCH_FORMAT_1B &&
          cfg->ack_nack_feedback_mode == ISRRAN_PUCCH_ACK_NACK_FEEDBACK_MODE_CS &&
          !cfg->uci_cfg.is_scheduling_request_tti && pucch_res.uci_data.ack.valid) {
        uint8_t b[2] = {pucch_res.uci_data.ack.ack_value[0], pucch_res.uci_data.ack.ack_value[1]};
        isrran_pucch_cs_get_ack(cfg, &cfg->uci_cfg, i, b, &pucch_res.uci_data);
      }

      // Compares correlation value, it stores the PUCCH result with the greatest correlation
      if (i == 0 || pucch_res.correlation > res->correlation) {
        // Copy measurements only if PUCCH was decoded successfully
        if (cfg->meas_ta_en) {
          pucch_res.ta_valid = !(isnan(q->chest_res.ta_us) || isinf(q->chest_res.ta_us));
          pucch_res.ta_us    = q->chest_res.ta_us;
        }

        *res = pucch_res;
      }
    }
  }

  return ret;
}

int isrran_enb_ul_get_pucch(isrran_enb_ul_t*    q,
                            isrran_ul_sf_cfg_t* ul_sf,
                            isrran_pucch_cfg_t* cfg,
                            isrran_pucch_res_t* res)
{
  if (!isrran_pucch_cfg_isvalid(cfg, q->cell.nof_prb)) {
    ERROR("Invalid PUCCH configuration");
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (get_pucch(q, ul_sf, cfg, res)) {
    return ISRRAN_ERROR;
  }

  // If we are looking for SR and ACK at the same time and ret=0, means there is no SR.
  // try again to decode ACK only
  if (cfg->uci_cfg.is_scheduling_request_tti && isrran_uci_cfg_total_ack(&cfg->uci_cfg)) {
    // Disable SR
    cfg->uci_cfg.is_scheduling_request_tti = false;

    // Init PUCCH result without SR
    isrran_pucch_res_t res_no_sr = {};

    // Actual decode without SR
    if (get_pucch(q, ul_sf, cfg, &res_no_sr)) {
      return ISRRAN_ERROR;
    }

    // Override PUCCH result if PUCCH without SR was detected, and
    // - no PUCCH with SR was detected; or
    // - PUCCH without SR has better correlation
    if (res_no_sr.detected && (!res->detected || res_no_sr.correlation > res->correlation)) {
      *res = res_no_sr;
    } else {
      // If the PUCCH decode result is not overridden, flag SR
      cfg->uci_cfg.is_scheduling_request_tti = true;
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_enb_ul_get_pusch(isrran_enb_ul_t*    q,
                            isrran_ul_sf_cfg_t* ul_sf,
                            isrran_pusch_cfg_t* cfg,
                            isrran_pusch_res_t* res)
{
  isrran_chest_ul_estimate_pusch(&q->chest, ul_sf, cfg, q->sf_symbols, &q->chest_res);

  return isrran_pusch_decode(&q->pusch, ul_sf, cfg, &q->chest_res, q->sf_symbols, res);
}
