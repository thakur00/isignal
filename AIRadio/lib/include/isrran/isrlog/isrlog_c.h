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

#ifndef ISRLOG_ISRLOG_C_H
#define ISRLOG_ISRLOG_C_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Common types.
 */
typedef int                               isrlog_bool;
typedef struct isrlog_opaque_sink         isrlog_sink;
typedef struct isrlog_opaque_log_channel  isrlog_log_channel;
typedef struct isrlog_opaque_basic_logger isrlog_logger;

/**
 * This function initializes the logging framework. It must be called before
 * any log entry is generated.
 * NOTE: Calling this function more than once has no side effects.
 */
void isrlog_init(void);

/**
 * Installs the specified sink to be used as the default one by new log
 * channels and loggers.
 * The initial default sink writes to stdout.
 */
void isrlog_set_default_sink(isrlog_sink* s);

/**
 * Returns the instance of the default sink being used.
 */
isrlog_sink* isrlog_get_default_sink(void);

/**
 * Returns an instance of a log_channel with the specified id that writes to
 * the default sink using the default log channel configuration.
 * NOTE: Any '#' characters in the id will get removed.
 */
isrlog_log_channel* isrlog_fetch_log_channel(const char* id);

/**
 * Finds a log channel with the specified id string in the repository. On
 * success returns a pointer to the requested log channel, otherwise NULL.
 */
isrlog_log_channel* isrlog_find_log_channel(const char* id);

/**
 * Controls whether the specified channel accepts incoming log entries.
 */
void isrlog_set_log_channel_enabled(isrlog_log_channel* channel, isrlog_bool enabled);

/**
 * Returns 1 if the specified channel is accepting incoming log entries,
 * otherwise 0.
 */
isrlog_bool isrlog_is_log_channel_enabled(isrlog_log_channel* channel);

/**
 * Returns the id string of the specified channel.
 */
const char* isrlog_get_log_channel_id(isrlog_log_channel* channel);

/**
 * Logs the provided log entry using the specified log channel. When the channel
 * is disabled the log entry wil be discarded.
 * NOTE: Only printf style formatting is supported when using the C API.
 */
void isrlog_log(isrlog_log_channel* channel, const char* fmt, ...);

/**
 * Returns an instance of a basic logger (see basic_logger type) with the
 * specified id string. All logger channels will write into the default sink.
 */
isrlog_logger* isrlog_fetch_default_logger(const char* id);

/**
 * Finds a logger with the specified id string in the repository. On success
 * returns a pointer to the requested log channel, otherwise NULL.
 */
isrlog_logger* isrlog_find_default_logger(const char* id);

/**
 * These functions log the provided log entry using the specified logger.
 * Entries are automatically discarded depending on the configured level of the
 * logger.
 * NOTE: Only printf style formatting is supported when using the C API.
 */
void isrlog_debug(isrlog_logger* log, const char* fmt, ...);
void isrlog_info(isrlog_logger* log, const char* fmt, ...);
void isrlog_warning(isrlog_logger* log, const char* fmt, ...);
void isrlog_error(isrlog_logger* log, const char* fmt, ...);

/**
 * Returns the id string of the specified logger.
 */
const char* isrlog_get_logger_id(isrlog_logger* log);

typedef enum {
  isrlog_lvl_none,    /**< disable logger */
  isrlog_lvl_error,   /**< error logging level */
  isrlog_lvl_warning, /**< warning logging level */
  isrlog_lvl_info,    /**< info logging level */
  isrlog_lvl_debug    /**< debug logging level */
} isrlog_log_levels;

/**
 * Sets the logging level into the specified logger.
 */
void isrlog_set_logger_level(isrlog_logger* log, isrlog_log_levels lvl);

/**
 * Finds a sink with the specified id string in the repository. On
 * success returns a pointer to the requested sink, otherwise NULL.
 */
isrlog_sink* isrlog_find_sink(const char* id);

/**
 * Returns an instance of a sink that writes to the stdout stream.
 */
isrlog_sink* isrlog_fetch_stdout_sink(void);

/**
 * Returns an instance of a sink that writes to the stderr stream.
 */
isrlog_sink* isrlog_fetch_stderr_sink(void);

/**
 * Returns an instance of a sink that writes into a file in the specified path.
 * Specifying a max_size value different to zero will make the sink create a
 * new file each time the current file exceeds this value. The units of
 * max_size are bytes.
 * Setting force_flush to true will flush the sink after every write.
 * NOTE: Any '#' characters in the id will get removed.
 */
isrlog_sink* isrlog_fetch_file_sink(const char* path, size_t max_size, isrlog_bool force_flush);

#ifdef __cplusplus
}
#endif

#endif
