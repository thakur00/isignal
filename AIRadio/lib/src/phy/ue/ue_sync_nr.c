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

#include "isrran/phy/ue/ue_sync_nr.h"
#include "isrran/phy/utils/vector.h"

#define UE_SYNC_NR_DEFAULT_CFO_ALPHA 0.1

int isrran_ue_sync_nr_init(isrran_ue_sync_nr_t* q, const isrran_ue_sync_nr_args_t* args)
{
  // Check inputs
  if (q == NULL || args == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Copy arguments
  q->recv_obj        = args->recv_obj;
  q->recv_callback   = args->recv_callback;
  q->nof_rx_channels = args->nof_rx_channels == 0 ? 1 : args->nof_rx_channels;
  q->disable_cfo     = args->disable_cfo;
  q->cfo_alpha       = isnormal(args->cfo_alpha) ? args->cfo_alpha : UE_SYNC_NR_DEFAULT_CFO_ALPHA;

  // Initialise SSB
  isrran_ssb_args_t ssb_args = {};
  ssb_args.max_srate_hz      = args->max_srate_hz;
  ssb_args.min_scs           = args->min_scs;
  ssb_args.enable_search     = true;
  ssb_args.enable_decode     = true;
  ssb_args.pbch_dmrs_thr     = args->pbch_dmrs_thr;
  if (isrran_ssb_init(&q->ssb, &ssb_args) < ISRRAN_SUCCESS) {
    ERROR("Error SSB init");
    return ISRRAN_ERROR;
  }

  // Allocate temporal buffer pointers
  q->tmp_buffer = ISRRAN_MEM_ALLOC(cf_t*, q->nof_rx_channels);
  if (q->tmp_buffer == NULL) {
    ERROR("Error alloc");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

void isrran_ue_sync_nr_free(isrran_ue_sync_nr_t* q)
{
  // Check inputs
  if (q == NULL) {
    return;
  }

  isrran_ssb_free(&q->ssb);

  if (q->tmp_buffer) {
    free(q->tmp_buffer);
  }

  ISRRAN_MEM_ZERO(q, isrran_ue_sync_nr_t, 1);
}

int isrran_ue_sync_nr_set_cfg(isrran_ue_sync_nr_t* q, const isrran_ue_sync_nr_cfg_t* cfg)
{
  // Check inputs
  if (q == NULL || cfg == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Copy parameters
  q->N_id     = cfg->N_id;
  q->srate_hz = cfg->ssb.srate_hz;

  // Calculate new subframe size
  q->sf_sz = (uint32_t)round(1e-3 * q->srate_hz);

  // Configure SSB
  if (isrran_ssb_set_cfg(&q->ssb, &cfg->ssb) < ISRRAN_SUCCESS) {
    ERROR("Error configuring SSB");
    return ISRRAN_ERROR;
  }

  // Transition to find
  q->state = ISRRAN_UE_SYNC_NR_STATE_FIND;

  return ISRRAN_SUCCESS;
}

static void ue_sync_nr_reset_feedback(isrran_ue_sync_nr_t* q)
{
  ISRRAN_MEM_ZERO(&q->feedback, isrran_csi_trs_measurements_t, 1);
}

static void ue_sync_nr_apply_feedback(isrran_ue_sync_nr_t* q)
{
  // Skip any update if there is no feedback available
  if (q->feedback.nof_re == 0) {
    return;
  }

  // Update number of samples
  q->avg_delay_us          = q->feedback.delay_us;
  q->next_rf_sample_offset = (uint32_t)round((double)q->avg_delay_us * (q->srate_hz * 1e-6));

  // Integrate CFO
  if (q->disable_cfo) {
    q->cfo_hz = ISRRAN_VEC_SAFE_EMA(q->feedback.cfo_hz, q->cfo_hz, q->cfo_alpha);
  } else {
    q->cfo_hz += q->feedback.cfo_hz * q->cfo_alpha;
  }

  // Reset feedback
  ue_sync_nr_reset_feedback(q);
}

static int ue_sync_nr_update_ssb(isrran_ue_sync_nr_t*                 q,
                                 const isrran_csi_trs_measurements_t* measurements,
                                 const isrran_pbch_msg_nr_t*          pbch_msg)
{
  isrran_mib_nr_t mib = {};
  if (isrran_pbch_msg_nr_mib_unpack(pbch_msg, &mib) != ISRRAN_SUCCESS) {
    return ISRRAN_SUCCESS;
  }

  // Reset feedback to prevent any previous erroneous measurement
  ue_sync_nr_reset_feedback(q);

  // Set feedback measurement
  isrran_combine_csi_trs_measurements(&q->feedback, measurements, &q->feedback);

  // Apply feedback
  ue_sync_nr_apply_feedback(q);

  // Setup context
  q->ssb_idx = pbch_msg->ssb_idx;
  q->sf_idx  = isrran_ssb_candidate_sf_idx(&q->ssb, pbch_msg->ssb_idx, pbch_msg->hrf);
  q->sfn     = mib.sfn;

  // Transition to track only if the measured delay is below 2.4 microseconds
  if (measurements->delay_us < 2.4f) {
    q->state = ISRRAN_UE_SYNC_NR_STATE_TRACK;
  }

  return ISRRAN_SUCCESS;
}

static int ue_sync_nr_run_find(isrran_ue_sync_nr_t* q, cf_t* buffer)
{
  isrran_csi_trs_measurements_t measurements = {};
  isrran_pbch_msg_nr_t          pbch_msg     = {};

  // Find SSB, measure PSS/SSS and decode PBCH
  if (isrran_ssb_find(&q->ssb, buffer, q->N_id, &measurements, &pbch_msg) < ISRRAN_SUCCESS) {
    ERROR("Error finding SSB");
    return ISRRAN_ERROR;
  }

  // If the PBCH message was NOT decoded, early return
  if (!pbch_msg.crc) {
    return ISRRAN_SUCCESS;
  }

  return ue_sync_nr_update_ssb(q, &measurements, &pbch_msg);
}

static int ue_sync_nr_run_track(isrran_ue_sync_nr_t* q, cf_t* buffer)
{
  isrran_csi_trs_measurements_t measurements = {};
  isrran_pbch_msg_nr_t          pbch_msg     = {};
  uint32_t                      half_frame   = q->sf_idx / (ISRRAN_NOF_SF_X_FRAME / 2);

  // Check if the SSB selected candidate index shall be received in this subframe
  bool is_ssb_opportunity = (q->sf_idx == isrran_ssb_candidate_sf_idx(&q->ssb, q->ssb_idx, half_frame > 0));

  // Use SSB periodicity
  if (q->ssb.cfg.periodicity_ms >= 10) {
    // SFN match with the periodicity
    is_ssb_opportunity = is_ssb_opportunity && (half_frame == 0) && (q->sfn % q->ssb.cfg.periodicity_ms / 10 == 0);
  }

  if (!is_ssb_opportunity) {
    return ISRRAN_SUCCESS;
  }

  // Measure PSS/SSS and decode PBCH
  if (isrran_ssb_track(&q->ssb, buffer, q->N_id, q->ssb_idx, half_frame, &measurements, &pbch_msg) < ISRRAN_SUCCESS) {
    ERROR("Error finding SSB");
    return ISRRAN_ERROR;
  }

  // If the PBCH message was NOT decoded, transition to find
  if (!pbch_msg.crc) {
    q->state = ISRRAN_UE_SYNC_NR_STATE_FIND;
    return ISRRAN_SUCCESS;
  }

  return ue_sync_nr_update_ssb(q, &measurements, &pbch_msg);
}

static int ue_sync_nr_recv(isrran_ue_sync_nr_t* q, cf_t** buffer, isrran_timestamp_t* timestamp)
{
  // Verify callback and srate are valid
  if (q->recv_callback == NULL && !isnormal(q->srate_hz)) {
    return ISRRAN_ERROR;
  }

  uint32_t buffer_offset = 0;
  uint32_t nof_samples   = q->sf_sz;

  if (q->next_rf_sample_offset > 0) {
    // Discard a number of samples from RF
    if (q->recv_callback(q->recv_obj, buffer, (uint32_t)q->next_rf_sample_offset, timestamp) < ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }
  } else {
    // Adjust receive buffer
    buffer_offset = (uint32_t)(-q->next_rf_sample_offset);
    nof_samples   = (uint32_t)(q->sf_sz + q->next_rf_sample_offset);
  }
  q->next_rf_sample_offset = 0;

  // Select buffer offsets
  for (uint32_t chan = 0; chan < q->nof_rx_channels; chan++) {
    // Set buffer to NULL if not present
    if (buffer[chan] == NULL) {
      q->tmp_buffer[chan] = NULL;
      continue;
    }

    // Initialise first offset samples to zero
    if (buffer_offset > 0) {
      isrran_vec_cf_zero(buffer[chan], buffer_offset);
    }

    // Set to sample index
    q->tmp_buffer[chan] = &buffer[chan][buffer_offset];
  }

  // Receive
  if (q->recv_callback(q->recv_obj, q->tmp_buffer, nof_samples, timestamp) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Compensate CFO
  for (uint32_t chan = 0; chan < q->nof_rx_channels; chan++) {
    if (buffer[chan] != 0 && !q->disable_cfo) {
      isrran_vec_apply_cfo(buffer[chan], -q->cfo_hz / q->srate_hz, buffer[chan], (int)q->sf_sz);
    }
  }

  return ISRRAN_SUCCESS;
}

int isrran_ue_sync_nr_zerocopy(isrran_ue_sync_nr_t* q, cf_t** buffer, isrran_ue_sync_nr_outcome_t* outcome)
{
  // Check inputs
  if (q == NULL || buffer == NULL || outcome == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Verify callback is valid
  if (q->recv_callback == NULL) {
    return ISRRAN_ERROR;
  }

  // Receive
  if (ue_sync_nr_recv(q, buffer, &outcome->timestamp) < ISRRAN_SUCCESS) {
    ERROR("Error receiving baseband");
    return ISRRAN_ERROR;
  }

  // Run FSM
  switch (q->state) {
    case ISRRAN_UE_SYNC_NR_STATE_IDLE:
      // Do nothing
      break;
    case ISRRAN_UE_SYNC_NR_STATE_FIND:
      if (ue_sync_nr_run_find(q, buffer[0]) < ISRRAN_SUCCESS) {
        ERROR("Error running find");
        return ISRRAN_ERROR;
      }
      break;
    case ISRRAN_UE_SYNC_NR_STATE_TRACK:
      if (ue_sync_nr_run_track(q, buffer[0]) < ISRRAN_SUCCESS) {
        ERROR("Error running track");
        return ISRRAN_ERROR;
      }
      break;
  }

  // Increment subframe counter
  q->sf_idx++;

  // Increment SFN
  if (q->sf_idx >= ISRRAN_NOF_SF_X_FRAME) {
    q->sfn    = (q->sfn + 1) % 1024;
    q->sf_idx = 0;
  }

  // Fill outcome
  outcome->in_sync  = (q->state == ISRRAN_UE_SYNC_NR_STATE_TRACK);
  outcome->sf_idx   = q->sf_idx;
  outcome->sfn      = q->sfn;
  outcome->cfo_hz   = q->cfo_hz;
  outcome->delay_us = q->avg_delay_us;

  return ISRRAN_SUCCESS;
}

int isrran_ue_sync_nr_feedback(isrran_ue_sync_nr_t* q, const isrran_csi_trs_measurements_t* measurements)
{
  if (q == NULL || measurements == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Accumulate feedback proportional to the number of elements provided by the measurement
  isrran_combine_csi_trs_measurements(&q->feedback, measurements, &q->feedback);

  return ISRRAN_SUCCESS;
}
