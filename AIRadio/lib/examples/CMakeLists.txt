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

#################################################################
# Applications
#################################################################

add_executable(synch_file synch_file.c)
target_link_libraries(synch_file isrran_phy)

#################################################################
# These can be compiled without UHD or graphics support
#################################################################

if(RF_FOUND)
  add_executable(pdsch_ue pdsch_ue.c)
  target_link_libraries(pdsch_ue isrran_phy isrran_common isrran_rf pthread)

  add_executable(pdsch_enodeb pdsch_enodeb.c)
  target_link_libraries(pdsch_enodeb isrran_phy isrran_common isrran_rf pthread)

  add_executable(npdsch_enodeb npdsch_enodeb.c)
  target_link_libraries(npdsch_enodeb isrran_phy isrran_rf pthread)

  add_executable(npdsch_ue npdsch_ue.c npdsch_ue_helper.cc)
  target_link_libraries(npdsch_ue isrran_common isrran_phy isrran_rf pthread rrc_asn1)
else(RF_FOUND)
  add_definitions(-DDISABLE_RF)

  add_executable(pdsch_ue pdsch_ue.c)
  target_link_libraries(pdsch_ue isrran_common isrran_phy pthread)

  add_executable(pdsch_enodeb pdsch_enodeb.c)
  target_link_libraries(pdsch_enodeb isrran_common isrran_phy pthread)

  add_executable(npdsch_enodeb npdsch_enodeb.c)
  target_link_libraries(npdsch_enodeb isrran_phy pthread)

  add_executable(npdsch_ue npdsch_ue.c npdsch_ue_helper.cc)
  target_link_libraries(npdsch_ue isrran_common isrran_phy pthread rrc_asn1)
endif(RF_FOUND)

if(ISRGUI_FOUND)
  include_directories(${ISRGUI_INCLUDE_DIRS})
  target_link_libraries(pdsch_ue ${ISRGUI_LIBRARIES})
  target_link_libraries(npdsch_ue ${ISRGUI_LIBRARIES})
  add_definitions(-DENABLE_GUI)
endif(ISRGUI_FOUND)

if (ZEROMQ_FOUND)
  add_executable(zmq_remote_rx zmq_remote_rx.c)
  target_link_libraries(zmq_remote_rx isrran_phy isrran_rf ${ZEROMQ_LIBRARIES})
endif (ZEROMQ_FOUND)

#################################################################
# These examples need the UHD driver
#################################################################

if(RF_FOUND)

  add_executable(cell_search cell_search.c)
  target_link_libraries(cell_search isrran_phy isrran_common isrran_rf)

  add_executable(usrp_capture usrp_capture.c)
  target_link_libraries(usrp_capture isrran_phy isrran_rf)

  add_executable(usrp_capture_sync usrp_capture_sync.c)
  target_link_libraries(usrp_capture_sync isrran_phy isrran_rf)

  add_executable(usrp_capture_sync_nbiot usrp_capture_sync_nbiot.c)
  target_link_libraries(usrp_capture_sync_nbiot isrran_rf isrran_phy)

  add_executable(cell_search_nbiot cell_search_nbiot.c)
  target_link_libraries(cell_search_nbiot isrran_rf isrran_phy)

  add_executable(usrp_txrx usrp_txrx.c)
  target_link_libraries(usrp_txrx isrran_phy isrran_rf)

  add_executable(pssch_ue pssch_ue.c)
  target_link_libraries(pssch_ue isrran_phy isrran_common isrran_rf pthread)

  if(ISRGUI_FOUND)
    target_link_libraries(pssch_ue ${ISRGUI_LIBRARIES})
  endif(ISRGUI_FOUND)

  message(STATUS "   examples will be installed.")

else(RF_FOUND)
  message(STATUS "   examples will NOT BE INSTALLED.")
endif(RF_FOUND)

# Add eNB/UE IQ test
add_subdirectory(test)
