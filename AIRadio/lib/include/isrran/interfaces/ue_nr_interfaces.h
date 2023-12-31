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

#ifndef ISRRAN_UE_NR_INTERFACES_H
#define ISRRAN_UE_NR_INTERFACES_H

#include "isrran/common/interfaces_common.h"
#include "isrran/common/phy_cfg_nr.h"
#include "isrran/interfaces/mac_interface_types.h"
#include <array>
#include <set>
#include <string>

namespace isrue {

class rrc_interface_phy_nr
{
public:
  virtual void in_sync()                            = 0;
  virtual void out_of_sync()                        = 0;
  virtual void set_phy_config_complete(bool status) = 0;

  /**
   * @brief Describes a cell search result
   */
  struct cell_search_result_t {
    bool                          cell_found   = false;
    uint32_t                      ssb_arfcn    = 0;  ///< SSB center ARFCN
    uint32_t                      pci          = 0;  ///< Physical Cell Identifier
    isrran_pbch_msg_nr_t          pbch_msg     = {}; ///< Packed PBCH message for the upper layers
    isrran_csi_trs_measurements_t measurements = {}; ///< Measurements from SSB block
  };

  /**
   * @brief Informs RRC about cell search process completion
   * @param result Cell search result completion
   */
  virtual void cell_search_found_cell(const cell_search_result_t& result) = 0;

  /**
   * @brief Describes a cell select result
   */
  struct cell_select_result_t {
    enum {
      ERROR = 0,    ///< The cell selection procedure failed due a to an invalid configuration
      UNSUCCESSFUL, ///< The cell selection failed to find and synchronise the SSB
      SUCCESSFUL,   ///< The cell selection was successful, resulting in a camping state
    } status;
  };

  /**
   * @brief Informs RRC about cell select process completion
   * @param result Cell select result completion
   */
  virtual void cell_select_completed(const cell_select_result_t& result) = 0;
};

class mac_interface_phy_nr
{
public:
  /// For DL, PDU buffer is allocated and passed to MAC in tb_decoded()
  typedef struct {
    bool                    enabled;    /// Whether or not PHY should attempt to decode PDSCH
    isrran_softbuffer_rx_t* softbuffer; /// Pointer to softbuffer to use
  } tb_dl_t;

  /// Struct provided by MAC with all necessary information for PHY
  typedef struct {
    tb_dl_t tb; // only single TB in DL
  } tb_action_dl_t;

  typedef struct {
    uint16_t rnti;
    uint32_t tti;
    uint8_t  pid; // HARQ process ID
    uint8_t  rv;  // Redundancy Version
    uint8_t  ndi; // Raw new data indicator extracted from DCI
    uint32_t tbs; // Transport block size in Bytes
  } mac_nr_grant_dl_t;

  typedef struct {
    uint32_t                     rx_slot_idx; // Slot when DL TB has been decoded
    isrran::unique_byte_buffer_t payload;     // TB when decoded successfully, nullptr otherwise
    bool                         ack;         // HARQ information
  } tb_action_dl_result_t;

  // UL grant as conveyed between PHY and MAC
  typedef struct {
    uint16_t rnti;
    uint32_t tti;
    uint8_t  pid;          // HARQ process ID
    uint32_t tbs;          // transport block size in Bytes
    uint8_t  ndi;          // Raw new data indicator extracted from DCI
    uint8_t  rv;           // Redundancy Version
    bool     is_rar_grant; // True if grant comes from RAR
  } mac_nr_grant_ul_t;

  /// For UL, payload buffer remains in MAC
  typedef struct {
    bool                    enabled;
    uint32_t                rv;
    isrran::byte_buffer_t*  payload;
    isrran_softbuffer_tx_t* softbuffer;
  } tb_ul_t;

  /// Struct provided by MAC with all necessary information for PHY
  typedef struct {
    tb_ul_t tb; // only single TB in UL
  } tb_action_ul_t;

  // Query the MAC for the current RNTI to look for
  struct sched_rnti_t {
    uint16_t           id;
    isrran_rnti_type_t type;
  };
  virtual sched_rnti_t get_dl_sched_rnti_nr(const uint32_t tti) = 0;
  virtual sched_rnti_t get_ul_sched_rnti_nr(const uint32_t tti) = 0;

  /**
   * @brief Indicate reception of DL grant to MAC
   *
   * The TB buffer is allocated in the PHY and handed as unique_ptr to MAC.
   *
   * @param cc_idx The carrier index on which the grant has been received
   * @param grant  Reference to the grant
   * @param action Pointer to the TB action to be filled by MAC
   */
  virtual void new_grant_dl(const uint32_t cc_idx, const mac_nr_grant_dl_t& grant, tb_action_dl_t* action) = 0;

  /**
   * Indicate decoding of PDSCH
   *
   * @param cc_idx The index of the carrier for which the PDSCH has been decoded
   * @param grant  The original DL grant
   * @param result Payload (if any) and ack information
   */
  virtual void tb_decoded(const uint32_t cc_idx, const mac_nr_grant_dl_t& grant, tb_action_dl_result_t result) = 0;

  /**
   * @brief Indicate reception of UL grant to MAC
   *
   * Buffer for resulting MAC PDU is provided and managed (owned) by MAC and is passed as pointer in ul_action
   *
   * @param cc_idx The carrier index on which the grant has been received
   * @param grant  Reference to the grant
   * @param action Pointer to the TB action to be filled by MAC
   */
  virtual void new_grant_ul(const uint32_t cc_idx, const mac_nr_grant_ul_t& grant, tb_action_ul_t* action) = 0;

  /**
   * @brief Indicate the successful transmission of a PRACH.
   * @param tti  The TTI from the PHY viewpoint at which the PRACH was sent over-the-air (not to the radio).
   * @param s_id The index of the first OFDM symbol of the specified PRACH (0 <= s_id < 14).
   * @param t_id The index of the first slot of the specified PRACH (0 <= t_id < 80).
   * @param f_id The index of the specified PRACH in the frequency domain (0 <= f_id < 8).
   * @param ul_carrier_id The UL carrier used for Msg1 transmission (0 for NUL carrier, and 1 for SUL carrier).
   */
  virtual void prach_sent(uint32_t tti, uint32_t s_id, uint32_t t_id, uint32_t f_id, uint32_t ul_carrier_id) = 0;

  /**
   * @brief Indicate a valid SR transmission occasion on the valid PUCCH resource for SR configured; and the SR
   * transmission occasion does not overlap with a measurement gap; and the PUCCH resource for the SR transmission
   * occasion does not overlap with a UL-SCH resource;
   * @param tti  The TTI from the PHY viewpoint at which the SR occasion was sent over-the-air (not to the radio).
   */
  virtual bool sr_opportunity(uint32_t tti, uint32_t sr_id, bool meas_gap, bool ul_sch_tx) = 0;
};

class mac_interface_rrc_nr
{
public:
  virtual void reset() = 0;
  // Config calls that return ISRRAN_SUCCESS or ISRRAN_ERROR
  virtual int  setup_lcid(const isrran::logical_channel_config_t& config) = 0;
  virtual int  set_config(const isrran::bsr_cfg_nr_t& bsr_cfg)            = 0;
  virtual int  set_config(const isrran::sr_cfg_nr_t& sr_cfg)              = 0;
  virtual int  set_config(const isrran::dl_harq_cfg_nr_t& dl_hrq_cfg)     = 0;
  virtual void set_config(const isrran::rach_cfg_nr_t& rach_cfg_nr)       = 0;
  virtual int  add_tag_config(const isrran::tag_cfg_nr_t& tag_cfg)        = 0;
  virtual int  set_config(const isrran::phr_cfg_nr_t& phr_cfg)            = 0;
  virtual int  remove_tag_config(const uint32_t tag_id)                   = 0;

  // RRC triggers MAC ra procedure
  virtual void start_ra_procedure() = 0;

  // RRC informs MAC about the (randomly) selected ID used for contention-based RA
  virtual void set_contention_id(const uint64_t ue_identity) = 0;

  // RRC informs MAC about new UE identity for contention-free RA
  virtual bool set_crnti(const uint16_t crnti) = 0;

  // RRC informs MAC to start/stop search for BCCH messages
  virtual void bcch_search(bool enabled) = 0;
};

struct phy_args_nr_t {
  uint32_t               rf_channel_offset     = 0; ///< Specifies the RF channel the NR carrier shall fill
  uint32_t               nof_carriers          = 1;
  uint32_t               max_nof_prb           = 106;
  double                 srate_hz              = 23.04e6;
  uint32_t               nof_phy_threads       = 3;
  uint32_t               worker_cpu_mask       = 0;
  int                    slot_recv_thread_prio = 0; /// Specifies the slot receive thread priority, RT by default
  int                    workers_thread_prio   = 2; /// Specifies the workers thread priority, RT by default
  isrran::phy_log_args_t log                   = {};
  isrran_ue_dl_nr_args_t dl                    = {};
  isrran_ue_ul_nr_args_t ul                    = {};
  std::set<uint32_t>     fixed_sr              = {1};
  uint32_t               fix_wideband_cqi      = 15; ///< Set to a non-zero value for fixing the wide-band CQI report
  bool                   store_pdsch_ko        = false;
  float                  trs_epre_ema_alpha    = 0.1f; ///< EPRE measurement exponential average alpha
  float                  trs_rsrp_ema_alpha    = 0.1f; ///< RSRP measurement exponential average alpha
  float                  trs_sinr_ema_alpha    = 0.1f; ///< SINR measurement exponential average alpha
  float                  trs_cfo_ema_alpha     = 0.1f; ///< RSRP measurement exponential average alpha
  bool                   enable_worker_cfo     = true; ///< Enable/Disable open loop CFO correction at the workers

  phy_args_nr_t()
  {
    dl.nof_rx_antennas        = 1;
    dl.nof_max_prb            = 106;
    dl.pdsch.max_prb          = 106;
    dl.pdsch.max_layers       = 1;
    dl.pdsch.measure_evm      = true;
    dl.pdsch.measure_time     = true;
    dl.pdsch.sch.disable_simd = false;
    dl.pdsch.sch.max_nof_iter = 10;
    ul.nof_max_prb            = 106;
    ul.pusch.max_prb          = 106;
    ul.pusch.max_layers       = 1;
    ul.pusch.measure_time     = true;
    ul.pusch.sch.disable_simd = false;

    // fixed_sr.insert(0); // Enable SR_id = 0 by default for testing purposes
  }
};

class phy_interface_mac_nr
{
public:
  // MAC informs PHY about UL grant included in RAR PDU
  virtual int set_rar_grant(uint32_t                                       rar_slot_idx,
                            std::array<uint8_t, ISRRAN_RAR_UL_GRANT_NBITS> packed_ul_grant,
                            uint16_t                                       rnti,
                            isrran_rnti_type_t                             rnti_type) = 0;

  /// Instruct PHY to send PRACH in the next occasion.
  virtual void send_prach(const uint32_t prach_occasion,
                          const int      preamble_index,
                          const float    preamble_received_target_power,
                          const float    ta_base_sec = 0.0f) = 0;

  /// Apply TA command after RAR
  virtual void set_timeadv_rar(uint32_t tti, uint32_t ta_cmd) = 0;

  /// Apply TA command after MAC CE
  virtual void set_timeadv(uint32_t tti, uint32_t ta_cmd) = 0;

  /**
   * @brief Query PHY if there is a valid PUCCH SR resource configured for a given SR identifier
   * @param sr_id SR identifier
   * @return True if there is a valid PUCCH resource configured
   */
  virtual bool has_valid_sr_resource(uint32_t sr_id) = 0;

  /**
   * @brief Clear any configured downlink assignments and uplink grants
   */
  virtual void clear_pending_grants() = 0;
};

class phy_interface_rrc_nr
{
public:
  virtual bool set_config(const isrran::phy_cfg_nr_t& cfg) = 0;

  /**
   * @brief Describe the possible NR standalone physical layer possible states
   */
  typedef enum {
    PHY_NR_STATE_IDLE = 0,    ///< There is no process going on
    PHY_NR_STATE_CELL_SEARCH, ///< Cell search is currently in progress
    PHY_NR_STATE_CELL_SELECT, ///< Cell selection is in progress or it is camped on a cell
    PHY_NR_STATE_CAMPING
  } phy_nr_state_t;

  /**
   * @brief Retrieves the physical layer state
   * @return
   */
  virtual phy_nr_state_t get_state() = 0;

  /**
   * @brief Stops the ongoing process and transitions to IDLE
   */
  virtual void reset_nr() = 0;

  /**
   * @brief Describes cell search arguments
   */
  struct cell_search_args_t {
    double                      center_freq_hz;
    double                      ssb_freq_hz;
    isrran_subcarrier_spacing_t ssb_scs;
    isrran_ssb_pattern_t        ssb_pattern;
    isrran_duplex_mode_t        duplex_mode;
  };

  /**
   * @brief Start cell search
   * @param args Cell Search arguments
   * @return true if the physical layer started successfully the cell search process
   */
  virtual bool start_cell_search(const cell_search_args_t& req) = 0;

  /**
   * @brief Describes cell select arguments
   */
  struct cell_select_args_t {
    isrran_ssb_cfg_t    ssb_cfg;
    isrran_carrier_nr_t carrier;
  };

  /**
   * @brief Start cell search
   * @param args Cell Search arguments
   * @return true if the physical layer started successfully the cell search process
   */
  virtual bool start_cell_select(const cell_select_args_t& req) = 0;
};

// Combined interface for PHY to access stack (MAC and RRC)
class stack_interface_phy_nr : public mac_interface_phy_nr, public rrc_interface_phy_nr
{
public:
  /* Indicate new TTI */
  virtual void run_tti(const uint32_t tti, const uint32_t tti_jump) = 0;
};

// Combined interface for stack (MAC and RRC) to access PHY
class phy_interface_stack_nr : public phy_interface_mac_nr, public phy_interface_rrc_nr
{};

} // namespace isrue

#endif // ISRRAN_UE_NR_INTERFACES_H
