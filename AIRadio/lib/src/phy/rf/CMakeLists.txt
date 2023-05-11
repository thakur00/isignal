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

if(RF_FOUND)
  # This library is only used by the examples 
  add_library(isrran_rf_utils STATIC rf_utils.c)
  target_link_libraries(isrran_rf_utils isrran_phy)

  # Top-level RF library sources
  set(SOURCES_RF "")
  list(APPEND SOURCES_RF rf_imp.c)

  # Lists of static (builtin) and dynamic RF plugins
  set(STATIC_PLUGINS "")
  set(DYNAMIC_PLUGINS "")

  if (ENABLE_RF_PLUGINS)
    add_definitions(-DENABLE_RF_PLUGINS)
  endif (ENABLE_RF_PLUGINS)

  # RF plugins
  if (UHD_FOUND AND ENABLE_UHD)
    add_definitions(-DENABLE_UHD)
    set(SOURCES_UHD rf_uhd_imp.cc)
    if (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_uhd SHARED ${SOURCES_UHD})
      set_target_properties(isrran_rf_uhd PROPERTIES VERSION ${ISRRAN_VERSION_STRING} SOVERSION ${ISRRAN_SOVERSION})
      list(APPEND DYNAMIC_PLUGINS isrran_rf_uhd)
    else (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_uhd STATIC ${SOURCES_UHD})
      list(APPEND STATIC_PLUGINS isrran_rf_uhd)
    endif (ENABLE_RF_PLUGINS)
    target_link_libraries(isrran_rf_uhd isrran_rf_utils isrran_phy ${UHD_LIBRARIES} ${Boost_LIBRARIES})
    install(TARGETS isrran_rf_uhd DESTINATION ${LIBRARY_DIR} OPTIONAL)

    # If found, add a macro to inform the UHD driver about the available feature
    if (UHD_ENABLE_X300_FW_RESET)
      add_definitions(-DENABLE_UHD_X300_FW_RESET)
    endif(UHD_ENABLE_X300_FW_RESET)
    if (UHD_ENABLE_RFNOC)
      add_definitions(-DUHD_ENABLE_RFNOC)
    endif(UHD_ENABLE_RFNOC)
    if (UHD_ENABLE_CUSTOM_RFNOC)
      add_definitions(-DUHD_ENABLE_CUSTOM_RFNOC)
    endif(UHD_ENABLE_CUSTOM_RFNOC)
  endif (UHD_FOUND AND ENABLE_UHD)

  if (BLADERF_FOUND AND ENABLE_BLADERF)
    add_definitions(-DENABLE_BLADERF)
    set(SOURCES_BLADE rf_blade_imp.c)
    if (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_blade SHARED ${SOURCES_BLADE})
      set_target_properties(isrran_rf_blade PROPERTIES VERSION ${ISRRAN_VERSION_STRING} SOVERSION ${ISRRAN_SOVERSION})
      list(APPEND DYNAMIC_PLUGINS isrran_rf_blade)
    else (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_blade STATIC ${SOURCES_BLADE})
      list(APPEND STATIC_PLUGINS isrran_rf_blade)
    endif (ENABLE_RF_PLUGINS)
    target_link_libraries(isrran_rf_blade isrran_rf_utils isrran_phy ${BLADERF_LIBRARIES})
    install(TARGETS isrran_rf_blade DESTINATION ${LIBRARY_DIR} OPTIONAL)
  endif (BLADERF_FOUND AND ENABLE_BLADERF)

  if (SOAPYSDR_FOUND AND ENABLE_SOAPYSDR)
    add_definitions(-DENABLE_SOAPYSDR)
    set(SOURCES_SOAPY rf_soapy_imp.c)
    if (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_soapy SHARED ${SOURCES_SOAPY})
      set_target_properties(isrran_rf_soapy PROPERTIES VERSION ${ISRRAN_VERSION_STRING} SOVERSION ${ISRRAN_SOVERSION})
      list(APPEND DYNAMIC_PLUGINS isrran_rf_soapy)
    else (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_soapy STATIC ${SOURCES_SOAPY})
      list(APPEND STATIC_PLUGINS isrran_rf_soapy)
    endif (ENABLE_RF_PLUGINS)
    target_link_libraries(isrran_rf_soapy isrran_rf_utils isrran_phy ${SOAPYSDR_LIBRARIES})
    install(TARGETS isrran_rf_soapy DESTINATION ${LIBRARY_DIR} OPTIONAL)
  endif (SOAPYSDR_FOUND AND ENABLE_SOAPYSDR)

  if(SKIQ_FOUND AND ENABLE_SKIQ)
    add_executable(skiq_pps_test skiq_pps_test.c)
    target_link_libraries(skiq_pps_test ${SKIQ_LIBRARIES} rt pthread m)
    add_definitions(-DENABLE_SIDEKIQ)
    set(SOURCES_SKIQ rf_skiq_imp.c rf_skiq_imp_card.c rf_skiq_imp_port.c)
    if (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_skiq SHARED ${SOURCES_SKIQ})
      set_target_properties(isrran_rf_skiq PROPERTIES VERSION ${ISRRAN_VERSION_STRING} SOVERSION ${ISRRAN_SOVERSION})
      list(APPEND DYNAMIC_PLUGINS isrran_rf_skiq)
    else (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_skiq STATIC ${SOURCES_SKIQ})
      list(APPEND STATIC_PLUGINS isrran_rf_skiq)
    endif (ENABLE_RF_PLUGINS)
    target_link_libraries(isrran_rf_skiq isrran_rf_utils isrran_phy ${SKIQ_LIBRARIES} rt)
    install(TARGETS isrran_rf_skiq DESTINATION ${LIBRARY_DIR} OPTIONAL)
  endif(SKIQ_FOUND AND ENABLE_SKIQ)

  if (ZEROMQ_FOUND AND ENABLE_ZEROMQ)
    add_definitions(-DENABLE_ZEROMQ)
    set(SOURCES_ZMQ rf_zmq_imp.c rf_zmq_imp_tx.c rf_zmq_imp_rx.c)
    if (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_zmq SHARED ${SOURCES_ZMQ})
      set_target_properties(isrran_rf_zmq PROPERTIES VERSION ${ISRRAN_VERSION_STRING} SOVERSION ${ISRRAN_SOVERSION})
      list(APPEND DYNAMIC_PLUGINS isrran_rf_zmq)
    else (ENABLE_RF_PLUGINS)
      add_library(isrran_rf_zmq STATIC ${SOURCES_ZMQ})
      list(APPEND STATIC_PLUGINS isrran_rf_zmq)
    endif (ENABLE_RF_PLUGINS)
    target_link_libraries(isrran_rf_zmq isrran_rf_utils isrran_phy ${ZEROMQ_LIBRARIES})
    install(TARGETS isrran_rf_zmq DESTINATION ${LIBRARY_DIR} OPTIONAL)
  endif (ZEROMQ_FOUND AND ENABLE_ZEROMQ)

  # Add sources of file-based RF directly to the RF library (not as a plugin)
  list(APPEND SOURCES_RF rf_file_imp.c rf_file_imp_tx.c rf_file_imp_rx.c)

  # Top-level RF library
  add_library(isrran_rf_object OBJECT ${SOURCES_RF})
  set_property(TARGET isrran_rf_object PROPERTY POSITION_INDEPENDENT_CODE 1)
  set(TOP_RF_LIBS)
  if (ENABLE_RF_PLUGINS)
    # Build as shared library with optional RF plugins (optional dependencies)
    if (DYNAMIC_PLUGINS)
      add_dependencies(isrran_rf_object ${DYNAMIC_PLUGINS})
    endif (DYNAMIC_PLUGINS)
    add_library(isrran_rf SHARED $<TARGET_OBJECTS:isrran_rf_object>)
    target_link_libraries(isrran_rf dl)
    list(APPEND TOP_RF_LIBS isrran_rf)

    # Add $ORIGIN (i.e. current location of this library) to rpath of isrran_rf.
    # This ensures that it will find the plugins that reside in the same directory as the library
    set_target_properties(isrran_rf PROPERTIES BUILD_RPATH "\$ORIGIN/")
    set_target_properties(isrran_rf PROPERTIES INSTALL_RPATH "\$ORIGIN/")
  else (ENABLE_RF_PLUGINS)
    # Build as static library with built-in RF plugins (mandatory dependencies)
    add_library(isrran_rf STATIC $<TARGET_OBJECTS:isrran_rf_object>)
    target_link_libraries(isrran_rf ${STATIC_PLUGINS})
    list(APPEND TOP_RF_LIBS isrran_rf)

    # Also build as shared library with built-in RF plugins (mandatory dependencies)
    add_library(isrran_rf_shared SHARED $<TARGET_OBJECTS:isrran_rf_object>)
    target_link_libraries(isrran_rf_shared ${STATIC_PLUGINS})
    list(APPEND TOP_RF_LIBS isrran_rf_shared)
    set_target_properties(isrran_rf_shared PROPERTIES OUTPUT_NAME isrran_rf)
  endif (ENABLE_RF_PLUGINS)

  foreach (TOP_RF_LIB ${TOP_RF_LIBS})
    target_link_libraries(${TOP_RF_LIB} isrran_rf_utils isrran_phy)
    set_target_properties(${TOP_RF_LIB} PROPERTIES VERSION ${ISRRAN_VERSION_STRING} SOVERSION ${ISRRAN_SOVERSION})
    install(TARGETS ${TOP_RF_LIB} DESTINATION ${LIBRARY_DIR} OPTIONAL)
  endforeach ()

  # Tests
  if (UHD_FOUND AND UHD_ENABLE_CUSTOM_RFNOC)
    add_executable(rfnoc_test rfnoc_test.cc)
    target_link_libraries(rfnoc_test isrran_rf ${UHD_LIBRARIES} ${Boost_LIBRARIES} /usr/lib/x86_64-linux-gnu/libboost_system.so)
    message(info ${Boost_LIBRARIES})
  endif (UHD_FOUND AND UHD_ENABLE_CUSTOM_RFNOC)

  if (ZEROMQ_FOUND)
    add_executable(rf_zmq_test rf_zmq_test.c)
    target_link_libraries(rf_zmq_test isrran_rf)
    #add_test(rf_zmq_test rf_zmq_test)
  endif (ZEROMQ_FOUND)

  add_executable(rf_file_test rf_file_test.c)
  target_link_libraries(rf_file_test isrran_rf)
  add_test(rf_file_test rf_file_test)
endif(RF_FOUND)
