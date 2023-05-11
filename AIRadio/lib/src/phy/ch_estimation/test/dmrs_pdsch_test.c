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

#include "isrran/phy/ch_estimation/dmrs_sch.h"
#include "isrran/phy/phch/ra_dl_nr.h"
#include "isrran/isrran.h"
#include "isrran/support/isrran_test.h"
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

static isrran_carrier_nr_t carrier = ISRRAN_DEFAULT_CARRIER_NR;

typedef struct {
  isrran_sch_mapping_type_t   mapping_type;
  isrran_dmrs_sch_typeA_pos_t typeA_pos;
  isrran_dmrs_sch_len_t       max_length;
  isrran_dmrs_sch_add_pos_t   additional_pos;
  isrran_dmrs_sch_type_t      type;
  uint32_t                    symbol_idx[ISRRAN_DMRS_SCH_MAX_SYMBOLS];
  uint32_t                    nof_symbols;
  uint32_t                    sc_idx[ISRRAN_NRE];
  uint32_t                    nof_sc;
} golden_t;

// Golden values extracted from https://www.sharetechnote.com/html/5G/5G_PDSCH_DMRS.html
static const golden_t gold[] = {{.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_2,
                                 .max_length     = isrran_dmrs_sch_len_1,
                                 .additional_pos = isrran_dmrs_sch_add_pos_0,
                                 .type           = isrran_dmrs_sch_type_2,
                                 .nof_symbols    = 1,
                                 .symbol_idx     = {2},
                                 .nof_sc         = 4,
                                 .sc_idx         = {0, 1, 6, 7}},
                                {.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_3,
                                 .max_length     = isrran_dmrs_sch_len_1,
                                 .additional_pos = isrran_dmrs_sch_add_pos_0,
                                 .type           = isrran_dmrs_sch_type_2,
                                 .nof_symbols    = 1,
                                 .symbol_idx     = {3},
                                 .nof_sc         = 4,
                                 .sc_idx         = {0, 1, 6, 7}},
                                {.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_2,
                                 .max_length     = isrran_dmrs_sch_len_2,
                                 .additional_pos = isrran_dmrs_sch_add_pos_0,
                                 .type           = isrran_dmrs_sch_type_2,
                                 .nof_symbols    = 2,
                                 .symbol_idx     = {2, 3},
                                 .nof_sc         = 4,
                                 .sc_idx         = {0, 1, 6, 7}},
                                {.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_2,
                                 .max_length     = isrran_dmrs_sch_len_1,
                                 .additional_pos = isrran_dmrs_sch_add_pos_1,
                                 .type           = isrran_dmrs_sch_type_2,
                                 .nof_symbols    = 2,
                                 .symbol_idx     = {2, 11},
                                 .nof_sc         = 4,
                                 .sc_idx         = {0, 1, 6, 7}},
                                {.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_2,
                                 .max_length     = isrran_dmrs_sch_len_1,
                                 .additional_pos = isrran_dmrs_sch_add_pos_2,
                                 .type           = isrran_dmrs_sch_type_2,
                                 .nof_symbols    = 3,
                                 .symbol_idx     = {2, 7, 11},
                                 .nof_sc         = 4,
                                 .sc_idx         = {0, 1, 6, 7}},
                                {.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_2,
                                 .max_length     = isrran_dmrs_sch_len_1,
                                 .additional_pos = isrran_dmrs_sch_add_pos_3,
                                 .type           = isrran_dmrs_sch_type_2,
                                 .nof_symbols    = 4,
                                 .symbol_idx     = {2, 5, 8, 11},
                                 .nof_sc         = 4,
                                 .sc_idx         = {0, 1, 6, 7}},
                                {.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_2,
                                 .max_length     = isrran_dmrs_sch_len_1,
                                 .additional_pos = isrran_dmrs_sch_add_pos_0,
                                 .type           = isrran_dmrs_sch_type_1,
                                 .nof_symbols    = 1,
                                 .symbol_idx     = {2},
                                 .nof_sc         = 6,
                                 .sc_idx         = {0, 2, 4, 6, 8, 10}},
                                {.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_2,
                                 .max_length     = isrran_dmrs_sch_len_2,
                                 .additional_pos = isrran_dmrs_sch_add_pos_0,
                                 .type           = isrran_dmrs_sch_type_1,
                                 .nof_symbols    = 2,
                                 .symbol_idx     = {2, 3},
                                 .nof_sc         = 6,
                                 .sc_idx         = {0, 2, 4, 6, 8, 10}},
                                {.mapping_type   = isrran_sch_mapping_type_A,
                                 .typeA_pos      = isrran_dmrs_sch_typeA_pos_2,
                                 .max_length     = isrran_dmrs_sch_len_1,
                                 .additional_pos = isrran_dmrs_sch_add_pos_3,
                                 .type           = isrran_dmrs_sch_type_1,
                                 .nof_symbols    = 4,
                                 .symbol_idx     = {2, 5, 8, 11},
                                 .nof_sc         = 6,
                                 .sc_idx         = {0, 2, 4, 6, 8, 10}},
                                {}};

static void usage(char* prog)
{
  printf("Usage: %s [recov]\n", prog);

  printf("\t-r nof_prb [Default %d]\n", carrier.nof_prb);

  printf("\t-c cell_id [Default %d]\n", carrier.pci);

  printf("\t-v increase verbosity\n");
}

static void parse_args(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "rcov")) != -1) {
    switch (opt) {
      case 'r':
        carrier.nof_prb = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'c':
        carrier.pci = (uint32_t)strtol(argv[optind], NULL, 10);
        break;
      case 'v':
        increase_isrran_verbose_level();
        break;
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
}

static int assert_cfg(const isrran_sch_cfg_nr_t* pdsch_cfg, const isrran_sch_grant_nr_t* grant)
{
  for (uint32_t i = 0; gold[i].nof_sc != 0; i++) {
    // Gold examples are done for more than 12 symbols
    if (grant->L <= 12) {
      continue;
    }

    // Skip golden sample if one of the parameters does not match
    if (grant->mapping != gold[i].mapping_type || pdsch_cfg->dmrs.typeA_pos != gold[i].typeA_pos ||
        pdsch_cfg->dmrs.additional_pos != gold[i].additional_pos || pdsch_cfg->dmrs.length != gold[i].max_length ||
        pdsch_cfg->dmrs.type != gold[i].type) {
      continue;
    }

    // Generate subcarrier mask from golden sample
    bool sc_mask[ISRRAN_NRE] = {};
    if (grant->nof_dmrs_cdm_groups_without_data == 1) {
      for (uint32_t j = 0; j < gold[i].nof_sc; j++) {
        sc_mask[gold[i].sc_idx[j] % ISRRAN_NRE] = true;
      }
    } else if (pdsch_cfg->dmrs.type == isrran_dmrs_sch_type_1) {
      for (uint32_t k = 0; k < ISRRAN_NRE; k++) {
        sc_mask[k] = true;
      }
    } else if (pdsch_cfg->dmrs.type == isrran_dmrs_sch_type_2) {
      for (uint32_t k = 0; k < ISRRAN_NRE; k++) {
        sc_mask[k] = ((k % 6) < grant->nof_dmrs_cdm_groups_without_data * 2);
      }
    }

    // Generate symbol mask from golden sample
    bool symbol_mask[ISRRAN_NSYMB_PER_SLOT_NR] = {};
    for (uint32_t j = 0; j < gold[i].nof_symbols; j++) {
      symbol_mask[gold[i].symbol_idx[j] % ISRRAN_NSYMB_PER_SLOT_NR] = true;
    }

    // Generate DMRS pattern
    isrran_re_pattern_t pattern = {};
    TESTASSERT(isrran_dmrs_sch_rvd_re_pattern(&pdsch_cfg->dmrs, grant, &pattern) == ISRRAN_SUCCESS);

    // Assert subcarrier mask
    TESTASSERT(memcmp(pattern.sc, sc_mask, sizeof(bool) * ISRRAN_NRE) == 0);

    // Assert symbol mask
    TESTASSERT(memcmp(pattern.symbol, symbol_mask, sizeof(bool) * ISRRAN_NSYMB_PER_SLOT_NR) == 0);

    return ISRRAN_SUCCESS;
  }

  return ISRRAN_SUCCESS;
}

static int run_test(isrran_dmrs_sch_t*           dmrs_pdsch,
                    const isrran_sch_cfg_nr_t*   pdsch_cfg,
                    const isrran_sch_grant_nr_t* grant,
                    cf_t*                        sf_symbols,
                    isrran_chest_dl_res_t*       chest_res)
{
  TESTASSERT(assert_cfg(pdsch_cfg, grant) == ISRRAN_SUCCESS);

  isrran_slot_cfg_t slot_cfg = {};
  for (slot_cfg.idx = 0; slot_cfg.idx < ISRRAN_NSLOTS_PER_FRAME_NR(dmrs_pdsch->carrier.scs); slot_cfg.idx++) {
    TESTASSERT(isrran_dmrs_sch_put_sf(dmrs_pdsch, &slot_cfg, pdsch_cfg, grant, sf_symbols) == ISRRAN_SUCCESS);

    TESTASSERT(isrran_dmrs_sch_estimate(dmrs_pdsch, &slot_cfg, pdsch_cfg, grant, sf_symbols, chest_res) ==
               ISRRAN_SUCCESS);

    float mse = 0.0f;
    for (uint32_t i = 0; i < chest_res->nof_re; i++) {
      cf_t err = chest_res->ce[0][0][i] - 1.0f;
      mse += cabsf(err);
    }
    mse /= (float)chest_res->nof_re;

    TESTASSERT(!isnan(mse));
    TESTASSERT(mse < 1e-6f);
  }

  return ISRRAN_SUCCESS;
}

int main(int argc, char** argv)
{
  int ret = ISRRAN_ERROR;

  parse_args(argc, argv);

  isrran_dmrs_sch_t     dmrs_pdsch   = {};
  isrran_sch_cfg_nr_t   pdsch_cfg    = {};
  isrran_sch_grant_nr_t grant        = {};
  isrran_chest_dl_res_t chest_dl_res = {};

  uint32_t nof_re     = carrier.nof_prb * ISRRAN_NRE * ISRRAN_NOF_SLOTS_PER_SF * ISRRAN_MAX_NSYMB;
  cf_t*    sf_symbols = isrran_vec_cf_malloc(nof_re);

  uint32_t test_counter = 0;
  uint32_t test_passed  = 0;

  // Initialise object DMRS for PDSCH
  if (isrran_dmrs_sch_init(&dmrs_pdsch, true) != ISRRAN_SUCCESS) {
    ERROR("Init");
    goto clean_exit;
  }

  // Set carrier configuration
  if (isrran_dmrs_sch_set_carrier(&dmrs_pdsch, &carrier) != ISRRAN_SUCCESS) {
    ERROR("Setting carrier");
    goto clean_exit;
  }

  if (isrran_chest_dl_res_init(&chest_dl_res, carrier.nof_prb) != ISRRAN_SUCCESS) {
    ERROR("Initiating channel estimation  result");
    goto clean_exit;
  }

  // For each DCI m param
  for (uint32_t m = 0; m < 16; m++) {
    isrran_dmrs_sch_type_t type_begin = isrran_dmrs_sch_type_1;
    isrran_dmrs_sch_type_t type_end   = isrran_dmrs_sch_type_2;

    for (pdsch_cfg.dmrs.type = type_begin; pdsch_cfg.dmrs.type <= type_end; pdsch_cfg.dmrs.type++) {
      isrran_dmrs_sch_typeA_pos_t typeA_pos_begin = isrran_dmrs_sch_typeA_pos_2;
      isrran_dmrs_sch_typeA_pos_t typeA_pos_end   = isrran_dmrs_sch_typeA_pos_3;

      for (pdsch_cfg.dmrs.typeA_pos = typeA_pos_begin; pdsch_cfg.dmrs.typeA_pos <= typeA_pos_end;
           pdsch_cfg.dmrs.typeA_pos++) {
        isrran_dmrs_sch_add_pos_t add_pos_begin = isrran_dmrs_sch_add_pos_2;
        isrran_dmrs_sch_add_pos_t add_pos_end   = isrran_dmrs_sch_add_pos_3;

        if (pdsch_cfg.dmrs.typeA_pos == isrran_dmrs_sch_typeA_pos_3) {
          add_pos_end = isrran_dmrs_sch_add_pos_1;
        }

        for (pdsch_cfg.dmrs.additional_pos = add_pos_begin; pdsch_cfg.dmrs.additional_pos <= add_pos_end;
             pdsch_cfg.dmrs.additional_pos++) {
          isrran_dmrs_sch_len_t max_len_begin = isrran_dmrs_sch_len_1;
          isrran_dmrs_sch_len_t max_len_end   = isrran_dmrs_sch_len_2;

          // Only single DMRS symbols can have additional positions 2 and 3
          if (pdsch_cfg.dmrs.additional_pos == isrran_dmrs_sch_add_pos_2 ||
              pdsch_cfg.dmrs.additional_pos == isrran_dmrs_sch_add_pos_3) {
            max_len_end = isrran_dmrs_sch_len_1;
          }

          for (pdsch_cfg.dmrs.length = max_len_begin; pdsch_cfg.dmrs.length <= max_len_end; pdsch_cfg.dmrs.length++) {
            for (uint32_t bw = 1; bw <= carrier.nof_prb; bw++) {
              for (uint32_t i = 0; i < carrier.nof_prb; i++) {
                grant.prb_idx[i] = (i < bw);
              }

              for (grant.nof_dmrs_cdm_groups_without_data = 1; grant.nof_dmrs_cdm_groups_without_data <= 3;
                   grant.nof_dmrs_cdm_groups_without_data++) {
                // Load default type A grant
                if (isrran_ra_dl_nr_time_default_A(m, pdsch_cfg.dmrs.typeA_pos, &grant) < ISRRAN_SUCCESS) {
                  ERROR("Error loading time resource");
                  continue;
                }

                // Mapping type B is not supported
                if (grant.mapping == isrran_sch_mapping_type_B) {
                  continue;
                }

                int n = run_test(&dmrs_pdsch, &pdsch_cfg, &grant, sf_symbols, &chest_dl_res);

                if (n == ISRRAN_SUCCESS) {
                  test_passed++;
                } else {
                  char str[64] = {};
                  isrran_dmrs_sch_cfg_to_str(&pdsch_cfg.dmrs, str, 64);

                  ERROR("Test %d failed. %s.", test_counter, str);
                }

                test_counter++;
              }
            }
          }
        }
      }
    }
  }

clean_exit:

  if (sf_symbols) {
    free(sf_symbols);
  }
  isrran_chest_dl_res_free(&chest_dl_res);
  isrran_dmrs_sch_free(&dmrs_pdsch);

  ret = test_passed == test_counter ? ISRRAN_SUCCESS : ISRRAN_ERROR;
  printf("%s, %d of %d test passed successfully.\n", ret ? "Failed" : "Passed", test_passed, test_counter);

  return ret;
}
