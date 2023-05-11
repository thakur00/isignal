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

#ifndef ISRRAN_ISRRAN_TEST_H
#define ISRRAN_ISRRAN_TEST_H

#ifdef __cplusplus

#include "isrran_assert.h"

#define TESTASSERT_EQ(EXPECTED, ACTUAL)                                                                                \
  (void)((EXPECTED == ACTUAL) ||                                                                                       \
         (isrran_assertion_failure(                                                                                    \
              "%s", fmt::format("Actual value '{}' differs from expected '{}'", ACTUAL, EXPECTED).c_str()),            \
          0))

#define TESTASSERT_NEQ(EXPECTED, ACTUAL)                                                                               \
  (void)((EXPECTED != ACTUAL) ||                                                                                       \
         (isrran_assertion_failure("%s", fmt::format("Value should not be equal to '{}'", ACTUAL).c_str()), 0))

#define TESTASSERT(cond) isrran_assert((cond), "Fail at \"%s\"", (#cond))

#define TESTASSERT_SUCCESS(cond) isrran_assert((cond == ISRRAN_SUCCESS), "Operation \"%s\" was not successful", (#cond))

#else // __cplusplus

#include <stdio.h>

#define TESTASSERT(cond)                                                                                               \
  do {                                                                                                                 \
    if (!(cond)) {                                                                                                     \
      fprintf(stderr, "%s:%d: Assertion Failure: \"%s\"\n", __FUNCTION__, __LINE__, (#cond));                          \
      return -1;                                                                                                       \
    }                                                                                                                  \
  } while (0)

#endif // __cplusplus

#endif // ISRRAN_ISRRAN_TEST_H
