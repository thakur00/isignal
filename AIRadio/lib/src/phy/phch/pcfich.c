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
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/phch/pcfich.h"
#include "isrran/phy/phch/regs.h"
#include "isrran/phy/utils/bit.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

// Table 5.3.4-1
static uint8_t cfi_table[4][PCFICH_CFI_LEN] = {
    {0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0},
    {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} // reserved
};

bool isrran_pcfich_exists(int nframe, int nslot)
{
  return true;
}

/** Initializes the pcfich channel receiver.
 * On ERROR returns -1 and frees the structrure
 */
int isrran_pcfich_init(isrran_pcfich_t* q, uint32_t nof_rx_antennas)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL) {
    ret = ISRRAN_ERROR;

    bzero(q, sizeof(isrran_pcfich_t));
    q->nof_rx_antennas = nof_rx_antennas;
    q->nof_symbols     = PCFICH_RE;

    if (isrran_modem_table_lte(&q->mod, ISRRAN_MOD_QPSK)) {
      goto clean;
    }

    /* convert cfi bit tables to floats for demodulation */
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < PCFICH_CFI_LEN; j++) {
        q->cfi_table_float[i][j] = (float)2.0 * cfi_table[i][j] - 1.0;
      }
    }

    ret = ISRRAN_SUCCESS;
  }

clean:
  if (ret == ISRRAN_ERROR) {
    isrran_pcfich_free(q);
  }
  return ret;
}

void isrran_pcfich_free(isrran_pcfich_t* q)
{
  for (int ns = 0; ns < ISRRAN_NOF_SF_X_FRAME; ns++) {
    isrran_sequence_free(&q->seq[ns]);
  }
  isrran_modem_table_free(&q->mod);

  bzero(q, sizeof(isrran_pcfich_t));
}

int isrran_pcfich_set_cell(isrran_pcfich_t* q, isrran_regs_t* regs, isrran_cell_t cell)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && regs != NULL && isrran_cell_isvalid(&cell)) {
    q->regs = regs;
    if (cell.id != q->cell.id || q->cell.nof_prb == 0) {
      q->cell = cell;
      for (int nsf = 0; nsf < ISRRAN_NOF_SF_X_FRAME; nsf++) {
        if (isrran_sequence_pcfich(&q->seq[nsf], 2 * nsf, q->cell.id)) {
          return ISRRAN_ERROR;
        }
      }
    }
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

/** Finds the CFI with minimum distance with the vector of received 32 bits.
 * Saves the CFI value in the cfi pointer and returns the distance.
 */
float isrran_pcfich_cfi_decode(isrran_pcfich_t* q, uint32_t* cfi)
{
  int   i;
  int   index    = 0;
  float max_corr = 0;
  float corr[3];

  for (i = 0; i < 3; i++) {
    corr[i] = isrran_vec_dot_prod_fff(q->cfi_table_float[i], q->data_f, PCFICH_CFI_LEN);
    if (corr[i] > max_corr) {
      max_corr = corr[i];
      index    = i;
    }
  }

  if (cfi) {
    *cfi = index + 1;
  }
  return max_corr;
}

/** Encodes the CFI producing a vector of 32 bits.
 *  36.211 10.3 section 5.3.4
 */
int isrran_pcfich_cfi_encode(uint32_t cfi, uint8_t bits[PCFICH_CFI_LEN])
{
  if (cfi < 1 || cfi > 3) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  } else {
    memcpy(bits, cfi_table[cfi - 1], PCFICH_CFI_LEN * sizeof(uint8_t));
    return ISRRAN_SUCCESS;
  }
}

/* Decodes the PCFICH channel and saves the CFI in the cfi pointer.
 *
 * Returns 1 if successfully decoded the CFI, 0 if not and -1 on error
 */
int isrran_pcfich_decode(isrran_pcfich_t*       q,
                         isrran_dl_sf_cfg_t*    sf,
                         isrran_chest_dl_res_t* channel,
                         cf_t*                  sf_symbols[ISRRAN_MAX_PORTS],
                         float*                 corr_result)
{
  /* Set pointers for layermapping & precoding */
  int   i;
  cf_t* x[ISRRAN_MAX_LAYERS];

  if (q != NULL && sf_symbols != NULL) {
    uint32_t sf_idx = sf->tti % 10;

    /* number of layers equals number of ports */
    for (i = 0; i < ISRRAN_MAX_PORTS; i++) {
      x[i] = q->x[i];
    }

    cf_t* q_symbols[ISRRAN_MAX_PORTS];
    cf_t* q_ce[ISRRAN_MAX_PORTS][ISRRAN_MAX_PORTS];

    /* extract symbols */
    for (int j = 0; j < q->nof_rx_antennas; j++) {
      if (q->nof_symbols != isrran_regs_pcfich_get(q->regs, sf_symbols[j], q->symbols[j])) {
        ERROR("There was an error getting the PCFICH symbols");
        return ISRRAN_ERROR;
      }

      q_symbols[j] = q->symbols[j];

      /* extract channel estimates */
      for (i = 0; i < q->cell.nof_ports; i++) {
        if (q->nof_symbols != isrran_regs_pcfich_get(q->regs, channel->ce[i][j], q->ce[i][j])) {
          ERROR("There was an error getting the PCFICH symbols");
          return ISRRAN_ERROR;
        }
        q_ce[i][j] = q->ce[i][j];
      }
    }

    /* in control channels, only diversity is supported */
    if (q->cell.nof_ports == 1) {
      /* no need for layer demapping */
      isrran_predecoding_single_multi(
          q_symbols, q_ce[0], q->d, NULL, q->nof_rx_antennas, q->nof_symbols, 1.0f, channel->noise_estimate);
    } else {
      isrran_predecoding_diversity_multi(
          q_symbols, q_ce, x, NULL, q->nof_rx_antennas, q->cell.nof_ports, q->nof_symbols, 1.0f);
      isrran_layerdemap_diversity(x, q->d, q->cell.nof_ports, q->nof_symbols / q->cell.nof_ports);
    }

    /* demodulate symbols */
    isrran_demod_soft_demodulate(ISRRAN_MOD_QPSK, q->d, q->data_f, q->nof_symbols);

    /* Scramble with the sequence for slot nslot */
    isrran_scrambling_f(&q->seq[sf_idx], q->data_f);

    /* decode CFI */
    float corr = isrran_pcfich_cfi_decode(q, &sf->cfi);
    if (corr_result) {
      *corr_result = corr;
    }
    return 1;
  } else {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }
}

/** Encodes CFI and maps symbols to the slot
 */
int isrran_pcfich_encode(isrran_pcfich_t* q, isrran_dl_sf_cfg_t* sf, cf_t* slot_symbols[ISRRAN_MAX_PORTS])
{
  int i;

  if (q != NULL && slot_symbols != NULL) {
    uint32_t sf_idx = sf->tti % 10;

    /* Set pointers for layermapping & precoding */
    cf_t* x[ISRRAN_MAX_LAYERS];
    cf_t* q_symbols[ISRRAN_MAX_PORTS];

    /* number of layers equals number of ports */
    for (i = 0; i < q->cell.nof_ports; i++) {
      x[i] = q->x[i];
    }
    for (i = 0; i < ISRRAN_MAX_PORTS; i++) {
      q_symbols[i] = q->symbols[i];
    }

    /* pack CFI */
    isrran_pcfich_cfi_encode(sf->cfi, q->data);

    /* scramble for slot sequence nslot */
    isrran_scrambling_b(&q->seq[sf_idx], q->data);

    isrran_mod_modulate(&q->mod, q->data, q->d, PCFICH_CFI_LEN);

    /* layer mapping & precoding */
    if (q->cell.nof_ports > 1) {
      isrran_layermap_diversity(q->d, x, q->cell.nof_ports, q->nof_symbols);
      isrran_precoding_diversity(x, q_symbols, q->cell.nof_ports, q->nof_symbols / q->cell.nof_ports, 1.0f);
    } else {
      memcpy(q->symbols[0], q->d, q->nof_symbols * sizeof(cf_t));
    }

    /* mapping to resource elements */
    for (i = 0; i < q->cell.nof_ports; i++) {
      if (isrran_regs_pcfich_put(q->regs, q->symbols[i], slot_symbols[i]) < 0) {
        ERROR("Error putting PCHICH resource elements");
        return ISRRAN_ERROR;
      }
    }
    return ISRRAN_SUCCESS;
  } else {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }
}
