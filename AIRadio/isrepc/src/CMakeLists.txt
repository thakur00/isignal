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

add_subdirectory(mme)
add_subdirectory(hss)
add_subdirectory(spgw)
add_subdirectory(mbms-gw)

# Link libstdc++ and libgcc
if(BUILD_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif(BUILD_STATIC)


if (RPATH)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif (RPATH)


add_executable(isrepc main.cc)
target_link_libraries( isrepc  isrepc_mme
                               isrepc_hss
                               isrepc_sgw
                               s1ap_asn1
                               isrran_gtpu
                               isrran_asn1
                               isrran_common
                               isrlog
                               support
                               ${CMAKE_THREAD_LIBS_INIT}
                               ${Boost_LIBRARIES}
                               ${SEC_LIBRARIES}
                               ${LIBCONFIGPP_LIBRARIES}
                               ${SCTP_LIBRARIES})

add_executable(isrmbms mbms-gw/main.cc )
target_link_libraries(isrmbms   isrepc_mbms_gw
                                isrran_asn1
                                isrran_gtpu
                                isrran_common
                                isrlog
                                ${CMAKE_THREAD_LIBS_INIT}
                                ${Boost_LIBRARIES}
                                ${SEC_LIBRARIES}
                                ${LIBCONFIGPP_LIBRARIES}
                                ${SCTP_LIBRARIES})
if (RPATH)
  set_target_properties(isrepc PROPERTIES INSTALL_RPATH ".")
  set_target_properties(isrmbms PROPERTIES INSTALL_RPATH ".")
endif (RPATH)

########################################################################
# Option to run command after build (useful for remote builds)
########################################################################
if (NOT ${BUILDEPC_CMD} STREQUAL "")
  message(STATUS "Added custom post-build-EPC command: ${BUILDENB_CMD}")
  add_custom_command(TARGET isrenb POST_BUILD COMMAND ${BUILDENB_CMD})
else(NOT ${BUILDEPC_CMD} STREQUAL "")
  message(STATUS "No post-build-EPC command defined")
endif (NOT ${BUILDEPC_CMD} STREQUAL "")

install(TARGETS isrepc DESTINATION ${RUNTIME_DIR} OPTIONAL)
install(TARGETS isrmbms DESTINATION ${RUNTIME_DIR} OPTIONAL)
