#
# Copyright 2013-2022 iSignal Research Labs Pvt Ltd.
#
# This file is part of isrRAN
#
# isrRAN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# isrRAN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

# - Try to find ISRGUI
# Once done this will define
#  ISRGUI_FOUND        - System has isrgui
#  ISRGUI_INCLUDE_DIRS - The isrgui include directories
#  ISRGUI_LIBRARIES    - The isrgui library

find_package(PkgConfig)
pkg_check_modules(PC_ISRGUI QUIET isrgui)
IF(NOT ISRGUI_FOUND)

FIND_PATH(
    ISRGUI_INCLUDE_DIRS
    NAMES isrgui/isrgui.h
    HINTS ${PC_ISRGUI_INCLUDEDIR}
          ${PC_ISRGUI_INCLUDE_DIRS}
          $ENV{ISRGUI_DIR}/include
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    ISRGUI_LIBRARIES
    NAMES isrgui
    HINTS ${PC_ISRGUI_LIBDIR}
          ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          $ENV{ISRGUI_DIR}/lib
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

message(STATUS "ISRGUI LIBRARIES " ${ISRGUI_LIBRARIES})
message(STATUS "ISRGUI INCLUDE DIRS " ${ISRGUI_INCLUDE_DIRS})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ISRGUI DEFAULT_MSG ISRGUI_LIBRARIES ISRGUI_INCLUDE_DIRS)
MARK_AS_ADVANCED(ISRGUI_LIBRARIES ISRGUI_INCLUDE_DIRS)

ENDIF(NOT ISRGUI_FOUND)

