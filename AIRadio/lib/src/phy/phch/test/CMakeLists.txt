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

set(CTEST_LABELS "lib;phy;phch")

########################################################################
# PBCH TEST
########################################################################

add_executable(pbch_test pbch_test.c)
target_link_libraries(pbch_test isrran_phy)

add_lte_test(pbch_test_6 pbch_test -p 1 -n 6 -c 100)
add_lte_test(pbch_test_62 pbch_test -p 2 -n 6 -c 100)
add_lte_test(pbch_test_64 pbch_test -p 4 -n 6 -c 100)
add_lte_test(pbch_test_50 pbch_test -p 1 -n 50 -c 50)
add_lte_test(pbch_test_502 pbch_test -p 2 -n 50 -c 50)
add_lte_test(pbch_test_504 pbch_test -p 4 -n 50 -c 50)

########################################################################
# PSBCH TEST
########################################################################

add_executable(psbch_test psbch_test.c)
target_link_libraries(psbch_test isrran_phy)

# TM2 self tests
add_lte_test(psbch_test_self_test_tm2_p6_c168_self psbch_test -p 6 -c 168 -t 2)
add_lte_test(psbch_test_self_test_tm2_p50_c168_self psbch_test -p 50 -c 252 -t 2)
add_lte_test(psbch_test_self_test_tm2_p100_c168_self psbch_test -p 100 -c 335 -t 2)
add_lte_test(psbch_test_self_test_tm2_p25_c168_ext_self psbch_test -p 25 -c 168 -e)
add_lte_test(psbch_test_self_test_tm2_p100_c335_ext_self psbch_test -p 100 -c 335 -e)

# TM4 self tests
add_lte_test(psbch_test_self_test_tm4_p6_c168_self psbch_test -p 6 -c 168 -t 4)
add_lte_test(psbch_test_self_test_tm4_p50_c168_self psbch_test -p 50 -c 252 -t 4)
add_lte_test(psbch_test_self_test_tm4_p100_c168_self psbch_test -p 100 -c 335 -t 4)

add_executable(psbch_file_test psbch_file_test.c)
target_link_libraries(psbch_file_test isrran_phy)

# TM2 file tests
add_lte_test(psbch_file_test_ideal_tm2_p6_c0 psbch_file_test -p 6 -c 0 -d -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_ideal_tm2_p6_c0_s1.92e6.dat)
add_lte_test(psbch_file_test_ideal_tm2_p15_c84 psbch_file_test -p 15 -c 84 -d -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_ideal_tm2_p15_c84_s3.84e6.dat)
add_lte_test(psbch_file_test_ideal_tm2_p25_c168 psbch_file_test -p 25 -c 168 -d -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_ideal_tm2_p25_c168_s7.68e6.dat)
add_lte_test(psbch_file_test_ideal_tm2_p50_c252 psbch_file_test -p 50 -c 252 -d -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_ideal_tm2_p50_c252_s15.36e6.dat)
add_lte_test(psbch_file_test_ideal_tm2_p100_c335 psbch_file_test -p 100 -c 335 -d -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_ideal_tm2_p100_c335_s30.72e6.dat)
add_lte_test(psbch_file_test_ideal_tm2_p50_c252_ext psbch_file_test -p 50 -c 252 -e -d -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_ideal_tm2_p50_c252_s15.36e6_ext.dat)

# TM4 file tests
add_lte_test(psbch_file_test_cmw_tm4_p50_c169 psbch_file_test -p 50 -c 169 -t 4 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_cmw500_f5.92e9_s11.52e6_50prb_slss_id169.dat)

########################################################################
# PSCCH TEST
########################################################################

add_executable(pscch_test pscch_test.c)
target_link_libraries(pscch_test isrran_phy)

# TM2 self tests
add_lte_test(pscch_test_tm2_p6 pscch_test -p 6)
add_lte_test(pscch_test_tm2_p15 pscch_test -p 15)
add_lte_test(pscch_test_tm2_p25 pscch_test -p 25)
add_lte_test(pscch_test_tm2_p50 pscch_test -p 50)
add_lte_test(pscch_test_tm2_p75 pscch_test -p 75)
add_lte_test(pscch_test_tm2_p100 pscch_test -p 100)

# TM4 self tests
add_lte_test(pscch_test_tm4_p6 pscch_test -p 6 -t 4)
add_lte_test(pscch_test_tm4_p15 pscch_test -p 15 -t 4)
add_lte_test(pscch_test_tm4_p25 pscch_test -p 25 -t 4)
add_lte_test(pscch_test_tm4_p50 pscch_test -p 50 -t 4)
add_lte_test(pscch_test_tm4_p75 pscch_test -p 75 -t 4)
add_lte_test(pscch_test_tm4_p100 pscch_test -p 100 -t 4)

########################################################################
# PSSCH TEST
########################################################################

add_executable(pssch_test pssch_test.c)
target_link_libraries(pssch_test isrran_phy)

# TM2 self tests
add_lte_test(pssch_test_tm2_p6 pssch_test -p 6 -m 2)
add_lte_test(pssch_test_tm2_p15 pssch_test -p 15 -m 6)
add_lte_test(pssch_test_tm2_p25 pssch_test -p 25 -m 7)
add_lte_test(pssch_test_tm2_p50 pssch_test -p 50 -m 9)
add_lte_test(pssch_test_tm2_p50_ext pssch_test -p 50 -m 9 -e)
add_lte_test(pssch_test_tm2_p75 pssch_test -p 75 -m 17)
add_lte_test(pssch_test_tm2_p100 pssch_test -p 100 -m 21)
add_lte_test(pssch_test_tm2_p100_ext pssch_test -p 100 -m 21 -e)

# TM4 self tests
add_lte_test(pssch_test_tm4_p6 pssch_test -p 6 -t 4 -m 2)
add_lte_test(pssch_test_tm4_p15 pssch_test -p 15 -t 4 -m 6)
add_lte_test(pssch_test_tm4_p25 pssch_test -p 25 -t 4 -m 7)
add_lte_test(pssch_test_tm4_p50 pssch_test -p 50 -t 4 -m 9)
add_lte_test(pssch_test_tm4_p75 pssch_test -p 75 -t 4 -m 17)
add_lte_test(pssch_test_tm4_p100 pssch_test -p 100 -t 4 -m 21)

########################################################################
# PSCCH AND PSSCH FILE TEST
########################################################################

add_executable(pssch_pscch_file_test pssch_pscch_file_test.c)
target_link_libraries(pssch_pscch_file_test isrran_phy)

# TM2 file tests
add_lte_test(pssch_pscch_file_test_ideal_tm2_p100 pssch_pscch_file_test -p 100 -d -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_ideal_tm2_p100_c335_s30.72e6.dat)
set_property(TEST pssch_pscch_file_test_ideal_tm2_p100 PROPERTY PASS_REGULAR_EXPRESSION "num_decoded_sci=[2,3] num_decoded_tb=1")

# TM4 file tests (first SF is sf_idx = 6 such that the PSSCH sf_idx=0)
add_lte_test(pssch_pscch_file_test_ideal_tm4_p100 pssch_pscch_file_test -p 100 -t 4 -s 10 -n 10 -d -m 6 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_ideal_tm4_p100_c335_size10_num10_cshift0_s30.72e6.dat)
set_property(TEST pssch_pscch_file_test_ideal_tm4_p100 PROPERTY PASS_REGULAR_EXPRESSION "num_decoded_sci=1")

add_lte_test(pssch_pscch_test_tm4_p50_qc pssch_pscch_file_test -p 50 -t 4 -d -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_qc9150_f5.92e9_s15.36e6_50prb_20offset.dat)
set_property(TEST pssch_pscch_test_tm4_p50_qc PROPERTY PASS_REGULAR_EXPRESSION "num_decoded_sci=1 num_decoded_tb=1")

# Capture has a SFO offset of ~64 samples, but offsetting by 20 is enough to decode it
add_lte_test(pssch_pscch_test_tm4_p50_cmw pssch_pscch_file_test -p 50 -t 4 -o 20 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_cmw500_f5.92e9_s11.52e6_50prb_0offset_1ms.dat)
set_property(TEST pssch_pscch_test_tm4_p50_cmw PROPERTY PASS_REGULAR_EXPRESSION "num_decoded_sci=1 num_decoded_tb=1")

# With PHY retransmission (3 TTI offset) first SF at sf_idx=5
add_lte_test(pssch_pscch_test_tm4_p50_huawei pssch_pscch_file_test -p 50 -t 4 -m 5 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_huawei_s11.52e6_50prb_10prb_offset_with_retx.dat)
set_property(TEST pssch_pscch_test_tm4_p50_huawei PROPERTY PASS_REGULAR_EXPRESSION "num_decoded_sci=2 num_decoded_tb=2")

# With PHY ReTx (0 TTI offset?)
add_lte_test(pssch_pscch_test_tm4_p50_uxm1 pssch_pscch_file_test -p 50 -d -t 4 -s 5 -n 10 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_uxm_s15.36e6_50prb_0prb_offset_mcs12.dat)
set_property(TEST pssch_pscch_test_tm4_p50_uxm1 PROPERTY PASS_REGULAR_EXPRESSION "mcs=12.*num_decoded_sci=2 num_decoded_tb=2")

# 100 PRB startOffset 1 MCS12 MAC padding, first SF is index 0
add_lte_test(pssch_pscch_test_tm4_p100_uxm2 pssch_pscch_file_test -p 100 -t 4 -s 10 -n 10 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_uxm_s23.04e6_100prb_1prb_offset_mcs12_padding.dat)
set_property(TEST pssch_pscch_test_tm4_p100_uxm2 PROPERTY PASS_REGULAR_EXPRESSION "mcs=12.*num_decoded_sci=4")

# 100 PRB LTE sampling rate, startOffset1 MCS12 ITS data, first SF is index 6
add_lte_test(pssch_pscch_test_tm4_p100_uxm3 pssch_pscch_file_test -p 100 -d -t 4 -s 10 -n 10 -m 6 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_uxm_s30.72e6_100prb_1prb_offset_mcs12_its.dat)
set_property(TEST pssch_pscch_test_tm4_p100_uxm3 PROPERTY PASS_REGULAR_EXPRESSION "mcs=12.*num_decoded_sci=1")

# 50 PRB LTE sampling rate, startOffset0 MCS28 MAC padding, first SF is index 1
add_lte_test(pssch_pscch_test_tm4_p50_uxm4 pssch_pscch_file_test -p 50 -d -t 4 -s 5 -n 10 -m 1 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_sidelink_uxm_s15.36e6_50prb_0prb_offset_mcs28_padding_5ms.dat)
set_property(TEST pssch_pscch_test_tm4_p50_uxm4 PROPERTY PASS_REGULAR_EXPRESSION "mcs=28.*num_decoded_sci=5")

########################################################################
# NPBCH TEST
########################################################################

add_executable(npbch_test npbch_test.c)
target_link_libraries(npbch_test isrran_phy)
add_lte_test(npbch_test npbch_test)

add_lte_test(npbch_file_test_r13 npbch_file_test -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_nbiot_amari_nid0_sfn514_sib2.bin)
add_lte_test(npbch_file_test_r14 npbch_file_test -l 256 -R -r 0 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_nbiot_nid256_r14_sf0.bin)
add_lte_test(npbch_file_test_nid257_r13 npbch_file_test -l 257 -r 4 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_nbiot_nid257_r13_sf0.bin)
add_lte_test(npbch_file_test_nid257_r14 npbch_file_test -l 257 -R -r 7 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_nbiot_nid257_r14_sf0.bin)

########################################################################
# PCFICH TEST
########################################################################

add_executable(pcfich_test pcfich_test.c)
target_link_libraries(pcfich_test isrran_phy)

add_lte_test(pcfich_test_6 pcfich_test -p 1 -n 6)
add_lte_test(pcfich_test_62 pcfich_test -p 2 -n 6)
add_lte_test(pcfich_test_64 pcfich_test -p 4 -n 6)
add_lte_test(pcfich_test_10 pcfich_test -p 1 -n 10)
add_lte_test(pcfich_test_102 pcfich_test -p 2 -n 10)
add_lte_test(pcfich_test_104 pcfich_test -p 4 -n 10)

########################################################################
# PHICH TEST
########################################################################

add_executable(phich_test phich_test.c)
target_link_libraries(phich_test isrran_phy)

add_lte_test(phich_test_6 phich_test -p 1 -n 6)
add_lte_test(phich_test_62 phich_test -p 2 -n 6)
add_lte_test(phich_test_64 phich_test -p 4 -n 6 -g 1/6)

add_lte_test(phich_test_6e phich_test -p 1 -n 6 -e)
add_lte_test(phich_test_62e phich_test -p 2 -n 6 -e -l)
add_lte_test(phich_test_64e phich_test -p 4 -n 6 -e -l -g 2)

add_lte_test(phich_test_10 phich_test -p 1 -n 10 -e)
add_lte_test(phich_test_102 phich_test -p 2 -n 10 -g 2)
add_lte_test(phich_test_104 phich_test -p 4 -n 10 -e -l -g 1/2)

########################################################################
# PDCCH TEST
########################################################################

add_executable(pdcch_test pdcch_test.c)
target_link_libraries(pdcch_test isrran_phy)

foreach (nof_prb 6 15 25 50 75 100)
    # Currently, the ARM and SSE platforms isrRAN has been tested are not capable of running 100PRB. So, skip 100 PRB in
    # ARM and SSE.
    if ((HAVE_NEON OR NOT HAVE_AVX2) AND (${nof_prb} EQUAL 100))
        continue()
    endif ()
    foreach (nof_ports 1 2)
        foreach (cfi 1 2 3)
            foreach (snr auto 15 300)
                # Build PDCCH test arguments
                set(pdcch_test_args -n ${nof_prb} -p ${nof_ports} -f ${cfi} -S ${snr} -R 1)

                # The current Viterbi decoder implementation for SSE and NEON does not perform good enough to pass the
                # test without false detection. So, enable false alarm check in AVX2 based machines only.
                if (HAVE_AVX2)
                    set(pdcch_test_args ${pdcch_test_args} -F)
                endif ()

                string(REGEX REPLACE "\ " "" test_name_args ${pdcch_test_args})

                add_lte_test(pdcch_test${test_name_args} pdcch_test ${pdcch_test_args})
            endforeach ()
        endforeach ()
    endforeach ()
endforeach ()

########################################################################
# DCI TEST
########################################################################

add_executable(dci_test dci_test.c)
target_link_libraries(dci_test isrran_phy)
add_test(dci_test dci_test)

########################################################################
# PDSCH TEST
########################################################################

add_executable(pdsch_test pdsch_test.c)
target_link_libraries(pdsch_test isrran_phy)

add_lte_test(pdsch_test_qpsk pdsch_test -m 10 -n 50 -r 1)
add_lte_test(pdsch_test_qam16 pdsch_test -m 20 -n 100)
add_lte_test(pdsch_test_qam16 pdsch_test -m 20 -n 100 -r 2)
add_lte_test(pdsch_test_qam64 pdsch_test -n 100)

# PDSCH test for 1 transmision mode and 2 Rx antennas
add_lte_test(pdsch_test_sin_6   pdsch_test -x 1 -a 2 -n 6)
add_lte_test(pdsch_test_sin_12  pdsch_test -x 1 -a 2 -n 12)
add_lte_test(pdsch_test_sin_25  pdsch_test -x 1 -a 2 -n 25)
add_lte_test(pdsch_test_sin_50  pdsch_test -x 1 -a 2 -n 50)
add_lte_test(pdsch_test_sin_75  pdsch_test -x 1 -a 2 -n 75)
add_lte_test(pdsch_test_sin_100 pdsch_test -x 1 -a 2 -n 100)

# PDSCH test for transmit 2 transmision mode (1 codeword)
add_lte_test(pdsch_test_div_6   pdsch_test -x 2 -a 2 -n 6)
add_lte_test(pdsch_test_div_12  pdsch_test -x 2 -a 2 -n 12)
add_lte_test(pdsch_test_div_25  pdsch_test -x 2 -a 2 -n 25)
add_lte_test(pdsch_test_div_50  pdsch_test -x 2 -a 2 -n 50)
add_lte_test(pdsch_test_div_75  pdsch_test -x 2 -a 2 -n 75)
add_lte_test(pdsch_test_div_100 pdsch_test -x 2 -a 2 -n 100)

# PDSCH test for CDD transmision mode (2 codeword)
add_lte_test(pdsch_test_cdd_6   pdsch_test -x 3 -a 2 -t 0 -n 6)
add_lte_test(pdsch_test_cdd_12  pdsch_test -x 3 -a 2 -t 0 -n 12)
add_lte_test(pdsch_test_cdd_25  pdsch_test -x 3 -a 2 -t 0 -n 25)
add_lte_test(pdsch_test_cdd_50  pdsch_test -x 3 -a 2 -t 0 -n 50)
add_lte_test(pdsch_test_cdd_75  pdsch_test -x 3 -a 2 -t 0 -n 75)
add_lte_test(pdsch_test_cdd_100 pdsch_test -x 3 -a 2 -t 0 -n 100)

# PDSCH test for CDD transmision mode (2 codeword) and 256QAM
add_lte_test(pdsch_test_cdd_6   pdsch_test -x 3 -a 2 -t 0 -m 27 -M 27 -n 6 -q)
add_lte_test(pdsch_test_cdd_12  pdsch_test -x 3 -a 2 -t 0 -m 27 -M 27 -n 12 -q)
add_lte_test(pdsch_test_cdd_25  pdsch_test -x 3 -a 2 -t 0 -m 27 -M 27 -n 25 -q)
add_lte_test(pdsch_test_cdd_50  pdsch_test -x 3 -a 2 -t 0 -m 27 -M 27 -n 50 -q)
add_lte_test(pdsch_test_cdd_75  pdsch_test -x 3 -a 2 -t 0 -m 27 -M 27 -n 75 -q)
add_lte_test(pdsch_test_cdd_100 pdsch_test -x 3 -a 2 -t 0 -m 27 -M 27 -n 100 -q)

# PDSCH test for Spatial Multiplex transmision mode with PMI = 0 (1 codeword)
add_lte_test(pdsch_test_multiplex1cw_p0_6   pdsch_test -x 4 -a 2 -p 0 -n 6)
add_lte_test(pdsch_test_multiplex1cw_p0_12  pdsch_test -x 4 -a 2 -p 0 -n 12)
add_lte_test(pdsch_test_multiplex1cw_p0_25  pdsch_test -x 4 -a 2 -p 0 -n 25)
add_lte_test(pdsch_test_multiplex1cw_p0_50  pdsch_test -x 4 -a 2 -p 0 -n 50)
add_lte_test(pdsch_test_multiplex1cw_p0_75  pdsch_test -x 4 -a 2 -p 0 -n 75)
add_lte_test(pdsch_test_multiplex1cw_p0_100 pdsch_test -x 4 -a 2 -p 0 -n 100)

# PDSCH test for Spatial Multiplex transmision mode with PMI = 0 (1 codeword, swapped)
add_lte_test(pdsch_test_multiplex1cw_p0_swap_6   pdsch_test -x 4 -a 2 -p 0 -w -n 6)
add_lte_test(pdsch_test_multiplex1cw_p0_swap_12  pdsch_test -x 4 -a 2 -p 0 -w -n 12)
add_lte_test(pdsch_test_multiplex1cw_p0_swap_25  pdsch_test -x 4 -a 2 -p 0 -w -n 25)
add_lte_test(pdsch_test_multiplex1cw_p0_swap_50  pdsch_test -x 4 -a 2 -p 0 -w -n 50)
add_lte_test(pdsch_test_multiplex1cw_p0_swap_75  pdsch_test -x 4 -a 2 -p 0 -w -n 75)
add_lte_test(pdsch_test_multiplex1cw_p0_swap_100 pdsch_test -x 4 -a 2 -p 0 -w -n 100)

# PDSCH test for Spatial Multiplex transmision mode with PMI = 1 (1 codeword)
add_lte_test(pdsch_test_multiplex1cw_p1_6   pdsch_test -x 4 -a 2 -p 1 -n 6)
add_lte_test(pdsch_test_multiplex1cw_p1_12  pdsch_test -x 4 -a 2 -p 1 -n 12)
add_lte_test(pdsch_test_multiplex1cw_p1_25  pdsch_test -x 4 -a 2 -p 1 -n 25)
add_lte_test(pdsch_test_multiplex1cw_p1_50  pdsch_test -x 4 -a 2 -p 1 -n 50)
add_lte_test(pdsch_test_multiplex1cw_p1_75  pdsch_test -x 4 -a 2 -p 1 -n 75)
add_lte_test(pdsch_test_multiplex1cw_p1_100 pdsch_test -x 4 -a 2 -p 1 -n 100)

# PDSCH test for Spatial Multiplex transmision mode with PMI = 2 (1 codeword)
add_lte_test(pdsch_test_multiplex1cw_p2_6   pdsch_test -x 4 -a 2 -p 2 -n 6)
add_lte_test(pdsch_test_multiplex1cw_p2_12  pdsch_test -x 4 -a 2 -p 2 -n 12)
add_lte_test(pdsch_test_multiplex1cw_p2_25  pdsch_test -x 4 -a 2 -p 2 -n 25)
add_lte_test(pdsch_test_multiplex1cw_p2_50  pdsch_test -x 4 -a 2 -p 2 -n 50)
add_lte_test(pdsch_test_multiplex1cw_p2_75  pdsch_test -x 4 -a 2 -p 2 -n 75)
add_lte_test(pdsch_test_multiplex1cw_p2_100 pdsch_test -x 4 -a 2 -p 2 -n 100)

# PDSCH test for Spatial Multiplex transmision mode with PMI = 3 (1 codeword)
add_lte_test(pdsch_test_multiplex1cw_p3_6   pdsch_test -x 4 -a 2 -p 3 -n 6)
add_lte_test(pdsch_test_multiplex1cw_p3_12  pdsch_test -x 4 -a 2 -p 3 -n 12)
add_lte_test(pdsch_test_multiplex1cw_p3_25  pdsch_test -x 4 -a 2 -p 3 -n 25)
add_lte_test(pdsch_test_multiplex1cw_p3_50  pdsch_test -x 4 -a 2 -p 3 -n 50)
add_lte_test(pdsch_test_multiplex1cw_p3_75  pdsch_test -x 4 -a 2 -p 3 -n 75)
add_lte_test(pdsch_test_multiplex1cw_p3_100 pdsch_test -x 4 -a 2 -p 3 -n 100)

# PDSCH test for Spatial Multiplex transmision mode with PMI = 0 (2 codeword)
add_lte_test(pdsch_test_multiplex2cw_p0_6   pdsch_test -x 4 -a 2 -t 0 -p 0 -n 6)
add_lte_test(pdsch_test_multiplex2cw_p0_12  pdsch_test -x 4 -a 2 -t 0 -p 0 -n 12)
add_lte_test(pdsch_test_multiplex2cw_p0_25  pdsch_test -x 4 -a 2 -t 0 -p 0 -n 25)
add_lte_test(pdsch_test_multiplex2cw_p0_50  pdsch_test -x 4 -a 2 -t 0 -p 0 -n 50)
add_lte_test(pdsch_test_multiplex2cw_p0_75  pdsch_test -x 4 -a 2 -t 0 -p 0 -n 75)
add_lte_test(pdsch_test_multiplex2cw_p0_100 pdsch_test -x 4 -a 2 -t 0 -p 0 -n 100)

# PDSCH test for Spatial Multiplex transmision mode with PMI = 0 (2 codeword, swapped)
add_lte_test(pdsch_test_multiplex2cw_p0_6_swap   pdsch_test -x 4 -a 2 -t 0 -p 0 -M 28 -n 6 -w -F 1)
add_lte_test(pdsch_test_multiplex2cw_p0_12_swap  pdsch_test -x 4 -a 2 -t 0 -p 0 -m 28 -n 12 -w)
add_lte_test(pdsch_test_multiplex2cw_p0_25_swap  pdsch_test -x 4 -a 2 -t 0 -p 0 -M 28 -n 25 -w)
add_lte_test(pdsch_test_multiplex2cw_p0_50_swap  pdsch_test -x 4 -a 2 -t 0 -p 0 -m 28 -n 50 -w)
add_lte_test(pdsch_test_multiplex2cw_p0_75_swap  pdsch_test -x 4 -a 2 -t 0 -p 0 -M 28 -n 75 -w)
add_lte_test(pdsch_test_multiplex2cw_p0_100_swap pdsch_test -x 4 -a 2 -t 0 -p 0 -m 28 -n 100 -w)

# PDSCH test for Spatial Multiplex transmision mode with PMI = 1 (2 codeword)
add_lte_test(pdsch_test_multiplex2cw_p1_6   pdsch_test -x 4 -a 2 -t 0 -p 1 -n 6)
add_lte_test(pdsch_test_multiplex2cw_p1_12  pdsch_test -x 4 -a 2 -t 0 -p 1 -n 12)
add_lte_test(pdsch_test_multiplex2cw_p1_25  pdsch_test -x 4 -a 2 -t 0 -p 1 -n 25)
add_lte_test(pdsch_test_multiplex2cw_p1_50  pdsch_test -x 4 -a 2 -t 0 -p 1 -n 50)
add_lte_test(pdsch_test_multiplex2cw_p1_75  pdsch_test -x 4 -a 2 -t 0 -p 1 -n 75)
add_lte_test(pdsch_test_multiplex2cw_p1_100 pdsch_test -x 4 -a 2 -t 0 -p 1 -n 100)

########################################################################
# PMCH TEST
########################################################################

add_executable(pmch_test pmch_test.c)
target_link_libraries(pmch_test isrran_phy)

add_lte_test(pmch_test_qpsk pmch_test -m 6 -n 50)
add_lte_test(pmch_test_qam16 pmch_test -m 15 -n 100)
add_lte_test(pmch_test_qam64 pmch_test -m 25 -n 100)


########################################################################
# NPDSCH TEST
########################################################################

add_executable(npdsch_test npdsch_test.c)
target_link_libraries(npdsch_test isrran_phy)

add_lte_test(npdsch_test_tbs208 npdsch_test -m 12)
add_lte_test(npdsch_test_tbs104 npdsch_test -m 7)
add_lte_test(npdsch_test_tbs40 npdsch_test -m 3)
add_lte_test(npdsch_test_tbs16 npdsch_test -m 0)

# Resource element extraction for different operation modes and cell configuration
# Standalone mode with one Tx port gives maximum number of NPDSCH symbols
add_lte_test(npdsch_test_cellid0_standalone_1port npdsch_test -l 0 -M 3 -x 160)
add_lte_test(npdsch_test_cellid1_standalone_1port npdsch_test -l 1 -M 3 -x 160)
add_lte_test(npdsch_test_cellid2_standalone_1port npdsch_test -l 2 -M 3 -x 160)
add_lte_test(npdsch_test_cellid3_standalone_1port npdsch_test -l 3 -M 3 -x 160)
add_lte_test(npdsch_test_cellid4_standalone_1port npdsch_test -l 4 -M 3 -x 160)
add_lte_test(npdsch_test_cellid5_standalone_1port npdsch_test -l 5 -M 3 -x 160)

# Standalone mode with two Tx ports
add_lte_test(npdsch_test_cellid0_standalone_2port npdsch_test -l 0 -M 3 -P 2 -x 152)
add_lte_test(npdsch_test_cellid1_standalone_2port npdsch_test -l 1 -M 3 -P 2 -x 152)
add_lte_test(npdsch_test_cellid2_standalone_2port npdsch_test -l 2 -M 3 -P 2 -x 152)
add_lte_test(npdsch_test_cellid3_standalone_2port npdsch_test -l 3 -M 3 -P 2 -x 152)
add_lte_test(npdsch_test_cellid4_standalone_2port npdsch_test -l 4 -M 3 -P 2 -x 152)
add_lte_test(npdsch_test_cellid5_standalone_2port npdsch_test -l 5 -M 3 -P 2 -x 152)

# Inband same PCI with 1 LTE antenna port and two NB-IoT ports
add_lte_test(npdsch_test_cellid0_inband_1port_2port npdsch_test -l 0 -M 0 -p 1 -P 2 -x 144)
add_lte_test(npdsch_test_cellid1_inband_1port_2port npdsch_test -l 1 -M 0 -p 1 -P 2 -x 144)
add_lte_test(npdsch_test_cellid2_inband_1port_2port npdsch_test -l 2 -M 0 -p 1 -P 2 -x 144)
add_lte_test(npdsch_test_cellid3_inband_1port_2port npdsch_test -l 3 -M 0 -p 1 -P 2 -x 144)
add_lte_test(npdsch_test_cellid4_inband_1port_2port npdsch_test -l 4 -M 0 -p 1 -P 2 -x 144)
add_lte_test(npdsch_test_cellid5_inband_1port_2port npdsch_test -l 5 -M 0 -p 1 -P 2 -x 144)

# Inband same PCI with 2 antenna ports each
add_lte_test(npdsch_test_cellid0_inband_2port_2port npdsch_test -l 0 -M 0 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid1_inband_2port_2port npdsch_test -l 1 -M 0 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid2_inband_2port_2port npdsch_test -l 2 -M 0 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid3_inband_2port_2port npdsch_test -l 3 -M 0 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid4_inband_2port_2port npdsch_test -l 4 -M 0 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid5_inband_2port_2port npdsch_test -l 5 -M 0 -p 2 -P 2 -x 136)

# Inband different PCI with 2 antenna ports each
add_lte_test(npdsch_test_cellid0_inband_diffpci_2port_2port npdsch_test -l 0 -M 1 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid1_inband_diffpci_2port_2port npdsch_test -l 1 -M 1 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid2_inband_diffpci_2port_2port npdsch_test -l 2 -M 1 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid3_inband_diffpci_2port_2port npdsch_test -l 3 -M 1 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid4_inband_diffpci_2port_2port npdsch_test -l 4 -M 1 -p 2 -P 2 -x 136)
add_lte_test(npdsch_test_cellid5_inband_diffpci_2port_2port npdsch_test -l 5 -M 1 -p 2 -P 2 -x 136)

########################################################################
# NB-IoT DCI TEST
########################################################################

add_executable(dci_nbiot_test dci_nbiot_test.c)
target_link_libraries(dci_nbiot_test isrran_phy)
add_lte_test(dci_nbiot_test dci_nbiot_test)

########################################################################
# FILE TEST
########################################################################

add_executable(pbch_file_test pbch_file_test.c)
target_link_libraries(pbch_file_test isrran_phy)

add_executable(pcfich_file_test pcfich_file_test.c)
target_link_libraries(pcfich_file_test isrran_phy)

add_executable(phich_file_test phich_file_test.c)
target_link_libraries(phich_file_test isrran_phy)

add_executable(pdcch_file_test pdcch_file_test.c)
target_link_libraries(pdcch_file_test isrran_phy)

add_executable(pdsch_pdcch_file_test pdsch_pdcch_file_test.c)
target_link_libraries(pdsch_pdcch_file_test isrran_phy)

add_executable(npbch_file_test npbch_file_test.c)
target_link_libraries(npbch_file_test isrran_phy)

add_executable(npdsch_npdcch_file_test npdsch_npdcch_file_test.c)
target_link_libraries(npdsch_npdcch_file_test isrran_phy)

add_lte_test(pbch_file_test pbch_file_test -i ${CMAKE_CURRENT_SOURCE_DIR}/signal.1.92M.dat)

add_executable(pmch_file_test pmch_file_test.c)
target_link_libraries(pmch_file_test isrran_phy)

add_lte_test(pbch_file_test pbch_file_test -i ${CMAKE_CURRENT_SOURCE_DIR}/signal.1.92M.dat)
add_lte_test(pcfich_file_test pcfich_file_test -c 150 -n 50 -p 2 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal.10M.dat)
add_lte_test(phich_file_test phich_file_test -c 150 -n 50 -p 2 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal.10M.dat)
add_lte_test(pdcch_file_test pdcch_file_test -c 1 -f 3 -n 6 -p 1 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal.1.92M.amar.dat)
add_lte_test(pdsch_pdcch_file_test pdsch_pdcch_file_test -c 1 -f 3 -n 6 -p 1 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal.1.92M.amar.dat)
add_lte_test(pmch_file_test pmch_file_test  -i ${CMAKE_CURRENT_SOURCE_DIR}/pmch_100prbs_MCS2_SR0.bin)

########################################################################
# NPDCCH TEST
########################################################################

add_executable(npdcch_test npdcch_test.c)
target_link_libraries(npdcch_test isrran_phy)
add_lte_test(npdcch_formatN1_test npdcch_test -o FormatN1)

add_executable(npdcch_file_test npdcch_file_test.c)
target_link_libraries(npdcch_file_test isrran_phy)
add_lte_test(npdcch_formatN0_file_test npdcch_file_test -c 0 -t 8624 -r 258 -L 1 -l 0 -v -o FormatN0 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_nbiot_dci_formatN0_L_1_nid0_tti_8624_rnti_0x102.bin)
add_lte_test(npdcch_formatN1_file_test npdcch_file_test -c 0 -t 5461 -r 137 -L 2 -l 0 -v -o FormatN1 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_nbiot_dci_formatN1_nid0_tti_5461_rnti_0x89.bin)

add_lte_test(npdsch_npdcch_dci_formatN0_test npdsch_npdcch_file_test -c 0 -s 4 -w 862 -r 0x102 -v -o FormatN0 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_nbiot_dci_formatN0_L_1_nid0_tti_8624_rnti_0x102.bin)
add_lte_test(npdsch_npdcch_dci_formatN1_test npdsch_npdcch_file_test -c 0 -s 1 -w 546 -r 0x89 -v -o FormatN1 -i ${CMAKE_CURRENT_SOURCE_DIR}/signal_nbiot_dci_formatN1_nid0_tti_5461_rnti_0x89.bin)

########################################################################
# PUSCH TEST
########################################################################

add_executable(pusch_test pusch_test.c)
target_link_libraries(pusch_test isrran_phy)

if (${ENABLE_ALL_TEST})
  # All valid number of PRBs for PUSCH
  set(cell_n_prb_valid 1 2 3 4 5 6 8 9 10 12 15 16 18 20 24 25 27 30 32 36 40 45 48 50 54 60 64 72 75 80 81 90 96 100)

  set(pusch_min_mcs 0)
  set(pusch_max_mcs 28)
  set(pusch_step_mcs 1)

  set(pusch_acks 0 1 2 4 5 10)

  set(pusch_cqi none wideband)

else ()
  set(cell_n_prb_valid 50)

  set(pusch_min_mcs 0)
  set(pusch_max_mcs 28)
  set(pusch_step_mcs 7)

  set(pusch_acks 0 1 2 10)

  set(pusch_cqi none wideband)

endif ()

foreach (cell_n_prb 6 15 25 50 75 100)
  set(pusch_cell_n_prb)
  foreach (n_prb ${cell_n_prb_valid})
    if (NOT (${n_prb} GREATER ${cell_n_prb}))
      set(pusch_cell_n_prb ${pusch_cell_n_prb} ${n_prb})
    endif (NOT (${n_prb} GREATER ${cell_n_prb}))
  endforeach (n_prb)

  foreach (n_prb ${pusch_cell_n_prb})
    foreach (mcs RANGE ${pusch_min_mcs} ${pusch_max_mcs} ${pusch_step_mcs})
      foreach (ack ${pusch_acks})
        foreach (cqi ${pusch_cqi})
          set(pusch_test_args "")

          set(pusch_test_args ${pusch_test_args} -n ${cell_n_prb})
          set(pusch_test_args ${pusch_test_args} -L ${n_prb})

          if (NOT (${ack} EQUAL 0))
            set(pusch_test_args ${pusch_test_args} -p uci_ack ${ack})
            if (mcs EQUAL 28)
              set(mcs 27)
            endif (mcs EQUAL 28)
          endif (NOT (${ack} EQUAL 0))

          if (NOT (${cqi} STREQUAL none))
            set(pusch_test_args ${pusch_test_args} -p cqi ${cqi})
            if (mcs EQUAL 28)
              set(mcs 27)
            endif (mcs EQUAL 28)
          endif (NOT (${cqi} STREQUAL none))

          if (${mcs} GREATER 24)
            set(pusch_test_args ${pusch_test_args} -p enable_64qam)
          endif(${mcs} GREATER 24)

          set(pusch_test_args ${pusch_test_args} -m ${mcs})

          string(REGEX REPLACE "\ " "" test_name_args ${pusch_test_args})

          add_lte_test(pusch_test${test_name_args} pusch_test ${pusch_test_args})
        endforeach (cqi)
      endforeach (ack)
    endforeach (mcs)
  endforeach (n_prb)
endforeach (cell_n_prb)

########################################################################
# PUCCH TEST
########################################################################

add_executable(pucch_test pucch_test.c)
target_link_libraries(pucch_test isrran_phy)

add_lte_test(pucch_test pucch_test)
add_lte_test(pucch_test_uci_cqi_decoder pucch_test -q)

########################################################################
# PRACH TEST
########################################################################

add_executable(prach_test prach_test.c)
target_link_libraries(prach_test isrran_phy)

add_lte_test(prach prach_test)

add_lte_test(prach_256 prach_test -n 15)
add_lte_test(prach_512 prach_test -n 25)
add_lte_test(prach_1024 prach_test -n 50)
add_lte_test(prach_1536 prach_test -n 75)
add_lte_test(prach_2048 prach_test -n 100)

add_lte_test(prach_f0 prach_test -f 0)
add_lte_test(prach_f1 prach_test -f 1)
add_lte_test(prach_f2 prach_test -f 2)
add_lte_test(prach_f3 prach_test -f 3)

add_lte_test(prach_rs1 prach_test -r 1)
add_lte_test(prach_rs2 prach_test -r 2)
add_lte_test(prach_rs3 prach_test -r 3)

add_lte_test(prach_zc0 prach_test -z 0)
add_lte_test(prach_zc2 prach_test -z 2)
add_lte_test(prach_zc3 prach_test -z 3)

add_nr_test(prach_nr prach_test -n 50 -f 0 -r 0 -z 0 -N 1)

add_executable(prach_test_multi prach_test_multi.c)
target_link_libraries(prach_test_multi isrran_phy)

add_lte_test(prach_test_multi prach_test_multi)

add_lte_test(prach_test_multi_n32 prach_test_multi -n 32)
add_lte_test(prach_test_multi_n16 prach_test_multi -n 16)
add_lte_test(prach_test_multi_n8 prach_test_multi -n 8)
add_lte_test(prach_test_multi_n4 prach_test_multi -n 4)

add_lte_test(prach_test_multi_stagger_power prach_test_multi -s -S)
add_lte_test(prach_test_multi_stagger_power prach_test_multi -s -S -N 50)

add_lte_test(prach_test_multi_offset_test prach_test_multi -O)
add_lte_test(prach_test_multi_offset_test_50 prach_test_multi -O -N 50)

add_lte_test(prach_test_multi_freq_offset_test_n1_o100_prb6 prach_test_multi -n 1 -F -z 0 -o 100)
add_lte_test(prach_test_multi_freq_offset_test_n1_o500_prb6 prach_test_multi -n 1 -F -z 0 -o 500)
add_lte_test(prach_test_multi_freq_offset_test_n1_o800_prb6 prach_test_multi -n 1 -F -z 0 -o 800)

add_lte_test(prach_test_multi_freq_offset_test_n1_o100_prb50 prach_test_multi -n 1 -F -z 0 -o 100 -N 50)
add_lte_test(prach_test_multi_freq_offset_test_n1_o500_prb50 prach_test_multi -n 1 -F -z 0 -o 500 -N 50)
add_lte_test(prach_test_multi_freq_offset_test_n1_o800_prb50 prach_test_multi -n 1 -F -z 0 -o 800 -N 50)


add_lte_test(prach_test_multi_freq_offset_test_n2_o100_prb6 prach_test_multi -n 2 -F -z 0 -o 100)
add_lte_test(prach_test_multi_freq_offset_test_n2_o500_prb6 prach_test_multi -n 2 -F -z 0 -o 500)
add_lte_test(prach_test_multi_freq_offset_test_n2_o800_prb6 prach_test_multi -n 2 -F -z 0 -o 800)

add_lte_test(prach_test_multi_freq_offset_test_n2_o100_prb50 prach_test_multi -n 2 -F -z 0 -o 100 -N 50)
add_lte_test(prach_test_multi_freq_offset_test_n2_o500_prb50 prach_test_multi -n 2 -F -z 0 -o 500 -N 50)
add_lte_test(prach_test_multi_freq_offset_test_n2_o800_prb50 prach_test_multi -n 2 -F -z 0 -o 800 -N 50)

add_lte_test(prach_test_multi_freq_offset_test_n4_o100_prb6 prach_test_multi -n 4 -F -z 0 -o 100)
add_lte_test(prach_test_multi_freq_offset_test_n4_o500_prb6 prach_test_multi -n 4 -F -z 0 -o 500)
add_lte_test(prach_test_multi_freq_offset_test_n4_o800_prb6 prach_test_multi -n 4 -F -z 0 -o 800)

add_lte_test(prach_test_multi_freq_offset_test_n4_o100_prb50 prach_test_multi -n 4 -F -z 0 -o 100 -N 50)
add_lte_test(prach_test_multi_freq_offset_test_n4_o500_prb50 prach_test_multi -n 4 -F -z 0 -o 500 -N 50)
add_lte_test(prach_test_multi_freq_offset_test_n4_o800_prb50 prach_test_multi -n 4 -F -z 0 -o 800 -N 50)

if(RF_FOUND)
  add_executable(prach_test_usrp prach_test_usrp.c)
  target_link_libraries(prach_test_usrp isrran_rf isrran_phy pthread)
endif(RF_FOUND)

add_executable(prach_nr_test_perf EXCLUDE_FROM_ALL prach_nr_test_perf.c)
target_link_libraries(prach_nr_test_perf isrran_phy)
# this is just for performance evaluation, not for unit testing

########################################################################
# NR
########################################################################

add_executable(dci_nr_test dci_nr_test.c)
target_link_libraries(dci_nr_test isrran_phy)
add_nr_test(dci_nr_test dci_nr_test)

add_executable(mib_nr_test mib_nr_test.c)
target_link_libraries(mib_nr_test isrran_phy)
add_nr_test(mib_nr_test mib_nr_test)

add_executable(pucch_nr_test pucch_nr_test.c)
target_link_libraries(pucch_nr_test isrran_phy)
add_nr_test(pucch_nr_test pucch_nr_test)

add_executable(sch_nr_test sch_nr_test.c)
target_link_libraries(sch_nr_test isrran_phy)
add_nr_test(sch_nr_test sch_nr_test -P 52 -p 1 -r 0)
add_nr_test(sch_nr_test sch_nr_test -P 52 -p 1 -r 1)
add_nr_test(sch_nr_test sch_nr_test -P 52 -p 10 -r 0)
add_nr_test(sch_nr_test sch_nr_test -P 52 -p 10 -r 1)
add_nr_test(sch_nr_test sch_nr_test -P 52 -p 20 -r 0)
add_nr_test(sch_nr_test sch_nr_test -P 52 -p 20 -r 1)
add_nr_test(sch_nr_test sch_nr_test -P 52 -p 52 -r 0)
add_nr_test(sch_nr_test sch_nr_test -P 52 -p 52 -r 1)

add_executable(pdsch_nr_test pdsch_nr_test.c)
target_link_libraries(pdsch_nr_test isrran_phy)
add_nr_test(pdsch_nr_test pdsch_nr_test -p 6 -m 20)

add_executable(pusch_nr_test pusch_nr_test.c)
target_link_libraries(pusch_nr_test isrran_phy)
add_nr_test(pusch_nr_test pusch_nr_test -p 6 -m 20)
add_nr_test(pusch_nr_ack1_test pusch_nr_test -p 50 -m 20 -A 1)
add_nr_test(pusch_nr_ack2_test pusch_nr_test -p 50 -m 20 -A 2)
add_nr_test(pusch_nr_ack4_test pusch_nr_test -p 50 -m 20 -A 4)
add_nr_test(pusch_nr_ack20_test pusch_nr_test -p 50 -m 20 -A 20)
add_nr_test(pusch_nr_csi4_test pusch_nr_test -p 50 -m 20 -C 4)
add_nr_test(pusch_nr_csi20_test pusch_nr_test -p 50 -m 20 -C 20)
add_nr_test(pusch_nr_ack1_csi4_test pusch_nr_test -p 50 -m 20 -A 1 -C 4)
add_nr_test(pusch_nr_ack2_csi4_test pusch_nr_test -p 50 -m 20 -A 2 -C 4)
add_nr_test(pusch_nr_ack4_csi4_test pusch_nr_test -p 50 -m 20 -A 4 -C 4)
add_nr_test(pusch_nr_ack20_csi4_test pusch_nr_test -p 50 -m 20 -A 20 -C 4)

add_executable(pusch_nr_bler_test EXCLUDE_FROM_ALL pusch_nr_bler_test.c)
target_link_libraries(pusch_nr_bler_test isrran_phy)
# this is just for performance evaluation, not for unit testing

add_executable(pdcch_nr_test pdcch_nr_test.c)
target_link_libraries(pdcch_nr_test isrran_phy)
add_nr_test(pdcch_nr_test_non_interleaved pdcch_nr_test)
add_nr_test(pdcch_nr_test_interleaved pdcch_nr_test -I)
