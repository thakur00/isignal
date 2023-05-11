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

#ifndef ISRENB_NGAP_INTERFACES_H
#define ISRENB_NGAP_INTERFACES_H

#include "isrran/asn1/ngap_utils.h"

namespace isrenb {
class ngap_interface_ngap_proc
{
public:
  virtual bool send_initial_ctxt_setup_response() = 0;
  virtual bool
               send_pdu_session_resource_setup_response(uint16_t                                    pdu_session_id,
                                                        uint32_t                                    teid_out,
                                                        asn1::bounded_bitstring<1, 160, true, true> transport_layer_address) = 0;
  virtual bool send_ue_ctxt_release_complete() = 0;
};
} // namespace isrenb

#endif