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

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "lte_tables.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/modem/modem_table.h"
#include "isrran/phy/utils/vector.h"

/** Internal functions */
static int table_create(isrran_modem_table_t* q)
{
  q->symbol_table = isrran_vec_cf_malloc(q->nsymbols);
  return q->symbol_table == NULL;
}

void isrran_modem_table_init(isrran_modem_table_t* q)
{
  bzero((void*)q, sizeof(isrran_modem_table_t));
}
void isrran_modem_table_free(isrran_modem_table_t* q)
{
  if (q->symbol_table) {
    free(q->symbol_table);
  }
  if (q->symbol_table_bpsk) {
    free(q->symbol_table_bpsk);
  }
  if (q->symbol_table_qpsk) {
    free(q->symbol_table_qpsk);
  }
  if (q->symbol_table_16qam) {
    free(q->symbol_table_16qam);
  }
  bzero(q, sizeof(isrran_modem_table_t));
}
void isrran_modem_table_reset(isrran_modem_table_t* q)
{
  isrran_modem_table_free(q);
  isrran_modem_table_init(q);
}

int isrran_modem_table_set(isrran_modem_table_t* q, cf_t* table, uint32_t nsymbols, uint32_t nbits_x_symbol)
{
  if (q->nsymbols) {
    return ISRRAN_ERROR;
  }
  q->nsymbols = nsymbols;
  if (table_create(q)) {
    return ISRRAN_ERROR;
  }
  memcpy(q->symbol_table, table, q->nsymbols * sizeof(cf_t));
  q->nbits_x_symbol = nbits_x_symbol;
  return ISRRAN_SUCCESS;
}

int isrran_modem_table_lte(isrran_modem_table_t* q, isrran_mod_t modulation)
{
  isrran_modem_table_init(q);
  switch (modulation) {
    case ISRRAN_MOD_BPSK:
      q->nbits_x_symbol = 1;
      q->nsymbols       = 2;
      if (table_create(q)) {
        return ISRRAN_ERROR;
      }
      set_BPSKtable(q->symbol_table);
      break;
    case ISRRAN_MOD_QPSK:
      q->nbits_x_symbol = 2;
      q->nsymbols       = 4;
      if (table_create(q)) {
        return ISRRAN_ERROR;
      }
      set_QPSKtable(q->symbol_table);
      break;
    case ISRRAN_MOD_16QAM:
      q->nbits_x_symbol = 4;
      q->nsymbols       = 16;
      if (table_create(q)) {
        return ISRRAN_ERROR;
      }
      set_16QAMtable(q->symbol_table);
      break;
    case ISRRAN_MOD_64QAM:
      q->nbits_x_symbol = 6;
      q->nsymbols       = 64;
      if (table_create(q)) {
        return ISRRAN_ERROR;
      }
      set_64QAMtable(q->symbol_table);
      break;
    case ISRRAN_MOD_256QAM:
      q->nbits_x_symbol = 8;
      q->nsymbols       = 256;
      if (table_create(q)) {
        return ISRRAN_ERROR;
      }
      set_256QAMtable(q->symbol_table);
      break;
    case ISRRAN_MOD_NITEMS:
    default:; // Do nothing
  }
  return ISRRAN_SUCCESS;
}

void isrran_modem_table_bytes(isrran_modem_table_t* q)
{
  uint8_t mask_qpsk[4]  = {0xc0, 0x30, 0xc, 0x3};
  uint8_t mask_16qam[2] = {0xf0, 0xf};

  switch (q->nbits_x_symbol) {
    case 1:
      q->symbol_table_bpsk = isrran_vec_malloc(sizeof(bpsk_packed_t) * 256);
      for (uint32_t i = 0; i < 256; i++) {
        for (int j = 0; j < 8; j++) {
          q->symbol_table_bpsk[i].symbol[j] = q->symbol_table[(i & (1 << (7 - j))) >> (7 - j)];
        }
      }
      q->byte_tables_init = true;
      break;
    case 2:
      q->symbol_table_qpsk = isrran_vec_malloc(sizeof(qpsk_packed_t) * 256);
      for (uint32_t i = 0; i < 256; i++) {
        for (int j = 0; j < 4; j++) {
          q->symbol_table_qpsk[i].symbol[j] = q->symbol_table[(i & mask_qpsk[j]) >> (6 - j * 2)];
        }
      }
      q->byte_tables_init = true;
      break;
    case 4:
      q->symbol_table_16qam = isrran_vec_malloc(sizeof(qam16_packed_t) * 256);
      for (uint32_t i = 0; i < 256; i++) {
        for (int j = 0; j < 2; j++) {
          q->symbol_table_16qam[i].symbol[j] = q->symbol_table[(i & mask_16qam[j]) >> (4 - j * 4)];
        }
      }
      q->byte_tables_init = true;
      break;
    case 6:
      q->byte_tables_init = true;
      break;
    case 8:
      q->byte_tables_init = true;
      break;
  }
}
