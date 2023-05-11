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

#ifndef ISRRAN_NAS_5G_UTILS_H
#define ISRRAN_NAS_5G_UTILS_H

#include "isrran/asn1/asn1_utils.h"
#include "isrran/common/byte_buffer.h"
#include "isrran/config.h"

using namespace asn1;
namespace isrran {
namespace nas_5g {

struct ecies_scheme_profile_a_out {
  uint8_t              ecc_ephemeral_key[33];
  std::vector<uint8_t> ciphertext;
  uint8_t              mac_tag[8];
};

struct ecies_scheme_profile_b_out {
  uint8_t              ecc_ephemeral_key[32];
  std::vector<uint8_t> ciphertext;
  uint8_t              mac_tag[8];
};

template <typename Enum, int bl>
ISRASN_CODE unpack_enum(asn1::cbit_ref& bref, Enum* e)
{
  uint32_t tmp = {};
  HANDLE_CODE(bref.unpack(tmp, bl));
  *e = static_cast<Enum>(tmp);
  return ISRASN_SUCCESS;
}

template <typename Enum, int bl>
ISRASN_CODE pack_enum(asn1::bit_ref& bref, Enum e)
{
  uint32_t tmp = static_cast<uint32_t>(e);
  HANDLE_CODE(bref.pack(tmp, bl));
  return ISRASN_SUCCESS;
}

template <class EnumType, uint32_t bit_length_>
class nas_enumerated : public EnumType
{
public:
  static const uint32_t bit_length = bit_length_;

  nas_enumerated() {}
  nas_enumerated(typename EnumType::options o) { EnumType::value = o; }
  ISRASN_CODE pack(asn1::bit_ref& bref) const
  {
    uint32_t tmp = static_cast<uint32_t>(EnumType::value);
    HANDLE_CODE(bref.pack(tmp, bit_length));
    return ISRASN_SUCCESS;
  }
  ISRASN_CODE unpack(asn1::cbit_ref& bref)
  {
    uint32_t tmp = {};
    HANDLE_CODE(bref.unpack(tmp, bit_length));
    *this = static_cast<typename EnumType::options>(tmp);
    return ISRASN_SUCCESS;
  }
  EnumType& operator=(EnumType v)
  {
    EnumType::value = v;
    return *this;
  }
  operator typename EnumType::options() const { return EnumType::value; }
};

ISRASN_CODE unpack_mcc_mnc(uint8_t* mcc_bytes, uint8_t* mnc_bytes, asn1::cbit_ref& bref);
ISRASN_CODE pack_mcc_mnc(uint8_t* mcc_bytes, uint8_t* mnc_bytes, asn1::bit_ref& bref);

} // namespace nas_5g
} // namespace isrran
#endif // MANUAL_H