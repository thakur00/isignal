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

#ifndef ISRRAN_NGAP_PCAP_H
#define ISRRAN_NGAP_PCAP_H

#include "isrran/common/pcap.h"
#include <string>

namespace isrran {

class ngap_pcap
{
public:
  ngap_pcap();
  ~ngap_pcap();
  ngap_pcap(const ngap_pcap& other) = delete;
  ngap_pcap& operator=(const ngap_pcap& other) = delete;
  ngap_pcap(ngap_pcap&& other)                 = delete;
  ngap_pcap& operator=(ngap_pcap&& other) = delete;

  void enable();
  void open(const char* filename_);
  void close();
  void write_ngap(uint8_t* pdu, uint32_t pdu_len_bytes);

private:
  bool        enable_write = false;
  std::string filename;
  FILE*       pcap_file            = nullptr;
  int         emergency_handler_id = -1;
};

} // namespace isrran

#endif // ISRRAN_NGAP_PCAP_H
