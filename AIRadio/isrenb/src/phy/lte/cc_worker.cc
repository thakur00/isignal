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

#include <iomanip>

#include "isrran/common/threads.h"
#include "isrran/isrran.h"

#include "isrenb/hdr/phy/lte/cc_worker.h"

#define Error(fmt, ...)                                                                                                \
  if (ISRRAN_DEBUG_ENABLED)                                                                                            \
  logger.error(fmt, ##__VA_ARGS__)
#define Warning(fmt, ...)                                                                                              \
  if (ISRRAN_DEBUG_ENABLED)                                                                                            \
  logger.warning(fmt, ##__VA_ARGS__)
#define Info(fmt, ...)                                                                                                 \
  if (ISRRAN_DEBUG_ENABLED)                                                                                            \
  logger.info(fmt, ##__VA_ARGS__)
#define Debug(fmt, ...)                                                                                                \
  if (ISRRAN_DEBUG_ENABLED)                                                                                            \
  logger.debug(fmt, ##__VA_ARGS__)

using namespace std;

// Enable this to log SI
//#define LOG_THIS(a) 1

// Enable this one to skip SI-RNTI
#define LOG_THIS(rnti) (rnti != 0xFFFF)

/*********************************************/

using namespace asn1::rrc;

//#define DEBUG_WRITE_FILE

namespace isrenb {
namespace lte {

cc_worker::cc_worker(isrlog::basic_logger& logger) : logger(logger)
{
  reset();
}

cc_worker::~cc_worker()
{
  isrran_softbuffer_tx_free(&temp_mbsfn_softbuffer);
  isrran_enb_dl_free(&enb_dl);
  isrran_enb_ul_free(&enb_ul);

  for (int p = 0; p < ISRRAN_MAX_PORTS; p++) {
    if (signal_buffer_rx[p]) {
      free(signal_buffer_rx[p]);
    }
    if (signal_buffer_tx[p]) {
      free(signal_buffer_tx[p]);
    }
  }

  // Delete all users
  for (auto& it : ue_db) {
    delete it.second;
  }
}

#ifdef DEBUG_WRITE_FILE
FILE* f;
#endif

void cc_worker::init(phy_common* phy_, uint32_t cc_idx_)
{
  phy                         = phy_;
  cc_idx                      = cc_idx_;
  isrran_cell_t    cell       = phy_->get_cell(cc_idx);
  uint32_t         nof_prb    = phy_->get_nof_prb(cc_idx);
  uint32_t         sf_len     = ISRRAN_SF_LEN_PRB(nof_prb);
  isrran_cfr_cfg_t cfr_config = phy_->get_cfr_config();

  // Init cell here
  for (uint32_t p = 0; p < phy->get_nof_ports(cc_idx); p++) {
    signal_buffer_rx[p] = isrran_vec_cf_malloc(2 * sf_len);
    if (!signal_buffer_rx[p]) {
      ERROR("Error allocating memory");
      return;
    }
    isrran_vec_cf_zero(signal_buffer_rx[p], 2 * sf_len);
    signal_buffer_tx[p] = isrran_vec_cf_malloc(2 * sf_len);
    if (!signal_buffer_tx[p]) {
      ERROR("Error allocating memory");
      return;
    }
    isrran_vec_cf_zero(signal_buffer_tx[p], 2 * sf_len);
  }
  if (isrran_enb_dl_init(&enb_dl, signal_buffer_tx, nof_prb)) {
    ERROR("Error initiating ENB DL (cc=%d)", cc_idx);
    return;
  }
  if (isrran_enb_dl_set_cell(&enb_dl, cell)) {
    ERROR("Error initiating ENB DL (cc=%d)", cc_idx);
    return;
  }
  if (isrran_enb_dl_set_cfr(&enb_dl, &cfr_config) < ISRRAN_SUCCESS) {
    ERROR("Error setting the CFR");
    return;
  }
  if (isrran_enb_ul_init(&enb_ul, signal_buffer_rx[0], nof_prb)) {
    ERROR("Error initiating ENB UL");
    return;
  }

  if (isrran_enb_ul_set_cell(&enb_ul, cell, &phy->dmrs_pusch_cfg, nullptr)) {
    ERROR("Error initiating ENB UL");
    return;
  }

  /* Setup SI-RNTI in PHY */
  add_rnti(ISRRAN_SIRNTI);

  /* Setup P-RNTI in PHY */
  add_rnti(ISRRAN_PRNTI);

  /* Setup RA-RNTI in PHY */
  for (int i = ISRRAN_RARNTI_START; i <= ISRRAN_RARNTI_END; i++) {
    add_rnti(i);
  }

  if (isrran_softbuffer_tx_init(&temp_mbsfn_softbuffer, nof_prb)) {
    ERROR("Error initiating soft buffer");
    exit(-1);
  }

  isrran_softbuffer_tx_reset(&temp_mbsfn_softbuffer);

  Info("Component Carrier Worker %d configured cell %d PRB", cc_idx, nof_prb);

  if (phy->params.pusch_8bit_decoder) {
    enb_ul.pusch.llr_is_8bit        = true;
    enb_ul.pusch.ul_sch.llr_is_8bit = true;
  }
  initiated = true;

#ifdef DEBUG_WRITE_FILE
  f = fopen("test.dat", "w");
#endif
}

void cc_worker::reset()
{
  initiated = false;
  ue_db.clear();
}

cf_t* cc_worker::get_buffer_rx(uint32_t antenna_idx)
{
  return signal_buffer_rx[antenna_idx];
}

cf_t* cc_worker::get_buffer_tx(uint32_t antenna_idx)
{
  return signal_buffer_tx[antenna_idx];
}

void cc_worker::set_tti(uint32_t tti_)
{
  tti_rx    = tti_;
  tti_tx_dl = TTI_ADD(tti_rx, FDD_HARQ_DELAY_UL_MS);
  tti_tx_ul = TTI_RX_ACK(tti_rx);
}

int cc_worker::add_rnti(uint16_t rnti)
{
  std::unique_lock<std::mutex> lock(mutex);

  // Create user unless already exists
  if (ue_db.count(rnti) == 0) {
    ue_db[rnti] = new ue(rnti);
  }
  return ISRRAN_SUCCESS;
}

void cc_worker::rem_rnti(uint16_t rnti)
{
  std::lock_guard<std::mutex> lock(mutex);
  if (ue_db.count(rnti)) {
    delete ue_db[rnti];
    ue_db.erase(rnti);
  }
}

uint32_t cc_worker::get_nof_rnti()
{
  std::lock_guard<std::mutex> lock(mutex);
  return ue_db.size();
}

void cc_worker::work_ul(const isrran_ul_sf_cfg_t& ul_sf_cfg, stack_interface_phy_lte::ul_sched_t& ul_grants)
{
  std::lock_guard<std::mutex> lock(mutex);
  ul_sf = ul_sf_cfg;
  logger.set_context(ul_sf.tti);

  // Process UL signal
  isrran_enb_ul_fft(&enb_ul);

  // Decode pending UL grants for the tti they were scheduled
  decode_pusch(ul_grants.pusch, ul_grants.nof_grants);

  // Decode remaining PUCCH ACKs not associated with PUSCH transmission and SR signals
  decode_pucch();
}

void cc_worker::work_dl(const isrran_dl_sf_cfg_t&            dl_sf_cfg,
                        stack_interface_phy_lte::dl_sched_t& dl_grants,
                        stack_interface_phy_lte::ul_sched_t& ul_grants,
                        isrran_mbsfn_cfg_t*                  mbsfn_cfg)
{
  std::lock_guard<std::mutex> lock(mutex);
  dl_sf = dl_sf_cfg;

  // Put base signals (references, PBCH, PCFICH and PSS/SSS) into the resource grid
  isrran_enb_dl_put_base(&enb_dl, &dl_sf);

  // Put DL grants to resource grid. PDSCH data will be encoded as well.
  if (dl_sf_cfg.sf_type == ISRRAN_SF_NORM) {
    encode_pdcch_dl(dl_grants.pdsch, dl_grants.nof_grants);
    encode_pdsch(dl_grants.pdsch, dl_grants.nof_grants);
  } else {
    if (mbsfn_cfg->enable) {
      encode_pmch(dl_grants.pdsch, mbsfn_cfg);
    }
  }

  // Put UL grants to resource grid.
  encode_pdcch_ul(ul_grants.pusch, ul_grants.nof_grants);

  // Put pending PHICH HARQ ACK/NACK indications into subframe
  encode_phich(ul_grants.phich, ul_grants.nof_phich);

  // Generate signal and transmit
  isrran_enb_dl_gen_signal(&enb_dl);

  // Scale if cell gain is set
  float cell_gain_db = phy->get_cell_gain(cc_idx);
  if (std::isnormal(cell_gain_db)) {
    float    scale  = isrran_convert_dB_to_amplitude(cell_gain_db);
    uint32_t sf_len = ISRRAN_SF_LEN_PRB(enb_dl.cell.nof_prb);
    for (uint32_t i = 0; i < enb_dl.cell.nof_ports; i++) {
      isrran_vec_sc_prod_cfc(signal_buffer_tx[i], scale, signal_buffer_tx[i], sf_len);
    }
  }

  // Measure PAPR if flag was triggered
  bool cell_meas_flag = phy->get_cell_measure_trigger(cc_idx);
  if (cell_meas_flag) {
    uint32_t sf_len = ISRRAN_SF_LEN_PRB(enb_dl.cell.nof_prb);
    for (uint32_t i = 0; i < enb_dl.cell.nof_ports; i++) {
      // PAPR measure
      float papr_db = 10.0f * log10(isrran_vec_papr_c(signal_buffer_tx[i], sf_len));
      std::cout << "Cell #" << cc_idx << " port #" << i << " PAPR = " << std::setprecision(4) << papr_db << " dB "
                << std::endl;
    }
    // clear measurement flag on cell
    phy->clear_cell_measure_trigger(cc_idx);
  }
}

bool cc_worker::decode_pusch_rnti(stack_interface_phy_lte::ul_sched_grant_t& ul_grant,
                                  isrran_ul_cfg_t&                           ul_cfg,
                                  isrran_pusch_res_t&                        pusch_res)
{
  uint16_t rnti = ul_grant.dci.rnti;

  // Invalid RNTI
  if (rnti == ISRRAN_INVALID_RNTI) {
    return false;
  }

  // RNTI does not exist
  if (ue_db.count(rnti) == 0) {
    return false;
  }

  // Get UE configuration
  if (phy->ue_db.get_ul_config(rnti, cc_idx, ul_cfg) < ISRRAN_SUCCESS) {
    // It could happen that the UL configuration is missing due to intra-enb HO which is not an error
    Info("Failed retrieving UL configuration for cc=%d rnti=0x%x", cc_idx, rnti);
    return false;
  }

  // Fill UCI configuration
  bool uci_required =
      phy->ue_db.fill_uci_cfg(tti_rx, cc_idx, rnti, ul_grant.dci.cqi_request, true, ul_cfg.pusch.uci_cfg);

  // Compute UL grant
  isrran_pusch_grant_t& grant = ul_cfg.pusch.grant;
  if (isrran_ra_ul_dci_to_grant(&enb_ul.cell, &ul_sf, &ul_cfg.hopping, &ul_grant.dci, &grant)) {
    Error("Computing PUSCH dci for RNTI %x", rnti);
    return false;
  }

  // Handle Format0 adaptive retx
  // Use last TBS for this TB in case of mcs>28
  if (ul_grant.dci.tb.mcs_idx > 28) {
    int rv_idx = grant.tb.rv;
    if (phy->ue_db.get_last_ul_tb(rnti, cc_idx, ul_grant.pid, grant.tb) < ISRRAN_SUCCESS) {
      Error("Error retrieving last UL TB for RNTI %x, CC %d, PID %d", rnti, cc_idx, ul_grant.pid);
      return false;
    }
    grant.tb.rv = rv_idx;
    Info("Adaptive retx: rnti=0x%x, pid=%d, rv_idx=%d, mcs=%d, old_tbs=%d",
         rnti,
         ul_grant.pid,
         grant.tb.rv,
         ul_grant.dci.tb.mcs_idx,
         grant.tb.tbs / 8);
  }

  if (phy->ue_db.set_last_ul_tb(rnti, cc_idx, ul_grant.pid, grant.tb) < ISRRAN_SUCCESS) {
    Error("Error setting last UL TB for RNTI %x, CC %d, PID %d", rnti, cc_idx, ul_grant.pid);
  }

  // Run PUSCH decoder
  ul_cfg.pusch.softbuffers.rx = ul_grant.softbuffer_rx;
  pusch_res.data              = ul_grant.data;
  if (pusch_res.data) {
    if (isrran_enb_ul_get_pusch(&enb_ul, &ul_sf, &ul_cfg.pusch, &pusch_res)) {
      Error("Decoding PUSCH for RNTI %x", rnti);
      return false;
    }
  }
  // Save PHICH scheduling for this user. Each user can have just 1 PUSCH dci per TTI
  ue_db[rnti]->phich_grant.n_prb_lowest = grant.n_prb_tilde[0];
  ue_db[rnti]->phich_grant.n_dmrs       = ul_grant.dci.n_dmrs;

  float snr_db = enb_ul.chest_res.snr_db;

  // Notify MAC of RL status
  if (snr_db >= PUSCH_RL_SNR_DB_TH) {
    // Notify MAC UL channel quality
    phy->stack->snr_info(ul_sf.tti, rnti, cc_idx, snr_db, mac_interface_phy_lte::PUSCH);

    // Notify MAC of Time Alignment only if it enabled and valid measurement, ignore value otherwise
    if (ul_cfg.pusch.meas_ta_en and not std::isnan(enb_ul.chest_res.ta_us) and not std::isinf(enb_ul.chest_res.ta_us)) {
      phy->stack->ta_info(ul_sf.tti, rnti, enb_ul.chest_res.ta_us);
    }
  }

  // Send UCI data to MAC
  if (uci_required) {
    phy->ue_db.send_uci_data(tti_rx, rnti, cc_idx, ul_cfg.pusch.uci_cfg, pusch_res.uci);
  }

  // Save statistics only if data was provided
  if (ul_grant.data != nullptr) {
    // Save metrics stats
    ue_db[rnti]->metrics_ul(ul_grant.dci.tb.mcs_idx,
                            enb_ul.chest_res.epre_dBfs - phy->params.rx_gain_offset,
                            enb_ul.chest_res.snr_db,
                            pusch_res.avg_iterations_block);
  }
  return true;
}

void cc_worker::decode_pusch(stack_interface_phy_lte::ul_sched_grant_t* grants, uint32_t nof_pusch)
{
  // Iterate over all the grants, all the grants need to report MAC the CRC status
  for (uint32_t i = 0; i < nof_pusch; i++) {
    // Get grant itself and RNTI
    stack_interface_phy_lte::ul_sched_grant_t& ul_grant = grants[i];
    uint16_t                                   rnti     = ul_grant.dci.rnti;

    isrran_pusch_res_t pusch_res = {};
    isrran_ul_cfg_t    ul_cfg    = {};

    // Decodes PUSCH for the given grant
    if (!decode_pusch_rnti(ul_grant, ul_cfg, pusch_res)) {
      return;
    }

    // Notify MAC new received data and HARQ Indication value
    if (ul_grant.data != nullptr) {
      // Inform MAC about the CRC result
      phy->stack->crc_info(tti_rx, rnti, cc_idx, ul_cfg.pusch.grant.tb.tbs / 8, pusch_res.crc);
      // Push PDU buffer
      phy->stack->push_pdu(
          tti_rx, rnti, cc_idx, ul_cfg.pusch.grant.tb.tbs / 8, pusch_res.crc, ul_cfg.pusch.grant.L_prb);
      // Logging
      if (logger.info.enabled()) {
        char str[512];
        isrran_pusch_rx_info(&ul_cfg.pusch, &pusch_res, &enb_ul.chest_res, str, sizeof(str));
        logger.info("PUSCH: cc=%d, %s", cc_idx, str);
      }
    }
  }
}

int cc_worker::decode_pucch()
{
  isrran_pucch_res_t pucch_res = {};

  for (auto& iter : ue_db) {
    uint16_t rnti = iter.first;

    // If it's a User RNTI and doesn't have PUSCH grant in this TTI
    if (ISRRAN_RNTI_ISUSER(rnti) and phy->ue_db.is_pcell(rnti, cc_idx)) {
      isrran_ul_cfg_t ul_cfg = {};

      if (phy->ue_db.get_ul_config(rnti, cc_idx, ul_cfg) < ISRRAN_SUCCESS) {
        Error("Error retrieving last UL configuration for RNTI %x, CC %d", rnti, cc_idx);
        continue;
      }

      // Check if user needs to receive PUCCH
      int ret = phy->ue_db.fill_uci_cfg(tti_rx, cc_idx, rnti, false, false, ul_cfg.pucch.uci_cfg);
      if (ret < ISRRAN_SUCCESS) {
        Error("Error retrieving UCI configuration for RNTI %x, CC %d", rnti, cc_idx);
        continue;
      }

      // If ret is more than success, UCI is present
      if (ret > ISRRAN_SUCCESS) {
        // Decode PUCCH
        if (isrran_enb_ul_get_pucch(&enb_ul, &ul_sf, &ul_cfg.pucch, &pucch_res)) {
          Error("Error getting PUCCH");
          continue;
        }

        // Send UCI data to MAC
        if (phy->ue_db.send_uci_data(tti_rx, rnti, cc_idx, ul_cfg.pucch.uci_cfg, pucch_res.uci_data) < ISRRAN_SUCCESS) {
          Error("Error sending UCI data for RNTI %x, CC %d", rnti, cc_idx);
          continue;
        }

        if (pucch_res.detected and pucch_res.ta_valid) {
          phy->stack->ta_info(tti_rx, rnti, pucch_res.ta_us);
          phy->stack->snr_info(tti_rx, rnti, cc_idx, pucch_res.snr_db, mac_interface_phy_lte::PUCCH);
        }

        // Logging
        if (logger.info.enabled()) {
          char str[512];
          isrran_pucch_rx_info(&ul_cfg.pucch, &pucch_res, str, sizeof(str));
          logger.info("PUCCH: cc=%d; %s", cc_idx, str);
        }

        // Save metrics
        if (pucch_res.detected) {
          ue_db[rnti]->metrics_ul_pucch(pucch_res.rssi_dbFs - phy->params.rx_gain_offset,
                                        pucch_res.ni_dbFs - -phy->params.rx_gain_offset,
                                        pucch_res.snr_db);
        }
      }
    }
  }
  return 0;
}

int cc_worker::encode_phich(stack_interface_phy_lte::ul_sched_ack_t* acks, uint32_t nof_acks)
{
  for (uint32_t i = 0; i < nof_acks; i++) {
    if (acks[i].rnti && ue_db.count(acks[i].rnti)) {
      isrran_enb_dl_put_phich(&enb_dl, &ue_db[acks[i].rnti]->phich_grant, acks[i].ack);

      Info("PHICH: rnti=0x%x, hi=%d, I_lowest=%d, n_dmrs=%d, tti_tx_dl=%d",
           acks[i].rnti,
           acks[i].ack,
           ue_db[acks[i].rnti]->phich_grant.n_prb_lowest,
           ue_db[acks[i].rnti]->phich_grant.n_dmrs,
           tti_tx_dl);
    }
  }
  return ISRRAN_SUCCESS;
}

int cc_worker::encode_pdcch_ul(stack_interface_phy_lte::ul_sched_grant_t* grants, uint32_t nof_grants)
{
  for (uint32_t i = 0; i < nof_grants; i++) {
    if (grants[i].needs_pdcch) {
      isrran_dci_cfg_t dci_cfg = {};

      if (phy->ue_db.get_dci_ul_config(grants[i].dci.rnti, cc_idx, dci_cfg) < ISRRAN_SUCCESS) {
        Error("Error retrieving DCI UL configuration for RNTI %x, CC %d", grants[i].dci.rnti, cc_idx);
        continue;
      }

      if (ISRRAN_RNTI_ISUSER(grants[i].dci.rnti)) {
        if (isrran_enb_dl_location_is_common_ncce(&enb_dl, &grants[i].dci.location) &&
            phy->ue_db.is_pcell(grants[i].dci.rnti, cc_idx)) {
          // Disable extended CSI request and ISR request in common SS
          isrran_dci_cfg_set_common_ss(&dci_cfg);
        }
      }

      if (isrran_enb_dl_put_pdcch_ul(&enb_dl, &dci_cfg, &grants[i].dci)) {
        Error("Error putting PUSCH %d", i);
        return ISRRAN_ERROR;
      }

      // Logging
      if (logger.info.enabled()) {
        char str[512];
        isrran_dci_ul_info(&grants[i].dci, str, 512);
        logger.info("PDCCH: cc=%d, rnti=0x%x, %s, tti_tx_dl=%d", cc_idx, grants[i].dci.rnti, str, tti_tx_dl);
      }
    }
  }
  return ISRRAN_SUCCESS;
}

int cc_worker::encode_pdcch_dl(stack_interface_phy_lte::dl_sched_grant_t* grants, uint32_t nof_grants)
{
  for (uint32_t i = 0; i < nof_grants; i++) {
    uint16_t rnti = grants[i].dci.rnti;
    if (rnti) {
      isrran_dci_cfg_t dci_cfg = {};

      if (phy->ue_db.get_dci_dl_config(grants[i].dci.rnti, cc_idx, dci_cfg) < ISRRAN_SUCCESS) {
        Error("Error retrieving DCI DL configuration for RNTI %x, CC %d", grants[i].dci.rnti, cc_idx);
        continue;
      }

      // Detect if the DCI location is in common SS, if that is the case, flag it as common SS
      // This makes possible UE specific DCI fields to be disabled, so it uses a fallback DCI size
      if (ISRRAN_RNTI_ISUSER(grants[i].dci.rnti) && grants[i].dci.format == ISRRAN_DCI_FORMAT1A) {
        if (isrran_enb_dl_location_is_common_ncce(&enb_dl, &grants[i].dci.location) &&
            phy->ue_db.is_pcell(grants[i].dci.rnti, cc_idx)) {
          isrran_dci_cfg_set_common_ss(&dci_cfg);
        }
      }

      if (isrran_enb_dl_put_pdcch_dl(&enb_dl, &dci_cfg, &grants[i].dci)) {
        ERROR("Error putting PDCCH %d", i);
        return ISRRAN_ERROR;
      }

      if (LOG_THIS(rnti) and logger.info.enabled()) {
        // Logging
        char str[512];
        isrran_dci_dl_info(&grants[i].dci, str, 512);
        logger.info("PDCCH: cc=%d, rnti=0x%x, %s, tti_tx_dl=%d", cc_idx, grants[i].dci.rnti, str, tti_tx_dl);
      }
    }
  }
  return 0;
}

int cc_worker::encode_pmch(stack_interface_phy_lte::dl_sched_grant_t* grant, isrran_mbsfn_cfg_t* mbsfn_cfg)
{
  isrran_pmch_cfg_t pmch_cfg;
  ZERO_OBJECT(pmch_cfg);
  isrran_configure_pmch(&pmch_cfg, &enb_dl.cell, mbsfn_cfg);
  isrran_ra_dl_compute_nof_re(&enb_dl.cell, &dl_sf, &pmch_cfg.pdsch_cfg.grant);

  // Set soft buffer
  pmch_cfg.pdsch_cfg.softbuffers.tx[0] = &temp_mbsfn_softbuffer;

  // Encode PMCH
  if (isrran_enb_dl_put_pmch(&enb_dl, &pmch_cfg, grant->data[0])) {
    Error("Error putting PMCH");
    return ISRRAN_ERROR;
  }

  // Logging
  if (logger.info.enabled()) {
    char str[512];
    isrran_pdsch_tx_info(&pmch_cfg.pdsch_cfg, str, 512);
    logger.info("PMCH: %s", str);
  }

  // Save metrics stats
  if (ue_db.count(ISRRAN_MRNTI)) {
    ue_db[ISRRAN_MRNTI]->metrics_dl(mbsfn_cfg->mbsfn_mcs);
  }
  return ISRRAN_SUCCESS;
}

int cc_worker::encode_pdsch(stack_interface_phy_lte::dl_sched_grant_t* grants, uint32_t nof_grants)
{
  /* Scales the Resources Elements affected by the power allocation (p_b) */
  // isrran_enb_dl_prepare_power_allocation(&enb_dl);
  for (uint32_t i = 0; i < nof_grants; i++) {
    uint16_t rnti = grants[i].dci.rnti;

    if (rnti && ue_db.count(rnti)) {
      isrran_dl_cfg_t dl_cfg = {};

      if (phy->ue_db.get_dl_config(rnti, cc_idx, dl_cfg) < ISRRAN_SUCCESS) {
        Error("Error retrieving DCI DL configuration for RNTI %x, CC %d", grants[i].dci.rnti, cc_idx);
        continue;
      }

      // Compute DL grant
      if (isrran_ra_dl_dci_to_grant(
              &enb_dl.cell, &dl_sf, dl_cfg.tm, dl_cfg.pdsch.use_tbs_index_alt, &grants[i].dci, &dl_cfg.pdsch.grant)) {
        Error("Computing DL grant");
        continue;
      }

      // Set soft buffer
      for (uint32_t j = 0; j < ISRRAN_MAX_CODEWORDS; j++) {
        dl_cfg.pdsch.softbuffers.tx[j] = grants[i].softbuffer_tx[j];
      }

      // Encode PDSCH
      if (isrran_enb_dl_put_pdsch(&enb_dl, &dl_cfg.pdsch, grants[i].data)) {
        Error("Error putting PDSCH %d", i);
        return ISRRAN_ERROR;
      }

      // Save pending ACK
      if (ISRRAN_RNTI_ISUSER(rnti)) {
        // Push whole DCI
        phy->ue_db.set_ack_pending(tti_tx_ul, cc_idx, grants[i].dci);
      }

      if (LOG_THIS(rnti) and logger.info.enabled()) {
        // Logging
        char str[512];
        isrran_pdsch_tx_info(&dl_cfg.pdsch, str, 512);
        logger.info("PDSCH: cc=%d, %s, tti_tx_dl=%d", cc_idx, str, tti_tx_dl);
      }

      // Save metrics stats
      ue_db[rnti]->metrics_dl(grants[i].dci.tb[0].mcs_idx);
    } else {
      Error("User rnti=0x%x not found in cc_worker=%d", rnti, cc_idx);
    }
  }

  // isrran_enb_dl_apply_power_allocation(&enb_dl);

  return ISRRAN_SUCCESS;
}

/************ METRICS interface ********************/
uint32_t cc_worker::get_metrics(std::vector<phy_metrics_t>& metrics)
{
  std::lock_guard<std::mutex> lock(mutex);
  uint32_t                    cnt = 0;
  metrics.resize(ue_db.size());
  for (auto& ue : ue_db) {
    if ((ISRRAN_RNTI_ISUSER(ue.first) || ue.first == ISRRAN_MRNTI)) {
      ue.second->metrics_read(&metrics[cnt++]);
    }
  }
  metrics.resize(cnt);
  return cnt;
}

void cc_worker::ue::metrics_read(phy_metrics_t* metrics_)
{
  if (metrics_) {
    *metrics_ = metrics;
  }
  metrics = {};
}

void cc_worker::ue::metrics_dl(uint32_t mcs)
{
  metrics.dl.mcs = ISRRAN_VEC_CMA(mcs, metrics.dl.mcs, metrics.dl.n_samples);
  metrics.dl.n_samples++;
}

void cc_worker::ue::metrics_ul(uint32_t mcs, float rssi, float sinr, float turbo_iters)
{
  if (isnan(rssi)) {
    rssi = 0;
  }
  metrics.ul.mcs         = ISRRAN_VEC_CMA((float)mcs, metrics.ul.mcs, metrics.ul.n_samples);
  metrics.ul.pusch_sinr  = ISRRAN_VEC_CMA((float)sinr, metrics.ul.pusch_sinr, metrics.ul.n_samples);
  metrics.ul.pusch_rssi  = ISRRAN_VEC_CMA((float)rssi, metrics.ul.pusch_rssi, metrics.ul.n_samples);
  metrics.ul.turbo_iters = ISRRAN_VEC_CMA((float)turbo_iters, metrics.ul.turbo_iters, metrics.ul.n_samples);
  metrics.ul.n_samples++;
}

void cc_worker::ue::metrics_ul_pucch(float rssi, float ni, float sinr)
{
  if (isnan(rssi)) {
    rssi = 0;
  }
  metrics.ul.pucch_rssi = ISRRAN_VEC_CMA((float)rssi, metrics.ul.pucch_rssi, metrics.ul.n_samples_pucch);
  metrics.ul.pucch_ni   = ISRRAN_VEC_CMA((float)ni, metrics.ul.pucch_ni, metrics.ul.n_samples_pucch);
  metrics.ul.pucch_sinr = ISRRAN_VEC_CMA((float)sinr, metrics.ul.pucch_sinr, metrics.ul.n_samples_pucch);
  metrics.ul.n_samples_pucch++;
}

int cc_worker::read_ce_abs(float* ce_abs)
{
  int sz = isrran_symbol_sz(phy->get_nof_prb(cc_idx));
  isrran_vec_f_zero(ce_abs, sz);
  int g = (sz - ISRRAN_NRE * phy->get_nof_prb(cc_idx)) / 2;
  isrran_vec_abs_dB_cf(enb_ul.chest_res.ce, -80.0f, &ce_abs[g], ISRRAN_NRE * phy->get_nof_prb(cc_idx));
  return sz;
}

int cc_worker::read_ce_arg(float* ce_arg)
{
  int sz = isrran_symbol_sz(phy->get_nof_prb(cc_idx));
  isrran_vec_f_zero(ce_arg, sz);
  int g = (sz - ISRRAN_NRE * phy->get_nof_prb(cc_idx)) / 2;
  isrran_vec_arg_deg_cf(enb_ul.chest_res.ce, -80.0f, &ce_arg[g], ISRRAN_NRE * phy->get_nof_prb(cc_idx));
  return sz;
}

int cc_worker::read_pusch_d(cf_t* pdsch_d)
{
  int nof_re = enb_ul.pusch.max_re;
  memcpy(pdsch_d, enb_ul.pusch.d, nof_re * sizeof(cf_t));
  return nof_re;
}

int cc_worker::read_pucch_d(cf_t* pdsch_d)
{
  int nof_re = ISRRAN_PUCCH_MAX_BITS / 2;
  memcpy(pdsch_d, enb_ul.pucch.z_tmp, nof_re * sizeof(cf_t));
  return nof_re;
}

} // namespace lte
} // namespace isrenb
