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

#include "isrran/phy/common/zc_sequence.h"
#include "isrran/phy/common/phy_common.h"
#include "isrran/phy/utils/debug.h"
#include "isrran/phy/utils/primes.h"
#include "isrran/phy/utils/vector.h"
#include <assert.h>
#include <complex.h>

#define NOF_ZC_SEQ 30

// Phi values for M_sc=12 Table 5.5.1.2-1 in TS 36.211
static const float zc_sequence_lte_phi_M_sc_12[NOF_ZC_SEQ][12] = {
    {-1, 1, 3, -3, 3, 3, 1, 1, 3, 1, -3, 3},      {1, 1, 3, 3, 3, -1, 1, -3, -3, 1, -3, 3},
    {1, 1, -3, -3, -3, -1, -3, -3, 1, -3, 1, -1}, {-1, 1, 1, 1, 1, -1, -3, -3, 1, -3, 3, -1},
    {-1, 3, 1, -1, 1, -1, -3, -1, 1, -1, 1, 3},   {1, -3, 3, -1, -1, 1, 1, -1, -1, 3, -3, 1},
    {-1, 3, -3, -3, -3, 3, 1, -1, 3, 3, -3, 1},   {-3, -1, -1, -1, 1, -3, 3, -1, 1, -3, 3, 1},
    {1, -3, 3, 1, -1, -1, -1, 1, 1, 3, -1, 1},    {1, -3, -1, 3, 3, -1, -3, 1, 1, 1, 1, 1},
    {-1, 3, -1, 1, 1, -3, -3, -1, -3, -3, 3, -1}, {3, 1, -1, -1, 3, 3, -3, 1, 3, 1, 3, 3},
    {1, -3, 1, 1, -3, 1, 1, 1, -3, -3, -3, 1},    {3, 3, -3, 3, -3, 1, 1, 3, -1, -3, 3, 3},
    {-3, 1, -1, -3, -1, 3, 1, 3, 3, 3, -1, 1},    {3, -1, 1, -3, -1, -1, 1, 1, 3, 1, -1, -3},
    {1, 3, 1, -1, 1, 3, 3, 3, -1, -1, 3, -1},     {-3, 1, 1, 3, -3, 3, -3, -3, 3, 1, 3, -1},
    {-3, 3, 1, 1, -3, 1, -3, -3, -1, -1, 1, -3},  {-1, 3, 1, 3, 1, -1, -1, 3, -3, -1, -3, -1},
    {-1, -3, 1, 1, 1, 1, 3, 1, -1, 1, -3, -1},    {-1, 3, -1, 1, -3, -3, -3, -3, -3, 1, -1, -3},
    {1, 1, -3, -3, -3, -3, -1, 3, -3, 1, -3, 3},  {1, 1, -1, -3, -1, -3, 1, -1, 1, 3, -1, 1},
    {1, 1, 3, 1, 3, 3, -1, 1, -1, -3, -3, 1},     {1, -3, 3, 3, 1, 3, 3, 1, -3, -1, -1, 3},
    {1, 3, -3, -3, 3, -3, 1, -1, -1, 3, -1, -3},  {-3, -1, -3, -1, -3, 3, 1, -1, 1, 3, -3, -3},
    {-1, 3, -3, 3, -1, 3, 3, -3, 3, 3, -1, -1},   {3, -3, -3, -1, -1, -3, -1, 3, -3, 3, 1, -1}};

// Phi values for M_sc=24 Table 5.5.1.2-2 in TS 36.211
static const float zc_sequence_lte_phi_M_sc_24[NOF_ZC_SEQ][24] = {
    {-1, 3, 1, -3, 3, -1, 1, 3, -3, 3, 1, 3, -3, 3, 1, 1, -1, 1, 3, -3, 3, -3, -1, -3},
    {-3, 3, -3, -3, -3, 1, -3, -3, 3, -1, 1, 1, 1, 3, 1, -1, 3, -3, -3, 1, 3, 1, 1, -3},
    {3, -1, 3, 3, 1, 1, -3, 3, 3, 3, 3, 1, -1, 3, -1, 1, 1, -1, -3, -1, -1, 1, 3, 3},
    {-1, -3, 1, 1, 3, -3, 1, 1, -3, -1, -1, 1, 3, 1, 3, 1, -1, 3, 1, 1, -3, -1, -3, -1},
    {-1, -1, -1, -3, -3, -1, 1, 1, 3, 3, -1, 3, -1, 1, -1, -3, 1, -1, -3, -3, 1, -3, -1, -1},
    {-3, 1, 1, 3, -1, 1, 3, 1, -3, 1, -3, 1, 1, -1, -1, 3, -1, -3, 3, -3, -3, -3, 1, 1},
    {1, 1, -1, -1, 3, -3, -3, 3, -3, 1, -1, -1, 1, -1, 1, 1, -1, -3, -1, 1, -1, 3, -1, -3},
    {-3, 3, 3, -1, -1, -3, -1, 3, 1, 3, 1, 3, 1, 1, -1, 3, 1, -1, 1, 3, -3, -1, -1, 1},
    {-3, 1, 3, -3, 1, -1, -3, 3, -3, 3, -1, -1, -1, -1, 1, -3, -3, -3, 1, -3, -3, -3, 1, -3},
    {1, 1, -3, 3, 3, -1, -3, -1, 3, -3, 3, 3, 3, -1, 1, 1, -3, 1, -1, 1, 1, -3, 1, 1},
    {-1, 1, -3, -3, 3, -1, 3, -1, -1, -3, -3, -3, -1, -3, -3, 1, -1, 1, 3, 3, -1, 1, -1, 3},
    {1, 3, 3, -3, -3, 1, 3, 1, -1, -3, -3, -3, 3, 3, -3, 3, 3, -1, -3, 3, -1, 1, -3, 1},
    {1, 3, 3, 1, 1, 1, -1, -1, 1, -3, 3, -1, 1, 1, -3, 3, 3, -1, -3, 3, -3, -1, -3, -1},
    {3, -1, -1, -1, -1, -3, -1, 3, 3, 1, -1, 1, 3, 3, 3, -1, 1, 1, -3, 1, 3, -1, -3, 3},
    {-3, -3, 3, 1, 3, 1, -3, 3, 1, 3, 1, 1, 3, 3, -1, -1, -3, 1, -3, -1, 3, 1, 1, 3},
    {-1, -1, 1, -3, 1, 3, -3, 1, -1, -3, -1, 3, 1, 3, 1, -1, -3, -3, -1, -1, -3, -3, -3, -1},
    {-1, -3, 3, -1, -1, -1, -1, 1, 1, -3, 3, 1, 3, 3, 1, -1, 1, -3, 1, -3, 1, 1, -3, -1},
    {1, 3, -1, 3, 3, -1, -3, 1, -1, -3, 3, 3, 3, -1, 1, 1, 3, -1, -3, -1, 3, -1, -1, -1},
    {1, 1, 1, 1, 1, -1, 3, -1, -3, 1, 1, 3, -3, 1, -3, -1, 1, 1, -3, -3, 3, 1, 1, -3},
    {1, 3, 3, 1, -1, -3, 3, -1, 3, 3, 3, -3, 1, -1, 1, -1, -3, -1, 1, 3, -1, 3, -3, -3},
    {-1, -3, 3, -3, -3, -3, -1, -1, -3, -1, -3, 3, 1, 3, -3, -1, 3, -1, 1, -1, 3, -3, 1, -1},
    {-3, -3, 1, 1, -1, 1, -1, 1, -1, 3, 1, -3, -1, 1, -1, 1, -1, -1, 3, 3, -3, -1, 1, -3},
    {-3, -1, -3, 3, 1, -1, -3, -1, -3, -3, 3, -3, 3, -3, -1, 1, 3, 1, -3, 1, 3, 3, -1, -3},
    {-1, -1, -1, -1, 3, 3, 3, 1, 3, 3, -3, 1, 3, -1, 3, -1, 3, 3, -3, 3, 1, -1, 3, 3},
    {1, -1, 3, 3, -1, -3, 3, -3, -1, -1, 3, -1, 3, -1, -1, 1, 1, 1, 1, -1, -1, -3, -1, 3},
    {1, -1, 1, -1, 3, -1, 3, 1, 1, -1, -1, -3, 1, 1, -3, 1, 3, -3, 1, 1, -3, -3, -1, -1},
    {-3, -1, 1, 3, 1, 1, -3, -1, -1, -3, 3, -3, 3, 1, -3, 3, -3, 1, -1, 1, -3, 1, 1, 1},
    {-1, -3, 3, 3, 1, 1, 3, -1, -3, -1, -1, -1, 3, 1, -3, -3, -1, 3, -3, -1, -3, -1, -3, -1},
    {-1, -3, -1, -1, 1, -3, -1, -1, 1, -1, -3, 1, 1, -3, 1, -3, -3, 3, 1, 1, -1, 3, -1, -1},
    {1, 1, -1, -1, -3, -1, 3, -1, 3, -1, 1, 3, 1, -1, 3, 1, 3, -3, -3, 1, -1, -1, 1, 3}};

// Phi values for M_sc=12 Table 5.2.2.2-1 in TS 38.211
static const float zc_sequence_nr_phi_M_sc_6[NOF_ZC_SEQ][6] = {
    {-3, -1, 3, 3, -1, -3},  {-3, 3, -1, -1, 3, -3},  {-3, -3, -3, 3, 1, -3},  {1, 1, 1, 3, -1, -3},
    {1, 1, 1, -3, -1, 3},    {-3, 1, -1, -3, -3, -3}, {-3, 1, 3, -3, -3, -3},  {-3, -1, 1, -3, 1, -1},
    {-3, -1, -3, 1, -3, -3}, {-3, -3, 1, -3, 3, -3},  {-3, 1, 3, 1, -3, -3},   {-3, -1, -3, 1, 1, -3},
    {1, 1, 3, -1, -3, 3},    {1, 1, 3, 3, -1, 3},     {1, 1, 1, -3, 3, -1},    {1, 1, 1, -1, 3, -3},
    {-3, -1, -1, -1, 3, -1}, {-3, -3, -1, 1, -1, -3}, {-3, -3, -3, 1, -3, -1}, {-3, 1, 1, -3, -1, -3},
    {-3, 3, -3, 1, 1, -3},   {-3, 1, -3, -3, -3, -1}, {1, 1, -3, 3, 1, 3},     {1, 1, -3, -3, 1, -3},
    {1, 1, 3, -1, 3, 3},     {1, 1, -3, 1, 3, 3},     {1, 1, -1, -1, 3, -1},   {1, 1, -1, 3, -1, -1},
    {1, 1, -1, 3, -3, -1},   {1, 1, -3, 1, -1, -1}};

// Phi values for M_sc=12 Table 5.2.2.2-2 in TS 38.211
static const float zc_sequence_nr_phi_M_sc_12[NOF_ZC_SEQ][12] = {
    {-3, 1, -3, -3, -3, 3, -3, -1, 1, 1, 1, -3},  {-3, 3, 1, -3, 1, 3, -1, -1, 1, 3, 3, 3},
    {-3, 3, 3, 1, -3, 3, -1, 1, 3, -3, 3, -3},    {-3, -3, -1, 3, 3, 3, -3, 3, -3, 1, -1, -3},
    {-3, -1, -1, 1, 3, 1, 1, -1, 1, -1, -3, 1},   {-3, -3, 3, 1, -3, -3, -3, -1, 3, -1, 1, 3},
    {1, -1, 3, -1, -1, -1, -3, -1, 1, 1, 1, -3},  {-1, -3, 3, -1, -3, -3, -3, -1, 1, -1, 1, -3},
    {-3, -1, 3, 1, -3, -1, -3, 3, 1, 3, 3, 1},    {-3, -1, -1, -3, -3, -1, -3, 3, 1, 3, -1, -3},
    {-3, 3, -3, 3, 3, -3, -1, -1, 3, 3, 1, -3},   {-3, -1, -3, -1, -1, -3, 3, 3, -1, -1, 1, -3},
    {-3, -1, 3, -3, -3, -1, -3, 1, -1, -3, 3, 3}, {-3, 1, -1, -1, 3, 3, -3, -1, -1, -3, -1, -3},
    {1, 3, -3, 1, 3, 3, 3, 1, -1, 1, -1, 3},      {-3, 1, 3, -1, -1, -3, -3, -1, -1, 3, 1, -3},
    {-1, -1, -1, -1, 1, -3, -1, 3, 3, -1, -3, 1}, {-1, 1, 1, -1, 1, 3, 3, -1, -1, -3, 1, -3},
    {-3, 1, 3, 3, -1, -1, -3, 3, 3, -3, 3, -3},   {-3, -3, 3, -3, -1, 3, 3, 3, -1, -3, 1, -3},
    {3, 1, 3, 1, 3, -3, -1, 1, 3, 1, -1, -3},     {-3, 3, 1, 3, -3, 1, 1, 1, 1, 3, -3, 3},
    {-3, 3, 3, 3, -1, -3, -3, -1, -3, 1, 3, -3},  {3, -1, -3, 3, -3, -1, 3, 3, 3, -3, -1, -3},
    {-3, -1, 1, -3, 1, 3, 3, 3, -1, -3, 3, 3},    {-3, 3, 1, -1, 3, 3, -3, 1, -1, 1, -1, 1},
    {-1, 1, 3, -3, 1, -1, 1, -1, -1, -3, 1, -1},  {-3, -3, 3, 3, 3, -3, -1, 1, -3, 3, 1, -3},
    {1, -1, 3, 1, 1, -1, -1, -1, 1, 3, -3, 1},    {-3, 3, -3, 3, -3, -3, 3, -1, -1, 1, 3, -3}};

// Phi values for M_sc=18 Table 5.2.2.2-3 in TS 38.211
static const float zc_sequence_nr_phi_M_sc_18[NOF_ZC_SEQ][18] = {
    {-1, 3, -1, -3, 3, 1, -3, -1, 3, -3, -1, -1, 1, 1, 1, -1, -1, -1},
    {3, -3, 3, -1, 1, 3, -3, -1, -3, -3, -1, -3, 3, 1, -1, 3, -3, 3},
    {-3, 3, 1, -1, -1, 3, -3, -1, 1, 1, 1, 1, 1, -1, 3, -1, -3, -1},
    {-3, -3, 3, 3, 3, 1, -3, 1, 3, 3, 1, -3, -3, 3, -1, -3, -1, 1},
    {1, 1, -1, -1, -3, -1, 1, -3, -3, -3, 1, -3, -1, -1, 1, -1, 3, 1},
    {3, -3, 1, 1, 3, -1, 1, -1, -1, -3, 1, 1, -1, 3, 3, -3, 3, -1},
    {-3, 3, -1, 1, 3, 1, -3, -1, 1, 1, -3, 1, 3, 3, -1, -3, -3, -3},
    {1, 1, -3, 3, 3, 1, 3, -3, 3, -1, 1, 1, -1, 1, -3, -3, -1, 3},
    {-3, 1, -3, -3, 1, -3, -3, 3, 1, -3, -1, -3, -3, -3, -1, 1, 1, 3},
    {3, -1, 3, 1, -3, -3, -1, 1, -3, -3, 3, 3, 3, 1, 3, -3, 3, -3},
    {-3, -3, -3, 1, -3, 3, 1, 1, 3, -3, -3, 1, 3, -1, 3, -3, -3, 3},
    {-3, -3, 3, 3, 3, -1, -1, -3, -1, -1, -1, 3, 1, -3, -3, -1, 3, -1},
    {-3, -1, -3, -3, 1, 1, -1, -3, -1, -3, -1, -1, 3, 3, -1, 3, 1, 3},
    {1, 1, -3, -3, -3, -3, 1, 3, -3, 3, 3, 1, -3, -1, 3, -1, -3, 1},
    {-3, 3, -1, -3, -1, -3, 1, 1, -3, -3, -1, -1, 3, -3, 1, 3, 1, 1},
    {3, 1, -3, 1, -3, 3, 3, -1, -3, -3, -1, -3, -3, 3, -3, -1, 1, 3},
    {-3, -1, -3, -1, -3, 1, 3, -3, -1, 3, 3, 3, 1, -1, -3, 3, -1, -3},
    {-3, -1, 3, 3, -1, 3, -1, -3, -1, 1, -1, -3, -1, -1, -1, 3, 3, 1},
    {-3, 1, -3, -1, -1, 3, 1, -3, -3, -3, -1, -3, -3, 1, 1, 1, -1, -1},
    {3, 3, 3, -3, -1, -3, -1, 3, -1, 1, -1, -3, 1, -3, -3, -1, 3, 3},
    {-3, 1, 1, -3, 1, 1, 3, -3, -1, -3, -1, 3, -3, 3, -1, -1, -1, -3},
    {1, -3, -1, -3, 3, 3, -1, -3, 1, -3, -3, -1, -3, -1, 1, 3, 3, 3},
    {-3, -3, 1, -1, -1, 1, 1, -3, -1, 3, 3, 3, 3, -1, 3, 1, 3, 1},
    {3, -1, -3, 1, -3, -3, -3, 3, 3, -1, 1, -3, -1, 3, 1, 1, 3, 3},
    {3, -1, -1, 1, -3, -1, -3, -1, -3, -3, -1, -3, 1, 1, 1, -3, -3, 3},
    {-3, -3, 1, -3, 3, 3, 3, -1, 3, 1, 1, -3, -3, -3, 3, -3, -1, -1},
    {-3, -1, -1, -3, 1, -3, 3, -1, -1, -3, 3, 3, -3, -1, 3, -1, -1, -1},
    {-3, -3, 3, 3, -3, 1, 3, -1, -3, 1, -1, -3, 3, -3, -1, -1, -1, 3},
    {-1, -3, 1, -3, -3, -3, 1, 1, 3, 3, -3, 3, 3, -3, -1, 3, -3, 1},
    {-3, 3, 1, -1, -1, -1, -1, 1, -1, 3, 3, -3, -1, 1, 3, -1, 3, -1}};

// Phi values for M_sc=18 Table 5.2.2.2-3 in TS 38.211
static const float zc_sequence_nr_phi_M_sc_24[NOF_ZC_SEQ][24] = {
    {-1, -3, 3, -1, 3, 1, 3, -1, 1, -3, -1, -3, -1, 1, 3, -3, -1, -3, 3, 3, 3, -3, -3, -3},
    {-1, -3, 3, 1, 1, -3, 1, -3, -3, 1, -3, -1, -1, 3, -3, 3, 3, 3, -3, 1, 3, 3, -3, -3},
    {-1, -3, -3, 1, -1, -1, -3, 1, 3, -1, -3, -1, -1, -3, 1, 1, 3, 1, -3, -1, -1, 3, -3, -3},
    {1, -3, 3, -1, -3, -1, 3, 3, 1, -1, 1, 1, 3, -3, -1, -3, -3, -3, -1, 3, -3, -1, -3, -3},
    {-1, 3, -3, -3, -1, 3, -1, -1, 1, 3, 1, 3, -1, -1, -3, 1, 3, 1, -1, -3, 1, -1, -3, -3},
    {-3, -1, 1, -3, -3, 1, 1, -3, 3, -1, -1, -3, 1, 3, 1, -1, -3, -1, -3, 1, -3, -3, -3, -3},
    {-3, 3, 1, 3, -1, 1, -3, 1, -3, 1, -1, -3, -1, -3, -3, -3, -3, -1, -1, -1, 1, 1, -3, -3},
    {-3, 1, 3, -1, 1, -1, 3, -3, 3, -1, -3, -1, -3, 3, -1, -1, -1, -3, -1, -1, -3, 3, 3, -3},
    {-3, 1, -3, 3, -1, -1, -1, -3, 3, 1, -1, -3, -1, 1, 3, -1, 1, -1, 1, -3, -3, -3, -3, -3},
    {1, 1, -1, -3, -1, 1, 1, -3, 1, -1, 1, -3, 3, -3, -3, 3, -1, -3, 1, 3, -3, 1, -3, -3},
    {-3, -3, -3, -1, 3, -3, 3, 1, 3, 1, -3, -1, -1, -3, 1, 1, 3, 1, -1, -3, 3, 1, 3, -3},
    {-3, 3, -1, 3, 1, -1, -1, -1, 3, 3, 1, 1, 1, 3, 3, 1, -3, -3, -1, 1, -3, 1, 3, -3},
    {3, -3, 3, -1, -3, 1, 3, 1, -1, -1, -3, -1, 3, -3, 3, -1, -1, 3, 3, -3, -3, 3, -3, -3},
    {-3, 3, -1, 3, -1, 3, 3, 1, 1, -3, 1, 3, -3, 3, -3, -3, -1, 1, 3, -3, -1, -1, -3, -3},
    {-3, 1, -3, -1, -1, 3, 1, 3, -3, 1, -1, 3, 3, -1, -3, 3, -3, -1, -1, -3, -3, -3, 3, -3},
    {-3, -1, -1, -3, 1, -3, -3, -1, -1, 3, -1, 1, -1, 3, 1, -3, -1, 3, 1, 1, -1, -1, -3, -3},
    {-3, -3, 1, -1, 3, 3, -3, -1, 1, -1, -1, 1, 1, -1, -1, 3, -3, 1, -3, 1, -1, -1, -1, -3},
    {3, -1, 3, -1, 1, -3, 1, 1, -3, -3, 3, -3, -1, -1, -1, -1, -1, -3, -3, -1, 1, 1, -3, -3},
    {-3, 1, -3, 1, -3, -3, 1, -3, 1, -3, -3, -3, -3, -3, 1, -3, -3, 1, 1, -3, 1, 1, -3, -3},
    {-3, -3, 3, 3, 1, -1, -1, -1, 1, -3, -1, 1, -1, 3, -3, -1, -3, -1, -1, 1, -3, 3, -1, -3},
    {-3, -3, -1, -1, -1, -3, 1, -1, -3, -1, 3, -3, 1, -3, 3, -3, 3, 3, 1, -1, -1, 1, -3, -3},
    {3, -1, 1, -1, 3, -3, 1, 1, 3, -1, -3, 3, 1, -3, 3, -1, -1, -1, -1, 1, -3, -3, -3, -3},
    {-3, 1, -3, 3, -3, 1, -3, 3, 1, -1, -3, -1, -3, -3, -3, -3, 1, 3, -1, 1, 3, 3, 3, -3},
    {-3, -1, 1, -3, -1, -1, 1, 1, 1, 3, 3, -1, 1, -1, 1, -1, -1, -3, -3, -3, 3, 1, -1, -3},
    {-3, 3, -1, -3, -1, -1, -1, 3, -1, -1, 3, -3, -1, 3, -3, 3, -3, -1, 3, 1, 1, -1, -3, -3},
    {-3, 1, -1, -3, -3, -1, 1, -3, -1, -3, 1, 1, -1, 1, 1, 3, 3, 3, -1, 1, -1, 1, -1, -3},
    {-1, 3, -1, -1, 3, 3, -1, -1, -1, 3, -1, -3, 1, 3, 1, 1, -3, -3, -3, -1, -3, -1, -3, -3},
    {3, -3, -3, -1, 3, 3, -3, -1, 3, 1, 1, 1, 3, -1, 3, -3, -1, 3, -1, 3, 1, -1, -3, -3},
    {-3, 1, -3, 1, -3, 1, 1, 3, 1, -3, -3, -1, 1, 3, -1, -3, 3, 1, -1, -3, -3, -3, -3, -3},
    {3, -3, -1, 1, 3, -1, -1, -3, -1, 3, -1, -3, -1, -3, 3, -1, 3, 1, 1, -3, 3, -3, -3, -3}};

static void zc_sequence_lte_r_uv_arg_1prb(uint32_t u, cf_t* tmp_arg)
{
  assert(u < NOF_ZC_SEQ);
  isrran_vec_sc_prod_fcc(zc_sequence_lte_phi_M_sc_12[u], M_PI_4, tmp_arg, ISRRAN_NRE);
}

static void zc_sequence_lte_r_uv_arg_2prb(uint32_t u, cf_t* tmp_arg)
{
  assert(u < NOF_ZC_SEQ);
  isrran_vec_sc_prod_fcc(zc_sequence_lte_phi_M_sc_24[u], M_PI_4, tmp_arg, 2 * ISRRAN_NRE);
}

static void zc_sequence_nr_r_uv_arg_0dot5prb(uint32_t u, cf_t* tmp_arg)
{
  assert(u < NOF_ZC_SEQ);
  isrran_vec_sc_prod_fcc(zc_sequence_nr_phi_M_sc_6[u], M_PI_4, tmp_arg, ISRRAN_NRE / 2);
}

static void zc_sequence_nr_r_uv_arg_1prb(uint32_t u, cf_t* tmp_arg)
{
  assert(u < NOF_ZC_SEQ);
  isrran_vec_sc_prod_fcc(zc_sequence_nr_phi_M_sc_12[u], M_PI_4, tmp_arg, ISRRAN_NRE);
}

static void zc_sequence_nr_r_uv_arg_1dot5prb(uint32_t u, cf_t* tmp_arg)
{
  assert(u < NOF_ZC_SEQ);
  isrran_vec_sc_prod_fcc(zc_sequence_nr_phi_M_sc_18[u], M_PI_4, tmp_arg, (3 * ISRRAN_NRE) / 2);
}

static void zc_sequence_nr_r_uv_arg_2prb(uint32_t u, cf_t* tmp_arg)
{
  assert(u < NOF_ZC_SEQ);
  isrran_vec_sc_prod_fcc(zc_sequence_nr_phi_M_sc_24[u], M_PI_4, tmp_arg, 2 * ISRRAN_NRE);
}

static uint32_t zc_sequence_q(uint32_t u, uint32_t v, uint32_t N_sz)
{
  float q;
  float q_hat;
  float n_sz = (float)N_sz;

  q_hat = n_sz * (u + 1) / 31;
  if ((((uint32_t)(2 * q_hat)) % 2) == 0) {
    q = q_hat + 0.5 + v;
  } else {
    q = q_hat + 0.5 - v;
  }
  return (uint32_t)q;
}

// Common for LTE and NR
static void zc_sequence_r_uv_arg_mprb(uint32_t M_zc, uint32_t u, uint32_t v, cf_t* tmp_arg)
{
  int32_t N_sz = isrran_prime_lower_than(M_zc); // N_zc - Zadoff Chu Sequence Length
  if (N_sz > 0) {
    float q    = zc_sequence_q(u, v, N_sz);
    float n_sz = (float)N_sz;
    for (uint32_t i = 0; i < M_zc; i++) {
      float m    = (float)(i % N_sz);
      tmp_arg[i] = -M_PI * q * m * (m + 1) / n_sz;
    }
  }
}

static int zc_sequence_lte_r_uv_arg(uint32_t M_zc, uint32_t u, uint32_t v, cf_t* tmp_arg)
{
  if (M_zc == 12) {
    zc_sequence_lte_r_uv_arg_1prb(u, tmp_arg);
  } else if (M_zc == 24) {
    zc_sequence_lte_r_uv_arg_2prb(u, tmp_arg);
  } else if (M_zc >= 36) {
    zc_sequence_r_uv_arg_mprb(M_zc, u, v, tmp_arg);
  } else {
    ERROR("Invalid M_zc (%d)", M_zc);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static int zc_sequence_nr_r_uv_arg(uint32_t M_zc, uint32_t u, uint32_t v, cf_t* tmp_arg)
{
  if (M_zc == 6) {
    zc_sequence_nr_r_uv_arg_0dot5prb(u, tmp_arg);
  } else if (M_zc == 12) {
    zc_sequence_nr_r_uv_arg_1prb(u, tmp_arg);
  } else if (M_zc == 18) {
    zc_sequence_nr_r_uv_arg_1dot5prb(u, tmp_arg);
  } else if (M_zc == 24) {
    zc_sequence_nr_r_uv_arg_2prb(u, tmp_arg);
  } else if (M_zc >= 36) {
    zc_sequence_r_uv_arg_mprb(M_zc, u, v, tmp_arg);
  } else {
    ERROR("Invalid M_zc (%d)", M_zc);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

static void zc_sequence_generate(uint32_t M_zc, float alpha, const cf_t* tmp_arg, cf_t* sequence)
{
  for (uint32_t i = 0; i < M_zc; i++) {
    sequence[i] = cexpf(I * (tmp_arg[i] + alpha * (cf_t)i));
  }
}

int isrran_zc_sequence_generate_lte(uint32_t u, uint32_t v, float alpha, uint32_t nof_prb, cf_t* sequence)
{
  // Check inputs
  if (sequence == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Check U and V
  if (u >= ISRRAN_ZC_SEQUENCE_NOF_GROUPS || v >= ISRRAN_ZC_SEQUENCE_NOF_BASE) {
    ERROR("Invalid u (%d) or v (%d)", u, v);
    return ISRRAN_ERROR_OUT_OF_BOUNDS;
  }

  // Calculate number of samples
  uint32_t M_zc = nof_prb * ISRRAN_NRE;

  // Calculate argument
  if (zc_sequence_lte_r_uv_arg(M_zc, u, v, sequence) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Do complex exponential and adjust amplitude
  zc_sequence_generate(M_zc, alpha, sequence, sequence);

  return ISRRAN_SUCCESS;
}

int isrran_zc_sequence_generate_nr(uint32_t u, uint32_t v, float alpha, uint32_t m, uint32_t delta, cf_t* sequence)
{
  // Check inputs
  if (sequence == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Check U and V
  if (u >= ISRRAN_ZC_SEQUENCE_NOF_GROUPS || v >= ISRRAN_ZC_SEQUENCE_NOF_BASE) {
    ERROR("Invalid u (%d) or v (%d)", u, v);
    return ISRRAN_ERROR_OUT_OF_BOUNDS;
  }

  // Calculate number of samples
  uint32_t M_zc = (m * ISRRAN_NRE) >> delta;

  // Calculate argument
  if (zc_sequence_nr_r_uv_arg(M_zc, u, v, sequence) < ISRRAN_SUCCESS) {
    return ISRRAN_ERROR;
  }

  // Do complex exponential and adjust amplitude
  zc_sequence_generate(M_zc, alpha, sequence, sequence);

  return ISRRAN_SUCCESS;
}

int isrran_zc_sequence_lut_init_nr(isrran_zc_sequence_lut_t* q,
                                   uint32_t                  m,
                                   uint32_t                  delta,
                                   float*                    alphas,
                                   uint32_t                  nof_alphas)
{
  if (q == NULL || alphas == NULL) {
    return ISRRAN_ERROR_INVALID_INPUTS;
  }

  // Set all structure to zero
  ISRRAN_MEM_ZERO(q, isrran_zc_sequence_lut_t, 1);

  // Calculate number of samples
  q->M_zc       = (m * ISRRAN_NRE) >> delta;
  q->nof_alphas = nof_alphas;

  for (uint32_t u = 0; u < ISRRAN_ZC_SEQUENCE_NOF_GROUPS; u++) {
    for (uint32_t v = 0; v < ISRRAN_ZC_SEQUENCE_NOF_BASE; v++) {
      // Allocate sequence
      q->sequence[u][v] = isrran_vec_cf_malloc(nof_alphas * q->M_zc);
      if (q->sequence[u][v] == NULL) {
        ERROR("Malloc");
        return ISRRAN_ERROR;
      }

      // Generate a sequence for each alpha
      for (uint32_t alpha_idx = 0; alpha_idx < nof_alphas; alpha_idx++) {
        if (isrran_zc_sequence_generate_nr(u, v, alphas[alpha_idx], m, delta, &q->sequence[u][v][alpha_idx * q->M_zc]) <
            ISRRAN_SUCCESS) {
          ERROR("Generating sequence");
          return ISRRAN_ERROR;
        }
      }
    }
  }

  return ISRRAN_SUCCESS;
}

void isrran_zc_sequence_lut_free(isrran_zc_sequence_lut_t* q)
{
  for (uint32_t u = 0; u < ISRRAN_ZC_SEQUENCE_NOF_GROUPS; u++) {
    for (uint32_t v = 0; v < ISRRAN_ZC_SEQUENCE_NOF_BASE; v++) {
      if (q->sequence[u][v] != NULL) {
        free(q->sequence[u][v]);
      }
    }
  }
  ISRRAN_MEM_ZERO(q, isrran_zc_sequence_lut_t, 1);
}

const cf_t* isrran_zc_sequence_lut_get(const isrran_zc_sequence_lut_t* q, uint32_t u, uint32_t v, uint32_t alpha_idx)
{
  if (q == NULL) {
    return NULL;
  }

  if (u >= ISRRAN_ZC_SEQUENCE_NOF_GROUPS) {
    return NULL;
  }

  if (v >= ISRRAN_ZC_SEQUENCE_NOF_BASE) {
    return NULL;
  }

  return &q->sequence[u][v][alpha_idx * q->M_zc];
}
