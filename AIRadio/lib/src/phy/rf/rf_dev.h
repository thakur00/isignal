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

#include "isrran/phy/rf/rf.h"
#include <stdbool.h>

/* Define implementation for UHD */
#ifdef ENABLE_UHD
#ifdef ENABLE_RF_PLUGINS
static isrran_rf_plugin_t plugin_uhd = {"libisrran_rf_uhd.so", NULL, NULL};
#else
#include "rf_uhd_imp.h"
static isrran_rf_plugin_t plugin_uhd   = {"", NULL, &isrran_rf_dev_uhd};
#endif
#endif

/* Define implementation for bladeRF */
#ifdef ENABLE_BLADERF
#ifdef ENABLE_RF_PLUGINS
static isrran_rf_plugin_t plugin_blade = {"libisrran_rf_blade.so", NULL, NULL};
#else
#include "rf_blade_imp.h"
static isrran_rf_plugin_t plugin_blade = {"", NULL, &isrran_rf_dev_blade};
#endif
#endif

/* Define implementation for SoapySDR */
#ifdef ENABLE_SOAPYSDR
#ifdef ENABLE_RF_PLUGINS
static isrran_rf_plugin_t plugin_soapy = {"libisrran_rf_soapy.so", NULL, NULL};
#else
#include "rf_soapy_imp.h"
static isrran_rf_plugin_t plugin_soapy = {"", NULL, &isrran_rf_dev_soapy};
#endif
#endif

/* Define implementation for ZeroMQ */
#ifdef ENABLE_ZEROMQ
#ifdef ENABLE_RF_PLUGINS
static isrran_rf_plugin_t plugin_zmq = {"libisrran_rf_zmq.so", NULL, NULL};
#else
#include "rf_zmq_imp.h"
static isrran_rf_plugin_t plugin_zmq   = {"", NULL, &isrran_rf_dev_zmq};
#endif
#endif

/* Define implementation for file-based RF */
#include "rf_file_imp.h"
static isrran_rf_plugin_t plugin_file = {"", NULL, &isrran_rf_dev_file};

/* Define implementation for Sidekiq */
#ifdef ENABLE_SIDEKIQ
#ifdef ENABLE_RF_PLUGINS
static isrran_rf_plugin_t plugin_skiq = {"libisrran_rf_skiq.so", NULL, NULL};
#else
#include "rf_skiq_imp.h"
static isrran_rf_plugin_t plugin_skiq  = {"", NULL, &isrran_rf_dev_skiq};
#endif
#endif

//#define ENABLE_DUMMY_DEV

#ifdef ENABLE_DUMMY_DEV
int dummy_rcv()
{
  usleep(100000);
  return 1;
}
void dummy_fnc() {}

static rf_dev_t isrran_rf_dev_dummy = {
    "dummy",   dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc,
    dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc,
    dummy_fnc, dummy_fnc, dummy_fnc, dummy_rcv, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc, dummy_fnc};
static isrran_rf_plugin_t plugin_dummy = {"", NULL, &isrran_rf_dev_dummy};

#endif

/**
 * Collection of all currently available RF plugins
 */
static isrran_rf_plugin_t* rf_plugins[] = {

#ifdef ENABLE_UHD
    &plugin_uhd,
#endif
#ifdef ENABLE_SOAPYSDR
    &plugin_soapy,
#endif
#ifdef ENABLE_BLADERF
    &plugin_blade,
#endif
#ifdef ENABLE_ZEROMQ
    &plugin_zmq,
#endif
#ifdef ENABLE_SIDEKIQ
    &plugin_skiq,
#endif
#ifdef ENABLE_DUMMY_DEV
    &plugin_dummy,
#endif
    &plugin_file,
    NULL};
