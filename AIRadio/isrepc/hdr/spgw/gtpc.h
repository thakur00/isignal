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
#ifndef ISREPC_GTPC_H
#define ISREPC_GTPC_H

#include "isrepc/hdr/spgw/spgw.h"
#include "isrran/asn1/gtpc.h"
#include "isrran/common/standard_streams.h"
#include "isrran/interfaces/epc_interfaces.h"
#include "isrran/isrlog/isrlog.h"

#include <set>
#include <sys/socket.h>
#include <sys/un.h>

namespace isrepc {

class spgw::gtpc : public gtpc_interface_gtpu
{
public:
  gtpc();
  virtual ~gtpc();
  int init(spgw_args_t* args, spgw* spgw, gtpu_interface_gtpc* gtpu, const std::map<std::string, uint64_t>& ip_to_imsi);
  void stop();

  int init_s11(spgw_args_t* args);
  int init_ue_ip(spgw_args_t* args, const std::map<std::string, uint64_t>& ip_to_imsi);

  int       get_s11();
  uint64_t  get_new_ctrl_teid();
  uint64_t  get_new_user_teid();
  in_addr_t get_new_ue_ipv4(uint64_t imsi);

  void handle_s11_pdu(isrran::byte_buffer_t* msg);
  bool send_s11_pdu(const isrran::gtpc_pdu& pdu);

  void handle_create_session_request(const isrran::gtpc_create_session_request& cs_req);
  void handle_modify_bearer_request(const isrran::gtpc_header&                mb_req_hdr,
                                    const isrran::gtpc_modify_bearer_request& mb_req);
  void handle_delete_session_request(const isrran::gtpc_header&                 header,
                                     const isrran::gtpc_delete_session_request& del_req);
  void handle_release_access_bearers_request(const isrran::gtpc_header&                         header,
                                             const isrran::gtpc_release_access_bearers_request& rel_req);
  void
       handle_downlink_data_notification_acknowledge(const isrran::gtpc_header&                                 header,
                                                     const isrran::gtpc_downlink_data_notification_acknowledge& not_ack);
  void handle_downlink_data_notification_failure_indication(
      const isrran::gtpc_header&                                        header,
      const isrran::gtpc_downlink_data_notification_failure_indication& not_fail);

  virtual bool queue_downlink_packet(uint32_t spgw_ctr_teid, isrran::unique_byte_buffer_t msg) override;
  virtual bool send_downlink_data_notification(uint32_t spgw_ctr_teid) override;

  spgw_tunnel_ctx_t* create_gtpc_ctx(const isrran::gtpc_create_session_request& cs_req);
  bool               delete_gtpc_ctx(uint32_t ctrl_teid);

  bool free_all_queued_packets(spgw_tunnel_ctx_t* tunnel_ctx);

  spgw*                m_spgw;
  gtpu_interface_gtpc* m_gtpu;

  int                m_s11;
  struct sockaddr_un m_spgw_addr, m_mme_addr;

  uint32_t m_h_next_ue_ip;
  uint64_t m_next_ctrl_teid;
  uint64_t m_next_user_teid;
  uint32_t m_max_paging_queue;

  std::map<uint64_t, uint32_t> m_imsi_to_ctr_teid;           // IMSI to control TEID map. Important to check if UE
                                                             // is previously connected
  std::map<uint32_t, spgw_tunnel_ctx*> m_teid_to_tunnel_ctx; // Map control TEID to tunnel ctx. Usefull to get
                                                             // reply ctrl TEID, UE IP, etc.

  std::set<uint32_t>                 m_ue_ip_addr_pool;
  std::map<uint64_t, struct in_addr> m_imsi_to_ip;

  isrlog::basic_logger& m_logger = isrlog::fetch_basic_logger("SPGW GTPC");
};

inline int spgw::gtpc::get_s11()
{
  return m_s11;
}

inline uint64_t spgw::gtpc::get_new_ctrl_teid()
{
  return m_next_ctrl_teid++;
}

inline uint64_t spgw::gtpc::get_new_user_teid()
{
  return m_next_user_teid++;
}

} // namespace isrepc
#endif // ISREPC_SPGW_H
