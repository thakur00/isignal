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

find_package(LibConfig REQUIRED)
find_package(SCTP REQUIRED)

if(BUILD_STATIC)
  set(LIBCONFIGPP_LIBRARIES "${LIBCONFIGPP_STATIC_LIBRARY_PATH}")
endif(BUILD_STATIC)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost required to compile isrEPC")
endif()

########################################################################
# Setup the include and linker paths
########################################################################
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${SEC_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}
)

link_directories(
    ${Boost_LIBRARY_DIRS}
    ${SEC_LIBRARY_DIRS}
)

########################################################################
# Add subdirectories
########################################################################
add_subdirectory(src)

########################################################################
# Default configuration files
########################################################################
install(FILES epc.conf.example DESTINATION ${DATA_DIR})
install(FILES mbms.conf.example DESTINATION ${DATA_DIR})
install(FILES user_db.csv.example DESTINATION ${DATA_DIR})
install(PROGRAMS isrepc_if_masq.sh DESTINATION ${RUNTIME_DIR})