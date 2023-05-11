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

#ifndef ISRLOG_DETAIL_LOG_ENTRY_H
#define ISRLOG_DETAIL_LOG_ENTRY_H

#include "isrran/isrlog/detail/log_entry_metadata.h"
#include "isrran/isrlog/detail/support/thread_utils.h"

namespace isrlog {

class sink;

namespace detail {

/// This command flushes all the messages pending in the backend.
struct flush_backend_cmd {
  shared_variable<bool>& completion_flag;
  std::vector<sink*>     sinks;
};

/// This structure packs all the required data required to create a log entry in
/// the backend.
//: TODO: replace this object using a real command pattern when we have a raw
// memory queue for passing entries.
struct log_entry {
  sink*                                                                          s;
  std::function<void(log_entry_metadata&& metadata, fmt::memory_buffer& buffer)> format_func;
  log_entry_metadata                                                             metadata;
  std::unique_ptr<flush_backend_cmd>                                             flush_cmd;
};

} // namespace detail

} // namespace isrlog

#endif // ISRLOG_DETAIL_LOG_ENTRY_H
