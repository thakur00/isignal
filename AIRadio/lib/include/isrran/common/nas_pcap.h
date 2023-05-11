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

#ifndef ISRRAN_NAS_PCAP_H
#define ISRRAN_NAS_PCAP_H

#include "isrran/common/common.h"
#include "isrran/common/pcap.h"
#include <string>

namespace isrran {

class nas_pcap
{
public:
  nas_pcap();
  ~nas_pcap();
  void     enable();
  uint32_t open(std::string filename_, uint32_t ue_id = 0, isrran_rat_t rat_type = isrran_rat_t::lte);
  void     close();
  void     write_nas(uint8_t* pdu, uint32_t pdu_len_bytes);

private:
  bool        enable_write = false;
  std::string filename;
  FILE*       pcap_file            = nullptr;
  uint32_t    ue_id                = 0;
  int         emergency_handler_id = -1;
  void        pack_and_write(uint8_t* pdu, uint32_t pdu_len_bytes);
};

} // namespace isrran

#endif // ISRRAN_NAS_PCAP_H
