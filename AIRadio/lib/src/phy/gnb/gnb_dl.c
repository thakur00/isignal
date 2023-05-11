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

#include "isrran/phy/gnb/gnb_dl.h"
#include <complex.h>

static float gnb_dl_get_norm_factor(uint32_t nof_prb)
{
  return 0.05f / sqrtf(nof_prb);
}

static int gnb_dl_alloc_prb(isrran_gnb_dl_t* q, uint32_t new_nof_prb)
{
  if (q->max_prb < new_nof_prb) {
    q->max_prb = new_nof_prb;

    for (uint32_t i = 0; i < q->nof_tx_antennas; i++) {
      if (q->sf_symbols[i] != NULL) {
        free(q->sf_symbols[i]);
      }

      q->sf_symbols[i] = isrran_vec_cf_malloc(ISRRAN_SLOT_LEN_RE_NR(q->max_prb));
      if (q->sf_symbols[i] == NULL) {
        ERROR("Malloc");
        return ISRRAN_ERROR;
      }
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_gnb_dl_init(isrran_gnb_dl_t* q, cf_t* output[ISRRAN_MAX_PORTS], const isrran_gnb_dl_args_t* args)
{
  if (!q || !output || !args) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (args->nof_tx_antennas == 0) {
    ERROR("Error invalid number of antennas (%d)", args->nof_tx_antennas);
    return ISRRAN_ERROR;
  }

  q->nof_tx_antennas = args->nof_tx_antennas;

  if (isrran_pdsch_nr_init_enb(&q->pdsch, &args->pdsch) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (gnb_dl_alloc_prb(q, args->nof_max_prb) < ISRRAN_SUCCESS) {
    ERROR("Error allocating");
    return ISRRAN_ERROR;
  }

  // Check symbol size is valid
  int symbol_sz = isrran_symbol_sz_from_srate(args->srate_hz, args->scs);
  if (symbol_sz <= 0) {
    ERROR("Error calculating symbol size from sampling rate of %.2f MHz and subcarrier spacing %s",
          q->srate_hz / 1e6,
          isrran_subcarrier_spacing_to_str(args->scs));
    return ISRRAN_ERROR;
  }
  q->symbol_sz = symbol_sz;

  // Create initial OFDM configuration
  isrran_ofdm_cfg_t fft_cfg = {};
  fft_cfg.nof_prb           = args->nof_max_prb;
  fft_cfg.symbol_sz         = (uint32_t)symbol_sz;
  fft_cfg.keep_dc           = true;

  // Initialise a different OFDM modulator per channel
  for (uint32_t i = 0; i < q->nof_tx_antennas; i++) {
    fft_cfg.in_buffer  = q->sf_symbols[i];
    fft_cfg.out_buffer = output[i];
    isrran_ofdm_tx_init_cfg(&q->fft[i], &fft_cfg);
  }

  if (isrran_dmrs_sch_init(&q->dmrs, false) < ISRRAN_SUCCESS) {
    ERROR("Error DMRS");
    return ISRRAN_ERROR;
  }

  if (isrran_pdcch_nr_init_tx(&q->pdcch, &args->pdcch) < ISRRAN_SUCCESS) {
    ERROR("Error PDCCH");
    return ISRRAN_ERROR;
  }

  isrran_ssb_args_t ssb_args = {};
  ssb_args.enable_encode     = true;
  ssb_args.max_srate_hz      = args->srate_hz;
  ssb_args.min_scs           = args->scs;
  if (isrran_ssb_init(&q->ssb, &ssb_args) < ISRRAN_SUCCESS) {
    ERROR("Error SSB");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

void isrran_gnb_dl_free(isrran_gnb_dl_t* q)
{
  if (q == NULL) {
    return;
  }

  for (uint32_t i = 0; i < ISRRAN_MAX_PORTS; i++) {
    isrran_ofdm_rx_free(&q->fft[i]);

    if (q->sf_symbols[i] != NULL) {
      free(q->sf_symbols[i]);
    }
  }

  isrran_pdsch_nr_free(&q->pdsch);
  isrran_dmrs_sch_free(&q->dmrs);

  isrran_pdcch_nr_free(&q->pdcch);
  isrran_ssb_free(&q->ssb);

  ISRRAN_MEM_ZERO(q, isrran_gnb_dl_t, 1);
}

int isrran_gnb_dl_set_carrier(isrran_gnb_dl_t* q, const isrran_carrier_nr_t* carrier)
{
  if (isrran_pdsch_nr_set_carrier(&q->pdsch, carrier) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (isrran_dmrs_sch_set_carrier(&q->dmrs, carrier) < ISRRAN_SUCCESS) {
    ERROR("Error DMRS");
    return ISRRAN_ERROR;
  }

  if (gnb_dl_alloc_prb(q, carrier->nof_prb) < ISRRAN_SUCCESS) {
    ERROR("Error allocating");
    return ISRRAN_ERROR;
  }

  if (carrier->nof_prb != q->carrier.nof_prb) {
    isrran_ofdm_cfg_t fft_cfg     = {};
    fft_cfg.nof_prb               = carrier->nof_prb;
    fft_cfg.symbol_sz             = isrran_min_symbol_sz_rb(carrier->nof_prb);
    fft_cfg.keep_dc               = true;
    fft_cfg.phase_compensation_hz = carrier->dl_center_frequency_hz;

    for (uint32_t i = 0; i < q->nof_tx_antennas; i++) {
      fft_cfg.in_buffer = q->sf_symbols[i];
      isrran_ofdm_tx_init_cfg(&q->fft[i], &fft_cfg);
    }
  }

  q->carrier = *carrier;

  return ISRRAN_SUCCESS;
}

int isrran_gnb_dl_set_ssb_config(isrran_gnb_dl_t* q, const isrran_ssb_cfg_t* ssb)
{
  if (q == NULL || ssb == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_ssb_set_cfg(&q->ssb, ssb) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_gnb_dl_add_ssb(isrran_gnb_dl_t* q, const isrran_pbch_msg_nr_t* pbch_msg, uint32_t sf_idx)
{
  if (q == NULL || pbch_msg == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Skip SSB if it is not the time for it
  if (!isrran_ssb_send(&q->ssb, sf_idx)) {
    return ISRRAN_SUCCESS;
  }

  if (isrran_ssb_add(&q->ssb, q->carrier.pci, pbch_msg, q->fft[0].cfg.out_buffer, q->fft[0].cfg.out_buffer) <
      ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_gnb_dl_set_pdcch_config(isrran_gnb_dl_t*             q,
                                   const isrran_pdcch_cfg_nr_t* cfg,
                                   const isrran_dci_cfg_nr_t*   dci_cfg)
{
  if (q == NULL || cfg == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  q->pdcch_cfg = *cfg;

  if (isrran_pdcch_nr_set_carrier(&q->pdcch, &q->carrier, &q->pdcch_cfg.coreset[0]) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (isrran_dci_nr_set_cfg(&q->dci, dci_cfg) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

void isrran_gnb_dl_gen_signal(isrran_gnb_dl_t* q)
{
  if (q == NULL) {
    return;
  }

  float norm_factor = gnb_dl_get_norm_factor(q->pdsch.carrier.nof_prb);

  for (uint32_t i = 0; i < q->nof_tx_antennas; i++) {
    isrran_ofdm_tx_sf(&q->fft[i]);

    isrran_vec_sc_prod_cfc(q->fft[i].cfg.out_buffer, norm_factor, q->fft[i].cfg.out_buffer, (uint32_t)q->fft[i].sf_sz);
  }
}

float isrran_gnb_dl_get_maximum_signal_power_dBfs(uint32_t nof_prb)
{
  return isrran_convert_amplitude_to_dB(gnb_dl_get_norm_factor(nof_prb)) +
         isrran_convert_power_to_dB((float)nof_prb * ISRRAN_NRE) + 3.0f;
}

int isrran_gnb_dl_base_zero(isrran_gnb_dl_t* q)
{
  if (q == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  for (uint32_t i = 0; i < q->nof_tx_antennas; i++) {
    isrran_vec_cf_zero(q->sf_symbols[i], ISRRAN_SLOT_LEN_RE_NR(q->carrier.nof_prb));
  }

  return ISRRAN_SUCCESS;
}

static int
gnb_dl_pdcch_put_msg(isrran_gnb_dl_t* q, const isrran_slot_cfg_t* slot_cfg, const isrran_dci_msg_nr_t* dci_msg)
{
  if (dci_msg->ctx.coreset_id >= ISRRAN_UE_DL_NR_MAX_NOF_CORESET ||
      !q->pdcch_cfg.coreset_present[dci_msg->ctx.coreset_id]) {
    ERROR("Invalid CORESET ID %d", dci_msg->ctx.coreset_id);
    return ISRRAN_ERROR;
  }
  isrran_coreset_t* coreset = &q->pdcch_cfg.coreset[dci_msg->ctx.coreset_id];

  if (isrran_pdcch_nr_set_carrier(&q->pdcch, &q->carrier, coreset) < ISRRAN_SUCCESS) {
    ERROR("Error setting PDCCH carrier/CORESET");
    return ISRRAN_ERROR;
  }

  // Put DMRS
  if (isrran_dmrs_pdcch_put(&q->carrier, coreset, slot_cfg, &dci_msg->ctx.location, q->sf_symbols[0]) <
      ISRRAN_SUCCESS) {
    ERROR("Error putting PDCCH DMRS");
    return ISRRAN_ERROR;
  }

  // PDCCH Encode
  if (isrran_pdcch_nr_encode(&q->pdcch, dci_msg, q->sf_symbols[0]) < ISRRAN_SUCCESS) {
    ERROR("Error encoding PDCCH");
    return ISRRAN_ERROR;
  }

  INFO("DCI DL NR: L=%d; ncce=%d;", dci_msg->ctx.location.L, dci_msg->ctx.location.ncce);

  return ISRRAN_SUCCESS;
}

int isrran_gnb_dl_pdcch_put_dl(isrran_gnb_dl_t* q, const isrran_slot_cfg_t* slot_cfg, const isrran_dci_dl_nr_t* dci_dl)
{
  if (q == NULL || slot_cfg == NULL || dci_dl == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Pack DCI
  isrran_dci_msg_nr_t dci_msg = {};
  if (isrran_dci_nr_dl_pack(&q->dci, dci_dl, &dci_msg) < ISRRAN_SUCCESS) {
    ERROR("Error packing DL DCI");
    return ISRRAN_ERROR;
  }

  INFO("DCI DL NR: L=%d; ncce=%d;", dci_dl->ctx.location.L, dci_dl->ctx.location.ncce);

  return gnb_dl_pdcch_put_msg(q, slot_cfg, &dci_msg);
}

int isrran_gnb_dl_pdcch_put_ul(isrran_gnb_dl_t* q, const isrran_slot_cfg_t* slot_cfg, const isrran_dci_ul_nr_t* dci_ul)
{
  if (q == NULL || slot_cfg == NULL || dci_ul == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Pack DCI
  isrran_dci_msg_nr_t dci_msg = {};
  if (isrran_dci_nr_ul_pack(&q->dci, dci_ul, &dci_msg) < ISRRAN_SUCCESS) {
    ERROR("Error packing UL DCI");
    return ISRRAN_ERROR;
  }

  INFO("DCI DL NR: L=%d; ncce=%d;", dci_ul->ctx.location.L, dci_ul->ctx.location.ncce);

  return gnb_dl_pdcch_put_msg(q, slot_cfg, &dci_msg);
}

int isrran_gnb_dl_pdsch_put(isrran_gnb_dl_t*           q,
                            const isrran_slot_cfg_t*   slot,
                            const isrran_sch_cfg_nr_t* cfg,
                            uint8_t*                   data[ISRRAN_MAX_TB])
{
  if (isrran_dmrs_sch_put_sf(&q->dmrs, slot, cfg, &cfg->grant, q->sf_symbols[0]) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (isrran_pdsch_nr_encode(&q->pdsch, cfg, &cfg->grant, data, q->sf_symbols) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_gnb_dl_pdsch_info(const isrran_gnb_dl_t* q, const isrran_sch_cfg_nr_t* cfg, char* str, uint32_t str_len)
{
  int len = 0;

  // Append PDSCH info
  len += isrran_pdsch_nr_tx_info(&q->pdsch, cfg, &cfg->grant, &str[len], str_len - len);

  return len;
}

int isrran_gnb_dl_pdcch_dl_info(const isrran_gnb_dl_t* q, const isrran_dci_dl_nr_t* dci, char* str, uint32_t str_len)
{
  int len = 0;

  // Append PDCCH info
  len += isrran_dci_dl_nr_to_str(&q->dci, dci, &str[len], str_len - len);

  return len;
}

int isrran_gnb_dl_pdcch_ul_info(const isrran_gnb_dl_t* q, const isrran_dci_ul_nr_t* dci, char* str, uint32_t str_len)
{
  int len = 0;

  // Append PDCCH info
  len += isrran_dci_ul_nr_to_str(&q->dci, dci, &str[len], str_len - len);

  return len;
}

int isrran_gnb_dl_nzp_csi_rs_put(isrran_gnb_dl_t*                    q,
                                 const isrran_slot_cfg_t*            slot_cfg,
                                 const isrran_csi_rs_nzp_resource_t* resource)
{
  if (q == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_csi_rs_nzp_put_resource(&q->carrier, slot_cfg, resource, q->sf_symbols[0]) < ISRRAN_SUCCESS) {
    ERROR("Error putting NZP-CSI-RS resource");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}
