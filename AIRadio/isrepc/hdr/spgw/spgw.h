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
 * File:        spgw.h
 * Description: Top-level SP-GW class. Creates and links all
 *              interfaces and helpers.
 *****************************************************************************/

#ifndef ISREPC_SPGW_H
#define ISREPC_SPGW_H

#include "isrran/asn1/gtpc.h"
#include "isrran/common/buffer_pool.h"
#include "isrran/common/threads.h"
#include "isrran/isrlog/isrlog.h"
#include <cstddef>
#include <queue>

namespace isrepc {

class mme_gtpc;

const uint16_t GTPU_RX_PORT = 2152;

typedef struct {
  std::string gtpu_bind_addr;
  std::string sgi_if_addr;
  std::string sgi_if_name;
  uint32_t    max_paging_queue;
} spgw_args_t;

typedef struct spgw_tunnel_ctx {
  uint64_t                                 imsi;
  in_addr_t                                ue_ipv4;
  uint8_t                                  ebi;
  isrran::gtp_fteid_t                      up_ctrl_fteid;
  isrran::gtp_fteid_t                      up_user_fteid;
  isrran::gtp_fteid_t                      dw_ctrl_fteid;
  isrran::gtp_fteid_t                      dw_user_fteid;
  bool                                     paging_pending;
  std::queue<isrran::unique_byte_buffer_t> paging_queue;
} spgw_tunnel_ctx_t;

class spgw : public isrran::thread
{
  class gtpc;
  class gtpu;

public:
  static spgw* get_instance(void);
  static void  cleanup(void);
  int          init(spgw_args_t* args, const std::map<std::string, uint64_t>& ip_to_imsi);
  void         stop();
  void         run_thread();

private:
  spgw();
  virtual ~spgw();
  static spgw* m_instance;

  spgw_tunnel_ctx_t* create_gtp_ctx(struct isrran::gtpc_create_session_request* cs_req);
  bool               delete_gtp_ctx(uint32_t ctrl_teid);

  bool      m_running;
  mme_gtpc* m_mme_gtpc;

  // GTP-C and GTP-U handlers
  gtpc* m_gtpc;
  gtpu* m_gtpu;

  // Logs
  isrlog::basic_logger& m_logger = isrlog::fetch_basic_logger("SPGW");
};

} // namespace isrepc
#endif // ISREPC_SPGW_H
