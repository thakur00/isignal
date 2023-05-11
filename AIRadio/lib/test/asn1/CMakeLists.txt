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

add_executable(s1ap_asn1_test s1ap_test.cc)
target_link_libraries(s1ap_asn1_test isrran_common isrran_asn1 s1ap_asn1 ${ATOMIC_LIBS})
add_test(s1ap_asn1_test s1ap_asn1_test)

add_executable(isrran_asn1_rrc_mcch_test isrran_asn1_rrc_mcch_test.cc)
target_link_libraries(isrran_asn1_rrc_mcch_test rrc_asn1 isrran_common)
add_test(isrran_asn1_rrc_mcch_test isrran_asn1_rrc_mcch_test)

add_executable(isrran_asn1_rrc_meas_test isrran_asn1_rrc_meas_test.cc)
target_link_libraries(isrran_asn1_rrc_meas_test rrc_asn1 rrc_nr_asn1 isrran_common)
add_test(isrran_asn1_rrc_meas_test isrran_asn1_rrc_meas_test)

add_executable(isrran_asn1_rrc_ul_dcch_test isrran_asn1_rrc_ul_dcch_test.cc)
target_link_libraries(isrran_asn1_rrc_ul_dcch_test rrc_asn1 isrran_common)
add_test(isrran_asn1_rrc_ul_dcch_test isrran_asn1_rrc_ul_dcch_test)

add_executable(isrran_asn1_rrc_dl_ccch_test isrran_asn1_rrc_dl_ccch_test.cc)
target_link_libraries(isrran_asn1_rrc_dl_ccch_test rrc_asn1 isrran_common)
add_test(isrran_asn1_rrc_dl_ccch_test isrran_asn1_rrc_dl_ccch_test)

add_executable(isrran_asn1_rrc_dl_dcch_test isrran_asn1_rrc_dl_dcch_test.cc)
target_link_libraries(isrran_asn1_rrc_dl_dcch_test rrc_asn1 isrran_common)
add_test(isrran_asn1_rrc_dl_dcch_test isrran_asn1_rrc_dl_dcch_test)

add_executable(isrran_asn1_nas_test isrran_asn1_nas_test.cc)
target_link_libraries(isrran_asn1_nas_test isrran_common isrran_phy isrran_asn1)
add_test(isrran_asn1_nas_test isrran_asn1_nas_test)

add_executable(asn1_utils_test asn1_utils_test.cc)
target_link_libraries(asn1_utils_test asn1_utils isrran_common)
add_test(asn1_utils_test asn1_utils_test)

add_executable(rrc_asn1_test rrc_test.cc)
target_link_libraries(rrc_asn1_test rrc_asn1 asn1_utils isrran_common)
add_test(rrc_asn1_test rrc_asn1_test)

add_executable(isrran_asn1_rrc_nr_test isrran_asn1_rrc_nr_test.cc)
target_link_libraries(isrran_asn1_rrc_nr_test rrc_nr_asn1 asn1_utils isrran_common isrran_mac)
add_test(isrran_asn1_rrc_nr_test isrran_asn1_rrc_nr_test)

add_executable(ngap_asn1_test ngap_test.cc)
target_link_libraries(ngap_asn1_test ngap_nr_asn1 isrran_common)
add_test(ngap_asn1_test ngap_asn1_test)

add_executable(rrc_nr_utils_test rrc_nr_utils_test.cc)
target_link_libraries(rrc_nr_utils_test ngap_nr_asn1 isrran_common rrc_nr_asn1)
add_test(rrc_nr_utils_test rrc_nr_utils_test)

add_executable(rrc_asn1_decoder rrc_asn1_decoder.cc)
target_link_libraries(rrc_asn1_decoder rrc_asn1)

add_executable(nas_decoder nas_decoder.cc)
target_link_libraries(nas_decoder isrran_asn1)

add_executable(nas_5g_msg_test nas_5g_msg_test.cc)
target_link_libraries(nas_5g_msg_test nas_5g_msg)