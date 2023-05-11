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

#include "isrran/common/slot_point.h"
#include "isrran/common/tti_point.h"
#include "isrran/support/isrran_test.h"

using isrran::tti_point;

void test_tti_type()
{
  // TEST: constructors
  tti_point tti1;
  TESTASSERT(not tti1.is_valid());
  tti1 = tti_point{5};
  TESTASSERT(tti1.is_valid());

  // TEST: operators
  TESTASSERT(tti1 != tti_point{2});
  TESTASSERT(tti1 == tti_point{5});
  TESTASSERT(tti1 <= tti_point{5});
  TESTASSERT(tti1 >= tti_point{5});
  TESTASSERT(tti1 < tti_point{6});
  TESTASSERT(tti1 > tti_point{4});
  TESTASSERT(tti1 - tti_point{2} == 3);
  TESTASSERT(tti1++ == tti_point{5});
  TESTASSERT(tti1 == tti_point{6});
  TESTASSERT(++tti1 == tti_point{7});

  // TEST: wrap arounds
  TESTASSERT(tti_point{10239} - tti_point{1} == -2);
  TESTASSERT(tti_point{10239} < tti_point{1});
  TESTASSERT(tti_point{10238} - tti_point{10239} == -1);
  TESTASSERT(tti_point{10238} < tti_point{10239});
  TESTASSERT(tti_point{10239} - tti_point{10238} == 1);
  TESTASSERT(tti_point{10239} > tti_point{10238});
  TESTASSERT(tti_point{1} - tti_point{10239} == 2);
  TESTASSERT(tti_point{1} > tti_point{10239});

  tti1 = tti_point{10239};
  TESTASSERT(tti1++ == tti_point{10239});
  TESTASSERT(tti1 == tti_point{0});

  // TEST: tti skips
  tti1 = tti_point{10239};
  tti1 += 10;
  TESTASSERT(tti1 == tti_point{9});
  tti1 -= 11;
  TESTASSERT(tti1 == tti_point{10238});
  tti1 -= 10237;
  TESTASSERT(tti1 == (tti_point)1);

  // TEST: wrap initializations
  TESTASSERT(tti_point{1u - 2u} == tti_point{10239});
  TESTASSERT(tti_point{1u - 100u} == tti_point{10141});
  TESTASSERT(tti_point{10239u + 3u} == tti_point{2});
}

void test_nr_slot_type()
{
  // TEST: constructors
  isrran::slot_point slot1;
  TESTASSERT(not slot1.valid());
  isrran::slot_point slot2{0, 1, 5};
  TESTASSERT(slot2.valid() and slot2.numerology_idx() == 0 and slot2.slot_idx() == 5 and slot2.slot_idx() == 5 and
             slot2.sfn() == 1);
  isrran::slot_point slot3{slot2};
  TESTASSERT(slot3 == slot2);

  // TEST: comparison and difference operators
  slot1 = isrran::slot_point{0, 1, 5};
  slot2 = isrran::slot_point{0, 1, 5};
  TESTASSERT(slot1 == slot2 and slot1 <= slot2 and slot1 >= slot2);
  slot1++;
  TESTASSERT(slot1 != slot2 and slot1 >= slot2 and slot1 > slot2 and slot2 < slot1 and slot2 <= slot1);
  TESTASSERT(slot1 - slot2 == 1 and slot2 - slot1 == -1);
  slot1 = isrran::slot_point{0, 2, 5};
  TESTASSERT(slot1 != slot2 and slot1 >= slot2 and slot1 > slot2 and slot2 < slot1 and slot2 <= slot1);
  TESTASSERT(slot1 - slot2 == 10 and slot2 - slot1 == -10);
  slot1 = isrran::slot_point{0, 1023, 5};
  TESTASSERT(slot1 != slot2 and slot1 <= slot2 and slot1 < slot2 and slot2 > slot1 and slot2 >= slot1);
  TESTASSERT(slot1 - slot2 == -20 and slot2 - slot1 == 20);

  // TEST: increment/decrement operators
  slot1 = isrran::slot_point{0, 1, 5};
  slot2 = isrran::slot_point{0, 1, 5};
  TESTASSERT(slot1++ == slot2);
  TESTASSERT(slot2 + 1 == slot1);
  TESTASSERT(++slot2 == slot1);
  slot1 = isrran::slot_point{0, 1, 5};
  slot2 = isrran::slot_point{0, 1, 5};
  TESTASSERT(slot1 - 100 == slot2 - 100);
  TESTASSERT(((slot1 - 100000) + 100000) == slot1);
  TESTASSERT((slot1 - 10240) == slot1);
  TESTASSERT((slot1 - 100).slot_idx() == 5 and (slot1 - 100).sfn() == 1015);
  TESTASSERT(((slot1 - 100) + 100) == slot1);
  TESTASSERT(((slot1 - 1) + 1) == slot1);

  fmt::print("[  {}]", slot1);
}

int main()
{
  isrlog::init();
  test_tti_type();
  test_nr_slot_type();
  return 0;
}
