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

#ifndef ISRRAN_GNB_INTERFACES_H
#define ISRRAN_GNB_INTERFACES_H

#include "isrran/isrran.h"

#include "isrran/common/interfaces_common.h"
#include "isrran/common/security.h"
#include "isrran/interfaces/pdcp_interface_types.h"
#include "isrran/interfaces/rlc_interface_types.h"
// EUTRA interfaces that are used unmodified
#include "isrran/interfaces/enb_rrc_interface_pdcp.h"
#include "isrran/interfaces/enb_rrc_interface_rlc.h"

namespace isrenb {

struct sched_nr_ue_cfg_t;

/*****************************
 *      RLC INTERFACES
 ****************************/
class rlc_interface_mac_nr
{
public:
  /* MAC calls RLC to get RLC segment of nof_bytes length.
   * Segmentation happens in this function. RLC PDU is stored in payload. */
  virtual int  read_pdu(uint16_t rnti, uint32_t lcid, uint8_t* payload, uint32_t nof_bytes) = 0;
  virtual void read_pdu_pcch(uint8_t* payload, uint32_t buffer_size)                        = 0;

  /* MAC calls RLC to push an RLC PDU. This function is called from an independent MAC thread.
   * PDU gets placed into the buffer and higher layer thread gets notified. */
  virtual void write_pdu(uint16_t rnti, uint32_t lcid, uint8_t* payload, uint32_t nof_bytes) = 0;
};
class rlc_interface_pdcp_nr
{
public:
  /* PDCP calls RLC to push an RLC SDU. SDU gets placed into the RLC buffer and MAC pulls
   * RLC PDUs according to TB size. */
  virtual void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t sdu) = 0;
  virtual bool rb_is_um(uint16_t rnti, uint32_t lcid)                                    = 0;
  virtual bool sdu_queue_is_full(uint16_t rnti, uint32_t lcid)                           = 0;
};
class rlc_interface_rrc_nr
{
public:
  virtual void clear_buffer(uint16_t rnti)                                               = 0;
  virtual void add_user(uint16_t rnti)                                                   = 0;
  virtual void rem_user(uint16_t rnti)                                                   = 0;
  virtual void add_bearer(uint16_t rnti, uint32_t lcid, isrran::rlc_config_t cnfg)       = 0;
  virtual void add_bearer_mrb(uint16_t rnti, uint32_t lcid)                              = 0;
  virtual void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t sdu) = 0;
};

/*****************************
 *      PDCP INTERFACES
 ****************************/
class pdcp_interface_rlc_nr
{
public:
  /* RLC calls PDCP to push a PDCP PDU. */
  virtual void write_pdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t sdu)               = 0;
  virtual void notify_delivery(uint16_t rnti, uint32_t lcid, const isrran::pdcp_sn_vector_t& pdcp_sns) = 0;
  virtual void notify_failure(uint16_t rnti, uint32_t lcid, const isrran::pdcp_sn_vector_t& pdcp_sns)  = 0;
};

class pdcp_interface_rrc_nr
{
public:
  virtual void reset(uint16_t rnti)                                                                       = 0;
  virtual void add_user(uint16_t rnti)                                                                    = 0;
  virtual void rem_user(uint16_t rnti)                                                                    = 0;
  virtual void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t sdu)                  = 0;
  virtual void add_bearer(uint16_t rnti, uint32_t lcid, isrran::pdcp_config_t cnfg)                       = 0;
  virtual void config_security(uint16_t rnti, uint32_t lcid, const isrran::as_security_config_t& sec_cfg) = 0;
  virtual void enable_integrity(uint16_t rnti, uint32_t lcid)                                             = 0;
  virtual void enable_encryption(uint16_t rnti, uint32_t lcid)                                            = 0;
};

class pdcp_interface_sdap_nr
{
public:
  virtual void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t pdu) = 0;
};

/*****************************
 *      SDAP INTERFACES
 ****************************/
class sdap_interface_pdcp_nr
{
public:
  virtual void write_pdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t pdu) = 0;
};
class sdap_interface_gtpu_nr
{
public:
  virtual void write_sdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t pdu) = 0;
};

/*****************************
 *      GTPU INTERFACES
 ****************************/
class gtpu_interface_rrc_nr
{
public:
};

class gtpu_interface_sdap_nr
{
public:
  virtual void write_pdu(uint16_t rnti, uint32_t lcid, isrran::unique_byte_buffer_t pdu) = 0;
};

/*****************************
 *      MAC internal INTERFACES
 ****************************/

class mac_interface_pdu_demux_nr
{
public:
  // Called by PDU handler from Stack thread to store Msg3 content (According to O-RAN WG8 v3.0, Sec. 9.2.2.3.5 MAC)
  virtual void store_msg3(uint16_t rnti, isrran::unique_byte_buffer_t pdu) = 0;
};

/*****************************
 *      RRC INTERFACES
 ****************************/
class rrc_interface_phy_nr
{};
class rrc_interface_mac_nr
{
public:
  // Provides MIB packed message
  virtual int read_pdu_bcch_bch(const uint32_t tti, isrran::byte_buffer_t& buffer)   = 0;
  virtual int read_pdu_bcch_dlsch(uint32_t sib_index, isrran::byte_buffer_t& buffer) = 0;

  /// User management
  virtual int  add_user(uint16_t rnti, uint32_t pcell_cc_idx)    = 0;
  virtual int  update_user(uint16_t new_rnti, uint16_t old_rnti) = 0;
  virtual void set_activity_user(uint16_t rnti)                  = 0;
};

// NR interface is almost identical to EUTRA version
class rrc_interface_rlc_nr : public rrc_interface_rlc
{
public:
  virtual void        read_pdu_pcch(uint8_t* payload, uint32_t payload_size) = 0;
  virtual const char* get_rb_name(uint32_t lcid)                             = 0;
};

// NR interface identical to EUTRA version
class rrc_interface_pdcp_nr : public rrc_interface_pdcp
{};

class phy_interface_rrc_nr
{
public:
  /**
   * @brief Describes physical layer configuration common among all the UEs for a given cell
   */
  struct common_cfg_t {
    isrran_carrier_nr_t   carrier;
    isrran_pdcch_cfg_nr_t pdcch;
    isrran_prach_cfg_t    prach;
    isrran_ssb_cfg_t      ssb;
    isrran_duplex_mode_t  duplex_mode;
  };

  virtual int set_common_cfg(const common_cfg_t& common_cfg) = 0;
};

class phy_interface_mac_nr
{
public:
  // TBD
};

// Combined interface for stack (MAC and RRC) to access PHY
class phy_interface_stack_nr : public phy_interface_rrc_nr, public phy_interface_mac_nr
{
public:
  // TBD
};

class stack_interface_mac
{
public:
  ///< MAC calls stack to inform about new PDUs having arrived to be further processes in stack thread
  virtual void process_pdus() = 0;
};

class mac_interface_phy_nr
{
public:
  const static int MAX_SSB              = 4;
  const static int MAX_GRANTS           = 4;
  const static int MAX_PUCCH_MSG        = 64;
  const static int MAX_PUCCH_CANDIDATES = 2;
  const static int MAX_NZP_CSI_RS       = 4;

  struct pdcch_dl_t {
    isrran_dci_cfg_nr_t dci_cfg = {};
    isrran_dci_dl_nr_t  dci     = {};
  };

  struct pdcch_ul_t {
    isrran_dci_cfg_nr_t dci_cfg = {};
    isrran_dci_ul_nr_t  dci     = {};
  };

  struct pdsch_t {
    isrran_sch_cfg_nr_t                               sch  = {}; ///< PDSCH configuration
    std::array<isrran::byte_buffer_t*, ISRRAN_MAX_TB> data = {}; ///< Data pointer
  };

  struct ssb_t {
    isrran_pbch_msg_nr_t pbch_msg = {};
  };

  struct dl_sched_t {
    isrran::bounded_vector<ssb_t, MAX_SSB>                               ssb;
    isrran::bounded_vector<pdcch_dl_t, MAX_GRANTS>                       pdcch_dl;
    isrran::bounded_vector<pdcch_ul_t, MAX_GRANTS>                       pdcch_ul;
    isrran::bounded_vector<pdsch_t, MAX_GRANTS>                          pdsch;
    isrran::bounded_vector<isrran_csi_rs_nzp_resource_t, MAX_NZP_CSI_RS> nzp_csi_rs;
  };

  struct pusch_t {
    uint32_t            pid = 0;  ///< HARQ process ID
    isrran_sch_cfg_nr_t sch = {}; ///< PUSCH configuration
  };

  /**
   * @brief Describes a possible PUCCH candidate transmission
   * @note The physical layer shall try decoding all the possible PUCCH candidates and report back to the stack the
   * strongest of the candidates. This is thought to be used in the case of SR opportunities in which the UE could
   * transmit HARQ-ACK in two possible resources.
   */
  struct pucch_candidate_t {
    isrran_uci_cfg_nr_t        uci_cfg;  ///< UCI configuration for the opportunity
    isrran_pucch_nr_resource_t resource; ///< PUCCH resource to use
  };

  struct pucch_t {
    isrran_pucch_nr_common_cfg_t                                    pucch_cfg;  ///< UE dedicated PUCCH configuration
    isrran::bounded_vector<pucch_candidate_t, MAX_PUCCH_CANDIDATES> candidates; ///< PUCCH candidates to decode
  };

  struct ul_sched_t {
    isrran::bounded_vector<pusch_t, MAX_GRANTS> pusch;
    isrran::bounded_vector<pucch_t, MAX_GRANTS> pucch;
  };

  struct pucch_info_t {
    isrran_uci_data_nr_t          uci_data; ///< RNTI is available under cfg->pucch->rnti
    isrran_csi_trs_measurements_t csi;      ///< DMRS based signal Channel State Information (CSI)
  };

  struct pusch_info_t {
    // Context
    uint16_t rnti;    ///< UE temporal identifier
    uint32_t pid = 0; ///< HARQ process ID

    // SCH and UCI payload information
    isrran_pusch_res_nr_t pusch_data;
    isrran_uci_cfg_nr_t   uci_cfg; ///< Provides UCI configuration, so stack does not need to keep the pending state

    // Actual SCH PDU
    isrran::unique_byte_buffer_t pdu = nullptr;

    // PUSCH signal measurements
    isrran_csi_trs_measurements_t csi; ///< DMRS based signal Channel State Information (CSI)
  };

  struct rach_info_t {
    uint32_t slot_index;
    uint32_t preamble;
    uint32_t time_adv;
  };

  virtual int         slot_indication(const isrran_slot_cfg_t& slot_cfg)                            = 0;
  virtual dl_sched_t* get_dl_sched(const isrran_slot_cfg_t& slot_cfg)                               = 0;
  virtual ul_sched_t* get_ul_sched(const isrran_slot_cfg_t& slot_cfg)                               = 0;
  virtual int         pucch_info(const isrran_slot_cfg_t& slot_cfg, const pucch_info_t& pucch_info) = 0;
  virtual int         pusch_info(const isrran_slot_cfg_t& slot_cfg, pusch_info_t& pusch_info)       = 0;
  virtual void        rach_detected(const rach_info_t& rach_info)                                   = 0;
};

class stack_interface_phy_nr : public mac_interface_phy_nr
{};

} // namespace isrenb

#endif // ISRRAN_GNB_INTERFACES_H
