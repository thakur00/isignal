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

#ifndef ISRLOG_DETAIL_SUPPORT_MEMORY_BUFFER_H
#define ISRLOG_DETAIL_SUPPORT_MEMORY_BUFFER_H

#include <string>

namespace isrlog {

namespace detail {

/// This class wraps a read-only and non owning memory block, providing simple
/// methods to access its contents.
class memory_buffer
{
  const char* const buffer;
  const size_t      length;

public:
  memory_buffer(const char* buffer, size_t length) : buffer(buffer), length(length) {}

  explicit memory_buffer(const std::string& s) : buffer(s.data()), length(s.size()) {}

  /// Returns a pointer to the start of the memory block.
  const char* data() const { return buffer; }

  /// Returns an iterator to the beginning of the buffer.
  const char* begin() const { return buffer; }

  /// Returns an iterator to the end of the buffer.
  const char* end() const { return buffer + length; }

  /// Returns the size of the memory block.
  size_t size() const { return length; }
};

} // namespace detail

} // namespace isrlog

#endif // ISRLOG_DETAIL_SUPPORT_MEMORY_BUFFER_H
