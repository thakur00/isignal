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

#include "isrran/phy/ch_estimation/dmrs_pucch.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <assert.h>
#include <complex.h>

// Implements TS 38.211 table 6.4.1.3.1.1-1: Number of DM-RS symbols and the corresponding N_PUCCH...
static uint32_t dmrs_pucch_format1_n_pucch(const isrran_pucch_nr_resource_t* resource, uint32_t m_prime)
{
  if (resource->intra_slot_hopping) {
    if (m_prime == 0) {
      switch (resource->nof_symbols) {
        case 4:
        case 5:
          return 1;
        case 6:
        case 7:
        case 8:
        case 9:
          return 2;
        case 10:
        case 11:
        case 12:
        case 13:
          return 3;
        case 14:
          return 4;
        default:; // Do nothing
      }
    } else {
      switch (resource->nof_symbols) {
        case 4:
        case 6:
          return 1;
        case 5:
        case 7:
        case 8:
        case 10:
          return 2;
        case 9:
        case 11:
        case 12:
        case 14:
          return 3;
        case 13:
          return 4;
        default:; // Do nothing
      }
    }
  } else if (m_prime == 0) {
    switch (resource->nof_symbols) {
      case 4:
        return 2;
      case 5:
      case 6:
        return 3;
      case 7:
      case 8:
        return 4;
      case 9:
      case 10:
        return 5;
      case 11:
      case 12:
        return 6;
      case 13:
      case 14:
        return 7;
      default:; // Do nothing
    }
  }

  ERROR("Invalid case nof_symbols=%d and m_prime=%d", resource->nof_symbols, m_prime);
  return 0;
}

int isrran_dmrs_pucch_format1_put(const isrran_pucch_nr_t*            q,
                                  const isrran_carrier_nr_t*          carrier,
                                  const isrran_pucch_nr_common_cfg_t* cfg,
                                  const isrran_slot_cfg_t*            slot,
                                  const isrran_pucch_nr_resource_t*   resource,
                                  cf_t*                               slot_symbols)
{
  if (q == NULL || carrier == NULL || cfg == NULL || slot == NULL || resource == NULL || slot_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    ERROR("Invalid PUCCH format 1 resource");
    return ISRRAN_ERROR;
  }

  // Get group sequence
  uint32_t u = 0;
  uint32_t v = 0;
  if (isrran_pucch_nr_group_sequence(carrier, cfg, &u, &v) < ISRRAN_SUCCESS) {
    ERROR("Error getting group sequence");
    return ISRRAN_ERROR;
  }

  // First symbol index
  uint32_t l_prime = resource->start_symbol_idx;

  // Clause 6.4.1.3.1.2 specifies l=0,2,4...
  for (uint32_t m_prime = 0, l = 0; m_prime < (resource->intra_slot_hopping ? 2 : 1); m_prime++) {
    // Get number of symbols carrying DMRS
    uint32_t n_pucch = dmrs_pucch_format1_n_pucch(resource, m_prime);
    if (n_pucch == 0) {
      ERROR("Error getting number of symbols");
      return ISRRAN_ERROR;
    }

    // Get the starting PRB
    uint32_t starting_prb = (m_prime == 0) ? resource->starting_prb : resource->second_hop_prb;

    for (uint32_t m = 0; m < n_pucch; m++, l += 2) {
      // Get start of the sequence in resource grid
      cf_t* slot_symbols_ptr = &slot_symbols[(q->carrier.nof_prb * (l + l_prime) + starting_prb) * ISRRAN_NRE];

      // Get Alpha index
      uint32_t alpha_idx = 0;
      if (isrran_pucch_nr_alpha_idx(carrier, cfg, slot, l, l_prime, resource->initial_cyclic_shift, 0, &alpha_idx) <
          ISRRAN_SUCCESS) {
        ERROR("Calculating alpha");
      }

      // get r_uv sequence from LUT object
      const cf_t* r_uv = isrran_zc_sequence_lut_get(&q->r_uv_1prb, u, v, alpha_idx);
      if (r_uv == NULL) {
        ERROR("Getting r_uv sequence");
        return ISRRAN_ERROR;
      }

      // Get w_i_m
      cf_t w_i_m = isrran_pucch_nr_format1_w(q, n_pucch, resource->time_domain_occ, m);

      // Compute z(n) = w(i) * r_uv(n)
      cf_t z[ISRRAN_NRE];
      isrran_vec_sc_prod_ccc(r_uv, w_i_m, z, ISRRAN_NRE);

      if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
        printf("[PUCCH Format 1 DMRS TX] m_prime=%d; m=%d; w_i_m=%+.3f%+.3f; z=",
               m_prime,
               m,
               __real__ w_i_m,
               __imag__ w_i_m);
        isrran_vec_fprint_c(stdout, z, ISRRAN_NRE);
      }

      // Put z in the grid
      isrran_vec_cf_copy(slot_symbols_ptr, z, ISRRAN_NRE);
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_dmrs_pucch_format1_estimate(const isrran_pucch_nr_t*            q,
                                       const isrran_pucch_nr_common_cfg_t* cfg,
                                       const isrran_slot_cfg_t*            slot,
                                       const isrran_pucch_nr_resource_t*   resource,
                                       const cf_t*                         slot_symbols,
                                       isrran_chest_ul_res_t*              res)
{
  if (q == NULL || cfg == NULL || slot == NULL || resource == NULL || slot_symbols == NULL || res == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    ERROR("Invalid PUCCH format 1 resource");
    return ISRRAN_ERROR;
  }

  // Get group sequence
  uint32_t u = 0;
  uint32_t v = 0;
  if (isrran_pucch_nr_group_sequence(&q->carrier, cfg, &u, &v) < ISRRAN_SUCCESS) {
    ERROR("Error getting group sequence");
    return ISRRAN_ERROR;
  }

  uint32_t start_rb_idx[ISRRAN_PUCCH_NR_FORMAT1_N_MAX];
  uint32_t symbol_idx[ISRRAN_PUCCH_NR_FORMAT1_N_MAX];
  cf_t     ce[ISRRAN_PUCCH_NR_FORMAT1_N_MAX][ISRRAN_NRE];

  // First symbol index
  uint32_t l_prime = resource->start_symbol_idx;

  uint32_t n_pucch_sum = 0;
  for (uint32_t m_prime = 0, l = 0; m_prime < (resource->intra_slot_hopping ? 2 : 1); m_prime++) {
    // Get number of symbols carrying DMRS
    uint32_t n_pucch = dmrs_pucch_format1_n_pucch(resource, m_prime);
    if (n_pucch == 0) {
      ERROR("Error getting number of symbols");
      return ISRRAN_ERROR;
    }

    // Prevent ce[m] overflow
    assert(n_pucch <= ISRRAN_PUCCH_NR_FORMAT1_N_MAX);

    // Get the starting PRB
    uint32_t starting_prb     = (m_prime == 0) ? resource->starting_prb : resource->second_hop_prb;
    start_rb_idx[n_pucch_sum] = starting_prb;

    for (uint32_t m = 0; m < n_pucch; m++, l += 2) { // Clause 6.4.1.3.1.2 specifies l=0,2,4...
      symbol_idx[n_pucch_sum] = l + l_prime;

      // Get start of the sequence in resource grid
      const cf_t* slot_symbols_ptr = &slot_symbols[(q->carrier.nof_prb * (l + l_prime) + starting_prb) * ISRRAN_NRE];

      // Get Alpha index
      uint32_t alpha_idx = 0;
      if (isrran_pucch_nr_alpha_idx(&q->carrier, cfg, slot, l, l_prime, resource->initial_cyclic_shift, 0, &alpha_idx) <
          ISRRAN_SUCCESS) {
        ERROR("Calculating alpha");
      }

      // get r_uv sequence from LUT object
      const cf_t* r_uv = isrran_zc_sequence_lut_get(&q->r_uv_1prb, u, v, alpha_idx);
      if (r_uv == NULL) {
        ERROR("Getting r_uv sequence");
        return ISRRAN_ERROR;
      }

      // Get w_i_m
      cf_t w_i_m = isrran_pucch_nr_format1_w(q, n_pucch, resource->time_domain_occ, m);

      // Compute z(n) = w(i) * r_uv(n)
      cf_t z[ISRRAN_NRE];
      isrran_vec_sc_prod_ccc(r_uv, w_i_m, z, ISRRAN_NRE);

      if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
        INFO("[PUCCH Format 1 DMRS RX] m_prime=%d; m=%d; w_i_m=%+.3f%+.3f", m_prime, m, __real__ w_i_m, __imag__ w_i_m);
        isrran_vec_fprint_c(stdout, z, ISRRAN_NRE);
      }

      // TODO: can ce[m] overflow?
      // Calculate least square estimates for this symbol
      isrran_vec_prod_conj_ccc(slot_symbols_ptr, z, ce[n_pucch_sum], ISRRAN_NRE);

      if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
        printf("[PUCCH Format 1 DMRS RX] ce[%d]=", n_pucch_sum);
        isrran_vec_fprint_c(stdout, ce[n_pucch_sum], ISRRAN_NRE);
      }
      n_pucch_sum++;
    }
  }

  // Perform measurements
  float rsrp                                = 0.0f;
  float epre                                = 0.0f;
  float ta_err                              = 0.0f;
  cf_t  corr[ISRRAN_PUCCH_NR_FORMAT1_N_MAX] = {};
  for (uint32_t m = 0; m < n_pucch_sum; m++) {
    corr[m] = isrran_vec_acc_cc(ce[m], ISRRAN_NRE) / ISRRAN_NRE;
    rsrp += ISRRAN_CSQABS(corr[m]);
    epre += isrran_vec_avg_power_cf(ce[m], ISRRAN_NRE);
    ta_err += isrran_vec_estimate_frequency(ce[m], ISRRAN_NRE);
  }

  // Average measurements
  rsrp /= n_pucch_sum;
  epre /= n_pucch_sum;
  ta_err /= n_pucch_sum;

  // Set power measures
  rsrp                    = ISRRAN_MIN(rsrp, epre);
  res->rsrp               = rsrp;
  res->rsrp_dBfs          = isrran_convert_power_to_dB(rsrp);
  res->epre               = epre;
  res->epre_dBfs          = isrran_convert_power_to_dB(epre);
  res->noise_estimate     = ISRRAN_MAX(epre - rsrp, 1e-6f);
  res->noise_estimate_dbFs = isrran_convert_power_to_dB(res->noise_estimate);
  res->snr                = rsrp / res->noise_estimate;
  res->snr_db             = isrran_convert_power_to_dB(res->snr);

  // Compute Time Aligment error in microseconds
  if (isnormal(ta_err)) {
    ta_err /= 15e3f * (float)(1U << q->carrier.scs); // Convert from normalized frequency to seconds
    ta_err *= 1e6f;                                  // Convert to micro-seconds
    ta_err     = roundf(ta_err * 10.0f) / 10.0f;     // Round to one tenth of micro-second
    res->ta_us = ta_err;
  } else {
    res->ta_us = 0.0f;
  }

  // Measure CFO
  if (n_pucch_sum > 1) {
    float cfo_avg_hz = 0.0f;
    for (uint32_t m = 0; m < n_pucch_sum - 1; m++) {
      uint32_t l0         = resource->start_symbol_idx + m * 2;
      uint32_t l1         = resource->start_symbol_idx + (m + 1) * 2;
      float    time_diff  = isrran_symbol_distance_s(l0, l1, q->carrier.scs);
      float    phase_diff = cargf(corr[m + 1] * conjf(corr[m]));

      if (isnormal(time_diff)) {
        cfo_avg_hz += phase_diff / (2.0f * M_PI * time_diff * (n_pucch_sum - 1));
      }
    }
    res->cfo_hz = cfo_avg_hz;
  } else {
    res->cfo_hz = NAN; // Not implemented
  }

  // Do averaging here
  // ... Not implemented

  // Interpolates between DMRS symbols
  for (uint32_t m = 0; m < n_pucch_sum; m++) {
    cf_t* ce_ptr = &res->ce[m * ISRRAN_NRE];

    if (m != n_pucch_sum - 1) {
      // If it is not the last symbol with DMRS, average between
      isrran_vec_sum_ccc(ce[m], ce[m + 1], ce_ptr, ISRRAN_NRE);
      isrran_vec_sc_prod_cfc(ce_ptr, 0.5f, ce_ptr, ISRRAN_NRE);
    } else if (m != 0) {
      // Extrapolate for the last if more than 1 are provided
      isrran_vec_sc_prod_cfc(ce[m], 3.0f, ce_ptr, ISRRAN_NRE);
      isrran_vec_sub_ccc(ce_ptr, ce[m - 1], ce_ptr, ISRRAN_NRE);
      isrran_vec_sc_prod_cfc(ce_ptr, 0.5f, ce_ptr, ISRRAN_NRE);
    } else {
      // Simply copy the estimated channel
      isrran_vec_cf_copy(ce_ptr, ce[m], ISRRAN_NRE);
    }
  }

  return ISRRAN_SUCCESS;
}

static uint32_t dmrs_pucch_format2_cinit(const isrran_carrier_nr_t*          carrier,
                                         const isrran_pucch_nr_common_cfg_t* cfg,
                                         const isrran_slot_cfg_t*            slot,
                                         uint32_t                            l)
{
  uint64_t n    = ISRRAN_SLOT_NR_MOD(carrier->scs, slot->idx);
  uint64_t n_id = (cfg->scrambling_id_present) ? cfg->scambling_id : carrier->pci;

  return ISRRAN_SEQUENCE_MOD((((ISRRAN_NSYMB_PER_SLOT_NR * n + l + 1UL) * (2UL * n_id + 1UL)) << 17UL) + 2UL * n_id);
}

int isrran_dmrs_pucch_format2_put(const isrran_pucch_nr_t*            q,
                                  const isrran_carrier_nr_t*          carrier,
                                  const isrran_pucch_nr_common_cfg_t* cfg,
                                  const isrran_slot_cfg_t*            slot,
                                  const isrran_pucch_nr_resource_t*   resource,
                                  cf_t*                               slot_symbols)
{
  if (q == NULL || carrier == NULL || cfg == NULL || slot == NULL || resource == NULL || slot_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    ERROR("Invalid PUCCH format 1 resource");
    return ISRRAN_ERROR;
  }

  uint32_t l_start = resource->start_symbol_idx;
  uint32_t l_end   = resource->start_symbol_idx + resource->nof_symbols;
  uint32_t k_start = ISRRAN_MIN(q->carrier.nof_prb - 1, resource->starting_prb) * ISRRAN_NRE + 1;
  uint32_t k_end   = ISRRAN_MIN(q->carrier.nof_prb, resource->starting_prb + resource->nof_prb) * ISRRAN_NRE;
  for (uint32_t l = l_start; l < l_end; l++) {
    // Compute sequence initial state
    uint32_t                cinit    = dmrs_pucch_format2_cinit(carrier, cfg, slot, l);
    isrran_sequence_state_t sequence = {};
    isrran_sequence_state_init(&sequence, cinit);

    // Skip PRBs to start
    isrran_sequence_state_advance(&sequence, 2 * 4 * resource->starting_prb);

    // Generate sequence
    cf_t r_l[ISRRAN_PUCCH_NR_FORMAT2_MAX_NPRB * 4];
    isrran_sequence_state_gen_f(&sequence, M_SQRT1_2, (float*)r_l, 2 * 4 * resource->nof_prb);

    // Put sequence in k = 3 * m + 1
    for (uint32_t k = k_start, i = 0; k < k_end; k += 3, i++) {
      slot_symbols[l * q->carrier.nof_prb * ISRRAN_NRE + k] = r_l[i];
    }
  }
  return ISRRAN_SUCCESS;
}

int isrran_dmrs_pucch_format2_estimate(const isrran_pucch_nr_t*            q,
                                       const isrran_pucch_nr_common_cfg_t* cfg,
                                       const isrran_slot_cfg_t*            slot,
                                       const isrran_pucch_nr_resource_t*   resource,
                                       const cf_t*                         slot_symbols,
                                       isrran_chest_ul_res_t*              res)
{
  if (q == NULL || cfg == NULL || slot == NULL || resource == NULL || slot_symbols == NULL || res == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (isrran_pucch_nr_cfg_resource_valid(resource) < ISRRAN_SUCCESS) {
    ERROR("Invalid PUCCH format 1 resource");
    return ISRRAN_ERROR;
  }

  cf_t ce[ISRRAN_PUCCH_NR_FORMAT2_MAX_NSYMB][ISRRAN_PUCCH_NR_FORMAT2_MAX_NPRB * 4];

  uint32_t l_start = resource->start_symbol_idx;
  uint32_t l_end   = resource->start_symbol_idx + resource->nof_symbols;
  uint32_t k_start = ISRRAN_MIN(q->carrier.nof_prb - 1, resource->starting_prb) * ISRRAN_NRE + 1;
  uint32_t k_end   = ISRRAN_MIN(q->carrier.nof_prb, resource->starting_prb + resource->nof_prb) * ISRRAN_NRE;
  uint32_t nof_ref = 4 * resource->nof_prb;
  for (uint32_t l = l_start, j = 0; l < l_end; l++, j++) {
    // Compute sequence initial state
    uint32_t                cinit    = dmrs_pucch_format2_cinit(&q->carrier, cfg, slot, l);
    isrran_sequence_state_t sequence = {};
    isrran_sequence_state_init(&sequence, cinit);

    // Skip PRBs to start
    isrran_sequence_state_advance(&sequence, 2 * 4 * resource->starting_prb);

    // Generate sequence
    cf_t r_l[ISRRAN_PUCCH_NR_FORMAT2_MAX_NPRB * 4];
    isrran_sequence_state_gen_f(&sequence, M_SQRT1_2, (float*)r_l, 2 * nof_ref);

    // Put sequence in k = 3 * m + 1
    for (uint32_t k = k_start, i = 0; k < k_end; k += 3, i++) {
      ce[j][i] = slot_symbols[l * q->carrier.nof_prb * ISRRAN_NRE + k];
    }

    isrran_vec_prod_conj_ccc(ce[j], r_l, ce[j], nof_ref);
  }

  // Perform measurements
  float epre                                    = 0.0f;
  float rsrp                                    = 0.0f;
  float ta_err                                  = 0.0f;
  cf_t  corr[ISRRAN_PUCCH_NR_FORMAT2_MAX_NSYMB] = {};
  for (uint32_t i = 0; i < resource->nof_symbols; i++) {
    corr[i] = isrran_vec_acc_cc(ce[i], nof_ref) / nof_ref;
    rsrp += ISRRAN_CSQABS(corr[i]);
    epre += isrran_vec_avg_power_cf(ce[i], nof_ref);
    ta_err += isrran_vec_estimate_frequency(ce[i], nof_ref);
  }
  epre /= resource->nof_symbols;
  rsrp /= resource->nof_symbols;
  ta_err /= resource->nof_symbols;

  // Set power measures
  rsrp                    = ISRRAN_MIN(rsrp, epre);
  res->rsrp               = rsrp;
  res->rsrp_dBfs          = isrran_convert_power_to_dB(rsrp);
  res->epre               = epre;
  res->epre_dBfs          = isrran_convert_power_to_dB(epre);
  res->noise_estimate     = ISRRAN_MAX(epre - rsrp, 1e-6f);
  res->noise_estimate_dbFs = isrran_convert_power_to_dB(res->noise_estimate);
  res->snr                = rsrp / res->noise_estimate;
  res->snr_db             = isrran_convert_power_to_dB(res->snr);

  // Compute Time Aligment error in microseconds
  if (isnormal(ta_err)) {
    ta_err /= 15e3f * (float)(1U << q->carrier.scs) * 3; // Convert from normalized frequency to seconds
    ta_err *= 1e6f;                                      // Convert to micro-seconds
    ta_err     = roundf(ta_err * 10.0f) / 10.0f;         // Round to one tenth of micro-second
    res->ta_us = ta_err;
  } else {
    res->ta_us = 0.0f;
  }

  // Measure CFO
  if (resource->nof_symbols > 1) {
    float cfo_avg_hz = 0.0f;
    for (uint32_t l = 0; l < resource->nof_symbols - 1; l++) {
      uint32_t l0         = resource->start_symbol_idx + l;
      uint32_t l1         = resource->start_symbol_idx + l + 1;
      float    time_diff  = isrran_symbol_distance_s(l0, l1, q->carrier.scs);
      float    phase_diff = cargf(corr[l + 1] * conjf(corr[l]));

      if (isnormal(time_diff)) {
        cfo_avg_hz += phase_diff / (2.0f * M_PI * time_diff * (resource->nof_symbols - 1));
      }
    }
    res->cfo_hz = cfo_avg_hz;
  } else {
    res->cfo_hz = NAN; // Not implemented
  }

  // Perform averaging
  // ...

  // Zero order hold
  for (uint32_t l = l_start, j = 0; l < l_end; l++, j++) {
    for (uint32_t k = k_start - 1, i = 0; k < k_end; k++, i++) {
      res->ce[l * q->carrier.nof_prb * ISRRAN_NRE + k] = ce[j][i / 3];
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_dmrs_pucch_format_3_4_get_symbol_idx(const isrran_pucch_nr_resource_t* resource,
                                                uint32_t idx[ISRRAN_DMRS_PUCCH_FORMAT_3_4_MAX_NSYMB])
{
  if (resource == NULL || idx == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  int count = 0;

  switch (resource->nof_symbols) {
    case 4:
      if (resource->intra_slot_hopping) {
        idx[count++] = 0;
        idx[count++] = 2;
      } else {
        idx[count++] = 1;
      }
      break;
    case 5:
      idx[count++] = 0;
      idx[count++] = 3;
      break;
    case 6:
    case 7:
      idx[count++] = 1;
      idx[count++] = 4;
      break;
    case 8:
      idx[count++] = 1;
      idx[count++] = 5;
      break;
    case 9:
      idx[count++] = 1;
      idx[count++] = 6;
      break;
    case 10:
      if (resource->additional_dmrs) {
        idx[count++] = 1;
        idx[count++] = 3;
        idx[count++] = 6;
        idx[count++] = 8;
      } else {
        idx[count++] = 2;
        idx[count++] = 7;
      }
      break;
    case 11:
      if (resource->additional_dmrs) {
        idx[count++] = 1;
        idx[count++] = 3;
        idx[count++] = 6;
        idx[count++] = 9;
      } else {
        idx[count++] = 2;
        idx[count++] = 7;
      }
      break;
    case 12:
      if (resource->additional_dmrs) {
        idx[count++] = 1;
        idx[count++] = 4;
        idx[count++] = 7;
        idx[count++] = 10;
      } else {
        idx[count++] = 2;
        idx[count++] = 8;
      }
      break;
    case 13:
      if (resource->additional_dmrs) {
        idx[count++] = 1;
        idx[count++] = 4;
        idx[count++] = 7;
        idx[count++] = 11;
      } else {
        idx[count++] = 2;
        idx[count++] = 9;
      }
      break;
    case 14:
      if (resource->additional_dmrs) {
        idx[count++] = 1;
        idx[count++] = 5;
        idx[count++] = 8;
        idx[count++] = 12;
      } else {
        idx[count++] = 3;
        idx[count++] = 10;
      }
      break;
    default:
      ERROR("Invalid case (%d)", resource->nof_symbols);
      return ISRRAN_ERROR;
  }

  return count;
}
