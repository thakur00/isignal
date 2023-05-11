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

#ifndef ISRRAN_SLIV_H
#define ISRRAN_SLIV_H

#include "isrran/config.h"
#include <inttypes.h>

ISRRAN_API void isrran_sliv_to_s_and_l(uint32_t N, uint32_t v, uint32_t* S, uint32_t* L);

ISRRAN_API uint32_t isrran_sliv_from_s_and_l(uint32_t N, uint32_t S, uint32_t L);

#endif // ISRRAN_SLIV_H
