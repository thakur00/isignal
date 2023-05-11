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

#include <strings.h>

#include "isrran/phy/phch/mib_sl.h"
#include "isrran/phy/utils/bit.h"

int isrran_mib_sl_init(isrran_mib_sl_t* q, isrran_sl_tm_t tm)
{
  if (tm == ISRRAN_SIDELINK_TM1 || tm == ISRRAN_SIDELINK_TM2) {
    q->mib_sl_len = ISRRAN_MIB_SL_LEN;
  } else if (tm == ISRRAN_SIDELINK_TM3 || tm == ISRRAN_SIDELINK_TM4) {
    q->mib_sl_len = ISRRAN_MIB_SL_V2X_LEN;
  } else {
    return ISRRAN_ERROR;
  }

  q->sl_bandwidth_r12           = 0;
  q->direct_frame_number_r12    = 0;
  q->direct_subframe_number_r12 = 0;
  q->in_coverage_r12            = false;
  q->tdd_config_sl_r12          = 0;

  return ISRRAN_SUCCESS;
}

int isrran_mib_sl_set(isrran_mib_sl_t* q,
                      uint32_t         nof_prb,
                      uint32_t         tdd_config,
                      uint32_t         direct_frame_number,
                      uint32_t         direct_subframe_number,
                      bool             in_coverage)
{
  int ret = ISRRAN_ERROR_INVALID_INPUTS;
  if (q != NULL) {
    switch (nof_prb) {
      case 6:
        q->sl_bandwidth_r12 = 0;
        break;
      case 15:
        q->sl_bandwidth_r12 = 1;
        break;
      case 25:
        q->sl_bandwidth_r12 = 2;
        break;
      case 50:
        q->sl_bandwidth_r12 = 3;
        break;
      case 75:
        q->sl_bandwidth_r12 = 4;
        break;
      case 100:
        q->sl_bandwidth_r12 = 5;
        break;
      default:
        printf("Invalid bandwidth\n");
        return ISRRAN_ERROR;
    }
    q->tdd_config_sl_r12          = tdd_config;
    q->direct_frame_number_r12    = direct_frame_number;
    q->direct_subframe_number_r12 = direct_subframe_number;
    q->in_coverage_r12            = in_coverage;

    ret = ISRRAN_SUCCESS;
  }

  return ret;
}

void isrran_mib_sl_pack(isrran_mib_sl_t* q, uint8_t* msg)
{
  bzero(msg, sizeof(uint8_t) * ISRRAN_MIB_SL_MAX_LEN);
  isrran_bit_unpack(q->sl_bandwidth_r12, &msg, 3);
  isrran_bit_unpack(q->tdd_config_sl_r12, &msg, 3);
  isrran_bit_unpack(q->direct_frame_number_r12, &msg, 10);
  isrran_bit_unpack(q->direct_subframe_number_r12, &msg, 4);
  isrran_bit_unpack((uint32_t)q->in_coverage_r12, &msg, 1);
}

void isrran_mib_sl_unpack(isrran_mib_sl_t* q, uint8_t* msg)
{
  q->sl_bandwidth_r12           = isrran_bit_pack(&msg, 3);
  q->tdd_config_sl_r12          = isrran_bit_pack(&msg, 3);
  q->direct_frame_number_r12    = isrran_bit_pack(&msg, 10);
  q->direct_subframe_number_r12 = isrran_bit_pack(&msg, 4);
  q->in_coverage_r12            = (bool)isrran_bit_pack(&msg, 1);
}

void isrran_mib_sl_printf(FILE* f, isrran_mib_sl_t* q)
{
  fprintf(f, " - Bandwidth:              %i\n", q->sl_bandwidth_r12);
  fprintf(f, " - Direct Frame Number:    %i\n", q->direct_frame_number_r12);
  fprintf(f, " - Direct Subframe Number: %i\n", q->direct_subframe_number_r12);
  fprintf(f, " - TDD config:             %i\n", q->tdd_config_sl_r12);
  fprintf(f, " - In coverage:            %s\n", q->in_coverage_r12 ? "yes" : "no");
}

void isrran_mib_sl_free(isrran_mib_sl_t* q)
{
  if (q != NULL) {
    bzero(q, sizeof(isrran_mib_sl_t));
  }
}
