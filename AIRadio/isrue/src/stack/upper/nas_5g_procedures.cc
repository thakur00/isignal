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

#include "isrue/hdr/stack/upper/nas_5g_procedures.h"
#include "isrran/common/standard_streams.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>

using namespace isrran;

namespace isrue {

nas_5g::registration_procedure::registration_procedure(nas_5g_interface_procedures* parent_nas_) :
  parent_nas(parent_nas_)
{}

isrran::proc_outcome_t nas_5g::registration_procedure::init()
{
  parent_nas->send_registration_request();
  return isrran::proc_outcome_t::yield;
}
isrran::proc_outcome_t nas_5g::registration_procedure::step()
{
  return isrran::proc_outcome_t::success;
}

// PDU Sessions Establishment Procedure
nas_5g::pdu_session_establishment_procedure::pdu_session_establishment_procedure(
    nas_5g_interface_procedures* parent_nas_,
    isrlog::basic_logger&        logger_) :
  logger(logger_), parent_nas(parent_nas_)
{}

isrran::proc_outcome_t nas_5g::pdu_session_establishment_procedure::init(const uint16_t           pdu_session_id_,
                                                                         const pdu_session_cfg_t& pdu_session_cfg)
{
  // Get PDU transaction identity
  transaction_identity = parent_nas->allocate_next_proc_trans_id();
  pdu_session_id       = pdu_session_id_;
  parent_nas->send_pdu_session_establishment_request(transaction_identity, pdu_session_id, pdu_session_cfg);

  return isrran::proc_outcome_t::yield;
}

isrran::proc_outcome_t nas_5g::pdu_session_establishment_procedure::react(
    const isrran::nas_5g::pdu_session_establishment_accept_t& pdu_session_est_accept)
{
  // TODO check the pdu session values
  if (pdu_session_est_accept.dnn_present == false) {
    logger.warning("Expected DNN in PDU session establishment accept");
    return proc_outcome_t::error;
  }
  if (pdu_session_est_accept.pdu_address_present == false) {
    logger.warning("Expected PDU Address in PDU session establishment accept");
    return proc_outcome_t::error;
  }
  if (parent_nas->add_pdu_session(pdu_session_id,
                                  pdu_session_est_accept.selected_pdu_session_type.pdu_session_type_value,
                                  pdu_session_est_accept.pdu_address) != ISRRAN_SUCCESS) {
    logger.warning("Adding PDU session failed\n");
    return isrran::proc_outcome_t::error;
  }
  return isrran::proc_outcome_t::success;
}

isrran::proc_outcome_t nas_5g::pdu_session_establishment_procedure::react(
    const isrran::nas_5g::pdu_session_establishment_reject_t& session_est_reject)
{
  logger.error("PDU Session Establishment Reject with cause: %s",
               session_est_reject.cause_5gsm.cause_value.to_string());
  isrran::console("PDU Session Establishment Reject with cause: %s\n",
                  session_est_reject.cause_5gsm.cause_value.to_string());
  return isrran::proc_outcome_t::error;
}

isrran::proc_outcome_t nas_5g::pdu_session_establishment_procedure::step()
{
  return isrran::proc_outcome_t::success;
}

} // namespace isrue