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

########################################################################
# PROGRAM TO DEBUG PSS FROM USRP  
########################################################################

add_executable(gen_ack_test gen_ack_test.c)
target_link_libraries(gen_ack_test isrran_phy)
add_test(gen_ack_test gen_ack_test)

add_executable(gen_ack_nr_test gen_ack_nr_test.c)
target_link_libraries(gen_ack_nr_test isrran_phy)
add_test(gen_ack_nr_test gen_ack_nr_test)

add_executable(pucch_resource_test pucch_resource_test.c)
target_link_libraries(pucch_resource_test isrran_phy)
add_test(pucch_resource_test pucch_resource_test)

add_executable(ue_dl_nbiot_test ue_dl_nbiot_test.c)
target_link_libraries(ue_dl_nbiot_test isrran_phy pthread)
add_test(ue_dl_nbiot_test ue_dl_nbiot_test)

add_executable(ue_sync_nr_test ue_sync_nr_test.c)
target_link_libraries(ue_sync_nr_test isrran_phy pthread)
add_test(ue_sync_nr_test ue_sync_nr_test)

if(RF_FOUND)
    add_executable(ue_mib_sync_test_nbiot_usrp ue_mib_sync_test_nbiot_usrp.c)
    target_link_libraries(ue_mib_sync_test_nbiot_usrp isrran_phy isrran_rf pthread)

    add_executable(ue_sync_test_nbiot_usrp ue_sync_test_nbiot_usrp.c)
    target_link_libraries(ue_sync_test_nbiot_usrp isrran_rf isrran_phy pthread)
    if(ISRGUI_FOUND)
        include_directories(${ISRGUI_INCLUDE_DIRS})
        target_link_libraries(ue_sync_test_nbiot_usrp ${ISRGUI_LIBRARIES})
    else(ISRGUI_FOUND)
        add_definitions(-DDISABLE_GRAPHICS)
    endif(ISRGUI_FOUND)
endif(RF_FOUND)

add_executable(ue_dl_nr_file_test ue_dl_nr_file_test.cc)
target_link_libraries(ue_dl_nr_file_test isrran_phy isrran_common pthread)
foreach (n RANGE 0 9)
    #add_test(ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0.dat -i 1 -P 25 -n ${n} -R 1234 -l 2)
    #add_test(ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0.dat -i 1 -P 25 -n ${n} -R 1234 -l 2)
    #add_test(ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0.dat -i 1 -P 25 -n ${n} -R 1234 -l 2)
    #add_test(ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0.dat -i 1 -P 25 -n ${n} -R 1234 -l 2)
    #add_test(ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0.dat -i 1 -P 25 -n ${n} -R 1234 -l 2)
    #add_test(ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci1_rb25_n${n}_common_L1_ncce0.dat -i 1 -P 25 -n ${n} -R 1234 -l 2)
endforeach ()
#add_test(ue_dl_nr_pci500_rb52_n4_ra_L2_ncce0 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci500_rb52_n4_ra_L2_ncce0.dat -i 1 -P 52 -n 4 -R 7f)
add_test(ue_dl_nr_pci500_rb52_si_coreset0_idx6 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci500_rb52_si_coreset0_idx6_s15.36e6.dat -S -i 500 -P 52 -n 0 -R ffff -T si -c 6 -s common0 -A 368500 -a 368410)
#add_test(ue_dl_nr_pci500_rb52_si_coreset0_idx7 ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci500_rb52_si_coreset0_idx7_s15.36e6.dat -S -i 500 -P 52 -n 0 -R ffff -T si -c 7 -s common0 -A 161200 -a 161290)
add_test(ue_dl_nr_pci500_rb52_pdsch ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci500_rb52_rnti0x100_s15.36e6.dat -S -i 500 -P 52 -N 48 -n 1 -R 0x100 -T c -s common3 -o 1 -A 368500 -a 368410 -I -t 1 13)
add_test(ue_dl_nr_pci500_rb52_rar ue_dl_nr_file_test -f ${CMAKE_CURRENT_SOURCE_DIR}/ue_dl_nr_pci500_rb52_rar_s15.36e6.dat -i 500 -P 52 -n 5 -R f -T ra -c 6 -S -s common1 -A 368500 -a 368410)
