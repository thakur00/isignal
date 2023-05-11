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

/******************************************************************************
 *  File:         mod.h
 *
 *  Description:  Modulation.
 *                Supports BPSK, QPSK, 16QAM, 64QAM and 256QAM.
 *
 *  Reference:    3GPP TS 36.211 version 10.0.0 Release 10 Sec. 7.1
 *****************************************************************************/

#ifndef ISRRAN_MOD_H
#define ISRRAN_MOD_H

#include <stdint.h>

#include "modem_table.h"
#include "isrran/config.h"

ISRRAN_API int isrran_mod_modulate(const isrran_modem_table_t* table, uint8_t* bits, cf_t* symbols, uint32_t nbits);

ISRRAN_API int
isrran_mod_modulate_bytes(const isrran_modem_table_t* q, const uint8_t* bits, cf_t* symbols, uint32_t nbits);

#endif // ISRRAN_MOD_H
