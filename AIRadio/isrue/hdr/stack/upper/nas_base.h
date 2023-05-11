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

#ifndef ISRUE_NAS_BASE_H
#define ISRUE_NAS_BASE_H

#include "isrran/asn1/liblte_mme.h"
#include "isrran/common/buffer_pool.h"
#include "isrran/common/common.h"
#include "isrran/common/nas_pcap.h"
#include "isrran/common/security.h"
#include "isrran/common/string_helpers.h"
#include "isrran/config.h"

using isrran::byte_buffer_t;

namespace isrue {

class nas_base
{
public:
  nas_base(isrlog::basic_logger& logger_, uint32_t mac_offset, uint32_t seq_offset_, uint32_t bearer_id_);
  // PCAP
  void start_pcap(isrran::nas_pcap* pcap_) { pcap = pcap_; }

protected:
  isrlog::basic_logger& logger;
  // PCAP
  isrran::nas_pcap* pcap = nullptr;

  // Security context
  // Base context applies for LTE and 5G
  struct nas_sec_base_ctxt {
    uint8_t                             k_nas_enc[32] = {};
    uint8_t                             k_nas_int[32] = {};
    isrran::CIPHERING_ALGORITHM_ID_ENUM cipher_algo;
    isrran::INTEGRITY_ALGORITHM_ID_ENUM integ_algo;
    uint32_t                            tx_count;
    uint32_t                            rx_count;
  };

  // Only applies for LTE
  struct nas_sec_ctxt {
    uint8_t                              ksi;
    uint8_t                              k_asme[32];
    uint32_t                             k_enb_count;
    LIBLTE_MME_EPS_MOBILE_ID_GUTI_STRUCT guti;
  };

  // Only applies for 5G
  struct nas_5g_sec_ctxt {
    uint8_t  ksi;
    uint8_t  k_amf[32];
    uint32_t k_gnb_count;
  };

  nas_sec_base_ctxt ctxt_base = {};
  nas_sec_ctxt      ctxt      = {};
  nas_5g_sec_ctxt   ctxt_5g   = {};

  int parse_security_algorithm_list(std::string algorithm_string, bool* algorithm_caps);

  // Security
  void
  integrity_generate(uint8_t* key_128, uint32_t count, uint8_t direction, uint8_t* msg, uint32_t msg_len, uint8_t* mac);
  bool integrity_check(isrran::byte_buffer_t* pdu);
  void cipher_encrypt(isrran::byte_buffer_t* pdu);
  void cipher_decrypt(isrran::byte_buffer_t* pdu);

  uint32_t mac_offset = 0;
  uint32_t seq_offset = 0;
  uint32_t bearer_id  = 0;

  const std::string gw_setup_failure_str = "Failed to setup/configure GW interface";
};

} // namespace isrue
#endif
