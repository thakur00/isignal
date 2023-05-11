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

/*! \brief Common types for TTCN3 test systems
 *
 */

#ifndef ISRUE_TTCN3_COMMON_H
#define ISRUE_TTCN3_COMMON_H

#include <memory>

const static uint32_t                    RX_BUF_SIZE = 1024 * 1024;
typedef std::array<uint8_t, RX_BUF_SIZE> byte_array_t;
typedef std::unique_ptr<byte_array_t>    unique_byte_array_t;

#endif // ISRUE_TTCN3_COMMON_H
