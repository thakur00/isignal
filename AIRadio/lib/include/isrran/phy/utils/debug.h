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
 *  File:         debug.h
 *
 *  Description:  Debug output utilities.
 *
 *  Reference:
 *****************************************************************************/

#ifndef ISRRAN_DEBUG_H
#define ISRRAN_DEBUG_H

#include "phy_logger.h"
#include "isrran/config.h"
#include <stdio.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define ISRRAN_VERBOSE_DEBUG 2
#define ISRRAN_VERBOSE_INFO 1
#define ISRRAN_VERBOSE_NONE 0

ISRRAN_API void get_time_interval(struct timeval* tdata);

#define ISRRAN_DEBUG_ENABLED 1

ISRRAN_API int  get_isrran_verbose_level(void);
ISRRAN_API void set_isrran_verbose_level(int level);
ISRRAN_API void increase_isrran_verbose_level(void);

ISRRAN_API bool is_handler_registered(void);
ISRRAN_API void set_handler_enabled(bool enable);

#define ISRRAN_VERBOSE_ISINFO() (get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO)
#define ISRRAN_VERBOSE_ISDEBUG() (get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG)
#define ISRRAN_VERBOSE_ISNONE() (get_isrran_verbose_level() == ISRRAN_VERBOSE_NONE)

#define PRINT_DEBUG set_isrran_verbose_level(ISRRAN_VERBOSE_DEBUG)
#define PRINT_INFO set_isrran_verbose_level(ISRRAN_VERBOSE_INFO)
#define PRINT_NONE set_isrran_verbose_level(ISRRAN_VERBOSE_NONE)

#define DEBUG(_fmt, ...)                                                                                               \
  do {                                                                                                                 \
    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_DEBUG && !is_handler_registered()) {      \
      fprintf(stdout, "[DEBUG]: " _fmt "\n", ##__VA_ARGS__);                                                           \
    } else {                                                                                                           \
      isrran_phy_log_print(LOG_LEVEL_DEBUG_S, _fmt, ##__VA_ARGS__);                                                    \
    }                                                                                                                  \
  } while (0)

#define INFO(_fmt, ...)                                                                                                \
  do {                                                                                                                 \
    if (ISRRAN_DEBUG_ENABLED && get_isrran_verbose_level() >= ISRRAN_VERBOSE_INFO && !is_handler_registered()) {       \
      fprintf(stdout, "[INFO]: " _fmt "\n", ##__VA_ARGS__);                                                            \
    } else {                                                                                                           \
      isrran_phy_log_print(LOG_LEVEL_INFO_S, _fmt, ##__VA_ARGS__);                                                     \
    }                                                                                                                  \
  } while (0)

#if CMAKE_BUILD_TYPE == Debug
/* In debug mode, it prints out the  */
#define ERROR(_fmt, ...)                                                                                               \
  do {                                                                                                                 \
    if (!is_handler_registered()) {                                                                                    \
      fprintf(stderr, "\e[31m%s:%d: " _fmt "\e[0m\n", __FILE__, __LINE__, ##__VA_ARGS__);                              \
    } else {                                                                                                           \
      isrran_phy_log_print(LOG_LEVEL_ERROR_S, _fmt, ##__VA_ARGS__);                                                    \
    }                                                                                                                  \
  } while (0)
#else
#define ERROR(_fmt, ...)                                                                                               \
  if (!is_handler_registered()) {                                                                                      \
    fprintf(stderr, "[ERROR in %s]:" _fmt "\n", __FUNCTION__, ##__VA_ARGS__);                                          \
  } else {                                                                                                             \
    isrran_phy_log_print(LOG_LEVEL_ERROR, _fmt, ##__VA_ARGS__);                                                        \
  }    //
#endif /* CMAKE_BUILD_TYPE==Debug */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ISRRAN_DEBUG_H
