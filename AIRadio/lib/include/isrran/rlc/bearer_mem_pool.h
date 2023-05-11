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

#ifndef ISRRAN_BEARER_MEM_POOL_H
#define ISRRAN_BEARER_MEM_POOL_H

#include <cstddef>

namespace isrran {

// Allocation of objects in rnti-dedicated memory pool
void  reserve_rlc_memblocks(size_t nof_blocks);
void* allocate_rlc_bearer(std::size_t size);
void  deallocate_rlc_bearer(void* p);

} // namespace isrran

#endif // ISRRAN_BEARER_MEM_POOL_H
