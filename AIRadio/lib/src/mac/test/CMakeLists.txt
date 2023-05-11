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

add_executable(pdu_test pdu_test.cc)
target_link_libraries(pdu_test isrran_phy isrran_common isrran_mac ${CMAKE_THREAD_LIBS_INIT})
add_test(pdu_test pdu_test)

add_executable(mac_pcap_test mac_pcap_test.cc)
target_link_libraries(mac_pcap_test isrran_common isrran_mac ${SCTP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(mac_pcap_test mac_pcap_test)

add_executable(mac_pdu_nr_test mac_pdu_nr_test.cc)
target_link_libraries(mac_pdu_nr_test isrran_mac isrran_common ${CMAKE_THREAD_LIBS_INIT})
add_test(mac_pdu_nr_test mac_pdu_nr_test)

