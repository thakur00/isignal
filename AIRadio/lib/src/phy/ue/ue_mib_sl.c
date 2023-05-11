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
#include <isrran/config.h>
#include <isrran/phy/utils/bit.h>
#include <isrran/phy/utils/vector.h>
#include <stdlib.h>
#include <strings.h>

#include <isrran/phy/ue/ue_mib_sl.h>
#include <isrran/phy/utils/debug.h>

int isrran_ue_mib_sl_set(isrran_ue_mib_sl_t* q,
                         uint32_t            nof_prb,
                         uint32_t            tdd_config,
                         uint32_t            direct_frame_number,
                         uint32_t            direct_subframe_number,
                         bool                in_coverage)
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

void isrran_ue_mib_sl_pack(isrran_ue_mib_sl_t* q, uint8_t* msg)
{
  bzero(msg, sizeof(uint8_t) * ISRRAN_MIB_SL_MAX_LEN);
  isrran_bit_unpack(q->sl_bandwidth_r12, &msg, 3);
  isrran_bit_unpack(q->tdd_config_sl_r12, &msg, 3);
  isrran_bit_unpack(q->direct_frame_number_r12, &msg, 10);
  isrran_bit_unpack(q->direct_subframe_number_r12, &msg, 4);
  isrran_bit_unpack((uint32_t)q->in_coverage_r12, &msg, 1);
}

void srlste_ue_mib_sl_unpack(isrran_ue_mib_sl_t* q, uint8_t* msg)
{
  q->sl_bandwidth_r12           = isrran_bit_pack(&msg, 3);
  q->tdd_config_sl_r12          = isrran_bit_pack(&msg, 3);
  q->direct_frame_number_r12    = isrran_bit_pack(&msg, 10);
  q->direct_subframe_number_r12 = isrran_bit_pack(&msg, 4);
  q->in_coverage_r12            = (bool)isrran_bit_pack(&msg, 1);
}

void isrran_ue_mib_sl_free(isrran_ue_mib_sl_t* q)
{
  if (q != NULL) {
    bzero(q, sizeof(isrran_ue_mib_sl_t));
  }
}