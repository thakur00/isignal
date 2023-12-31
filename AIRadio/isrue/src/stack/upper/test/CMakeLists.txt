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

add_executable(usim_test usim_test.cc)
target_link_libraries(usim_test isrue_upper isrran_phy rrc_asn1 isrlog)
add_test(usim_test usim_test)

if(HAVE_PCSC)
  add_executable(pcsc_usim_test pcsc_usim_test.cc)
  target_link_libraries(pcsc_usim_test isrue_upper isrran_phy isrran_common isrlog)
endif(HAVE_PCSC)

add_executable(nas_test nas_test.cc)
target_link_libraries(nas_test isrue_upper isrran_common isrran_phy rrc_asn1 isrran_asn1)
add_test(nas_test nas_test)

add_executable(nas_5g_test nas_5g_test.cc)
target_link_libraries(nas_5g_test isrue_upper isrran_phy rrc_asn1)
add_test(nas_5g_test nas_5g_test)

add_executable(gw_test gw_test.cc)
target_link_libraries(gw_test isrue_upper isrran_common isrran_phy)
add_test(gw_test gw_test)

add_executable(tft_test tft_test.cc)
target_link_libraries(tft_test isrue_upper isrran_common isrran_phy)
add_test(tft_test tft_test)

########################################################################
# Option to run command after build (useful for remote builds)
########################################################################
if (NOT ${BUILD_CMD} STREQUAL "")
  message(STATUS "Added custom post-build command: ${BUILD_CMD}")
  add_custom_command(TARGET ip_test POST_BUILD COMMAND ${BUILD_CMD})
else(NOT ${BUILD_CMD} STREQUAL "")
  message(STATUS "No post-build command defined")
endif (NOT ${BUILD_CMD} STREQUAL "")

