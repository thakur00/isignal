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

#include "isrgnb/hdr/stack/ngap/ngap.h"
#include "isrran/common/network_utils.h"
#include "isrran/common/test_common.h"

using namespace isrenb;

struct amf_dummy {
  amf_dummy(const char* addr_str_, int port_) : addr_str(addr_str_), port(port_)
  {
    isrran::net_utils::set_sockaddr(&amf_sockaddr, addr_str, port);
    {
      using namespace isrran::net_utils;
      fd = open_socket(addr_family::ipv4, socket_type::seqpacket, protocol_type::SCTP);
      TESTASSERT(fd > 0);
      TESTASSERT(bind_addr(fd, amf_sockaddr));
    }

    int success = listen(fd, SOMAXCONN);
    isrran_assert(success == 0, "Failed to listen to incoming SCTP connections");
  }

  ~amf_dummy()
  {
    if (fd > 0) {
      close(fd);
    }
  }

  isrran::unique_byte_buffer_t read_msg(sockaddr_in* sockfrom = nullptr)
  {
    isrran::unique_byte_buffer_t pdu     = isrran::make_byte_buffer();
    sockaddr_in                  from    = {};
    socklen_t                    fromlen = sizeof(from);
    sctp_sndrcvinfo              sri     = {};
    int                          flags   = 0;
    ssize_t n_recv = sctp_recvmsg(fd, pdu->msg, pdu->get_tailroom(), (struct sockaddr*)&from, &fromlen, &sri, &flags);
    if (n_recv > 0) {
      if (sockfrom != nullptr) {
        *sockfrom = from;
      }
      pdu->N_bytes = n_recv;
    }
    return pdu;
  }

  const char*                  addr_str;
  int                          port;
  struct sockaddr_in           amf_sockaddr = {};
  int                          fd;
  isrran::unique_byte_buffer_t last_sdu;
};

class rrc_nr_dummy : public rrc_interface_ngap_nr
{
public:
  rrc_nr_dummy() : rrc_logger(isrlog::fetch_basic_logger("RRC_NR")) { rrc_logger.set_hex_dump_max_size(32); }
  int ue_set_security_cfg_key(uint16_t rnti, const asn1::fixed_bitstring<256, false, true>& key)
  {
    for (uint32_t i = 0; i < key.nof_octets(); ++i) {
      sec_key[i] = key.data()[key.nof_octets() - 1 - i];
    }
    rrc_logger.info(sec_key, 32, "Security key");
    return ISRRAN_SUCCESS;
  }
  int ue_set_bitrates(uint16_t rnti, const asn1::ngap::ue_aggregate_maximum_bit_rate_s& rates)
  {
    rrc_logger.info("Setting aggregate max bitrate. RNTI 0x%x", rnti);
    return ISRRAN_SUCCESS;
  }
  int set_aggregate_max_bitrate(uint16_t rnti, const asn1::ngap::ue_aggregate_maximum_bit_rate_s& rates)
  {
    rrc_logger.info("Setting aggregate max bitrate");
    return ISRRAN_SUCCESS;
  }
  int ue_set_security_cfg_capabilities(uint16_t rnti, const asn1::ngap::ue_security_cap_s& caps)
  {
    rrc_logger.info("Setting security capabilities");
    for (uint8_t i = 0; i < 8; i++) {
      rrc_logger.info("NIA%d = %s", i, caps.nrintegrity_protection_algorithms.get(i) ? "true" : "false");
    }
    for (uint8_t i = 0; i < 8; i++) {
      rrc_logger.info("NEA%d = %s", i, caps.nrencryption_algorithms.get(i) ? "true" : "false");
    }
    for (uint8_t i = 0; i < 8; i++) {
      rrc_logger.info("EEA%d = %s", i, caps.eutr_aencryption_algorithms.get(i) ? "true" : "false");
    }
    for (uint8_t i = 0; i < 8; i++) {
      rrc_logger.info("EIA%d = %s", i, caps.eutr_aintegrity_protection_algorithms.get(i) ? "true" : "false");
    }
    sec_caps = caps;
    return ISRRAN_SUCCESS;
  }
  int start_security_mode_procedure(uint16_t rnti, isrran::unique_byte_buffer_t nas_pdu)
  {
    rrc_logger.info("Starting securtity mode procedure");
    sec_mod_proc_started = true;
    return ISRRAN_SUCCESS;
  }
  int establish_rrc_bearer(uint16_t                rnti,
                           uint16_t                pdu_session_id,
                           isrran::const_byte_span nas_pdu,
                           uint32_t                lcid,
                           uint32_t                five_qi)
  {
    rrc_logger.info("Establish RRC bearer");
    return ISRRAN_SUCCESS;
  }

  int  release_bearers(uint16_t rnti) { return ISRRAN_SUCCESS; }
  void release_user(uint16_t rnti) {}
  int  allocate_lcid(uint16_t rnti) { return ISRRAN_SUCCESS; }
  void write_dl_info(uint16_t rnti, isrran::unique_byte_buffer_t sdu) {}

  bool                          sec_mod_proc_started = false;
  uint8_t                       sec_key[32]          = {};
  asn1::ngap::ue_security_cap_s sec_caps             = {};
  isrlog::basic_logger&         rrc_logger;
};
struct dummy_socket_manager : public isrran::socket_manager_itf {
  dummy_socket_manager() : isrran::socket_manager_itf(isrlog::fetch_basic_logger("TEST")) {}

  /// Register (fd, callback). callback is called within socket thread when fd has data.
  bool add_socket_handler(int fd, recv_callback_t handler) final
  {
    if (s1u_fd > 0) {
      return false;
    }
    s1u_fd   = fd;
    callback = std::move(handler);
    return true;
  }

  /// remove registered socket fd
  bool remove_socket(int fd) final
  {
    if (s1u_fd < 0) {
      return false;
    }
    s1u_fd = -1;
    return true;
  }

  int             s1u_fd = 0;
  recv_callback_t callback;
};

void run_ng_setup(ngap& ngap_obj, amf_dummy& amf)
{
  asn1::ngap::ngap_pdu_c ngap_pdu;

  // gNB -> AMF: NG Setup Request
  isrran::unique_byte_buffer_t sdu = amf.read_msg();
  TESTASSERT(sdu->N_bytes > 0);
  asn1::cbit_ref cbref(sdu->msg, sdu->N_bytes);
  TESTASSERT(ngap_pdu.unpack(cbref) == asn1::ISRASN_SUCCESS);
  TESTASSERT(ngap_pdu.type().value == asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  TESTASSERT(ngap_pdu.init_msg().proc_code == ASN1_NGAP_ID_NG_SETUP);

  // AMF -> gNB: ng Setup Response
  sockaddr_in     amf_addr = {};
  sctp_sndrcvinfo rcvinfo  = {};
  int             flags    = 0;

  uint8_t ng_setup_resp[] = {0x20, 0x15, 0x00, 0x55, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x31, 0x17, 0x00, 0x61, 0x6d,
                             0x61, 0x72, 0x69, 0x73, 0x6f, 0x66, 0x74, 0x2e, 0x61, 0x6d, 0x66, 0x2e, 0x35, 0x67, 0x63,
                             0x2e, 0x6d, 0x6e, 0x63, 0x30, 0x30, 0x31, 0x2e, 0x6d, 0x63, 0x63, 0x30, 0x30, 0x31, 0x2e,
                             0x33, 0x67, 0x70, 0x70, 0x6e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x2e, 0x6f, 0x72, 0x67,
                             0x00, 0x60, 0x00, 0x08, 0x00, 0x00, 0x00, 0xf1, 0x10, 0x80, 0x01, 0x01, 0x00, 0x56, 0x40,
                             0x01, 0x32, 0x00, 0x50, 0x00, 0x08, 0x00, 0x00, 0xf1, 0x10, 0x00, 0x00, 0x00, 0x08};
  memcpy(sdu->msg, ng_setup_resp, sizeof(ng_setup_resp));
  sdu->N_bytes = sizeof(ng_setup_resp);
  TESTASSERT(ngap_obj.handle_amf_rx_msg(std::move(sdu), amf_addr, rcvinfo, flags));
}

void run_ng_initial_ue(ngap& ngap_obj, amf_dummy& amf, rrc_nr_dummy& rrc)
{
  // RRC will call the initial UE request with the NAS PDU
  uint8_t nas_reg_req[] = {0x7e, 0x00, 0x41, 0x79, 0x00, 0x0d, 0x01, 0x00, 0xf1, 0x10, 0x00, 0x00,
                           0x00, 0x00, 0x10, 0x32, 0x54, 0x76, 0x98, 0x2e, 0x02, 0xf0, 0xf0};

  isrran::unique_byte_buffer_t nas_pdu = isrran::make_byte_buffer();
  memcpy(nas_pdu->msg, nas_reg_req, sizeof(nas_reg_req));
  nas_pdu->N_bytes = sizeof(nas_reg_req);

  uint32_t rnti = 0xf0f0;
  ngap_obj.initial_ue(rnti, 0, asn1::ngap::rrcestablishment_cause_opts::mo_sig, isrran::make_span(nas_pdu));

  // gNB -> AMF: Inital UE message
  asn1::ngap::ngap_pdu_c       ngap_initial_ue_pdu;
  isrran::unique_byte_buffer_t sdu = amf.read_msg();
  TESTASSERT(sdu->N_bytes > 0);
  asn1::cbit_ref cbref(sdu->msg, sdu->N_bytes);
  TESTASSERT_EQ(ngap_initial_ue_pdu.unpack(cbref), asn1::ISRASN_SUCCESS);
  TESTASSERT_EQ(ngap_initial_ue_pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  TESTASSERT_EQ(ngap_initial_ue_pdu.init_msg().proc_code, ASN1_NGAP_ID_INIT_UE_MSG);

  // AMF -> gNB: Initial Context Setup Request
  sockaddr_in     amf_addr = {};
  sctp_sndrcvinfo rcvinfo  = {};
  int             flags    = 0;

  asn1::ngap::ngap_pdu_c ngap_initial_ctx_req_pdu;
  ngap_initial_ctx_req_pdu.set_init_msg().load_info_obj(ASN1_NGAP_ID_INIT_CONTEXT_SETUP);
  auto& container = ngap_initial_ctx_req_pdu.init_msg().value.init_context_setup_request();

  container->amf_ue_ngap_id.value = 0x1;
  container->ran_ue_ngap_id.value = 0x1;
  container->nas_pdu_present      = true;

  // Set allowed NSSAI
  container->allowed_nssai.value.resize(1);
  container->allowed_nssai.value[0].s_nssai.sst.from_number(1);

  // Set security key
  uint8_t sec_key[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
  for (uint8_t i = 0; i < 32; ++i) {
    container->security_key.value.data()[31 - i] = sec_key[i];
  }

  // Set security capabilities
  container->ue_security_cap.value.nrencryption_algorithms.set(0, true);
  container->ue_security_cap.value.nrencryption_algorithms.set(1, true);
  container->ue_security_cap.value.nrintegrity_protection_algorithms.set(0, true);
  container->ue_security_cap.value.nrintegrity_protection_algorithms.set(1, true);
  container->ue_security_cap.value.eutr_aencryption_algorithms.set(0, true);
  container->ue_security_cap.value.eutr_aencryption_algorithms.set(1, true);
  container->ue_security_cap.value.eutr_aintegrity_protection_algorithms.set(0, true);
  container->ue_security_cap.value.eutr_aintegrity_protection_algorithms.set(1, true);

  // Set PDU Session Response Setup Item
  // TODO

  isrran::unique_byte_buffer_t buf = isrran::make_byte_buffer();
  TESTASSERT_NEQ(buf, nullptr);
  asn1::bit_ref bref(buf->msg, buf->get_tailroom());
  TESTASSERT_EQ(ngap_initial_ctx_req_pdu.pack(bref), asn1::ISRASN_SUCCESS);
  buf->N_bytes = bref.distance_bytes();

  // Feed Initial Context Setup Request to NGAP
  TESTASSERT(ngap_obj.handle_amf_rx_msg(std::move(buf), amf_addr, rcvinfo, flags));

  // Check RRC security key
  for (uint8_t i = 0; i < 32; ++i) {
    TESTASSERT_EQ(sec_key[i], rrc.sec_key[i]);
  }

  // Check RRC security capabilities
  for (uint8_t i = 0; i < 8; ++i) {
    TESTASSERT_EQ(container->ue_security_cap.value.nrencryption_algorithms.get(i),
                  rrc.sec_caps.nrencryption_algorithms.get(i));
    TESTASSERT_EQ(container->ue_security_cap.value.nrintegrity_protection_algorithms.get(i),
                  rrc.sec_caps.nrintegrity_protection_algorithms.get(i));
    TESTASSERT_EQ(container->ue_security_cap.value.eutr_aencryption_algorithms.get(i),
                  rrc.sec_caps.eutr_aencryption_algorithms.get(i));
    TESTASSERT_EQ(container->ue_security_cap.value.eutr_aintegrity_protection_algorithms.get(i),
                  rrc.sec_caps.eutr_aintegrity_protection_algorithms.get(i));
  }

  // Check PDU Session Response Setup Item
  // TODO

  // Check RRC security mode command was started
  TESTASSERT(rrc.sec_mod_proc_started);
}

int main(int argc, char** argv)
{
  // Setup logging.
  isrlog::basic_logger& logger = isrlog::fetch_basic_logger("NGAP");
  logger.set_level(isrlog::basic_levels::debug);
  logger.set_hex_dump_max_size(-1);

  isrran::task_scheduler task_sched;
  dummy_socket_manager   rx_sockets;
  ngap                   ngap_obj(&task_sched, logger, &rx_sockets);

  const char*    amf_addr_str = "127.0.0.1";
  const uint32_t AMF_PORT     = 38412;
  amf_dummy      amf(amf_addr_str, AMF_PORT);

  ngap_args_t args   = {};
  args.cell_id       = 0x01;
  args.gnb_id        = 0x19B;
  args.mcc           = 907;
  args.mnc           = 70;
  args.ngc_bind_addr = "127.0.0.100";
  args.tac           = 7;
  args.gtp_bind_addr = "127.0.0.100";
  args.amf_addr      = "127.0.0.1";
  args.gnb_name      = "isrgnb01";

  rrc_nr_dummy        rrc;
  gtpu_interface_rrc* gtpu = nullptr;
  ngap_obj.init(args, &rrc, gtpu);

  // Start the log backend.
  isrran::test_init(argc, argv);
  run_ng_setup(ngap_obj, amf);
  run_ng_initial_ue(ngap_obj, amf, rrc);
}
