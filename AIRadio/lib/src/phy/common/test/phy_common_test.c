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
#include "isrran/common/test_common.h"
#include "isrran/phy/common/phy_common.h"

int isrran_default_rates_test()
{
  // Verify calculated sample rates for all valid PRB sizes.
  // By default we use the reduced 3/4 sampling to save bandwidth on the fronthaul.
#ifdef FORCE_STANDARD_RATE
  isrran_use_standard_symbol_size(false);
#endif
  TESTASSERT(isrran_sampling_freq_hz(6) == 1920000);
  TESTASSERT(isrran_sampling_freq_hz(15) == 3840000);
  TESTASSERT(isrran_sampling_freq_hz(25) == 5760000);
  TESTASSERT(isrran_sampling_freq_hz(50) == 11520000);
  TESTASSERT(isrran_sampling_freq_hz(75) == 15360000); // need to use default rate for 15 MHz BW
  TESTASSERT(isrran_sampling_freq_hz(100) == 23040000);
  return ISRRAN_SUCCESS;
}

int lte_standard_rates_test()
{
  // Verify calculated sample rates for all valid PRB sizes.
  // Enable standard LTE rates (required by some RF HW).
  isrran_use_standard_symbol_size(true);
  TESTASSERT(isrran_sampling_freq_hz(6) == 1920000);
  TESTASSERT(isrran_sampling_freq_hz(15) == 3840000);
  TESTASSERT(isrran_sampling_freq_hz(25) == 7680000);
  TESTASSERT(isrran_sampling_freq_hz(50) == 15360000);
  TESTASSERT(isrran_sampling_freq_hz(75) == 23040000);
  TESTASSERT(isrran_sampling_freq_hz(100) == 30720000);
  return ISRRAN_SUCCESS;
}

int main(int argc, char** argv)
{
  TESTASSERT(isrran_default_rates_test() == ISRRAN_SUCCESS);
  TESTASSERT(lte_standard_rates_test() == ISRRAN_SUCCESS);
  return ISRRAN_SUCCESS;
}