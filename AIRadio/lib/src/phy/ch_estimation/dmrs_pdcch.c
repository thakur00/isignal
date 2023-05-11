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

#include "isrran/phy/ch_estimation/dmrs_pdcch.h"
#include "isrran/phy/ch_estimation/chest_common.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/phch/pdcch_nr.h"
#include "isrran/phy/utils/convolution.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <complex.h>
#include <math.h>

#define NOF_PILOTS_X_RB 3

/// @brief Every frequency resource is 6 Resource blocks, every resource block carries 3 pilots. So 18 possible pilots
/// per frequency resource.
#define NOF_PILOTS_X_FREQ_RES (NOF_PILOTS_X_RB * 6)

///@brief Maximum number of pilots in a PDCCH candidate location
#define DMRS_PDCCH_MAX_NOF_PILOTS_CANDIDATE                                                                            \
  ((ISRRAN_NRE / 3) * (1U << (ISRRAN_SEARCH_SPACE_NOF_AGGREGATION_LEVELS_NR - 1U)) * 6U)

#define DMRS_PDCCH_INFO_TX(...) INFO("PDCCH DMRS Tx: " __VA_ARGS__)
#define DMRS_PDCCH_INFO_RX(...) INFO("PDCCH DMRS Rx: " __VA_ARGS__)
#define DMRS_PDCCH_DEBUG_RX(...) DEBUG("PDCCH DMRS Rx: " __VA_ARGS__)

/// @brief Enables interpolation at CCE frequency bandwidth to avoid interference with adjacent PDCCH DMRS
#define DMRS_PDCCH_INTERPOLATE_GROUP 1

///@brief Enables synchronization error pre-compensation before group interpolator. It should decrease EVM in expense of
/// computing complexity.
#define DMRS_PDCCH_SYNC_PRECOMPENSATE_INTERP 0

///@brief Enables synchronization error pre-compensation before candidate measurement. It improves detection probability
/// in expense of computing complexity.
#define DMRS_PDCCH_SYNC_PRECOMPENSATE_MEAS 1

///@brief Enables/Disables smoothing filter
#define DMRS_PDCCH_SMOOTH_FILTER 0

static uint32_t dmrs_pdcch_get_cinit(uint32_t slot_idx, uint32_t symbol_idx, uint32_t n_id)
{
  return ISRRAN_SEQUENCE_MOD((((ISRRAN_NSYMB_PER_SLOT_NR * slot_idx + symbol_idx + 1UL) * (2UL * n_id + 1UL)) << 17U) +
                             2UL * n_id);
}

static void dmrs_pdcch_put_symbol(const isrran_carrier_nr_t* carrier,
                                  const isrran_coreset_t*    coreset,
                                  const bool                 rb_mask[ISRRAN_MAX_PRB_NR],
                                  uint32_t                   cinit,
                                  cf_t*                      sf_symbol)
{
  uint32_t nof_freq_res = ISRRAN_MIN(carrier->nof_prb / 6, ISRRAN_CORESET_FREQ_DOMAIN_RES_SIZE);

  // Initialise sequence for this symbol
  isrran_sequence_state_t sequence_state = {};
  isrran_sequence_state_init(&sequence_state, cinit);
  uint32_t sequence_skip = 0; // Accumulates pilot locations to skip

  // CORESET Resource Block counter
  uint32_t rb_coreset_idx = 0;

  // Get CORESET offset
  uint32_t offset_k = coreset->offset_rb * ISRRAN_NRE;

  // For each frequency resource (6 RB groups)
  for (uint32_t res_idx = 0; res_idx < nof_freq_res; res_idx++) {
    // Skip frequency resource if outside of the CORESET
    if (!coreset->freq_resources[res_idx]) {
      // Skip possible DMRS locations in this region
      sequence_skip += NOF_PILOTS_X_FREQ_RES;
      continue;
    }

    // For each RB in the enabled frequency resource
    for (uint32_t rb = 0; rb < 6; rb++, rb_coreset_idx++) {
      // Skip if mask is disabled
      if (!rb_mask[rb_coreset_idx]) {
        sequence_skip += NOF_PILOTS_X_RB;
        continue;
      }

      // Skip all discarded possible pilot locations
      isrran_sequence_state_advance(&sequence_state, 2 * sequence_skip);
      sequence_skip = 0;

      // Generate pilots for the given RB
      cf_t rl[NOF_PILOTS_X_RB];
      isrran_sequence_state_gen_f(&sequence_state, M_SQRT1_2, (float*)rl, NOF_PILOTS_X_RB * 2);

      //  Absolute RB index in the resource grid
      uint32_t n = res_idx * 6 + rb;

      // Write pilots in the symbol
      for (uint32_t k_prime = 0; k_prime < NOF_PILOTS_X_RB; k_prime++) {
        // Calculate absolute sub-carrier index in the resource grid
        uint32_t k = n * ISRRAN_NRE + 4 * k_prime + 1;

        // Write DMRS
        sf_symbol[k + offset_k] = rl[k_prime];
      }
    }
  }
}

int isrran_dmrs_pdcch_put(const isrran_carrier_nr_t*   carrier,
                          const isrran_coreset_t*      coreset,
                          const isrran_slot_cfg_t*     slot_cfg,
                          const isrran_dci_location_t* dci_location,
                          cf_t*                        sf_symbols)
{
  if (carrier == NULL || coreset == NULL || slot_cfg == NULL || dci_location == NULL || sf_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (coreset->duration < ISRRAN_CORESET_DURATION_MIN || coreset->duration > ISRRAN_CORESET_DURATION_MAX) {
    ERROR("Error CORESET duration %d is out-of-bounds (%d,%d)",
          coreset->duration,
          ISRRAN_CORESET_DURATION_MIN,
          ISRRAN_CORESET_DURATION_MAX);
    return ISRRAN_ERROR;
  }

  // Calculate CCE-to-REG mapping mask
  bool rb_mask[ISRRAN_MAX_PRB_NR] = {};
  if (isrran_pdcch_nr_cce_to_reg_mapping(coreset, dci_location, rb_mask) < ISRRAN_SUCCESS) {
    ERROR("Error in CCE-to-REG mapping");
    return ISRRAN_SUCCESS;
  }

  // Use cell id if the DMR scrambling id is not provided by higher layers
  uint32_t n_id = carrier->pci;
  if (coreset->dmrs_scrambling_id_present) {
    n_id = coreset->dmrs_scrambling_id;
  }

  // Bound slot index
  uint32_t slot_idx = ISRRAN_SLOT_NR_MOD(carrier->scs, slot_cfg->idx);

  for (uint32_t l = 0; l < coreset->duration; l++) {
    // Get Cin
    uint32_t cinit = dmrs_pdcch_get_cinit(slot_idx, l, n_id);

    DMRS_PDCCH_INFO_TX("n=%d; l=%d; cinit=%08x", slot_idx, l, cinit);

    // Put data
    dmrs_pdcch_put_symbol(carrier, coreset, rb_mask, cinit, &sf_symbols[carrier->nof_prb * ISRRAN_NRE * l]);
  }

  return ISRRAN_SUCCESS;
}

int isrran_dmrs_pdcch_estimator_init(isrran_dmrs_pdcch_estimator_t* q,
                                     const isrran_carrier_nr_t*     carrier,
                                     const isrran_coreset_t*        coreset)
{
  if (q == NULL || carrier == NULL || coreset == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (coreset->duration < ISRRAN_CORESET_DURATION_MIN || coreset->duration > ISRRAN_CORESET_DURATION_MAX) {
    ERROR("Error CORESET duration %d is out-of-bounds (%d,%d)",
          coreset->duration,
          ISRRAN_CORESET_DURATION_MIN,
          ISRRAN_CORESET_DURATION_MAX);
    return ISRRAN_ERROR;
  }

  // The carrier configuration is not used for initialization, so copy it always
  q->carrier = *carrier;

  // Detect change in CORESET, if none, return early
  if (memcmp(&q->coreset, coreset, sizeof(isrran_coreset_t)) == 0) {
    return ISRRAN_SUCCESS;
  }

  // Save new CORESET
  q->coreset = *coreset;

  // The interpolator may return without reconfiguring after the first call
  if (q->interpolator.M != 0) {
    isrran_interp_linear_free(&q->interpolator);
  }

  // Calculate new bandwidth and size
  uint32_t coreset_bw = isrran_coreset_get_bw(coreset);
  uint32_t coreset_sz = isrran_coreset_get_sz(coreset);

#if DMRS_PDCCH_INTERPOLATE_GROUP
  if (isrran_interp_linear_init(&q->interpolator, NOF_PILOTS_X_FREQ_RES / q->coreset.duration, 4)) {
#else
  if (isrran_interp_linear_init(&q->interpolator, coreset_bw * 3, 4)) {
#endif
    ERROR("Initiating interpolator");
    return ISRRAN_ERROR;
  }

  // Reallocate only if the CORESET size or bandwidth changed
  if (q->coreset_bw != coreset_bw || q->coreset_sz != coreset_sz) {
    // Iterate all possible symbols
    for (uint32_t l = 0; l < ISRRAN_CORESET_DURATION_MAX; l++) {
      // Free if allocated
      if (q->lse[l] != NULL) {
        free(q->lse[l]);
        q->lse[l] = NULL;
      }

      // Allocate
      if (l < coreset->duration) {
        // Allocate for 3 pilots per physical resource block
        q->lse[l] = isrran_vec_cf_malloc(coreset_bw * 3);
      }
    }

    if (q->ce) {
      free(q->ce);
    }
    q->ce = isrran_vec_cf_malloc(coreset_sz);
  }

  if (q->filter == NULL) {
    q->filter_len = 5U;

    q->filter = isrran_vec_f_malloc(q->filter_len);
    if (q->filter == NULL) {
      return ISRRAN_ERROR;
    }

    isrran_chest_set_smooth_filter_gauss(q->filter, q->filter_len - 1, 2);
  }

  // Save new calculated values
  q->coreset_bw = coreset_bw;
  q->coreset_sz = coreset_sz;

  return ISRRAN_SUCCESS;
}

void isrran_dmrs_pdcch_estimator_free(isrran_dmrs_pdcch_estimator_t* q)
{
  if (q == NULL) {
    return;
  }

  if (q->ce) {
    free(q->ce);
  }

  for (uint32_t i = 0; i < ISRRAN_CORESET_DURATION_MAX; i++) {
    if (q->lse[i]) {
      free(q->lse[i]);
    }
  }

  if (q->filter) {
    free(q->filter);
  }

  isrran_interp_linear_free(&q->interpolator);

  ISRRAN_MEM_ZERO(q, isrran_dmrs_pdcch_estimator_t, 1);
}

static void
isrran_dmrs_pdcch_extract(isrran_dmrs_pdcch_estimator_t* q, uint32_t cinit, const cf_t* sf_symbol, cf_t* lse)
{
  // Get CORESET offset
  uint32_t offset_k = q->coreset.offset_rb * ISRRAN_NRE;

  // Initialise pseudo-random sequence
  isrran_sequence_state_t sequence_state = {};
  isrran_sequence_state_init(&sequence_state, cinit);

  // Counter for skipping unused pilot locations
  uint32_t sequence_skip = 0;

  // Counts enabled frequency domain resources
  uint32_t lse_count = 0;

  // Iterate over all possible frequency resources
  uint32_t freq_domain_res_size = ISRRAN_MIN(q->carrier.nof_prb / 6, ISRRAN_CORESET_FREQ_DOMAIN_RES_SIZE);
  for (uint32_t i = 0; i < freq_domain_res_size; i++) {
    // Skip disabled frequency resources
    if (!q->coreset.freq_resources[i]) {
      sequence_skip += NOF_PILOTS_X_FREQ_RES;
      continue;
    }

    // Skip unused pilots
    isrran_sequence_state_advance(&sequence_state, 2 * sequence_skip);
    sequence_skip = 0;

    // Generate sequence
    cf_t rl[NOF_PILOTS_X_FREQ_RES];
    isrran_sequence_state_gen_f(&sequence_state, M_SQRT1_2, (float*)rl, NOF_PILOTS_X_FREQ_RES * 2);

    // Iterate all PRBs in the enabled frequency domain resource
    cf_t* lse_ptr = &lse[lse_count];
    for (uint32_t j = 0; j < 6; j++) {
      // Calculate Grid PRB index (n)
      uint32_t n = i * 6 + j;

      // For each pilot in the PRB
      for (uint32_t k_prime = 0; k_prime < 3; k_prime++) {
        // Calculate sub-carrier index
        uint32_t k = n * ISRRAN_NRE + 4 * k_prime + 1;

        // Extract symbol
        lse[lse_count++] = sf_symbol[k + offset_k];
      }
    }

    // Calculate least squared estimates
    isrran_vec_prod_conj_ccc(lse_ptr, rl, lse_ptr, NOF_PILOTS_X_FREQ_RES);
  }
}

int isrran_dmrs_pdcch_estimate(isrran_dmrs_pdcch_estimator_t* q,
                               const isrran_slot_cfg_t*       slot_cfg,
                               const cf_t*                    sf_symbols)
{
  if (q == NULL || sf_symbols == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Use cell id if the DMR scrambling id is not provided by higher layers
  uint32_t n_id = q->carrier.pci;
  if (q->coreset.dmrs_scrambling_id_present) {
    n_id = q->coreset.dmrs_scrambling_id;
  }

  // Bound slot index
  uint32_t slot_idx = ISRRAN_SLOT_NR_MOD(q->carrier.scs, slot_cfg->idx);

  // Extract pilots
  for (uint32_t l = 0; l < q->coreset.duration; l++) {
    // Calculate PRN sequence initial state
    uint32_t cinit = dmrs_pdcch_get_cinit(slot_idx, l, n_id);

    // Extract pilots least square estimates
    isrran_dmrs_pdcch_extract(q, cinit, &sf_symbols[l * q->carrier.nof_prb * ISRRAN_NRE], q->lse[l]);
  }

  // Time averaging and smoothing should be implemented here
  // ...

  // Interpolation in groups
#if DMRS_PDCCH_INTERPOLATE_GROUP
  uint32_t group_count = (q->coreset.duration * q->coreset_bw * 3) / NOF_PILOTS_X_FREQ_RES;
  uint32_t group_size  = NOF_PILOTS_X_FREQ_RES / q->coreset.duration;
  for (uint32_t l = 0; l < q->coreset.duration; l++) {
    for (uint32_t j = 0; j < group_count; j++) {
      cf_t tmp[NOF_PILOTS_X_FREQ_RES];

      // Copy group into temporal vector
      isrran_vec_cf_copy(tmp, &q->lse[l][j * group_size], group_size);

#if DMRS_PDCCH_SYNC_PRECOMPENSATE_INTERP
      float sync_err = isrran_vec_estimate_frequency(tmp, group_size);
      if (isnormal(sync_err)) {
        isrran_vec_apply_cfo(tmp, sync_err, tmp, group_size);
      }
#endif // DMRS_PDCCH_SYNC_PRECOMPENSATION

#if DMRS_PDCCH_SMOOTH_FILTER
      // Smoothing filter group
      isrran_conv_same_cf(tmp, q->filter, tmp, group_size, q->filter_len);
#endif // DMRS_PDCCH_SMOOTH_FILTER

      // Interpolate group
      cf_t* dst = &q->ce[ISRRAN_NRE * q->coreset_bw * l + j * group_size * 4];
      isrran_interp_linear_offset(&q->interpolator, tmp, dst, 1, 3);

#if DMRS_PDCCH_SYNC_PRECOMPENSATE_INTERP
      if (isnormal(sync_err)) {
        isrran_vec_apply_cfo(dst, -sync_err / 4, dst, group_size * 4);
      }
#endif // DMRS_PDCCH_SYNC_PRECOMPENSATION
    }
  }
#else  // DMRS_PDCCH_INTERPOLATE_GROUP
  for (uint32_t l = 0; l < q->coreset.duration; l++) {
    isrran_interp_linear_offset(&q->interpolator, q->lse[l], &q->ce[ISRRAN_NRE * q->coreset_bw * l], 1, 3);
  }
#endif // DMRS_PDCCH_INTERPOLATE_GROUP

  return ISRRAN_SUCCESS;
}

int isrran_dmrs_pdcch_get_measure(const isrran_dmrs_pdcch_estimator_t* q,
                                  const isrran_dci_location_t*         dci_location,
                                  isrran_dmrs_pdcch_measure_t*         measure)
{
  if (q == NULL || dci_location == NULL || measure == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Check that CORESET duration is not less than minimum
  if (q->coreset.duration < ISRRAN_CORESET_DURATION_MIN) {
    ERROR("Invalid CORESET duration");
    return ISRRAN_ERROR;
  }

  // Calculate CCE-to-REG mapping mask
  bool rb_mask[ISRRAN_MAX_PRB_NR] = {};
  if (isrran_pdcch_nr_cce_to_reg_mapping(&q->coreset, dci_location, rb_mask) < ISRRAN_SUCCESS) {
    ERROR("Error in CCE-to-REG mapping");
    return ISRRAN_SUCCESS;
  }

  // Initialise measurements
  float rsrp                              = 0.0f; //< Averages linear RSRP
  float epre                              = 0.0f; //< Averages linear EPRE
  float cfo_avg_Hz                        = 0.0f; //< Averages CFO in Radians
  float sync_err_avg                      = 0.0f; //< Averages synchronization
  cf_t  corr[ISRRAN_CORESET_DURATION_MAX] = {};   //< Saves correlation for the different symbols

  // For each CORESET symbol
  for (uint32_t l = 0; l < q->coreset.duration; l++) {
    // Temporal least square estimates
    cf_t     tmp[DMRS_PDCCH_MAX_NOF_PILOTS_CANDIDATE] = {};
    uint32_t nof_pilots = 0;

    // For each RB in the CORESET
    for (uint32_t rb = 0; rb < q->coreset_bw; rb++) {
      // Skip RB if unused
      if (!rb_mask[rb]) {
        continue;
      }

      // Copy LSE
      isrran_vec_cf_copy(&tmp[nof_pilots], &q->lse[l][rb * NOF_PILOTS_X_RB], NOF_PILOTS_X_RB);
      nof_pilots += NOF_PILOTS_X_RB;
    }

    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
      DMRS_PDCCH_DEBUG_RX("Measuring PDCCH l=%d; lse=", l);
      isrran_vec_fprint_c(stdout, tmp, nof_pilots);
    }

    // Measure synchronization error and accumulate for average
    float tmp_sync_err = isrran_vec_estimate_frequency(tmp, nof_pilots);
    sync_err_avg += tmp_sync_err;

#if DMRS_PDCCH_SYNC_PRECOMPENSATE_MEAS
    // Pre-compensate synchronization error
    isrran_vec_apply_cfo(tmp, tmp_sync_err, tmp, nof_pilots);
#endif // DMRS_PDCCH_SYNC_PRECOMPENSATE_MEAS

    // Prevent undefined division
    if (!nof_pilots) {
      ERROR("Error in DMRS correlation. nof_pilots cannot be zero");
      return ISRRAN_ERROR;
    }
    // Correlate DMRS
    corr[l] = isrran_vec_acc_cc(tmp, nof_pilots) / (float)nof_pilots;

    // Measure symbol RSRP
    rsrp += __real__ corr[l] * __real__ corr[l] + __imag__ corr[l] * __imag__ corr[l];

    // Measure symbol EPRE
    epre += isrran_vec_avg_power_cf(tmp, nof_pilots);

    // Measure CFO only from the second and third symbols
    if (l != 0) {
      // Calculates the time between the previous and the current symbol
      float Ts = isrran_symbol_distance_s(l - 1, l, q->carrier.scs);
      if (isnormal(Ts)) {
        // Compute phase difference between symbols and convert to Hz
        cfo_avg_Hz += cargf(corr[l] * conjf(corr[l - 1])) / (2.0f * (float)M_PI * Ts);
      }
    }
  }

  // Store results
  measure->rsrp = rsrp / (float)q->coreset.duration;
  measure->epre = epre / (float)q->coreset.duration;
  if (q->coreset.duration > 1) {
    // Protected zero division
    measure->cfo_hz /= (float)(q->coreset.duration - 1);
  } else {
    // There are not enough symbols for computing CFO, set to NAN
    measure->cfo_hz = NAN;
  }
  measure->sync_error_us =
      sync_err_avg / (4.0e-6f * (float)q->coreset.duration * ISRRAN_SUBC_SPACING_NR(q->carrier.scs));

  // Convert power measurements into logarithmic scale
  measure->rsrp_dBfs = isrran_convert_power_to_dB(measure->rsrp);
  measure->epre_dBfs = isrran_convert_power_to_dB(measure->epre);

  // Store DMRS correlation
  if (isnormal(measure->rsrp) && isnormal(measure->epre)) {
    measure->norm_corr = measure->rsrp / measure->epre;
  } else {
    measure->norm_corr = 0.0f;
  }

  DMRS_PDCCH_INFO_RX("Measure L=%d; ncce=%d; RSRP=%+.1f dBfs; EPRE=%+.1f dBfs; Corr=%.3f",
                     dci_location->L,
                     dci_location->ncce,
                     measure->rsrp_dBfs,
                     measure->epre_dBfs,
                     measure->norm_corr);

  return ISRRAN_SUCCESS;
}

int isrran_dmrs_pdcch_get_ce(const isrran_dmrs_pdcch_estimator_t* q,
                             const isrran_dci_location_t*         dci_location,
                             isrran_dmrs_pdcch_ce_t*              ce)
{
  if (q == NULL || dci_location == NULL || ce == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  uint32_t L = 1U << dci_location->L;

  // Check that CORESET duration is not less than minimum
  if (q->coreset.duration < ISRRAN_CORESET_DURATION_MIN) {
    ERROR("Invalid CORESET duration");
    return ISRRAN_ERROR;
  }

  // Calculate CCE-to-REG mapping mask
  bool rb_mask[ISRRAN_MAX_PRB_NR] = {};
  if (isrran_pdcch_nr_cce_to_reg_mapping(&q->coreset, dci_location, rb_mask) < ISRRAN_SUCCESS) {
    ERROR("Error in CCE-to-REG mapping");
    return ISRRAN_SUCCESS;
  }

  // Extract CE for PDCCH
  uint32_t count = 0;

  // For each PDCCH symbol
  for (uint32_t l = 0; l < q->coreset.duration; l++) {
    // For each CORESET RB
    for (uint32_t rb = 0; rb < q->coreset_bw; rb++) {
      // Skip RB if unused
      if (!rb_mask[rb]) {
        continue;
      }

      // Copy RB, skipping DMRS
      for (uint32_t k = rb * ISRRAN_NRE; k < (rb + 1) * ISRRAN_NRE; k++) {
        if (k % 4 != 1) {
          ce->ce[count++] = q->ce[q->coreset_bw * ISRRAN_NRE * l + k];
        }
      }
    }
  }

  // Double check extracted RE match ideal count
  ce->nof_re = (ISRRAN_NRE - 3) * 6 * L;
  if (count != ce->nof_re) {
    ERROR("Incorrect number of extracted resources (%d != %d)", count, ce->nof_re);
  }

  // At the moment Noise is not calculated
  ce->noise_var = 0.0f;

  return ISRRAN_SUCCESS;
}
