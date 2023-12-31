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

#ifndef ISRENB_CFG_PARSER_H
#define ISRENB_CFG_PARSER_H

#include "enb.h"

namespace isrenb {
class cfg_parser
{
public:
  void parse_sibs(all_args_t* args, rrc_cfg_t* rrc_cfg, phy_cfg_t* phy_config_common);
};

} // namespace isrenb

#endif // CFG_PARSER_H
