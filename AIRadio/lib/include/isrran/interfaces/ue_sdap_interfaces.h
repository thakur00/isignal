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

/******************************************************************************
 * File:        ue_sdap_interfaces.h
 * Description: Abstract base class interfaces for SDAP layer
 *****************************************************************************/

#ifndef ISRRAN_UE_SDAP_INTERFACES_H
#define ISRRAN_UE_SDAP_INTERFACES_H

/*****************************
 *      SDAP INTERFACES
 ****************************/
class sdap_interface_pdcp_nr
{
public:
  virtual void write_pdu(uint32_t lcid, isrran::unique_byte_buffer_t pdu) = 0;
};
class sdap_interface_gw_nr
{
public:
  virtual void write_sdu(uint32_t lcid, isrran::unique_byte_buffer_t pdu) = 0;
};

class sdap_interface_rrc
{
public:
  struct bearer_cfg_t {
    bool     is_data;
    bool     add_downlink_header;
    bool     add_uplink_header;
    uint32_t qfi;
  };
  virtual bool set_bearer_cfg(uint32_t lcid, const bearer_cfg_t& cfg) = 0;
};

#endif // ISRRAN_UE_SDAP_INTERFACES_H
