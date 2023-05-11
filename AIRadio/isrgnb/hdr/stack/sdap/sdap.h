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

#ifndef ISRENB_SDAP_H
#define ISRENB_SDAP_H

#include "isrran/common/buffer_pool.h"
#include "isrran/common/common.h"
#include "isrran/interfaces/gnb_interfaces.h"
#include "isrran/interfaces/ue_gw_interfaces.h"

namespace isrenb {

class sdap final : public sdap_interface_pdcp_nr, public sdap_interface_gtpu_nr
{
public:
  explicit sdap();
  bool init(pdcp_interface_sdap_nr* pdcp_, gtpu_interface_sdap_nr* gtpu_, isrue::gw_interface_pdcp* gw_);
  void stop();

  // Interface for PDCP
  void write_pdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t pdu) final;

  // Interface for GTPU
  void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t pdu) final;

private:
  gtpu_interface_sdap_nr*   m_gtpu = nullptr;
  pdcp_interface_sdap_nr*   m_pdcp = nullptr;
  isrue::gw_interface_pdcp* m_gw   = nullptr;

  // state
  bool running = false;
};

} // namespace isrenb

#endif // ISRENB_SDAP_H
