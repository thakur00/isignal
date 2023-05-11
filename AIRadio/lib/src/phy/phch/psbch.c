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

#include "isrran/phy/phch/psbch.h"
#include "isrran/phy/fec/convolutional/rm_conv.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/phch/sch.h"
#include "isrran/phy/utils/bit.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/vector.h"
#include <stdlib.h>
#include <string.h>

int isrran_psbch_init(isrran_psbch_t* q, uint32_t nof_prb, uint32_t N_sl_id, isrran_sl_tm_t tm, isrran_cp_t cp)
{
  bzero(q, sizeof(isrran_psbch_t));

  q->N_sl_id = N_sl_id;
  q->tm      = tm;
  q->nof_prb = nof_prb;

  if (ISRRAN_CP_ISEXT(cp) && (tm >= ISRRAN_SIDELINK_TM3)) {
    ERROR("Selected TM does not support extended CP");
    return ISRRAN_ERROR;
  }
  q->cp = cp;

  // Calculate rate matching params
  if (q->tm <= ISRRAN_SIDELINK_TM2) {
    q->nof_data_symbols = ISRRAN_PSBCH_TM12_NUM_DATA_SYMBOLS;
    q->sl_bch_tb_len    = ISRRAN_MIB_SL_LEN;
    if (ISRRAN_CP_ISEXT(cp)) {
      q->nof_data_symbols = ISRRAN_PSBCH_TM12_NUM_DATA_SYMBOLS_EXT;
    }
  } else {
    q->nof_data_symbols = ISRRAN_PSBCH_TM34_NUM_DATA_SYMBOLS;
    q->sl_bch_tb_len    = ISRRAN_MIB_SL_V2X_LEN;
  }
  q->nof_tx_symbols     = q->nof_data_symbols - 1; ///< Last OFDM symbol is used channel processing but not transmitted
  q->nof_data_re        = q->nof_data_symbols * (ISRRAN_NRE * ISRRAN_PSBCH_NOF_PRB);
  q->nof_tx_re          = q->nof_tx_symbols * (ISRRAN_NRE * ISRRAN_PSBCH_NOF_PRB);
  q->sl_bch_tb_crc_len  = q->sl_bch_tb_len + ISRRAN_SL_BCH_CRC_LEN;
  q->sl_bch_encoded_len = 3 * q->sl_bch_tb_crc_len;

  q->c = isrran_vec_u8_malloc(q->sl_bch_tb_crc_len);
  if (!q->c) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  q->d = isrran_vec_u8_malloc(q->sl_bch_encoded_len);
  if (!q->d) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  q->d_16 = isrran_vec_i16_malloc(q->sl_bch_encoded_len);
  if (!q->d_16) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  // CRC
  if (isrran_crc_init(&q->crc_mib_sl, ISRRAN_LTE_CRC16, ISRRAN_SL_BCH_CRC_LEN)) {
    ERROR("Error crc init");
    return ISRRAN_ERROR;
  }

  q->crc_temp = isrran_vec_u8_malloc(ISRRAN_SL_BCH_CRC_LEN);
  if (!q->crc_temp) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  // Channel coding
  q->encoder.K           = 7;
  q->encoder.R           = 3;
  q->encoder.tail_biting = true;
  int poly[3]            = {0x6D, 0x4F, 0x57};
  memcpy(q->encoder.poly, poly, 3 * sizeof(int));

  if (isrran_viterbi_init(&q->dec, ISRRAN_VITERBI_37, poly, q->sl_bch_tb_crc_len, true)) {
    return ISRRAN_ERROR;
  }

  // QPSK modulation
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.6 - Table 9.6.2-1: PSBCH modulation schemes
  q->Qm = isrran_mod_bits_x_symbol(ISRRAN_MOD_QPSK);
  q->E  = q->nof_data_re * q->Qm;

  q->e = isrran_vec_u8_malloc(q->E);
  if (!q->e) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  q->e_16 = isrran_vec_i16_malloc(q->E);
  if (!q->e_16) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  q->e_bytes = isrran_vec_u8_malloc(q->E / 8);
  if (!q->e_bytes) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  // Scrambling
  bzero(&q->seq, sizeof(isrran_sequence_t));
  if (isrran_sequence_LTE_pr(&q->seq, q->E, N_sl_id) != ISRRAN_SUCCESS) {
    ERROR("Error isrran_sequence_LTE_pr");
    return ISRRAN_ERROR;
  }

  q->codeword = isrran_vec_u8_malloc(q->E);
  if (!q->codeword) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  q->codeword_bytes = isrran_vec_u8_malloc(q->E / 8);
  if (!q->codeword_bytes) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  // Interleaving
  q->interleaver_lut = isrran_vec_u32_malloc(q->E);
  if (!q->interleaver_lut) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  // Modulation QPSK
  if (isrran_modem_table_lte(&q->mod, ISRRAN_MOD_QPSK) != ISRRAN_SUCCESS) {
    ERROR("Error isrran_modem_table_lte");
    return ISRRAN_ERROR;
  }

  q->mod_symbols = isrran_vec_cf_malloc(q->nof_data_re);
  if (!q->mod_symbols) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  // Soft-demod
  q->llr = isrran_vec_i16_malloc(q->E);
  if (!q->llr) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }

  // Transform precoding
  q->precoding_scaling = 1.0f;
  if (isrran_dft_precoding_init_tx(&q->dft_precoder, ISRRAN_PSBCH_NOF_PRB) != ISRRAN_SUCCESS) {
    ERROR("Error isrran_dft_precoding_init");
    return ISRRAN_ERROR;
  }

  q->scfdma_symbols = isrran_vec_cf_malloc(q->nof_data_re);
  if (!q->scfdma_symbols) {
    ERROR("Error allocating memory");
    return ISRRAN_ERROR;
  }
  ///< Make sure last bits are zero as they are not considered during unpack
  isrran_vec_cf_zero(q->scfdma_symbols, q->nof_data_re);

  if (isrran_dft_precoding_init_rx(&q->idft_precoder, ISRRAN_PSBCH_NOF_PRB) != ISRRAN_SUCCESS) {
    ERROR("Error isrran_idft_precoding_init");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_psbch_encode(isrran_psbch_t* q, uint8_t* input, uint32_t input_len, cf_t* sf_buffer)
{
  if (input == NULL || input_len > q->sl_bch_tb_len) {
    ERROR("Can't encode PSBCH, input too long (%d > %d)", input_len, q->sl_bch_tb_len);
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Copy into codeword buffer
  isrran_vec_u8_copy(q->c, input, input_len);

  // CRC Attachment
  isrran_crc_attach(&q->crc_mib_sl, q->c, input_len);

  // Channel Coding
  isrran_convcoder_encode(&q->encoder, q->c, q->d, q->sl_bch_tb_crc_len);

  // Rate matching
  isrran_rm_conv_tx(q->d, q->sl_bch_encoded_len, q->e, q->E);

  // Interleaving
  isrran_bit_pack_vector(q->e, q->e_bytes, q->E);
  isrran_sl_ulsch_interleave(q->e_bytes,          // input bytes
                             q->Qm,               // modulation
                             q->nof_data_re,      // prime number
                             q->nof_data_symbols, // number of symbols
                             q->codeword_bytes    // output
  );
  isrran_bit_unpack_vector(q->codeword_bytes, q->codeword, q->E);

  // Scrambling
  isrran_scrambling_b(&q->seq, q->codeword);

  // Modulation
  isrran_mod_modulate(&q->mod, q->codeword, q->mod_symbols, q->E);

  // Layer Mapping
  // Void: Single layer
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.6.3

  // Transform precoding
  isrran_dft_precoding(&q->dft_precoder, q->mod_symbols, q->scfdma_symbols, ISRRAN_PSBCH_NOF_PRB, q->nof_data_symbols);

  // Precoding
  // Void: Single antenna port
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.6.5

  // RE mapping
  isrran_psbch_put(q, q->scfdma_symbols, sf_buffer);

  return ISRRAN_SUCCESS;
}

int isrran_psbch_decode(isrran_psbch_t* q, cf_t* equalized_sf_syms, uint8_t* output, uint32_t max_output_len)
{
  if (max_output_len < q->sl_bch_tb_len) {
    ERROR("Can't decode PSBCH, provided buffer too small (%d < %d)", max_output_len, q->sl_bch_tb_len);
    return ISRRAN_ERROR;
  }

  // RE extraction
  if (q->nof_tx_re != isrran_psbch_get(q, equalized_sf_syms, q->scfdma_symbols)) {
    ERROR("There was an error getting the PSBCH symbols");
    return ISRRAN_ERROR;
  }

  // Precoding
  // Void: Single antenna port
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.6.5

  // Transform precoding
  isrran_dft_precoding(&q->idft_precoder, q->scfdma_symbols, q->mod_symbols, ISRRAN_PSBCH_NOF_PRB, q->nof_data_symbols);

  // Layer Mapping
  // Void: Single layer
  // 3GPP TS 36.211 version 15.6.0 Release 15 Sec. 9.6.3

  // Demodulation
  isrran_demod_soft_demodulate_s(ISRRAN_MOD_QPSK, q->mod_symbols, q->llr, q->nof_data_re);

  // De-scramble
  isrran_scrambling_s(&q->seq, q->llr);

  // Deinterleaving
  isrran_sl_ulsch_deinterleave(q->llr, q->Qm, q->nof_data_re, q->nof_data_symbols, q->e_16, q->interleaver_lut);

  // Rate match
  isrran_rm_conv_rx_s(q->e_16, q->E, q->d_16, q->sl_bch_encoded_len);

  // Channel decoding
  isrran_viterbi_decode_s(&q->dec, q->d_16, q->c, q->sl_bch_tb_crc_len);

  // Copy received crc to temp
  isrran_vec_u8_copy(q->crc_temp, &q->c[q->sl_bch_tb_len], ISRRAN_SL_BCH_CRC_LEN);

  // Re-attach crc
  isrran_crc_attach(&q->crc_mib_sl, q->c, q->sl_bch_tb_len);

  // CRC check
  if (isrran_bit_diff(q->crc_temp, &q->c[q->sl_bch_tb_len], ISRRAN_SL_BCH_CRC_LEN) != 0) {
    return ISRRAN_ERROR;
  }

  // Remove CRC and copy to output buffer
  isrran_vec_u8_copy(output, q->c, q->sl_bch_tb_len);

  return ISRRAN_SUCCESS;
}

int isrran_psbch_reset(isrran_psbch_t* q, uint32_t N_sl_id)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q != NULL) {
    if (q->N_sl_id != N_sl_id) {
      q->N_sl_id = N_sl_id;

      // Regen scrambling sequence
      if (isrran_sequence_LTE_pr(&q->seq, q->E, N_sl_id) != ISRRAN_SUCCESS) {
        ERROR("Error isrran_sequence_LTE_pr");
        return ISRRAN_ERROR;
      }
    }
    ret = ISRRAN_SUCCESS;
  }
  return ret;
}

int isrran_psbch_put(isrran_psbch_t* q, cf_t* symbols, cf_t* sf_buffer)
{
  uint32_t sample_pos = 0;
  uint32_t k          = q->nof_prb * ISRRAN_NRE / 2 - 36;

  // Mapping to physical resources
  for (uint32_t i = 0; i < isrran_sl_get_num_symbols(q->tm, q->cp); i++) {
    if (isrran_psbch_is_symbol(ISRRAN_SIDELINK_DATA_SYMBOL, q->tm, i, q->cp)) {
      isrran_vec_cf_copy(
          &sf_buffer[k + i * q->nof_prb * ISRRAN_NRE], &symbols[sample_pos], ISRRAN_NRE * ISRRAN_PSBCH_NOF_PRB);
      sample_pos += (ISRRAN_NRE * ISRRAN_PSBCH_NOF_PRB);
    }
  }

  return sample_pos;
}

int isrran_psbch_get(isrran_psbch_t* q, cf_t* sf_buffer, cf_t* symbols)
{
  uint32_t sample_pos = 0;
  uint32_t k          = q->nof_prb * ISRRAN_NRE / 2 - 36;

  // Get PSBCH REs
  for (uint32_t i = 0; i < isrran_sl_get_num_symbols(q->tm, q->cp); i++) {
    if (isrran_psbch_is_symbol(ISRRAN_SIDELINK_DATA_SYMBOL, q->tm, i, q->cp)) {
      isrran_vec_cf_copy(
          &symbols[sample_pos], &sf_buffer[k + i * q->nof_prb * ISRRAN_NRE], ISRRAN_NRE * ISRRAN_PSBCH_NOF_PRB);
      sample_pos += (ISRRAN_NRE * ISRRAN_PSBCH_NOF_PRB);
    }
  }

  return sample_pos;
}

void isrran_psbch_free(isrran_psbch_t* q)
{
  if (q) {
    isrran_dft_precoding_free(&q->dft_precoder);
    isrran_dft_precoding_free(&q->idft_precoder);
    isrran_viterbi_free(&q->dec);
    isrran_sequence_free(&q->seq);
    isrran_modem_table_free(&q->mod);

    if (q->crc_temp) {
      free(q->crc_temp);
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
    if (q->e_bytes) {
      free(q->e_bytes);
    }
    if (q->e_16) {
      free(q->e_16);
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

    bzero(q, sizeof(isrran_psbch_t));
  }
}
