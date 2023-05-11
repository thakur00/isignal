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

#ifndef ISRUE_NAS_5G_PROCEDURES_H_
#define ISRUE_NAS_5G_PROCEDURES_H_

#include "isrue/hdr/stack/upper/nas_5g.h"

namespace isrue {

/**
 * @brief 5G NAS registration procedure
 *
 * Specified in 24 501 V16.7.0
 * 5GMM specific procedures
 * 5.5.1 Registration procedure
 */
class nas_5g::registration_procedure
{
public:
  explicit registration_procedure(nas_5g_interface_procedures* parent_nas_);
  isrran::proc_outcome_t init();
  isrran::proc_outcome_t step();
  isrran::proc_outcome_t then();
  static const char*     name() { return "Registration Procedure"; }

private:
  nas_5g_interface_procedures* parent_nas;
};

/**
 * @brief 5G NAS (5GSM) UE-requested PDU session establishment procedure
 *
 * Specified in 24 501 V16.7.0
 * UE-requested 5GSM procedures
 * 6.4.1 UE-requested PDU session establishment procedure
 */
class nas_5g::pdu_session_establishment_procedure
{
public:
  explicit pdu_session_establishment_procedure(nas_5g_interface_procedures* parent_nas_, isrlog::basic_logger& logger_);
  isrran::proc_outcome_t init(const uint16_t pdu_session_id, const pdu_session_cfg_t& pdu_session);
  isrran::proc_outcome_t react(const isrran::nas_5g::pdu_session_establishment_accept_t& pdu_session_est_accept);
  isrran::proc_outcome_t react(const isrran::nas_5g::pdu_session_establishment_reject_t& pdu_session_est_reject);
  isrran::proc_outcome_t step();
  isrran::proc_outcome_t then();
  static const char*     name() { return "PDU Session Establishment Procedure"; }

private:
  isrlog::basic_logger&        logger;
  nas_5g_interface_procedures* parent_nas;
  uint32_t                     transaction_identity = 0;
  uint16_t                     pdu_session_id       = 0;
};

} // namespace isrue

#endif // ISRUE_NAS_5G_PROCEDURES_H_