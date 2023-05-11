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

#include "isrue/hdr/ue.h"
#include "isrran/build_info.h"
#include "isrran/common/standard_streams.h"
#include "isrran/common/string_helpers.h"
#include "isrran/radio/radio.h"
#include "isrran/radio/radio_null.h"
#include "isrran/isrran.h"
#include "isrue/hdr/phy/dummy_phy.h"
#include "isrue/hdr/phy/phy.h"
#include "isrue/hdr/phy/phy_nr_sa.h"
#include "isrue/hdr/stack/ue_stack_lte.h"
#include "isrue/hdr/stack/ue_stack_nr.h"
#include <algorithm>
#include <iostream>
#include <string>

using namespace isrran;

namespace isrue {

ue::ue() : logger(isrlog::fetch_basic_logger("UE", false)), sys_proc(logger)
{
  // print build info
  std::cout << std::endl << get_build_string() << std::endl << std::endl;
}

ue::~ue()
{
  stack.reset();
}

int ue::init(const all_args_t& args_)
{
  int ret = ISRRAN_SUCCESS;

  // Init UE log
  logger.set_level(isrlog::basic_levels::info);
  logger.info("%s", get_build_string().c_str());

  // Validate arguments
  if (parse_args(args_)) {
    isrran::console("Error processing arguments. Please check %s for more details.\n", args_.log.filename.c_str());
    return ISRRAN_ERROR;
  }

  // Instantiate layers and stack together our UE
  std::unique_ptr<ue_stack_lte> lte_stack(new ue_stack_lte);
  if (!lte_stack) {
    isrran::console("Error creating LTE stack instance.\n");
    return ISRRAN_ERROR;
  }

  std::unique_ptr<gw> gw_ptr(new gw(isrlog::fetch_basic_logger("GW", false)));
  if (!gw_ptr) {
    isrran::console("Error creating a GW instance.\n");
    return ISRRAN_ERROR;
  }

  std::unique_ptr<isrran::radio> lte_radio = std::unique_ptr<isrran::radio>(new isrran::radio);
  if (!lte_radio) {
    isrran::console("Error creating radio multi instance.\n");
    return ISRRAN_ERROR;
  }

  isrue::phy_args_nr_t phy_args_nr = {};
  phy_args_nr.max_nof_prb          = args.phy.nr_max_nof_prb;
  phy_args_nr.rf_channel_offset    = args.phy.nof_lte_carriers;
  phy_args_nr.nof_carriers         = args.phy.nof_nr_carriers;
  phy_args_nr.nof_phy_threads      = args.phy.nof_phy_threads;
  phy_args_nr.worker_cpu_mask      = args.phy.worker_cpu_mask;
  phy_args_nr.log                  = args.phy.log;
  phy_args_nr.store_pdsch_ko       = args.phy.nr_store_pdsch_ko;
  phy_args_nr.srate_hz             = args.rf.srate_hz;

  // init layers
  if (args.phy.nof_lte_carriers == 0) {
    // SA mode
    std::unique_ptr<isrue::phy_nr_sa> nr_phy = std::unique_ptr<isrue::phy_nr_sa>(new isrue::phy_nr_sa("PHY-SA"));
    if (!nr_phy) {
      isrran::console("Error creating NR PHY instance.\n");
      return ISRRAN_ERROR;
    }

    // In SA mode, pass the NR SA phy to the radio
    if (lte_radio->init(args.rf, nr_phy.get())) {
      isrran::console("Error initializing radio.\n");
      return ISRRAN_ERROR;
    }
    if (nr_phy->init(phy_args_nr, lte_stack.get(), lte_radio.get())) {
      isrran::console("Error initializing PHY NR SA.\n");
      ret = ISRRAN_ERROR;
    }

    std::unique_ptr<isrue::dummy_phy> dummy_lte_phy = std::unique_ptr<isrue::dummy_phy>(new isrue::dummy_phy);
    if (!dummy_lte_phy) {
      isrran::console("Error creating dummy LTE PHY instance.\n");
      return ISRRAN_ERROR;
    }

    // In SA mode, pass NR PHY pointer to stack
    args.stack.sa_mode = true;
    if (lte_stack->init(args.stack, dummy_lte_phy.get(), nr_phy.get(), gw_ptr.get())) {
      isrran::console("Error initializing stack.\n");
      ret = ISRRAN_ERROR;
    }
    phy       = std::move(nr_phy);
    dummy_phy = std::move(dummy_lte_phy);
  } else {
    // LTE or NSA mode
    std::unique_ptr<isrue::phy> lte_phy = std::unique_ptr<isrue::phy>(new isrue::phy);
    if (!lte_phy) {
      isrran::console("Error creating LTE PHY instance.\n");
      return ISRRAN_ERROR;
    }

    if (lte_radio->init(args.rf, lte_phy.get())) {
      isrran::console("Error initializing radio.\n");
      return ISRRAN_ERROR;
    }
    // from here onwards do not exit immediately if something goes wrong as sub-layers may already use interfaces
    if (lte_phy->init(args.phy, lte_stack.get(), lte_radio.get())) {
      isrran::console("Error initializing PHY.\n");
      ret = ISRRAN_ERROR;
    }
    if (args.phy.nof_nr_carriers > 0) {
      if (lte_phy->init(phy_args_nr, lte_stack.get(), lte_radio.get())) {
        isrran::console("Error initializing NR PHY.\n");
        ret = ISRRAN_ERROR;
      }
    }
    if (lte_stack->init(args.stack, lte_phy.get(), lte_phy.get(), gw_ptr.get())) {
      isrran::console("Error initializing stack.\n");
      ret = ISRRAN_ERROR;
    }
    phy = std::move(lte_phy);
  }

  if (gw_ptr->init(args.gw, lte_stack.get())) {
    isrran::console("Error initializing GW.\n");
    ret = ISRRAN_ERROR;
  }

  // move ownership
  stack   = std::move(lte_stack);
  gw_inst = std::move(gw_ptr);
  radio   = std::move(lte_radio);

  if (phy) {
    isrran::console("Waiting PHY to initialize ... ");
    phy->wait_initialize();
    isrran::console("done!\n");
  }
  return ret;
}

int ue::parse_args(const all_args_t& args_)
{
  // set member variable
  args = args_;

  // carry out basic sanity checks
  if (args.stack.rrc.mbms_service_id > -1) {
    if (!args.phy.interpolate_subframe_enabled) {
      logger.error("interpolate_subframe_enabled = %d, While using MBMS, "
                   "please set interpolate_subframe_enabled to true",
                   args.phy.interpolate_subframe_enabled);
      return ISRRAN_ERROR;
    }
    if (args.phy.nof_phy_threads > 2) {
      logger.error("nof_phy_threads = %d, While using MBMS, please set "
                   "number of phy threads to 1 or 2",
                   args.phy.nof_phy_threads);
      return ISRRAN_ERROR;
    }
    if ((0 == args.phy.snr_estim_alg.find("refs"))) {
      logger.error("snr_estim_alg = refs, While using MBMS, please set "
                   "algorithm to pss or empty");
      return ISRRAN_ERROR;
    }
  }

  if (args.rf.nof_antennas > ISRRAN_MAX_PORTS) {
    fprintf(stderr, "Maximum number of antennas exceeded (%d > %d)\n", args.rf.nof_antennas, ISRRAN_MAX_PORTS);
    return ISRRAN_ERROR;
  }

  args.rf.nof_carriers = args.phy.nof_lte_carriers + args.phy.nof_nr_carriers;

  if (args.rf.nof_carriers > ISRRAN_MAX_CARRIERS) {
    fprintf(stderr,
            "Maximum number of carriers exceeded (%d > %d) (nof_lte_carriers %d + nof_nr_carriers %d)\n",
            args.rf.nof_carriers,
            ISRRAN_MAX_CARRIERS,
            args.phy.nof_lte_carriers,
            args.phy.nof_nr_carriers);
    return ISRRAN_ERROR;
  }

  // replicate some RF parameter to make them available to PHY
  args.phy.nof_rx_ant = args.rf.nof_antennas;
  args.phy.agc_enable = args.rf.rx_gain < 0.0f;

  // populate DL EARFCN list
  if (not args.phy.dl_earfcn.empty()) {
    // Parse DL-EARFCN list
    isrran::string_parse_list(args.phy.dl_earfcn, ',', args.phy.dl_earfcn_list);

    // Populates supported bands
    args.stack.rrc.nof_supported_bands = 0;
    for (uint32_t& earfcn : args.phy.dl_earfcn_list) {
      uint8_t band = isrran_band_get_band(earfcn);
      // Try to find band, if not appends it
      if (std::find(args.stack.rrc.supported_bands.begin(), args.stack.rrc.supported_bands.end(), band) ==
          args.stack.rrc.supported_bands.end()) {
        args.stack.rrc.supported_bands[args.stack.rrc.nof_supported_bands++] = band;
      }
      // RRC NR needs also information about supported eutra bands
      if (std::find(args.stack.rrc_nr.supported_bands_eutra.begin(),
                    args.stack.rrc_nr.supported_bands_eutra.end(),
                    band) == args.stack.rrc_nr.supported_bands_eutra.end()) {
        args.stack.rrc_nr.supported_bands_eutra.push_back(band);
      }
    }
  } else {
    logger.error("Error: dl_earfcn list is empty");
    isrran::console("Error: dl_earfcn list is empty\n");
    return ISRRAN_ERROR;
  }

  // populate UL EARFCN list
  if (not args.phy.ul_earfcn.empty()) {
    std::vector<uint32_t> ul_earfcn_list;
    isrran::string_parse_list(args.phy.ul_earfcn, ',', ul_earfcn_list);

    // For each parsed UL-EARFCN links it to the corresponding DL-EARFCN
    args.phy.ul_earfcn_map.clear();
    for (size_t i = 0; i < ISRRAN_MIN(ul_earfcn_list.size(), args.phy.dl_earfcn_list.size()); i++) {
      args.phy.ul_earfcn_map[args.phy.dl_earfcn_list[i]] = ul_earfcn_list[i];
    }
  }

  // populate NR DL ARFCNs
  if (args.phy.nof_nr_carriers > 0) {
    if (not args.stack.rrc_nr.supported_bands_nr_str.empty()) {
      // Populates supported bands
      isrran::string_parse_list(args.stack.rrc_nr.supported_bands_nr_str, ',', args.stack.rrc_nr.supported_bands_nr);
      args.stack.rrc.supported_bands_nr = args.stack.rrc_nr.supported_bands_nr;
    } else {
      logger.error("Error: rat.nr.bands list is empty");
      isrran::console("Error: rat.nr.bands list is empty\n");
      return ISRRAN_ERROR;
    }
  }

  // Set UE category
  args.stack.rrc.ue_category = (uint32_t)strtoul(args.stack.rrc.ue_category_str.c_str(), nullptr, 10);

  // Consider Carrier Aggregation support if more than one
  args.stack.rrc.support_ca = (args.phy.nof_lte_carriers > 1);

  // Make sure fix sampling rate is set for SA mode
  if (args.phy.nof_lte_carriers == 0 and not std::isnormal(args.rf.srate_hz)) {
    isrran::console("Error. NR Standalone PHY requires a fixed RF sampling rate.\n");
    return ISRRAN_ERROR;
  }

  // SA params
  if (args.phy.nof_lte_carriers == 0 && args.phy.nof_nr_carriers > 0) {
    // Update NAS-5G args
    args.stack.nas_5g.ia5g = args.stack.nas.eia;
    args.stack.nas_5g.ea5g = args.stack.nas.eea;
    args.stack.nas_5g.pdu_session_cfgs.push_back({args.stack.nas.apn_name});
  }

  // Validate the CFR args
  isrran_cfr_cfg_t cfr_test_cfg = {};
  cfr_test_cfg.cfr_enable       = args.phy.cfr_args.enable;
  cfr_test_cfg.cfr_mode         = args.phy.cfr_args.mode;
  cfr_test_cfg.alpha            = args.phy.cfr_args.strength;
  cfr_test_cfg.manual_thr       = args.phy.cfr_args.manual_thres;
  cfr_test_cfg.max_papr_db      = args.phy.cfr_args.auto_target_papr;
  cfr_test_cfg.ema_alpha        = args.phy.cfr_args.ema_alpha;

  if (!isrran_cfr_params_valid(&cfr_test_cfg)) {
    isrran::console("Invalid CFR parameters: cfr_mode=%d, alpha=%.2f, manual_thr=%.2f, \n "
                    "max_papr_db=%.2f, ema_alpha=%.2f\n",
                    cfr_test_cfg.cfr_mode,
                    cfr_test_cfg.alpha,
                    cfr_test_cfg.manual_thr,
                    cfr_test_cfg.max_papr_db,
                    cfr_test_cfg.ema_alpha);

    logger.error("Invalid CFR parameters: cfr_mode=%d, alpha=%.2f, manual_thr=%.2f, max_papr_db=%.2f, ema_alpha=%.2f\n",
                 cfr_test_cfg.cfr_mode,
                 cfr_test_cfg.alpha,
                 cfr_test_cfg.manual_thr,
                 cfr_test_cfg.max_papr_db,
                 cfr_test_cfg.ema_alpha);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

void ue::stop()
{
  // tear down UE in reverse order
  if (stack) {
    stack->stop();
  }

  if (gw_inst) {
    gw_inst->stop();
  }

  if (phy) {
    phy->stop();
  }

  if (radio) {
    radio->stop();
  }
}

bool ue::switch_on()
{
  return stack->switch_on();
}

bool ue::switch_off()
{
  if (gw_inst) {
    gw_inst->stop();
  }

  // send switch off
  stack->switch_off();

  // wait for max. 5s for it to be sent (according to TS 24.301 Sec 25.5.2.2)
  int             cnt = 0, timeout_s = 5;
  stack_metrics_t metrics = {};
  stack->get_metrics(&metrics);

  while (metrics.rrc.state != RRC_STATE_IDLE && ++cnt <= timeout_s) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    stack->get_metrics(&metrics);
  }

  if (metrics.rrc.state != RRC_STATE_IDLE) {
    isrlog::fetch_basic_logger("NAS").warning("Detach couldn't be sent after %ds.", timeout_s);
    return false;
  }

  return true;
}

void ue::start_plot()
{
  phy->start_plot();
}

bool ue::get_metrics(ue_metrics_t* m)
{
  *m = {};
  phy->get_metrics(isrran::isrran_rat_t::lte, &m->phy);
  phy->get_metrics(isrran::isrran_rat_t::nr, &m->phy_nr);
  radio->get_metrics(&m->rf);
  stack->get_metrics(&m->stack);
  gw_inst->get_metrics(m->gw, m->stack.mac[0].nof_tti);
  m->sys = sys_proc.get_metrics();
  return true;
}

std::string ue::get_build_mode()
{
  return std::string(isrran_get_build_mode());
}

std::string ue::get_build_info()
{
  if (std::string(isrran_get_build_info()).find("  ") != std::string::npos) {
    return std::string(isrran_get_version());
  }
  return std::string(isrran_get_build_info());
}

std::string ue::get_build_string()
{
  std::stringstream ss;
  ss << "Built in " << get_build_mode() << " mode using " << get_build_info() << ".";
  return ss.str();
}

} // namespace isrue
