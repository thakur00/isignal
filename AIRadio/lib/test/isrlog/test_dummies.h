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

#ifndef TEST_DUMMIES
#define TEST_DUMMIES

#include "isrran/isrlog/detail/log_backend.h"
#include "isrran/isrlog/shared_types.h"
#include "isrran/isrlog/sink.h"

namespace test_dummies {

/// A Dummy implementation of a formatter.
class log_formatter_dummy : public isrlog::log_formatter
{
public:
  void format(isrlog::detail::log_entry_metadata&& metadata, fmt::memory_buffer& buffer) override {}

  std::unique_ptr<log_formatter> clone() const override { return nullptr; }

private:
  void format_context_begin(const isrlog::detail::log_entry_metadata& md,
                            fmt::string_view                          ctx_name,
                            unsigned                                  size,
                            fmt::memory_buffer&                       buffer) override
  {}
  void format_context_end(const isrlog::detail::log_entry_metadata& md,
                          fmt::string_view                          ctx_name,
                          fmt::memory_buffer&                       buffer) override
  {}
  void
  format_metric_set_begin(fmt::string_view set_name, unsigned size, unsigned level, fmt::memory_buffer& buffer) override
  {}
  void format_metric_set_end(fmt::string_view set_name, unsigned level, fmt::memory_buffer& buffer) override {}
  void format_list_begin(fmt::string_view list_name, unsigned size, unsigned level, fmt::memory_buffer& buffer) override
  {}
  void format_list_end(fmt::string_view list_name, unsigned level, fmt::memory_buffer& buffer) override {}
  void format_metric(fmt::string_view    metric_name,
                     fmt::string_view    metric_value,
                     fmt::string_view    metric_units,
                     isrlog::metric_kind kind,
                     unsigned            level,
                     fmt::memory_buffer& buffer) override
  {}
};

/// A Dummy implementation of a sink.
class sink_dummy : public isrlog::sink
{
public:
  sink_dummy() : sink(std::unique_ptr<isrlog::log_formatter>(new log_formatter_dummy)) {}

  isrlog::detail::error_string write(isrlog::detail::memory_buffer buffer) override { return {}; }

  isrlog::detail::error_string flush() override { return {}; }
};

/// A Dummy implementation of the log backend.
class backend_dummy : public isrlog::detail::log_backend
{
public:
  void start(isrlog::backend_priority priority) override {}

  bool push(isrlog::detail::log_entry&& entry) override { return true; }

  bool is_running() const override { return true; }

  fmt::dynamic_format_arg_store<fmt::printf_context>* alloc_arg_store() override { return nullptr; }
};

} // namespace test_dummies

#endif // TEST_DUMMIES
