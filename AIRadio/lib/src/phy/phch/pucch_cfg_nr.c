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

#include "isrran/phy/phch/pucch_cfg_nr.h"
#include "isrran/phy/common/phy_common_nr.h"
#include "isrran/phy/utils/debug.h"

static int pucch_nr_cfg_format0_resource_valid(const isrran_pucch_nr_resource_t* resource)
{
  if (resource->format != ISRRAN_PUCCH_NR_FORMAT_0) {
    ERROR("Invalid format (%d)", resource->format);
    return ISRRAN_ERROR;
  }

  if (resource->nof_symbols < ISRRAN_PUCCH_NR_FORMAT0_MIN_NSYMB ||
      resource->nof_symbols > ISRRAN_PUCCH_NR_FORMAT0_MAX_NSYMB) {
    ERROR("Invalid number of symbols (%d)", resource->nof_symbols);
    return ISRRAN_ERROR;
  }

  if (resource->initial_cyclic_shift > ISRRAN_PUCCH_NR_FORMAT0_MAX_CS) {
    ERROR("Invalid initial cyclic shift (%d)", resource->initial_cyclic_shift);
    return ISRRAN_ERROR;
  }

  if (resource->start_symbol_idx > ISRRAN_PUCCH_NR_FORMAT0_MAX_STARTSYMB) {
    ERROR("Invalid initial start symbol idx (%d)", resource->start_symbol_idx);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int pucch_nr_cfg_format1_resource_valid(const isrran_pucch_nr_resource_t* resource)
{
  if (resource->format != ISRRAN_PUCCH_NR_FORMAT_1) {
    ERROR("Invalid format (%d)", resource->format);
    return ISRRAN_ERROR;
  }

  if (resource->nof_symbols < ISRRAN_PUCCH_NR_FORMAT1_MIN_NSYMB ||
      resource->nof_symbols > ISRRAN_PUCCH_NR_FORMAT1_MAX_NSYMB) {
    ERROR("Invalid number of symbols (%d)", resource->nof_symbols);
    return ISRRAN_ERROR;
  }

  if (resource->initial_cyclic_shift > ISRRAN_PUCCH_NR_FORMAT1_MAX_CS) {
    ERROR("Invalid initial cyclic shift (%d)", resource->initial_cyclic_shift);
    return ISRRAN_ERROR;
  }

  if (resource->start_symbol_idx > ISRRAN_PUCCH_NR_FORMAT1_MAX_STARTSYMB) {
    ERROR("Invalid initial start symbol idx (%d)", resource->start_symbol_idx);
    return ISRRAN_ERROR;
  }

  if (resource->time_domain_occ > ISRRAN_PUCCH_NR_FORMAT1_MAX_TOCC) {
    ERROR("Invalid time domain occ (%d)", resource->time_domain_occ);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int pucch_nr_cfg_format2_resource_valid(const isrran_pucch_nr_resource_t* resource)
{
  if (resource->format != ISRRAN_PUCCH_NR_FORMAT_2) {
    ERROR("Invalid format (%d)", resource->format);
    return ISRRAN_ERROR;
  }

  if (resource->nof_symbols < ISRRAN_PUCCH_NR_FORMAT2_MIN_NSYMB ||
      resource->nof_symbols > ISRRAN_PUCCH_NR_FORMAT2_MAX_NSYMB) {
    ERROR("Invalid number of symbols (%d)", resource->nof_symbols);
    return ISRRAN_ERROR;
  }

  if (resource->nof_prb < ISRRAN_PUCCH_NR_FORMAT2_MIN_NPRB || resource->nof_prb > ISRRAN_PUCCH_NR_FORMAT2_MAX_NPRB) {
    ERROR("Invalid number of prb (%d)", resource->nof_prb);
    return ISRRAN_ERROR;
  }

  if (resource->start_symbol_idx > ISRRAN_PUCCH_NR_FORMAT2_MAX_STARTSYMB) {
    ERROR("Invalid initial start symbol idx (%d)", resource->start_symbol_idx);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int pucch_nr_cfg_format3_resource_valid(const isrran_pucch_nr_resource_t* resource)
{
  if (resource->format != ISRRAN_PUCCH_NR_FORMAT_3) {
    ERROR("Invalid format (%d)", resource->format);
    return ISRRAN_ERROR;
  }

  if (resource->nof_symbols < ISRRAN_PUCCH_NR_FORMAT3_MIN_NSYMB ||
      resource->nof_symbols > ISRRAN_PUCCH_NR_FORMAT3_MAX_NSYMB) {
    ERROR("Invalid number of symbols (%d)", resource->nof_symbols);
    return ISRRAN_ERROR;
  }

  if (resource->nof_prb < ISRRAN_PUCCH_NR_FORMAT3_MIN_NPRB || resource->nof_prb > ISRRAN_PUCCH_NR_FORMAT3_MAX_NPRB) {
    ERROR("Invalid number of prb (%d)", resource->nof_prb);
    return ISRRAN_ERROR;
  }

  if (resource->start_symbol_idx > ISRRAN_PUCCH_NR_FORMAT3_MAX_STARTSYMB) {
    ERROR("Invalid initial start symbol idx (%d)", resource->start_symbol_idx);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int pucch_nr_cfg_format4_resource_valid(const isrran_pucch_nr_resource_t* resource)
{
  if (resource->format != ISRRAN_PUCCH_NR_FORMAT_4) {
    ERROR("Invalid format (%d)", resource->format);
    return ISRRAN_ERROR;
  }

  if (resource->nof_symbols < ISRRAN_PUCCH_NR_FORMAT4_MIN_NSYMB ||
      resource->nof_symbols > ISRRAN_PUCCH_NR_FORMAT4_MAX_NSYMB) {
    ERROR("Invalid number of symbols (%d)", resource->nof_symbols);
    return ISRRAN_ERROR;
  }

  if (resource->start_symbol_idx > ISRRAN_PUCCH_NR_FORMAT4_MAX_STARTSYMB) {
    ERROR("Invalid initial start symbol idx (%d)", resource->start_symbol_idx);
    return ISRRAN_ERROR;
  }

  if (resource->occ_lenth != 2 && resource->occ_lenth != 4) {
    ERROR("Invalid OCC length (%d)", resource->occ_lenth);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

int isrran_pucch_nr_cfg_resource_valid(const isrran_pucch_nr_resource_t* resource)
{
  // Check pointer
  if (resource == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  if (resource->starting_prb > ISRRAN_MAX_NRE_NR - 1) {
    return ISRRAN_ERROR;
  }

  // Frequency hopping is only possible with Format 1
  if (resource->intra_slot_hopping && resource->format != ISRRAN_PUCCH_NR_FORMAT_1) {
    ERROR("Intra-slot hopping is not implemented");
    return ISRRAN_ERROR;
  }

  if (resource->second_hop_prb > ISRRAN_MAX_NRE_NR - 1) {
    return ISRRAN_ERROR;
  }

  if (resource->max_code_rate > ISRRAN_PUCCH_NR_MAX_CODE_RATE) {
    ERROR("Invalid maximum code rate (%d)", resource->max_code_rate);
    return ISRRAN_ERROR;
  }

  switch (resource->format) {
    case ISRRAN_PUCCH_NR_FORMAT_0:
      return pucch_nr_cfg_format0_resource_valid(resource);
    case ISRRAN_PUCCH_NR_FORMAT_1:
      return pucch_nr_cfg_format1_resource_valid(resource);
    case ISRRAN_PUCCH_NR_FORMAT_2:
      return pucch_nr_cfg_format2_resource_valid(resource);
    case ISRRAN_PUCCH_NR_FORMAT_3:
      return pucch_nr_cfg_format3_resource_valid(resource);
    case ISRRAN_PUCCH_NR_FORMAT_4:
      return pucch_nr_cfg_format4_resource_valid(resource);
    case ISRRAN_PUCCH_NR_FORMAT_ERROR:
    default:
      ERROR("Invalid case");
      break;
  }

  return ISRRAN_ERROR;
}
