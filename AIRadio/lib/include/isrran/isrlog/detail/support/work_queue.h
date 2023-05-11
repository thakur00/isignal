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

#ifndef ISRLOG_DETAIL_SUPPORT_WORK_QUEUE_H
#define ISRLOG_DETAIL_SUPPORT_WORK_QUEUE_H

#include "isrran/adt/circular_buffer.h"
#include "isrran/isrlog/detail/support/backend_capacity.h"
#include "isrran/isrlog/detail/support/thread_utils.h"

namespace isrlog {

namespace detail {

//: TODO: this is a temp work queue.

/// Thread safe generic data type work queue.
template <typename T, size_t capacity = ISRLOG_QUEUE_CAPACITY>
class work_queue
{
  isrran::dyn_circular_buffer<T> queue;
  mutable mutex                  m;
  static constexpr size_t        threshold = capacity * 0.98;

public:
  work_queue() : queue(capacity) {}

  work_queue(const work_queue&) = delete;
  work_queue& operator=(const work_queue&) = delete;

  /// Inserts a new element into the back of the queue. Returns false when the
  /// queue is full, otherwise true.
  bool push(const T& value)
  {
    m.lock();
    // Discard the new element if we reach the maximum capacity.
    if (queue.full()) {
      m.unlock();
      return false;
    }
    queue.push(value);
    m.unlock();

    return true;
  }

  /// Inserts a new element into the back of the queue. Returns false when the
  /// queue is full, otherwise true.
  bool push(T&& value)
  {
    m.lock();
    // Discard the new element if we reach the maximum capacity.
    if (queue.full()) {
      m.unlock();
      return false;
    }
    queue.push(std::move(value));
    m.unlock();

    return true;
  }

  /// Extracts the top most element from the queue if it exists.
  /// Returns a pair with a bool indicating if the pop has been successful.
  std::pair<bool, T> try_pop()
  {
    m.lock();

    if (queue.empty()) {
      m.unlock();
      return {false, T()};
    }

    // Here we have been woken up normally.
    T Item = std::move(queue.top());
    queue.pop();

    m.unlock();

    return {true, std::move(Item)};
  }

  /// Capacity of the queue.
  size_t get_capacity() const { return capacity; }

  /// Returns true when the queue is almost full, otherwise returns false.
  bool is_almost_full() const
  {
    scoped_lock lock(m);

    return queue.size() > threshold;
  }
};

} // namespace detail

} // namespace isrlog

#endif // ISRLOG_DETAIL_SUPPORT_WORK_QUEUE_H
