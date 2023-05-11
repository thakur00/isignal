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

set(SOURCES mac_nr.cc
            proc_ra_nr.cc
            proc_bsr_nr.cc
            proc_sr_nr.cc
            mux_nr.cc
            demux_nr.cc
            dl_harq_nr.cc
            ul_harq_nr.cc)
add_library(isrue_mac_nr STATIC ${SOURCES})
target_link_libraries(isrue_mac_nr isrue_mac_common isrran_mac)

add_subdirectory(test)
