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
#include <math.h>
#include <stdlib.h>
#include <strings.h>

#include "isrran/phy/sync/cfo.h"
#include "isrran/phy/utils/cexptab.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

/* Set next macro to 1 for using table generated CFO compensation */
#define ISRRAN_CFO_USE_EXP_TABLE 0

int isrran_cfo_init(isrran_cfo_t* h, uint32_t nsamples)
{
#if ISRRAN_CFO_USE_EXP_TABLE
  int ret = ISRRAN_ERROR;
  bzero(h, sizeof(isrran_cfo_t));

  if (isrran_cexptab_init(&h->tab, ISRRAN_CFO_CEXPTAB_SIZE)) {
    goto clean;
  }
  h->cur_cexp = isrran_vec_cf_malloc(nsamples);
  if (!h->cur_cexp) {
    goto clean;
  }
  h->tol         = 0;
  h->last_freq   = 0;
  h->nsamples    = nsamples;
  h->max_samples = nsamples;
  isrran_cexptab_gen(&h->tab, h->cur_cexp, h->last_freq, h->nsamples);

  ret = ISRRAN_SUCCESS;
clean:
  if (ret == ISRRAN_ERROR) {
    isrran_cfo_free(h);
  }
  return ret;
#else  /* ISRRAN_CFO_USE_EXP_TABLE */
  h->nsamples = nsamples;
  return ISRRAN_SUCCESS;
#endif /* ISRRAN_CFO_USE_EXP_TABLE */
}

void isrran_cfo_free(isrran_cfo_t* h)
{
#if ISRRAN_CFO_USE_EXP_TABLE
  isrran_cexptab_free(&h->tab);
  if (h->cur_cexp) {
    free(h->cur_cexp);
  }
#endif /* ISRRAN_CFO_USE_EXP_TABLE */
  bzero(h, sizeof(isrran_cfo_t));
}

void isrran_cfo_set_tol(isrran_cfo_t* h, float tol)
{
  h->tol = tol;
}

int isrran_cfo_resize(isrran_cfo_t* h, uint32_t samples)
{
#if ISRRAN_CFO_USE_EXP_TABLE
  if (samples <= h->max_samples) {
    isrran_cexptab_gen(&h->tab, h->cur_cexp, h->last_freq, samples);
    h->nsamples = samples;
  } else {
    ERROR("Error in cfo_resize(): nof_samples must be lower than initialized");
    return ISRRAN_ERROR;
  }
#endif /* ISRRAN_CFO_USE_EXP_TABLE */
  return ISRRAN_SUCCESS;
}

void isrran_cfo_correct(isrran_cfo_t* h, const cf_t* input, cf_t* output, float freq)
{
#if ISRRAN_CFO_USE_EXP_TABLE
  if (fabs(h->last_freq - freq) > h->tol) {
    h->last_freq = freq;
    isrran_cexptab_gen(&h->tab, h->cur_cexp, h->last_freq, h->nsamples);
    DEBUG("CFO generating new table for frequency %.4fe-6", freq * 1e6);
  }
  isrran_vec_prod_ccc(h->cur_cexp, input, output, h->nsamples);
#else  /* ISRRAN_CFO_USE_EXP_TABLE */
  isrran_vec_apply_cfo(input, freq, output, h->nsamples);
#endif /* ISRRAN_CFO_USE_EXP_TABLE */
}

/* CFO correction which allows to specify the offset within the correction
 * table to allow phase-continuity across multi-subframe transmissions (NB-IoT)
 * Note that when correction table needs to be regenerated, the regeneration
 * takes place for the maximum number of samples
 */
void isrran_cfo_correct_offset(isrran_cfo_t* h,
                               const cf_t*   input,
                               cf_t*         output,
                               float         freq,
                               int           cexp_offset,
                               int           nsamples)
{
  if (fabs(h->last_freq - freq) > h->tol) {
    h->last_freq = freq;
    isrran_cexptab_gen(&h->tab, h->cur_cexp, h->last_freq, h->nsamples);
    DEBUG("CFO generating new table for frequency %.4fe-6", freq * 1e6);
  }
  isrran_vec_prod_ccc(&h->cur_cexp[cexp_offset], input, output, nsamples);
}

float isrran_cfo_est_corr_cp(cf_t* input_buffer, uint32_t nof_prb)
{
  int   nFFT         = isrran_symbol_sz(nof_prb);
  int   sf_n_samples = nFFT * 15;
  float tFFT         = (float)(1 / 15000.0);
  int   cp_size      = ISRRAN_CP_LEN_NORM(1, nFFT);

  // Compensate for initial SC-FDMA half subcarrier shift
  isrran_vec_apply_cfo(input_buffer, (float)(1 / (nFFT * 15e3)) * (15e3 / 2.0), input_buffer, sf_n_samples);

  // Conjugate multiply the correlation inputs
  cf_t cfo_estimated = isrran_vec_dot_prod_conj_ccc(&input_buffer[nFFT + ISRRAN_CP_LEN_NORM(0, nFFT)],
                                                    &input_buffer[2 * nFFT + ISRRAN_CP_LEN_NORM(0, nFFT)],
                                                    cp_size);

  // CFO correction and compensation for initial SC-FDMA half subcarrier shift
  float cfo = (float)(-1 * carg(cfo_estimated) / (float)(2 * M_PI * tFFT));
  isrran_vec_apply_cfo(input_buffer, (float)(1 / (nFFT * 15e3)) * ((-15e3 / 2.0) - cfo), input_buffer, sf_n_samples);
  return cfo;
}
