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

add_subdirectory(agc)
add_subdirectory(ch_estimation)
add_subdirectory(common)
add_subdirectory(fec)
add_subdirectory(mimo)
add_subdirectory(phch)
add_subdirectory(rf)
add_subdirectory(sync)
add_subdirectory(utils)
add_subdirectory(channel)
add_subdirectory(dft)
add_subdirectory(io)
add_subdirectory(modem)
add_subdirectory(resampling)
add_subdirectory(scrambling)
add_subdirectory(ue)
add_subdirectory(enb)
add_subdirectory(gnb)
add_subdirectory(cfr)
set(isrran_srcs     $<TARGET_OBJECTS:isrran_agc>
        $<TARGET_OBJECTS:isrran_ch_estimation>
        $<TARGET_OBJECTS:isrran_phy_common>
        $<TARGET_OBJECTS:isrran_fec>
        $<TARGET_OBJECTS:isrran_mimo>
        $<TARGET_OBJECTS:isrran_phch>
        $<TARGET_OBJECTS:isrran_sync>
        $<TARGET_OBJECTS:isrran_utils>
        $<TARGET_OBJECTS:isrran_channel>
        $<TARGET_OBJECTS:isrran_dft>
        $<TARGET_OBJECTS:isrran_io>
        $<TARGET_OBJECTS:isrran_modem>
        $<TARGET_OBJECTS:isrran_resampling>
        $<TARGET_OBJECTS:isrran_scrambling>
        $<TARGET_OBJECTS:isrran_ue>
        $<TARGET_OBJECTS:isrran_enb>
        $<TARGET_OBJECTS:isrran_gnb>
        $<TARGET_OBJECTS:isrran_cfr>
        )

add_library(isrran_phy STATIC ${isrran_srcs} )
target_link_libraries(isrran_phy pthread m ${FFT_LIBRARIES})
install(TARGETS isrran_phy DESTINATION ${LIBRARY_DIR} OPTIONAL)
