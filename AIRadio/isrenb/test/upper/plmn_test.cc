/**
 * Copyright 2013-2022 iSignal Research Labs Pvt Ltd.
 *
 * This file is part of isrRAN.
 *
 * isrRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * isrRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "isrenb/hdr/common/common_enb.h"
#include "isrran/asn1/rrc/common.h"
#include "isrran/asn1/rrc_utils.h"
#include "isrran/common/bcd_helpers.h"
#include "isrran/interfaces/rrc_interface_types.h"
#include <arpa/inet.h> //for inet_ntop()
#include <iostream>

using namespace asn1::rrc;

#define TESTASSERT(cond)                                                                                               \
  {                                                                                                                    \
    if (!(cond)) {                                                                                                     \
      std::cout << "[" << __FUNCTION__ << "][Line " << __LINE__ << "]: FAIL at " << (#cond) << std::endl;              \
      return -1;                                                                                                       \
    }                                                                                                                  \
  }

int rrc_plmn_test()
{
  plmn_id_s plmn_in, plmn_out;
  uint8_t   ref[3] = {0x89, 0x19, 0x14};
  uint8_t   byte_buf[4];

  // 2-digit MNC test
  asn1::cbit_ref bref_out(&ref[0], sizeof(ref));

  plmn_out.unpack(bref_out);

  TESTASSERT(plmn_out.mcc_present);
  uint16_t mcc, mnc;
  isrran::bytes_to_mcc(&plmn_out.mcc[0], &mcc);
  isrran::bytes_to_mnc(&plmn_out.mnc[0], &mnc, plmn_out.mnc.size());
  TESTASSERT(mcc == 0xF123);
  TESTASSERT(mnc == 0xFF45);

  // Test MCC/MNC --> vector
  plmn_in.mcc_present = plmn_out.mcc_present;
  TESTASSERT(isrran::mcc_to_bytes(mcc, &plmn_in.mcc[0]));
  TESTASSERT(isrran::mnc_to_bytes(mnc, plmn_in.mnc));
  TESTASSERT(plmn_in.mcc_present == plmn_out.mcc_present);
  TESTASSERT(plmn_in.mcc == plmn_out.mcc);
  TESTASSERT(plmn_in.mnc == plmn_out.mnc);

  // Test plmn --> string
  isrran::plmn_id_t isrplmn_out = isrran::make_plmn_id_t(plmn_out);
  TESTASSERT(isrplmn_out.to_string() == "12345");

  asn1::bit_ref bref_in(&byte_buf[0], sizeof(byte_buf));
  asn1::bit_ref bref_in0(&byte_buf[0], sizeof(byte_buf));
  plmn_out.pack(bref_in);

  TESTASSERT(bref_in.distance() == bref_out.distance());
  TESTASSERT(memcmp(&ref[0], &byte_buf[0], sizeof(ref)) == 0);

  // 3-digit MNC test
  TESTASSERT(isrran::mnc_to_bytes(0xF456, plmn_in.mnc));
  bref_in = asn1::bit_ref(&byte_buf[0], sizeof(byte_buf));
  plmn_in.pack(bref_in);
  uint8_t ref2[4] = {0x89, 0x1D, 0x15, 0x80};
  TESTASSERT(bref_in.distance(bref_in0) == (1 + 3 * 4 + 1 + 3 * 4));
  TESTASSERT(memcmp(&byte_buf[0], &ref2[0], sizeof(ref)) == 0);

  bref_out = asn1::cbit_ref(&ref2[0], sizeof(ref2));
  plmn_out.unpack(bref_out);
  TESTASSERT(plmn_in.mcc_present == plmn_out.mcc_present);
  TESTASSERT(plmn_in.mcc == plmn_out.mcc);
  TESTASSERT(plmn_in.mnc == plmn_out.mnc);

  return 0;
}

int s1ap_plmn_test()
{
  uint16_t                       mcc = 0xF123;
  uint16_t                       mnc = 0xFF45;
  uint32_t                       plmn;
  isrran::plmn_id_t              isrran_plmn, isrran_plmn2;
  asn1::fixed_octstring<3, true> s1ap_plmn{};

  // 2-digit MNC test
  isrran::s1ap_mccmnc_to_plmn(mcc, mnc, &plmn);
  TESTASSERT(plmn == 0x21F354);
  isrran::s1ap_plmn_to_mccmnc(plmn, &mcc, &mnc);
  TESTASSERT(mcc == 0xF123);
  TESTASSERT(mnc == 0xFF45);

  // Test MCC/MNC --> S1AP
  isrran_plmn.from_number(mcc, mnc);
  TESTASSERT(isrran_plmn.to_string() == "12345");
  isrran::to_asn1(&s1ap_plmn, isrran_plmn);
  TESTASSERT(s1ap_plmn[0] == ((uint8_t*)&plmn)[2]);
  TESTASSERT(s1ap_plmn[1] == ((uint8_t*)&plmn)[1]);
  TESTASSERT(s1ap_plmn[2] == ((uint8_t*)&plmn)[0]);
  isrran_plmn2 = isrran::make_plmn_id_t(s1ap_plmn);
  TESTASSERT(isrran_plmn2 == isrran_plmn);

  // 3-digit MNC test
  mnc = 0xF456;
  isrran::s1ap_mccmnc_to_plmn(mcc, mnc, &plmn);
  TESTASSERT(plmn == 0x214365);
  isrran::s1ap_plmn_to_mccmnc(plmn, &mcc, &mnc);
  TESTASSERT(mcc == 0xF123);
  TESTASSERT(mnc == 0xF456);

  // Test MCC/MNC --> S1AP
  isrran_plmn.from_number(mcc, mnc);
  TESTASSERT(isrran_plmn.to_string() == "123456");
  isrran::to_asn1(&s1ap_plmn, isrran_plmn);
  TESTASSERT(s1ap_plmn[0] == ((uint8_t*)&plmn)[2]);
  TESTASSERT(s1ap_plmn[1] == ((uint8_t*)&plmn)[1]);
  TESTASSERT(s1ap_plmn[2] == ((uint8_t*)&plmn)[0]);
  isrran_plmn2 = isrran::make_plmn_id_t(s1ap_plmn);
  TESTASSERT(isrran_plmn2 == isrran_plmn);

  return 0;
}

int main(int argc, char** argv)
{
  TESTASSERT(rrc_plmn_test() == 0);
  TESTASSERT(s1ap_plmn_test() == 0);
  return 0;
}
