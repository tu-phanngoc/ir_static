
/// @file
/// @brief Support for Daikin A/C protocols.

#include <string.h>


#include "ir_Daikin.h"
#include "IRcommon.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "IR_DeviceConstructor.h"
#include "app_ac_status.h"
#include "IR_Common.h"


DaikinESPProtocol_t _DaikinESPProtocol = { .raw = {0x11, 0xDA, 0x27, 0x00, 0xC5, 0x00, 0x00, 0xD7, 0x11, 0xDA, 0x27, 0x00, 0x42, 0x00, 0x00, 0x54, 0x11, 0xDA, 0x27, 0x00, 0x00, 0x49, 0x1E, 0x00, 0xB0, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xC8} };
Daikin2Protocol_t   _Daikin2Protocol   = { .raw = {0x11, 0xDA, 0x27, 0x00, 0x01, 0x00, 0xC0, 0x70, 0x08, 0x0C, 0x80, 0x04, 0xB0, 0x16, 0x24, 0x00, 0x00, 0xBE, 0xD0, 0x53, 0x11, 0xDA, 0x27, 0x00, 0x00, 0x08, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0xC1, 0x80, 0x60, 0xC1} };
Daikin216Protocol_t _Daikin216Protocol = { .raw = {0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x11, 0xDA, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00} };
Daikin160Protocol_t _Daikin160Protocol = { .raw = {0x11, 0xDA, 0x27, 0xF0, 0x0D, 0x00, 0x00, 0x11, 0xDA, 0x27, 0x00, 0xD3, 0x30, 0x11, 0x00, 0x00, 0x1E, 0x0A, 0x08, 0x00} };

Daikin176Protocol_t _Daikin176Protocol = { .raw = {0x11, 0xDA, 0x17, 0x18, 0x04, 0x00, 0x00, 0x11, 0xDA, 0x17, 0x18, 0x00, 0x73, 0x00, 0x20, 0x00, 0x00, 0x00, 0x16, 0x00, 0x20, 0x00} };
uint8_t _Daikin176_savedTemp;  ///< The previously user requested temp value.

Daikin128Protocol_t _Daikin128Protocol = { .raw = {0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };
Daikin152Protocol_t _Daikin152Protocol = { .raw = {0x11, 0xDA, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC5, 0x00, 0x00, 0x00} };
Daikin64Protocol_t  _Daikin64Protocol  = { .raw = kDaikin64KnownGoodState };
bool _Daikin64_savePower = false;

decode_type_t _savedType = UNKNOWN;




//################################################################




void encode_DaikinESP(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){
  DaikinESP_setPower(ac_status.power_status);
  DaikinESP_setTemp(ac_status.temperature);
  DaikinESP_setMode(DaikinESP_convertMode(ac_status.mode));
  DaikinESP_setFan(DaikinESP_convertFan(ac_status.fan));
  DaikinESP_send(DaikinESP_getRaw(), kDaikinStateLength, kDaikinDefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}

void decode_DaikinESP(int16_t* input, uint8_t* output) {

  initDecodeData(input, DAIKIN_BITS);
  if(DaikinESP_recv(&gDecodeResult, 0, kDaikinBits, false))
    DaikinESP_setRaw(gDecodeResult.state, kDaikinStateLength);
  else
    return;
  output[0] = DaikinESP_getPower();
  output[1] = DaikinESP_getTemp();
  output[2] = DaikinESP_toCommonFanSpeed(DaikinESP_getFan());
  output[3] = DaikinESP_getSwingVertical();
  output[4] = DaikinESP_toCommonMode(DaikinESP_getMode());
  
  setAcStatus(output);
  ac_control_update_status_to_payload();
}


/// Send a Daikin 280-bit A/C formatted message.
/// Status: STABLE
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void DaikinESP_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw) {
  uint16_t index = 0;

  if (nbytes < kDaikinStateLengthShort)
    return;  // Not enough bytes to send a proper message.

  for (uint16_t r = 0; r <= repeat; r++) {
    uint16_t offset = 0;
    // Send the header, 0b00000
    sendGeneric_64( 0, 0,  // No header for the header
                    kDaikinBitMark, kDaikinOneSpace,
                    kDaikinBitMark, kDaikinZeroSpace,
                    kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                    0, (uint64_t)0,
                    kDaikinHeaderLength, 38, false, 0, 50,
                    &irRaw[index]);
    index += (2 + kDaikinHeaderLength * 2);
    // Data #1
    if (nbytes < kDaikinStateLength) {  // Are we using the legacy size?
      // Do this as a constant to save RAM and keep in flash memory
      sendGeneric_64(kDaikinHdrMark, kDaikinHdrSpace,
                  kDaikinBitMark, kDaikinOneSpace,
                  kDaikinBitMark, kDaikinZeroSpace,
                  kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                  0, kDaikinFirstHeader64,
                  64, 38, false, 0, 50, 
                  &irRaw[index]);
      index += (4 + 64 * 2);
    } else {  // We are using the newer/more correct size.
      sendGeneric_8( kDaikinHdrMark, kDaikinHdrSpace,
                      kDaikinBitMark, kDaikinOneSpace,
                      kDaikinBitMark, kDaikinZeroSpace,
                      kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                      data, kDaikinSection1Length, 38, false, 0, 50,
                      &irRaw[index]);
      index += (4 + kDaikinSection1Length * 8 * 2);
      offset += kDaikinSection1Length;
    }
    // Data #2
    sendGeneric_8(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
                kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
                kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                data + offset, kDaikinSection2Length, 38, false, 0, 50,
                &irRaw[index]);
    index += (4 + kDaikinSection2Length * 8 * 2);
    offset += kDaikinSection2Length;
    // Data #3
    sendGeneric_8(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
                kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
                kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                data + offset, nbytes - offset, 38, false, 0, 50,
                &irRaw[index]);
    index += (4 + (nbytes - offset) * 8 * 2);
  }
}



/// Decode the supplied Daikin 280-bit message. (DAIKIN)
/// Status: STABLE / Reported as working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool DaikinESP_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  // Is there enough data to match successfully?
  if (results->rawlen < (2 * (nbits + kDaikinHeaderLength) +
                         kDaikinSections * (kHeader + kFooter) + kFooter - 1) +
                         offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikinBits) return false;

  match_result_t data_result;

  // Header #1 - Doesn't count as data.
  data_result = matchData(&(results->rawbuf[offset]), kDaikinHeaderLength,
                          kDaikinBitMark, kDaikinOneSpace,
                          kDaikinBitMark, kDaikinZeroSpace,
                          kTolerance, kMarkExcess,
                          false, true);
  offset += data_result.used;
  if (data_result.success == false) return false;  // Fail
  if (data_result.data) return false;  // The header bits should be zero.
  // Footer
  if (!matchMark(results->rawbuf[offset++], kDaikinBitMark, kTolerance)) return false;
  if (!matchSpace(results->rawbuf[offset++], kDaikinZeroSpace + kDaikinGap, kTolerance)) return false;
  // Sections
  const uint8_t ksectionSize[kDaikinSections] = {
      kDaikinSection1Length, kDaikinSection2Length, kDaikinSection3Length};
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikinSections; section++) {
    uint16_t used;
    // Section Header + Section Data (7 bytes) + Section Footer
    used = matchGeneric_8(results->rawbuf + offset, results->state + pos,
                          results->rawlen - offset, ksectionSize[section] * 8,
                          kDaikinHdrMark, kDaikinHdrSpace,
                          kDaikinBitMark, kDaikinOneSpace,
                          kDaikinBitMark, kDaikinZeroSpace,
                          kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                          section >= kDaikinSections - 1,
                          kTolerance, kMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    if (pos * 8 != kDaikinBits) return false; /* Kiem tra lai logic, Tu Phan */
    // Validate the checksum.
    if (!DaikinESP_validChecksum(results->state, kDaikinStateLength)) return false;
  }

  // Success
  results->decode_type = DAIKIN;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}

bool DaikinESP_isHeaderMatch(int16_t *rawbuf){

  match_result_t data_result;
  

  initDecodeData(rawbuf, DAIKIN_BITS);

  data_result = matchData(&(gDecodeResult.rawbuf[0]), kDaikinHeaderLength,
                          kDaikinBitMark, kDaikinOneSpace,
                          kDaikinBitMark, kDaikinZeroSpace,
                          kTolerance, kMarkExcess,
                          false, true);
  if (data_result.success == false)
    return false;  // Fail
  else 
    return true;
}



/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool DaikinESP_validChecksum(uint8_t state[], const uint16_t length) {
  // Data #1
  if (length < kDaikinSection1Length || state[kDaikinByteChecksum1] != sumBytes(state, kDaikinSection1Length - 1, 0))
    return false;
  // Data #2
  if (length < kDaikinSection1Length + kDaikinSection2Length ||
      state[kDaikinByteChecksum2] != sumBytes(state + kDaikinSection1Length, kDaikinSection2Length - 1, 0))
    return false;
  // Data #3
  if (length < kDaikinSection1Length + kDaikinSection2Length + 2 ||
      state[length - 1] != sumBytes(state + kDaikinSection1Length + kDaikinSection2Length,
                                    length - (kDaikinSection1Length + kDaikinSection2Length) - 1, 0))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void DaikinESP_checksum(void) {
  _DaikinESPProtocol.Sum1 = sumBytes(_DaikinESPProtocol.raw, kDaikinSection1Length - 1, 0);
  _DaikinESPProtocol.Sum2 = sumBytes(_DaikinESPProtocol.raw + kDaikinSection1Length, kDaikinSection2Length - 1, 0);
  _DaikinESPProtocol.Sum3 = sumBytes(_DaikinESPProtocol.raw + kDaikinSection1Length + kDaikinSection2Length, kDaikinSection3Length - 1, 0);
}

/// Reset the internal state to a fixed known good state.
void DaikinESP_stateReset(void) {
  for (uint8_t i = 0; i < kDaikinStateLength; i++) _DaikinESPProtocol.raw[i] = 0x0;

  _DaikinESPProtocol.raw[0] = 0x11;
  _DaikinESPProtocol.raw[1] = 0xDA;
  _DaikinESPProtocol.raw[2] = 0x27;
  _DaikinESPProtocol.raw[4] = 0xC5;
  // _DaikinESPProtocol.raw[7] is a checksum byte, it will be set by checksum().
  _DaikinESPProtocol.raw[8] = 0x11;
  _DaikinESPProtocol.raw[9] = 0xDA;
  _DaikinESPProtocol.raw[10] = 0x27;
  _DaikinESPProtocol.raw[12] = 0x42;
  // _DaikinESPProtocol.raw[15] is a checksum byte, it will be set by checksum().
  _DaikinESPProtocol.raw[16] = 0x11;
  _DaikinESPProtocol.raw[17] = 0xDA;
  _DaikinESPProtocol.raw[18] = 0x27;
  _DaikinESPProtocol.raw[21] = 0x49;
  _DaikinESPProtocol.raw[22] = 0x1E;
  _DaikinESPProtocol.raw[24] = 0xB0;
  _DaikinESPProtocol.raw[27] = 0x06;
  _DaikinESPProtocol.raw[28] = 0x60;
  _DaikinESPProtocol.raw[31] = 0xC0;
  // _DaikinESPProtocol.raw[34] is a checksum byte, it will be set by checksum().
  DaikinESP_checksum();
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *DaikinESP_getRaw(void) {
  DaikinESP_checksum();  // Ensure correct settings before sending.
  return _DaikinESPProtocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] length Length of the code in bytes.
void DaikinESP_setRaw(const uint8_t new_code[], const uint16_t length) {
  uint8_t offset = 0;
  if (length == kDaikinStateLengthShort) {  // Handle the "short" length case.
    offset = kDaikinStateLength - kDaikinStateLengthShort; /* Kiem tra lai logic, offset luon bang -8, Tu Phan */
    DaikinESP_stateReset();
  }
  for (uint8_t i = 0; i < length && i < kDaikinStateLength; i++)
    _DaikinESPProtocol.raw[i + offset] = new_code[i];
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setPower(const bool on) {
  _DaikinESPProtocol.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getPower(void) {
  return _DaikinESPProtocol.Power;
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void DaikinESP_setTemp(const uint8_t temp) {
  uint8_t degrees = MAX(temp + 16, kDaikinMinTemp + 6);
  degrees = MIN(degrees, kDaikinMaxTemp);
  _DaikinESPProtocol.Temp = degrees;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t DaikinESP_getTemp(void) { 
  if(_DaikinESPProtocol.Temp < 16)
    return 0;
  return _DaikinESPProtocol.Temp - 16; 
}

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void DaikinESP_setFan(const uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
    fanset = fan;
  else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
    fanset = kDaikinFanAuto;
  else
    fanset = 2 + fan;
  _DaikinESPProtocol.Fan = fanset;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t DaikinESP_getFan(void) {
  uint8_t fan = _DaikinESPProtocol.Fan;
  if (fan != kDaikinFanQuiet && fan != kDaikinFanAuto) fan -= 2;
  return fan;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t DaikinESP_getMode(void) {
  return _DaikinESPProtocol.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void DaikinESP_setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikinAuto:
    case kDaikinCool:
    case kDaikinHeat:
    case kDaikinFan:
    case kDaikinDry:
      _DaikinESPProtocol.Mode = mode;
      break;
    default:
      _DaikinESPProtocol.Mode = kDaikinAuto;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setSwingVertical(const bool on) {
  _DaikinESPProtocol.SwingV = (on ? kDaikinSwingOn : kDaikinSwingOff);
}

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getSwingVertical(void) {
  return _DaikinESPProtocol.SwingV;
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setSwingHorizontal(const bool on) {
  _DaikinESPProtocol.SwingH = (on ? kDaikinSwingOn : kDaikinSwingOff);
}

/// Get the Horizontal Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getSwingHorizontal(void) {
  return _DaikinESPProtocol.SwingH;
}

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setQuiet(const bool on) {
  _DaikinESPProtocol.Quiet = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) DaikinESP_setPowerful(false);
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getQuiet(void) {
  return _DaikinESPProtocol.Quiet;
}

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setPowerful(const bool on) {
  _DaikinESPProtocol.Powerful = on;
  if (on) {
    // Powerful, Quiet, & Econo mode being on are mutually exclusive.
    DaikinESP_setQuiet(false);
    DaikinESP_setEcono(false);
  }
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getPowerful(void) {
  return _DaikinESPProtocol.Powerful;
}

/// Set the Sensor mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setSensor(const bool on) {
  _DaikinESPProtocol.Sensor = on;
}

/// Get the Sensor mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getSensor(void) {
  return _DaikinESPProtocol.Sensor;
}

/// Set the Economy mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setEcono(const bool on) {
  _DaikinESPProtocol.Econo = on;
  // Powerful & Econo mode being on are mutually exclusive.
  if (on) DaikinESP_setPowerful(false);
}

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getEcono(void) {
  return _DaikinESPProtocol.Econo;
}

/// Set the Mould mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setMold(const bool on) {
  _DaikinESPProtocol.Mold = on;
}

/// Get the Mould mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getMold(void) {
  return _DaikinESPProtocol.Mold;
}

/// Set the Comfort mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setComfort(const bool on) {
  _DaikinESPProtocol.Comfort = on;
}

/// Get the Comfort mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getComfort(void) {
  return _DaikinESPProtocol.Comfort;
}


#if 0 //temporary disable timer

/// Set the enable status & time of the On Timer.
/// @param[in] starttime The number of minutes past midnight.
void DaikinESP_enableOnTimer(const uint16_t starttime) {
  _DaikinESPProtocol.OnTimer = true;
  _DaikinESPProtocol.OnTime = starttime;
}

/// Clear and disable the On timer.
void DaikinESP_disableOnTimer(void) {
  _DaikinESPProtocol.OnTimer = false;
  _DaikinESPProtocol.OnTime = kDaikinUnusedTime;
}

/// Get the On Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t DaikinESP_getOnTime(void) {
  return _DaikinESPProtocol.OnTime;
}

/// Get the enable status of the On Timer.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getOnTimerEnabled(void) {
  return _DaikinESPProtocol.OnTimer;
}

/// Set the enable status & time of the Off Timer.
/// @param[in] endtime The number of minutes past midnight.
void DaikinESP_enableOffTimer(const uint16_t endtime) {
  _DaikinESPProtocol.OffTimer = true;
  _DaikinESPProtocol.OffTime = endtime;
}

/// Clear and disable the Off timer.
void DaikinESP_disableOffTimer(void) {
  _DaikinESPProtocol.OffTimer = false;
  _DaikinESPProtocol.OffTime = kDaikinUnusedTime;
}

/// Get the Off Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t DaikinESP_getOffTime(void) {
  return _DaikinESPProtocol.OffTime;
}

/// Get the enable status of the Off Timer.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getOffTimerEnabled(void) {
  return _DaikinESPProtocol.OffTimer;
}

/// Set the clock on the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void DaikinESP_setCurrentTime(const uint16_t mins_since_midnight) {
  uint16_t mins = mins_since_midnight;
  if (mins > 24 * 60) mins = 0;  // If > 23:59, set to 00:00
  _DaikinESPProtocol.CurrentTime = mins;
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t DaikinESP_getCurrentTime(void) {
  return _DaikinESPProtocol.CurrentTime;
}

/// Set the current day of the week to be sent to the A/C unit.
/// @param[in] day_of_week The numerical representation of the day of the week.
/// @note 1 is SUN, 2 is MON, ..., 7 is SAT
void DaikinESP_setCurrentDay(const uint8_t day_of_week) {
  _DaikinESPProtocol.CurrentDay = day_of_week;
}

/// Get the current day of the week to be sent to the A/C unit.
/// @return The numerical representation of the day of the week.
/// @note 1 is SUN, 2 is MON, ..., 7 is SAT
uint8_t DaikinESP_getCurrentDay(void) {
  return _DaikinESPProtocol.CurrentDay;
}

/// Set the enable status of the Weekly Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void DaikinESP_setWeeklyTimerEnable(const bool on) {
  // Bit is cleared for `on`.
  _DaikinESPProtocol.WeeklyTimer = !on;
}

/// Get the enable status of the Weekly Timer.
/// @return true, the setting is on. false, the setting is off.
bool DaikinESP_getWeeklyTimerEnable(void) {
  return !_DaikinESPProtocol.WeeklyTimer;
}

#endif // temporary disable timer


/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t DaikinESP_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kDaikinCool;
    case kOpModeHeat: return kDaikinHeat;
    case kOpModeDry:  return kDaikinDry;
    case kOpModeFan:  return kDaikinFan;
    default:          return kDaikinAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t DaikinESP_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kDaikinFanQuiet;
    case kFanSpeedLow:    return kDaikinFanMin;
    case kFanSpeedMedium: return kDaikinFanMed;
    case kFanSpeedHigh:   return kDaikinFanMax - 1;
    case kFanSpeedMax:    return kDaikinFanMax;
    default:              return kDaikinFanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t DaikinESP_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kDaikinCool: return kOpModeCool;
    case kDaikinHeat: return kOpModeHeat;
    case kDaikinDry:  return kOpModeDry;
    case kDaikinFan:  return kOpModeFan;
    default:          return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t DaikinESP_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kDaikinFanMax:     return kFanSpeedMax;
    case kDaikinFanMax - 1: return kFanSpeedHigh;
    case kDaikinFanMed:
    case kDaikinFanMin + 1: return kFanSpeedMedium;
    case kDaikinFanMin:     return kFanSpeedLow;
    case kDaikinFanQuiet:   return kFanSpeedMin;
    default:                return kFanSpeedAuto;
  }
}


//#########################################################

void encode_Daikin2(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){
  Daikin2_setPower(ac_status.power_status);
  Daikin2_setTemp(ac_status.temperature);
  Daikin2_setMode(Daikin2_convertMode(ac_status.mode));
  Daikin2_setFan(Daikin2_convertFan(ac_status.fan));
  Daikin2_send(Daikin2_getRaw(), kDaikin2StateLength, kDaikin2DefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}

void decode_Daikin2(int16_t* input, uint8_t* output) {

  initDecodeData(input, DAIKIN2_BITS);
  if(Daikin2_recv(&gDecodeResult, 0, kDaikin2Bits, true))
    Daikin2_setRaw(gDecodeResult.state);
  else
    return;
  output[0] = Daikin2_getPower();
  output[1] = Daikin2_getTemp();
  output[2] = DaikinESP_toCommonFanSpeed(Daikin2_getFan()); // using same as DaikinESP
  output[3] = Daikin2_getSwingVertical();
  output[4] = DaikinESP_toCommonMode(Daikin2_getMode());

  setAcStatus(output);
  ac_control_update_status_to_payload();
}
/// Send a Daikin2 (312-bit) A/C formatted message.
/// Status: STABLE / Expected to work.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Daikin2_send(const unsigned char data[], const uint16_t nbytes,
                         const uint16_t repeat, int16_t *irRaw) {
  uint16_t offset = 0;

  if (nbytes < kDaikin2Section1Length)
    return;  // Not enough bytes to send a partial message.

  for (uint16_t r = 0; r <= repeat; r++) {
    // Leader
    sendGeneric_64(kDaikin2LeaderMark, kDaikin2LeaderSpace,
                    0, 0, 0, 0, 0, 0, 
                    0, (uint64_t) 0,  // No data payload.
                    0, kDaikin2Freq, false, 0, 50, 
                    &irRaw[offset]);
    offset += 2;
    // Section #1
    sendGeneric_8( kDaikin2HdrMark, kDaikin2HdrSpace,
                    kDaikin2BitMark, kDaikin2OneSpace,
                    kDaikin2BitMark, kDaikin2ZeroSpace,
                    kDaikin2BitMark, kDaikin2Gap,
                    data, kDaikin2Section1Length,
                    kDaikin2Freq, false, 0, 50,
                    &irRaw[offset]);
    offset += ( 4 + kDaikin2Section1Length * 8 * 2 );
    // Section #2
    sendGeneric_8( kDaikin2HdrMark, kDaikin2HdrSpace, 
                    kDaikin2BitMark, kDaikin2OneSpace,
                    kDaikin2BitMark, kDaikin2ZeroSpace,
                    kDaikin2BitMark, kDaikin2Gap,
                    data + kDaikin2Section1Length, nbytes - kDaikin2Section1Length,
                    kDaikin2Freq, false, 0, 50,
                    &irRaw[offset]);
    offset += ( 4 + (nbytes - kDaikin2Section1Length) * 8 * 2 );
  }
}



/// Decode the supplied Daikin 312-bit message. (DAIKIN2)
/// Status: STABLE / Works as expected.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Daikin2_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) + kHeader - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin2Bits) return false;

  const uint8_t ksectionSize[kDaikin2Sections] = {kDaikin2Section1Length,
                                                  kDaikin2Section2Length};

  // Leader
  if (!matchMark(results->rawbuf[offset++], kDaikin2LeaderMark, kTolerance + kDaikin2Tolerance)) return false;
  if (!matchSpace(results->rawbuf[offset++], kDaikin2LeaderSpace, kTolerance + kDaikin2Tolerance)) return false;

  // Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin2Sections; section++) {
    uint16_t used;
    // Section Header + Section Data + Section Footer
    used = matchGeneric_8(results->rawbuf + offset, results->state + pos,
                          results->rawlen - offset, ksectionSize[section] * 8,
                          kDaikin2HdrMark, kDaikin2HdrSpace,
                          kDaikin2BitMark, kDaikin2OneSpace,
                          kDaikin2BitMark, kDaikin2ZeroSpace,
                          kDaikin2BitMark, kDaikin2Gap,
                          section >= kDaikin2Sections - 1,
                          kTolerance + kDaikin2Tolerance, kMarkExcess,
                          false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    if (pos * 8 != kDaikin2Bits) return false;
    // Validate the checksum.
    if (!Daikin2_validChecksum(results->state, kDaikin2StateLength)) return false;
  }

  // Success
  results->decode_type = DAIKIN2;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Daikin2_validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of section #1.
  if (length <= kDaikin2Section1Length - 1 || state[kDaikin2Section1Length - 1] != sumBytes(state, kDaikin2Section1Length - 1, 0))
    return false;
  // Validate the checksum of section #2 (a.k.a. the rest)
  if (length <= kDaikin2Section1Length + 1 || state[length - 1] != sumBytes(state + kDaikin2Section1Length, length - kDaikin2Section1Length - 1, 0))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void Daikin2_checksum(void) {
  _Daikin2Protocol.Sum1 = sumBytes(_Daikin2Protocol.raw, kDaikin2Section1Length - 1, 0);
  _Daikin2Protocol.Sum2 = sumBytes(_Daikin2Protocol.raw + kDaikin2Section1Length, kDaikin2Section2Length - 1, 0);
}

/// Reset the internal state to a fixed known good state.
void Daikin2_stateReset(void) {
  for (uint8_t i = 0; i < kDaikin2StateLength; i++) _Daikin2Protocol.raw[i] = 0x0;

  _Daikin2Protocol.raw[0] = 0x11;
  _Daikin2Protocol.raw[1] = 0xDA;
  _Daikin2Protocol.raw[2] = 0x27;
  _Daikin2Protocol.raw[4] = 0x01;
  _Daikin2Protocol.raw[6] = 0xC0;
  _Daikin2Protocol.raw[7] = 0x70;
  _Daikin2Protocol.raw[8] = 0x08;
  _Daikin2Protocol.raw[9] = 0x0C;
  _Daikin2Protocol.raw[10] = 0x80;
  _Daikin2Protocol.raw[11] = 0x04;
  _Daikin2Protocol.raw[12] = 0xB0;
  _Daikin2Protocol.raw[13] = 0x16;
  _Daikin2Protocol.raw[14] = 0x24;
  _Daikin2Protocol.raw[17] = 0xBE;
  _Daikin2Protocol.raw[18] = 0xD0;
  // _Daikin2Protocol.raw[19] is a checksum byte, it will be set by checksum().
  _Daikin2Protocol.raw[20] = 0x11;
  _Daikin2Protocol.raw[21] = 0xDA;
  _Daikin2Protocol.raw[22] = 0x27;
  _Daikin2Protocol.raw[25] = 0x08;
  _Daikin2Protocol.raw[28] = 0xA0;
  _Daikin2Protocol.raw[35] = 0xC1;
  _Daikin2Protocol.raw[36] = 0x80;
  _Daikin2Protocol.raw[37] = 0x60;
  // _Daikin2Protocol.raw[38] is a checksum byte, it will be set by checksum().
  // Daikin2_disableOnTimer();
  // Daikin2_disableOffTimer();
  // Daikin2_disableSleepTimer();
  // Daikin2_checksum();
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Daikin2_getRaw(void) {
  Daikin2_checksum();  // Ensure correct settings before sending.
  return _Daikin2Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void Daikin2_setRaw(const uint8_t new_code[]) {
  memcpy(_Daikin2Protocol.raw, new_code, kDaikin2StateLength);
}


/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setPower(const bool on) {
  _Daikin2Protocol.Power = on;
  _Daikin2Protocol.Power2 = !on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getPower(void) { return _Daikin2Protocol.Power && !_Daikin2Protocol.Power2; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Daikin2_getMode(void) { return _Daikin2Protocol.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] desired_mode The desired operating mode.
void Daikin2_setMode(const uint8_t desired_mode) {
  uint8_t mode = desired_mode;
  switch (mode) {
    case kDaikinCool:
    case kDaikinHeat:
    case kDaikinFan:
    case kDaikinDry: break;
    default: mode = kDaikinAuto;
  }
  _Daikin2Protocol.Mode = mode;
  // Redo the temp setting as Cool mode has a different min temp.
  if (mode == kDaikinCool) Daikin2_setTemp(Daikin2_getTemp());
  Daikin2_setHumidity(Daikin2_getHumidity());  // Make sure the humidity is okay for this mode.
}

/// Set the temperature.
/// @param[in] desired The temperature in degrees celsius.
void Daikin2_setTemp(const uint8_t desired) {
  // The A/C has a different min temp if in cool mode.
  uint8_t temp = MAX( (_Daikin2Protocol.Mode == kDaikinCool) ? kDaikin2MinCoolTemp : kDaikinMinTemp, desired + kDaikin2MinCoolTemp);
  _Daikin2Protocol.Temp = MIN(kDaikinMaxTemp, temp);
  // If the humidity setting is in use, the temp is a fixed value.
  if (_Daikin2Protocol.HumidOn) _Daikin2Protocol.Temp = kDaikinMaxTemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Daikin2_getTemp(void) { 
  if(_Daikin2Protocol.Temp < kDaikin2MinCoolTemp)
    return kDaikinMinTemp;
  if(_Daikin2Protocol.HumidOn) 
    return kDaikinMaxTemp;

  return _Daikin2Protocol.Temp - kDaikin2MinCoolTemp; 
}

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void Daikin2_setFan(const uint8_t fan) {
  uint8_t fanset;
  if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
    fanset = fan;
  else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
    fanset = kDaikinFanAuto;
  else
    fanset = 2 + fan;
  _Daikin2Protocol.Fan = fanset;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Daikin2_getFan(void) {
  const uint8_t fan = _Daikin2Protocol.Fan;
  switch (fan) {
    case kDaikinFanAuto:
    case kDaikinFanQuiet: return fan;
    default: return fan - 2;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void Daikin2_setSwingVertical(const uint8_t position) {
  switch (position) {
    case kDaikin2SwingVHighest:
    case kDaikin2SwingVHigh:
    case kDaikin2SwingVUpperMiddle:
    case kDaikin2SwingVLowerMiddle:
    case kDaikin2SwingVLow:
    case kDaikin2SwingVLowest:
    case kDaikin2SwingVOff:
    case kDaikin2SwingVBreeze:
    case kDaikin2SwingVCirculate:
    case kDaikin2SwingVAuto:
      _Daikin2Protocol.SwingV = position;
  }
}

/// Get the Vertical Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Daikin2_getSwingVertical(void) { return _Daikin2Protocol.SwingV; }

/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin2_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest:
    case kSwingVHigh:
    case kSwingVMiddle:
    case kSwingVLow:
    case kSwingVLowest: return (uint8_t)position + kDaikin2SwingVHighest;
    case kSwingVOff:    return kDaikin2SwingVOff;
    default:            return kDaikin2SwingVAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common vertical swing position.
swingv_t Daikin2_toCommonSwingV(const uint8_t setting) {
  switch (setting) {
    case kDaikin2SwingVHighest:     return kSwingVHighest;
    case kDaikin2SwingVHigh:        return kSwingVHigh;
    case kDaikin2SwingVUpperMiddle:
    case kDaikin2SwingVLowerMiddle: return kSwingVMiddle;
    case kDaikin2SwingVLow:         return kSwingVLow;
    case kDaikin2SwingVLowest:      return kSwingVLowest;
    case kDaikin2SwingVOff:         return kSwingVOff;
    default:                        return kSwingVAuto;
  }
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void Daikin2_setSwingHorizontal(const uint8_t position) {
  _Daikin2Protocol.SwingH = position;
}

/// Get the Horizontal Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Daikin2_getSwingHorizontal(void) { return _Daikin2Protocol.SwingH; }


#if 0 // DISABLE TIMER
/// Set the clock on the A/C unit.
/// @param[in] numMins Nr. of minutes past midnight.
void Daikin2_setCurrentTime(const uint16_t numMins) {
  uint16_t mins = numMins;
  if (numMins > 24 * 60) mins = 0;  // If > 23:59, set to 00:00
  _Daikin2Protocol.CurrentTime = mins;
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin2_getCurrentTime(void) { return _Daikin2Protocol.CurrentTime; }

/// Set the enable status & time of the On Timer.
/// @param[in] starttime The number of minutes past midnight.
/// @note Timer location is shared with sleep timer.
void Daikin2_enableOnTimer(const uint16_t starttime) {
  Daikin2_clearSleepTimerFlag();
  _Daikin2Protocol.OnTimer = true;
  _Daikin2Protocol.OnTime = starttime;
}

/// Clear the On Timer flag.
void Daikin2_clearOnTimerFlag(void) { _Daikin2Protocol.OnTimer = false; }

/// Disable the On timer.
void Daikin2_disableOnTimer(void) {
  _Daikin2Protocol.OnTime = kDaikinUnusedTime;
  Daikin2_clearOnTimerFlag();
  Daikin2_clearSleepTimerFlag();
}

/// Get the On Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin2_getOnTime(void) { return _Daikin2Protocol.OnTime; }

/// Get the enable status of the On Timer.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getOnTimerEnabled(void) { return _Daikin2Protocol.OnTimer; }

/// Set the enable status & time of the Off Timer.
/// @param[in] endtime The number of minutes past midnight.
void Daikin2_enableOffTimer(const uint16_t endtime) {
  // Set the Off Timer flag.
  _Daikin2Protocol.OffTimer = true;
  _Daikin2Protocol.OffTime = endtime;
}

/// Disable the Off timer.
void Daikin2_disableOffTimer(void) {
  _Daikin2Protocol.OffTime = kDaikinUnusedTime;
  // Clear the Off Timer flag.
  _Daikin2Protocol.OffTimer = false;
}

/// Get the Off Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin2_getOffTime(void) { return _Daikin2Protocol.OffTime; }

/// Get the enable status of the Off Timer.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getOffTimerEnabled(void) { return _Daikin2Protocol.OffTimer; }


/// Set the enable status & time of the Sleep Timer.
/// @param[in] sleeptime The number of minutes past midnight.
/// @note The Timer location is shared with On Timer.
void Daikin2_enableSleepTimer(const uint16_t sleeptime) {
  Daikin2_enableOnTimer(sleeptime);
  Daikin2_clearOnTimerFlag();
  _Daikin2Protocol.SleepTimer = true;
}

/// Clear the sleep timer flag.
void Daikin2_clearSleepTimerFlag(void) { _Daikin2Protocol.SleepTimer = false; }

/// Disable the sleep timer.
void Daikin2_disableSleepTimer(void) { Daikin2_disableOnTimer(); }

/// Get the Sleep Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin2_getSleepTime(void) { return Daikin2_getOnTime(); }

/// Get the Sleep timer enabled status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getSleepTimerEnabled(void) { return _Daikin2Protocol.SleepTimer; }


#endif // DISABLE TIMER


/// Get the Beep status of the A/C.
/// @return true, the setting is on. false, the setting is off.
uint8_t Daikin2_getBeep(void) { return _Daikin2Protocol.Beep; }

/// Set the Beep mode of the A/C.
/// @param[in] beep true, the setting is on. false, the setting is off.
void Daikin2_setBeep(const uint8_t beep) { _Daikin2Protocol.Beep = beep; }

/// Get the Light status of the A/C.
/// @return true, the setting is on. false, the setting is off.
uint8_t Daikin2_getLight(void) { return _Daikin2Protocol.Light; }

/// Set the Light (LED) mode of the A/C.
/// @param[in] light true, the setting is on. false, the setting is off.
void Daikin2_setLight(const uint8_t light) { _Daikin2Protocol.Light = light; }

/// Set the Mould (filter) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setMold(const bool on) { _Daikin2Protocol.Mold = on; }

/// Get the Mould (filter) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getMold(void) { return _Daikin2Protocol.Mold; }

/// Set the Auto clean mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setClean(const bool on) { _Daikin2Protocol.Clean = on; }

/// Get the Auto Clean mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getClean(void) { return _Daikin2Protocol.Clean; }

/// Set the Fresh Air mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setFreshAir(const bool on) { _Daikin2Protocol.FreshAir = on; }

/// Get the Fresh Air mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getFreshAir(void) { return _Daikin2Protocol.FreshAir; }

/// Set the (High) Fresh Air mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setFreshAirHigh(const bool on) { _Daikin2Protocol.FreshAirHigh = on; }

/// Get the (High) Fresh Air mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getFreshAirHigh(void) { return _Daikin2Protocol.FreshAirHigh; }

/// Set the Automatic Eye (Sensor) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setEyeAuto(bool on) { _Daikin2Protocol.EyeAuto = on; }

/// Get the Automaitc Eye (Sensor) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getEyeAuto(void) { return _Daikin2Protocol.EyeAuto; }

/// Set the Eye (Sensor) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setEye(bool on) { _Daikin2Protocol.Eye = on; }

/// Get the Eye (Sensor) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getEye(void) { return _Daikin2Protocol.Eye; }

/// Set the Economy mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setEcono(bool on) { _Daikin2Protocol.Econo = on; }

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getEcono(void) { return _Daikin2Protocol.Econo; }


/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setQuiet(const bool on) {
  _Daikin2Protocol.Quiet = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) Daikin2_setPowerful(false);
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getQuiet(void) { return _Daikin2Protocol.Quiet; }

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setPowerful(const bool on) {
  _Daikin2Protocol.Powerful = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) Daikin2_setQuiet(false);
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getPowerful(void) { return _Daikin2Protocol.Powerful; }

/// Set the Purify (Filter) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin2_setPurify(const bool on) { _Daikin2Protocol.Purify = on; }

/// Get the Purify (Filter) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin2_getPurify(void) { return _Daikin2Protocol.Purify; }

/// Get the Humidity percentage setting of the A/C.
/// @return The setting percentage. 255 is Automatic. 0 is Off.
uint8_t Daikin2_getHumidity(void) { return _Daikin2Protocol.Humidity; }

/// Set the Humidity percentage setting of the A/C.
/// @param[in] percent Percentage humidty. 255 is Auto. 0 is Off.
/// @note Only available in Dry & Heat modes, otherwise it is Off.
void Daikin2_setHumidity(const uint8_t percent) {
  _Daikin2Protocol.Humidity = kDaikin2HumidityOff;  // Default to off.
  switch (Daikin2_getMode()) {
    case kDaikinHeat:
      switch (percent) {
        case kDaikin2HumidityOff:
        case kDaikin2HumidityHeatLow:
        case kDaikin2HumidityHeatMedium:
        case kDaikin2HumidityHeatHigh:
        case kDaikin2HumidityAuto:
          _Daikin2Protocol.Humidity = percent;
      }
      break;
    case kDaikinDry:
      switch (percent) {
        case kDaikin2HumidityOff:
        case kDaikin2HumidityDryLow:
        case kDaikin2HumidityDryMedium:
        case kDaikin2HumidityDryHigh:
        case kDaikin2HumidityAuto:
          _Daikin2Protocol.Humidity = percent;
      }
      break;
  }
  _Daikin2Protocol.HumidOn = (_Daikin2Protocol.Humidity != kDaikin2HumidityOff);  // Enabled?
  Daikin2_setTemp(Daikin2_getTemp());  // Adjust the temperature if we need to.
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin2_convertMode(const opmode_t mode) {
  return DaikinESP_convertMode(mode);
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin2_convertFan(const fanspeed_t speed) {
  return DaikinESP_convertFan(speed);
}

/// Convert a swingh_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin2_convertSwingH(const swingh_t position) {
  switch (position) {
    case kSwingHAuto:     return kDaikin2SwingHSwing;
    case kSwingHLeftMax:  return kDaikin2SwingHLeftMax;
    case kSwingHLeft:     return kDaikin2SwingHLeft;
    case kSwingHMiddle:   return kDaikin2SwingHMiddle;
    case kSwingHRight:    return kDaikin2SwingHRight;
    case kSwingHRightMax: return kDaikin2SwingHRightMax;
    case kSwingHWide:     return kDaikin2SwingHWide;
    default:                         return kDaikin2SwingHAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common horizontal swing position.
swingh_t Daikin2_toCommonSwingH(const uint8_t setting) {
  switch (setting) {
    case kDaikin2SwingHSwing:    return kSwingHAuto;
    case kDaikin2SwingHLeftMax:  return kSwingHLeftMax;
    case kDaikin2SwingHLeft:     return kSwingHLeft;
    case kDaikin2SwingHMiddle:   return kSwingHMiddle;
    case kDaikin2SwingHRight:    return kSwingHRight;
    case kDaikin2SwingHRightMax: return kSwingHRightMax;
    case kDaikin2SwingHWide:     return kSwingHWide;
    default:                     return kSwingHOff;
  }
}

//###############################################################


void encode_Daikin216(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){
  Daikin216_setPower(ac_status.power_status);
  Daikin216_setTemp(ac_status.temperature);
  Daikin216_setMode(Daikin216_convertMode(ac_status.mode));
  Daikin216_setFan(Daikin216_convertFan(ac_status.fan));
  Daikin216_send(Daikin216_getRaw(), kDaikin216StateLength, kDaikin216DefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}
void decode_Daikin216(int16_t* input, uint8_t* output) {
  initDecodeData(input, DAIKIN216_BITS);
  if(Daikin216_recv(&gDecodeResult, 0, kDaikin216Bits, false))
    Daikin216_setRaw(gDecodeResult.state);
  else
    return;
  output[0] = Daikin216_getPower();
  output[1] = Daikin216_getTemp();
  output[2] = DaikinESP_toCommonFanSpeed(Daikin216_getFan()); // same as ESP
  output[3] = Daikin216_getSwingVertical();
  output[4] = DaikinESP_toCommonMode(Daikin216_getMode()); // same as ESP

  setAcStatus(output);
  ac_control_update_status_to_payload();
}
/// Send a Daikin216 (216-bit) A/C formatted message.
/// Status: Alpha / Untested on a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Daikin216_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw) {
  if (nbytes < kDaikin216Section1Length)
    return;  // Not enough bytes to send a partial message.

  uint16_t offset = 0;

  for (uint16_t r = 0; r <= repeat; r++) {
    // Section #1
    sendGeneric_8( kDaikin216HdrMark, kDaikin216HdrSpace, 
                    kDaikin216BitMark, kDaikin216OneSpace,
                    kDaikin216BitMark, kDaikin216ZeroSpace,
                    kDaikin216BitMark, kDaikin216Gap,
                    data, kDaikin216Section1Length,
                    kDaikin216Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += ( 4 + kDaikin216Section1Length * 8 * 2 );
    // Section #2
    sendGeneric_8( kDaikin216HdrMark, kDaikin216HdrSpace, 
                    kDaikin216BitMark, kDaikin216OneSpace,
                    kDaikin216BitMark, kDaikin216ZeroSpace,
                    kDaikin216BitMark, kDaikin216Gap,
                    data + kDaikin216Section1Length, nbytes - kDaikin216Section1Length,
                    kDaikin216Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (4 + (nbytes - kDaikin216Section1Length) * 8 * 2);
  }
}



/// Decode the supplied Daikin 216-bit message. (DAIKIN216)
/// Status: STABLE / Should be working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Daikin216_recv(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin216Bits) return false;

  const uint8_t ksectionSize[kDaikin216Sections] = {kDaikin216Section1Length,
                                                    kDaikin216Section2Length};
  // Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin216Sections; section++) {
    uint16_t used;
    // Section Header + Section Data + Section Footer
    used = matchGeneric_8(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, ksectionSize[section] * 8,
                        kDaikin216HdrMark, kDaikin216HdrSpace,
                        kDaikin216BitMark, kDaikin216OneSpace,
                        kDaikin216BitMark, kDaikin216ZeroSpace,
                        kDaikin216BitMark, kDaikin216Gap,
                        section >= kDaikin216Sections - 1,
                        kTolerance, kMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    if (pos * 8 != kDaikin216Bits) return false;
    // Validate the checksum.
    if (!Daikin216_validChecksum(results->state, kDaikin216StateLength)) return false;
  }

  // Success
  results->decode_type = DAIKIN216;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


bool Daikin216_isHeaderMatch(int16_t *rawbuf){

  initDecodeData(rawbuf, DAIKIN216_BITS);
  uint16_t used = 0;
  // Section Header + Section Data + Section Footer
  used = matchGeneric_8(gDecodeResult.rawbuf, gDecodeResult.state,
                        gDecodeResult.rawlen, kDaikin216Section1Length * 8,
                        kDaikin216HdrMark, kDaikin216HdrSpace,
                        kDaikin216BitMark, kDaikin216OneSpace,
                        kDaikin216BitMark, kDaikin216ZeroSpace,
                        kDaikin216BitMark, kDaikin216Gap,
                        true, kTolerance, kMarkExcess, false);
  if (used == 0)
    return false;
  else 
    return true;
}


/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Daikin216_validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of section #1.
  if (length <= kDaikin216Section1Length - 1 ||
      state[kDaikin216Section1Length - 1] != sumBytes(
          state, kDaikin216Section1Length - 1, 0))
    return false;
  // Validate the checksum of section #2 (a.k.a. the rest)
  if (length <= kDaikin216Section1Length + 1 ||
      state[length - 1] != sumBytes(state + kDaikin216Section1Length, length - kDaikin216Section1Length - 1, 0))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void Daikin216_checksum(void) {
  _Daikin216Protocol.Sum1 = sumBytes(_Daikin216Protocol.raw, kDaikin216Section1Length - 1, 0);
  _Daikin216Protocol.Sum2 = sumBytes(_Daikin216Protocol.raw + kDaikin216Section1Length, kDaikin216Section2Length - 1, 0);
}

/// Reset the internal state to a fixed known good state.
void Daikin216_stateReset(void) {
  for (uint8_t i = 0; i < kDaikin216StateLength; i++) _Daikin216Protocol.raw[i] = 0x00;
  _Daikin216Protocol.raw[0] =  0x11;
  _Daikin216Protocol.raw[1] =  0xDA;
  _Daikin216Protocol.raw[2] =  0x27;
  _Daikin216Protocol.raw[3] =  0xF0;
  // _Daikin216Protocol.raw[7] is a checksum byte, it will be set by checksum().
  _Daikin216Protocol.raw[8] =  0x11;
  _Daikin216Protocol.raw[9] =  0xDA;
  _Daikin216Protocol.raw[10] = 0x27;
  _Daikin216Protocol.raw[23] = 0xC0;
  // _Daikin216Protocol.raw[26] is a checksum byte, it will be set by checksum().
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Daikin216_getRaw(void) {
   Daikin216_checksum();  // Ensure correct settings before sending.
  return _Daikin216Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void Daikin216_setRaw(const uint8_t new_code[]) {
  memcpy(_Daikin216Protocol.raw, new_code, kDaikin216StateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin216_setPower(const bool on) { _Daikin216Protocol.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Daikin216_getPower(void) { return _Daikin216Protocol.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Daikin216_getMode(void) { return _Daikin216Protocol.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Daikin216_setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikinAuto:
    case kDaikinCool:
    case kDaikinHeat:
    case kDaikinFan:
    case kDaikinDry:
      _Daikin216Protocol.Mode = mode;
      break;
    default:
      _Daikin216Protocol.Mode = kDaikinAuto;
  }
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin216_convertMode(const opmode_t mode) {
  return DaikinESP_convertMode(mode);
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Daikin216_setTemp(const uint8_t temp) {
  uint8_t degrees = MAX(temp + 16, kDaikinMinTemp + 6);
  degrees = MIN(degrees, kDaikinMaxTemp);
  _Daikin216Protocol.Temp = degrees;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Daikin216_getTemp(void) { 
  return MAX(16, _Daikin216Protocol.Temp) - 16; 
}

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void Daikin216_setFan(const uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
    fanset = fan;
  else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
    fanset = kDaikinFanAuto;
  else
    fanset = 2 + fan;
  _Daikin216Protocol.Fan = fanset;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Daikin216_getFan(void) {
  uint8_t fan = _Daikin216Protocol.Fan;
  if (fan != kDaikinFanQuiet && fan != kDaikinFanAuto) fan -= 2;
  return fan;
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin216_convertFan(const fanspeed_t speed) {
  return DaikinESP_convertFan(speed);
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin216_setSwingVertical(const bool on) {
  _Daikin216Protocol.SwingV = (on ? kDaikin216SwingOn : kDaikin216SwingOff);
}

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin216_getSwingVertical(void) { return _Daikin216Protocol.SwingV; }

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin216_setSwingHorizontal(const bool on) {
  _Daikin216Protocol.SwingH = (on ? kDaikin216SwingOn : kDaikin216SwingOff);
}

/// Get the Horizontal Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin216_getSwingHorizontal(void) { return _Daikin216Protocol.SwingH; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @note This is a horrible hack till someone works out the quiet mode bit.
void Daikin216_setQuiet(const bool on) {
  if (on) {
    Daikin216_setFan(kDaikinFanQuiet);
    // Powerful & Quiet mode being on are mutually exclusive.
    Daikin216_setPowerful(false);
  } else if (Daikin216_getFan() == kDaikinFanQuiet) {
    Daikin216_setFan(kDaikinFanAuto);
  }
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
/// @note This is a horrible hack till someone works out the quiet mode bit.
bool Daikin216_getQuiet(void) { return Daikin216_getFan() == kDaikinFanQuiet; }

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin216_setPowerful(const bool on) {
  _Daikin216Protocol.Powerful = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) Daikin216_setQuiet(false);
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin216_getPowerful(void) { return _Daikin216Protocol.Powerful; }










void encode_Daikin160(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){
  Daikin160_setPower(ac_status.power_status);
  Daikin160_setTemp(ac_status.temperature);
  Daikin160_setMode(Daikin160_convertMode(ac_status.mode));
  Daikin160_setFan(Daikin160_convertFan(ac_status.fan));
  Daikin160_send(Daikin160_getRaw(), kDaikin160StateLength, kDaikin160DefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Daikin160(int16_t* input, uint8_t* output) {
  initDecodeData(input, DAIKIN160_BITS);
  if(Daikin160_recv(&gDecodeResult, 0, kDaikin160Bits, false))
    Daikin160_setRaw(gDecodeResult.state);
  else
    return;
  output[0] = Daikin160_getPower();
  output[1] = Daikin160_getTemp();
  output[2] = Daikin160_getFan();
  output[3] = Daikin160_getSwingVertical();
  output[4] = Daikin160_getMode();

  setAcStatus(output);
  ac_control_update_status_to_payload();
}

/// Send a Daikin160 (160-bit) A/C formatted message.
/// Status: STABLE / Confirmed working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Daikin160_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw) {
  if (nbytes < kDaikin160Section1Length)
    return;  // Not enough bytes to send a partial message.

  uint16_t offset = 0;

  for (uint16_t r = 0; r <= repeat; r++) {
    // Section #1
    sendGeneric_8( kDaikin160HdrMark, kDaikin160HdrSpace, 
                    kDaikin160BitMark, kDaikin160OneSpace,
                    kDaikin160BitMark, kDaikin160ZeroSpace,
                    kDaikin160BitMark, kDaikin160Gap,
                    data, kDaikin160Section1Length,
                    kDaikin160Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (4 + kDaikin160Section1Length * 8 * 2);
    // Section #2
    sendGeneric_8( kDaikin160HdrMark, kDaikin160HdrSpace,
                    kDaikin160BitMark, kDaikin160OneSpace,
                    kDaikin160BitMark, kDaikin160ZeroSpace,
                    kDaikin160BitMark, kDaikin160Gap,
                    data + kDaikin160Section1Length, nbytes - kDaikin160Section1Length,
                    kDaikin160Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (4 + (nbytes - kDaikin160Section1Length) * 8 * 2);
  }
}


#if SEND_DAIKIN160
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void Daikin160_send(const uint16_t repeat) {
  _irsend.sendDaikin160(getRaw(), kDaikin160StateLength, repeat);
}
#endif  // SEND_DAIKIN160

/// Decode the supplied Daikin 160-bit message. (DAIKIN160)
/// Status: STABLE / Confirmed working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Daikin160_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin160Bits) return false;

  const uint8_t ksectionSize[kDaikin160Sections] = {kDaikin160Section1Length,
                                                    kDaikin160Section2Length};

  // Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin160Sections; section++) {
    uint16_t used;
    // Section Header + Section Data (7 bytes) + Section Footer
    used = matchGeneric_8(results->rawbuf + offset, results->state + pos,
                          results->rawlen - offset, ksectionSize[section] * 8,
                          kDaikin160HdrMark, kDaikin160HdrSpace,
                          kDaikin160BitMark, kDaikin160OneSpace,
                          kDaikin160BitMark, kDaikin160ZeroSpace,
                          kDaikin160BitMark, kDaikin160Gap,
                          section >= kDaikin160Sections - 1,
                          kTolerance, kMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    // Validate the checksum.
    if (!Daikin160_validChecksum(results->state, kDaikin160StateLength)) return false;
  }

  // Success
  results->decode_type = DAIKIN160;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}



/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Daikin160_validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of section #1.
  if (length <= kDaikin160Section1Length - 1 ||
      state[kDaikin160Section1Length - 1] != sumBytes( state, kDaikin160Section1Length - 1, 0))
    return false;
  // Validate the checksum of section #2 (a.k.a. the rest)
  if (length <= kDaikin160Section1Length + 1 ||
      state[length - 1] != sumBytes(state + kDaikin160Section1Length, length - kDaikin160Section1Length - 1, 0))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void Daikin160_checksum(void) {
  _Daikin160Protocol.Sum1 = sumBytes(_Daikin160Protocol.raw, kDaikin160Section1Length - 1, 0);
  _Daikin160Protocol.Sum2 = sumBytes(_Daikin160Protocol.raw + kDaikin160Section1Length, kDaikin160Section2Length - 1, 0);
}

/// Reset the internal state to a fixed known good state.
void Daikin160_stateReset(void) {
  for (uint8_t i = 0; i < kDaikin160StateLength; i++) _Daikin160Protocol.raw[i] = 0x00;
  _Daikin160Protocol.raw[0] =  0x11;
  _Daikin160Protocol.raw[1] =  0xDA;
  _Daikin160Protocol.raw[2] =  0x27;
  _Daikin160Protocol.raw[3] =  0xF0;
  _Daikin160Protocol.raw[4] =  0x0D;
  // _Daikin160Protocol.raw[6] is a checksum byte, it will be set by checksum().
  _Daikin160Protocol.raw[7] =  0x11;
  _Daikin160Protocol.raw[8] =  0xDA;
  _Daikin160Protocol.raw[9] =  0x27;
  _Daikin160Protocol.raw[11] = 0xD3;
  _Daikin160Protocol.raw[12] = 0x30;
  _Daikin160Protocol.raw[13] = 0x11;
  _Daikin160Protocol.raw[16] = 0x1E;
  _Daikin160Protocol.raw[17] = 0x0A;
  _Daikin160Protocol.raw[18] = 0x08;
  // _Daikin160Protocol.raw[19] is a checksum byte, it will be set by checksum().
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Daikin160_getRaw(void) {
  Daikin160_checksum();  // Ensure correct settings before sending.
  return _Daikin160Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void Daikin160_setRaw(const uint8_t new_code[]) {
  memcpy(_Daikin160Protocol.raw, new_code, kDaikin160StateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin160_setPower(const bool on) { _Daikin160Protocol.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Daikin160_getPower(void) { return _Daikin160Protocol.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Daikin160_getMode(void) { return _Daikin160Protocol.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Daikin160_setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikinAuto:
    case kDaikinCool:
    case kDaikinHeat:
    case kDaikinFan:
    case kDaikinDry:
      _Daikin160Protocol.Mode = mode;
      break;
    default: _Daikin160Protocol.Mode = kDaikinAuto;
  }
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin160_convertMode(const opmode_t mode) {
  return DaikinESP_convertMode(mode);
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Daikin160_setTemp(const uint8_t temp) {
  uint8_t degrees = MAX(temp + 16, kDaikinMinTemp + 6);
  degrees = MIN(degrees, kDaikinMaxTemp) - 10;
  _Daikin160Protocol.Temp = degrees;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Daikin160_getTemp(void) { 
  if(_Daikin160Protocol.Temp <= 6)
    return 0;
  return _Daikin160Protocol.Temp;
}

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void Daikin160_setFan(const uint8_t fan) {
  uint8_t fanset;
  if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
    fanset = fan;
  else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
    fanset = kDaikinFanAuto;
  else
    fanset = 2 + fan;
  _Daikin160Protocol.Fan = fanset;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Daikin160_getFan(void) {
  uint8_t fan = _Daikin160Protocol.Fan;
  if (fan != kDaikinFanQuiet && fan != kDaikinFanAuto) fan -= 2;
  return fan;
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin160_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin: return kDaikinFanMin;
    case kFanSpeedLow: return kDaikinFanMin + 1;
    case kFanSpeedMedium: return kDaikinFanMin + 2;
    case kFanSpeedHigh: return kDaikinFanMax - 1;
    case kFanSpeedMax: return kDaikinFanMax;
    default:
      return kDaikinFanAuto;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void Daikin160_setSwingVertical(const uint8_t position) {
  switch (position) {
    case kDaikin160SwingVLowest:
    case kDaikin160SwingVLow:
    case kDaikin160SwingVMiddle:
    case kDaikin160SwingVHigh:
    case kDaikin160SwingVHighest:
    case kDaikin160SwingVAuto:
      _Daikin160Protocol.SwingV = position;
      break;
    default: _Daikin160Protocol.SwingV = kDaikin160SwingVAuto;
  }
}

/// Get the Vertical Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Daikin160_getSwingVertical(void) { return _Daikin160Protocol.SwingV; }

/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin160_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest:
    case kSwingVHigh:
    case kSwingVMiddle:
    case kSwingVLow:
    case kSwingVLowest:
      return kDaikin160SwingVHighest + 1 - (uint8_t)position;
    default:
      return kDaikin160SwingVAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common vertical swing position.
swingv_t Daikin160_toCommonSwingV(const uint8_t setting) {
  switch (setting) {
    case kDaikin160SwingVHighest: return kSwingVHighest;
    case kDaikin160SwingVHigh:    return kSwingVHigh;
    case kDaikin160SwingVMiddle:  return kSwingVMiddle;
    case kDaikin160SwingVLow:     return kSwingVLow;
    case kDaikin160SwingVLowest:  return kSwingVLowest;
    default:
      return kSwingVAuto;
  }
}






void encode_Daikin176(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){

  Daikin176_setPower(ac_status.power_status);
  Daikin176_setTemp(ac_status.temperature);
  Daikin176_setMode(Daikin176_convertMode(ac_status.mode));
  Daikin176_setFan(Daikin176_convertFan(ac_status.fan));
  Daikin176_send(Daikin176_getRaw(), kDaikin176StateLength, kDaikin176DefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Daikin176(int16_t* input, uint8_t* output) {
  initDecodeData(input, DAIKIN176_BITS);
  if(Daikin176_recv(&gDecodeResult, 0, kDaikin176Bits, false))
    Daikin176_setRaw(gDecodeResult.state);
  else
    return;
  output[0] = Daikin176_getPower();
  output[1] = Daikin176_getTemp();
  output[2] = Daikin176_toCommonFanSpeed(Daikin176_getFan());
  // output[3] = Daikin176_getSwing();
  output[4] = Daikin176_toCommonMode(Daikin176_getMode());

  setAcStatus(output);
  ac_control_update_status_to_payload();
}


/// Send a Daikin176 (176-bit) A/C formatted message.
/// Status: STABLE / Working on a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Daikin176_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw) {
  if (nbytes < kDaikin176Section1Length)
    return;  // Not enough bytes to send a partial message.

  uint16_t offset = 0;

  for (uint16_t r = 0; r <= repeat; r++) {
    // Section #1
    sendGeneric_8( kDaikin176HdrMark, kDaikin176HdrSpace, 
                    kDaikin176BitMark, kDaikin176OneSpace,
                    kDaikin176BitMark, kDaikin176ZeroSpace,
                    kDaikin176BitMark, kDaikin176Gap,
                    data, kDaikin176Section1Length,
                    kDaikin176Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (4 + kDaikin176Section1Length * 8 * 2);
    // Section #2
    sendGeneric_8( kDaikin176HdrMark, kDaikin176HdrSpace, 
                    kDaikin176BitMark, kDaikin176OneSpace,
                    kDaikin176BitMark, kDaikin176ZeroSpace,
                    kDaikin176BitMark, kDaikin176Gap,
                    data + kDaikin176Section1Length, nbytes - kDaikin176Section1Length,
                    kDaikin176Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (4 + (nbytes - kDaikin176Section1Length) * 8 * 2);
  }
}


#if SEND_DAIKIN176
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void Daikin176_send(const uint16_t repeat) {
  _irsend.sendDaikin176(getRaw(), kDaikin176StateLength, repeat);
}
#endif  // SEND_DAIKIN176

/// Decode the supplied Daikin 176-bit message. (DAIKIN176)
/// Status: STABLE / Expected to work.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Daikin176_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin176Bits) return false;

  const uint8_t ksectionSize[kDaikin176Sections] = {kDaikin176Section1Length,
                                                    kDaikin176Section2Length};

  // Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin176Sections; section++) {
    uint16_t used;
    // Section Header + Section Data (7 bytes) + Section Footer
    used = matchGeneric_8(  results->rawbuf + offset, results->state + pos,
                            results->rawlen - offset, ksectionSize[section] * 8,
                            kDaikin176HdrMark, kDaikin176HdrSpace,
                            kDaikin176BitMark, kDaikin176OneSpace,
                            kDaikin176BitMark, kDaikin176ZeroSpace,
                            kDaikin176BitMark, kDaikin176Gap,
                            section >= kDaikin176Sections - 1,
                            kTolerance, kMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    // Validate the checksum.
    if (!Daikin176_validChecksum(results->state, kDaikin176StateLength)) return false;
  }

  // Success
  results->decode_type = DAIKIN176;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}




/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Daikin176_validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of section #1.
  if (length <= kDaikin176Section1Length - 1 ||
      state[kDaikin176Section1Length - 1] != sumBytes( state, kDaikin176Section1Length - 1, 0))
    return false;
  // Validate the checksum of section #2 (a.k.a. the rest)
  if (length <= kDaikin176Section1Length + 1 ||
      state[length - 1] != sumBytes(state + kDaikin176Section1Length, length - kDaikin176Section1Length - 1, 0))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void Daikin176_checksum(void) {
  _Daikin176Protocol.Sum1 = sumBytes(_Daikin176Protocol.raw, kDaikin176Section1Length - 1, 0);
  _Daikin176Protocol.Sum2 = sumBytes(_Daikin176Protocol.raw + kDaikin176Section1Length, kDaikin176Section2Length - 1, 0);
}

/// Reset the internal state to a fixed known good state.
void Daikin176_stateReset(void) {
  for (uint8_t i = 0; i < kDaikin176StateLength; i++) _Daikin176Protocol.raw[i] = 0x00;
  _Daikin176Protocol.raw[0] =  0x11;
  _Daikin176Protocol.raw[1] =  0xDA;
  _Daikin176Protocol.raw[2] =  0x17;
  _Daikin176Protocol.raw[3] =  0x18;
  _Daikin176Protocol.raw[4] =  0x04;
  // _Daikin176Protocol.raw[6] is a checksum byte, it will be set by checksum().
  _Daikin176Protocol.raw[7] =  0x11;
  _Daikin176Protocol.raw[8] =  0xDA;
  _Daikin176Protocol.raw[9] =  0x17;
  _Daikin176Protocol.raw[10] = 0x18;
  _Daikin176Protocol.raw[12] = 0x73;
  _Daikin176Protocol.raw[14] = 0x20;
  _Daikin176Protocol.raw[18] = 0x16;  // Fan speed and swing
  _Daikin176Protocol.raw[20] = 0x20;
  // _Daikin176Protocol.raw[21] is a checksum byte, it will be set by checksum().
  _Daikin176_savedTemp = Daikin176_getTemp();
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Daikin176_getRaw(void) {
  Daikin176_checksum();  // Ensure correct settings before sending.
  return _Daikin176Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void Daikin176_setRaw(const uint8_t new_code[]) {
  memcpy(_Daikin176Protocol.raw, new_code, kDaikin176StateLength);
  _Daikin176_savedTemp = Daikin176_getTemp();
}


/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin176_setPower(const bool on) {
  _Daikin176Protocol.ModeButton = 0;
  _Daikin176Protocol.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Daikin176_getPower(void) { return _Daikin176Protocol.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Daikin176_getMode(void) { return _Daikin176Protocol.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Daikin176_setMode(const uint8_t mode) {
  uint8_t altmode = 0;
  // Set the mode bits.
  _Daikin176Protocol.Mode = mode;
  // Daikin172 has some alternate/additional mode bits that need to be changed
  // in line with the operating mode. The following few lines match up these
  // bits with the corresponding operating bits.
  switch (mode) {
    case kDaikin176Dry:  altmode = 2; break;
    case kDaikin176Fan:  altmode = 6; break;
    case kDaikin176Auto:
    case kDaikin176Cool:
    case kDaikin176Heat: altmode = 7; break;
    default: _Daikin176Protocol.Mode = kDaikin176Cool; altmode = 7; break;
  }
  // Set the additional mode bits.
  _Daikin176Protocol.AltMode = altmode;
  // Needs to happen after setTemp() as it will clear it.
  _Daikin176Protocol.ModeButton = kDaikin176ModeButton;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin176_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeDry:   return kDaikin176Dry;
    case kOpModeHeat:  return kDaikin176Heat;
    case kOpModeFan:   return kDaikin176Fan;
    case kOpModeAuto:  return kDaikin176Auto;
    default:                      return kDaikin176Cool;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Daikin176_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin176Dry:  return kOpModeDry;
    case kDaikin176Heat: return kOpModeHeat;
    case kDaikin176Fan:  return kOpModeFan;
    case kDaikin176Auto: return kOpModeAuto;
    default: return kOpModeCool;
  }
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Daikin176_setTemp(const uint8_t temp) {
  uint8_t degrees = MIN(kDaikinMaxTemp, MAX(temp + 16, kDaikinMinTemp + 6));
  _Daikin176_savedTemp = degrees;
  switch (_Daikin176Protocol.Mode) {
    case kDaikin176Dry:
    case kDaikin176Fan:
      degrees = kDaikin176DryFanTemp; break;
  }
  _Daikin176Protocol.Temp = degrees - 9;
  _Daikin176Protocol.ModeButton = 0;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Daikin176_getTemp(void) { 

  if(_Daikin176Protocol.Temp + 9 <= 16)
    return 0;
  return _Daikin176Protocol.Temp + 9 - 16;
}

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1 for Min or 3 for Max
void Daikin176_setFan(const uint8_t fan) {
  switch (fan) {
    case kDaikinFanMin:
    case kDaikin176FanMax:
      _Daikin176Protocol.Fan = fan;
      break;
    default:
      _Daikin176Protocol.Fan = kDaikin176FanMax;
      break;
  }
  _Daikin176Protocol.ModeButton = 0;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Daikin176_getFan(void) { return _Daikin176Protocol.Fan; }

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin176_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:
    case kFanSpeedLow: return kDaikinFanMin;
    default: return kDaikin176FanMax;
  }
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void Daikin176_setSwingHorizontal(const uint8_t position) {
  switch (position) {
    case kDaikin176SwingHOff:
    case kDaikin176SwingHAuto:
      _Daikin176Protocol.SwingH = position;
      break;
    default: _Daikin176Protocol.SwingH = kDaikin176SwingHAuto;
  }
}

/// Get the Horizontal Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Daikin176_getSwingHorizontal(void) { return _Daikin176Protocol.SwingH; }

/// Get the Unit Id of the A/C.
/// @return The Unit Id the A/C is to use.
uint8_t Daikin176_getId(void) { return _Daikin176Protocol.Id1; }

/// Set the Unit Id of the A/C.
/// @param[in] num The Unit Id the A/C is to use.
/// @note 0 for Unit A; 1 for Unit B
void Daikin176_setId(const uint8_t num) { _Daikin176Protocol.Id1 = _Daikin176Protocol.Id2 = num; }

/// Convert a swingh_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin176_convertSwingH(const swingh_t position) {
  switch (position) {
    case kSwingHOff:  return kDaikin176SwingHOff;
    case kSwingHAuto: return kDaikin176SwingHAuto;
    default: return kDaikin176SwingHAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common horizontal swing position.
swingh_t Daikin176_toCommonSwingH(const uint8_t setting) {
  switch (setting) {
    case kDaikin176SwingHOff: return kSwingHOff;
    case kDaikin176SwingHAuto: return kSwingHAuto;
    default:
      return kSwingHAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Daikin176_toCommonFanSpeed(const uint8_t speed) {
  return (speed == kDaikinFanMin) ? kFanSpeedMin
                                  : kFanSpeedMax;
}




void encode_Daikin128(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){
  Daikin128_setPowerToggle(ac_status.power_status);
  Daikin128_setTemp(ac_status.temperature);
  Daikin128_setMode(Daikin128_convertMode(ac_status.mode));
  Daikin128_setFan(Daikin128_convertFan(ac_status.fan));
  Daikin128_send(Daikin128_getRaw(), kDaikin128StateLength, kDaikin128DefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Daikin128(int16_t* input, uint8_t* output) {

  initDecodeData(input, DAIKIN128_BITS);
  if(Daikin128_recv(&gDecodeResult, 0, kDaikin128Bits, false))
    Daikin128_setRaw(gDecodeResult.state);
  else
    return;
  output[0] = Daikin128_getPowerToggle();
  output[1] = Daikin128_getTemp();
  output[2] = Daikin128_toCommonFanSpeed(Daikin128_getFan());
  output[3] = Daikin128_getSwingVertical();
  output[4] = Daikin128_toCommonMode(Daikin128_getMode());

  setAcStatus(output);
  ac_control_update_status_to_payload();
}

/// Send a Daikin128 (128-bit) A/C formatted message.
/// Status: STABLE / Known Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Daikin128_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw) {
  if (nbytes < kDaikin128SectionLength)
    return;  // Not enough bytes to send a partial message.

  uint16_t offset = 0;

  for (uint16_t r = 0; r <= repeat; r++) {
    // Leader
    for (uint8_t i = 0; i < 2; i++) {
      irRaw[offset++] = kDaikin128LeaderMark;
      irRaw[offset++] = -kDaikin128LeaderSpace;
    }
    // Section #1 (Header + Data)
    sendGeneric_8( kDaikin128HdrMark, kDaikin128HdrSpace,
                    kDaikin128BitMark, kDaikin128OneSpace,
                    kDaikin128BitMark, kDaikin128ZeroSpace,
                    kDaikin128BitMark, kDaikin128Gap,
                    data, kDaikin128SectionLength,
                    kDaikin128Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (4 + kDaikin128SectionLength  * 8 * 2);
    // Section #2 (Data + Footer)
    sendGeneric_8( 0, 0,
                    kDaikin128BitMark, kDaikin128OneSpace,
                    kDaikin128BitMark, kDaikin128ZeroSpace,
                    kDaikin128FooterMark, kDaikin128Gap,
                    data + kDaikin128SectionLength, nbytes - kDaikin128SectionLength,
                    kDaikin128Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (2 + (nbytes - kDaikin128SectionLength)  * 8 * 2);
  }
}


uint8_t Daikin128_calcFirstChecksum(const uint8_t state[]) {
  return sumNibbles_8(state, kDaikin128SectionLength - 1, state[kDaikin128SectionLength - 1] & 0x0F) & 0x0F;
}

uint8_t Daikin128_calcSecondChecksum(const uint8_t state[]) {
  return sumNibbles_8(state + kDaikin128SectionLength, kDaikin128SectionLength - 1, 0);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Daikin128_validChecksum(uint8_t state[]) {
  // Validate the checksum of section #1.
  if (state[kDaikin128SectionLength - 1] >> 4 != Daikin128_calcFirstChecksum(state))
    return false;
  // Validate the checksum of section #2
  if (state[kDaikin128StateLength - 1] != Daikin128_calcSecondChecksum(state))
    return false;
  return true;
}

/// Decode the supplied Daikin 128-bit message. (DAIKIN128)
/// Status: STABLE / Known Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Daikin128_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader) + kFooter - 1 + offset)
    return false;
  if (nbits / 8 <= kDaikin128SectionLength) return false;
 
  // Compliance
  if (strict && nbits != kDaikin128Bits) return false;

  // Leader
  for (uint8_t i = 0; i < 2; i++) {
    if (!matchMark(results->rawbuf[offset++], kDaikin128LeaderMark, kTolerance)) return false;
    if (!matchSpace(results->rawbuf[offset++], kDaikin128LeaderSpace, kTolerance)) return false;
  }
  const uint16_t ksectionSize[kDaikin128Sections] = {
      kDaikin128SectionLength, (uint16_t)(nbits / 8 - kDaikin128SectionLength)};
  // Data Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin128Sections; section++) {
    uint16_t used;
    // Section Header (first section only) + Section Data (8 bytes) +
    //     Section Footer (Not for first section)
    used = matchGeneric_8(results->rawbuf + offset, results->state + pos,
                          results->rawlen - offset, ksectionSize[section] * 8,
                          section == 0 ? kDaikin128HdrMark : 0,
                          section == 0 ? kDaikin128HdrSpace : 0,
                          kDaikin128BitMark, kDaikin128OneSpace,
                          kDaikin128BitMark, kDaikin128ZeroSpace,
                          section > 0 ? kDaikin128FooterMark : kDaikin128BitMark,
                          kDaikin128Gap,
                          section > 0,
                          kTolerance, kMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    if (!Daikin128_validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = DAIKIN128;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


/// Calculate and set the checksum values for the internal state.
void Daikin128_checksum(void) {
  _Daikin128Protocol.Sum1 = Daikin128_calcFirstChecksum(_Daikin128Protocol.raw);
  _Daikin128Protocol.Sum2 = Daikin128_calcSecondChecksum(_Daikin128Protocol.raw);
}

/// Reset the internal state to a fixed known good state.
void Daikin128_stateReset(void) {
  for (uint8_t i = 0; i < kDaikin128StateLength; i++) _Daikin128Protocol.raw[i] = 0x00;
  _Daikin128Protocol.raw[0] = 0x16;
  _Daikin128Protocol.raw[7] = 0x04;  // Most significant nibble is a checksum.
  _Daikin128Protocol.raw[8] = 0xA1;
  // _Daikin128Protocol.raw[15] is a checksum byte, it will be set by checksum().
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Daikin128_getRaw(void) {
  Daikin128_checksum();  // Ensure correct settings before sending.
  return _Daikin128Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void Daikin128_setRaw(const uint8_t new_code[]) {
  memcpy(_Daikin128Protocol.raw, new_code, kDaikin128StateLength);
}

#if SEND_DAIKIN128
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void Daikin128_send(const uint16_t repeat) {
  _irsend.sendDaikin128(getRaw(), kDaikin128StateLength, repeat);
}
#endif  // SEND_DAIKIN128

/// Set the Power toggle setting of the A/C.
/// @param[in] toggle true, the setting is on. false, the setting is off.
void Daikin128_setPowerToggle(const bool toggle) { _Daikin128Protocol.Power = toggle; }

/// Get the Power toggle setting of the A/C.
/// @return The current operating mode setting.
bool Daikin128_getPowerToggle(void) { return _Daikin128Protocol.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Daikin128_getMode(void) { return _Daikin128Protocol.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Daikin128_setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin128Auto:
    case kDaikin128Cool:
    case kDaikin128Heat:
    case kDaikin128Fan:
    case kDaikin128Dry:
      _Daikin128Protocol.Mode = mode;
      break;
    default:
      _Daikin128Protocol.Mode = kDaikin128Auto;
      break;
  }
  // Force a reset of mode dependant things.
  Daikin128_setFan(Daikin128_getFan());  // Covers Quiet & Powerful too.
  Daikin128_setEcono(Daikin128_getEcono());
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin128_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kDaikin128Cool;
    case kOpModeHeat: return kDaikin128Heat;
    case kOpModeDry: return kDaikinDry;
    case kOpModeFan: return kDaikin128Fan;
    default: return kDaikin128Auto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Daikin128_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin128Cool: return kOpModeCool;
    case kDaikin128Heat: return kOpModeHeat;
    case kDaikin128Dry: return kOpModeDry;
    case kDaikin128Fan: return kOpModeFan;
    default: return kOpModeAuto;
  }
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Daikin128_setTemp(const uint8_t temp) {
  _Daikin128Protocol.Temp = uint8ToBcd(MIN(kDaikin128MaxTemp, MAX(temp +16, kDaikin128MinTemp)));
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Daikin128_getTemp(void) { return bcdToUint8(_Daikin128Protocol.Temp) - 16; }

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Daikin128_getFan(void) { return _Daikin128Protocol.Fan; }

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Daikin128_setFan(const uint8_t speed) {
  uint8_t new_speed = speed;
  uint8_t mode = _Daikin128Protocol.Mode;
  switch (speed) {
    case kDaikin128FanQuiet:
    case kDaikin128FanPowerful:
      if (mode == kDaikin128Auto) new_speed = kDaikin128FanAuto;
      // FALL-THRU
    case kDaikin128FanAuto:
    case kDaikin128FanHigh:
    case kDaikin128FanMed:
    case kDaikin128FanLow:
      _Daikin128Protocol.Fan = new_speed;
      break;
    default:
      _Daikin128Protocol.Fan = kDaikin128FanAuto;
      return;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin128_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin: return kDaikinFanQuiet;
    case kFanSpeedLow: return kDaikin128FanLow;
    case kFanSpeedMedium: return kDaikin128FanMed;
    case kFanSpeedHigh: return kDaikin128FanHigh;
    case kFanSpeedMax: return kDaikin128FanPowerful;
    default: return kDaikin128FanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Daikin128_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kDaikin128FanPowerful: return kFanSpeedMax;
    case kDaikin128FanHigh: return kFanSpeedHigh;
    case kDaikin128FanMed: return kFanSpeedMedium;
    case kDaikin128FanLow: return kFanSpeedLow;
    case kDaikinFanQuiet: return kFanSpeedMin;
    default: return kFanSpeedAuto;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin128_setSwingVertical(const bool on) { _Daikin128Protocol.SwingV = on; }

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin128_getSwingVertical(void) { return _Daikin128Protocol.SwingV; }

/// Set the Sleep mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin128_setSleep(const bool on) { _Daikin128Protocol.Sleep = on; }

/// Get the Sleep mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin128_getSleep(void) { return _Daikin128Protocol.Sleep; }

/// Set the Economy mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin128_setEcono(const bool on) {
  uint8_t mode = _Daikin128Protocol.Mode;
  _Daikin128Protocol.Econo = (on && (mode == kDaikin128Cool || mode == kDaikin128Heat));
}

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin128_getEcono(void) { return _Daikin128Protocol.Econo; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin128_setQuiet(const bool on) {
  uint8_t mode = _Daikin128Protocol.Mode;
  if (on && (mode == kDaikin128Cool || mode == kDaikin128Heat))
    Daikin128_setFan(kDaikin128FanQuiet);
  else if (_Daikin128Protocol.Fan == kDaikin128FanQuiet)
    Daikin128_setFan(kDaikin128FanAuto);
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin128_getQuiet(void) { return _Daikin128Protocol.Fan == kDaikin128FanQuiet; }

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin128_setPowerful(const bool on) {
  uint8_t mode = _Daikin128Protocol.Mode;
  if (on && (mode == kDaikin128Cool || mode == kDaikin128Heat))
    Daikin128_setFan(kDaikin128FanPowerful);
  else if (_Daikin128Protocol.Fan == kDaikin128FanPowerful)
    Daikin128_setFan(kDaikin128FanAuto);
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin128_getPowerful(void) {
  return _Daikin128Protocol.Fan == kDaikin128FanPowerful;
}

/// Set the clock on the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void Daikin128_setClock(const uint16_t mins_since_midnight) {
  uint16_t mins = mins_since_midnight;
  if (mins_since_midnight >= 24 * 60) mins = 0;  // Bounds check.
  // Hours.
  _Daikin128Protocol.ClockHours = uint8ToBcd(mins / 60);
  // Minutes.
  _Daikin128Protocol.ClockMins = uint8ToBcd(mins % 60);
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin128_getClock(void) {
  return bcdToUint8(_Daikin128Protocol.ClockHours) * 60 + bcdToUint8(_Daikin128Protocol.ClockMins);
}

/// Set the enable status of the On Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin128_setOnTimerEnabled(const bool on) { _Daikin128Protocol.OnTimer = on; }

/// Get the enable status of the On Timer.
/// @return true, the setting is on. false, the setting is off.
bool Daikin128_getOnTimerEnabled(void) { return _Daikin128Protocol.OnTimer; }

#define SETTIME(x, n) do { \
  uint16_t mins = n;\
  if (n >= 24 * 60) mins = 0;\
  _Daikin128Protocol.x##HalfHour = (mins % 60) >= 30;\
  _Daikin128Protocol.x##Hours = uint8ToBcd(mins / 60);\
} while (0)

#define GETTIME(x) bcdToUint8(_Daikin128Protocol.x##Hours) * 60 + (_Daikin128Protocol.x##HalfHour ? 30 : 0)

/// Set the On Timer time for the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void Daikin128_setOnTimer(const uint16_t mins_since_midnight) {
  SETTIME(On, mins_since_midnight);
}

/// Get the On Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin128_getOnTimer(void) { return GETTIME(On); }

/// Set the enable status of the Off Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin128_setOffTimerEnabled(const bool on) { _Daikin128Protocol.OffTimer = on; }

/// Get the enable status of the Off Timer.
/// @return true, the setting is on. false, the setting is off.
bool Daikin128_getOffTimerEnabled(void) { return _Daikin128Protocol.OffTimer; }

/// Set the Off Timer time for the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void Daikin128_setOffTimer(const uint16_t mins_since_midnight) {
  SETTIME(Off, mins_since_midnight);
}

/// Get the Off Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin128_getOffTimer(void) { return GETTIME(Off); }

/// Set the Light toggle setting of the A/C.
/// @param[in] unit Device to show the LED (Light) Display info about.
/// @note 0 is off.
void Daikin128_setLightToggle(const uint8_t unit) {
  _Daikin128Protocol.Ceiling = 0;
  _Daikin128Protocol.Wall = 0;
  switch (unit) {
    case kDaikin128BitCeiling:
      _Daikin128Protocol.Ceiling = 1;
      break;
    case kDaikin128BitWall:
      _Daikin128Protocol.Wall = 1;
      break;
  }
}

/// Get the Light toggle setting of the A/C.
/// @return The current operating mode setting.
uint8_t Daikin128_getLightToggle(void) {
  uint8_t code = 0;
  if (_Daikin128Protocol.Ceiling) {
    code = kDaikin128BitCeiling;
  } else if (_Daikin128Protocol.Wall) {
    code = kDaikin128BitWall;
  }

  return code;
}







void encode_Daikin152(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){

  Daikin152_setPower(ac_status.power_status);
  Daikin152_setTemp(ac_status.temperature);
  Daikin152_setMode(Daikin152_convertMode(ac_status.mode));
  Daikin152_setFan(Daikin152_convertFan(ac_status.fan));
  Daikin152_send(Daikin152_getRaw(), kDaikin152StateLength, kDaikin152DefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}

void decode_Daikin152(int16_t* input, uint8_t* output) {

  initDecodeData(input, DAIKIN152_BITS);
  if(Daikin152_recv(&gDecodeResult, 0, kDaikin152Bits, false))
    Daikin152_setRaw(gDecodeResult.state);
  else
    return;
  output[0] = Daikin152_getPower();
  output[1] = Daikin152_getTemp();
  output[2] = DaikinESP_toCommonFanSpeed(Daikin152_getFan()); // same as ESP
  output[3] = Daikin152_getSwingV();
  output[4] = DaikinESP_toCommonMode(Daikin152_getMode());    // same as ESP

  setAcStatus(output);
  ac_control_update_status_to_payload();
}

/// Send a Daikin152 (152-bit) A/C formatted message.
/// Status: STABLE / Known Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Daikin152_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw) {
  int16_t offset = 0;
  for (uint16_t r = 0; r <= repeat; r++) {
    // Leader
    sendGeneric_64( 0, 0,
                    kDaikin152BitMark, kDaikin152OneSpace,
                    kDaikin152BitMark, kDaikin152ZeroSpace,
                    kDaikin152BitMark, kDaikin152Gap,
                    0, (uint64_t)0, kDaikin152LeaderBits,
                    kDaikin152Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (2 + kDaikin152LeaderBits*2);
    // Header + Data + Footer
    sendGeneric_8( kDaikin152HdrMark, kDaikin152HdrSpace,
                    kDaikin152BitMark, kDaikin152OneSpace, 
                    kDaikin152BitMark, kDaikin152ZeroSpace,
                    kDaikin152BitMark, kDaikin152Gap,
                    data, nbytes,
                    kDaikin152Freq, false, 0, kDutyDefault,
                    &irRaw[offset]);
    offset += (4 + nbytes * 8 * 2);
  }
}

/// Decode the supplied Daikin 152-bit message. (DAIKIN152)
/// Status: STABLE / Known Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Daikin152_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (5 + nbits + kFooter) + kHeader - 1 + offset)
    return false;
  if (nbits / 8 < kDaikin152StateLength) return false;

  // Compliance
  if (strict && nbits != kDaikin152Bits) return false;

  uint16_t used;

  // Leader
  uint64_t leader = 0;
  used = matchGeneric_64(results->rawbuf + offset, &leader,
                      results->rawlen - offset, kDaikin152LeaderBits,
                      0, 0,  // No Header
                      kDaikin152BitMark, kDaikin152OneSpace,
                      kDaikin152BitMark, kDaikin152ZeroSpace,
                      kDaikin152BitMark, kDaikin152Gap,  // Footer gap
                      false, kTolerance, kMarkExcess, false);
  if (used == 0 || leader != 0) return false;
  offset += used;

  // Header + Data + Footer
  used = matchGeneric_8(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kDaikin152HdrMark, kDaikin152HdrSpace,
                      kDaikin152BitMark, kDaikin152OneSpace,
                      kDaikin152BitMark, kDaikin152ZeroSpace,
                      kDaikin152BitMark, kDaikin152Gap,
                      true, kTolerance, kMarkExcess, false);
  if (used == 0) return false;

  // Compliance
  if (strict) {
    if (!Daikin152_validChecksum(results->state, kDaikin152StateLength)) return false;
  }

  // Success
  results->decode_type = DAIKIN152;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}

bool Daikin152_isHeaderMatch(int16_t *rawbuf){
  
  initDecodeData(rawbuf, DAIKIN152_BITS); // 10 bits header + space + gap
  uint16_t used;

  // Leader
  uint64_t leader = 0;
  used = matchGeneric_64( gDecodeResult.rawbuf, &leader,
                          gDecodeResult.rawlen, kDaikin152LeaderBits,
                          0, 0,  // No Header
                          kDaikin152BitMark, kDaikin152OneSpace,
                          kDaikin152BitMark, kDaikin152ZeroSpace,
                          kDaikin152BitMark, kDaikin152Gap,  // Footer gap
                          false, kTolerance, kMarkExcess, false);
  if (used == 0)
    return false;
  return true;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Daikin152_validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of the given state.
  if (length <= 1 || state[length - 1] != sumBytes(state, length - 1, 0))
    return false;
  else
    return true;
}

/// Calculate and set the checksum values for the internal state.
void Daikin152_checksum(void) {
  _Daikin152Protocol.Sum = sumBytes(_Daikin152Protocol.raw, kDaikin152StateLength - 1, 0);
}

/// Reset the internal state to a fixed known good state.
void Daikin152_stateReset(void) {
  for (uint8_t i = 3; i < kDaikin152StateLength; i++) _Daikin152Protocol.raw[i] = 0x00;
  _Daikin152Protocol.raw[0] =  0x11;
  _Daikin152Protocol.raw[1] =  0xDA;
  _Daikin152Protocol.raw[2] =  0x27;
  _Daikin152Protocol.raw[15] = 0xC5;
  // _Daikin152Protocol.raw[19] is a checksum byte, it will be set by checksum().
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Daikin152_getRaw(void) {
  Daikin152_checksum();  // Ensure correct settings before sending.
  return _Daikin152Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void Daikin152_setRaw(const uint8_t new_code[]) {
  memcpy(_Daikin152Protocol.raw, new_code, kDaikin152StateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin152_setPower(const bool on) { _Daikin152Protocol.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Daikin152_getPower(void) { return _Daikin152Protocol.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Daikin152_getMode(void) { return _Daikin152Protocol.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Daikin152_setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikinFan:
      Daikin152_setTemp(kDaikin152FanTemp);  // Handle special temp for fan mode.
      break;
    case kDaikinDry:
      Daikin152_setTemp(kDaikin152DryTemp);  // Handle special temp for dry mode.
      break;
    case kDaikinAuto:
    case kDaikinCool:
    case kDaikinHeat:
      break;
    default:
      _Daikin152Protocol.Mode = kDaikinAuto;
      return;
  }
  _Daikin152Protocol.Mode = mode;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin152_convertMode(const opmode_t mode) {
  return DaikinESP_convertMode(mode);
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Daikin152_setTemp(const uint8_t temp) {
  uint8_t degrees = MAX(
      temp + 16, (_Daikin152Protocol.Mode == kDaikinHeat) ? kDaikinMinTemp : kDaikin2MinCoolTemp);
  degrees = MIN(degrees, kDaikinMaxTemp);
  if (temp == kDaikin152FanTemp) degrees = temp;  // Handle fan only temp.
  _Daikin152Protocol.Temp = degrees;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Daikin152_getTemp(void) { 
  if(_Daikin152Protocol.Temp == kDaikin152FanTemp)
    return 0;
  if(_Daikin152Protocol.Mode == kDaikinHeat)
    return 0;
  return _Daikin152Protocol.Temp - 16; 
}

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void Daikin152_setFan(const uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
    fanset = fan;
  else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
    fanset = kDaikinFanAuto;
  else
    fanset = 2 + fan;
  _Daikin152Protocol.Fan = fanset;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Daikin152_getFan(void) {
  const uint8_t fan = _Daikin152Protocol.Fan;
  switch (fan) {
    case kDaikinFanAuto:
    case kDaikinFanQuiet: return fan;
    default: return fan - 2;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin152_convertFan(const fanspeed_t speed) {
  return DaikinESP_convertFan(speed);
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin152_setSwingV(const bool on) {
  _Daikin152Protocol.SwingV = (on ? kDaikinSwingOn : kDaikinSwingOff);
}

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin152_getSwingV(void) { return _Daikin152Protocol.SwingV; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin152_setQuiet(const bool on) {
  _Daikin152Protocol.Quiet = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) Daikin152_setPowerful(false);
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin152_getQuiet(void) { return _Daikin152Protocol.Quiet; }

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin152_setPowerful(const bool on) {
  _Daikin152Protocol.Powerful = on;
  if (on) {
    // Powerful, Quiet, Comfort & Econo mode being on are mutually exclusive.
    Daikin152_setQuiet(false);
    Daikin152_setComfort(false);
    Daikin152_setEcono(false);
  }
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin152_getPowerful(void) { return _Daikin152Protocol.Powerful; }

/// Set the Economy mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin152_setEcono(const bool on) {
  _Daikin152Protocol.Econo = on;
  // Powerful & Econo mode being on are mutually exclusive.
  if (on) Daikin152_setPowerful(false);
}

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin152_getEcono(void) { return _Daikin152Protocol.Econo; }

/// Set the Sensor mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin152_setSensor(const bool on) { _Daikin152Protocol.Sensor = on; }

/// Get the Sensor mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin152_getSensor(void) { return _Daikin152Protocol.Sensor; }

/// Set the Comfort mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin152_setComfort(const bool on) {
  _Daikin152Protocol.Comfort = on;
  if (on) {
    // Comfort mode is incompatible with Powerful mode.
    Daikin152_setPowerful(false);
    // It also sets the fan to auto and turns off swingv.
    Daikin152_setFan(kDaikinFanAuto);
    Daikin152_setSwingV(false);
  }
}

/// Get the Comfort mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin152_getComfort(void) { return _Daikin152Protocol.Comfort; }










void encode_Daikin64(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){
  Daikin64_setPowerToggle(ac_status.power_status);
  Daikin64_setTemp(ac_status.temperature);
  Daikin64_setMode(Daikin64_convertMode(ac_status.mode));
  Daikin64_setFan(Daikin64_convertFan(ac_status.fan));
  Daikin64_send(Daikin64_getRaw(), kDaikin64Bits, kDaikin64DefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Daikin64(int16_t* input, uint8_t* output) {

  initDecodeData(input, DAIKIN64_BITS);
  if(Daikin64_recv(&gDecodeResult, 0, kDaikin64Bits, false))
    Daikin64_setRaw(*(uint64_t*)&(gDecodeResult.state[0]));
  else 
    return;
  output[0] = Daikin64_getPowerToggle();
  output[1] = Daikin64_getTemp();
  output[2] = Daikin64_toCommonFanSpeed(Daikin64_getFan());
  output[3] = Daikin64_getSwingVertical();
  output[4] = Daikin64_toCommonMode(Daikin64_getMode());

  setAcStatus(output);
  ac_control_update_status_to_payload();
}

void setAcStatus(uint8_t* output){

  ac_control_set_power_status(output[0]);   // POWER
  ac_control_set_temperature(output[1]);    // TEMPERATURE
  ac_control_set_fan(output[2]);           // FAN
  ac_control_set_swing(output[3]);          // SWING
  ac_control_set_mode(output[4]);           // MODE

  ac_control_update_status_to_payload();
}

/// Send a Daikin64 (64-bit) A/C formatted message.
/// Status: Beta / Probably Working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Daikin64_send(const uint64_t data, const uint16_t nbits, const uint16_t repeat, int16_t *irRaw) {
  uint8_t offset = 0;
  for (uint16_t r = 0; r <= repeat; r++) {
    for (uint8_t i = 0; i < 2; i++) {
      // Leader
      irRaw[offset++] = kDaikin64LdrMark;
      irRaw[offset++] = -kDaikin64LdrSpace;
    }
    // Header + Data + Footer #1
    sendGeneric_64( kDaikin64HdrMark, kDaikin64HdrSpace,
                    kDaikin64BitMark, kDaikin64OneSpace,
                    kDaikin64BitMark, kDaikin64ZeroSpace,
                    kDaikin64BitMark, kDaikin64Gap,
                    0, //message time
                    data, nbits, 
                    kDaikin64Freq, false, 0, 50,
                    &irRaw[offset]);
    offset += (4 + nbits*2);
    // Footer #2
    irRaw[offset++] = kDaikin64HdrMark;
    irRaw[offset++] = -kDefaultMessageGap;
  }
}

/// Decode the supplied Daikin 64-bit message. (DAIKIN64)
/// Status: Beta / Probably Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Daikin64_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kDaikin64Overhead - offset)
    return false;  // Too short a message to match.
  // Compliance
  if (strict && nbits != kDaikin64Bits)
    return false;

  // Leader
  for (uint8_t i = 0; i < 2; i++) {
    if (!matchMark(results->rawbuf[offset++], kDaikin64LdrMark, kTolerance)) return false;
    if (!matchSpace(results->rawbuf[offset++], kDaikin64LdrSpace, kTolerance)) return false;
  }
  // Header + Data + Footer #1
  uint16_t used = matchGeneric_64(results->rawbuf + offset, 
                                  &results->value,
                                  results->rawlen - offset, nbits,
                                  kDaikin64HdrMark, kDaikin64HdrSpace,
                                  kDaikin64BitMark, kDaikin64OneSpace,
                                  kDaikin64BitMark, kDaikin64ZeroSpace,
                                  kDaikin64BitMark, kDaikin64Gap,
                                  false, kTolerance,
                                  kMarkExcess, false);
  if (used == 0) return false;
  offset += used;
  // Footer #2
  if (!matchMark(results->rawbuf[offset++], kDaikin64HdrMark, kTolerance))
    return false;

  // Compliance
  if (strict && ! Daikin64_validChecksum(results->value)) return false;
  // Success
  results->decode_type = DAIKIN64;
  results->bits = nbits;
  results->command = 0;
  results->address = 0;
  return true;
}



/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @return The 4-bit checksum stored in a uint_8.
uint8_t Daikin64_calcChecksum(const uint64_t state) {
  uint64_t data = GETBITS64(state, 0, kDaikin64ChecksumOffset);
  uint8_t result = 0;
  for (; data; data >>= 4)  // Add each nibble together.
    result += GETBITS64(data, 0, 4);
  return result & 0xF;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The state to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Daikin64_validChecksum(const uint64_t state) {
  // Validate the checksum of the given state.
  return (GETBITS64(state, kDaikin64ChecksumOffset, kDaikin64ChecksumSize) == Daikin64_calcChecksum(state));
}

/// Calculate and set the checksum values for the internal state.
void Daikin64_checksum(void) { _Daikin64Protocol.Sum = Daikin64_calcChecksum(_Daikin64Protocol.raw); }

/// Reset the internal state to a fixed known good state.
void Daikin64_stateReset(void) { _Daikin64Protocol.raw = kDaikin64KnownGoodState; }

/// Get a copy of the internal state as a valid code for this protocol.
/// @return A valid code for this protocol based on the current internal state.
uint64_t Daikin64_getRaw(void) {
  Daikin64_checksum();  // Ensure correct settings before sending.
  return _Daikin64Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_state A valid code for this protocol.
void Daikin64_setRaw(const uint64_t new_state) { _Daikin64Protocol.raw = new_state; }

/// Set the Power toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin64_setPowerToggle(const bool on) { 
  if(on){
    _Daikin64Protocol.Power = 0; 
    _Daikin64_savePower = true; // on
  }
  else{
    _Daikin64Protocol.Power = 1; 
    _Daikin64_savePower = false; // off
  }
}

/// Get the Power toggle setting of the A/C.
/// @return The current operating mode setting.
bool Daikin64_getPowerToggle(void) { 
  if(_Daikin64Protocol.Power){
    if(_Daikin64_savePower){
      Daikin64_setPowerToggle(false);
      return false;
    }
    else{
      Daikin64_setPowerToggle(true);
      return true;
    }
  }
  else{
    return true;
  }
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Daikin64_setTemp(const uint8_t temp) {
  uint8_t degrees = MAX(temp + 16, kDaikin64MinTemp);
  degrees = MIN(degrees, kDaikin64MaxTemp);
  _Daikin64Protocol.Temp = uint8ToBcd(degrees);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Daikin64_getTemp(void) { return bcdToUint8(_Daikin64Protocol.Temp) - 16; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Daikin64_getMode(void) { return _Daikin64Protocol.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Daikin64_setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin64Fan:
    case kDaikin64Dry:
    case kDaikin64Cool:
      _Daikin64Protocol.Mode = mode;
      break;
    default:
      _Daikin64Protocol.Mode = kDaikin64Cool;
  }
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin64_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeDry:  return kDaikin64Dry;
    case kOpModeFan:  return kDaikin64Fan;
    case kOpModeHeat: return kDaikin64Heat;
    default:          return kDaikin64Cool;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Daikin64_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin64Cool: return kOpModeCool;
    case kDaikin64Heat: return kOpModeHeat;
    case kDaikin64Dry:  return kOpModeDry;
    case kDaikin64Fan:  return kOpModeFan;
    default: return kOpModeAuto;
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Daikin64_getFan(void) { return _Daikin64Protocol.Fan; }

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Daikin64_setFan(const uint8_t speed) {
  switch (speed) {
    case kDaikin64FanQuiet:
    case kDaikin64FanTurbo:
    case kDaikin64FanAuto:
    case kDaikin64FanHigh:
    case kDaikin64FanMed:
    case kDaikin64FanLow:
      _Daikin64Protocol.Fan = speed;
      break;
    default:
      _Daikin64Protocol.Fan = kDaikin64FanAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Daikin64_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kDaikin64FanLow;
    case kFanSpeedLow:    return kDaikin64FanMed;
    case kFanSpeedMedium: 
    case kFanSpeedHigh:   
    case kFanSpeedMax:    return kDaikin64FanHigh;
    default:              return kDaikin64FanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Daikin64_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kDaikin64FanTurbo: return kFanSpeedMax;
    case kDaikin64FanHigh:  return kFanSpeedHigh;
    case kDaikin64FanMed:   return kFanSpeedMedium;
    case kDaikin64FanLow:   return kFanSpeedLow;
    case kDaikinFanQuiet:   return kFanSpeedMin;
    default:                return kFanSpeedAuto;
  }
}

/// Get the Turbo (Powerful) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin64_getTurbo(void) { return _Daikin64Protocol.Fan == kDaikin64FanTurbo; }

/// Set the Turbo (Powerful) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin64_setTurbo(const bool on) {
  if (on) {
    Daikin64_setFan(kDaikin64FanTurbo);
  } else if (_Daikin64Protocol.Fan == kDaikin64FanTurbo) {
    Daikin64_setFan(kDaikin64FanAuto);
  }
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin64_getQuiet(void) { return _Daikin64Protocol.Fan == kDaikin64FanQuiet; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin64_setQuiet(const bool on) {
  if (on) {
    Daikin64_setFan(kDaikin64FanQuiet);
  } else if (_Daikin64Protocol.Fan == kDaikin64FanQuiet) {
    Daikin64_setFan(kDaikin64FanAuto);
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin64_setSwingVertical(const bool on) { _Daikin64Protocol.SwingV = on; }

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin64_getSwingVertical(void) { return _Daikin64Protocol.SwingV; }

/// Set the Sleep mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin64_setSleep(const bool on) { _Daikin64Protocol.Sleep = on; }

/// Get the Sleep mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Daikin64_getSleep(void) { return _Daikin64Protocol.Sleep; }

/// Set the clock on the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void Daikin64_setClock(const uint16_t mins_since_midnight) {
  uint16_t mins = mins_since_midnight;
  if (mins_since_midnight >= 24 * 60) mins = 0;  // Bounds check.
  _Daikin64Protocol.ClockMins = uint8ToBcd(mins % 60);
  _Daikin64Protocol.ClockHours = uint8ToBcd(mins / 60);
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin64_getClock(void) {
  return bcdToUint8(_Daikin64Protocol.ClockHours) * 60 + bcdToUint8(_Daikin64Protocol.ClockMins);
}

/// Set the enable status of the On Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin64_setOnTimeEnabled(const bool on) { _Daikin64Protocol.OnTimer = on; }

/// Get the enable status of the On Timer.
/// @return true, the setting is on. false, the setting is off.
bool Daikin64_getOnTimeEnabled(void) { return _Daikin64Protocol.OnTimer; }

/// Get the On Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin64_getOnTime(void) { return GETTIME(On); }

/// Set the On Timer time for the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void Daikin64_setOnTime(const uint16_t mins_since_midnight) {
  SETTIME(On, mins_since_midnight);
}

/// Set the enable status of the Off Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void Daikin64_setOffTimeEnabled(const bool on) { _Daikin64Protocol.OffTimer = on; }

/// Get the enable status of the Off Timer.
/// @return true, the setting is on. false, the setting is off.
bool Daikin64_getOffTimeEnabled(void) { return _Daikin64Protocol.OffTimer; }

/// Get the Off Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t Daikin64_getOffTime(void) { return GETTIME(Off); }

/// Set the Off Timer time for the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void Daikin64_setOffTime(const uint16_t mins_since_midnight) {
  SETTIME(Off, mins_since_midnight);
}




