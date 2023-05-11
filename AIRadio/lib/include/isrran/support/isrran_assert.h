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

#ifndef ISRRAN_ASSERT_H
#define ISRRAN_ASSERT_H

#ifdef __cplusplus
#include "isrran/isrlog/isrlog.h"
#include <cstdio>
#include <stdarg.h>

#define isrran_unlikely(expr) __builtin_expect(!!(expr), 0)

/**
 * Command to terminate isrRAN application with an error message, ensuring first that the log is flushed
 */
[[gnu::noinline, noreturn]] inline bool isrran_terminate(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  isrlog::flush();
  vfprintf(stderr, fmt, args);
  va_end(args);
  std::abort();
}

#define isrran_assertion_failure(fmt, ...)                                                                             \
  isrran_terminate("%s:%d: Assertion Failure: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/**
 * Macro that asserts condition is true. If false, it logs the remaining macro args, flushes the log,
 * prints the backtrace (if it was activated) and closes the application.
 */
#define isrran_always_assert(condition, fmt, ...) (void)((condition) || isrran_assertion_failure(fmt, ##__VA_ARGS__))

#define ISRRAN_IS_DEFINED(x) ISRRAN_IS_DEFINED2(x)
#define ISRRAN_IS_DEFINED2(x) (#x[0] == 0 || (#x[0] >= '1' && #x[0] <= '9'))

/**
 * Same as "isrran_always_assert" but it is only active when "enable_check" flag is defined
 */
#define isrran_assert_ifdef(enable_check, condition, fmt, ...)                                                         \
  (void)((not ISRRAN_IS_DEFINED(enable_check)) || (isrran_always_assert(condition, fmt, ##__VA_ARGS__), 0))

/**
 * Specialization of "isrran_assert_ifdef" for the ASSERTS_ENABLED flag
 */
#define isrran_assert(condition, fmt, ...) isrran_assert_ifdef(ASSERTS_ENABLED, condition, fmt, ##__VA_ARGS__)

/**
 * Specialization of "isrran_assert_ifdef" for the SANITY_CHECKS_ENABLED flag
 */
#ifndef NDEBUG
#define SANITY_CHECKS_ENABLED
#endif
#define isrran_sanity_check(condition, fmt, ...)                                                                       \
  isrran_assert_ifdef(SANITY_CHECKS_ENABLED, condition, fmt, ##__VA_ARGS__)

#ifdef STOP_ON_WARNING

#define isrran_expect(condition, fmt, ...) isrran_assert(condition, fmt, ##__VA_ARGS__)

#else

#define isrran_expect(condition, fmt, ...)                                                                             \
  do {                                                                                                                 \
    if (isrran_unlikely(not(condition))) {                                                                             \
      isrlog::fetch_basic_logger("ALL").warning("%s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__);                     \
    }                                                                                                                  \
  } while (0)

#endif

#else // __ifcplusplus

#include <cassert>

#ifdef ASSERTS_ENABLED
#define isrran_assert(condition, fmt, ...) (void)((condition) || (__assert(#condition, __FILE__, __FLAG__), 0))
#else
#define isrran_assert(condition, fmt, ...)                                                                             \
  do {                                                                                                                 \
  } while (0)
#endif

#endif // __ifcplusplus

#endif // ISRRAN_ASSERT_H
