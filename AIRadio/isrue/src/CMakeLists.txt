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

add_subdirectory(phy)
add_subdirectory(stack)
add_subdirectory(test)

# Link libstdc++ and libgcc
if(BUILD_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif(BUILD_STATIC)

if (RPATH)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif (RPATH)

add_executable(isrue main.cc ue.cc metrics_stdout.cc metrics_csv.cc metrics_json.cc)

set(ISRUE_SOURCES isrue_phy isrue_stack isrue_upper isrue_mac isrue_rrc isrlog system)
set(ISRRAN_SOURCES isrran_common isrran_mac isrran_phy isrran_radio isrran_gtpu isrran_rlc isrran_pdcp rrc_asn1 isrlog support system)

set(ISRUE_SOURCES ${ISRUE_SOURCES} isrue_nr_stack isrue_rrc_nr isrue_mac_nr)
set(ISRRAN_SOURCES ${ISRRAN_SOURCES} rrc_nr_asn1 ngap_nr_asn1)

target_link_libraries(isrue   ${ISRUE_SOURCES}
                              ${ISRRAN_SOURCES}
                              ${CMAKE_THREAD_LIBS_INIT}
                              ${Boost_LIBRARIES}
                              ${ATOMIC_LIBS})

if (RPATH)
  set_target_properties(isrue PROPERTIES INSTALL_RPATH ".")
endif (RPATH)

########################################################################
# UE initialization test
########################################################################

if(NOT ISRGUI_FOUND)
# Make sure isrue returns 0 and it does not leak
add_test(isrue_version isrue --version)
add_test(isrue_help isrue --help)
endif(NOT ISRGUI_FOUND)

# Checks that ue.conf.example is valid and it does not leak memory if RF fails
if (ZEROMQ_FOUND)
  add_test(ue_rf_failure isrue ${CMAKE_SOURCE_DIR}/isrue/ue.conf.example --rf.device_name=zmq)
  add_test(ue_rf_failure_max_channels isrue ${CMAKE_SOURCE_DIR}/isrue/ue.conf.example --rf.device_name=zmq --rf.nof_antennas=4 --rat.eutra.nof_carriers=5)
  add_test(ue_rf_failure_exceeds_channels isrue ${CMAKE_SOURCE_DIR}/isrue/ue.conf.example --rf.device_name=zmq --rf.nof_antennas=5 --rat.eutra.nof_carriers=5)
endif(ZEROMQ_FOUND)

########################################################################
# Option to run command after build (useful for remote builds)
########################################################################
if (NOT ${BUILDUE_CMD} STREQUAL "")
  message(STATUS "Added custom post-build-UE command: ${BUILDUE_CMD}")
  add_custom_command(TARGET isrue POST_BUILD COMMAND ${BUILDUE_CMD})
else(NOT ${BUILDUE_CMD} STREQUAL "")
  message(STATUS "No post-build-UE command defined")
endif (NOT ${BUILDUE_CMD} STREQUAL "")

install(TARGETS isrue DESTINATION ${RUNTIME_DIR} OPTIONAL)
