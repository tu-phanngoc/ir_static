
/// @file
/// @brief Support for Samsung protocols.

#include "ir_Samsung.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "app_ac_status.h"
#include "IR_Common.h"


bool g_samsungIsPowerTriggerred = false;

union SamsungProtocol _SamsungProtocol = { .raw = { 0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
                                                    0x01, 0x02, 0xAE, 0x71, 0x00, 0x15, 0xF0,
                                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

bool _Samsung_forceextended = false;  ///< Flag to know when we need to send an extended mesg.
bool _Samsung_lastsentpowerstate = false;
bool _Samsung_OnTimerEnable = false;
bool _Samsung_OffTimerEnable = false;
bool _Samsung_Sleep = false;
bool _Samsung_lastSleep = false;

uint16_t _Samsung_OnTimer = 0;
uint16_t _Samsung_OffTimer = 0;
uint16_t _Samsung_lastOnTimer = 0;
uint16_t _Samsung_lastOffTimer = 0;

// Constants
const uint16_t kSamsungTick             = 560;
const uint16_t kSamsungHdrMarkTicks     = 8;
const uint16_t kSamsungHdrMark          = 4480;
const uint16_t kSamsungHdrSpaceTicks    = 8;
const uint16_t kSamsungHdrSpace         = 4480;
const uint16_t kSamsungBitMarkTicks     = 1;
const uint16_t kSamsungBitMark          = 560;
const uint16_t kSamsungOneSpaceTicks    = 3;
const uint16_t kSamsungOneSpace         = 1680;
const uint16_t kSamsungZeroSpaceTicks   = 1;
const uint16_t kSamsungZeroSpace        = 560;
const uint16_t kSamsungRptSpaceTicks    = 4;
const uint16_t kSamsungRptSpace         = 2240;
const uint16_t kSamsungMinMessageLengthTicks    = 193;
const uint32_t kSamsungMinMessageLength         = 10808;
const uint16_t kSamsungMinGapTicks =
    kSamsungMinMessageLengthTicks -
    (kSamsungHdrMarkTicks + kSamsungHdrSpaceTicks +
     kSamsungBits * (kSamsungBitMarkTicks + kSamsungOneSpaceTicks) +
     kSamsungBitMarkTicks);
const uint32_t kSamsungMinGap = kSamsungMinGapTicks * kSamsungTick;

const uint16_t kSamsungAcHdrMark        = 580;
const uint16_t kSamsungAcHdrSpace       = 17844;
const uint8_t  kSamsungAcSections       = 2;
const uint16_t kSamsungAcSectionMark    = 3086;
const uint16_t kSamsungAcSectionSpace   = 8864;
const uint16_t kSamsungAcSectionGap     = 2886;
const uint16_t kSamsungAcBitMark        = 586;
const uint16_t kSamsungAcOneSpace       = 1432;
const uint16_t kSamsungAcZeroSpace      = 436;

// Values calculated based on the average of ten messages.
const uint16_t kSamsung36HdrMark        = 4515; /// < uSeconds
const uint16_t kSamsung36HdrSpace       = 4438; /// < uSeconds
const uint16_t kSamsung36BitMark        = 512;  /// < uSeconds
const uint16_t kSamsung36OneSpace       = 1468; /// < uSeconds
const uint16_t kSamsung36ZeroSpace      = 490;  /// < uSeconds

// _SamsungProtocol.Swing
const uint8_t kSamsungAcSwingV =        2; // 0b010
const uint8_t kSamsungAcSwingH =        3; // 0b011
const uint8_t kSamsungAcSwingBoth =     4; // 0b100
const uint8_t kSamsungAcSwingOff =      7; // 0b111
// _SamsungProtocol.FanSpecial
const uint8_t kSamsungAcFanSpecialOff = 0; // 0b000
const uint8_t kSamsungAcPowerfulOn =    3; // 0b011
const uint8_t kSamsungAcBreezeOn =      5; // 0b101
const uint8_t kSamsungAcEconoOn =       7; // 0b111





/* Get values from BLE command then fill to IR protocol */
void encode_Samsung(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){
  if(_Samsung_lastsentpowerstate != ac_status.power_status){
    g_samsungIsPowerTriggerred = true;
    if(ac_status.power_status)
      Samsung_sendOn(0, OutputIRProtocol);
    else
      Samsung_sendOff(0, OutputIRProtocol);

  }
  else{
    g_samsungIsPowerTriggerred = false;
    Samsung_setPower(ac_status.power_status);
    Samsung_setTemp(ac_status.temperature);
    Samsung_setMode(Samsung_convertMode(ac_status.mode));
    Samsung_setFan(Samsung_convertFan(ac_status.fan));
    Samsung_setSwing(ac_status.swing);
    Samsung_send(Samsung_getRaw(), kSamsungAcStateLength, kNoRepeat, OutputIRProtocol);
  }

  setIrTxState(1);
}


void decode_Samsung(int16_t* input, uint8_t* output) {
  // normal sigal
  initDecodeData(input, SAMSUNG_BITS);
  if( Samsung_recv(&gDecodeResult, 0, kSamsungBits, true) ){
    Samsung_setRaw(gDecodeResult.state, kSamsungAcStateLength);
  }
  else{
    // power on/off signal
    initDecodeData(input, SAMSUNG_EXT_BITS);
    if( Samsung_recv(&gDecodeResult, 0, kSamsungAcExtendedBits, true) ){
      Samsung_setRaw(gDecodeResult.state, kSamsungAcExtendedStateLength);
    }
    else{
      return; // if false, do nothing
    }
  }

  output[0] = Samsung_getPower();
  output[1] = Samsung_getTemp();
  output[2] = Samsung_toCommonFanSpeed(Samsung_getFan());
  output[3] = Samsung_getSwing();
  output[4] = Samsung_toCommonMode(Samsung_getMode());

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3]);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}



/// Send a Samsung A/C message.
/// Status: Stable / Known working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Samsung_send(const uint8_t data[], const uint16_t nbytes,
                  const uint16_t repeat, int16_t *irRaw) {
  if (nbytes < kSamsungAcStateLength && nbytes % kSamsungAcSectionLength)
    return;  // Not an appropriate number of bytes to send a proper message.

  uint16_t offset = 0;

  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    irRaw[offset++] = kSamsungAcHdrMark;
    irRaw[offset++] = -kSamsungAcHdrSpace;
    // Send in 7 byte sections.
    for (uint16_t i = 0; i < nbytes; i += kSamsungAcSectionLength) {
      sendGeneric_8(kSamsungAcSectionMark, kSamsungAcSectionSpace,
                    kSamsung36BitMark,     kSamsung36OneSpace,
                    kSamsung36BitMark,     kSamsung36ZeroSpace,
                    kSamsung36BitMark,     kSamsungAcSectionGap,
                    data + i, kSamsungAcSectionLength,  // 7 bytes == 56 bits
                    38000, false, 0, 50, &irRaw[offset]);                    // Send in LSBF order
      offset += 116; // 116 = + 2bit sections + 7bytes_data*2 + 2gap
    }
    // // Complete made up guess at inter-message gap.
    // space(kDefaultMessageGap - kSamsungAcSectionGap);
  }
}


/// Decode the supplied Samsung A/C message.
/// Status: Stable / Known to be working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool Samsung_recv(decode_results *results, uint16_t offset,
                  const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kHeader * 3 + kFooter * 2 - 1 + offset)
    return false;  // Can't possibly be a valid Samsung A/C message.
  if (nbits != kSamsungAcBits && nbits != kSamsungAcExtendedBits) return false;

  // Message Header
  if (!matchMark( results->rawbuf[offset++], kSamsungAcBitMark , kTolerance)) return false;
  if (!matchSpace(results->rawbuf[offset++], kSamsungAcHdrSpace, kTolerance)) return false;
  // Section(s)
  for (uint16_t pos = 0; pos <= (nbits / 8) - kSamsungAcSectionLength;
       pos += kSamsungAcSectionLength) {
    uint16_t used;
    // Section Header + Section Data (7 bytes) + Section Footer
    used = matchGeneric_8(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, kSamsungAcSectionLength * 8,
                        kSamsungAcSectionMark, kSamsungAcSectionSpace,
                        kSamsungAcBitMark, kSamsungAcOneSpace,
                        kSamsungAcBitMark, kSamsungAcZeroSpace,
                        kSamsungAcBitMark, kSamsungAcSectionGap,
                        pos + kSamsungAcSectionLength >= nbits / 8,
                        kTolerance, 0, false);
    if (used == 0) return false;
    offset += used;
  }
  // Compliance
  if (strict) {
    // Is the checksum valid?
    if (!Samsung_validChecksum(results->state, nbits / 8)) {
      return false;
    }
  }
  // Success
  results->decode_type = SAMSUNG_AC;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


bool isSamsung(int16_t *irRaw){
  // normal sigal
  initDecodeData(irRaw, SAMSUNG_BITS);
  if( Samsung_recv(&gDecodeResult, 0, kSamsungBits, true) ){
    return true;
  }
  else{
    // power on/off signal
    initDecodeData(irRaw, SAMSUNG_EXT_BITS);
    if( Samsung_recv(&gDecodeResult, 0, kSamsungAcExtendedBits, true) ){
      return true;
    }
  }
  return false;
}

#if SEND_SAMSUNG
/// Send a 32-bit Samsung formatted message.
/// Status: STABLE / Should be working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
/// @note Samsung has a separate message to indicate a repeat, like NEC does.
/// @todo Confirm that is actually how Samsung sends a repeat.
///   The refdoc doesn't indicate it is true.
void IRsend::sendSAMSUNG(const uint64_t data, const uint16_t nbits,
                         const uint16_t repeat) {
  sendGeneric(kSamsungHdrMark, kSamsungHdrSpace, kSamsungBitMark,
              kSamsungOneSpace, kSamsungBitMark, kSamsungZeroSpace,
              kSamsungBitMark, kSamsungMinGap, kSamsungMinMessageLength, data,
              nbits, 38, true, repeat, 33);
}

/// Construct a raw Samsung message from the supplied customer(address) &
/// command.
/// Status: STABLE / Should be working.
/// @param[in] customer The customer code. (aka. Address)
/// @param[in] command The command code.
/// @return A raw 32-bit Samsung message suitable for `sendSAMSUNG()`.
uint32_t IRsend::encodeSAMSUNG(const uint8_t customer, const uint8_t command) {
  uint8_t revcustomer = reverseBits(customer, sizeof(customer) * 8);
  uint8_t revcommand = reverseBits(command, sizeof(command) * 8);
  return ((revcommand ^ 0xFF) | (revcommand << 8) | (revcustomer << 16) |
          (revcustomer << 24));
}
#endif

#if DECODE_SAMSUNG
/// Decode the supplied Samsung 32-bit message.
/// Status: STABLE
/// @note Samsung messages whilst 32 bits in size, only contain 16 bits of
///   distinct data. e.g. In transmition order:
///   customer_byte + customer_byte(same) + address_byte + invert(address_byte)
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @note LG 32bit protocol appears near identical to the Samsung protocol.
///   They differ on their compliance criteria and how they repeat.
/// @see http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
bool IRrecv::decodeSAMSUNG(decode_results *results, uint16_t offset,
                           const uint16_t nbits, const bool strict) {
  if (strict && nbits != kSamsungBits)
    return false;  // We expect Samsung to be 32 bits of message.

  uint64_t data = 0;

  // Match Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kSamsungHdrMark, kSamsungHdrSpace,
                    kSamsungBitMark, kSamsungOneSpace,
                    kSamsungBitMark, kSamsungZeroSpace,
                    kSamsungBitMark, kSamsungMinGap, true)) return false;
  // Compliance
  // According to the spec, the customer (address) code is the first 8
  // transmitted bits. It's then repeated. Check for that.
  uint8_t address = data >> 24;
  if (strict && address != ((data >> 16) & 0xFF)) return false;
  // Spec says the command code is the 3rd block of transmitted 8-bits,
  // followed by the inverted command code.
  uint8_t command = (data & 0xFF00) >> 8;
  if (strict && command != ((data & 0xFF) ^ 0xFF)) return false;

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = SAMSUNG;
  // command & address need to be reversed as they are transmitted LSB first,
  results->command = reverseBits(command, sizeof(command) * 8);
  results->address = reverseBits(address, sizeof(address) * 8);
  return true;
}
#endif

#if SEND_SAMSUNG36
/// Send a Samsung 36-bit formatted message.
/// Status: STABLE / Works on real devices.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/621
void IRsend::sendSamsung36(const uint64_t data, const uint16_t nbits,
                           const uint16_t repeat) {
  if (nbits < 16) return;  // To small to send.
  for (uint16_t r = 0; r <= repeat; r++) {
    // Block #1 (16 bits)
    sendGeneric(kSamsung36HdrMark, kSamsung36HdrSpace,
                kSamsung36BitMark, kSamsung36OneSpace,
                kSamsung36BitMark, kSamsung36ZeroSpace,
                kSamsung36BitMark, kSamsung36HdrSpace,
                data >> (nbits - 16), 16, 38, true, 0, kDutyDefault);
    // Block #2 (The rest, typically 20 bits)
    sendGeneric(0, 0,  // No header
                kSamsung36BitMark, kSamsung36OneSpace,
                kSamsung36BitMark, kSamsung36ZeroSpace,
                kSamsung36BitMark, kSamsungMinGap,  // Gap is just a guess.
                // Mask off the rest of the bits.
                data & ((1ULL << (nbits - 16)) - 1),
                nbits - 16, 38, true, 0, kDutyDefault);
  }
}
#endif  // SEND_SAMSUNG36

#if DECODE_SAMSUNG36
/// Decode the supplied Samsung36 message.
/// Status: STABLE / Expected to work.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/621
bool IRrecv::decodeSamsung36(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + kFooter * 2 - 1 + offset)
    return false;  // Can't possibly be a valid Samsung message.
  // We need to be looking for > 16 bits to make sense.
  if (nbits <= 16) return false;
  if (strict && nbits != kSamsung36Bits)
    return false;  // We expect nbits to be 36 bits of message.

  uint64_t data = 0;

  // Match Header + Data + Footer
  uint16_t used;
  used = matchGeneric(results->rawbuf + offset, &data,
                      results->rawlen - offset, 16,
                      kSamsung36HdrMark, kSamsung36HdrSpace,
                      kSamsung36BitMark, kSamsung36OneSpace,
                      kSamsung36BitMark, kSamsung36ZeroSpace,
                      kSamsung36BitMark, kSamsung36HdrSpace, false);
  if (!used) return false;
  offset += used;
  // Data (Block #2)
  uint64_t data2 = 0;
  if (!matchGeneric(results->rawbuf + offset, &data2,
                    results->rawlen - offset, nbits - 16,
                    0, 0,
                    kSamsung36BitMark, kSamsung36OneSpace,
                    kSamsung36BitMark, kSamsung36ZeroSpace,
                    kSamsung36BitMark, kSamsungMinGap, true)) return false;
  data <<= (nbits - 16);
  data += data2;

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = SAMSUNG36;
  results->command = data & ((1ULL << (nbits - 16)) - 1);
  results->address = data >> (nbits - 16);
  return true;
}
#endif  // DECODE_SAMSUNG36

/// Class constructor

/// Get the existing checksum for a given state section.
/// @param[in] section The array to extract the checksum from.
/// @return The existing checksum value.
uint8_t Samsung_getSectionChecksum(const uint8_t *section) {
  return ((GETBITS8(*(section + 2), kLowNibble, kNibbleSize) << kNibbleSize) +
          GETBITS8(*(section + 1), kHighNibble, kNibbleSize));
}

/// Calculate the checksum for a given state section.
/// @param[in] section The array to calc the checksum of.
/// @return The calculated checksum value.
uint8_t Samsung_calcSectionChecksum(const uint8_t *section) {
  uint8_t sum = 0;

  sum += countBits_64(*section, 8, true, 0);  // Include the entire first byte
  // The lower half of the second byte.
  sum += countBits_64(GETBITS8(*(section + 1), kLowNibble, kNibbleSize), 8, true, 0);
  // The upper half of the third byte.
  sum += countBits_64(GETBITS8(*(section + 2), kHighNibble, kNibbleSize), 8, true, 0);
  // The next 4 bytes.
  sum += countBits_8(section + 3, 4, true, 0);
  // Bitwise invert the result.
  return sum ^ UINT8_MAX;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Samsung_validChecksum(const uint8_t state[], const uint16_t length) {
  bool result = true;
  const uint16_t MAXlength =
      (length > kSamsungAcExtendedStateLength) ? kSamsungAcExtendedStateLength
                                               : length;
  for (uint16_t offset = 0;
       offset + kSamsungAcSectionLength <= MAXlength;
       offset += kSamsungAcSectionLength)
    result &= (Samsung_getSectionChecksum(state + offset) ==
               Samsung_calcSectionChecksum(state + offset));
  return result;
}

/// Update the checksum for the internal state.
void Samsung_checksum(void) {
  uint8_t sectionsum = Samsung_calcSectionChecksum(_SamsungProtocol.raw);
  _SamsungProtocol.Sum1Upper = GETBITS8(sectionsum, kHighNibble, kNibbleSize);
  _SamsungProtocol.Sum1Lower = GETBITS8(sectionsum, kLowNibble, kNibbleSize);
  sectionsum = Samsung_calcSectionChecksum(_SamsungProtocol.raw + kSamsungAcSectionLength);
  _SamsungProtocol.Sum2Upper = GETBITS8(sectionsum, kHighNibble, kNibbleSize);
  _SamsungProtocol.Sum2Lower = GETBITS8(sectionsum, kLowNibble, kNibbleSize);
  sectionsum = Samsung_calcSectionChecksum(_SamsungProtocol.raw + kSamsungAcSectionLength * 2);
  _SamsungProtocol.Sum3Upper = GETBITS8(sectionsum, kHighNibble, kNibbleSize);
  _SamsungProtocol.Sum3Lower = GETBITS8(sectionsum, kLowNibble, kNibbleSize);
}

#if SEND_SAMSUNG_AC
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
/// @note Use for most function/mode/settings changes to the unit.
///   i.e. When the device is already running.
void Samsung_send(const uint16_t repeat) {
  // Do we need to send a special (extended) message?
  if (getPower() != _lastsentpowerstate || _forceextended ||
      (_lastOnTimer != _OnTimer) || (_lastOffTimer != _OffTimer) ||
      (_Sleep != _lastSleep))  // We do.
    sendExtended(repeat);
  else  // No, it's just a normal message.
    _irsend.sendSamsungAC(getRaw(), kSamsungAcStateLength, repeat);
}
#endif  // SEND_SAMSUNG_AC

/// Send the extended current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
/// @note Samsung A/C requires an extended length message when you want to
/// change the power operating mode, Timers, or Sleep setting of the A/C unit.
void Samsung_sendExtended(const uint16_t repeat, int16_t *irRaw) {
  _Samsung_lastsentpowerstate = Samsung_getPower();  // Remember the last power state sent.
  _Samsung_lastOnTimer = _Samsung_OnTimer;
  _Samsung_lastOffTimer = _Samsung_OffTimer;
  static const uint8_t extended_middle_section[kSamsungAcSectionLength] = {
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00};
  // Copy/convert the internal state to an extended state by
  // copying the second section to the third section, and inserting the extended
  // middle (second) section.
  memcpy(_SamsungProtocol.raw + 2 * kSamsungAcSectionLength,
              _SamsungProtocol.raw + kSamsungAcSectionLength,
              kSamsungAcSectionLength);
  memcpy(_SamsungProtocol.raw + kSamsungAcSectionLength, extended_middle_section,
              kSamsungAcSectionLength);
  _Samsung_setOnTimer();
  _Samsung_setSleepTimer();  // This also sets any Off Timer if needed too.
  // Send it.
  Samsung_send(Samsung_getRaw(), kSamsungAcExtendedStateLength, repeat, irRaw);
  // Now revert it by copying the third section over the second section.
  memcpy(_SamsungProtocol.raw + kSamsungAcSectionLength,
              _SamsungProtocol.raw + 2 * kSamsungAcSectionLength,
              kSamsungAcSectionLength);

  _Samsung_forceextended = false;  // It has now been sent, so clear the flag if set.
}

/// Send the special extended "On" message as the library can't seem to
/// reproduce this message automatically.
/// @param[in] repeat Nr. of times the message will be repeated.
void Samsung_sendOn(const uint16_t repeat, int16_t *irRaw) {
  const uint8_t extended_state[kSamsungAcExtendedStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0xE2, 0xFE, 0x71, 0x80, 0x11, 0xF0};
  Samsung_send(extended_state, kSamsungAcExtendedStateLength, repeat, irRaw);
  _Samsung_lastsentpowerstate = true;  // On
}

/// Send the special extended "Off" message as the library can't seem to
/// reproduce this message automatically.
/// @param[in] repeat Nr. of times the message will be repeated.
void Samsung_sendOff(const uint16_t repeat, int16_t *irRaw) {
  const uint8_t extended_state[kSamsungAcExtendedStateLength] = {
      0x02, 0xB2, 0x0F, 0x00, 0x00, 0x00, 0xC0,
      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,
      0x01, 0x02, 0xFF, 0x71, 0x80, 0x11, 0xC0};
  Samsung_send(extended_state, kSamsungAcExtendedStateLength, repeat, irRaw);
  _Samsung_lastsentpowerstate = false;  // Off
}


/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Samsung_getRaw(void) {
  Samsung_checksum();
  return _SamsungProtocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] length The length/size of the new_code array.
void Samsung_setRaw(const uint8_t new_code[], const uint16_t length) {
  memcpy(_SamsungProtocol.raw, new_code, MIN(length, kSamsungAcExtendedStateLength));
  // Shrink the extended state into a normal state.
  if (length > kSamsungAcStateLength) {
    _Samsung_OnTimerEnable  = _SamsungProtocol.OnTimerEnable;
    _Samsung_OffTimerEnable = _SamsungProtocol.OffTimerEnable;
    _Samsung_Sleep          = _SamsungProtocol.Sleep5 && _SamsungProtocol.Sleep12;
    _Samsung_OnTimer        = _Samsung_getOnTimer();
    _Samsung_OffTimer       = _Samsung_getOffTimer();
    for (uint8_t i = kSamsungAcStateLength; i < length; i++)
      _SamsungProtocol.raw[i - kSamsungAcSectionLength] = _SamsungProtocol.raw[i];
  }
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setPower(const bool on) {
  _SamsungProtocol.Power1 = _SamsungProtocol.Power2 = (on ? kSamsungAcPowerOn : kSamsungAcPowerOff);
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getPower(void) {
  return ((_SamsungProtocol.Power1 == kSamsungAcPowerOn) && 
          (_SamsungProtocol.Power2 == kSamsungAcPowerOn));
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Samsung_setTemp(const uint8_t temp) {
  uint8_t newtemp = MAX(kSamsungAcMinTemp, temp);
  newtemp = MIN(kSamsungAcMaxTemp, newtemp);
  _SamsungProtocol.Temp = newtemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Samsung_getTemp(void) {
  return _SamsungProtocol.Temp;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Samsung_setMode(const uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  uint8_t newmode = mode;
  if (newmode > kSamsungAcHeat) newmode = kSamsungAcAuto;
  _SamsungProtocol.Mode = newmode;

  // Auto mode has a special fan setting valid only in auto mode.
  if (newmode == kSamsungAcAuto) {
    _SamsungProtocol.Fan = kSamsungAcFanAuto2;
  } else {
    // Non-Auto can't have this fan setting
    if (_SamsungProtocol.Fan == kSamsungAcFanAuto2)
      _SamsungProtocol.Fan = kSamsungAcFanAuto;  // Default to something safe.
  }
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Samsung_getMode(void) {
  return _SamsungProtocol.Mode;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Samsung_setFan(const uint8_t speed) {
  switch (speed) {
    case kSamsungAcFanAuto:
    case kSamsungAcFanLow:
    case kSamsungAcFanMed:
    case kSamsungAcFanHigh:
    case kSamsungAcFanTurbo:
      if (_SamsungProtocol.Mode == kSamsungAcAuto) return;  // Not valid in Auto mode.
      break;
    case kSamsungAcFanAuto2:  // Special fan setting for when in Auto mode.
      if (_SamsungProtocol.Mode != kSamsungAcAuto) return;
      break;
    default:
      return;
  }
  _SamsungProtocol.Fan = speed;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Samsung_getFan(void) {
  return _SamsungProtocol.Fan;
}

/// Get the vertical swing setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getSwing(void) {
  switch (_SamsungProtocol.Swing) {
    case kSamsungAcSwingV:
    case kSamsungAcSwingBoth: return true;
    default:                  return false;
  }
}

/// Set the vertical swing setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setSwing(const bool on) {
  switch (_SamsungProtocol.Swing) {
    case kSamsungAcSwingBoth:
    case kSamsungAcSwingH:
      _SamsungProtocol.Swing = on ? kSamsungAcSwingBoth : kSamsungAcSwingH;
      break;
    default:
      _SamsungProtocol.Swing = on ? kSamsungAcSwingV : kSamsungAcSwingOff;
  }
}

/// Get the horizontal swing setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getSwingH(void) {
  switch (_SamsungProtocol.Swing) {
    case kSamsungAcSwingH:
    case kSamsungAcSwingBoth: return true;
    default:                  return false;
  }
}

/// Set the horizontal swing setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setSwingH(const bool on) {
  switch (_SamsungProtocol.Swing) {
    case kSamsungAcSwingV:
    case kSamsungAcSwingBoth:
      _SamsungProtocol.Swing = on ? kSamsungAcSwingBoth : kSamsungAcSwingV;
      break;
    default:
      _SamsungProtocol.Swing = on ? kSamsungAcSwingH : kSamsungAcSwingOff;
  }
}

/// Get the Beep toggle setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getBeep(void) { return _SamsungProtocol.BeepToggle; }

/// Set the Beep toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setBeep(const bool on) { _SamsungProtocol.BeepToggle = on; }

/// Get the Clean toggle setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getClean(void) {
  return _SamsungProtocol.CleanToggle10 && _SamsungProtocol.CleanToggle11;
}

/// Set the Clean toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setClean(const bool on) {
  _SamsungProtocol.CleanToggle10 = on;
  _SamsungProtocol.CleanToggle11 = on;
}

/// Get the Quiet setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getQuiet(void) { return _SamsungProtocol.Quiet; }

/// Set the Quiet setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setQuiet(const bool on) {
  _SamsungProtocol.Quiet = on;
  if (on) {
    // Quiet mode seems to set fan speed to auto.
    Samsung_setFan(kSamsungAcFanAuto);
    Samsung_setPowerful(false);  // Quiet 'on' is mutually exclusive to Powerful.
  }
}

/// Get the Powerful (Turbo) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getPowerful(void) {
  return (_SamsungProtocol.FanSpecial == kSamsungAcPowerfulOn) &&
         (_SamsungProtocol.Fan == kSamsungAcFanTurbo);
}

/// Set the Powerful (Turbo) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setPowerful(const bool on) {
  uint8_t off_value = (Samsung_getBreeze() || Samsung_getEcono()) ? _SamsungProtocol.FanSpecial
                                                  : kSamsungAcFanSpecialOff;
  _SamsungProtocol.FanSpecial = (on ? kSamsungAcPowerfulOn : off_value);
  if (on) {
    // Powerful mode sets fan speed to Turbo.
    Samsung_setFan(kSamsungAcFanTurbo);
    Samsung_setQuiet(false);  // Powerful 'on' is mutually exclusive to Quiet.
  }
}

/// Are the vanes closed over the fan outlet, to stop direct wind? Aka. WindFree
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getBreeze(void) {
  return (_SamsungProtocol.FanSpecial == kSamsungAcBreezeOn) &&
         (_SamsungProtocol.Fan == kSamsungAcFanAuto && !Samsung_getSwing());
}

/// Closes the vanes over the fan outlet, to stop direct wind. Aka. WindFree
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setBreeze(const bool on) {
  const uint8_t off_value = (Samsung_getPowerful() ||
                             Samsung_getEcono()) ? _SamsungProtocol.FanSpecial
                                         : kSamsungAcFanSpecialOff;
  _SamsungProtocol.FanSpecial = (on ? kSamsungAcBreezeOn : off_value);
  if (on) {
    Samsung_setFan(kSamsungAcFanAuto);
    Samsung_setSwing(false);
  }
}

/// Get the current Economy (Eco) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getEcono(void) {
  return (_SamsungProtocol.FanSpecial == kSamsungAcEconoOn) &&
         (_SamsungProtocol.Fan == kSamsungAcFanAuto && Samsung_getSwing());
}

/// Set the current Economy (Eco) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setEcono(const bool on) {
  const uint8_t off_value = (Samsung_getBreeze() ||
                             Samsung_getPowerful()) ? _SamsungProtocol.FanSpecial
                                            : kSamsungAcFanSpecialOff;
  _SamsungProtocol.FanSpecial = (on ? kSamsungAcEconoOn : off_value);
  if (on) {
    Samsung_setFan(kSamsungAcFanAuto);
    Samsung_setSwing(true);
  }
}

/// Get the Display (Light/LED) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getDisplay(void) { return _SamsungProtocol.Display; }

/// Set the Display (Light/LED) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setDisplay(const bool on) { _SamsungProtocol.Display = on; }

/// Get the Ion (Filter) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Samsung_getIon(void) { return _SamsungProtocol.Ion; }

/// Set the Ion (Filter) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Samsung_setIon(const bool on) { _SamsungProtocol.Ion = on; }

/// Get the On Timer setting of the A/C from a raw extended state.
/// @return The Nr. of MINutes the On Timer is set for.
uint16_t _Samsung_getOnTimer(void) {
  if (_SamsungProtocol.OnTimeDay) return 24 * 60;
  return (_SamsungProtocol.OnTimeHrs2 * 2 + _SamsungProtocol.OnTimeHrs1) * 60 + _SamsungProtocol.OnTimeMins * 10;
}

/// Set the current On Timer value of the A/C into the raw extended state.
void _Samsung_setOnTimer(void) {
  _SamsungProtocol.OnTimerEnable = _Samsung_OnTimerEnable = (_Samsung_OnTimer > 0);
  _SamsungProtocol.OnTimeDay = (_Samsung_OnTimer >= 24 * 60);
  if (_SamsungProtocol.OnTimeDay) {
    _SamsungProtocol.OnTimeHrs2 = _SamsungProtocol.OnTimeHrs1 = _SamsungProtocol.OnTimeMins = 0;
    return;
  }
  _SamsungProtocol.OnTimeMins = (_Samsung_OnTimer % 60) / 10;
  const uint8_t hours = _Samsung_OnTimer / 60;
  _SamsungProtocol.OnTimeHrs1 = hours & 1;
  _SamsungProtocol.OnTimeHrs2 = hours >> 1;
}

/// Get the Off Timer setting of the A/C from a raw extended state.
/// @return The Nr. of MINutes the Off Timer is set for.
uint16_t _Samsung_getOffTimer(void) {
  if (_SamsungProtocol.OffTimeDay) return 24 * 60;
  return (_SamsungProtocol.OffTimeHrs2 * 2 + _SamsungProtocol.OffTimeHrs1) * 60 + _SamsungProtocol.OffTimeMins * 10;
}

/// Set the current Off Timer value of the A/C into the raw extended state.
void _Samsung_setOffTimer(void) {
  _SamsungProtocol.OffTimerEnable = _Samsung_OffTimerEnable = (_Samsung_OffTimer > 0);
  _SamsungProtocol.OffTimeDay = (_Samsung_OffTimer >= 24 * 60);
  if (_SamsungProtocol.OffTimeDay) {
    _SamsungProtocol.OffTimeHrs2 = _SamsungProtocol.OffTimeHrs1 = _SamsungProtocol.OffTimeMins = 0;
    return;
  }
  _SamsungProtocol.OffTimeMins = (_Samsung_OffTimer % 60) / 10;
  const uint8_t hours = _Samsung_OffTimer / 60;
  _SamsungProtocol.OffTimeHrs1 = hours & 1;
  _SamsungProtocol.OffTimeHrs2 = hours >> 1;
}

// Set the current Sleep Timer value of the A/C into the raw extended state.
void _Samsung_setSleepTimer(void) {
  _Samsung_setOffTimer();
  // The Sleep mode/timer should only be engaged if an off time has been set.
  _SamsungProtocol.Sleep5 = _Samsung_Sleep && _Samsung_OffTimerEnable;
  _SamsungProtocol.Sleep12 = _SamsungProtocol.Sleep5;
}

/// Get the On Timer setting of the A/C.
/// @return The Nr. of MINutes the On Timer is set for.
uint16_t Samsung_getOnTimer(void) { return _Samsung_OnTimer; }

/// Get the Off Timer setting of the A/C.
/// @return The Nr. of MINutes the Off Timer is set for.
/// @note Sleep & Off Timer share the same timer.
uint16_t Samsung_getOffTimer(void) {
  return _Samsung_Sleep ? 0 : _Samsung_OffTimer;
}

/// Get the Sleep Timer setting of the A/C.
/// @return The Nr. of MINutes the Off Timer is set for.
/// @note Sleep & Off Timer share the same timer.
uint16_t Samsung_getSleepTimer(void) {
  return _Samsung_Sleep ? _Samsung_OffTimer : 0;
}

#define TIMER_RESOLUTION(MINs) \
    (((MIN((MINs), (uint16_t)(24 * 60))) / 10) * 10)

/// Set the On Timer value of the A/C.
/// @param[in] nr_of_MINs The number of MINutes the timer should be.
/// @note The timer time only has a resolution of 10 MINs.
/// @note Setting the On Timer active will cancel the Sleep timer/setting.
void Samsung_setOnTimer(const uint16_t nr_of_MINs) {
  // Limit to one day, and round down to nearest 10 MIN increment.
  _Samsung_OnTimer = TIMER_RESOLUTION(nr_of_MINs);
  _Samsung_OnTimerEnable = _Samsung_OnTimer > 0;
  if (_Samsung_OnTimer) _Samsung_Sleep = false;
}

/// Set the Off Timer value of the A/C.
/// @param[in] nr_of_MINs The number of MINutes the timer should be.
/// @note The timer time only has a resolution of 10 MINs.
/// @note Setting the Off Timer active will cancel the Sleep timer/setting.
void Samsung_setOffTimer(const uint16_t nr_of_MINs) {
  // Limit to one day, and round down to nearest 10 MIN increment.
  _Samsung_OffTimer = TIMER_RESOLUTION(nr_of_MINs);
  _Samsung_OffTimerEnable = _Samsung_OffTimer > 0;
  if (_Samsung_OffTimer) _Samsung_Sleep = false;
}

/// Set the Sleep Timer value of the A/C.
/// @param[in] nr_of_MINs The number of MINutes the timer should be.
/// @note The timer time only has a resolution of 10 MINs.
/// @note Sleep timer acts as an Off timer, and cancels any On Timer.
void Samsung_setSleepTimer(const uint16_t nr_of_MINs) {
  // Limit to one day, and round down to nearest 10 MIN increment.
  _Samsung_OffTimer = TIMER_RESOLUTION(nr_of_MINs);
  if (_Samsung_OffTimer) Samsung_setOnTimer(0);  // Clear the on timer if set.
  _Samsung_Sleep = _Samsung_OffTimer > 0;
  _Samsung_OffTimerEnable = _Samsung_Sleep;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Samsung_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kSamsungAcCool;
    case kOpModeHeat: return kSamsungAcHeat;
    case kOpModeDry:  return kSamsungAcDry;
    case kOpModeFan:  return kSamsungAcFan;
    default:          return kSamsungAcAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Samsung_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kSamsungAcFanLow;
    case kFanSpeedLow:    return kSamsungAcFanMed;
    case kFanSpeedMedium: return kSamsungAcFanHigh;
    case kFanSpeedHigh:
    case kFanSpeedMax:    return kSamsungAcFanTurbo;
    case kFanSpeedAuto:
    default:              return kSamsungAcFanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Samsung_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kSamsungAcCool: return kOpModeCool;
    case kSamsungAcHeat: return kOpModeHeat;
    case kSamsungAcDry:  return kOpModeDry;
    case kSamsungAcFan:  return kOpModeFan;
    default:             return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Samsung_toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kSamsungAcFanAuto:  return kFanSpeedAuto;
    case kSamsungAcFanAuto2:
    case kSamsungAcFanLow:   return kFanSpeedMin;
    case kSamsungAcFanMed:   return kFanSpeedLow;
    case kSamsungAcFanHigh:  return kFanSpeedMedium;
    case kSamsungAcFanTurbo: return kFanSpeedHigh;
    default:                 return kFanSpeedAuto;
  }
}

/// Convert the current internal state into its state_t equivalent.
/// @return The stdAc equivalent of the native settings.
state_t Samsung_toCommon(void) {
  state_t result = {0};
  result.protocol = SAMSUNG_AC;
  result.model = -1;  // Not supported.
  result.power = Samsung_getPower();
  result.mode = Samsung_toCommonMode(_SamsungProtocol.Mode);
  result.celsius = true;
  result.degrees = Samsung_getTemp();
  result.fanspeed = Samsung_toCommonFanSpeed(_SamsungProtocol.Fan);
  result.swingv = Samsung_getSwing() ? kSwingVAuto : kSwingVOff;
  result.swingh = Samsung_getSwingH() ? kSwingHAuto : kSwingHOff;
  result.quiet = Samsung_getQuiet();
  result.turbo = Samsung_getPowerful();
  result.econo = Samsung_getEcono();
  result.clean = Samsung_getClean();
  result.beep = _SamsungProtocol.BeepToggle;
  result.light = _SamsungProtocol.Display;
  result.filter = _SamsungProtocol.Ion;
  result.sleep = _Samsung_Sleep ? Samsung_getSleepTimer() : -1;
  // Not supported.
  result.clock = -1;
  return result;
}




