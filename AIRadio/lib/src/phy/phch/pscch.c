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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "isrran/phy/fec/convolutional/rm_conv.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/pscch.h"
#include "isrran/phy/phch/sch.h"
#include "isrran/phy/scrambling/scrambling.h"
#include "isrran/phy/utils/bit.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"

int isrran_pscch_init(isrran_pscch_t* q, uint32_t max_prb)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL) {
    ret = ISRRAN_ERROR;

    q->max_prb = max_prb;

    // CRC
    uint32_t crc_poly = 0x11021;
    if (isrran_crc_init(&q->crc, crc_poly, ISRRAN_SCI_CRC_LEN)) {
      return ISRRAN_ERROR;
    }
    q->c = isrran_vec_u8_malloc(ISRRAN_SCI_MAX_LEN + ISRRAN_SCI_CRC_LEN);
    if (!q->c) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }
    q->sci_crc = isrran_vec_u8_malloc(ISRRAN_SCI_CRC_LEN);
    if (!q->sci_crc) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }

    // Max E value for memory allocation
    uint32_t E_max = ISRRAN_NRE * ISRRAN_PSCCH_MAX_NUM_DATA_SYMBOLS * ISRRAN_PSCCH_MAX_NOF_PRB * ISRRAN_PSCCH_QM;

    // Channel Coding
    q->encoder.K           = 7;
    q->encoder.R           = 3;
    q->encoder.tail_biting = true;
    int poly[3]            = {0x6D, 0x4F, 0x57};
    memcpy(q->encoder.poly, poly, 3 * sizeof(int));
    q->d = isrran_vec_u8_malloc(E_max);
    if (!q->d) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }
    isrran_vec_u8_zero(q->d, E_max);

    q->d_16 = isrran_vec_i16_malloc(E_max);
    if (!q->d_16) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }

    isrran_viterbi_init(
        &q->dec, ISRRAN_VITERBI_37, q->encoder.poly, ISRRAN_SCI_MAX_LEN + ISRRAN_SCI_CRC_LEN, q->encoder.tail_biting);

    q->e = isrran_vec_u8_malloc(E_max);
    if (!q->e) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }
    q->e_16 = isrran_vec_i16_malloc(E_max);
    if (!q->e_16) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }
    q->e_bytes = isrran_vec_u8_malloc(E_max / 8);
    if (!q->e_bytes) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }

    q->interleaver_lut = isrran_vec_u32_malloc(E_max);
    if (!q->interleaver_lut) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }

    q->codeword = isrran_vec_u8_malloc(E_max);
    if (!q->codeword) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }
    q->codeword_bytes = isrran_vec_u8_malloc(E_max / 8);
    if (!q->codeword_bytes) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }

    // Scrambling
    bzero(&q->seq, sizeof(isrran_sequence_t));
    isrran_sequence_LTE_pr(&q->seq, E_max, ISRRAN_PSCCH_SCRAMBLING_SEED);

    // Modulation
    if (isrran_modem_table_lte(&q->mod, ISRRAN_MOD_QPSK)) {
      return ISRRAN_ERROR;
    }

    q->mod_symbols = isrran_vec_cf_malloc(E_max / ISRRAN_PSCCH_QM);
    if (!q->mod_symbols) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }
    isrran_vec_cf_zero(q->mod_symbols, E_max / ISRRAN_PSCCH_QM);

    q->llr = isrran_vec_i16_malloc(E_max);
    if (!q->llr) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }

    // DFT Precoding
    if (isrran_dft_precoding_init(&q->dft_precoder, ISRRAN_PSCCH_MAX_NOF_PRB, true)) {
      return ISRRAN_ERROR;
    }
    q->scfdma_symbols = isrran_vec_cf_malloc(E_max / ISRRAN_PSCCH_QM);
    if (!q->scfdma_symbols) {
      ERROR("Error allocating memory");
      return ISRRAN_ERROR;
    }
    isrran_vec_cf_zero(q->scfdma_symbols, E_max / ISRRAN_PSCCH_QM);

    // IDFT Predecoding
    if (isrran_dft_precoding_init(&q->idft_precoder, ISRRAN_PSCCH_MAX_NOF_PRB, false)) {
      return ISRRAN_ERROR;
    }

    ret = ISRRAN_SUCCESS;
  }

  return ret;
}

int isrran_pscch_set_cell(isrran_pscch_t* q, isrran_cell_sl_t cell)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;

  if (q != NULL && cell.nof_prb <= q->max_prb) {
    ret = ISRRAN_ERROR;

    if (cell.tm == ISRRAN_SIDELINK_TM1 || cell.tm == ISRRAN_SIDELINK_TM2) {
      q->sci_len       = isrran_sci_format0_sizeof(cell.nof_prb);
      q->nof_symbols   = ISRRAN_PSCCH_TM12_NUM_DATA_SYMBOLS;
      q->pscch_nof_prb = ISRRAN_PSCCH_TM12_NOF_PRB;
      q->E             = ISRRAN_PSCCH_TM12_NOF_CODED_BITS;

      if (cell.cp == ISRRAN_CP_EXT) {
        q->nof_symbols = ISRRAN_PSCCH_TM12_NUM_DATA_SYMBOLS_EXT;
      }
    } else if (cell.tm == ISRRAN_SIDELINK_TM3 || cell.tm == ISRRAN_SIDELINK_TM4) {
      q->sci_len       = ISRRAN_SCI_TM34_LEN;
      q->nof_symbols   = ISRRAN_PSCCH_TM34_NUM_DATA_SYMBOLS;
      q->pscch_nof_prb = ISRRAN_PSCCH_TM34_NOF_PRB;
      q->E             = ISRRAN_PSCCH_TM34_NOF_CODED_BITS;
    } else {
      return ret;
    }

    q->cell = cell;

    ///< Last OFDM symbol is processed but not transmitted
    q->nof_tx_re = (q->nof_symbols - 1) * ISRRAN_NRE * q->pscch_nof_prb;

    ret = ISRRAN_SUCCESS;
  }

  return ret;
}

int isrran_pscch_encode(isrran_pscch_t* q, uint8_t* sci, cf_t* sf_buffer, uint32_t prb_start_idx)
{
  memcpy(q->c, sci, sizeof(uint8_t) * q->sci_len);

  // CRC Attachment
  isrran_crc_attach(&q->crc, q->c, q->sci_len);

  // Channel Coding
  isrran_convcoder_encode(&q->encoder, q->c, q->d, q->sci_len + ISRRAN_SCI_CRC_LEN);

  // Rate Matching
  if (isrran_rm_conv_tx(q->d, (3 * (q->sci_len + ISRRAN_SCI_CRC_LEN)), q->e, q->E)) {
    return ISRRAN_ERROR;
  }

  // Interleaving
  isrran_bit_pack_vector(q->e, q->e_bytes, q->E);
  isrran_sl_ulsch_interleave(q->e_bytes,             // input bytes
                             ISRRAN_PSCCH_QM,        // modulation
                             q->E / ISRRAN_PSCCH_QM, // prime number
                             q->nof_symbols,         // nof pscch symbols
                             q->codeword_bytes       // output
  );
  isrran_bit_unpack_vector(q->codeword_bytes, q->codeword, q->E);

  // Scrambling
  isrran_scrambling_b(&q->seq, q->codeword);

  // Modulation
  isrran_mod_modulate(&q->mod, q->codeword, q->mod_symbols, q->E);

  // Layer Mapping
  // Void: Single layer
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.4.3

  // DFT Precoding
  isrran_dft_precoding(&q->dft_precoder, q->mod_symbols, q->scfdma_symbols, q->pscch_nof_prb, q->nof_symbols);

  // Precoding
  // Void: Single antenna port
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.4.5

  if (isrran_pscch_put(q, sf_buffer, prb_start_idx) != q->nof_tx_re) {
    printf("Error during PSCCH RE mapping\n");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_pscch_decode(isrran_pscch_t* q, cf_t* equalized_sf_syms, uint8_t* sci, uint32_t prb_start_idx)
{
  if (isrran_pscch_get(q, equalized_sf_syms, prb_start_idx) != q->nof_tx_re) {
    printf("Error during PSCCH RE extraction\n");
    return ISRRAN_ERROR;
  }

  // Precoding
  // Void: Single antenna port
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.4.5

  // IDFT Precoding
  isrran_dft_precoding(&q->idft_precoder, q->scfdma_symbols, q->mod_symbols, q->pscch_nof_prb, q->nof_symbols);

  // Layer Mapping
  // Void: Single layer
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.4.3

  // Demodulation
  isrran_demod_soft_demodulate_s(ISRRAN_MOD_QPSK, q->mod_symbols, q->llr, q->E / ISRRAN_PSCCH_QM);

  // Descrambling
  isrran_scrambling_s(&q->seq, q->llr);

  // Deinterleaving
  isrran_sl_ulsch_deinterleave(
      q->llr, ISRRAN_PSCCH_QM, q->E / ISRRAN_PSCCH_QM, q->nof_symbols, q->e_16, q->interleaver_lut);

  // Rate matching
  isrran_rm_conv_rx_s(q->e_16, q->E, q->d_16, (3 * (q->sci_len + ISRRAN_SCI_CRC_LEN)));

  // Channel decoding
  isrran_viterbi_decode_s(&q->dec, q->d_16, q->c, q->sci_len + ISRRAN_SCI_CRC_LEN);

  // Copy received crc
  memcpy(q->sci_crc, &q->c[q->sci_len], sizeof(uint8_t) * ISRRAN_SCI_CRC_LEN);

  // Re-attach crc
  isrran_crc_attach(&q->crc, q->c, q->sci_len);

  // CRC check
  if (isrran_bit_diff(q->sci_crc, &q->c[q->sci_len], ISRRAN_SCI_CRC_LEN) != 0) {
    return ISRRAN_ERROR;
  }

  // Remove CRC and copy content to sci buffer
  memcpy(sci, q->c, sizeof(uint8_t) * q->sci_len);

  return ISRRAN_SUCCESS;
}

int isrran_pscch_put(isrran_pscch_t* q, cf_t* sf_buffer, uint32_t prb_start_idx)
{
  int sample_pos = 0;
  int k          = prb_start_idx * ISRRAN_NRE;
  for (int i = 0; i < isrran_sl_get_num_symbols(q->cell.tm, q->cell.cp); ++i) {
    if (isrran_pscch_is_symbol(ISRRAN_SIDELINK_DATA_SYMBOL, q->cell.tm, i, q->cell.cp)) {
      memcpy(&sf_buffer[k + i * q->cell.nof_prb * ISRRAN_NRE],
             &q->scfdma_symbols[sample_pos],
             sizeof(cf_t) * (ISRRAN_NRE * q->pscch_nof_prb));
      sample_pos += (ISRRAN_NRE * q->pscch_nof_prb);
    }
  }
  return sample_pos;
}

int isrran_pscch_get(isrran_pscch_t* q, cf_t* sf_buffer, uint32_t prb_start_idx)
{
  int sample_pos = 0;
  int k          = prb_start_idx * ISRRAN_NRE;
  for (int i = 0; i < isrran_sl_get_num_symbols(q->cell.tm, q->cell.cp); ++i) {
    if (isrran_pscch_is_symbol(ISRRAN_SIDELINK_DATA_SYMBOL, q->cell.tm, i, q->cell.cp)) {
      memcpy(&q->scfdma_symbols[sample_pos],
             &sf_buffer[k + i * q->cell.nof_prb * ISRRAN_NRE],
             sizeof(cf_t) * (ISRRAN_NRE * q->pscch_nof_prb));
      sample_pos += (ISRRAN_NRE * q->pscch_nof_prb);
    }
  }

  // Force zeros in last symbol
  isrran_vec_cf_zero(&q->scfdma_symbols[sample_pos], ISRRAN_NRE * q->pscch_nof_prb);

  return sample_pos;
}

void isrran_pscch_free(isrran_pscch_t* q)
{
  if (q != NULL) {
    isrran_dft_precoding_free(&q->dft_precoder);
    isrran_dft_precoding_free(&q->idft_precoder);
    isrran_viterbi_free(&q->dec);
    isrran_sequence_free(&q->seq);
    isrran_modem_table_free(&q->mod);

    if (q->sci_crc) {
      free(q->sci_crc);
    }
    if (q->c) {
      free(q->c);
    }
    if (q->d) {
      free(q->d);
    }
    if (q->d_16) {
      free(q->d_16);
    }
    if (q->e) {
      free(q->e);
    }
    if (q->e_16) {
      free(q->e_16);
    }
    if (q->e_bytes) {
      free(q->e_bytes);
    }
    if (q->interleaver_lut) {
      free(q->interleaver_lut);
    }
    if (q->codeword) {
      free(q->codeword);
    }
    if (q->codeword_bytes) {
      free(q->codeword_bytes);
    }
    if (q->llr) {
      free(q->llr);
    }
    if (q->mod_symbols) {
      free(q->mod_symbols);
    }
    if (q->scfdma_symbols) {
      free(q->scfdma_symbols);
    }

    bzero(q, sizeof(isrran_pscch_t));
  }
}
