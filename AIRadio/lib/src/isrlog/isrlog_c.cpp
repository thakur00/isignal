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

#include "isrran/isrlog/isrlog_c.h"
#include "isrran/isrlog/isrlog.h"
#include <cstdarg>

using namespace isrlog;

template <typename To, typename From>
static inline To* c_cast(From* x)
{
  return reinterpret_cast<To*>(x);
}

/// Helper to format the input argument list writing it into a channel.
static void log_to(log_channel& c, const char* fmt, std::va_list args)
{
  char buffer[1024];
  std::vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
  c(buffer);
}

void isrlog_init(void)
{
  init();
}

void isrlog_set_default_sink(isrlog_sink* s)
{
  assert(s && "Expected a valid sink");
  set_default_sink(*c_cast<sink>(s));
}

isrlog_sink* isrlog_get_default_sink(void)
{
  return c_cast<isrlog_sink>(&get_default_sink());
}

isrlog_log_channel* isrlog_fetch_log_channel(const char* id)
{
  return c_cast<isrlog_log_channel>(&fetch_log_channel(id));
}

isrlog_log_channel* isrlog_find_log_channel(const char* id)
{
  return c_cast<isrlog_log_channel>(find_log_channel(id));
}

void isrlog_set_log_channel_enabled(isrlog_log_channel* channel, isrlog_bool enabled)
{
  assert(channel && "Expected a valid channel");
  c_cast<log_channel>(channel)->set_enabled(enabled);
}

isrlog_bool isrlog_is_log_channel_enabled(isrlog_log_channel* channel)
{
  assert(channel && "Expected a valid channel");
  return c_cast<log_channel>(channel)->enabled();
}

const char* isrlog_get_log_channel_id(isrlog_log_channel* channel)
{
  assert(channel && "Expected a valid channel");
  return c_cast<log_channel>(channel)->id().c_str();
}

void isrlog_log(isrlog_log_channel* channel, const char* fmt, ...)
{
  assert(channel && "Expected a valid channel");

  std::va_list args;
  va_start(args, fmt);
  log_to(*c_cast<log_channel>(channel), fmt, args);
  va_end(args);
}

isrlog_logger* isrlog_fetch_default_logger(const char* id)
{
  return c_cast<isrlog_logger>(&fetch_basic_logger(id));
}

isrlog_logger* isrlog_find_default_logger(const char* id)
{
  return c_cast<isrlog_logger>(find_logger<basic_logger>(id));
}

void isrlog_debug(isrlog_logger* log, const char* fmt, ...)
{
  assert(log && "Expected a valid logger");

  std::va_list args;
  va_start(args, fmt);
  log_to(c_cast<basic_logger>(log)->debug, fmt, args);
  va_end(args);
}

void isrlog_info(isrlog_logger* log, const char* fmt, ...)
{
  assert(log && "Expected a valid logger");

  std::va_list args;
  va_start(args, fmt);
  log_to(c_cast<basic_logger>(log)->info, fmt, args);
  va_end(args);
}

void isrlog_warning(isrlog_logger* log, const char* fmt, ...)
{
  assert(log && "Expected a valid logger");

  std::va_list args;
  va_start(args, fmt);
  log_to(c_cast<basic_logger>(log)->warning, fmt, args);
  va_end(args);
}

void isrlog_error(isrlog_logger* log, const char* fmt, ...)
{
  assert(log && "Expected a valid logger");

  std::va_list args;
  va_start(args, fmt);
  log_to(c_cast<basic_logger>(log)->error, fmt, args);
  va_end(args);
}

const char* isrlog_get_logger_id(isrlog_logger* log)
{
  assert(log && "Expected a valid logger");
  return c_cast<basic_logger>(log)->id().c_str();
}

/// Translate the C API level enum to basic_levels.
static basic_levels convert_c_enum_to_basic_levels(isrlog_log_levels lvl)
{
  switch (lvl) {
    case isrlog_lvl_none:
      return basic_levels::none;
    case isrlog_lvl_debug:
      return basic_levels::debug;
    case isrlog_lvl_info:
      return basic_levels::info;
    case isrlog_lvl_warning:
      return basic_levels ::warning;
    case isrlog_lvl_error:
      return basic_levels::error;
  }

  assert(false && "Invalid enum value");
  return basic_levels::none;
}

void isrlog_set_logger_level(isrlog_logger* log, isrlog_log_levels lvl)
{
  assert(log && "Expected a valid logger");
  c_cast<basic_logger>(log)->set_level(convert_c_enum_to_basic_levels(lvl));
}

isrlog_sink* isrlog_find_sink(const char* id)
{
  return c_cast<isrlog_sink>(find_sink(id));
}

isrlog_sink* isrlog_fetch_stdout_sink(void)
{
  return c_cast<isrlog_sink>(&fetch_stdout_sink());
}

isrlog_sink* isrlog_fetch_stderr_sink(void)
{
  return c_cast<isrlog_sink>(&fetch_stderr_sink());
}

isrlog_sink* isrlog_fetch_file_sink(const char* path, size_t max_size, isrlog_bool force_flush)
{
  return c_cast<isrlog_sink>(&fetch_file_sink(path, max_size, force_flush));
}
