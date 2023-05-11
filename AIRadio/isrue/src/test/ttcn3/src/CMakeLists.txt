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

set(SOURCES ttcn3_ue.cc ttcn3_syssim.cc)
add_executable(ttcn3_dut ttcn3_dut.cc lte_ttcn3_phy.cc ${SOURCES})

set(LINK_LIBRARIES isrue_stack
                    isrue_upper
                    isrue_rrc
                    isrue_rrc_nr
                    isrran_gtpu
                    isrran_pdcp
                    isrran_rlc
                    isrran_common
                    isrran_mac
                    isrran_phy
                    isrran_radio
                    isrran_gtpu
                    isrran_pdcp
                    isrue_phy
                    isrue_mac
                    isrue_mac_nr
                    rrc_asn1
                    rrc_nr_asn1
                    sctp
                    ${Boost_LIBRARIES})

target_link_libraries(ttcn3_dut ${LINK_LIBRARIES})

include_directories(${PROJECT_SOURCE_DIR}/isrue/src/test/ttcn3/hdr)
