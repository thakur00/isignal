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

#include "isrenb/hdr/common/rnti_pool.h"
#include "isrenb/hdr/common/common_enb.h"
#include "isrenb/hdr/stack/mac/ue.h"
#include "isrenb/hdr/stack/rrc/rrc_endc.h"
#include "isrenb/hdr/stack/rrc/rrc_mobility.h"
#include "isrenb/hdr/stack/rrc/rrc_ue.h"
#include "isrran/adt/pool/circular_stack_pool.h"
#include "isrran/rlc/rlc.h"
#include "isrran/upper/pdcp.h"

namespace isrenb {

const static size_t UE_MEM_BLOCK_SIZE = 1024 + sizeof(ue) + sizeof(rrc::ue) + sizeof(rrc::ue::rrc_mobility) +
                                        sizeof(rrc::ue::rrc_endc) + sizeof(isrran::rlc) + sizeof(isrran::pdcp);

isrran::circular_stack_pool<ISRENB_MAX_UES>* get_rnti_pool()
{
  static std::unique_ptr<isrran::circular_stack_pool<ISRENB_MAX_UES> > pool(
      new isrran::circular_stack_pool<ISRENB_MAX_UES>(8, UE_MEM_BLOCK_SIZE, 4));
  return pool.get();
}

void reserve_rnti_memblocks(size_t nof_blocks)
{
  while (get_rnti_pool()->cache_size() < nof_blocks) {
    get_rnti_pool()->allocate_batch();
  }
}

void* allocate_rnti_dedicated_mem(uint16_t rnti, size_t size, size_t align)
{
  return get_rnti_pool()->allocate(rnti, size, align);
}
void deallocate_rnti_dedicated_mem(uint16_t rnti, void* ptr)
{
  get_rnti_pool()->deallocate(rnti, ptr);
}

} // namespace isrenb
