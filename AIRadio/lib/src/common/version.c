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

#include "isrran/version.h"

char* isrran_get_version()
{
  return ISRRAN_VERSION_STRING;
}

int isrran_get_version_major()
{
  return ISRRAN_VERSION_MAJOR;
}
int isrran_get_version_minor()
{
  return ISRRAN_VERSION_MINOR;
}
int isrran_get_version_patch()
{
  return ISRRAN_VERSION_PATCH;
}

int isrran_check_version(int major, int minor, int patch)
{
  return (ISRRAN_VERSION >= ISRRAN_VERSION_ENCODE(major, minor, patch));
}
