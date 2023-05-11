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

#include "isrran/common/common.h"
#include "isrran/common/interfaces_common.h"
#include "isrran/isrran.h"

#ifndef ISRRAN_ENB_INTERFACES_H
#define ISRRAN_ENB_INTERFACES_H

namespace isrenb {

class stack_interface_rrc
{
public:
  virtual void add_eps_bearer(uint16_t rnti, uint8_t eps_bearer_id, isrran::isrran_rat_t rat, uint32_t lcid) = 0;
  virtual void remove_eps_bearer(uint16_t rnti, uint8_t eps_bearer_id)                                       = 0;
  virtual void remove_eps_bearers(uint16_t rnti)                                                             = 0;
};

} // namespace isrenb

#endif // ISRRAN_ENB_INTERFACES_H
