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

#include "isrran/common/bcd_helpers.h"
#include "isrran/common/test_common.h"
#include "isrran/common/tsan_options.h"
#include "isrran/interfaces/ue_pdcp_interfaces.h"
#include "isrran/isrlog/isrlog.h"
#include "isrran/test/ue_test_interfaces.h"
#include "isrue/hdr/stack/upper/gw.h"
#include "isrue/hdr/stack/upper/nas_5g.h"
#include "isrue/hdr/stack/upper/test/nas_test_common.h"

using namespace isrue;
using namespace isrran;

#define HAVE_PCAP 0

int amf_attach_request_test(isrran::nas_pcap* pcap)
{
  int ret = ISRRAN_ERROR;

  rrc_nr_dummy rrc_nr_dummy;
  pdcp_dummy   pdcp_dummy;

  isrue::usim usim(isrlog::fetch_basic_logger("USIM"));
  usim_args_t args;
  args.mode = "soft";
  args.algo = "xor";
  args.imei = "353490069873319";
  args.imsi = "001010123456789";
  args.k    = "00112233445566778899aabbccddeeff";
  args.op   = "63BFA50EE6523365FF14C1F45F88737D";
  usim.init(&args);

  nas_5g_args_t nas_5g_cfg;
  nas_5g_cfg.force_imsi_attach = true;
  pdu_session_cfg_t pdu_session;
  pdu_session.apn_name = "test123";

  nas_5g_cfg.pdu_session_cfgs.push_back(pdu_session);

  nas_5g_cfg.eia = "0,1,2,3";
  nas_5g_cfg.eea = "0,1,2,3";

  nas_5g_cfg.ia5g = "0,1,2,3";
  nas_5g_cfg.ea5g = "0,1,2,3";

  test_stack_dummy<isrue::nas_5g> stack(&pdcp_dummy);
  isrue::nas_5g                   nas_5g(isrlog::fetch_basic_logger("NAS-5G"), &stack.task_sched);
  isrue::gw                       gw(isrlog::fetch_basic_logger("GW"));

  if (pcap != nullptr) {
    nas_5g.start_pcap(pcap);
  }

  gw_args_t gw_args;
  gw_args.tun_dev_name     = "tun0";
  gw_args.log.gw_level     = "debug";
  gw_args.log.gw_hex_limit = 100000;

  gw.init(gw_args, &stack);
  stack.init(&nas_5g);

  nas_5g.init(&usim, &rrc_nr_dummy, &gw, nas_5g_cfg);
  rrc_nr_dummy.init(&nas_5g);

  // trigger test
  stack.switch_on();
  stack.stop();

  gw.stop();
  ret = ISRRAN_SUCCESS;

  return ret;
}

int main(int argc, char** argv)
{
  // Setup logging.
  auto& rrc_logger = isrlog::fetch_basic_logger("RRC", false);
  rrc_logger.set_level(isrlog::basic_levels::debug);
  rrc_logger.set_hex_dump_max_size(100000);
  auto& nas_logger = isrlog::fetch_basic_logger("NAS", false);
  nas_logger.set_level(isrlog::basic_levels::debug);
  nas_logger.set_hex_dump_max_size(100000);
  auto& usim_logger = isrlog::fetch_basic_logger("USIM", false);
  usim_logger.set_level(isrlog::basic_levels::debug);
  usim_logger.set_hex_dump_max_size(100000);
  auto& gw_logger = isrlog::fetch_basic_logger("GW", false);
  gw_logger.set_level(isrlog::basic_levels::debug);
  gw_logger.set_hex_dump_max_size(100000);

  // Start the log backend.
  isrlog::init();
#if HAVE_PCAP
  isrran::nas_pcap pcap;
  pcap.open("nas_5g_test.pcap", 0, isrran::isrran_rat_t::nr);
  TESTASSERT(amf_attach_request_test(&pcap) == ISRRAN_SUCCESS);
  pcap.close();
#else
  TESTASSERT(amf_attach_request_test(nullptr) == ISRRAN_SUCCESS);
#endif // HAVE_PCAP

  return ISRRAN_SUCCESS;
}
