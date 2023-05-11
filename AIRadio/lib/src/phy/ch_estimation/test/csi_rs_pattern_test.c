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

#include "isrran/phy/ch_estimation/csi_rs.h"
#include "isrran/support/isrran_test.h"

static isrran_carrier_nr_t carrier = {};

static int test_row1()
{
  for (uint32_t k = 0; k < ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW1; k++) {
    // Create CSI-RS mapping
    isrran_csi_rs_resource_mapping_t m; // Dont initialise for detecting not initialised memory
    m.row = isrran_csi_rs_resource_mapping_row_1;
    for (uint32_t j = 0; j < ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW1; j++) {
      m.frequency_domain_alloc[j] = (k == j);
    }
    m.nof_ports          = 1;
    m.first_symbol_idx   = 0;
    m.first_symbol_idx2  = 0;
    m.cdm                = isrran_csi_rs_cdm_nocdm;
    m.density            = isrran_csi_rs_resource_mapping_density_three;
    m.freq_band.start_rb = carrier.start;
    m.freq_band.nof_rb   = carrier.nof_prb;

    // Create Pattern list and initialise
    isrran_re_pattern_list_t patterns;
    isrran_re_pattern_reset(&patterns);

    // Generate pattern list from CSI-RS mapping
    TESTASSERT(isrran_csi_rs_append_resource_to_pattern(&carrier, &m, &patterns) == ISRRAN_SUCCESS);

    // Assert generated pattern
    TESTASSERT(patterns.count == 1);
    TESTASSERT(patterns.data[0].rb_begin == m.freq_band.start_rb);
    TESTASSERT(patterns.data[0].rb_end == m.freq_band.start_rb + m.freq_band.nof_rb);
    TESTASSERT(patterns.data[0].rb_stride == 1);
    for (uint32_t j = 0; j < ISRRAN_NRE; j++) {
      TESTASSERT(patterns.data[0].sc[j] ==
                 (j % ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW1 == (ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW1 - 1 - k)));
    }
    for (uint32_t j = 0; j < ISRRAN_NSYMB_PER_SLOT_NR; j++) {
      TESTASSERT(patterns.data[0].symbol[j] == (j == m.first_symbol_idx));
    }
  }

  return ISRRAN_SUCCESS;
}

static int test_row2()
{
  for (isrran_csi_rs_density_t density = isrran_csi_rs_resource_mapping_density_dot5_even;
       density <= isrran_csi_rs_resource_mapping_density_one;
       density++) {
    for (uint32_t k = 0; k < ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW2; k++) {
      // Create CSI-RS mapping
      isrran_csi_rs_resource_mapping_t m; // Dont initialise for detecting not initialised memory
      m.row = isrran_csi_rs_resource_mapping_row_2;
      for (uint32_t j = 0; j < ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW2; j++) {
        m.frequency_domain_alloc[j] = (k == j);
      }
      m.nof_ports          = 1;
      m.first_symbol_idx   = 0;
      m.first_symbol_idx2  = 0;
      m.cdm                = isrran_csi_rs_cdm_nocdm;
      m.density            = density;
      m.freq_band.start_rb = carrier.start;
      m.freq_band.nof_rb   = carrier.nof_prb;

      // Create Pattern list and initialise
      isrran_re_pattern_list_t patterns;
      isrran_re_pattern_reset(&patterns);

      // Generate pattern list from CSI-RS mapping
      TESTASSERT(isrran_csi_rs_append_resource_to_pattern(&carrier, &m, &patterns) == ISRRAN_SUCCESS);

      // Assert generated pattern
      uint32_t rb_stride = (density == isrran_csi_rs_resource_mapping_density_one) ? 1 : 2;
      uint32_t rb_start  = carrier.start;
      if ((rb_start % 2 == 0 && density == isrran_csi_rs_resource_mapping_density_dot5_odd) ||
          (rb_start % 2 == 1 && density == isrran_csi_rs_resource_mapping_density_dot5_even)) {
        rb_start++;
      }
      TESTASSERT(patterns.count == 1);
      TESTASSERT(patterns.data[0].rb_begin == rb_start);
      TESTASSERT(patterns.data[0].rb_end == m.freq_band.start_rb + m.freq_band.nof_rb);
      TESTASSERT(patterns.data[0].rb_stride == rb_stride);
      for (uint32_t j = 0; j < ISRRAN_NRE; j++) {
        TESTASSERT(patterns.data[0].sc[j] == (j == (ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW2 - 1 - k)));
      }
      for (uint32_t j = 0; j < ISRRAN_NSYMB_PER_SLOT_NR; j++) {
        TESTASSERT(patterns.data[0].symbol[j] == (j == m.first_symbol_idx));
      }
    }
  }

  return ISRRAN_SUCCESS;
}

static int test_row4()
{
  for (uint32_t k = 0; k < ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW4; k++) {
    // Create CSI-RS mapping
    isrran_csi_rs_resource_mapping_t m; // Dont initialise for detecting not initialised memory
    m.row = isrran_csi_rs_resource_mapping_row_4;
    for (uint32_t j = 0; j < ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW4; j++) {
      m.frequency_domain_alloc[j] = (k == j);
    }
    m.nof_ports          = 4;
    m.first_symbol_idx   = 0;
    m.cdm                = isrran_csi_rs_cdm_fd_cdm2;
    m.density            = isrran_csi_rs_resource_mapping_density_one;
    m.freq_band.start_rb = carrier.start;
    m.freq_band.nof_rb   = carrier.nof_prb;

    // Create Pattern list and initialise
    isrran_re_pattern_list_t patterns;
    isrran_re_pattern_reset(&patterns);

    // Generate pattern list from CSI-RS mapping
    TESTASSERT(isrran_csi_rs_append_resource_to_pattern(&carrier, &m, &patterns) == ISRRAN_SUCCESS);

    // Assert generated pattern
    TESTASSERT(patterns.count == 1);
    TESTASSERT(patterns.data[0].rb_begin == m.freq_band.start_rb);
    TESTASSERT(patterns.data[0].rb_end == m.freq_band.start_rb + m.freq_band.nof_rb);
    TESTASSERT(patterns.data[0].rb_stride == 1);
    for (uint32_t j = 0; j < ISRRAN_NRE; j++) {
      uint32_t k_begin = (ISRRAN_CSI_RS_NOF_FREQ_DOMAIN_ALLOC_ROW4 - 1 - k) * 4;
      uint32_t k_end   = k_begin + 4;
      TESTASSERT(patterns.data[0].sc[j] == (j >= k_begin && j < k_end));
    }
    for (uint32_t j = 0; j < ISRRAN_NSYMB_PER_SLOT_NR; j++) {
      TESTASSERT(patterns.data[0].symbol[j] == (j == m.first_symbol_idx));
    }
  }

  return ISRRAN_SUCCESS;
}

static int test_mix()
{
  isrran_csi_rs_resource_mapping_t resource0 = {};
  resource0.row                              = isrran_csi_rs_resource_mapping_row_4;
  resource0.frequency_domain_alloc[0]        = true;
  resource0.frequency_domain_alloc[1]        = false;
  resource0.frequency_domain_alloc[2]        = false;
  resource0.nof_ports                        = 4;
  resource0.first_symbol_idx                 = 8;
  resource0.cdm                              = isrran_csi_rs_cdm_fd_cdm2;
  resource0.density                          = isrran_csi_rs_resource_mapping_density_one;
  resource0.freq_band.start_rb               = 0;
  resource0.freq_band.nof_rb                 = 52;

  isrran_csi_rs_resource_mapping_t resource1 = {};
  resource1.row                              = isrran_csi_rs_resource_mapping_row_2;
  resource1.frequency_domain_alloc[0]        = true;
  resource1.frequency_domain_alloc[1]        = false;
  resource1.frequency_domain_alloc[2]        = false;
  resource1.frequency_domain_alloc[3]        = false;
  resource1.frequency_domain_alloc[4]        = false;
  resource1.frequency_domain_alloc[5]        = false;
  resource1.frequency_domain_alloc[6]        = false;
  resource1.frequency_domain_alloc[7]        = false;
  resource1.frequency_domain_alloc[8]        = false;
  resource1.frequency_domain_alloc[9]        = false;
  resource1.frequency_domain_alloc[10]       = false;
  resource1.frequency_domain_alloc[11]       = false;
  resource1.nof_ports                        = 1;
  resource1.first_symbol_idx                 = 4;
  resource1.cdm                              = isrran_csi_rs_cdm_nocdm;
  resource1.density                          = isrran_csi_rs_resource_mapping_density_one;
  resource1.freq_band.start_rb               = 0;
  resource1.freq_band.nof_rb                 = 52;

  // Initialise pattern list
  isrran_re_pattern_list_t patterns;
  isrran_re_pattern_reset(&patterns);

  // Generate pattern list from CSI-RS mapping
  TESTASSERT(isrran_csi_rs_append_resource_to_pattern(&carrier, &resource0, &patterns) == ISRRAN_SUCCESS);
  TESTASSERT(isrran_csi_rs_append_resource_to_pattern(&carrier, &resource1, &patterns) == ISRRAN_SUCCESS);

  // Assert generated pattern
  TESTASSERT(patterns.count == 2);

  for (uint32_t l = 0; l < ISRRAN_NSYMB_PER_SLOT_NR; l++) {
    bool mask[ISRRAN_NRE * ISRRAN_MAX_PRB_NR] = {};

    TESTASSERT(isrran_re_pattern_list_to_symbol_mask(&patterns, l, mask) == ISRRAN_SUCCESS);

    if (l == resource0.first_symbol_idx) {
      for (uint32_t k = 0; k < ISRRAN_NRE * ISRRAN_MAX_PRB_NR; k++) {
        TESTASSERT(mask[k] == ((k < 52 * ISRRAN_NRE) && (k % ISRRAN_NRE >= 8)));
      }
    } else if (l == resource1.first_symbol_idx) {
      for (uint32_t k = 0; k < ISRRAN_NRE * ISRRAN_MAX_PRB_NR; k++) {
        TESTASSERT(mask[k] == ((k < 52 * ISRRAN_NRE) && (k % ISRRAN_NRE == 11)));
      }
    } else {
      for (uint32_t k = 0; k < ISRRAN_NRE * ISRRAN_MAX_PRB_NR; k++) {
        TESTASSERT(mask[k] == false);
      }
    }
  }

  return ISRRAN_SUCCESS;
}

int main(int argc, char** argv)
{
  // Initialise carrier
  carrier.start   = 0;
  carrier.nof_prb = 52;

  TESTASSERT(test_row1() == ISRRAN_SUCCESS);
  TESTASSERT(test_row2() == ISRRAN_SUCCESS);
  TESTASSERT(test_row4() == ISRRAN_SUCCESS);
  TESTASSERT(test_mix() == ISRRAN_SUCCESS);

  return ISRRAN_SUCCESS;
}
