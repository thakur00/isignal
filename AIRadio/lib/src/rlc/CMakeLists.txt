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

set(SOURCES rlc.cc
            rlc_tm.cc
            rlc_um_base.cc
            rlc_um_lte.cc
            rlc_um_nr.cc
            rlc_am_base.cc
            rlc_am_lte.cc
            rlc_am_nr.cc
            rlc_am_lte_packing.cc
            rlc_am_nr_packing.cc
            bearer_mem_pool.cc)

add_library(isrran_rlc STATIC ${SOURCES})
target_link_libraries(isrran_rlc isrran_common ${ATOMIC_LIBS})
install(TARGETS isrran_rlc DESTINATION ${LIBRARY_DIR} OPTIONAL)
