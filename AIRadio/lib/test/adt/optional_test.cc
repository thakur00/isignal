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

#include "isrran/adt/optional.h"
#include "isrran/common/test_common.h"

using namespace isrran;

void test_optional_int()
{
  optional<int> opt, opt2(5);
  TESTASSERT(not opt.has_value() and opt2.has_value());
  TESTASSERT(not static_cast<bool>(opt) and static_cast<bool>(opt2));
  TESTASSERT(opt2.value() == 5 and *opt2 == 5);

  opt = 4;
  TESTASSERT(opt.has_value());
  TESTASSERT(opt != opt2);
  opt2 = 4;
  TESTASSERT(opt == opt2);
}

struct C {
  std::unique_ptr<int> val;

  C(int val = 0) : val(std::make_unique<int>(val)) {}
};

void test_optional_move_only()
{
  optional<C> a, b;
  a.emplace(C{});
  TESTASSERT(a.has_value());
  TESTASSERT_EQ(0, *a.value().val);
  TESTASSERT(not b.has_value());
  b.emplace(C{5});
  a = std::move(b);
  TESTASSERT_EQ(5, *a.value().val);
}

int main()
{
  test_optional_int();
  test_optional_move_only();
}