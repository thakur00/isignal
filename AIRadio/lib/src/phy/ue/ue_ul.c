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

#include "isrran/isrran.h"
#include <complex.h>
#include <math.h>
#include <string.h>

#include "isrran/phy/ue/ue_ul.h"

#define CURRENT_FFTSIZE isrran_symbol_sz(q->cell.nof_prb)
#define CURRENT_SFLEN ISRRAN_SF_LEN(CURRENT_FFTSIZE)

#define CURRENT_SLOTLEN_RE ISRRAN_SLOT_LEN_RE(q->cell.nof_prb, q->cell.cp)
#define CURRENT_SFLEN_RE ISRRAN_NOF_RE(q->cell)

#define MAX_SFLEN ISRRAN_SF_LEN(isrran_symbol_sz(max_prb))

#define DEFAULT_CFO_TOL 1.0 // Hz

static bool isr_tx_enabled(isrran_refsignal_isr_cfg_t* isr_cfg, uint32_t tti);

int isrran_ue_ul_init(isrran_ue_ul_t* q, cf_t* out_buffer, uint32_t max_prb)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL) {
    ret = ISRRAN_ERROR;

    bzero(q, sizeof(isrran_ue_ul_t));

    q->sf_symbols = isrran_vec_cf_malloc(ISRRAN_SF_LEN_PRB(max_prb));
    if (!q->sf_symbols) {
      perror("malloc");
      goto clean_exit;
    }

    isrran_ofdm_cfg_t ofdm_cfg = {};
    ofdm_cfg.nof_prb           = max_prb;
    ofdm_cfg.in_buffer         = q->sf_symbols;
    ofdm_cfg.out_buffer        = out_buffer;
    ofdm_cfg.cp                = ISRRAN_CP_NORM;
    ofdm_cfg.freq_shift_f      = 0.5f;
    ofdm_cfg.normalize         = true;
    if (isrran_ofdm_tx_init_cfg(&q->fft, &ofdm_cfg)) {
      ERROR("Error initiating FFT");
      goto clean_exit;
    }

    if (isrran_cfo_init(&q->cfo, MAX_SFLEN)) {
      ERROR("Error creating CFO object");
      goto clean_exit;
    }

    if (isrran_pusch_init_ue(&q->pusch, max_prb)) {
      ERROR("Error creating PUSCH object");
      goto clean_exit;
    }
    if (isrran_pucch_init_ue(&q->pucch)) {
      ERROR("Error creating PUSCH object");
      goto clean_exit;
    }
    q->refsignal = isrran_vec_cf_malloc(2 * ISRRAN_NRE * max_prb);
    if (!q->refsignal) {
      perror("malloc");
      goto clean_exit;
    }

    q->isr_signal = isrran_vec_cf_malloc(ISRRAN_NRE * max_prb);
    if (!q->isr_signal) {
      perror("malloc");
      goto clean_exit;
    }
    q->out_buffer           = out_buffer;
    q->signals_pregenerated = false;
    ret                     = ISRRAN_SUCCESS;
  } else {
    ERROR("Invalid parameters");
  }

clean_exit:
  if (ret == ISRRAN_ERROR) {
    isrran_ue_ul_free(q);
  }
  return ret;
}

void isrran_ue_ul_free(isrran_ue_ul_t* q)
{
  if (q) {
    isrran_ofdm_tx_free(&q->fft);
    isrran_pusch_free(&q->pusch);
    isrran_pucch_free(&q->pucch);

    isrran_cfo_free(&q->cfo);

    if (q->sf_symbols) {
      free(q->sf_symbols);
    }
    if (q->refsignal) {
      free(q->refsignal);
    }
    if (q->isr_signal) {
      free(q->isr_signal);
    }
    if (q->signals_pregenerated) {
      isrran_refsignal_dmrs_pusch_pregen_free(&q->signals, &q->pregen_dmrs);
      isrran_refsignal_isr_pregen_free(&q->signals, &q->pregen_isr);
    }
    isrran_ra_ul_pusch_hopping_free(&q->hopping);

    bzero(q, sizeof(isrran_ue_ul_t));
  }
}

int isrran_ue_ul_set_cell(isrran_ue_ul_t* q, isrran_cell_t cell)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && isrran_cell_isvalid(&cell)) {
    if (q->cell.id != cell.id || q->cell.nof_prb == 0) {
      q->cell = cell;

      if (isrran_ofdm_tx_set_prb(&q->fft, q->cell.cp, q->cell.nof_prb)) {
        ERROR("Error resizing FFT");
        return ISRRAN_ERROR;
      }
      if (isrran_cfo_resize(&q->cfo, ISRRAN_SF_LEN_PRB(q->cell.nof_prb))) {
        ERROR("Error resizing CFO object");
        return ISRRAN_ERROR;
      }

      if (isrran_pusch_set_cell(&q->pusch, q->cell)) {
        ERROR("Error resizing PUSCH object");
        return ISRRAN_ERROR;
      }
      if (isrran_pucch_set_cell(&q->pucch, q->cell)) {
        ERROR("Error resizing PUSCH object");
        return ISRRAN_ERROR;
      }
      if (isrran_refsignal_ul_set_cell(&q->signals, q->cell)) {
        ERROR("Error resizing isrran_refsignal_ul");
        return ISRRAN_ERROR;
      }

      if (isrran_ra_ul_pusch_hopping_init(&q->hopping, q->cell)) {
        ERROR("Error setting hopping procedure cell");
        return ISRRAN_ERROR;
      }
      q->signals_pregenerated = false;
    }
    ret = ISRRAN_SUCCESS;
  } else {
    ERROR("Invalid cell properties ue_ul: Id=%d, Ports=%d, PRBs=%d", cell.id, cell.nof_ports, cell.nof_prb);
  }
  return ret;
}

int isrran_ue_ul_set_cfr(isrran_ue_ul_t* q, const isrran_cfr_cfg_t* cfr)
{
  if (q == NULL || cfr == NULL) {
    ERROR("Error, invalid inputs");
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Copy the cfr config into the UE
  q->cfr_config = *cfr;

  // Set the cfr for the fft's
  if (isrran_ofdm_set_cfr(&q->fft, &q->cfr_config) < ISRRAN_SUCCESS) {
    ERROR("Error setting the CFR for the fft");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_ue_ul_pregen_signals(isrran_ue_ul_t* q, isrran_ue_ul_cfg_t* cfg)
{
  if (q->signals_pregenerated) {
    isrran_refsignal_dmrs_pusch_pregen_free(&q->signals, &q->pregen_dmrs);
    isrran_refsignal_isr_pregen_free(&q->signals, &q->pregen_isr);
  }
  if (isrran_refsignal_dmrs_pusch_pregen(&q->signals, &q->pregen_dmrs, &cfg->ul_cfg.dmrs)) {
    return ISRRAN_ERROR;
  }
  if (isrran_refsignal_isr_pregen(&q->signals, &q->pregen_isr, &cfg->ul_cfg.isr, &cfg->ul_cfg.dmrs)) {
    return ISRRAN_ERROR;
  }
  q->signals_pregenerated = true;
  return ISRRAN_SUCCESS;
}

int isrran_ue_ul_dci_to_pusch_grant(isrran_ue_ul_t*       q,
                                    isrran_ul_sf_cfg_t*   sf,
                                    isrran_ue_ul_cfg_t*   cfg,
                                    isrran_dci_ul_t*      dci,
                                    isrran_pusch_grant_t* grant)
{
  // Convert DCI to Grant
  if (isrran_ra_ul_dci_to_grant(&q->cell, sf, &cfg->ul_cfg.hopping, dci, grant)) {
    return ISRRAN_ERROR;
  }

  // Update shortened before computing grant
  isrran_refsignal_isr_pusch_shortened(&q->signals, sf, &cfg->ul_cfg.isr, &cfg->ul_cfg.pusch);

  // Update RE assuming if shortened is true
  if (sf->shortened) {
    isrran_ra_ul_compute_nof_re(grant, q->cell.cp, true);
  }

  // Assert Grant is valid
  return isrran_pusch_assert_grant(grant);
}

void isrran_ue_ul_pusch_hopping(isrran_ue_ul_t*       q,
                                isrran_ul_sf_cfg_t*   sf,
                                isrran_ue_ul_cfg_t*   cfg,
                                isrran_pusch_grant_t* grant)
{
  if (cfg->ul_cfg.isr.configured && cfg->ul_cfg.hopping.hopping_enabled) {
    ERROR("UL ISR and frequency hopping not currently supported");
  }
  return isrran_ra_ul_pusch_hopping(&q->hopping, sf, &cfg->ul_cfg.hopping, grant);
}

static float limit_norm_factor(isrran_ue_ul_t* q, float norm_factor, cf_t* output_signal)
{
  uint32_t p   = isrran_vec_max_abs_fi((float*)output_signal, 2 * ISRRAN_SF_LEN_PRB(q->cell.nof_prb));
  float    amp = fabsf(*((float*)output_signal + p));

  if (amp * norm_factor > 0.95) {
    norm_factor = 0.95 / amp;
  }
  if (amp * norm_factor < 0.1) {
    norm_factor = 0.1 / amp;
  }
  return norm_factor;
}

static void apply_cfo(isrran_ue_ul_t* q, isrran_ue_ul_cfg_t* cfg)
{
  if (cfg->cfo_en) {
    isrran_cfo_set_tol(&q->cfo, cfg->cfo_tol / (15000.0 * isrran_symbol_sz(q->cell.nof_prb)));
    isrran_cfo_correct(&q->cfo, q->out_buffer, q->out_buffer, cfg->cfo_value / isrran_symbol_sz(q->cell.nof_prb));
  }
}

static void apply_norm(isrran_ue_ul_t* q, isrran_ue_ul_cfg_t* cfg, float norm_factor)
{
  uint32_t sf_len               = ISRRAN_SF_LEN_PRB(q->cell.nof_prb);
  float*   buf                  = NULL;
  float    force_peak_amplitude = cfg->force_peak_amplitude > 0 ? cfg->force_peak_amplitude : 1.0f;

  switch (cfg->normalize_mode) {
    case ISRRAN_UE_UL_NORMALIZE_MODE_AUTO:
    default:
      // Automatic normalization (default)
      norm_factor = limit_norm_factor(q, norm_factor, q->out_buffer);
      isrran_vec_sc_prod_cfc(q->out_buffer, norm_factor, q->out_buffer, sf_len);
      break;
    case ISRRAN_UE_UL_NORMALIZE_MODE_FORCE_AMPLITUDE:
      // Force amplitude
      // Typecast buffer
      buf = (float*)q->out_buffer;

      // Get index of maximum absolute sample
      uint32_t idx = isrran_vec_max_abs_fi(buf, sf_len * 2);

      // Get maximum value
      float scale = fabsf(buf[idx]);

      // Avoid zero division
      if (scale != 0.0f && scale != INFINITY) {
        // Apply maximum peak amplitude
        isrran_vec_sc_prod_cfc(q->out_buffer, force_peak_amplitude / scale, q->out_buffer, sf_len);
      }
      break;
  }
}

static void add_isr(isrran_ue_ul_t* q, isrran_ue_ul_cfg_t* cfg, uint32_t tti)
{
  if (isr_tx_enabled(&cfg->ul_cfg.isr, tti)) {
    if (q->signals_pregenerated) {
      isrran_refsignal_isr_pregen_put(&q->signals, &q->pregen_isr, &cfg->ul_cfg.isr, tti, q->sf_symbols);
    } else {
      isrran_refsignal_isr_gen(&q->signals, &cfg->ul_cfg.isr, &cfg->ul_cfg.dmrs, tti % 10, q->isr_signal);
      isrran_refsignal_isr_put(&q->signals, &cfg->ul_cfg.isr, tti, q->isr_signal, q->sf_symbols);
    }
  }
}

static int pusch_encode(isrran_ue_ul_t* q, isrran_ul_sf_cfg_t* sf, isrran_ue_ul_cfg_t* cfg, isrran_pusch_data_t* data)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL) {
    isrran_vec_cf_zero(q->sf_symbols, ISRRAN_NOF_RE(q->cell));

    if (isrran_pusch_encode(&q->pusch, sf, &cfg->ul_cfg.pusch, data, q->sf_symbols)) {
      ERROR("Error encoding PUSCH");
      return ISRRAN_ERROR;
    }

    if (q->signals_pregenerated) {
      isrran_refsignal_dmrs_pusch_pregen_put(&q->signals, sf, &q->pregen_dmrs, &cfg->ul_cfg.pusch, q->sf_symbols);
    } else {
      if (isrran_refsignal_dmrs_pusch_gen(&q->signals,
                                          &cfg->ul_cfg.dmrs,
                                          cfg->ul_cfg.pusch.grant.L_prb,
                                          sf->tti % 10,
                                          cfg->ul_cfg.pusch.grant.n_dmrs,
                                          q->refsignal)) {
        ERROR("Error generating PUSCH DMRS signals");
        return ret;
      }
      isrran_refsignal_dmrs_pusch_put(&q->signals, &cfg->ul_cfg.pusch, q->refsignal, q->sf_symbols);
    }

    add_isr(q, cfg, sf->tti);

    isrran_ofdm_tx_sf(&q->fft);

    apply_cfo(q, cfg);
    apply_norm(q, cfg, q->cell.nof_prb / 15 / sqrtf(cfg->ul_cfg.pusch.grant.L_prb) / 2);

    ret = ISRRAN_SUCCESS;
  }

  return ret;
}

/* Returns the transmission power for PUSCH for this subframe as defined in Section 5.1.1 of 36.213 */
float isrran_ue_ul_pusch_power(isrran_ue_ul_t* q, isrran_ue_ul_cfg_t* cfg, float PL, float p0_preamble)
{
  float p0_pusch, alpha;
  if (p0_preamble) {
    p0_pusch = p0_preamble + cfg->ul_cfg.power_ctrl.delta_preamble_msg3;
    alpha    = 1;
  } else {
    alpha    = cfg->ul_cfg.power_ctrl.alpha;
    p0_pusch = cfg->ul_cfg.power_ctrl.p0_nominal_pusch + cfg->ul_cfg.power_ctrl.p0_ue_pusch;
  }
  float delta = 0;
#ifdef kk
  if (ul_cfg->ul_cfg.power_ctrl.delta_mcs_based) {
    float beta_offset_pusch = 1;
    float MPR = q->pusch.cb_segm.K1 * q->pusch_cfg.cb_segm.C1 + q->pusch_cfg.cb_segm.K2 * q->pusch_cfg.cb_segm.C2;
    if (q->pusch.dci.cw.tbs == 0) {
      beta_offset_pusch = isrran_sch_beta_cqi(q->pusch.uci_offset.I_offset_cqi);
      MPR               = q->pusch_cfg.last_O_cqi;
    }
    MPR /= q->pusch.dci.nof_re;
    delta = 10 * log10f((powf(2, MPR * 1.25) - 1) * beta_offset_pusch);
  }
#else
  printf("Do this in pusch??");
#endif
  // TODO: This implements closed-loop power control
  float f = 0;

  float pusch_power = 10 * log10f(cfg->ul_cfg.pusch.grant.L_prb) + p0_pusch + alpha * PL + delta + f;
  DEBUG("PUSCH: P=%f -- 10M=%f, p0=%f,alpha=%f,PL=%f,",
        pusch_power,
        10 * log10f(cfg->ul_cfg.pusch.grant.L_prb),
        p0_pusch,
        alpha,
        PL);
  return ISRRAN_MIN(ISRRAN_PC_MAX, pusch_power);
}

/* Returns the transmission power for PUCCH for this subframe as defined in Section 5.1.2 of 36.213 */
float isrran_ue_ul_pucch_power(isrran_ue_ul_t* q, isrran_ue_ul_cfg_t* cfg, isrran_uci_cfg_t* uci_cfg, float PL)
{
  isrran_pucch_format_t format = cfg->ul_cfg.pucch.format;

  float p0_pucch = cfg->ul_cfg.power_ctrl.p0_nominal_pucch + cfg->ul_cfg.power_ctrl.p0_ue_pucch;

  uint8_t format_idx = (uint8_t)((format == 0) ? (0) : ((uint8_t)format - 1));

  float delta_f = cfg->ul_cfg.power_ctrl.delta_f_pucch[format_idx];

  float h;
  int   n_cqi  = isrran_cqi_size(&uci_cfg->cqi);
  int   n_harq = isrran_uci_cfg_total_ack(uci_cfg);

  if (format <= ISRRAN_PUCCH_FORMAT_1B) {
    h = 0;
  } else {
    if (ISRRAN_CP_ISNORM(q->cell.cp)) {
      if (n_cqi >= 4) {
        h = 10.0f * log10f(n_cqi / 4.0f);
      } else {
        h = 0;
      }
    } else {
      if (n_cqi + n_harq >= 4) {
        h = 10.0f * log10f((n_cqi + n_harq) / 4.0f);
      } else {
        h = 0;
      }
    }
  }

  // TODO: This implements closed-loop power control
  float g = 0;

  float pucch_power = p0_pucch + PL + h + delta_f + g;

  DEBUG("PUCCH: P=%f -- p0=%f, PL=%f, delta_f=%f, h=%f, g=%f", pucch_power, p0_pucch, PL, delta_f, h, g);

  return 0;
}

static int isr_encode(isrran_ue_ul_t* q, uint32_t tti, isrran_ue_ul_cfg_t* cfg)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q && cfg) {
    isrran_vec_cf_zero(q->sf_symbols, ISRRAN_NOF_RE(q->cell));

    add_isr(q, cfg, tti);

    isrran_ofdm_tx_sf(&q->fft);

    apply_cfo(q, cfg);
    apply_norm(q, cfg, (float)q->cell.nof_prb / 15 / sqrtf(isrran_refsignal_isr_M_sc(&q->signals, &cfg->ul_cfg.isr)));

    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

static bool isr_tx_enabled(isrran_refsignal_isr_cfg_t* isr_cfg, uint32_t tti)
{
  if (isr_cfg->configured) {
    if (isrran_refsignal_isr_send_cs(isr_cfg->subframe_config, tti % 10) == 1 &&
        isrran_refsignal_isr_send_ue(isr_cfg->I_isr, tti) == 1) {
      return true;
    }
  }
  return false;
}

/* Returns the transmission power for ISR for this subframe as defined in Section 5.1.3 of 36.213 */
float isr_power(isrran_ue_ul_t* q, isrran_ue_ul_cfg_t* cfg, float PL)
{
  float alpha    = cfg->ul_cfg.power_ctrl.alpha;
  float p0_pusch = cfg->ul_cfg.power_ctrl.p0_nominal_pusch + cfg->ul_cfg.power_ctrl.p0_ue_pusch;

  // TODO: This implements closed-loop power control
  float f = 0;

  uint32_t M_sc = isrran_refsignal_isr_M_sc(&q->signals, &cfg->ul_cfg.isr);

  float p_isr_offset;
  if (cfg->ul_cfg.power_ctrl.delta_mcs_based) {
    p_isr_offset = -3 + cfg->ul_cfg.power_ctrl.p_isr_offset;
  } else {
    p_isr_offset = -10.5 + 1.5 * cfg->ul_cfg.power_ctrl.p_isr_offset;
  }

  float p_isr = p_isr_offset + 10 * log10f(M_sc) + p0_pusch + alpha * PL + f;

  DEBUG("ISR: P=%f -- p_offset=%f, 10M=%f, p0_pusch=%f, alpha=%f, PL=%f, f=%f",
        p_isr,
        p_isr_offset,
        10 * log10f(M_sc),
        p0_pusch,
        alpha,
        PL,
        f);

  return p_isr;
}

/* Procedure for determining PUCCH assignment 10.1 36.213 */
void isrran_ue_ul_pucch_resource_selection(const isrran_cell_t*      cell,
                                           isrran_pucch_cfg_t*       cfg,
                                           const isrran_uci_cfg_t*   uci_cfg,
                                           const isrran_uci_value_t* uci_value,
                                           uint8_t                   b[ISRRAN_UCI_MAX_ACK_BITS])
{
  // Get PUCCH Resources
  cfg->format  = isrran_pucch_proc_select_format(cell, cfg, uci_cfg, uci_value);
  cfg->n_pucch = isrran_pucch_proc_get_npucch(cell, cfg, uci_cfg, uci_value, b);
}

/* Choose PUCCH format as in Sec 10.1 of 36.213 and generate PUCCH signal
 */
static int
pucch_encode(isrran_ue_ul_t* q, isrran_ul_sf_cfg_t* sf, isrran_ue_ul_cfg_t* cfg, isrran_uci_value_t* uci_data)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && cfg != NULL) {
    isrran_uci_value_t uci_value2 = *uci_data; ///< Make copy of UCI Data, so the original input does not get altered
    ret                           = ISRRAN_ERROR;

    if (!isrran_pucch_cfg_isvalid(&cfg->ul_cfg.pucch, q->cell.nof_prb)) {
      ERROR("Invalid PUCCH configuration");
      return ret;
    }

    isrran_vec_cf_zero(q->sf_symbols, ISRRAN_NOF_RE(q->cell));

    // Prepare configuration
    isrran_ue_ul_pucch_resource_selection(
        &q->cell, &cfg->ul_cfg.pucch, &cfg->ul_cfg.pucch.uci_cfg, uci_data, uci_value2.ack.ack_value);

    isrran_refsignal_isr_pucch_shortened(&q->signals, sf, &cfg->ul_cfg.isr, &cfg->ul_cfg.pucch);

    if (isrran_pucch_encode(&q->pucch, sf, &cfg->ul_cfg.pucch, &uci_value2, q->sf_symbols)) {
      ERROR("Error encoding TB");
      return ret;
    }

    if (isrran_refsignal_dmrs_pucch_gen(&q->signals, sf, &cfg->ul_cfg.pucch, q->refsignal)) {
      ERROR("Error generating PUSCH DMRS signals");
      return ret;
    }
    isrran_refsignal_dmrs_pucch_put(&q->signals, &cfg->ul_cfg.pucch, q->refsignal, q->sf_symbols);

    add_isr(q, cfg, sf->tti);

    isrran_ofdm_tx_sf(&q->fft);

    apply_cfo(q, cfg);
    apply_norm(q, cfg, (float)q->cell.nof_prb / 15 / 10);

    char txt[256];
    isrran_pucch_tx_info(&cfg->ul_cfg.pucch, uci_data, txt, sizeof(txt));
    INFO("[PUCCH] Encoded %s", txt);

    ret = ISRRAN_SUCCESS;
  }

  return ret;
}

/* Returns 1 if a SR needs to be sent at current_tti given I_sr, as defined in Section 10.1 of 36.213 */
int isrran_ue_ul_sr_send_tti(const isrran_pucch_cfg_t* cfg, uint32_t current_tti)
{
  if (!cfg->sr_configured) {
    return ISRRAN_SUCCESS;
  }
  uint32_t sr_periodicity;
  uint32_t sr_N_offset;
  uint32_t I_sr = cfg->I_sr;

  if (I_sr < 5) {
    sr_periodicity = 5;
    sr_N_offset    = I_sr;
  } else if (I_sr < 15) {
    sr_periodicity = 10;
    sr_N_offset    = I_sr - 5;
  } else if (I_sr < 35) {
    sr_periodicity = 20;
    sr_N_offset    = I_sr - 15;
  } else if (I_sr < 75) {
    sr_periodicity = 40;
    sr_N_offset    = I_sr - 35;
  } else if (I_sr < 155) {
    sr_periodicity = 80;
    sr_N_offset    = I_sr - 75;
  } else if (I_sr < 157) {
    sr_periodicity = 2;
    sr_N_offset    = I_sr - 155;
  } else if (I_sr == 157) {
    sr_periodicity = 1;
    sr_N_offset    = I_sr - 157;
  } else {
    return ISRRAN_ERROR;
  }
  if (current_tti >= sr_N_offset) {
    if ((current_tti - sr_N_offset) % sr_periodicity == 0) {
      return 1;
    }
  }
  return ISRRAN_SUCCESS;
}

bool isrran_ue_ul_gen_sr(isrran_ue_ul_cfg_t* cfg, isrran_ul_sf_cfg_t* sf, isrran_uci_data_t* uci_data, bool sr_request)
{
  uci_data->value.scheduling_request = false;
  if (isrran_ue_ul_sr_send_tti(&cfg->ul_cfg.pucch, sf->tti)) {
    if (sr_request) {
      // Get I_sr parameter
      uci_data->value.scheduling_request = true;
    }
    uci_data->cfg.is_scheduling_request_tti = true;
    return true;
  }
  return false;
}

#define uci_pending(cfg) (isrran_uci_cfg_total_ack(&cfg) > 0 || cfg.cqi.data_enable || cfg.cqi.ri_len > 0)

int isrran_ue_ul_encode(isrran_ue_ul_t* q, isrran_ul_sf_cfg_t* sf, isrran_ue_ul_cfg_t* cfg, isrran_pusch_data_t* data)
{
  int ret = ISRRAN_SUCCESS;

  /* Convert DTX to NACK in channel-selection mode (Release 10 only)*/
  if (cfg->ul_cfg.pucch.ack_nack_feedback_mode != ISRRAN_PUCCH_ACK_NACK_FEEDBACK_MODE_NORMAL) {
    uint32_t dtx_count = 0;
    for (uint32_t a = 0; a < isrran_uci_cfg_total_ack(&cfg->ul_cfg.pusch.uci_cfg); a++) {
      if (data->uci.ack.ack_value[a] == 2) {
        data->uci.ack.ack_value[a] = 0;
        dtx_count++;
      }
    }

    /* If all bits are DTX, do not transmit HARQ */
    if (dtx_count == isrran_uci_cfg_total_ack(&cfg->ul_cfg.pusch.uci_cfg)) {
      for (int i = 0; i < ISRRAN_MAX_CARRIERS; i++) {
        cfg->ul_cfg.pusch.uci_cfg.ack[i].nof_acks = 0;
      }
    }
  }

  if (cfg->grant_available) {
    ret = pusch_encode(q, sf, cfg, data) ? -1 : 1;
  } else if ((uci_pending(cfg->ul_cfg.pucch.uci_cfg) || data->uci.scheduling_request) &&
             cfg->cc_idx == 0) { // Send PUCCH over PCell only
    if (!cfg->ul_cfg.pucch.rnti) {
      ERROR("Encoding PUCCH: rnti not set in ul_cfg\n");
      return ISRRAN_ERROR;
    }
    ret = pucch_encode(q, sf, cfg, &data->uci) ? -1 : 1;
  } else if (isr_tx_enabled(&cfg->ul_cfg.isr, sf->tti)) {
    ret = isr_encode(q, sf->tti, cfg) ? -1 : 1;
  } else {
    // Set Zero output buffer if no UL transmission is required so the buffer does not keep previous transmission data
    if (q->cell.nof_prb) {
      isrran_vec_cf_zero(q->out_buffer, ISRRAN_SF_LEN_PRB(q->cell.nof_prb));
    }
  }

  return ret;
}

bool isrran_ue_ul_info(isrran_ue_ul_cfg_t* cfg,
                       isrran_ul_sf_cfg_t* sf,
                       isrran_uci_value_t* uci_data,
                       char*               str,
                       uint32_t            str_len)
{
  uint32_t n   = 0;
  bool     ret = false;

  if (cfg->grant_available) {
    n = snprintf(str, str_len, "PUSCH: cc=%d, tti_tx=%d, %s", cfg->cc_idx, sf->tti, sf->shortened ? "shortened, " : "");
    isrran_pusch_tx_info(&cfg->ul_cfg.pusch, uci_data, &str[n], str_len - n);
    ret = true;
  } else if ((uci_pending(cfg->ul_cfg.pucch.uci_cfg) || uci_data->scheduling_request) &&
             cfg->cc_idx == 0) { // Send PUCCH over PCell only
    n = sprintf(str, "PUCCH: cc=0, tti_tx=%d, %s", sf->tti, sf->shortened ? "shortened, " : "");
    isrran_pucch_tx_info(&cfg->ul_cfg.pucch, uci_data, &str[n], str_len - n);
    ret = true;
  } else if (isr_tx_enabled(&cfg->ul_cfg.isr, sf->tti)) {
    n   = sprintf(str, "ISR: tx_tti=%d", sf->tti);
    ret = true;
  }
  return ret;
}
