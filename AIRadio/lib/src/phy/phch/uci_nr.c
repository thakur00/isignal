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

#include "isrran/phy/phch/uci_nr.h"
#include "isrran/phy/fec/block/block.h"
#include "isrran/phy/fec/polar/polar_chanalloc.h"
#include "isrran/phy/phch/csi.h"
#include "isrran/phy/phch/uci_cfg.h"
#include "isrran/phy/utils/bit.h"
#include "isrran/phy/utils/vector.h"

#define UCI_NR_INFO_TX(...) INFO("UCI-NR Tx: " __VA_ARGS__)
#define UCI_NR_INFO_RX(...) INFO("UCI-NR Rx: " __VA_ARGS__)

#define UCI_NR_MAX_L 11U
#define UCI_NR_POLAR_MAX 2048U
#define UCI_NR_POLAR_RM_IBIL 1
#define UCI_NR_PUCCH_POLAR_N_MAX 10
#define UCI_NR_BLOCK_DEFAULT_CORR_THRESHOLD 0.5f
#define UCI_NR_ONE_BIT_CORR_THRESHOLD 0.5f

uint32_t isrran_uci_nr_crc_len(uint32_t A)
{
  return (A <= 11) ? 0 : (A < 20) ? 6 : 11;
}

static inline int uci_nr_pusch_cfg_valid(const isrran_uci_nr_pusch_cfg_t* cfg)
{
  // No data pointer
  if (cfg == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Unset configuration is unset
  if (cfg->nof_re == 0 && cfg->nof_layers == 0 && !isnormal(cfg->R)) {
    return ISRRAN_SUCCESS;
  }

  // Detect invalid number of layers
  if (cfg->nof_layers == 0) {
    ERROR("Invalid number of layers %d", cfg->nof_layers);
    return ISRRAN_ERROR;
  }

  // Detect invalid number of RE
  if (cfg->nof_re == 0) {
    ERROR("Invalid number of RE %d", cfg->nof_re);
    return ISRRAN_ERROR;
  }

  // Detect invalid Rate
  if (!isnormal(cfg->R)) {
    ERROR("Invalid R %f", cfg->R);
    return ISRRAN_ERROR;
  }

  // Otherwise it is set and valid
  return 1;
}

int isrran_uci_nr_init(isrran_uci_nr_t* q, const isrran_uci_nr_args_t* args)
{
  if (q == NULL || args == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  isrran_polar_encoder_type_t polar_encoder_type = ISRRAN_POLAR_ENCODER_PIPELINED;
  isrran_polar_decoder_type_t polar_decoder_type = ISRRAN_POLAR_DECODER_SSC_C;
#ifdef LV_HAVE_AVX2
  if (!args->disable_simd) {
    polar_encoder_type = ISRRAN_POLAR_ENCODER_AVX2;
    polar_decoder_type = ISRRAN_POLAR_DECODER_SSC_C_AVX2;
  }
#endif // LV_HAVE_AVX2

  if (isrran_polar_code_init(&q->code)) {
    ERROR("Initialising polar code");
    return ISRRAN_ERROR;
  }

  if (isrran_polar_encoder_init(&q->encoder, polar_encoder_type, NMAX_LOG) < ISRRAN_SUCCESS) {
    ERROR("Initialising polar encoder");
    return ISRRAN_ERROR;
  }

  if (isrran_polar_decoder_init(&q->decoder, polar_decoder_type, NMAX_LOG) < ISRRAN_SUCCESS) {
    ERROR("Initialising polar encoder");
    return ISRRAN_ERROR;
  }

  if (isrran_polar_rm_tx_init(&q->rm_tx) < ISRRAN_SUCCESS) {
    ERROR("Initialising polar RM");
    return ISRRAN_ERROR;
  }

  if (isrran_polar_rm_rx_init_c(&q->rm_rx) < ISRRAN_SUCCESS) {
    ERROR("Initialising polar RM");
    return ISRRAN_ERROR;
  }

  if (isrran_crc_init(&q->crc6, ISRRAN_LTE_CRC6, 6) < ISRRAN_SUCCESS) {
    ERROR("Initialising CRC");
    return ISRRAN_ERROR;
  }

  if (isrran_crc_init(&q->crc11, ISRRAN_LTE_CRC11, 11) < ISRRAN_SUCCESS) {
    ERROR("Initialising CRC");
    return ISRRAN_ERROR;
  }

  // Allocate bit sequence with space for the CRC
  q->bit_sequence = isrran_vec_u8_malloc(ISRRAN_UCI_NR_MAX_NOF_BITS);
  if (q->bit_sequence == NULL) {
    ERROR("Error malloc");
    return ISRRAN_ERROR;
  }

  // Allocate c with space for a and the CRC
  q->c = isrran_vec_u8_malloc(ISRRAN_UCI_NR_MAX_NOF_BITS + UCI_NR_MAX_L);
  if (q->c == NULL) {
    ERROR("Error malloc");
    return ISRRAN_ERROR;
  }

  q->allocated = isrran_vec_u8_malloc(UCI_NR_POLAR_MAX);
  if (q->allocated == NULL) {
    ERROR("Error malloc");
    return ISRRAN_ERROR;
  }

  q->d = isrran_vec_u8_malloc(UCI_NR_POLAR_MAX);
  if (q->d == NULL) {
    ERROR("Error malloc");
    return ISRRAN_ERROR;
  }

  if (isnormal(args->block_code_threshold)) {
    q->block_code_threshold = args->block_code_threshold;
  } else {
    q->block_code_threshold = UCI_NR_BLOCK_DEFAULT_CORR_THRESHOLD;
  }
  if (isnormal(args->one_bit_threshold)) {
    q->one_bit_threshold = args->one_bit_threshold;
  } else {
    q->one_bit_threshold = UCI_NR_ONE_BIT_CORR_THRESHOLD;
  }

  return ISRRAN_SUCCESS;
}

void isrran_uci_nr_free(isrran_uci_nr_t* q)
{
  if (q == NULL) {
    return;
  }

  isrran_polar_code_free(&q->code);
  isrran_polar_encoder_free(&q->encoder);
  isrran_polar_decoder_free(&q->decoder);
  isrran_polar_rm_tx_free(&q->rm_tx);
  isrran_polar_rm_rx_free_c(&q->rm_rx);

  if (q->bit_sequence != NULL) {
    free(q->bit_sequence);
  }
  if (q->c != NULL) {
    free(q->c);
  }
  if (q->allocated != NULL) {
    free(q->allocated);
  }
  if (q->d != NULL) {
    free(q->d);
  }

  ISRRAN_MEM_ZERO(q, isrran_uci_nr_t, 1);
}

static int uci_nr_pack_ack_sr(const isrran_uci_cfg_nr_t* cfg, const isrran_uci_value_nr_t* value, uint8_t* sequence)
{
  int A = 0;

  // Append ACK bits
  isrran_vec_u8_copy(&sequence[A], value->ack, cfg->ack.count);
  A += cfg->ack.count;

  // Append SR bits
  uint8_t* bits = &sequence[A];
  isrran_bit_unpack(value->sr, &bits, cfg->o_sr);
  A += cfg->o_sr;

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_TX("Packed UCI bits: ");
    isrran_vec_fprint_byte(stdout, sequence, A);
  }

  return A;
}

static int uci_nr_unpack_ack_sr(const isrran_uci_cfg_nr_t* cfg, uint8_t* sequence, isrran_uci_value_nr_t* value)
{
  int A = 0;

  // Append ACK bits
  isrran_vec_u8_copy(value->ack, &sequence[A], cfg->ack.count);
  A += cfg->ack.count;

  // Append SR bits
  uint8_t* bits = &sequence[A];
  value->sr     = isrran_bit_pack(&bits, cfg->o_sr);
  A += cfg->o_sr;

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_RX("Unpacked UCI bits: ");
    isrran_vec_fprint_byte(stdout, sequence, A);
  }

  return A;
}

static int uci_nr_pack_ack_sr_csi(const isrran_uci_cfg_nr_t* cfg, const isrran_uci_value_nr_t* value, uint8_t* sequence)
{
  int A = 0;

  // Append ACK bits
  isrran_vec_u8_copy(&sequence[A], value->ack, cfg->ack.count);
  A += cfg->ack.count;

  // Append SR bits
  uint8_t* bits = &sequence[A];
  isrran_bit_unpack(value->sr, &bits, cfg->o_sr);
  A += cfg->o_sr;

  // Append CSI bits
  int n = isrran_csi_part1_pack(cfg->csi, value->csi, cfg->nof_csi, bits, ISRRAN_UCI_NR_MAX_NOF_BITS - A);
  if (n < ISRRAN_SUCCESS) {
    ERROR("Packing CSI part 1");
    return ISRRAN_ERROR;
  }
  A += n;

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_TX("Packed UCI bits: ");
    isrran_vec_fprint_byte(stdout, sequence, A);
  }

  return A;
}

static int uci_nr_unpack_ack_sr_csi(const isrran_uci_cfg_nr_t* cfg, uint8_t* sequence, isrran_uci_value_nr_t* value)
{
  int A = 0;

  // Append ACK bits
  isrran_vec_u8_copy(value->ack, &sequence[A], cfg->ack.count);
  A += cfg->ack.count;

  // Append SR bits
  uint8_t* bits = &sequence[A];
  value->sr     = isrran_bit_pack(&bits, cfg->o_sr);
  A += cfg->o_sr;

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_RX("Unpacked UCI bits: ");
    isrran_vec_fprint_byte(stdout, sequence, A);
  }

  // Append CSI bits
  int n = isrran_csi_part1_unpack(cfg->csi, cfg->nof_csi, bits, ISRRAN_UCI_NR_MAX_NOF_BITS - A, value->csi);
  if (n < ISRRAN_SUCCESS) {
    ERROR("Packing CSI part 1");
    return ISRRAN_ERROR;
  }

  return A;
}

static int uci_nr_A(const isrran_uci_cfg_nr_t* cfg)
{
  int o_csi = isrran_csi_part1_nof_bits(cfg->csi, cfg->nof_csi);

  // 6.3.1.1.1 HARQ-ACK/SR only UCI bit sequence generation
  if (o_csi == 0) {
    return cfg->ack.count + cfg->o_sr;
  }

  // 6.3.1.1.2 CSI only
  if (cfg->ack.count == 0 && cfg->o_sr == 0) {
    return o_csi;
  }

  // 6.3.1.1.3 HARQ-ACK/SR and CSI
  return cfg->ack.count + cfg->o_sr + o_csi;
}

static int uci_nr_pack_pucch(const isrran_uci_cfg_nr_t* cfg, const isrran_uci_value_nr_t* value, uint8_t* sequence)
{
  int o_csi = isrran_csi_part1_nof_bits(cfg->csi, cfg->nof_csi);

  // 6.3.1.1.1 HARQ-ACK/SR only UCI bit sequence generation
  if (o_csi == 0) {
    return uci_nr_pack_ack_sr(cfg, value, sequence);
  }

  // 6.3.1.1.2 CSI only
  if (cfg->ack.count == 0 && cfg->o_sr == 0) {
    return isrran_csi_part1_pack(cfg->csi, value->csi, cfg->nof_csi, sequence, ISRRAN_UCI_NR_MAX_NOF_BITS);
  }

  // 6.3.1.1.3 HARQ-ACK/SR and CSI
  return uci_nr_pack_ack_sr_csi(cfg, value, sequence);
}

static int uci_nr_unpack_pucch(const isrran_uci_cfg_nr_t* cfg, uint8_t* sequence, isrran_uci_value_nr_t* value)
{
  int o_csi = isrran_csi_part1_nof_bits(cfg->csi, cfg->nof_csi);

  // 6.3.1.1.1 HARQ-ACK/SR only UCI bit sequence generation
  if (o_csi == 0) {
    return uci_nr_unpack_ack_sr(cfg, sequence, value);
  }

  // 6.3.1.1.2 CSI only
  if (cfg->ack.count == 0 && cfg->o_sr == 0) {
    return isrran_csi_part1_unpack(cfg->csi, cfg->nof_csi, sequence, ISRRAN_UCI_NR_MAX_NOF_BITS, value->csi);
  }

  // 6.3.1.1.3 HARQ-ACK/SR and CSI
  return uci_nr_unpack_ack_sr_csi(cfg, sequence, value);
}

static int uci_nr_encode_1bit(isrran_uci_nr_t* q, const isrran_uci_cfg_nr_t* cfg, uint8_t* o, uint32_t E)
{
  uint32_t              i  = 0;
  isrran_uci_bit_type_t c0 = (q->bit_sequence[0] == 0) ? UCI_BIT_0 : UCI_BIT_1;

  switch (cfg->pusch.modulation) {
    case ISRRAN_MOD_BPSK:
      while (i < E) {
        o[i++] = c0;
      }
      break;
    case ISRRAN_MOD_QPSK:
      while (i < E) {
        o[i++] = c0;
        o[i++] = (uint8_t)UCI_BIT_REPETITION;
      }
      break;
    case ISRRAN_MOD_16QAM:
      while (i < E) {
        o[i++] = c0;
        o[i++] = (uint8_t)UCI_BIT_REPETITION;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
      }
      break;
    case ISRRAN_MOD_64QAM:
      while (i < E) {
        while (i < E) {
          o[i++] = c0;
          o[i++] = (uint8_t)UCI_BIT_REPETITION;
          o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
          o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
          o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
          o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        }
      }
      break;
    case ISRRAN_MOD_256QAM:
      while (i < E) {
        o[i++] = c0;
        o[i++] = (uint8_t)UCI_BIT_REPETITION;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
      }
      break;
    case ISRRAN_MOD_NITEMS:
    default:
      ERROR("Invalid modulation");
      return ISRRAN_ERROR;
  }

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_TX("One bit encoded NR-UCI; o=");
    isrran_vec_fprint_b(stdout, o, E);
  }

  return E;
}

static int uci_nr_decode_1_bit(isrran_uci_nr_t*           q,
                               const isrran_uci_cfg_nr_t* cfg,
                               uint32_t                   A,
                               const int8_t*              llr,
                               uint32_t                   E,
                               bool*                      decoded_ok)
{
  uint32_t Qm = isrran_mod_bits_x_symbol(cfg->pusch.modulation);
  if (Qm == 0) {
    ERROR("Invalid modulation (%s)", isrran_mod_string(cfg->pusch.modulation));
    return ISRRAN_ERROR;
  }

  // Correlate LLR
  float    corr  = 0.0f;
  float    pwr   = 0.0f;
  uint32_t count = 0;
  for (uint32_t i = 0; i < E; i += Qm) {
    float t = (float)llr[i];
    corr += t;
    pwr += t * t;
    count++;
  }

  // Normalise correlation
  float norm_corr = fabsf(corr) / sqrtf(pwr * count);

  // Take decoded decision with threshold
  *decoded_ok = (norm_corr > q->one_bit_threshold);

  // Save decoded bit
  q->bit_sequence[0] = (corr < 0) ? 0 : 1;

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_RX("One bit decoding NR-UCI llr=");
    isrran_vec_fprint_bs(stdout, llr, E);
    UCI_NR_INFO_RX("One bit decoding NR-UCI A=%d; E=%d; pwr=%f; corr=%f; norm=%f; thr=%f; %s",
                   A,
                   E,
                   pwr,
                   corr,
                   norm_corr,
                   q->block_code_threshold,
                   *decoded_ok ? "OK" : "KO");
  }

  return ISRRAN_SUCCESS;
}

static int uci_nr_encode_2bit(isrran_uci_nr_t* q, const isrran_uci_cfg_nr_t* cfg, uint8_t* o, uint32_t E)
{
  uint32_t i  = 0;
  uint8_t  c0 = (uint8_t)((q->bit_sequence[0] == 0) ? UCI_BIT_0 : UCI_BIT_1);
  uint8_t  c1 = (uint8_t)((q->bit_sequence[1] == 0) ? UCI_BIT_0 : UCI_BIT_1);
  uint8_t  c2 = (uint8_t)(((q->bit_sequence[0] ^ q->bit_sequence[1]) == 0) ? UCI_BIT_0 : UCI_BIT_1);

  switch (cfg->pusch.modulation) {
    case ISRRAN_MOD_BPSK:
    case ISRRAN_MOD_QPSK:
      while (i < E) {
        o[i++] = c0;
        o[i++] = c1;
        o[i++] = c2;
      }
      break;
    case ISRRAN_MOD_16QAM:
      while (i < E) {
        o[i++] = c0;
        o[i++] = c1;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = c2;
        o[i++] = c0;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = c1;
        o[i++] = c2;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
      }
      break;
    case ISRRAN_MOD_64QAM:
      while (i < E) {
        o[i++] = c0;
        o[i++] = c1;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = c2;
        o[i++] = c0;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = c1;
        o[i++] = c2;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
      }
      break;
    case ISRRAN_MOD_256QAM:

      while (i < E) {
        o[i++] = c0;
        o[i++] = c1;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = c2;
        o[i++] = c0;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = c1;
        o[i++] = c2;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
        o[i++] = (uint8_t)UCI_BIT_PLACEHOLDER;
      }
      break;
    case ISRRAN_MOD_NITEMS:
    default:
      ERROR("Invalid modulation");
      return ISRRAN_ERROR;
  }

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_TX("Two bit encoded NR-UCI; E=%d; o=", E);
    isrran_vec_fprint_b(stdout, o, E);
  }

  return E;
}

static int uci_nr_decode_2_bit(isrran_uci_nr_t*           q,
                               const isrran_uci_cfg_nr_t* cfg,
                               uint32_t                   A,
                               const int8_t*              llr,
                               uint32_t                   E,
                               bool*                      decoded_ok)
{
  uint32_t Qm = isrran_mod_bits_x_symbol(cfg->pusch.modulation);
  if (Qm == 0) {
    ERROR("Invalid modulation (%s)", isrran_mod_string(cfg->pusch.modulation));
    return ISRRAN_ERROR;
  }

  // Correlate LLR
  float corr[3] = {};
  if (Qm == 1) {
    for (uint32_t i = 0; i < E / Qm; i++) {
      corr[i % 3] = llr[i];
    }
  } else {
    for (uint32_t i = 0, j = 0; i < E; i += Qm) {
      corr[(j++) % 3] = llr[i + 0];
      corr[(j++) % 3] = llr[i + 1];
    }
  }

  // Take decoded decision
  bool c0 = corr[0] > 0.0f;
  bool c1 = corr[1] > 0.0f;
  bool c2 = corr[2] > 0.0f;

  // Check redundancy bit
  *decoded_ok = (c2 == (c0 ^ c1));

  // Save decoded bits
  q->bit_sequence[0] = c0 ? 1 : 0;
  q->bit_sequence[1] = c1 ? 1 : 0;

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_RX("Two bit decoding NR-UCI llr=");
    isrran_vec_fprint_bs(stdout, llr, E);
    UCI_NR_INFO_RX("Two bit decoding NR-UCI A=%d; E=%d; Qm=%d; c0=%d; c1=%d; c2=%d %s",
                   A,
                   E,
                   Qm,
                   c0,
                   c1,
                   c2,
                   *decoded_ok ? "OK" : "KO");
  }

  return ISRRAN_SUCCESS;
}

static int
uci_nr_encode_3_11_bit(isrran_uci_nr_t* q, const isrran_uci_cfg_nr_t* cfg, uint32_t A, uint8_t* o, uint32_t E)
{
  isrran_block_encode(q->bit_sequence, A, o, E);

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_TX("Block encoded UCI bits; o=");
    isrran_vec_fprint_b(stdout, o, E);
  }

  return E;
}

static int uci_nr_decode_3_11_bit(isrran_uci_nr_t*           q,
                                  const isrran_uci_cfg_nr_t* cfg,
                                  uint32_t                   A,
                                  const int8_t*              llr,
                                  uint32_t                   E,
                                  bool*                      decoded_ok)
{
  // Check E for avoiding zero division
  if (E < 1) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (A == 11 && E <= 16) {
    ERROR("NR-UCI Impossible to decode A=%d; E=%d", A, E);
    return ISRRAN_ERROR;
  }

  // Compute average LLR power
  float pwr = isrran_vec_avg_power_bf(llr, E);

  // If the power measurement is invalid (zero, NAN, INF) then consider it cannot be decoded
  if (!isnormal(pwr)) {
    *decoded_ok = false;
    return ISRRAN_SUCCESS;
  }

  // Decode
  float corr = (float)isrran_block_decode_i8(llr, E, q->bit_sequence, A);

  // Normalise correlation
  float norm_corr = corr / (sqrtf(pwr) * E);

  // Take decoded decision with threshold
  *decoded_ok = (corr > q->block_code_threshold);

  if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
    UCI_NR_INFO_RX("Block decoding NR-UCI llr=");
    isrran_vec_fprint_bs(stdout, llr, E);
    UCI_NR_INFO_RX("Block decoding NR-UCI A=%d; E=%d; pwr=%f; corr=%f; norm=%f; thr=%f; %s",
                   A,
                   E,
                   pwr,
                   corr,
                   norm_corr,
                   q->block_code_threshold,
                   *decoded_ok ? "OK" : "KO");
  }

  return ISRRAN_SUCCESS;
}

static int
uci_nr_encode_11_1706_bit(isrran_uci_nr_t* q, const isrran_uci_cfg_nr_t* cfg, uint32_t A, uint8_t* o, uint32_t E_uci)
{
  // If ( A ≥ 360 and E ≥ 1088 ) or if A ≥ 1013 , I seg = 1 ; otherwise I seg = 0
  uint32_t I_seg = 0;
  if ((A >= 360 && E_uci >= 1088) || A >= 1013) {
    I_seg = 1;
  }

  // Select CRC
  uint32_t      L   = isrran_uci_nr_crc_len(A);
  isrran_crc_t* crc = (L == 6) ? &q->crc6 : &q->crc11;

  // Segmentation
  uint32_t C = 1;
  if (I_seg == 1) {
    C = 2;
  }
  uint32_t A_prime = ISRRAN_CEIL(A, C) * C;

  // Get polar code
  uint32_t K_r = A_prime / C + L;
  uint32_t E_r = E_uci / C;
  if (isrran_polar_code_get(&q->code, K_r, E_r, UCI_NR_PUCCH_POLAR_N_MAX) < ISRRAN_SUCCESS) {
    ERROR("Error computing Polar code");
    return ISRRAN_ERROR;
  }

  // Write codeword
  for (uint32_t r = 0, s = 0; r < C; r++) {
    uint32_t k = 0;

    // Prefix (A_prime - A) zeros for the first CB only
    if (r == 0) {
      for (uint32_t i = 0; i < (A_prime - A); i++) {
        q->c[k++] = 0;
      }
    }

    // Load codeword bits
    while (k < A_prime / C) {
      q->c[k++] = q->bit_sequence[s++];
    }

    // Attach CRC
    isrran_crc_attach(crc, q->c, A_prime / C);
    UCI_NR_INFO_TX("Attaching %d/%d CRC%d=%" PRIx64, r, C, L, isrran_crc_checksum_get(crc));

    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
      UCI_NR_INFO_TX("Polar cb %d/%d c=", r, C);
      isrran_vec_fprint_byte(stdout, q->c, K_r);
    }

    // Allocate channel
    isrran_polar_chanalloc_tx(q->c, q->allocated, q->code.N, q->code.K, q->code.nPC, q->code.K_set, q->code.PC_set);

    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
      UCI_NR_INFO_TX("Polar alloc %d/%d ", r, C);
      isrran_vec_fprint_byte(stdout, q->allocated, q->code.N);
    }

    // Encode bits
    if (isrran_polar_encoder_encode(&q->encoder, q->allocated, q->d, q->code.n) < ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }

    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
      UCI_NR_INFO_TX("Polar encoded %d/%d ", r, C);
      isrran_vec_fprint_byte(stdout, q->d, q->code.N);
    }

    // Rate matching
    isrran_polar_rm_tx(&q->rm_tx, q->d, &o[E_r * r], q->code.n, E_r, K_r, UCI_NR_POLAR_RM_IBIL);

    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
      UCI_NR_INFO_TX("Polar RM cw %d/%d ", r, C);
      isrran_vec_fprint_byte(stdout, &o[E_r * r], E_r);
    }
  }

  return E_uci;
}

static int uci_nr_decode_11_1706_bit(isrran_uci_nr_t*           q,
                                     const isrran_uci_cfg_nr_t* cfg,
                                     uint32_t                   A,
                                     int8_t*                    llr,
                                     uint32_t                   E_uci,
                                     bool*                      decoded_ok)
{
  *decoded_ok = true;

  // If ( A ≥ 360 and E ≥ 1088 ) or if A ≥ 1013 , I seg = 1 ; otherwise I seg = 0
  uint32_t I_seg = 0;
  if ((A >= 360 && E_uci >= 1088) || A >= 1013) {
    I_seg = 1;
  }

  // Select CRC
  uint32_t      L   = isrran_uci_nr_crc_len(A);
  isrran_crc_t* crc = (L == 6) ? &q->crc6 : &q->crc11;

  // Segmentation
  uint32_t C = 1;
  if (I_seg == 1) {
    C = 2;
  }
  uint32_t A_prime = ISRRAN_CEIL(A, C) * C;

  // Get polar code
  uint32_t K_r = A_prime / C + L;
  uint32_t E_r = E_uci / C;
  if (isrran_polar_code_get(&q->code, K_r, E_r, UCI_NR_PUCCH_POLAR_N_MAX) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Negate all LLR
  for (uint32_t i = 0; i < E_r; i++) {
    llr[i] *= -1;
  }

  // Write codeword
  for (uint32_t r = 0, s = 0; r < C; r++) {
    uint32_t k = 0;

    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
      UCI_NR_INFO_RX("Polar LLR %d/%d ", r, C);
      isrran_vec_fprint_bs(stdout, &llr[E_r * r], q->code.N);
    }

    // Undo rate matching
    int8_t* d = (int8_t*)q->d;
    isrran_polar_rm_rx_c(&q->rm_rx, &llr[E_r * r], d, E_r, q->code.n, K_r, UCI_NR_POLAR_RM_IBIL);

    // Decode bits
    if (isrran_polar_decoder_decode_c(&q->decoder, d, q->allocated, q->code.n, q->code.F_set, q->code.F_set_size) <
        ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }

    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
      UCI_NR_INFO_RX("Polar alloc %d/%d ", r, C);
      isrran_vec_fprint_byte(stdout, q->allocated, q->code.N);
    }

    // Undo channel allocation
    isrran_polar_chanalloc_rx(q->allocated, q->c, q->code.K, q->code.nPC, q->code.K_set, q->code.PC_set);

    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {
      UCI_NR_INFO_RX("Polar cb %d/%d c=", r, C);
      isrran_vec_fprint_byte(stdout, q->c, K_r);
    }

    // Calculate checksum
    uint8_t* ptr       = &q->c[A_prime / C];
    uint32_t checksum1 = isrran_crc_checksum(crc, q->c, A_prime / C);
    uint32_t checksum2 = isrran_bit_pack(&ptr, L);
    (*decoded_ok)      = ((*decoded_ok) && (checksum1 == checksum2));
    UCI_NR_INFO_RX("Checking %d/%d CRC%d={%02x,%02x}", r, C, L, checksum1, checksum2);

    // Prefix (A_prime - A) zeros for the first CB only
    if (r == 0) {
      for (uint32_t i = 0; i < (A_prime - A); i++) {
        k++;
      }
    }

    // Load codeword bits
    while (k < A_prime / C) {
      q->bit_sequence[s++] = q->c[k++];
    }
  }

  return ISRRAN_SUCCESS;
}

static int uci_nr_encode(isrran_uci_nr_t* q, const isrran_uci_cfg_nr_t* uci_cfg, uint32_t A, uint8_t* o, uint32_t E_uci)
{
  // 5.3.3.1 Encoding of 1-bit information
  if (A == 1) {
    return uci_nr_encode_1bit(q, uci_cfg, o, E_uci);
  }

  // 5.3.3.2 Encoding of 2-bit information
  if (A == 2) {
    return uci_nr_encode_2bit(q, uci_cfg, o, E_uci);
  }

  // 5.3.3.3 Encoding of other small block lengths
  if (A <= ISRRAN_FEC_BLOCK_MAX_NOF_BITS) {
    return uci_nr_encode_3_11_bit(q, uci_cfg, A, o, E_uci);
  }

  // Encoding of other sizes up to 1906
  if (A < ISRRAN_UCI_NR_MAX_NOF_BITS) {
    return uci_nr_encode_11_1706_bit(q, uci_cfg, A, o, E_uci);
  }

  return ISRRAN_ERROR;
}

static int uci_nr_decode(isrran_uci_nr_t*           q,
                         const isrran_uci_cfg_nr_t* uci_cfg,
                         int8_t*                    llr,
                         uint32_t                   A,
                         uint32_t                   E_uci,
                         bool*                      valid)
{
  if (q == NULL || uci_cfg == NULL || valid == NULL || llr == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Decode LLR
  if (A == 1) {
    if (uci_nr_decode_1_bit(q, uci_cfg, A, llr, E_uci, valid) < ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }
  } else if (A == 2) {
    if (uci_nr_decode_2_bit(q, uci_cfg, A, llr, E_uci, valid) < ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }
  } else if (A <= 11) {
    if (uci_nr_decode_3_11_bit(q, uci_cfg, A, llr, E_uci, valid) < ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }
  } else if (A < ISRRAN_UCI_NR_MAX_NOF_BITS) {
    if (uci_nr_decode_11_1706_bit(q, uci_cfg, A, llr, E_uci, valid) < ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }
  } else {
    ERROR("Invalid number of bits (A=%d)", A);
  }

  return ISRRAN_SUCCESS;
}

int isrran_uci_nr_pucch_format_2_3_4_E(const isrran_pucch_nr_resource_t* resource)
{
  if (resource == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  switch (resource->format) {
    case ISRRAN_PUCCH_NR_FORMAT_2:
      return (int)(16 * resource->nof_symbols * resource->nof_prb);
    case ISRRAN_PUCCH_NR_FORMAT_3:
      if (!resource->enable_pi_bpsk) {
        return (int)(24 * resource->nof_symbols * resource->nof_prb);
      }
      return (int)(12 * resource->nof_symbols * resource->nof_prb);
    case ISRRAN_PUCCH_NR_FORMAT_4:
      if (resource->occ_lenth != 1 && resource->occ_lenth != 2) {
        ERROR("Invalid spreading factor (%d)", resource->occ_lenth);
        return ISRRAN_ERROR;
      }
      if (!resource->enable_pi_bpsk) {
        return (int)(24 * resource->nof_symbols / resource->occ_lenth);
      }
      return (int)(12 * resource->nof_symbols / resource->occ_lenth);
    default:
      ERROR("Invalid case");
  }
  return ISRRAN_ERROR;
}

// Implements TS 38.212 Table 6.3.1.4.1-1: Rate matching output sequence length E UCI
static int
uci_nr_pucch_E_uci(const isrran_pucch_nr_resource_t* pucch_cfg, const isrran_uci_cfg_nr_t* uci_cfg, uint32_t E_tot)
{
  //  if (uci_cfg->o_csi1 != 0 && uci_cfg->o_csi2) {
  //    ERROR("Simultaneous CSI part 1 and CSI part 2 is not implemented");
  //    return ISRRAN_ERROR;
  //  }

  return E_tot;
}

int isrran_uci_nr_encode_pucch(isrran_uci_nr_t*                  q,
                               const isrran_pucch_nr_resource_t* pucch_resource_cfg,
                               const isrran_uci_cfg_nr_t*        uci_cfg,
                               const isrran_uci_value_nr_t*      value,
                               uint8_t*                          o)
{
  int E_tot = isrran_uci_nr_pucch_format_2_3_4_E(pucch_resource_cfg);
  if (E_tot < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of bits");
    return ISRRAN_ERROR;
  }

  int E_uci = uci_nr_pucch_E_uci(pucch_resource_cfg, uci_cfg, E_tot);
  if (E_uci < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of bits");
    return ISRRAN_ERROR;
  }

  // 6.3.1.1 UCI bit sequence generation
  int A = uci_nr_pack_pucch(uci_cfg, value, q->bit_sequence);
  if (A < ISRRAN_SUCCESS) {
    ERROR("Generating bit sequence");
    return ISRRAN_ERROR;
  }

  return uci_nr_encode(q, uci_cfg, A, o, E_uci);
}

int isrran_uci_nr_decode_pucch(isrran_uci_nr_t*                  q,
                               const isrran_pucch_nr_resource_t* pucch_resource_cfg,
                               const isrran_uci_cfg_nr_t*        uci_cfg,
                               int8_t*                           llr,
                               isrran_uci_value_nr_t*            value)
{
  int E_tot = isrran_uci_nr_pucch_format_2_3_4_E(pucch_resource_cfg);
  if (E_tot < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  int E_uci = uci_nr_pucch_E_uci(pucch_resource_cfg, uci_cfg, E_tot);
  if (E_uci < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of encoded PUCCH UCI bits");
    return ISRRAN_ERROR;
  }

  // 6.3.1.1 UCI bit sequence generation
  int A = uci_nr_A(uci_cfg);
  if (A < ISRRAN_SUCCESS) {
    ERROR("Error getting number of bits");
    return ISRRAN_ERROR;
  }

  if (uci_nr_decode(q, uci_cfg, llr, A, E_uci, &value->valid) < ISRRAN_SUCCESS) {
    ERROR("Error decoding UCI bits");
    return ISRRAN_ERROR;
  }

  // Unpack bits
  if (uci_nr_unpack_pucch(uci_cfg, q->bit_sequence, value) < ISRRAN_SUCCESS) {
    ERROR("Error unpacking PUCCH UCI bits");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

uint32_t isrran_uci_nr_total_bits(const isrran_uci_cfg_nr_t* uci_cfg)
{
  if (uci_cfg == NULL) {
    return 0;
  }

  uint32_t o_csi = isrran_csi_part1_nof_bits(uci_cfg->csi, uci_cfg->nof_csi);

  // According to 38.213 9.2.4 UE procedure for reporting SR
  // The UE transmits a PUCCH in the PUCCH resource for the corresponding SR configuration only when the UE transmits a
  // positive SR
  if (uci_cfg->ack.count == 0 && o_csi == 0 && !uci_cfg->sr_positive_present) {
    return 0;
  }

  return uci_cfg->ack.count + uci_cfg->o_sr + o_csi;
}

uint32_t isrran_uci_nr_info(const isrran_uci_data_nr_t* uci_data, char* str, uint32_t str_len)
{
  uint32_t len = 0;

  if (uci_data->cfg.ack.count > 0) {
    char str_ack[10];
    isrran_vec_sprint_bin(str_ack, (uint32_t)sizeof(str_ack), uci_data->value.ack, uci_data->cfg.ack.count);
    len = isrran_print_check(str, str_len, len, "ack=%s ", str_ack);
  }

  if (uci_data->cfg.nof_csi > 0) {
    len += isrran_csi_str(uci_data->cfg.csi, uci_data->value.csi, uci_data->cfg.nof_csi, &str[len], str_len - len);
  }

  if (uci_data->cfg.o_sr > 0) {
    len = isrran_print_check(str, str_len, len, "sr=%d ", uci_data->value.sr);
  }

  return len;
}

static int uci_nr_pusch_Q_prime_ack(const isrran_uci_nr_pusch_cfg_t* cfg, uint32_t O_ack)
{
  uint32_t L_ack = isrran_uci_nr_crc_len(O_ack);              // Number of CRC bits
  uint32_t Qm    = isrran_mod_bits_x_symbol(cfg->modulation); // modulation order of the PUSCH

  uint32_t M_uci_sum    = 0;
  uint32_t M_uci_l0_sum = 0;
  for (uint32_t l = 0; l < ISRRAN_NSYMB_PER_SLOT_NR; l++) {
    M_uci_sum += cfg->M_uci_sc[l];
    if (l >= cfg->l0) {
      M_uci_l0_sum += cfg->M_uci_sc[l];
    }
  }

  if (!isnormal(cfg->R)) {
    ERROR("Invalid Rate (%f)", cfg->R);
    return ISRRAN_ERROR;
  }

  if (cfg->K_sum == 0) {
    return (int)ISRRAN_MIN(ceilf(((O_ack + L_ack) * cfg->beta_harq_ack_offset) / (Qm * cfg->R)),
                           cfg->alpha * M_uci_l0_sum);
  }
  return (int)ISRRAN_MIN(ceilf(((O_ack + L_ack) * cfg->beta_harq_ack_offset * M_uci_sum) / cfg->K_sum),
                         cfg->alpha * M_uci_l0_sum);
}

int isrran_uci_nr_pusch_ack_nof_bits(const isrran_uci_nr_pusch_cfg_t* cfg, uint32_t O_ack)
{
  // Validate configuration
  int err = uci_nr_pusch_cfg_valid(cfg);
  if (err < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Configuration is unset
  if (err == 0) {
    return 0;
  }

  int Q_ack_prime = uci_nr_pusch_Q_prime_ack(cfg, O_ack);
  if (Q_ack_prime < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of RE");
    return Q_ack_prime;
  }

  return (int)(Q_ack_prime * cfg->nof_layers * isrran_mod_bits_x_symbol(cfg->modulation));
}

int isrran_uci_nr_encode_pusch_ack(isrran_uci_nr_t*             q,
                                   const isrran_uci_cfg_nr_t*   cfg,
                                   const isrran_uci_value_nr_t* value,
                                   uint8_t*                     o)
{
  // Check inputs
  if (q == NULL || cfg == NULL || value == NULL || o == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  int A = cfg->ack.count;

  // 6.3.2.1 UCI bit sequence generation
  // 6.3.2.1.1 HARQ-ACK
  bool has_csi_part2 = isrran_csi_has_part2(cfg->csi, cfg->nof_csi);
  if (cfg->pusch.K_sum == 0 && cfg->nof_csi > 1 && !has_csi_part2 && A < 2) {
    q->bit_sequence[0] = (A == 0) ? 0 : value->ack[0];
    q->bit_sequence[1] = 0;
    A                  = 2;
  } else if (A == 0) {
    UCI_NR_INFO_TX("No HARQ-ACK to mux");
    return ISRRAN_SUCCESS;
  } else {
    isrran_vec_u8_copy(q->bit_sequence, value->ack, cfg->ack.count);
  }

  // Compute total of encoded bits according to 6.3.2.4 Rate matching
  int E_uci = isrran_uci_nr_pusch_ack_nof_bits(&cfg->pusch, A);
  if (E_uci < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of encoded bits");
    return ISRRAN_ERROR;
  }

  return uci_nr_encode(q, cfg, A, o, E_uci);
}

int isrran_uci_nr_decode_pusch_ack(isrran_uci_nr_t*           q,
                                   const isrran_uci_cfg_nr_t* cfg,
                                   int8_t*                    llr,
                                   isrran_uci_value_nr_t*     value)
{
  // Check inputs
  if (q == NULL || cfg == NULL || llr == NULL || value == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  int A = cfg->ack.count;

  // 6.3.2.1 UCI bit sequence generation
  // 6.3.2.1.1 HARQ-ACK
  bool has_csi_part2 = isrran_csi_has_part2(cfg->csi, cfg->nof_csi);
  if (cfg->pusch.K_sum == 0 && cfg->nof_csi > 1 && !has_csi_part2 && cfg->ack.count < 2) {
    A = 2;
  }

  // Compute total of encoded bits according to 6.3.2.4 Rate matching
  int E_uci = isrran_uci_nr_pusch_ack_nof_bits(&cfg->pusch, A);
  if (E_uci < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of encoded bits");
    return ISRRAN_ERROR;
  }

  // Decode
  if (uci_nr_decode(q, cfg, llr, A, E_uci, &value->valid) < ISRRAN_SUCCESS) {
    ERROR("Error decoding UCI");
    return ISRRAN_ERROR;
  }

  // Unpack
  isrran_vec_u8_copy(value->ack, q->bit_sequence, A);

  return ISRRAN_SUCCESS;
}

static int uci_nr_pusch_Q_prime_csi1(const isrran_uci_nr_pusch_cfg_t* cfg, uint32_t O_csi1, uint32_t O_ack)
{
  if (cfg == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  uint32_t L_ack = isrran_uci_nr_crc_len(O_csi1);             // Number of CRC bits
  uint32_t Qm    = isrran_mod_bits_x_symbol(cfg->modulation); // modulation order of the PUSCH

  int Q_prime_ack = uci_nr_pusch_Q_prime_ack(cfg, ISRRAN_MAX(2, O_ack));
  if (Q_prime_ack < ISRRAN_ERROR) {
    ERROR("Calculating Q_prime_ack");
    return ISRRAN_ERROR;
  }

  uint32_t M_uci_sum = 0;
  for (uint32_t l = 0; l < ISRRAN_NSYMB_PER_SLOT_NR; l++) {
    M_uci_sum += cfg->M_uci_sc[l];
  }

  if (!isnormal(cfg->R)) {
    ERROR("Invalid Rate (%f)", cfg->R);
    return ISRRAN_ERROR;
  }

  if (cfg->K_sum == 0) {
    if (cfg->csi_part2_present) {
      return (int)ISRRAN_MIN(ceilf(((O_csi1 + L_ack) * cfg->beta_csi1_offset) / (Qm * cfg->R)),
                             cfg->alpha * M_uci_sum - Q_prime_ack);
    }
    return (int)(M_uci_sum - Q_prime_ack);
  }
  return (int)ISRRAN_MIN(ceilf(((O_csi1 + L_ack) * cfg->beta_csi1_offset * M_uci_sum) / cfg->K_sum),
                         ceilf(cfg->alpha * M_uci_sum) - Q_prime_ack);
}

int isrran_uci_nr_pusch_csi1_nof_bits(const isrran_uci_cfg_nr_t* cfg)
{
  // Validate configuration
  int err = uci_nr_pusch_cfg_valid(&cfg->pusch);
  if (err < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Configuration is unset
  if (err == 0) {
    return 0;
  }

  int O_csi1 = isrran_csi_part1_nof_bits(cfg->csi, cfg->nof_csi);
  if (O_csi1 < ISRRAN_SUCCESS) {
    ERROR("Errpr calculating CSI part 1 number of bits");
    return ISRRAN_ERROR;
  }
  uint32_t O_ack = ISRRAN_MAX(2, cfg->ack.count);

  int Q_csi1_prime = uci_nr_pusch_Q_prime_csi1(&cfg->pusch, (uint32_t)O_csi1, O_ack);
  if (Q_csi1_prime < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of RE");
    return Q_csi1_prime;
  }

  return (int)(Q_csi1_prime * cfg->pusch.nof_layers * isrran_mod_bits_x_symbol(cfg->pusch.modulation));
}

int isrran_uci_nr_encode_pusch_csi1(isrran_uci_nr_t*             q,
                                    const isrran_uci_cfg_nr_t*   cfg,
                                    const isrran_uci_value_nr_t* value,
                                    uint8_t*                     o)
{
  // Check inputs
  if (q == NULL || cfg == NULL || value == NULL || o == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  int A = isrran_csi_part1_pack(cfg->csi, value->csi, cfg->nof_csi, q->bit_sequence, ISRRAN_UCI_NR_MAX_NOF_BITS);
  if (A < ISRRAN_SUCCESS) {
    ERROR("Error packing CSI part 1 report");
    return ISRRAN_ERROR;
  }

  if (A == 0) {
    UCI_NR_INFO_TX("No CSI part 1 to mux");
    return ISRRAN_SUCCESS;
  }

  // Compute total of encoded bits according to 6.3.2.4 Rate matching
  int E_uci = isrran_uci_nr_pusch_csi1_nof_bits(cfg);
  if (E_uci < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of encoded bits");
    return ISRRAN_ERROR;
  }

  return uci_nr_encode(q, cfg, A, o, E_uci);
}

int isrran_uci_nr_decode_pusch_csi1(isrran_uci_nr_t*           q,
                                    const isrran_uci_cfg_nr_t* cfg,
                                    int8_t*                    llr,
                                    isrran_uci_value_nr_t*     value)
{
  // Check inputs
  if (q == NULL || cfg == NULL || llr == NULL || value == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Compute total of encoded bits according to 6.3.2.4 Rate matching
  int E_uci = isrran_uci_nr_pusch_csi1_nof_bits(cfg);
  if (E_uci < ISRRAN_SUCCESS) {
    ERROR("Error calculating number of encoded bits");
    return ISRRAN_ERROR;
  }

  int A = isrran_csi_part1_nof_bits(cfg->csi, cfg->nof_csi);
  if (A < ISRRAN_SUCCESS) {
    ERROR("Error getting number of CSI part 1 bits");
    return ISRRAN_ERROR;
  }

  // Decode
  if (uci_nr_decode(q, cfg, llr, (uint32_t)A, (uint32_t)E_uci, &value->valid) < ISRRAN_SUCCESS) {
    ERROR("Error decoding UCI");
    return ISRRAN_ERROR;
  }

  // Unpack
  if (isrran_csi_part1_unpack(cfg->csi, cfg->nof_csi, q->bit_sequence, A, value->csi) < ISRRAN_SUCCESS) {
    ERROR("Error unpacking CSI");
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}
