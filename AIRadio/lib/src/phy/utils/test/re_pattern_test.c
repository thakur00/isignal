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

#include "isrran/phy/utils/re_pattern.h"
#include "isrran/support/isrran_test.h"

int main(int argc, char** argv)
{
  isrran_re_pattern_list_t pattern_list;

  // Reset list
  isrran_re_pattern_reset(&pattern_list);

  // Create first pattern and merge
  isrran_re_pattern_t pattern_1 = {};
  pattern_1.rb_begin            = 1;
  pattern_1.rb_end              = 50;
  pattern_1.rb_stride           = 1;
  for (uint32_t k = 0; k < ISRRAN_NRE; k++) {
    pattern_1.sc[k] = (k % 2 == 0); // Only even subcarriers
  }
  for (uint32_t l = 0; l < ISRRAN_NSYMB_PER_SLOT_NR; l++) {
    pattern_1.symbol[l] = (l % 2 == 0); // Only even symbols
  }
  TESTASSERT(isrran_re_pattern_merge(&pattern_list, &pattern_1) == ISRRAN_SUCCESS);
  TESTASSERT(pattern_list.count == 1);

  // Create second pattern and merge
  isrran_re_pattern_t pattern_2 = pattern_1;
  for (uint32_t l = 0; l < ISRRAN_NSYMB_PER_SLOT_NR; l++) {
    pattern_2.symbol[l] = (l % 2 == 1); // Only odd symbols
  }
  TESTASSERT(isrran_re_pattern_merge(&pattern_list, &pattern_2) == ISRRAN_SUCCESS);
  TESTASSERT(pattern_list.count == 1);

  // Assert generated mask
  for (uint32_t l = 0; l < ISRRAN_NSYMB_PER_SLOT_NR; l++) {
    bool mask[ISRRAN_NRE * ISRRAN_MAX_PRB_NR] = {};
    TESTASSERT(isrran_re_pattern_list_to_symbol_mask(&pattern_list, l, mask) == ISRRAN_SUCCESS);
    for (uint32_t k = 0; k < ISRRAN_NRE * ISRRAN_MAX_PRB_NR; k++) {
      if (k >= pattern_1.rb_begin * ISRRAN_NRE && k < pattern_1.rb_end * ISRRAN_NRE &&
          (k / ISRRAN_NRE - pattern_1.rb_begin) % pattern_1.rb_stride == 0) {
        TESTASSERT(mask[k] == (k % 2 == 0));
      } else {
        TESTASSERT(mask[k] == false);
      }
    }
  }

  return ISRRAN_SUCCESS;
}