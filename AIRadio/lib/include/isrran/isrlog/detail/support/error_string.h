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

#ifndef ISRLOG_DETAIL_SUPPORT_ERROR_STRING_H
#define ISRLOG_DETAIL_SUPPORT_ERROR_STRING_H

#include <string>

namespace isrlog {

namespace detail {

/// This is a lightweight error class that encapsulates a string for error
/// reporting.
class error_string
{
  std::string error;

public:
  error_string() = default;

  /*implicit*/ error_string(std::string error) : error(std::move(error)) {}
  /*implicit*/ error_string(const char* error) : error(error) {}

  /// Returns the error string.
  const std::string& get_error() const { return error; }

  explicit operator bool() const { return !error.empty(); }
};

} // namespace detail

} // namespace isrlog

#endif // ISRLOG_DETAIL_SUPPORT_ERROR_STRING_H
