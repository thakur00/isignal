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

#include "isrran/mac/mac_sch_pdu_nr.h"

namespace isrran {

mac_sch_subpdu_nr::nr_lcid_sch_t mac_sch_subpdu_nr::get_type()
{
  if (lcid >= 32) {
    return (nr_lcid_sch_t)lcid;
  }

  return CCCH;
}

bool mac_sch_subpdu_nr::is_sdu() const
{
  // UL-CCCH handling in done as CE
  return (lcid <= 32 && !is_ul_ccch());
}

bool mac_sch_subpdu_nr::has_length_field()
{
  // CCCH (both versions) don't have a length field in the UL
  if (parent->is_ulsch()) {
    if (lcid == CCCH_SIZE_48 || lcid == CCCH_SIZE_64) {
      return false;
    }
  }
  return (is_sdu() || is_var_len_ce(lcid));
}

// returns false for all reserved values in Table 6.2.1-1 and 6.2.1-2
bool mac_sch_subpdu_nr::is_valid_lcid()
{
  return (lcid <= 63 && ((parent->is_ulsch() && (lcid <= 32 || lcid >= 52)) || (lcid <= 32 || lcid >= 47)));
}

bool mac_sch_subpdu_nr::is_var_len_ce(uint32_t lcid)
{
  if (parent->is_ulsch()) {
    // UL fixed-size CE
    switch (lcid) {
      case LONG_TRUNC_BSR:
      case LONG_BSR:
        return true;
      default:
        return false;
    }
  } else {
    // all currently supported CEs in the DL are fixed-size
    return false;
  }
}

// return length of PDU (or ISRRAN_ERROR otherwise)
int32_t mac_sch_subpdu_nr::read_subheader(const uint8_t* ptr)
{
  // Skip R, read F bit and LCID
  F_bit = (bool)(*ptr & 0x40) ? true : false;
  lcid  = (uint8_t)*ptr & 0x3f;
  ptr++;
  header_length = 1;

  if (is_valid_lcid()) {
    if (has_length_field()) {
      // Read first length byte
      sdu_length = (uint32_t)*ptr;
      ptr++;
      header_length++;

      if (F_bit) {
        // add second length byte
        sdu_length = sdu_length << 8 | ((uint32_t)*ptr & 0xff);
        ptr++;
        header_length++;
      }
    } else {
      sdu_length = sizeof_ce(lcid, parent->is_ulsch());
    }
    sdu.set_storage_to((uint8_t*)ptr);
  } else {
    isrlog::fetch_basic_logger("MAC-NR").warning("Invalid LCID (%d) in MAC PDU", lcid);
    return ISRRAN_ERROR;
  }
  return header_length;
}

void mac_sch_subpdu_nr::set_sdu(const uint32_t lcid_, const uint8_t* payload_, const uint32_t len_)
{
  // Use CCCH_SIZE_48 when SDU len fits
  lcid = (lcid_ == CCCH_SIZE_64 && len_ == sizeof_ce(CCCH_SIZE_48, true)) ? CCCH_SIZE_48 : lcid_;
  sdu.set_storage_to(const_cast<uint8_t*>(payload_));
  header_length = is_ul_ccch() ? 1 : 2;
  sdu_length    = len_;
  if (is_ul_ccch()) {
    F_bit      = false;
    sdu_length = sizeof_ce(lcid, parent->is_ulsch());
    if (len_ != static_cast<uint32_t>(sdu_length)) {
      logger->warning("Invalid SDU length of UL-SCH SDU (%d != %d)", len_, sdu_length);
    }
  }

  if (sdu_length >= MAC_SUBHEADER_LEN_THRESHOLD) {
    F_bit = true;
    header_length += 1;
  }
}

void mac_sch_subpdu_nr::set_padding(const uint32_t len_)
{
  lcid = PADDING;
  // 1 Byte R/LCID MAC subheader
  sdu_length    = len_ - 1;
  header_length = 1;
}

// Turn a subPDU into a C-RNTI CE, error checking takes place in the caller
void mac_sch_subpdu_nr::set_c_rnti(const uint16_t crnti_)
{
  lcid           = CRNTI;
  header_length  = 1;
  sdu_length     = sizeof_ce(lcid, parent->is_ulsch());
  uint16_t crnti = htole16(crnti_);
  uint8_t* ptr   = sdu.use_internal_storage();
  ptr[0]         = (uint8_t)((crnti & 0xff00) >> 8);
  ptr[1]         = (uint8_t)((crnti & 0x00ff));
}

// Turn a subPDU into a single entry PHR CE, error checking takes place in the caller
void mac_sch_subpdu_nr::set_se_phr(const uint8_t phr_, const uint8_t pcmax_)
{
  lcid          = SE_PHR;
  header_length = 1;
  sdu_length    = sizeof_ce(lcid, parent->is_ulsch());
  uint8_t* ptr  = sdu.use_internal_storage();
  ptr[0]        = (uint8_t)(phr_ & 0x3f);
  ptr[1]        = (uint8_t)(pcmax_ & 0x3f);
}

// Turn a subPDU into a single short BSR
void mac_sch_subpdu_nr::set_sbsr(const lcg_bsr_t bsr_)
{
  lcid          = SHORT_BSR;
  header_length = 1;
  sdu_length    = sizeof_ce(lcid, parent->is_ulsch());
  uint8_t* ptr  = sdu.use_internal_storage();
  ptr[0]        = ((bsr_.lcg_id & 0x07) << 5) | (bsr_.buffer_size & 0x1f);
}

// Turn a subPDU into a long BSR with variable size
void mac_sch_subpdu_nr::set_lbsr(const std::array<mac_sch_subpdu_nr::lcg_bsr_t, max_num_lcg_lbsr> bsr_) {}

// Turn subPDU into a Con
void mac_sch_subpdu_nr::set_ue_con_res_id_ce(const mac_sch_subpdu_nr::ue_con_res_id_t id)
{
  lcid          = CON_RES_ID;
  header_length = 1;
  sdu_length    = sizeof_ce(lcid, parent->is_ulsch());
  uint8_t* ptr  = sdu.use_internal_storage();
  for (int32_t i = 0; i < sdu_length; ++i) {
    ptr[i] = id.at(i);
  }
}

// Section 6.1.2
uint32_t mac_sch_subpdu_nr::write_subpdu(const uint8_t* start_)
{
  uint8_t* ptr = const_cast<uint8_t*>(start_);
  *ptr         = (uint8_t)((F_bit ? 1 : 0) << 6) | ((uint8_t)lcid & 0x3f);
  ptr += 1;

  if (header_length == 3) {
    // 3 Byte R/F/LCID/L MAC subheader with 16-bit L field
    *ptr = ((sdu_length & 0xff00) >> 8);
    ptr += 1;
    *ptr = static_cast<uint8_t>(sdu_length);
    ptr += 1;

  } else if (header_length == 2) {
    // 2 Byte R/F/LCID/L MAC subheader with 8-bit L field
    *ptr = static_cast<uint8_t>(sdu_length);
    ptr += 1;
  } else if (header_length == 1) {
    // do nothing
  } else {
    logger->error("Error while packing PDU. Unsupported header length (%d)", header_length);
  }

  // copy SDU payload
  if (sdu) {
    memcpy(ptr, sdu.ptr(), sdu_length);
  } else {
    // clear memory
    memset(ptr, 0, sdu_length);
  }

  ptr += sdu_length;

  // return total length of subpdu
  return ptr - start_;
}

uint32_t mac_sch_subpdu_nr::get_total_length() const
{
  return (header_length + sdu_length);
}

uint32_t mac_sch_subpdu_nr::get_sdu_length() const
{
  return sdu_length;
}

uint32_t mac_sch_subpdu_nr::get_lcid() const
{
  return lcid;
}

uint8_t* mac_sch_subpdu_nr::get_sdu()
{
  return sdu.ptr();
}

const uint8_t* mac_sch_subpdu_nr::get_sdu() const
{
  return sdu.ptr();
}

uint16_t mac_sch_subpdu_nr::get_c_rnti() const
{
  if (parent->is_ulsch() && lcid == CRNTI) {
    const uint8_t* ptr = sdu.ptr();
    return le16toh((uint16_t)ptr[0] << 8 | ptr[1]);
  }
  return 0;
}

uint8_t mac_sch_subpdu_nr::get_phr()
{
  if (parent->is_ulsch() && lcid == SE_PHR) {
    uint8_t* ptr = sdu.ptr();
    return ptr[0] & 0x3f;
  }
  return 0;
}

uint8_t mac_sch_subpdu_nr::get_pcmax()
{
  if (parent->is_ulsch() && lcid == SE_PHR) {
    uint8_t* ptr = sdu.ptr();
    return ptr[1] & 0x3f;
  }
  return 0;
}

mac_sch_subpdu_nr::ta_t mac_sch_subpdu_nr::get_ta()
{
  ta_t ta = {};
  if (lcid == TA_CMD) {
    uint8_t* ptr  = sdu.ptr();
    ta.tag_id     = (ptr[0] & 0xc0) >> 6;
    ta.ta_command = ptr[0] & 0x3f;
  }
  return ta;
}

mac_sch_subpdu_nr::lcg_bsr_t mac_sch_subpdu_nr::get_sbsr() const
{
  lcg_bsr_t sbsr = {};
  if (parent->is_ulsch() && (lcid == SHORT_BSR || lcid == SHORT_TRUNC_BSR)) {
    const uint8_t* ptr = sdu.ptr();
    sbsr.lcg_id        = (ptr[0] & 0xe0) >> 5;
    sbsr.buffer_size   = ptr[0] & 0x1f;
  }
  return sbsr;
}

mac_sch_subpdu_nr::lbsr_t mac_sch_subpdu_nr::get_lbsr() const
{
  lbsr_t lbsr = {};
  lbsr.list.reserve(mac_sch_subpdu_nr::max_num_lcg_lbsr);

  if (parent->is_ulsch() && (lcid == LONG_BSR || lcid == LONG_TRUNC_BSR)) {
    const uint8_t* ptr = sdu.ptr();
    lbsr.bitmap        = *ptr; // read LCG bitmap
    ptr++;                     // skip LCG bitmap

    // early stop if LBSR is empty
    if (lbsr.bitmap == 0) {
      return lbsr;
    }

    int bsr_cnt = 0;
    for (int i = 0; i < mac_sch_subpdu_nr::max_num_lcg_lbsr; i++) {
      // If LCGi bit is enabled, it means the next 8-bit BSR value corresponds to it
      if (lbsr.bitmap & (0x1 << i)) {
        lcg_bsr_t bsr = {};
        bsr.lcg_id    = i;
        // For the Long truncated, some BSR words can be not present, assume BSR > 0 in that case
        if (1 + bsr_cnt < sdu_length) {
          bsr.buffer_size = ptr[bsr_cnt];
          bsr_cnt++;
        } else if (lcid == LONG_TRUNC_BSR) {
          bsr.buffer_size = 63; // just assume it has 526 bytes to transmit
        } else {
          fprintf(stderr, "Error parsing LongBSR CE: sdu_length=%d but there are %d active bsr\n", sdu_length, bsr_cnt);
        }
        lbsr.list.push_back(bsr);
      }
    }
  }

  return lbsr;
}

mac_sch_subpdu_nr::ue_con_res_id_t mac_sch_subpdu_nr::get_ue_con_res_id_ce()
{
  mac_sch_subpdu_nr::ue_con_res_id_t id = {};
  if (!parent->is_ulsch() && lcid == CON_RES_ID) {
    const uint8_t* ptr = sdu.ptr();
    memcpy(id.data(), ptr, id.size());
  }
  return id;
}

uint64_t mac_sch_subpdu_nr::get_ue_con_res_id_ce_packed()
{
  if (!parent->is_ulsch() && lcid == CON_RES_ID) {
    const uint8_t* payload = sdu.ptr();
    return le64toh(((uint64_t)payload[5]) | (((uint64_t)payload[4]) << 8) | (((uint64_t)payload[3]) << 16) |
                   (((uint64_t)payload[2]) << 24) | (((uint64_t)payload[1]) << 32) | (((uint64_t)payload[0]) << 40));
  } else {
    return 0;
  }
}

uint32_t mac_sch_subpdu_nr::sizeof_ce(uint32_t lcid, bool is_ul)
{
  if (is_ul) {
    switch (lcid) {
      case CCCH_SIZE_48:
        return 6;
      case CCCH_SIZE_64:
        return 8;
      case CRNTI:
        return 2;
      case SHORT_BSR:
      case SHORT_TRUNC_BSR:
        return 1;
      case SE_PHR:
        return 2;
      case LONG_BSR:
      case LONG_TRUNC_BSR:
        return 1; // minimum size, could be more than that
      case PADDING:
        return 0;
    }
  } else {
    switch (lcid) {
      case CON_RES_ID:
        return 6;
      case TA_CMD:
        return 1;
      case DRX_CMD:
        return 0;
      case PADDING:
        return 0;
    }
  }
  return 0;
}

bool mac_sch_subpdu_nr::is_ul_ccch() const
{
  return (parent->is_ulsch() && (lcid == CCCH_SIZE_48 || lcid == CCCH_SIZE_64));
}

void mac_sch_subpdu_nr::to_string(fmt::memory_buffer& buffer)
{
  // print subPDU
  if (is_sdu()) {
    fmt::format_to(buffer, " LCID={} len={}", get_lcid(), get_sdu_length());
  } else {
    if (parent->is_ulsch()) {
      // UL-SCH case
      switch (get_lcid()) {
        case mac_sch_subpdu_nr::CCCH_SIZE_48:
          fmt::format_to(buffer, " CCCH48: len={}", get_sdu_length());
          break;
        case mac_sch_subpdu_nr::CCCH_SIZE_64:
          fmt::format_to(buffer, " CCCH64: len={}", get_sdu_length());
          break;
        case mac_sch_subpdu_nr::CRNTI:
          fmt::format_to(buffer, " C-RNTI: {:#04x}", get_c_rnti());
          break;
        case mac_sch_subpdu_nr::SHORT_TRUNC_BSR:
          fmt::format_to(buffer, " SHORT_TRUNC_BSR: len={}", get_total_length());
          break;
        case mac_sch_subpdu_nr::LONG_TRUNC_BSR:
          fmt::format_to(buffer, " LONG_TRUNC_BSR: len={}", get_total_length());
          break;
        case mac_sch_subpdu_nr::SHORT_BSR: {
          lcg_bsr_t sbsr = get_sbsr();
          fmt::format_to(buffer, " SBSR: lcg={} bs={}", sbsr.lcg_id, sbsr.buffer_size);
        } break;
        case mac_sch_subpdu_nr::LONG_BSR: {
          mac_sch_subpdu_nr::lbsr_t lbsr = get_lbsr();
          fmt::format_to(buffer, " LBSR: bitmap={:#02x}", lbsr.bitmap);
          for (const auto& lcg : lbsr.list) {
            fmt::format_to(buffer, " lcg={} bs={}", lcg.lcg_id, lcg.buffer_size);
          }
        } break;
        case mac_sch_subpdu_nr::SE_PHR:
          fmt::format_to(buffer, " SE_PHR: ph={} pc={}", get_phr(), get_pcmax());
          break;
        case mac_sch_subpdu_nr::PADDING:
          fmt::format_to(buffer, " PAD: len={}", get_sdu_length());
          break;
        default:
          fmt::format_to(buffer, " CE={} total_len={}", get_lcid(), get_total_length());
          break;
      }
    } else {
      // DL-SCH PDU
      switch (get_lcid()) {
        case mac_sch_subpdu_nr::TA_CMD:
          fmt::format_to(buffer, " TA: id={} command={}", get_ta().tag_id, get_ta().ta_command);
          break;
        case mac_sch_subpdu_nr::CON_RES_ID: {
          ue_con_res_id_t id = get_ue_con_res_id_ce();
          fmt::format_to(buffer,
                         " CON_RES: id={:x}{:x}{:x}{:x}{:x}{:x}",
                         id.at(0),
                         id.at(1),
                         id.at(2),
                         id.at(3),
                         id.at(4),
                         id.at(5));
        } break;
        case mac_sch_subpdu_nr::PADDING:
          fmt::format_to(buffer, " PAD: len={}", get_sdu_length());
          break;
        default:
          fmt::format_to(buffer, " CE={} total_len={}", get_lcid(), get_total_length());
          break;
      }
    }
  }
}

void mac_sch_pdu_nr::pack()
{
  // SDU and CEs are written in-place, only add padding if needed
  if (remaining_len) {
    mac_sch_subpdu_nr padding_subpdu(this);
    padding_subpdu.set_padding(remaining_len);
    padding_subpdu.write_subpdu(buffer->msg + buffer->N_bytes);

    // update length
    buffer->N_bytes += padding_subpdu.get_total_length();
    remaining_len -= padding_subpdu.get_total_length();
    subpdus.push_back(padding_subpdu);
  }
}

int mac_sch_pdu_nr::unpack(const uint8_t* payload, const uint32_t& len)
{
  uint32_t offset = 0;
  while (offset < len) {
    mac_sch_subpdu_nr sch_pdu(this);
    if (sch_pdu.read_subheader(payload + offset) == ISRRAN_ERROR) {
      logger.error("Malformed MAC PDU (len=%d, offset=%d)", len, offset);
      return ISRRAN_ERROR;
    }
    offset += sch_pdu.get_total_length();
    if (sch_pdu.get_lcid() == mac_sch_subpdu_nr::PADDING) {
      // set SDU length to rest of PDU
      sch_pdu.set_padding(len - offset + 1); // One byte for Padding header will be substracted again
      // skip remaining bytes
      offset = len;
    }
    subpdus.push_back(sch_pdu);
  }
  if (offset != len) {
    logger.error("Malformed MAC PDU (len=%d, offset=%d)", len, offset);
    return ISRRAN_ERROR;
  }

  return ISRRAN_SUCCESS;
}

const mac_sch_subpdu_nr& mac_sch_pdu_nr::get_subpdu(const uint32_t& index) const
{
  return subpdus.at(index);
}

mac_sch_subpdu_nr& mac_sch_pdu_nr::get_subpdu(uint32_t index)
{
  return subpdus.at(index);
}

bool mac_sch_pdu_nr::is_ulsch()
{
  return ulsch;
}

int mac_sch_pdu_nr::init_tx(byte_buffer_t* buffer_, uint32_t pdu_len_, bool ulsch_)
{
  if (buffer_ == nullptr || buffer_->msg == nullptr) {
    logger.error("Invalid buffer");
    return ISRRAN_ERROR;
  }
  buffer = buffer_;
  subpdus.clear();
  pdu_len       = pdu_len_;
  remaining_len = pdu_len_;
  ulsch         = ulsch_;
  return ISRRAN_SUCCESS;
}

void mac_sch_pdu_nr::init_rx(bool ulsch_)
{
  buffer = nullptr;
  subpdus.clear();
  pdu_len       = 0;
  remaining_len = 0;
  ulsch         = ulsch_;
}

uint32_t mac_sch_pdu_nr::size_header_sdu(const uint32_t lcid, const uint32_t nbytes)
{
  if (ulsch && (lcid == mac_sch_subpdu_nr::CCCH_SIZE_48 || lcid == mac_sch_subpdu_nr::CCCH_SIZE_64)) {
    return 1;
  }
  return nbytes < 256 ? 2 : 3;
}

uint32_t mac_sch_pdu_nr::get_remaing_len()
{
  return remaining_len;
}

uint32_t mac_sch_pdu_nr::add_sdu(const uint32_t lcid_, const uint8_t* payload_, const uint32_t len_)
{
  int header_size = size_header_sdu(lcid_, len_);
  if (header_size + len_ > remaining_len) {
    logger.error("Header and SDU exceed space in PDU (%d + %d > %d)", header_size, len_, remaining_len);
    return ISRRAN_ERROR;
  }

  mac_sch_subpdu_nr sch_pdu(this);
  sch_pdu.set_sdu(lcid_, payload_, len_);
  return add_sudpdu(sch_pdu);
}

uint32_t mac_sch_pdu_nr::add_crnti_ce(const uint16_t crnti)
{
  mac_sch_subpdu_nr ce(this);
  ce.set_c_rnti(crnti);
  return add_sudpdu(ce);
}

uint32_t mac_sch_pdu_nr::add_se_phr_ce(const uint8_t phr, const uint8_t pcmax)
{
  mac_sch_subpdu_nr ce(this);
  ce.set_se_phr(phr, pcmax);
  return add_sudpdu(ce);
}

uint32_t mac_sch_pdu_nr::add_sbsr_ce(const mac_sch_subpdu_nr::lcg_bsr_t bsr_)
{
  mac_sch_subpdu_nr ce(this);
  ce.set_sbsr(bsr_);
  return add_sudpdu(ce);
}

uint32_t
mac_sch_pdu_nr::add_lbsr_ce(const std::array<mac_sch_subpdu_nr::lcg_bsr_t, mac_sch_subpdu_nr::max_num_lcg_lbsr> bsr_)
{
  mac_sch_subpdu_nr ce(this);
  ce.set_lbsr(bsr_);
  return add_sudpdu(ce);
}

uint32_t mac_sch_pdu_nr::add_ue_con_res_id_ce(const mac_sch_subpdu_nr::ue_con_res_id_t id)
{
  mac_sch_subpdu_nr ce(this);
  ce.set_ue_con_res_id_ce(id);
  return add_sudpdu(ce);
}

uint32_t mac_sch_pdu_nr::add_sudpdu(mac_sch_subpdu_nr& subpdu)
{
  uint32_t subpdu_len = subpdu.get_total_length();
  if (subpdu_len > remaining_len) {
    logger.warning("Not enough space to add subPDU to PDU (%d > %d)", subpdu_len, remaining_len);
    return ISRRAN_ERROR;
  }

  // Write subPDU straigt into provided buffer
  subpdu.write_subpdu(buffer->msg + buffer->N_bytes);

  // adopt buffer variables
  buffer->N_bytes += subpdu_len;
  remaining_len -= subpdu_len;
  subpdus.push_back(subpdu);

  return ISRRAN_SUCCESS;
}

void mac_sch_pdu_nr::to_string(fmt::memory_buffer& fmtbuffer)
{
  fmt::format_to(fmtbuffer, "{}", is_ulsch() ? "UL" : "DL");
  for (auto& subpdu : subpdus) {
    subpdu.to_string(fmtbuffer);
  }
}

} // namespace isrran
