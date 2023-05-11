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

#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "isrran/config.h"
#include "isrran/phy/utils/debug.h"

#include "isrran/phy/ch_estimation/chest_dl_nbiot.h"
#include "isrran/phy/utils/convolution.h"
#include "isrran/phy/utils/vector.h"

int isrran_chest_dl_nbiot_init(isrran_chest_dl_nbiot_t* q, uint32_t max_prb)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q != NULL) {
    bzero(q, sizeof(isrran_chest_dl_nbiot_t));

    ret = isrran_refsignal_dl_nbiot_init(&q->nrs_signal);
    if (ret != ISRRAN_SUCCESS) {
      fprintf(stderr, "Error initializing CSR signal (%d)\n", ret);
      goto clean_exit;
    }

    q->tmp_noise = isrran_vec_cf_malloc(ISRRAN_REFSIGNAL_MAX_NUM_SF(max_prb));
    if (!q->tmp_noise) {
      perror("malloc");
      goto clean_exit;
    }
    q->pilot_estimates = isrran_vec_cf_malloc(ISRRAN_REFSIGNAL_MAX_NUM_SF(max_prb) + ISRRAN_CHEST_MAX_SMOOTH_FIL_LEN);
    if (!q->pilot_estimates) {
      perror("malloc");
      goto clean_exit;
    }
    q->pilot_estimates_average =
        isrran_vec_cf_malloc(ISRRAN_REFSIGNAL_MAX_NUM_SF(max_prb) + ISRRAN_CHEST_MAX_SMOOTH_FIL_LEN);
    if (!q->pilot_estimates_average) {
      perror("malloc");
      goto clean_exit;
    }
    for (int i = 0; i < ISRRAN_REFSIGNAL_MAX_NUM_SF(max_prb); i++) {
      q->pilot_estimates_average[i] = 1;
    }

    q->pilot_recv_signal = isrran_vec_cf_malloc(ISRRAN_REFSIGNAL_MAX_NUM_SF(max_prb));
    if (!q->pilot_recv_signal) {
      perror("malloc");
      goto clean_exit;
    }

    if (isrran_interp_linear_vector_init(&q->isrran_interp_linvec, ISRRAN_NRE * max_prb)) {
      fprintf(stderr, "Error initializing vector interpolator\n");
      goto clean_exit;
    }

    if (isrran_interp_linear_init(&q->isrran_interp_lin, 2 * max_prb, ISRRAN_NRE / 2)) {
      fprintf(stderr, "Error initializing interpolator\n");
      goto clean_exit;
    }

    q->noise_alg = ISRRAN_NOISE_ALG_REFS;
    isrran_chest_dl_nbiot_set_smooth_filter3_coeff(q, 0.1);

    ret = ISRRAN_SUCCESS;
  }

clean_exit:
  if (ret != ISRRAN_SUCCESS  && q != NULL) {
    isrran_chest_dl_nbiot_free(q);
  }
  return ret;
}

void isrran_chest_dl_nbiot_free(isrran_chest_dl_nbiot_t* q)
{
  isrran_refsignal_dl_nbiot_free(&q->nrs_signal);

  if (q->tmp_noise) {
    free(q->tmp_noise);
  }
  isrran_interp_linear_vector_free(&q->isrran_interp_linvec);
  isrran_interp_linear_free(&q->isrran_interp_lin);

  if (q->pilot_estimates) {
    free(q->pilot_estimates);
  }
  if (q->pilot_estimates_average) {
    free(q->pilot_estimates_average);
  }
  if (q->pilot_recv_signal) {
    free(q->pilot_recv_signal);
  }
}

int isrran_chest_dl_nbiot_set_cell(isrran_chest_dl_nbiot_t* q, isrran_nbiot_cell_t cell)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  // set ports configuration
  if (cell.nof_ports == 0) {
    cell.nof_ports = ISRRAN_NBIOT_MAX_PORTS;
  }

  if (q != NULL && isrran_nbiot_cell_isvalid(&cell)) {
    if (q->cell.n_id_ncell != cell.n_id_ncell || q->cell.base.nof_prb == 0) {
      q->cell = cell;

      ret = isrran_refsignal_dl_nbiot_set_cell(&q->nrs_signal, cell);
      if (ret != ISRRAN_SUCCESS) {
        fprintf(stderr, "Error initializing NRS signal (%d)\n", ret);
        return ISRRAN_ERROR;
      }

      if (isrran_interp_linear_vector_resize(&q->isrran_interp_linvec, ISRRAN_NRE * cell.base.nof_prb)) {
        fprintf(stderr, "Error initializing vector interpolator\n");
        return ISRRAN_ERROR;
      }

      if (isrran_interp_linear_resize(&q->isrran_interp_lin, 2 * cell.base.nof_prb, ISRRAN_NRE / 2)) {
        fprintf(stderr, "Error initializing interpolator\n");
        return ISRRAN_ERROR;
      }
    }
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

/* Uses the difference between the averaged and non-averaged pilot estimates */
static float estimate_noise_pilots(isrran_chest_dl_nbiot_t* q, uint32_t port_id)
{
  int nref = isrran_refsignal_cs_nof_re(NULL, NULL, port_id);

  // Substract noisy pilot estimates
  isrran_vec_sub_ccc(q->pilot_estimates_average, q->pilot_estimates, q->tmp_noise, nref);

  // Compute average power. Normalized for filter len 3 using ML
  float norm = 1;
  if (q->smooth_filter_len == 3) {
    float a     = q->smooth_filter[0];
    float norm3 = 6.143 * a * a + 0.04859 * a - 0.002774;
    norm /= norm3;
  }
  float power = norm * q->cell.nof_ports * isrran_vec_avg_power_cf(q->tmp_noise, nref);
  return power;
}

#define cesymb(i) ce[ISRRAN_RE_IDX(q->cell.base.nof_prb, (i), 0)]

static void interpolate_pilots(isrran_chest_dl_nbiot_t* q, cf_t* pilot_estimates, cf_t* ce, uint32_t port_id)
{
  uint32_t nsymbols = isrran_refsignal_cs_nof_symbols(NULL, NULL, port_id);
  int      num_ces  = q->cell.base.nof_prb * ISRRAN_NRE;
  cf_t     ce_avg[2][num_ces];

  // interpolate the symbols with references in the freq domain
  DEBUG("Interpolating %d pilots in %d symbols at port %d.", nsymbols * 2, nsymbols, port_id);
  for (int l = 0; l < nsymbols; l++) {
    uint32_t fidx_offset = isrran_refsignal_dl_nbiot_fidx(q->cell, l, port_id, 0);
    // points to the RE of the beginning of the l'th symbol containing a ref
    uint32_t ce_idx = isrran_refsignal_nrs_nsymbol(l) * q->cell.base.nof_prb * ISRRAN_NRE;
    ce_idx += q->cell.nbiot_prb * ISRRAN_NRE;
    DEBUG("  - freq.-dmn interp. in sym %d at RE %d", isrran_refsignal_nrs_nsymbol(l), ce_idx + fidx_offset);
    isrran_interp_linear_offset(
        &q->isrran_interp_lin, &pilot_estimates[2 * l], &ce[ce_idx], fidx_offset, ISRRAN_NRE / 2 - fidx_offset);
  }

  // average the two neighbor subcarriers containing pilots
  for (int l = 0; l < nsymbols / 2; l++) {
    int sym_idx = isrran_refsignal_nrs_nsymbol(l * 2);
    isrran_vec_sum_ccc(&cesymb(sym_idx), &cesymb(sym_idx + 1), ce_avg[l], num_ces);
    // TODO: use vector operation for this
    for (int k = 0; k < num_ces; k++) {
      ce_avg[l][k] /= 2;
    }
    // use avarage for both symbols
    memcpy(&cesymb(sym_idx), ce_avg[l], num_ces * sizeof(cf_t));
    memcpy(&cesymb(sym_idx + 1), ce_avg[l], num_ces * sizeof(cf_t));
  }

  // now interpolate in the time domain between symbols
  isrran_interp_linear_vector(&q->isrran_interp_linvec, ce_avg[0], ce_avg[1], &cesymb(0), 5, 5);
  isrran_interp_linear_vector(&q->isrran_interp_linvec, ce_avg[0], ce_avg[1], &cesymb(7), 5, 5);
}

void isrran_chest_dl_nbiot_set_smooth_filter(isrran_chest_dl_nbiot_t* q, float* filter, uint32_t filter_len)
{
  if (filter_len < ISRRAN_CHEST_MAX_SMOOTH_FIL_LEN) {
    if (filter) {
      memcpy(q->smooth_filter, filter, filter_len * sizeof(float));
      q->smooth_filter_len = filter_len;
    } else {
      q->smooth_filter_len = 0;
    }
  } else {
    fprintf(stderr,
            "Error setting smoothing filter: filter len exceeds maximum (%d>%d)\n",
            filter_len,
            ISRRAN_CHEST_MAX_SMOOTH_FIL_LEN);
  }
}

void isrran_chest_dl_nbiot_set_noise_alg(isrran_chest_dl_nbiot_t* q, isrran_chest_dl_noise_alg_t noise_estimation_alg)
{
  q->noise_alg = noise_estimation_alg;
}

void isrran_chest_dl_nbiot_set_smooth_filter3_coeff(isrran_chest_dl_nbiot_t* q, float w)
{
  q->smooth_filter_len = 3;
  q->smooth_filter[0]  = w;
  q->smooth_filter[2]  = w;
  q->smooth_filter[1]  = 1 - 2 * w;
}

static void average_pilots(isrran_chest_dl_nbiot_t* q, cf_t* input, cf_t* output, uint32_t port_id)
{
  uint32_t nsymbols = isrran_refsignal_cs_nof_symbols(NULL, NULL, port_id);
  uint32_t nref     = 2;

  // Average in the frequency domain
  for (int l = 0; l < nsymbols; l++) {
    isrran_conv_same_cf(&input[l * nref], q->smooth_filter, &output[l * nref], nref, q->smooth_filter_len);
  }
}

float isrran_chest_nbiot_dl_rssi(isrran_chest_dl_nbiot_t* q, cf_t* input, uint32_t port_id)
{
  float    rssi     = 0;
  uint32_t nsymbols = isrran_refsignal_cs_nof_symbols(NULL, NULL, port_id);
  for (uint32_t l = 0; l < nsymbols; l++) {
    cf_t* tmp = &input[isrran_refsignal_nrs_nsymbol(l) * ISRRAN_NRE];
    rssi += isrran_vec_dot_prod_conj_ccc(tmp, tmp, ISRRAN_NRE);
  }
  return rssi / nsymbols;
}

int isrran_chest_dl_nbiot_estimate_port(isrran_chest_dl_nbiot_t* q,
                                        cf_t*                    input,
                                        cf_t*                    ce,
                                        uint32_t                 sf_idx,
                                        uint32_t                 port_id)
{
  DEBUG("x.%d: Estimating DL channel on port %d.", sf_idx, port_id);
  int nref = isrran_refsignal_nbiot_cs_nof_re(&q->cell, port_id);

  // Get references from the input signal
  isrran_refsignal_nrs_get_sf(q->cell, port_id, input, q->pilot_recv_signal);

  // Use the known NRS signal to compute Least-squares estimates
  isrran_vec_prod_conj_ccc(q->pilot_recv_signal, q->nrs_signal.pilots[port_id][sf_idx], q->pilot_estimates, nref);

  if (ISRRAN_VERBOSE_ISDEBUG()) {
    printf("Rx pilots:\n");
    isrran_vec_fprint_c(stdout, q->pilot_recv_signal, nref);
    printf("Tx pilots on port=%d for sf_idx=%d:\n", port_id, sf_idx);
    isrran_vec_fprint_c(stdout, q->nrs_signal.pilots[port_id][sf_idx], nref);
    printf("Estimates:\n");
    isrran_vec_fprint_c(stdout, q->pilot_estimates, nref);
    printf("Average estimates:\n");
    isrran_vec_fprint_c(stdout, q->pilot_estimates_average, nref);
  }

  if (ce != NULL) {
    // Smooth estimates (if applicable) and interpolate
    if (q->smooth_filter_len == 0 || (q->smooth_filter_len == 3 && q->smooth_filter[0] == 0)) {
      interpolate_pilots(q, q->pilot_estimates, ce, port_id);
    } else {
      average_pilots(q, q->pilot_estimates, q->pilot_estimates_average, port_id);
      interpolate_pilots(q, q->pilot_estimates_average, ce, port_id);
    }

    // Estimate noise power
    if (q->noise_alg == ISRRAN_NOISE_ALG_REFS && q->smooth_filter_len > 0) {
      q->noise_estimate[port_id] = estimate_noise_pilots(q, port_id);
    }
  }

  if (ISRRAN_VERBOSE_ISDEBUG()) {
    printf("Updated Average estimates:\n");
    isrran_vec_fprint_c(stdout, q->pilot_estimates_average, nref);
  }

  // Compute RSRP for the channel estimates in this port
  q->rsrp[port_id] = isrran_vec_avg_power_cf(q->pilot_recv_signal, nref);
  if (port_id == 0) {
    // compute rssi only for port 0
    q->rssi[port_id] = isrran_chest_nbiot_dl_rssi(q, input, port_id);
  }

  return 0;
}

int isrran_chest_dl_nbiot_estimate(isrran_chest_dl_nbiot_t* q, cf_t* input, cf_t** ce, uint32_t sf_idx)
{
  for (uint32_t port_id = 0; port_id < q->cell.nof_ports; port_id++) {
    isrran_chest_dl_nbiot_estimate_port(q, input, ce[port_id], sf_idx, port_id);
  }
  return ISRRAN_SUCCESS;
}

float isrran_chest_dl_nbiot_get_noise_estimate(isrran_chest_dl_nbiot_t* q)
{
  return isrran_vec_acc_ff(q->noise_estimate, q->cell.nof_ports) / q->cell.nof_ports;
}

float isrran_chest_dl_nbiot_get_snr(isrran_chest_dl_nbiot_t* q)
{
  return isrran_chest_dl_nbiot_get_rsrp(q) / isrran_chest_dl_nbiot_get_noise_estimate(q);
}

float isrran_chest_dl_nbiot_get_rssi(isrran_chest_dl_nbiot_t* q)
{
  return 4 * q->rssi[0] / ISRRAN_NRE;
}

/* q->rssi[0] is the average power in all RE in all symbol containing references for port 0.
 * q->rssi[0]/q->cell.nof_prb is the average power per PRB
 * q->rsrp[0] is the average power of RE containing references only (for port 0).
 */
float isrran_chest_dl_nbiot_get_rsrq(isrran_chest_dl_nbiot_t* q)
{
  return q->rsrp[0] / q->rssi[0];
}

float isrran_chest_dl_nbiot_get_rsrp(isrran_chest_dl_nbiot_t* q)
{
  // return sum of power received from all tx ports
  return isrran_vec_acc_ff(q->rsrp, q->cell.nof_ports);
}