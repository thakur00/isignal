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

#ifndef ISRRAN_UE_SYNC_NR_H
#define ISRRAN_UE_SYNC_NR_H

#include "isrran/phy/common/timestamp.h"
#include "isrran/phy/sync/ssb.h"

#define ISRRAN_RECV_CALLBACK_TEMPLATE(NAME) int (*NAME)(void*, cf_t**, uint32_t, isrran_timestamp_t*)

/**
 * @brief Describes NR UE synchronization object internal states
 */
typedef enum ISRRAN_API {
  ISRRAN_UE_SYNC_NR_STATE_IDLE = 0, ///< Initial state, the object has no configuration
  ISRRAN_UE_SYNC_NR_STATE_FIND,     ///< State just after configuring, baseband is not aligned
  ISRRAN_UE_SYNC_NR_STATE_TRACK     ///< Baseband is aligned with subframes
} isrran_ue_sync_nr_state_t;

/**
 * @brief Describes a UE sync NR object arguments
 */
typedef struct ISRRAN_API {
  // Memory allocation constraints
  double                      max_srate_hz;    ///< Maximum sampling rate in Hz, set to zero to use default
  isrran_subcarrier_spacing_t min_scs;         ///< Minimum subcarrier spacing
  uint32_t                    nof_rx_channels; ///< Number of receive channels, set to 0 for 1

  // Enable/Disable features
  bool disable_cfo; ///< Set to true for disabling the CFO compensation close loop

  // Signal detection thresholds and averaging coefficients
  float pbch_dmrs_thr; ///< NR-PBCH DMRS threshold for blind decoding, set to 0 for default
  float cfo_alpha;     ///< Exponential Moving Average (EMA) alpha coefficient for CFO

  // Receive callback
  void* recv_obj;                               ///< Receive object
  ISRRAN_RECV_CALLBACK_TEMPLATE(recv_callback); ///< Receive callback
} isrran_ue_sync_nr_args_t;

/**
 * @brief Describes a UE sync NR object configuration
 */
typedef struct ISRRAN_API {
  isrran_ssb_cfg_t ssb;  ///< SSB configuration
  uint32_t         N_id; ///< Physical cell identifier
} isrran_ue_sync_nr_cfg_t;

/**
 * @brief  Describes a UE sync NR object
 */
typedef struct ISRRAN_API {
  // State
  isrran_ue_sync_nr_state_t     state;                 ///< Internal state
  int32_t                       next_rf_sample_offset; ///< Next RF sample offset
  uint32_t                      ssb_idx;               ///< Tracking SSB candidate index
  uint32_t                      sf_idx;                ///< Current subframe index (0-9)
  uint32_t                      sfn;                   ///< Current system frame number (0-1023)
  isrran_csi_trs_measurements_t feedback;              ///< Feedback measurements

  // Components
  isrran_ssb_t ssb;        ///< SSB internal object
  cf_t**       tmp_buffer; ///< Temporal buffer pointers

  // Initialised arguments
  uint32_t nof_rx_channels;                     ///< Number of receive channels
  bool     disable_cfo;                         ///< Set to true for disabling the CFO compensation close loop
  float    cfo_alpha;                           ///< Exponential Moving Average (EMA) alpha coefficient for CFO
  void*    recv_obj;                            ///< Receive object
  ISRRAN_RECV_CALLBACK_TEMPLATE(recv_callback); ///< Receive callback

  // Current configuration
  uint32_t N_id;     ///< Current physical cell identifier
  double   srate_hz; ///< Current sampling rate in Hz
  uint32_t sf_sz;    ///< Current subframe size

  // Metrics
  float cfo_hz;       ///< Current CFO in Hz
  float avg_delay_us; ///< Current average delay
} isrran_ue_sync_nr_t;

/**
 * @brief Describes a UE sync NR zerocopy outcome
 */
typedef struct ISRRAN_API {
  bool               in_sync;   ///< Indicates whether the received baseband is synchronized
  uint32_t           sf_idx;    ///< Subframe index
  uint32_t           sfn;       ///< System Frame Number
  isrran_timestamp_t timestamp; ///< Last received timestamp
  float              cfo_hz;    ///< Current CFO in Hz
  float              delay_us;  ///< Current average delay in microseconds
} isrran_ue_sync_nr_outcome_t;

/**
 * @brief Initialises a UE sync NR object
 * @param q NR UE synchronization object
 * @param[in] args NR UE synchronization initialization arguments
 * @return ISRRAN_SUCCESS if no error occurs, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ue_sync_nr_init(isrran_ue_sync_nr_t* q, const isrran_ue_sync_nr_args_t* args);

/**
 * @brief Deallocate an NR UE synchronization object
 * @param q NR UE synchronization object
 */
ISRRAN_API void isrran_ue_sync_nr_free(isrran_ue_sync_nr_t* q);

/**
 * @brief Configures a UE sync NR object
 * @param q NR UE synchronization object
 * @param[in] cfg NR UE synchronization configuration
 * @return ISRRAN_SUCCESS if no error occurs, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ue_sync_nr_set_cfg(isrran_ue_sync_nr_t* q, const isrran_ue_sync_nr_cfg_t* cfg);

/**
 * @brief Runs the NR UE synchronization object, tries to find and track the configured SSB leaving in buffer the
 * received baseband subframe
 * @param q NR UE synchronization object
 * @param buffer 2D complex buffer
 * @param outcome zerocopy outcome
 * @return ISRRAN_SUCCESS if no error occurs, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ue_sync_nr_zerocopy(isrran_ue_sync_nr_t* q, cf_t** buffer, isrran_ue_sync_nr_outcome_t* outcome);

/**
 * @brief Feedback Channel State Information from Tracking Reference Signals into a UE synchronization object
 * @param q NR UE synchronization object
 * @param measurements CSI-TRS feedback measurement
 * @return ISRRAN_SUCCESS if no error occurs, ISRRAN_ERROR code otherwise
 */
ISRRAN_API int isrran_ue_sync_nr_feedback(isrran_ue_sync_nr_t* q, const isrran_csi_trs_measurements_t* measurements);

#endif // ISRRAN_UE_SYNC_NR_H
