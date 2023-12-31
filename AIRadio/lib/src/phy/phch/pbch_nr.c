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

#include "isrran/phy/phch/pbch_nr.h"
#include "isrran/phy/common/sequence.h"
#include "isrran/phy/fec/polar/polar_chanalloc.h"
#include "isrran/phy/fec/polar/polar_interleaver.h"
#include "isrran/phy/mimo/precoding.h"
#include "isrran/phy/modem/demod_soft.h"
#include "isrran/phy/modem/mod.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/simd.h"
#include "isrran/phy/utils/vector.h"

#define PBCH_NR_DEBUG_TX(...) DEBUG("PBCH-NR Tx: " __VA_ARGS__)
#define PBCH_NR_DEBUG_RX(...) DEBUG("PBCH-NR Rx: " __VA_ARGS__)

/*
 * CRC Parameters
 */
#define PBCH_NR_CRC ISRRAN_LTE_CRC24C
#define PBCH_NR_CRC_LEN 24

/*
 * Polar code N_max
 */
#define PBCH_NR_POLAR_N_MAX 9U

/*
 * Polar rate matching I_BIL
 */
#define PBCH_NR_POLAR_RM_IBIL 0

/*
 * Number of generated payload bits, called A
 */
#define PBCH_NR_A (ISRRAN_PBCH_MSG_NR_SZ + 8)

/*
 * Number of payload bits plus CRC
 */
#define PBCH_NR_K (PBCH_NR_A + PBCH_NR_CRC_LEN)

/*
 * Number of Polar encoded bits
 */
#define PBCH_NR_N (1U << PBCH_NR_POLAR_N_MAX)

/*
 * Number of RM bits
 */
#define PBCH_NR_E (864)

/*
 * Number of symbols
 */
#define PBCH_NR_M (PBCH_NR_E / 2)

static int pbch_nr_init_encoder(isrran_pbch_nr_t* q, const isrran_pbch_nr_args_t* args)
{
  // Skip encoder init if not requested
  if (!args->enable_encode) {
    return ISRRAN_SUCCESS;
  }

  isrran_polar_encoder_type_t encoder_type = ISRRAN_POLAR_ENCODER_PIPELINED;

#ifdef LV_HAVE_AVX2
  if (!args->disable_simd) {
    encoder_type = ISRRAN_POLAR_ENCODER_AVX2;
  }
#endif /* LV_HAVE_AVX2 */

  if (isrran_polar_encoder_init(&q->polar_encoder, encoder_type, PBCH_NR_POLAR_N_MAX) < ISRRAN_SUCCESS) {
    ERROR("Error initiating polar encoder");
    return ISRRAN_ERROR;
  }

  if (isrran_polar_rm_tx_init(&q->polar_rm_tx) < ISRRAN_SUCCESS) {
    ERROR("Error initiating polar RM");
    return ISRRAN_ERROR;
  }

  if (isrran_modem_table_lte(&q->qpsk, ISRRAN_MOD_QPSK) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int pbch_nr_init_decoder(isrran_pbch_nr_t* q, const isrran_pbch_nr_args_t* args)
{
  // Skip decoder init if not requested
  if (!args->enable_decode) {
    return ISRRAN_SUCCESS;
  }

  isrran_polar_decoder_type_t decoder_type = ISRRAN_POLAR_DECODER_SSC_C;

#ifdef LV_HAVE_AVX2
  if (!args->disable_simd) {
    decoder_type = ISRRAN_POLAR_DECODER_SSC_C_AVX2;
  }
#endif /* LV_HAVE_AVX2 */

  if (isrran_polar_decoder_init(&q->polar_decoder, decoder_type, PBCH_NR_POLAR_N_MAX) < ISRRAN_SUCCESS) {
    ERROR("Error initiating polar decoder");
    return ISRRAN_ERROR;
  }

  if (isrran_polar_rm_rx_init_c(&q->polar_rm_rx) < ISRRAN_SUCCESS) {
    ERROR("Error initiating polar RM");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_pbch_nr_init(isrran_pbch_nr_t* q, const isrran_pbch_nr_args_t* args)
{
  if (q == NULL || args == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (!args->enable_encode && !args->enable_decode) {
    ERROR("Encoder and decoder are disabled, at least one of them shall be active");
    return ISRRAN_ERROR;
  }

  if (pbch_nr_init_encoder(q, args) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (pbch_nr_init_decoder(q, args) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (isrran_crc_init(&q->crc, PBCH_NR_CRC, PBCH_NR_CRC_LEN) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (isrran_polar_code_init(&q->code) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (isrran_polar_code_get(&q->code, PBCH_NR_K, PBCH_NR_E, PBCH_NR_POLAR_N_MAX) < ISRRAN_SUCCESS) {
    ERROR("Error Getting polar code");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

void isrran_pbch_nr_free(isrran_pbch_nr_t* q)
{
  if (q == NULL) {
    return;
  }
  isrran_polar_encoder_free(&q->polar_encoder);
  isrran_polar_decoder_free(&q->polar_decoder);
  isrran_polar_rm_rx_free_c(&q->polar_rm_rx);
  isrran_polar_rm_tx_free(&q->polar_rm_tx);
  isrran_polar_code_free(&q->code);
  isrran_modem_table_free(&q->qpsk);
  ISRRAN_MEM_ZERO(q, isrran_pbch_nr_t, 1);
}

/*
 *  Implements TS 38.212 Table 7.1.1-1: Value of PBCH payload interleaver pattern G ( j )
 */
static const uint32_t G[PBCH_NR_A] = {16, 23, 18, 17, 8,  30, 10, 6,  24, 7,  0,  5,  3,  2,  1,  4,
                                      9,  11, 12, 13, 14, 15, 19, 20, 21, 22, 25, 26, 27, 28, 29, 31};

#define PBCH_SFN_PAYLOAD_BEGIN 1
#define PBCH_SFN_PAYLOAD_LENGTH 6
#define PBCH_SFN_2ND_LSB_G (G[PBCH_SFN_PAYLOAD_LENGTH + 2])
#define PBCH_SFN_3RD_LSB_G (G[PBCH_SFN_PAYLOAD_LENGTH + 1])

static void
pbch_nr_pbch_msg_pack(const isrran_pbch_nr_cfg_t* cfg, const isrran_pbch_msg_nr_t* msg, uint8_t a[PBCH_NR_A])
{
  // Extract actual payload size
  uint32_t A_hat = ISRRAN_PBCH_MSG_NR_SZ;

  // Put actual payload
  uint32_t j_sfn   = 0;
  uint32_t j_other = 14;
  for (uint32_t i = 0; i < A_hat; i++) {
    if (i >= PBCH_SFN_PAYLOAD_BEGIN && i < (PBCH_SFN_PAYLOAD_BEGIN + PBCH_SFN_PAYLOAD_LENGTH)) {
      a[G[j_sfn++]] = msg->payload[i];
    } else {
      a[G[j_other++]] = msg->payload[i];
    }
  }

  // Put SFN in a_hat[A_hat] to a_hat[A_hat + 3]
  a[G[j_sfn++]] = (uint8_t)((msg->sfn_4lsb >> 3U) & 1U); // 4th LSB of SFN
  a[G[j_sfn++]] = (uint8_t)((msg->sfn_4lsb >> 2U) & 1U); // 3th LSB of SFN
  a[G[j_sfn++]] = (uint8_t)((msg->sfn_4lsb >> 1U) & 1U); // 2th LSB of SFN
  a[G[j_sfn++]] = (uint8_t)((msg->sfn_4lsb >> 0U) & 1U); // 1th LSB of SFN

  // Put HRF in a_hat[A_hat + 4]
  a[G[10]] = (msg->hrf ? 1 : 0);

  // Put SSB related in a_hat[A_hat + 5] to a_hat[A_hat + 7]
  if (cfg->Lmax == 64) {
    a[G[11]] = (uint8_t)((msg->ssb_idx >> 5U) & 1U); // 6th bit of SSB index
    a[G[12]] = (uint8_t)((msg->ssb_idx >> 4U) & 1U); // 5th bit of SSB index
    a[G[13]] = (uint8_t)((msg->ssb_idx >> 3U) & 1U); // 4th bit of SSB index
  } else {
    a[G[11]] = (uint8_t)msg->k_ssb_msb; // 6th bit of SSB index
    a[G[12]] = 0;                       // Reserved
    a[G[13]] = 0;                       // Reserved
  }

  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
    PBCH_NR_DEBUG_TX("Packed PBCH bits: ");
    isrran_vec_fprint_byte(stdout, a, PBCH_NR_A);
  }
}
static void
pbch_nr_pbch_msg_unpack(const isrran_pbch_nr_cfg_t* cfg, const uint8_t a[PBCH_NR_A], isrran_pbch_msg_nr_t* msg)
{
  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
    PBCH_NR_DEBUG_RX("Packed PBCH bits: ");
    isrran_vec_fprint_byte(stdout, a, PBCH_NR_A);
  }

  // Extract actual payload size
  uint32_t A_hat = ISRRAN_PBCH_MSG_NR_SZ;

  // Get actual payload
  uint32_t j_sfn   = 0;
  uint32_t j_other = 14;
  for (uint32_t i = 0; i < A_hat; i++) {
    if (i >= PBCH_SFN_PAYLOAD_BEGIN && i < (PBCH_SFN_PAYLOAD_BEGIN + PBCH_SFN_PAYLOAD_LENGTH)) {
      msg->payload[i] = a[G[j_sfn++]];
    } else {
      msg->payload[i] = a[G[j_other++]];
    }
  }

  // Put SFN in a_hat[A_hat] to a_hat[A_hat + 3]
  msg->sfn_4lsb = 0;
  msg->sfn_4lsb |= (uint8_t)(a[G[j_sfn++]] << 3U); // 4th LSB of SFN
  msg->sfn_4lsb |= (uint8_t)(a[G[j_sfn++]] << 2U); // 3th LSB of SFN
  msg->sfn_4lsb |= (uint8_t)(a[G[j_sfn++]] << 1U); // 2th LSB of SFN
  msg->sfn_4lsb |= (uint8_t)(a[G[j_sfn++]] << 0U); // 1th LSB of SFN

  // Put HRF in a_hat[A_hat + 4]
  msg->hrf = (a[G[10]] == 1);

  // Put SSB related in a_hat[A_hat + 5] to a_hat[A_hat + 7]
  msg->ssb_idx = cfg->ssb_idx; // Load 4 LSB
  if (cfg->Lmax == 64) {
    msg->ssb_idx = msg->ssb_idx & 0b111;
    msg->ssb_idx |= (uint8_t)(a[G[11]] << 5U); // 6th bit of SSB index
    msg->ssb_idx |= (uint8_t)(a[G[12]] << 4U); // 5th bit of SSB index
    msg->ssb_idx |= (uint8_t)(a[G[13]] << 3U); // 4th bit of SSB index
  } else {
    msg->k_ssb_msb = a[G[11]];
  }
}

static void pbch_nr_scramble(const isrran_pbch_nr_cfg_t* cfg, const uint8_t a[PBCH_NR_A], uint8_t a_prime[PBCH_NR_A])
{
  uint32_t i = 0;
  uint32_t j = 0;

  // Initialise sequence
  isrran_sequence_state_t sequence_state = {};
  isrran_sequence_state_init(&sequence_state, ISRRAN_SEQUENCE_MOD(cfg->N_id));

  // Select value M
  uint32_t M = PBCH_NR_A - 3;
  if (cfg->Lmax == 64) {
    M = PBCH_NR_A - 6;
  }

  // Select value v
  uint32_t v = 2 * a[PBCH_SFN_3RD_LSB_G] + a[PBCH_SFN_2ND_LSB_G];

  // Advance sequence
  isrran_sequence_state_advance(&sequence_state, M * v);

  // Generate actual sequence
  uint8_t c[PBCH_NR_A] = {};
  isrran_sequence_state_apply_bit(&sequence_state, c, c, PBCH_NR_A);

  while (i < PBCH_NR_A) {
    uint8_t s_i = c[j];

    // Check if i belongs to a SS/PBCH block index which is only multiplexed when L_max is 64
    bool is_ssb_idx = (i == G[11] || i == G[12] || i == G[13]) && cfg->Lmax == 64;

    // a i corresponds to any one of the bits belonging to the SS/PBCH block index, the half frame index, and 2 nd and 3
    // rd least significant bits of the system frame number
    if (is_ssb_idx || i == G[10] || i == PBCH_SFN_2ND_LSB_G || i == PBCH_SFN_3RD_LSB_G) {
      s_i = 0;
    } else {
      j++;
    }

    a_prime[i] = a[i] ^ s_i;
    i++;
  }
}

static int pbch_nr_polar_encode(isrran_pbch_nr_t* q, const uint8_t c[PBCH_NR_K], uint8_t d[PBCH_NR_N])
{
  // Interleave
  uint8_t c_prime[ISRRAN_POLAR_INTERLEAVER_K_MAX_IL];
  isrran_polar_interleaver_run_u8(c, c_prime, PBCH_NR_K, true);

  // Allocate channel
  uint8_t allocated[PBCH_NR_N];
  isrran_polar_chanalloc_tx(c_prime, allocated, q->code.N, q->code.K, q->code.nPC, q->code.K_set, q->code.PC_set);

  // Encode bits
  if (isrran_polar_encoder_encode(&q->polar_encoder, allocated, d, q->code.n) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
    PBCH_NR_DEBUG_TX("Allocated: ");
    isrran_vec_fprint_byte(stdout, allocated, PBCH_NR_N);
  }

  return ISRRAN_SUCCESS;
}

static int pbch_nr_polar_decode(isrran_pbch_nr_t* q, const int8_t d[PBCH_NR_N], uint8_t c[PBCH_NR_K])
{
  // Decode bits
  uint8_t allocated[PBCH_NR_N];
  if (isrran_polar_decoder_decode_c(&q->polar_decoder, d, allocated, q->code.n, q->code.F_set, q->code.F_set_size) <
      ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
    PBCH_NR_DEBUG_RX("Allocated: ");
    isrran_vec_fprint_byte(stdout, allocated, PBCH_NR_N);
  }

  // Allocate channel
  uint8_t c_prime[ISRRAN_POLAR_INTERLEAVER_K_MAX_IL];
  isrran_polar_chanalloc_rx(allocated, c_prime, q->code.K, q->code.nPC, q->code.K_set, q->code.PC_set);

  // Interleave
  isrran_polar_interleaver_run_u8(c_prime, c, PBCH_NR_K, false);

  return ISRRAN_SUCCESS;
}

static int pbch_nr_polar_rm_tx(isrran_pbch_nr_t* q, const uint8_t d[PBCH_NR_N], uint8_t o[PBCH_NR_E])
{
  if (isrran_polar_rm_tx(&q->polar_rm_tx, d, o, q->code.n, PBCH_NR_E, PBCH_NR_K, PBCH_NR_POLAR_RM_IBIL) <
      ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
    PBCH_NR_DEBUG_TX("d: ");
    isrran_vec_fprint_byte(stdout, d, PBCH_NR_N);
  }

  return ISRRAN_SUCCESS;
}

static int pbch_nr_polar_rm_rx(isrran_pbch_nr_t* q, const int8_t llr[PBCH_NR_E], int8_t d[PBCH_NR_N])
{
  if (isrran_polar_rm_rx_c(&q->polar_rm_rx, llr, d, PBCH_NR_E, q->code.n, PBCH_NR_K, PBCH_NR_POLAR_RM_IBIL) <
      ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Negate all LLR
  for (uint32_t i = 0; i < PBCH_NR_N; i++) {
    d[i] *= -1;
  }

  if (get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
    PBCH_NR_DEBUG_RX("d: ");
    isrran_vec_fprint_bs(stdout, d, PBCH_NR_N);
  }

  return ISRRAN_SUCCESS;
}

static void pbch_nr_scramble_tx(const isrran_pbch_nr_cfg_t* cfg,
                                uint32_t                    ssb_idx,
                                const uint8_t               b[PBCH_NR_E],
                                uint8_t                     b_hat[PBCH_NR_E])
{
  // Initialise sequence
  isrran_sequence_state_t sequence_state = {};
  isrran_sequence_state_init(&sequence_state, ISRRAN_SEQUENCE_MOD(cfg->N_id));

  // Select value M
  uint32_t M_bit = PBCH_NR_E;

  // Select value v
  // for L max = 8 or L max = 64 , & is the three least significant bits of the SS/PBCH block index
  uint32_t v = (ssb_idx & 0b111U);
  if (cfg->Lmax == 4) {
    // for L max = 4 , & is the two least significant bits of the SS/PBCH block index
    v = ssb_idx & 0b11U;
  }

  // Advance sequence
  isrran_sequence_state_advance(&sequence_state, v * M_bit);

  // Apply sequence
  isrran_sequence_state_apply_bit(&sequence_state, b, b_hat, PBCH_NR_E);
}

static void pbch_nr_scramble_rx(const isrran_pbch_nr_cfg_t* cfg,
                                uint32_t                    ssb_idx,
                                const int8_t                b_hat[PBCH_NR_E],
                                int8_t                      b[PBCH_NR_E])
{
  // Initialise sequence
  isrran_sequence_state_t sequence_state = {};
  isrran_sequence_state_init(&sequence_state, ISRRAN_SEQUENCE_MOD(cfg->N_id));

  // Select value M
  uint32_t M_bit = PBCH_NR_E;

  // Select value v
  // for L max = 8 or L max = 64 , & is the three least significant bits of the SS/PBCH block index
  uint32_t v = (ssb_idx & 0b111U);
  if (cfg->Lmax == 4) {
    // for L max = 4 , & is the two least significant bits of the SS/PBCH block index
    v = ssb_idx & 0b11U;
  }

  // Advance sequence
  isrran_sequence_state_advance(&sequence_state, v * M_bit);

  // Apply sequence
  isrran_sequence_state_apply_c(&sequence_state, b_hat, b, PBCH_NR_E);
}

static void
pbch_nr_mapping(const isrran_pbch_nr_cfg_t* cfg, const cf_t symbols[PBCH_NR_M], cf_t ssb_grid[ISRRAN_SSB_NOF_RE])
{
  uint32_t count = 0;

  // PBCH DMRS shift
  uint32_t v = cfg->N_id % 4;

  // Symbol 1
  for (uint32_t k = 0; k < ISRRAN_SSB_BW_SUBC; k++) {
    // Skip DMRS
    if (k % 4 == v) {
      continue;
    }

    ssb_grid[1 * ISRRAN_SSB_BW_SUBC + k] = symbols[count++];
  }

  // Symbol 2
  for (uint32_t k = 0; k < 48; k++) {
    // Skip DMRS
    if (k % 4 == v) {
      continue;
    }

    ssb_grid[2 * ISRRAN_SSB_BW_SUBC + k] = symbols[count++];
  }
  for (uint32_t k = 192; k < ISRRAN_SSB_BW_SUBC; k++) {
    // Skip DMRS
    if (k % 4 == v) {
      continue;
    }

    ssb_grid[2 * ISRRAN_SSB_BW_SUBC + k] = symbols[count++];
  }

  // Symbol 3
  for (uint32_t k = 0; k < ISRRAN_SSB_BW_SUBC; k++) {
    // Skip DMRS
    if (k % 4 == v) {
      continue;
    }

    ssb_grid[3 * ISRRAN_SSB_BW_SUBC + k] = symbols[count++];
  }

  //  if (isrran_verbose >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
  //    PBCH_NR_DEBUG_TX("Symbols: ");
  //    isrran_vec_fprint_c(stdout, symbols, PBCH_NR_M);
  //  }
}

static void
pbch_nr_demapping(const isrran_pbch_nr_cfg_t* cfg, const cf_t ssb_grid[ISRRAN_SSB_NOF_RE], cf_t symbols[PBCH_NR_M])
{
  uint32_t count = 0;

  // PBCH DMRS shift
  uint32_t v = cfg->N_id % 4;

  // Symbol 1
  for (uint32_t k = 0; k < ISRRAN_SSB_BW_SUBC; k++) {
    // Skip DMRS
    if (k % 4 == v) {
      continue;
    }

    symbols[count++] = ssb_grid[1 * ISRRAN_SSB_BW_SUBC + k];
  }

  // Symbol 2
  for (uint32_t k = 0; k < 48; k++) {
    // Skip DMRS
    if (k % 4 == v) {
      continue;
    }

    symbols[count++] = ssb_grid[2 * ISRRAN_SSB_BW_SUBC + k];
  }
  for (uint32_t k = 192; k < ISRRAN_SSB_BW_SUBC; k++) {
    // Skip DMRS
    if (k % 4 == v) {
      continue;
    }

    symbols[count++] = ssb_grid[2 * ISRRAN_SSB_BW_SUBC + k];
  }

  // Symbol 3
  for (uint32_t k = 0; k < ISRRAN_SSB_BW_SUBC; k++) {
    // Skip DMRS
    if (k % 4 == v) {
      continue;
    }

    symbols[count++] = ssb_grid[3 * ISRRAN_SSB_BW_SUBC + k];
  }

  //  if (isrran_verbose >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {
  //    PBCH_NR_DEBUG_RX("Symbols: ");
  //    isrran_vec_fprint_c(stdout, symbols, PBCH_NR_M);
  //  }
}

int isrran_pbch_nr_encode(isrran_pbch_nr_t*           q,
                          const isrran_pbch_nr_cfg_t* cfg,
                          const isrran_pbch_msg_nr_t* msg,
                          cf_t                        ssb_grid[ISRRAN_SSB_NOF_RE])
{
  if (q == NULL || cfg == NULL || msg == NULL || ssb_grid == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // TS 38.212 7.1 Broadcast channel
  // 7.1.1  PBCH payload generation
  uint8_t a[PBCH_NR_A];
  pbch_nr_pbch_msg_pack(cfg, msg, a);

  // 7.1.2 Scrambling
  uint8_t b[PBCH_NR_K];
  pbch_nr_scramble(cfg, a, b);

  // 7.1.3 Transport block CRC attachment
  uint32_t checksum = isrran_crc_attach(&q->crc, b, PBCH_NR_A);
  PBCH_NR_DEBUG_TX("checksum=%06x", checksum);

  // 7.1.4 Channel coding
  uint8_t d[PBCH_NR_N];
  if (pbch_nr_polar_encode(q, b, d) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // 7.1.5 Rate matching
  uint8_t f[PBCH_NR_E];
  if (pbch_nr_polar_rm_tx(q, d, f) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // TS 38.211 7.3.3 Physical broadcast channel
  // 7.3.3.1 Scrambling
  pbch_nr_scramble_tx(cfg, msg->ssb_idx, f, f);

  // 7.3.3.2 Modulation
  cf_t symbols[PBCH_NR_M];
  isrran_mod_modulate(&q->qpsk, f, symbols, PBCH_NR_E);

  // 7.3.3.3 Mapping to physical resources
  // 7.4.3.1.3 Mapping of PBCH and DM-RS within an SS/PBCH block
  pbch_nr_mapping(cfg, symbols, ssb_grid);

  return ISRRAN_SUCCESS;
}

int isrran_pbch_nr_decode(isrran_pbch_nr_t*           q,
                          const isrran_pbch_nr_cfg_t* cfg,
                          const cf_t                  ssb_grid[ISRRAN_SSB_NOF_RE],
                          const cf_t                  ce_grid[ISRRAN_SSB_NOF_RE],
                          isrran_pbch_msg_nr_t*       msg)
{
  if (q == NULL || cfg == NULL || msg == NULL || ssb_grid == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // 7.3.3.3 Mapping to physical resources
  // 7.4.3.1.3 Mapping of PBCH and DM-RS within an SS/PBCH block
  isrran_simd_aligned cf_t symbols[PBCH_NR_M];
  pbch_nr_demapping(cfg, ssb_grid, symbols);

  isrran_simd_aligned cf_t ce[PBCH_NR_M];
  pbch_nr_demapping(cfg, ce_grid, ce);

  // Channel equalizer
  if (isrran_predecoding_single(symbols, ce, symbols, NULL, PBCH_NR_M, 1.0f, 0.0f) < ISRRAN_SUCCESS) {
    ERROR("Error in predecoder");
    return ISRRAN_ERROR;
  }

  // Avoid NAN getting into the demodulator
  for (uint32_t i = 0; i < PBCH_NR_M; i++) {
    if (isnan(__real__ symbols[i]) || isnan(__imag__ symbols[i])) {
      symbols[i] = 0.0f;
    }
  }

  // 7.3.3.2 Modulation
  int8_t llr[PBCH_NR_E];
  isrran_demod_soft_demodulate_b(ISRRAN_MOD_QPSK, symbols, llr, PBCH_NR_M);

  // If all LLR are zero, no message could be received
  if (isrran_vec_avg_power_bf(llr, PBCH_NR_E) == 0) {
    ISRRAN_MEM_ZERO(msg, isrran_pbch_msg_nr_t, 1);
    return ISRRAN_SUCCESS;
  }

  // TS 38.211 7.3.3 Physical broadcast channel
  // 7.3.3.1 Scrambling
  pbch_nr_scramble_rx(cfg, cfg->ssb_idx, llr, llr);

  // 7.1.5 Rate matching
  int8_t d[PBCH_NR_N];
  if (pbch_nr_polar_rm_rx(q, llr, d) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // TS 38.212 7.1 Broadcast channel
  // 7.1.4 Channel coding
  uint8_t b[PBCH_NR_K];
  if (pbch_nr_polar_decode(q, d, b) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // 7.1.3 Transport block CRC attachment
  msg->crc = isrran_crc_match(&q->crc, b, PBCH_NR_A);
  PBCH_NR_DEBUG_RX("crc=%s", msg->crc ? "OK" : "KO");

  // 7.1.2 Scrambling
  uint8_t a[PBCH_NR_A];
  pbch_nr_scramble(cfg, b, a);

  // 7.1.1  PBCH payload generation
  pbch_nr_pbch_msg_unpack(cfg, a, msg);

  return ISRRAN_SUCCESS;
}
