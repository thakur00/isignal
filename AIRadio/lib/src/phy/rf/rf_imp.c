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

#include "rf_dev.h"
#include "isrran/phy/rf/rf.h"
#include "isrran/phy/utils/debug.h"
#include <dlfcn.h>
#include <string.h>

int rf_get_available_devices(char** devnames, int max_strlen)
{
  int i = 0;
  while (rf_plugins[i] != NULL) {
    if (rf_plugins[i]->rf_api != NULL) {
      strncpy(devnames[i], rf_plugins[i]->rf_api->name, max_strlen);
    }
    i++;
  }
  return i;
}

int isrran_rf_set_rx_gain_th(isrran_rf_t* rf, double gain)
{
  pthread_mutex_lock(&rf->mutex);
  if (gain > rf->cur_rx_gain + 2 || gain < rf->cur_rx_gain - 2) {
    rf->new_rx_gain = gain;
    pthread_cond_signal(&rf->cond);
  }
  pthread_mutex_unlock(&rf->mutex);
  return ISRRAN_SUCCESS;
}

void isrran_rf_set_tx_rx_gain_offset(isrran_rf_t* rf, double offset)
{
  rf->tx_rx_gain_offset = offset;
}

/* This thread listens for set_rx_gain commands to the USRP */
static void* thread_gain_fcn(void* h)
{
  isrran_rf_t* rf = (isrran_rf_t*)h;

  while (rf->thread_gain_run) {
    pthread_mutex_lock(&rf->mutex);
    while (rf->cur_rx_gain == rf->new_rx_gain && rf->thread_gain_run) {
      pthread_cond_wait(&rf->cond, &rf->mutex);
    }
    if (rf->new_rx_gain != rf->cur_rx_gain) {
      isrran_rf_set_rx_gain(h, rf->new_rx_gain);
      rf->cur_rx_gain = isrran_rf_get_rx_gain(h);
      rf->new_rx_gain = rf->cur_rx_gain;
    }
    if (rf->tx_gain_same_rx) {
      isrran_rf_set_tx_gain(h, rf->cur_rx_gain + rf->tx_rx_gain_offset);
    }
    pthread_mutex_unlock(&rf->mutex);
  }
  return NULL;
}

/* Create auxiliary thread and mutexes for AGC */
int isrran_rf_start_gain_thread(isrran_rf_t* rf, bool tx_gain_same_rx)
{
  rf->tx_gain_same_rx   = tx_gain_same_rx;
  rf->tx_rx_gain_offset = 0.0;
  if (pthread_mutex_init(&rf->mutex, NULL)) {
    return -1;
  }
  if (pthread_cond_init(&rf->cond, NULL)) {
    return -1;
  }
  rf->thread_gain_run = true;
  if (pthread_create(&rf->thread_gain, NULL, thread_gain_fcn, rf)) {
    perror("pthread_create");
    rf->thread_gain_run = false;
    return -1;
  }
  return 0;
}

const char* isrran_rf_get_devname(isrran_rf_t* rf)
{
  return ((rf_dev_t*)rf->dev)->name;
}

int isrran_rf_open_devname(isrran_rf_t* rf, const char* devname, char* args, uint32_t nof_channels)
{
  rf->thread_gain_run = false;

  bool no_rf_devs_detected = true;
  printf("Supported RF device list:");
  for (unsigned int i = 0; rf_plugins[i] && rf_plugins[i]->rf_api; i++) {
    no_rf_devs_detected = false;
    printf(" %s", rf_plugins[i]->rf_api->name);
  }
  printf("%s\n", no_rf_devs_detected ? " <none>" : "");

  // Try to open the device if name is provided
  if (devname && devname[0] != '\0') {
    int i = 0;
    while (rf_plugins[i] != NULL) {
      if (rf_plugins[i]->rf_api) {
        if (!strcasecmp(rf_plugins[i]->rf_api->name, devname)) {
          rf->dev = rf_plugins[i]->rf_api;
          return rf_plugins[i]->rf_api->isrran_rf_open_multi(args, &rf->handler, nof_channels);
        }
      }
      i++;
    }

    ERROR("RF device '%s' not found. Please check the available isrRAN CMAKE options to verify if this device is being "
          "detected in your system",
          devname);
    // provided device not found, abort
    return ISRRAN_ERROR;
  }

  // auto-mode, try to open in order of apperance in rf_plugins[] array
  int i = 0;
  while (rf_plugins[i] != NULL && rf_plugins[i]->rf_api != NULL) {
    printf("Trying to open RF device '%s'\n", rf_plugins[i]->rf_api->name);
    if (!rf_plugins[i]->rf_api->isrran_rf_open_multi(args, &rf->handler, nof_channels)) {
      rf->dev = rf_plugins[i]->rf_api;
      printf("RF device '%s' successfully opened\n", rf_plugins[i]->rf_api->name);
      return ISRRAN_SUCCESS;
    }
    printf("Unable to open RF device '%s'\n", rf_plugins[i]->rf_api->name);
    i++;
  }

  ERROR(
      "Failed to open a RF frontend device. Please check the available isrRAN CMAKE options to verify what RF frontend "
      "devices have been detected in your system");
  return ISRRAN_ERROR;
}

int isrran_rf_open_file(isrran_rf_t* rf, FILE** rx_files, FILE** tx_files, uint32_t nof_channels, uint32_t base_srate)
{
  rf->dev = &isrran_rf_dev_file;

  // file abstraction has custom "open" function with file-related args
  return rf_file_open_file(&rf->handler, rx_files, tx_files, nof_channels, base_srate);
}

const char* isrran_rf_name(isrran_rf_t* rf)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_devname(rf->handler);
}

int isrran_rf_start_rx_stream(isrran_rf_t* rf, bool now)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_start_rx_stream(rf->handler, now);
}

int isrran_rf_stop_rx_stream(isrran_rf_t* rf)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_stop_rx_stream(rf->handler);
}

void isrran_rf_flush_buffer(isrran_rf_t* rf)
{
  ((rf_dev_t*)rf->dev)->isrran_rf_flush_buffer(rf->handler);
}

bool isrran_rf_has_rssi(isrran_rf_t* rf)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_has_rssi(rf->handler);
}

float isrran_rf_get_rssi(isrran_rf_t* rf)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_get_rssi(rf->handler);
}

void isrran_rf_suppress_stdout(isrran_rf_t* rf)
{
  ((rf_dev_t*)rf->dev)->isrran_rf_suppress_stdout(rf->handler);
}

void isrran_rf_register_error_handler(isrran_rf_t* rf, isrran_rf_error_handler_t error_handler, void* arg)
{
  ((rf_dev_t*)rf->dev)->isrran_rf_register_error_handler(rf->handler, error_handler, arg);
}

int isrran_rf_open(isrran_rf_t* h, char* args)
{
  return isrran_rf_open_devname(h, NULL, args, 1);
}

int isrran_rf_open_multi(isrran_rf_t* h, char* args, uint32_t nof_channels)
{
  return isrran_rf_open_devname(h, NULL, args, nof_channels);
}

int isrran_rf_close(isrran_rf_t* rf)
{
  // Stop gain thread
  pthread_mutex_lock(&rf->mutex);
  if (rf->thread_gain_run) {
    rf->thread_gain_run = false;
  }
  pthread_mutex_unlock(&rf->mutex);
  pthread_cond_signal(&rf->cond);
  if (rf->thread_gain) {
    pthread_join(rf->thread_gain, NULL);
  }

  return ((rf_dev_t*)rf->dev)->isrran_rf_close(rf->handler);
}

double isrran_rf_set_rx_srate(isrran_rf_t* rf, double freq)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_set_rx_srate(rf->handler, freq);
}

int isrran_rf_set_rx_gain(isrran_rf_t* rf, double gain)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_set_rx_gain(rf->handler, gain);
}

int isrran_rf_set_rx_gain_ch(isrran_rf_t* rf, uint32_t ch, double gain)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_set_rx_gain_ch(rf->handler, ch, gain);
}

double isrran_rf_get_rx_gain(isrran_rf_t* rf)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_get_rx_gain(rf->handler);
}

double isrran_rf_get_tx_gain(isrran_rf_t* rf)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_get_tx_gain(rf->handler);
}

isrran_rf_info_t* isrran_rf_get_info(isrran_rf_t* rf)
{
  isrran_rf_info_t* ret = NULL;
  if (((rf_dev_t*)rf->dev)->isrran_rf_get_info) {
    ret = ((rf_dev_t*)rf->dev)->isrran_rf_get_info(rf->handler);
  }
  return ret;
}

double isrran_rf_set_rx_freq(isrran_rf_t* rf, uint32_t ch, double freq)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_set_rx_freq(rf->handler, ch, freq);
}

int isrran_rf_recv(isrran_rf_t* rf, void* data, uint32_t nsamples, bool blocking)
{
  return isrran_rf_recv_with_time(rf, data, nsamples, blocking, NULL, NULL);
}

int isrran_rf_recv_multi(isrran_rf_t* rf, void** data, uint32_t nsamples, bool blocking)
{
  return isrran_rf_recv_with_time_multi(rf, data, nsamples, blocking, NULL, NULL);
}

int isrran_rf_recv_with_time(isrran_rf_t* rf,
                             void*        data,
                             uint32_t     nsamples,
                             bool         blocking,
                             time_t*      secs,
                             double*      frac_secs)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_recv_with_time(rf->handler, data, nsamples, blocking, secs, frac_secs);
}

int isrran_rf_recv_with_time_multi(isrran_rf_t* rf,
                                   void**       data,
                                   uint32_t     nsamples,
                                   bool         blocking,
                                   time_t*      secs,
                                   double*      frac_secs)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_recv_with_time_multi(rf->handler, data, nsamples, blocking, secs, frac_secs);
}

int isrran_rf_set_tx_gain(isrran_rf_t* rf, double gain)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_set_tx_gain(rf->handler, gain);
}

int isrran_rf_set_tx_gain_ch(isrran_rf_t* rf, uint32_t ch, double gain)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_set_tx_gain_ch(rf->handler, ch, gain);
}

double isrran_rf_set_tx_srate(isrran_rf_t* rf, double freq)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_set_tx_srate(rf->handler, freq);
}

double isrran_rf_set_tx_freq(isrran_rf_t* rf, uint32_t ch, double freq)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_set_tx_freq(rf->handler, ch, freq);
}

void isrran_rf_get_time(isrran_rf_t* rf, time_t* secs, double* frac_secs)
{
  return ((rf_dev_t*)rf->dev)->isrran_rf_get_time(rf->handler, secs, frac_secs);
}

int isrran_rf_sync(isrran_rf_t* rf)
{
  int ret = ISRRAN_ERROR;

  if (((rf_dev_t*)rf->dev)->isrran_rf_sync_pps) {
    ((rf_dev_t*)rf->dev)->isrran_rf_sync_pps(rf->handler);

    ret = ISRRAN_SUCCESS;
  }

  return ret;
}

int isrran_rf_send_timed3(isrran_rf_t* rf,
                          void*        data,
                          int          nsamples,
                          time_t       secs,
                          double       frac_secs,
                          bool         has_time_spec,
                          bool         blocking,
                          bool         is_start_of_burst,
                          bool         is_end_of_burst)
{
  return ((rf_dev_t*)rf->dev)
      ->isrran_rf_send_timed(
          rf->handler, data, nsamples, secs, frac_secs, has_time_spec, blocking, is_start_of_burst, is_end_of_burst);
}

int isrran_rf_send_timed_multi(isrran_rf_t* rf,
                               void**       data,
                               int          nsamples,
                               time_t       secs,
                               double       frac_secs,
                               bool         blocking,
                               bool         is_start_of_burst,
                               bool         is_end_of_burst)
{
  return ((rf_dev_t*)rf->dev)
      ->isrran_rf_send_timed_multi(
          rf->handler, data, nsamples, secs, frac_secs, true, blocking, is_start_of_burst, is_end_of_burst);
}

int isrran_rf_send_multi(isrran_rf_t* rf,
                         void**       data,
                         int          nsamples,
                         bool         blocking,
                         bool         is_start_of_burst,
                         bool         is_end_of_burst)
{
  return ((rf_dev_t*)rf->dev)
      ->isrran_rf_send_timed_multi(
          rf->handler, data, nsamples, 0, 0, false, blocking, is_start_of_burst, is_end_of_burst);
}

int isrran_rf_send(isrran_rf_t* rf, void* data, uint32_t nsamples, bool blocking)
{
  return isrran_rf_send2(rf, data, nsamples, blocking, true, true);
}

int isrran_rf_send2(isrran_rf_t* rf,
                    void*        data,
                    uint32_t     nsamples,
                    bool         blocking,
                    bool         start_of_burst,
                    bool         end_of_burst)
{
  return isrran_rf_send_timed3(rf, data, nsamples, 0, 0, false, blocking, start_of_burst, end_of_burst);
}

int isrran_rf_send_timed(isrran_rf_t* rf, void* data, int nsamples, time_t secs, double frac_secs)
{
  return isrran_rf_send_timed2(rf, data, nsamples, secs, frac_secs, true, true);
}

int isrran_rf_send_timed2(isrran_rf_t* rf,
                          void*        data,
                          int          nsamples,
                          time_t       secs,
                          double       frac_secs,
                          bool         is_start_of_burst,
                          bool         is_end_of_burst)
{
  return isrran_rf_send_timed3(rf, data, nsamples, secs, frac_secs, true, true, is_start_of_burst, is_end_of_burst);
}

#ifdef ENABLE_RF_PLUGINS
static void unload_plugin(isrran_rf_plugin_t* rf_plugin)
{
  if (rf_plugin == NULL) {
    return;
  }
  if (rf_plugin->dl_handle != NULL) {
    rf_plugin->rf_api = NULL;
    dlclose(rf_plugin->dl_handle);
    rf_plugin->dl_handle = NULL;
  }
}

static int load_plugin(isrran_rf_plugin_t* rf_plugin)
{
  if (rf_plugin->rf_api != NULL) {
    // already loaded
    return ISRRAN_SUCCESS;
  }

  rf_plugin->dl_handle = dlopen(rf_plugin->plugin_name, RTLD_NOW);
  if (rf_plugin->dl_handle == NULL) {
    // Not an error, if loading failed due to missing dependencies.
    // Flag this plugin as not available and return SUCCESS.
    // Note: as this function is called before log-level is configured, use plain printf for any messages < ERROR
    printf("Skipping RF plugin %s: %s\n", rf_plugin->plugin_name, dlerror());
    rf_plugin->rf_api = NULL;
    return ISRRAN_SUCCESS;
  }

  // clear errors
  dlerror();
  char* err = NULL;

  // load symbols
  int (*register_plugin)(rf_dev_t * *rf_api) = dlsym(rf_plugin->dl_handle, "register_plugin");
  if ((err = dlerror()) != NULL) {
    ERROR("Error loading symbol '%s': %s", "register_plugin", err);
    goto clean_exit;
  }

  // register plugin
  int ret = register_plugin(&rf_plugin->rf_api);
  if (ret != ISRRAN_SUCCESS) {
    ERROR("Failed to register RF API for plugin %s", rf_plugin->plugin_name);
    goto clean_exit;
  }
  return ISRRAN_SUCCESS;
clean_exit:
  unload_plugin(rf_plugin);
  return ISRRAN_ERROR;
}
#endif /* ENABLE_RF_PLUGINS */

int isrran_rf_load_plugins()
{
#ifdef ENABLE_RF_PLUGINS
  for (unsigned int i = 0; rf_plugins[i]; i++) {
    if (load_plugin(rf_plugins[i]) != ISRRAN_SUCCESS) {
      return ISRRAN_ERROR;
    }
  }

  printf("Active RF plugins:");
  for (unsigned int i = 0; rf_plugins[i]; i++) {
    if (rf_plugins[i]->dl_handle != NULL) {
      printf(" %s", rf_plugins[i]->plugin_name);
    }
  }
  printf("\n");

  printf("Inactive RF plugins:");
  for (unsigned int i = 0; rf_plugins[i]; i++) {
    if (rf_plugins[i]->dl_handle == NULL) {
      printf(" %s", rf_plugins[i]->plugin_name);
    }
  }
  printf("\n");

#endif /* ENABLE_RF_PLUGINS */
  return ISRRAN_SUCCESS;
}

// Search and load plugins when this library is loaded (shared) or right before main (static)
void __attribute__((constructor)) init()
{
  if (isrran_rf_load_plugins() != ISRRAN_SUCCESS) {
    ERROR("Failed to load RF plugins");
  }
}
