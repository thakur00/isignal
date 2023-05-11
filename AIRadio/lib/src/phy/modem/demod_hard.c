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

#include <stdlib.h>
#include <strings.h>

#include "hard_demod_lte.h"
#include "isrran/phy/modem/demod_hard.h"

void isrran_demod_hard_init(isrran_demod_hard_t* q)
{
  bzero((void*)q, sizeof(isrran_demod_hard_t));
}

void isrran_demod_hard_table_set(isrran_demod_hard_t* q, isrran_mod_t mod)
{
  q->mod = mod;
}

int isrran_demod_hard_demodulate(isrran_demod_hard_t* q, cf_t* symbols, uint8_t* bits, uint32_t nsymbols)
{

  int nbits = -1;
  switch (q->mod) {
    case ISRRAN_MOD_BPSK:
      hard_bpsk_demod(symbols, bits, nsymbols);
      nbits = nsymbols;
      break;
    case ISRRAN_MOD_QPSK:
      hard_qpsk_demod(symbols, bits, nsymbols);
      nbits = nsymbols * 2;
      break;
    case ISRRAN_MOD_16QAM:
      hard_qam16_demod(symbols, bits, nsymbols);
      nbits = nsymbols * 4;
      break;
    case ISRRAN_MOD_64QAM:
      hard_qam64_demod(symbols, bits, nsymbols);
      nbits = nsymbols * 6;
      break;
    case ISRRAN_MOD_256QAM:
      hard_qam256_demod(symbols, bits, nsymbols);
      nbits = nsymbols * 8;
      break;
    case ISRRAN_MOD_NITEMS:
    default:; // Do nothing
  }
  return nbits;
}
