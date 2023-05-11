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
#include "isrenb/hdr/phy/nr/worker_pool.h"
#include "isrran/common/band_helper.h"

namespace isrenb {
namespace nr {

worker_pool::worker_pool(isrran::phy_common_interface& common_,
                         stack_interface_phy_nr&       stack_,
                         isrlog::sink&                 log_sink_,
                         uint32_t                      max_workers) :
  pool(max_workers, "NR-"),
  common(common_),
  stack(stack_),
  log_sink(log_sink_),
  logger(isrlog::fetch_basic_logger("PHY-NR", log_sink)),
  prach_stack_adaptor(stack_)
{
  // Do nothing
}

bool worker_pool::init(const args_t& args, const phy_cell_cfg_list_nr_t& cell_list)
{
  nof_prach_workers = args.nof_prach_workers;

  // Calculate sampling rate in Hz
  if (not std::isnormal(args.srate_hz)) {
    srate_hz = ISRRAN_SUBC_SPACING_NR(cell_list[0].carrier.scs) * isrran_min_symbol_sz_rb(cell_list[0].carrier.nof_prb);
  } else {
    srate_hz = args.srate_hz;
  }

  // Configure logger
  isrlog::basic_levels log_level = isrlog::str_to_basic_level(args.log.phy_level);
  logger.set_level(log_level);

  // Add workers to workers pool and start threads
  for (uint32_t i = 0; i < args.nof_phy_threads; i++) {
    auto& log = isrlog::fetch_basic_logger(fmt::format("{}PHY{}-NR", args.log.id_preamble, i), log_sink);
    log.set_level(log_level);
    log.set_hex_dump_max_size(args.log.phy_hex_limit);

    auto w = new slot_worker(common, stack, *this, log);
    pool.init_worker(i, w, args.prio);
    workers.push_back(std::unique_ptr<slot_worker>(w));

    slot_worker::args_t w_args     = {};
    uint32_t            cell_index = 0;
    w_args.cell_index              = cell_index;
    w_args.nof_max_prb             = cell_list[cell_index].carrier.nof_prb;
    w_args.nof_tx_ports            = cell_list[cell_index].carrier.max_mimo_layers;
    w_args.nof_rx_ports            = cell_list[cell_index].carrier.max_mimo_layers;
    w_args.rf_port                 = cell_list[cell_index].rf_port;
    w_args.srate_hz                = srate_hz;
    w_args.pusch_max_its           = args.pusch_max_its;
    w_args.pusch_min_snr_dB        = args.pusch_min_snr_dB;

    if (not w->init(w_args)) {
      return false;
    }
  }

  return true;
}

void worker_pool::start_worker(slot_worker* w)
{
  // Push worker into synchronization queue
  slot_sync.push(w);

  // Feed PRACH detection before start processing
  prach.new_tti(0, current_tti, w->get_buffer_rx(0));

  // Start actual worker
  pool.start_worker(w);
}

slot_worker* worker_pool::wait_worker(uint32_t tti)
{
  slot_worker* w = (slot_worker*)pool.wait_worker(tti);

  // Save current TTI
  current_tti = tti;

  // Return worker
  return w;
}

slot_worker* worker_pool::wait_worker_id(uint32_t id)
{
  return (slot_worker*)pool.wait_worker_id(id);
}

void worker_pool::stop()
{
  pool.stop();
  prach.stop();
}

int worker_pool::set_common_cfg(const phy_interface_rrc_nr::common_cfg_t& common_cfg)
{
  // Best effort to convert NR carrier into LTE cell
  isrran_cell_t cell = {};
  int           ret  = isrran_carrier_to_cell(&common_cfg.carrier, &cell);
  if (ret < ISRRAN_SUCCESS) {
    logger.error("Converting carrier to cell for PRACH (%d)", ret);
    return ISRRAN_ERROR;
  }

  // Best effort to set up NR-PRACH config reused for NR
  isrran_prach_cfg_t prach_cfg           = common_cfg.prach;
  uint32_t           lte_nr_prach_offset = (common_cfg.carrier.nof_prb - cell.nof_prb) / 2;
  if (prach_cfg.freq_offset < lte_nr_prach_offset) {
    logger.error("prach_cfg.freq_offset=%d is not compatible with LTE", prach_cfg.freq_offset);
    return ISRRAN_ERROR;
  }
  prach_cfg.freq_offset -= lte_nr_prach_offset;
  prach_cfg.is_nr                 = true;
  prach_cfg.tdd_config.configured = (common_cfg.duplex_mode == ISRRAN_DUPLEX_MODE_TDD);

  // Set the PRACH configuration
  prach.init(0, cell, prach_cfg, &prach_stack_adaptor, logger, 0, nof_prach_workers);
  prach.set_max_prach_offset_us(1000);

  // Setup SSB sampling rate and scaling
  isrran_ssb_cfg_t ssb_cfg = common_cfg.ssb;
  ssb_cfg.srate_hz         = srate_hz;
  ssb_cfg.scaling =
      isrran_convert_dB_to_amplitude(isrran_gnb_dl_get_maximum_signal_power_dBfs(common_cfg.carrier.nof_prb));

  // Print SSB configuration, helps debugging gNb and UE
  if (logger.info.enabled()) {
    std::array<char, 512> ssb_cfg_str = {};
    isrran_ssb_cfg_to_str(&ssb_cfg, ssb_cfg_str.data(), (uint32_t)ssb_cfg_str.size());
    logger.info("Setting SSB configuration %s", ssb_cfg_str.data());
  }

  // For each worker set configuration
  for (uint32_t i = 0; i < pool.get_nof_workers(); i++) {
    // Reserve worker from pool
    slot_worker* w = (slot_worker*)pool.wait_worker_id(i);
    if (w == nullptr) {
      // Skip worker if invalid pointer
      continue;
    }

    // Setup worker common configuration
    if (not w->set_common_cfg(common_cfg.carrier, common_cfg.pdcch, ssb_cfg)) {
      return ISRRAN_ERROR;
    }

    // Release worker
    w->release();
  }

  return ISRRAN_SUCCESS;
}

} // namespace nr
} // namespace isrenb
