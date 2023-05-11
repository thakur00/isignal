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

#include "isrue/hdr/stack/mac_nr/demux_nr.h"
#include "isrran/common/buffer_pool.h"
#include "isrran/common/string_helpers.h"
#include "isrran/interfaces/ue_rlc_interfaces.h"

namespace isrue {

demux_nr::demux_nr(isrlog::basic_logger& logger_) : logger(logger_) {}

demux_nr::~demux_nr() {}

int32_t demux_nr::init(rlc_interface_mac* rlc_, phy_interface_mac_nr* phy_)
{
  rlc = rlc_;
  phy = phy_;
  return ISRRAN_SUCCESS;
}

uint64_t demux_nr::get_received_crueid()
{
  return received_crueid;
}

// Enqueues PDU and returns quickly
void demux_nr::push_pdu(isrran::unique_byte_buffer_t pdu, uint32_t tti)
{
  pdu_queue.push(std::move(pdu));
}

void demux_nr::push_bcch(isrran::unique_byte_buffer_t pdu)
{
  bcch_queue.push(std::move(pdu));
}

/* Demultiplexing of MAC PDU associated with a Temporal C-RNTI. The PDU will
 * remain in buffer until demultiplex_pending_pdu() is called.
 * This features is provided to enable the Random Access Procedure to decide
 * whether the PDU shall pass to upper layers or not, which depends on the
 * Contention Resolution result.
 *
 * Warning: this function does some processing here assuming ACK deadline is not an
 * issue here because Temp C-RNTI messages have small payloads
 */
void demux_nr::push_pdu_temp_crnti(isrran::unique_byte_buffer_t pdu, uint32_t tti)
{
  received_crueid = 0;
  handle_pdu(rx_pdu_tcrnti, std::move(pdu));
}

void demux_nr::process_pdus()
{
  // Handle first BCCH
  while (not bcch_queue.empty()) {
    isrran::unique_byte_buffer_t pdu = bcch_queue.wait_pop();
    logger.debug(pdu->msg, pdu->N_bytes, "Handling MAC BCCH PDU (%d B)", pdu->N_bytes);
    rlc->write_pdu_bcch_dlsch(pdu->msg, pdu->N_bytes);
  }
  // Then user PDUs
  while (not pdu_queue.empty()) {
    isrran::unique_byte_buffer_t pdu = pdu_queue.wait_pop();
    handle_pdu(rx_pdu, std::move(pdu));
  }
}

/// Handling of DLSCH PDUs only
void demux_nr::handle_pdu(isrran::mac_sch_pdu_nr& pdu_buffer, isrran::unique_byte_buffer_t pdu)
{
  logger.debug(pdu->msg, pdu->N_bytes, "Handling MAC PDU (%d B)", pdu->N_bytes);

  pdu_buffer.init_rx();
  if (pdu_buffer.unpack(pdu->msg, pdu->N_bytes) != ISRRAN_SUCCESS) {
    return;
  }

  if (logger.info.enabled()) {
    fmt::memory_buffer str_buffer;
    pdu_buffer.to_string(str_buffer);
    logger.info("%s", isrran::to_c_str(str_buffer));
  }

  for (uint32_t i = 0; i < pdu_buffer.get_num_subpdus(); ++i) {
    isrran::mac_sch_subpdu_nr subpdu = pdu_buffer.get_subpdu(i);
    logger.debug("Handling subPDU %d/%d: rnti=0x%x lcid=%d, sdu_len=%d",
                 i + 1,
                 pdu_buffer.get_num_subpdus(),
                 subpdu.get_c_rnti(),
                 subpdu.get_lcid(),
                 subpdu.get_sdu_length());

    // Handle Timing Advance CE
    switch (subpdu.get_lcid()) {
      case isrran::mac_sch_subpdu_nr::nr_lcid_sch_t::DRX_CMD:
        logger.info("DRX CE not implemented.");
        break;
      case isrran::mac_sch_subpdu_nr::nr_lcid_sch_t::TA_CMD:
        logger.info("Received TA=%d.", subpdu.get_ta().ta_command);
        phy->set_timeadv(0, subpdu.get_ta().ta_command);
        break;
      case isrran::mac_sch_subpdu_nr::nr_lcid_sch_t::CON_RES_ID:
        received_crueid = subpdu.get_ue_con_res_id_ce_packed();
        logger.info("Received Contention Resolution ID 0x%lx", subpdu.get_ue_con_res_id_ce_packed());
        break;
      default:
        if (subpdu.is_sdu()) {
          rlc->write_pdu(subpdu.get_lcid(), subpdu.get_sdu(), subpdu.get_sdu_length());
        }
    }
  }
}

} // namespace isrue
