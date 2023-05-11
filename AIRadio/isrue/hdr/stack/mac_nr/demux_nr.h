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

#ifndef ISRRAN_DEMUX_NR_H
#define ISRRAN_DEMUX_NR_H

#include "mac_nr_interfaces.h"
#include "isrran/common/block_queue.h"
#include "isrran/interfaces/ue_nr_interfaces.h"
#include "isrran/interfaces/ue_rlc_interfaces.h"

namespace isrue {

/**
 * @brief Logical Channel Demultiplexing and MAC CE dissassemble according to TS 38.321
 *
 * Currently only SDU handling for SCH PDU processing is implemented.
 * Downlink CE are parsed but not handled.
 *
 * PDUs can be pushed by multiple HARQ processes in parallel.
 * Handling of the PDUs is done from Stack thread which reads the enqueued PDUs
 * from the thread-safe queue.
 */
class demux_nr : public demux_interface_harq_nr
{
public:
  demux_nr(isrlog::basic_logger& logger_);
  ~demux_nr();

  int32_t init(rlc_interface_mac* rlc_, phy_interface_mac_nr* phy_);

  void process_pdus(); /// Called by MAC to process received PDUs

  // HARQ interface
  void     push_bcch(isrran::unique_byte_buffer_t pdu);
  void     push_pdu(isrran::unique_byte_buffer_t pdu, uint32_t tti);
  void     push_pdu_temp_crnti(isrran::unique_byte_buffer_t pdu, uint32_t tti);
  uint64_t get_received_crueid();

private:
  // internal helpers
  void handle_pdu(isrran::mac_sch_pdu_nr& pdu_buffer, isrran::unique_byte_buffer_t pdu);

  isrlog::basic_logger& logger;
  rlc_interface_mac*    rlc = nullptr;
  phy_interface_mac_nr* phy = nullptr;

  uint64_t received_crueid = 0;

  ///< currently only DCH & BCH PDUs supported (add PCH, etc)
  isrran::block_queue<isrran::unique_byte_buffer_t> pdu_queue;
  isrran::block_queue<isrran::unique_byte_buffer_t> bcch_queue;

  isrran::mac_sch_pdu_nr rx_pdu;
  isrran::mac_sch_pdu_nr rx_pdu_tcrnti;
};

} // namespace isrue

#endif // ISRRAN_DEMUX_NR_H
