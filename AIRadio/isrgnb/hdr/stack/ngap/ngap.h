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
#ifndef ISRENB_NGAP_H
#define ISRENB_NGAP_H

#include "ngap_metrics.h"
#include "isrenb/hdr/common/common_enb.h"
#include "isrran/adt/circular_map.h"
#include "isrran/adt/optional.h"
#include "isrran/asn1/asn1_utils.h"
#include "isrran/asn1/ngap.h"
#include "isrran/common/bcd_helpers.h"
#include "isrran/common/buffer_pool.h"
#include "isrran/common/common.h"
#include "isrran/common/network_utils.h"
#include "isrran/common/ngap_pcap.h"
#include "isrran/common/stack_procedure.h"
#include "isrran/common/standard_streams.h"
#include "isrran/common/task_scheduler.h"
#include "isrran/common/threads.h"
#include "isrran/interfaces/enb_gtpu_interfaces.h"
#include "isrran/interfaces/gnb_ngap_interfaces.h"
#include "isrran/interfaces/gnb_rrc_nr_interfaces.h"
#include "isrran/isrlog/isrlog.h"
#include <iostream>
#include <unordered_map>

namespace isrenb {

class ngap final : public ngap_interface_rrc_nr
{
public:
  class ue;
  ngap(isrran::task_sched_handle   task_sched_,
       isrlog::basic_logger&       logger,
       isrran::socket_manager_itf* rx_socket_handler);
  ~ngap();
  int  init(const ngap_args_t& args_, rrc_interface_ngap_nr* rrc_, gtpu_interface_rrc* gtpu_);
  void stop();

  // RRC NR interface
  void initial_ue(uint16_t                             rnti,
                  uint32_t                             gnb_cc_idx,
                  asn1::ngap::rrcestablishment_cause_e cause,
                  isrran::const_byte_span              pdu) override;
  void initial_ue(uint16_t                             rnti,
                  uint32_t                             gnb_cc_idx,
                  asn1::ngap::rrcestablishment_cause_e cause,
                  isrran::const_byte_span              pdu,
                  uint32_t                             s_tmsi) override;

  void write_pdu(uint16_t rnti, isrran::const_byte_span pdu) override;
  bool user_exists(uint16_t rnti) override { return true; };
  void user_mod(uint16_t old_rnti, uint16_t new_rnti) override {}

  // TS 38.413 - Section 8.3.2 - UE Context Release Request
  void user_release_request(uint16_t rnti, asn1::ngap::cause_radio_network_e cause_radio) override;

  bool is_amf_connected() override;
  bool send_error_indication(const asn1::ngap::cause_c& cause,
                             isrran::optional<uint32_t> ran_ue_ngap_id = {},
                             isrran::optional<uint32_t> amf_ue_ngap_id = {});
  void ue_notify_rrc_reconf_complete(uint16_t rnti, bool outcome) override;
  bool send_pdu_session_resource_setup_response();

  // Stack interface
  bool
  handle_amf_rx_msg(isrran::unique_byte_buffer_t pdu, const sockaddr_in& from, const sctp_sndrcvinfo& sri, int flags);
  void get_metrics(ngap_metrics_t& m);
  void get_args(ngap_args_t& args_);

  // PCAP
  void start_pcap(isrran::ngap_pcap* pcap_);

  // Logging
  typedef enum { Rx = 0, Tx } direction_t;
  void log_ngap_message(const asn1::ngap::ngap_pdu_c& msg, const direction_t dir, isrran::const_byte_span pdu);

private:
  static const int AMF_PORT        = 38412;
  static const int ADDR_FAMILY     = AF_INET;
  static const int SOCK_TYPE       = SOCK_STREAM;
  static const int PROTO           = IPPROTO_SCTP;
  static const int PPID            = 60;
  static const int NONUE_STREAM_ID = 0;

  // args
  rrc_interface_ngap_nr*      rrc  = nullptr;
  gtpu_interface_rrc*         gtpu = nullptr;
  ngap_args_t                 args = {};
  isrlog::basic_logger&       logger;
  isrran::task_sched_handle   task_sched;
  isrran::task_queue_handle   amf_task_queue;
  isrran::socket_manager_itf* rx_socket_handler;

  isrran::unique_socket amf_socket;
  struct sockaddr_in    amf_addr            = {}; // AMF address
  bool                  amf_connected       = false;
  bool                  running             = false;
  uint32_t              next_gnb_ue_ngap_id = 1; // Next GNB-side UE identifier
  uint16_t              next_ue_stream_id   = 1; // Next UE SCTP stream identifier
  isrran::unique_timer  amf_connect_timer, ngsetup_timeout;

  // Protocol IEs sent with every UL NGAP message
  asn1::ngap::tai_s    tai;
  asn1::ngap::nr_cgi_s nr_cgi;

  asn1::ngap::ng_setup_resp_s ngsetupresponse;

  int  build_tai_cgi();
  bool connect_amf();
  bool setup_ng();
  bool sctp_send_ngap_pdu(const asn1::ngap::ngap_pdu_c& tx_pdu, uint32_t rnti, const char* procedure_name);

  bool handle_ngap_rx_pdu(isrran::byte_buffer_t* pdu);
  bool handle_successful_outcome(const asn1::ngap::successful_outcome_s& msg);
  bool handle_unsuccessful_outcome(const asn1::ngap::unsuccessful_outcome_s& msg);
  bool handle_initiating_message(const asn1::ngap::init_msg_s& msg);

  // TS 38.413 - Section 8.6.2 - Downlink NAS Transport
  bool handle_dl_nas_transport(const asn1::ngap::dl_nas_transport_s& msg);
  // TS 38.413 - Section 9.2.6.2 - NG Setup Response
  bool handle_ng_setup_response(const asn1::ngap::ng_setup_resp_s& msg);
  // TS 38.413 - Section 9.2.6.3 - NG Setup Failure
  bool handle_ng_setup_failure(const asn1::ngap::ng_setup_fail_s& msg);
  // TS 38.413 - Section 9.2.2.5 - UE Context Release Command
  bool handle_ue_context_release_cmd(const asn1::ngap::ue_context_release_cmd_s& msg);
  // TS 38.413 - Section 9.2.2.1 - Initial Context Setup Request
  bool handle_initial_ctxt_setup_request(const asn1::ngap::init_context_setup_request_s& msg);
  // TS 38.413 - Section 9.2.1.1 - PDU Session Resource Setup Request
  bool handle_ue_pdu_session_res_setup_request(const asn1::ngap::pdu_session_res_setup_request_s& msg);
  // TS 38.413 - Section 9.2.4.1 - Paging
  bool handle_paging(const asn1::ngap::paging_s& msg);

  // PCAP
  isrran::ngap_pcap* pcap = nullptr;

  class user_list
  {
  public:
    using value_type     = std::unique_ptr<ue>;
    using iterator       = std::unordered_map<uint32_t, value_type>::iterator;
    using const_iterator = std::unordered_map<uint32_t, value_type>::const_iterator;
    using pair_type      = std::unordered_map<uint32_t, value_type>::value_type;

    ue*            find_ue_rnti(uint16_t rnti);
    ue*            find_ue_gnbid(uint32_t gnbid);
    ue*            find_ue_amfid(uint64_t amfid);
    ue*            add_user(value_type user);
    void           erase(ue* ue_ptr);
    iterator       begin() { return users.begin(); }
    iterator       end() { return users.end(); }
    const_iterator cbegin() const { return users.begin(); }
    const_iterator cend() const { return users.end(); }
    size_t         size() const { return users.size(); }

  private:
    std::unordered_map<uint32_t, std::unique_ptr<ue> > users; // maps ran_ue_ngap_id to user
  };
  user_list users;

  // procedures
  class ng_setup_proc_t
  {
  public:
    struct ngsetupresult {
      bool success = false;
      enum class cause_t { timeout, failure } cause;
    };

    explicit ng_setup_proc_t(ngap* ngap_) : ngap_ptr(ngap_) {}
    isrran::proc_outcome_t init();
    isrran::proc_outcome_t step() { return isrran::proc_outcome_t::yield; }
    isrran::proc_outcome_t react(const ngsetupresult& event);
    void                   then(const isrran::proc_state_t& result) const;
    const char*            name() const { return "AMF Connection"; }

  private:
    isrran::proc_outcome_t start_amf_connection();

    ngap* ngap_ptr = nullptr;
  };

  ue* handle_ngapmsg_ue_id(uint32_t gnb_id, uint64_t amf_id);

  isrran::proc_t<ng_setup_proc_t> ngsetup_proc;

  std::string get_cause(const asn1::ngap::cause_c& c);
  void        log_ngap_msg(const asn1::ngap::ngap_pdu_c& msg, isrran::const_span<uint8_t> sdu, bool is_rx);
};

} // namespace isrenb
#endif
