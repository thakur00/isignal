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

#include "isrran/adt/scope_exit.h"
#include "isrran/common/test_common.h"

int test_scope_exit(int* value)
{
  auto test_exit = isrran::make_scope_exit([value]() {
    *value = ISRRAN_SUCCESS;
    printf("Finished successfully\n");
  });

  // TEST: simple scope_exit call
  int nof_calls = 0;
  {
    auto scope_exit = isrran::make_scope_exit([&]() {
      nof_calls++;
      printf("TEST1: Exited first scope\n");
    });
    printf("TEST1: Entered first scope\n");
  }
  TESTASSERT(nof_calls == 1);

  // TEST: Cancelling scope_exit call via release()
  nof_calls = 0;
  {
    auto scope_exit = isrran::make_scope_exit([&]() {
      nof_calls++;
      printf("TEST2: This should not be called\n");
    });
    scope_exit.release();
    printf("TEST2: Entered second scope\n");
  }
  TESTASSERT(nof_calls == 0);

  // TEST: move is safe
  nof_calls = 0;
  {
    printf("TEST3: Entered third scope\n");
    auto scope_exit = isrran::make_scope_exit([&]() { nof_calls++; });
    auto scope_exit2{std::move(scope_exit)};
  }
  TESTASSERT(nof_calls == 1);

  // TEST: lvalue callback is safe
  {
    nof_calls       = 0;
    auto task       = [&nof_calls]() { nof_calls += 2; };
    auto scope_exit = isrran::make_scope_exit(task);
  }
  TESTASSERT(nof_calls == 2);

  return ISRRAN_SUCCESS;
}

int main()
{
  int ret = ISRRAN_ERROR;
  TESTASSERT(test_scope_exit(&ret) == ISRRAN_SUCCESS);
  TESTASSERT(ret == ISRRAN_SUCCESS);
}