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

add_subdirectory(common)
add_subdirectory(phy)
add_subdirectory(stack)


# Link libstdc++ and libgcc
if(BUILD_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif(BUILD_STATIC)


if (RPATH)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif (RPATH)

add_library(enb_cfg_parser STATIC parser.cc enb_cfg_parser.cc)
target_link_libraries(enb_cfg_parser isrran_common isrgnb_rrc_config_utils ${LIBCONFIGPP_LIBRARIES})

add_executable(isrenb main.cc enb.cc metrics_stdout.cc metrics_csv.cc metrics_json.cc)

set(ISRENB_SOURCES isrenb_phy isrenb_stack isrenb_common isrenb_s1ap isrenb_upper isrenb_mac isrenb_rrc isrlog system)
set(ISRRAN_SOURCES isrran_common isrran_mac isrran_phy isrran_gtpu isrran_rlc isrran_pdcp isrran_radio rrc_asn1 s1ap_asn1 enb_cfg_parser isrlog support system)

set(ISRENB_SOURCES ${ISRENB_SOURCES} isrgnb_stack isrgnb_ngap isrgnb_mac isrgnb_rrc)
set(ISRRAN_SOURCES ${ISRRAN_SOURCES} rrc_nr_asn1 ngap_nr_asn1)

target_link_libraries(isrenb  ${ISRENB_SOURCES}
                              ${ISRRAN_SOURCES}
                              ${CMAKE_THREAD_LIBS_INIT}
                              ${Boost_LIBRARIES}
                              ${SEC_LIBRARIES}
                              ${LIBCONFIGPP_LIBRARIES}
                              ${SCTP_LIBRARIES})

if (RPATH)
  set_target_properties(isrenb PROPERTIES INSTALL_RPATH ".")
endif (RPATH)

########################################################################
# Option to run command after build (useful for remote builds)
########################################################################
if (NOT ${BUILDENB_CMD} STREQUAL "")
  message(STATUS "Added custom post-build-ENB command: ${BUILDENB_CMD}")
  add_custom_command(TARGET isrenb POST_BUILD COMMAND ${BUILDENB_CMD})
else(NOT ${BUILDENB_CMD} STREQUAL "")
  message(STATUS "No post-build-ENB command defined")
endif (NOT ${BUILDENB_CMD} STREQUAL "")

install(TARGETS isrenb DESTINATION ${RUNTIME_DIR} OPTIONAL)
