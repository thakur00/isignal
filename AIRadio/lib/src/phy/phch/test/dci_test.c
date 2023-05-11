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

#include "isrran/phy/phch/dci.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/random.h"
#include "isrran/isrran.h"
#include "isrran/support/isrran_test.h"
#include <getopt.h>

#define UE_CRNTI 0x1234

static int test_pdcch_orders()
{
  static isrran_cell_t cell = {
      52,                // nof_prb
      1,                 // nof_ports
      0,                 // cell_id
      ISRRAN_CP_NORM,    // cyclic prefix
      ISRRAN_PHICH_NORM, // PHICH length
      ISRRAN_PHICH_R_1,  // PHICH resources
      ISRRAN_FDD,
  };

  isrran_dl_sf_cfg_t dl_sf;
  ZERO_OBJECT(dl_sf);

  isrran_dci_location_t locations[ISRRAN_NOF_SF_X_FRAME][30];
  static uint32_t       cfi = 2;

  static isrran_pdcch_t pdcch;
  static isrran_regs_t  regs;

  if (isrran_regs_init(&regs, cell)) {
    ERROR("Error initiating regs");
    exit(-1);
  }

  if (isrran_pdcch_init_enb(&pdcch, cell.nof_prb)) {
    ERROR("Error creating PDCCH object");
    exit(-1);
  }
  if (isrran_pdcch_set_cell(&pdcch, &regs, cell)) {
    ERROR("Error creating PDCCH object");
    exit(-1);
  }

  /* Initiate valid DCI locations */
  for (int i = 0; i < ISRRAN_NOF_SF_X_FRAME; i++) {
    dl_sf.cfi = cfi;
    dl_sf.tti = i;
    isrran_pdcch_ue_locations(&pdcch, &dl_sf, locations[i], 30, UE_CRNTI);
  }

  isrran_dci_dl_t dci_tx;
  bzero(&dci_tx, sizeof(isrran_dci_dl_t));
  dci_tx.rnti           = UE_CRNTI;
  dci_tx.location       = locations[1][0];
  dci_tx.format         = ISRRAN_DCI_FORMAT1A;
  dci_tx.cif_present    = false;
  dci_tx.is_pdcch_order = true;
  dci_tx.preamble_idx   = 0;
  dci_tx.prach_mask_idx = 0;

  isrran_dci_cfg_t cfg    = {};
  cfg.cif_enabled         = false;
  cfg.isr_request_enabled = false;

  // Pack
  isrran_dci_msg_t dci_msg = {};
  TESTASSERT(isrran_dci_msg_pack_pdsch(&cell, &dl_sf, &cfg, &dci_tx, &dci_msg) == ISRRAN_SUCCESS);

  // Unpack
  isrran_dci_dl_t dci_rx = {};
  TESTASSERT(isrran_dci_msg_unpack_pdsch(&cell, &dl_sf, &cfg, &dci_msg, &dci_rx) == ISRRAN_SUCCESS);

  // To string
  char str[128];
  isrran_dci_dl_info(&dci_tx, str, sizeof(str));
  printf("Tx: %s\n", str);
  isrran_dci_dl_info(&dci_rx, str, sizeof(str));
  printf("Rx: %s\n", str);

  // Assert
  TESTASSERT(memcmp(&dci_tx, &dci_rx, isrran_dci_format_sizeof(&cell, &dl_sf, &cfg, dci_tx.format)) == 0);

  return ISRRAN_SUCCESS;
}

int main(int argc, char** argv)
{
  if (test_pdcch_orders() != ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  printf("Success!\n");

  return ISRRAN_SUCCESS;
}