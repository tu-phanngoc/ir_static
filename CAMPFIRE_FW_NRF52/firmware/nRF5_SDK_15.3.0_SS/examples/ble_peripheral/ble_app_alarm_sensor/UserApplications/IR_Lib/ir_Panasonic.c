/// @file
/// @brief Support for Panasonic protocols.
/// Panasonic A/C Clock & Timer support:

#include "string.h"

#include "ir_Panasonic.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"
#include "IRcommon.h"
#include "checksum.h"

#include "app_ac_status.h"


PanasonicAcProtocol_t _PanasonicAcProtocol = { .raw = {
    0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06, 0x02,
    0x20, 0xE0, 0x04, 0x09, 0x00, 0x00, 0x80, 0xAF, 0x0D,
    0x00, 0x0E, 0xE0, 0x00, 0x00, 0x89, 0x00, 0x00, 0x00
}};

// Constants
const uint16_t kPanasonicHdrMark          = 3456;   ///< uSeconds.
const uint16_t kPanasonicHdrSpace         = 1728;   ///< uSeconds.
const uint16_t kPanasonicBitMark          = 432;    ///< uSeconds.
const uint16_t kPanasonicOneSpace         = 1296;   ///< uSeconds.
const uint16_t kPanasonicZeroSpace        = 432;    ///< uSeconds.
const uint32_t kPanasonicMinCommandLength = 163296; ///< uSeconds.
const uint16_t kPanasonicEndGap           = 5000;   ///< uSeconds. See #245
const uint32_t kPanasonicMinGap           = 74736;  ///< uSeconds.

const uint16_t kPanasonicAcSectionGap     = 10000;       ///< uSeconds.
const uint16_t kPanasonicAcSection1Length = 8;
const uint32_t kPanasonicAcMessageGap     = kDefaultMessageGap;  // Just a guess.




void encode_PanasonicAc(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  PanasonicAc_setPower(ac_status.power_status);
  PanasonicAc_setTemp(ac_status.temperature, true);
  PanasonicAc_setMode(PanasonicAc_convertMode(ac_status.mode));
  PanasonicAc_setFan(PanasonicAc_convertFan(ac_status.fan));

  if(ac_status.swing)
    PanasonicAc_setSwingVertical(kPanasonicAcSwingVAuto);
  else
    PanasonicAc_setSwingVertical(kPanasonicAcSwingVLowest);

  PanasonicAc_send(OutputIRProtocol);
  setIrTxState(1);
}


void decode_PanasonicAc(int16_t* input, uint8_t* output) {
  
  // copy raw buf, init data
  initDecodeData(input, PANASONICAC_BITS);
  
  if( PanasonicAc_recv(&gDecodeResult, 0, kPanasonicAcBits, false) ){
    PanasonicAc_setRaw(gDecodeResult.state);
  }

  /* ON/OFF */
  output[0] = PanasonicAc_getPower();
  output[1] = PanasonicAc_getTemp();
  output[2] = PanasonicAc_toCommonFanSpeed(PanasonicAc_getFan());
  output[3] = PanasonicAc_getSwingVertical();
  output[4] = PanasonicAc_toCommonMode(PanasonicAc_getMode());
  
  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3] == kPanasonicAcSwingVAuto);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}



bool isPanasonicAcHeader(int16_t *irRaw){
  decode_results results;
  getRawBuf(irRaw, &results, 132); // For header bitcounts (8 bytes header)
                                  // 132 = kPanasonicAcSection1Length*8 *2 + 4
                                  // data bitcounts + header bitcounts
  if (!matchGeneric_8(results.rawbuf, results.state,
                      results.rawlen, kPanasonicAcSection1Length*8,
                      kPanasonicHdrMark, kPanasonicHdrSpace,
                      kPanasonicBitMark, kPanasonicOneSpace,
                      kPanasonicBitMark, kPanasonicZeroSpace,
                      kPanasonicBitMark, kPanasonicAcSectionGap,
                      true, kTolerance,
                      kMarkExcess, false)) 
                        return false;
  if( results.state[0] != 0x02 ||
      results.state[1] != 0x20 ||
      results.state[2] != 0xE0 ||
      results.state[3] != 0x04 ||
      results.state[7] != 0x06 ) return false;
  return true;
}

/// Decode the supplied Panasonic message.
/// Status: STABLE / Should be working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] manufacturer A 16-bit manufacturer code. e.g. 0x4004 is Panasonic
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @warning Results to be used with `sendPanasonic64()`, not `sendPanasonic()`.
/// @note Panasonic 48-bit protocol is a modified version of Kaseikyo.
bool PanasonicAc_recv(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (strict && nbits != kPanasonicBits)
    return false;  // Request is out of spec.

  uint64_t data = 0;

  const uint8_t sectionSize[2] = {kPanasonicAcSection1Length, kPanasonicAcStateLength - kPanasonicAcSection1Length};

  // Data Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < 2; section++) {
    uint16_t used;
    // Section Data
    used = matchGeneric_8(results->rawbuf + offset, results->state + pos,
                          results->rawlen - offset, sectionSize[section] * 8,
                          kPanasonicHdrMark, kPanasonicHdrSpace,
                          kPanasonicBitMark, kPanasonicOneSpace,
                          kPanasonicBitMark, kPanasonicZeroSpace,
                          kPanasonicBitMark, kPanasonicAcSectionGap,
                          section >= 1,
                          kTolerance, kMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += sectionSize[section];
  }

  if (strict) {
    // Verify the checksum.
    if (PanasonicAc_validChecksum(results->state, kPanasonicAcStateLength)) return false;
  }

  // Success
  results->decode_type = PANASONIC;
  results->bits = nbits;
  return true;
}


/// Send a Panasonic A/C message.
/// Status: STABLE / Work with real device(s).
void PanasonicAc_send(int16_t *irRaw) {

  for (uint16_t r = 0; r <= kPanasonicAcDefaultRepeat; r++) {
    // First section. (8 bytes)
    sendGeneric_8( kPanasonicHdrMark, kPanasonicHdrSpace, 
                    kPanasonicBitMark, kPanasonicOneSpace,
                    kPanasonicBitMark, kPanasonicZeroSpace,
                    kPanasonicBitMark, kPanasonicAcSectionGap,
                    PanasonicAc_getRaw(), kPanasonicAcSection1Length,
                    kPanasonicFreq, false, 0, 50,
                    irRaw);
    // First section. (The rest of the data bytes)
    sendGeneric_8( kPanasonicHdrMark, kPanasonicHdrSpace,
                    kPanasonicBitMark, kPanasonicOneSpace,
                    kPanasonicBitMark, kPanasonicZeroSpace,
                    kPanasonicBitMark, kPanasonicAcMessageGap,
                    PanasonicAc_getRaw() + kPanasonicAcSection1Length, kPanasonicAcStateLength - kPanasonicAcSection1Length,
                    kPanasonicFreq, false, 0, 50, 
                    irRaw + 132); // 132 = kPanasonicAcSection1Length*8 *2 + 4
                                  //       data bitcounts + header bitcounts
  }
}



/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *PanasonicAc_getRaw(void) {
  PanasonicAc_Checksum(kPanasonicAcStateLength);
  return _PanasonicAcProtocol.raw;
}

/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @param[in] length The size/length of the state.
/// @return The calculated checksum value.
uint8_t PanasonicAc_calcChecksum(const uint8_t *state, const uint16_t length) {
  return sumBytes(state, length - 1, kPanasonicAcChecksumInit);
}

/// Calculate and set the checksum values for the internal state.
/// @param[in] length The size/length of the state.
void PanasonicAc_Checksum(const uint16_t length) {
  _PanasonicAcProtocol.raw[length - 1] = PanasonicAc_calcChecksum(_PanasonicAcProtocol.raw, length);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool PanasonicAc_validChecksum(const uint8_t *state, const uint16_t length) {
  if (length < 2) return false;  // 1 byte of data can't have a checksum.
  return (state[length - 1] == sumBytes(state, length - 1, kPanasonicAcChecksumInit));
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] state A valid code for this protocol.
void PanasonicAc_setRaw(const uint8_t state[]) {
  memcpy(_PanasonicAcProtocol.raw, state, kPanasonicAcStateLength);
}

/// Control the power state of the A/C unit.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @warning For CKP models, the remote has no memory of the power state the A/C
///   unit should be in. For those models setting this on/true will toggle the
///   power state of the Panasonic A/C unit with the next message.
///     e.g. If the A/C unit is already on, setPower(true) will turn it off.
///       If the A/C unit is already off, setPower(true) will turn it on.
///       `setPower(false)` will leave the A/C power state as it was.
///   For all other models, setPower(true) should set the internal state to
///   turn it on, and setPower(false) should turn it off.
void PanasonicAc_setPower(const bool on) {
  _PanasonicAcProtocol.Power = on;
}

/// Get the A/C power state of the remote.
/// @return true, the setting is on. false, the setting is off.
/// @warning Except for CKP models, where it returns if the power state will be
///   toggled on the A/C unit when the next message is sent.
bool PanasonicAc_getPower(void) {
  return _PanasonicAcProtocol.Power;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t PanasonicAc_getMode(void) {
  return _PanasonicAcProtocol.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] desired The desired operating mode.
void PanasonicAc_setMode(const uint8_t desired) {
  switch (desired) {
    case kPanasonicAcFan:
      // Allegedly Fan mode has a temperature of 27.
      PanasonicAc_setTemp(kPanasonicAcFanModeTemp, false);
      break;
    case kPanasonicAcAuto:
    case kPanasonicAcCool:
    case kPanasonicAcHeat:
    case kPanasonicAcDry:
      break;
  }
  _PanasonicAcProtocol.Mode = desired;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t PanasonicAc_getTemp(void) {
   return (_PanasonicAcProtocol.Temp - kPanasonicAcFanTempDelta) / 2;
}

/// Set the temperature.
/// @param[in] celsius The temperature in degrees celsius.
/// @param[in] remember: A flag for the class to remember the temperature.
/// @note Automatically safely limits the temp to the operating range supported.
void PanasonicAc_setTemp(const uint8_t celsius, const bool remember) {
  uint8_t temperature;
  temperature = MAX(celsius + kPanasonicAcMinTemp, kPanasonicAcMinTemp);
  temperature = MIN(temperature, kPanasonicAcMaxTemp);
  _PanasonicAcProtocol.Temp = (temperature - kPanasonicAcMinTemp) * 2 + kPanasonicAcFanTempDelta;
}

/// Get the current vertical swing setting.
/// @return The current position it is set to.
uint8_t PanasonicAc_getSwingVertical(void) {
  return _PanasonicAcProtocol.SwingV;
}

/// Control the vertical swing setting.
/// @param[in] desired_elevation The position to set the vertical swing to.
void PanasonicAc_setSwingVertical(const uint8_t desired_elevation) {
  switch (desired_elevation)
  {
  case kPanasonicAcSwingVHighest:
  case kPanasonicAcSwingVHigh:
  case kPanasonicAcSwingVMiddle:
  case kPanasonicAcSwingVLow:
  case kPanasonicAcSwingVLowest: 
    _PanasonicAcProtocol.SwingV = desired_elevation;
    break;
  default: 
    _PanasonicAcProtocol.SwingV = kPanasonicAcSwingVAuto;
    break;
  }
}

/// Get the current horizontal swing setting.
/// @return The current position it is set to.
uint8_t PanasonicAc_getSwingHorizontal(void) {
  return _PanasonicAcProtocol.SwingH;
}

/// Control the horizontal swing setting.
/// @param[in] desired_direction The position to set the horizontal swing to.
void PanasonicAc_setSwingHorizontal(const uint8_t desired_direction) {
  switch (desired_direction) {
    case kPanasonicAcSwingHAuto:
    case kPanasonicAcSwingHMiddle:
    case kPanasonicAcSwingHFullLeft:
    case kPanasonicAcSwingHLeft:
    case kPanasonicAcSwingHRight:
    case kPanasonicAcSwingHFullRight: break;
    default: return;
  }
  _PanasonicAcProtocol.SwingH = desired_direction;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void PanasonicAc_setFan(const uint8_t speed) {
  switch (speed) {
    case kPanasonicAcFanMin:
    case kPanasonicAcFanLow:
    case kPanasonicAcFanMed:
    case kPanasonicAcFanHigh:
    case kPanasonicAcFanMax:
    case kPanasonicAcFanAuto:
      _PanasonicAcProtocol.Fan = speed;
      break;
    default: PanasonicAc_setFan(kPanasonicAcFanAuto);
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t PanasonicAc_getFan(void) {
  return _PanasonicAcProtocol.Fan;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t PanasonicAc_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kPanasonicAcCool;
    case kOpModeHeat: return kPanasonicAcHeat;
    case kOpModeDry:  return kPanasonicAcDry;
    case kOpModeFan:  return kPanasonicAcFan;
    default:          return kPanasonicAcAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t PanasonicAc_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kPanasonicAcFanMin;
    case kFanSpeedLow:    return kPanasonicAcFanLow;
    case kFanSpeedMedium: return kPanasonicAcFanMed;
    case kFanSpeedHigh:   return kPanasonicAcFanHigh;
    case kFanSpeedMax:    return kPanasonicAcFanMax;
    default:              return kPanasonicAcFanAuto;
  }
}

/// Convert a standard A/C vertical swing into its native setting.
/// @param[in] position A swingv_t position to convert.
/// @return The equivalent native horizontal swing position.
uint8_t PanasonicAc_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest:
    case kSwingVHigh:
    case kSwingVMiddle:
    case kSwingVLow:
    case kSwingVLowest: return (uint8_t)position;
    default:            return kPanasonicAcSwingVAuto;
  }
}

/// Convert a standard A/C horizontal swing into its native setting.
/// @param[in] position A swingh_t position to convert.
/// @return The equivalent native horizontal swing position.
uint8_t PanasonicAc_convertSwingH(const swingh_t position) {
  switch (position) {
    case kSwingHLeftMax:  return kPanasonicAcSwingHFullLeft;
    case kSwingHLeft:     return kPanasonicAcSwingHLeft;
    case kSwingHMiddle:   return kPanasonicAcSwingHMiddle;
    case kSwingHRight:    return kPanasonicAcSwingHRight;
    case kSwingHRightMax: return kPanasonicAcSwingHFullRight;
    default:                         return kPanasonicAcSwingHAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t PanasonicAc_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kPanasonicAcCool: return kOpModeCool;
    case kPanasonicAcHeat: return kOpModeHeat;
    case kPanasonicAcDry:  return kOpModeDry;
    case kPanasonicAcFan:  return kOpModeFan;
    default:               return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t PanasonicAc_toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kPanasonicAcFanMax:     return kFanSpeedMax;
    case kPanasonicAcFanHigh:    return kFanSpeedHigh;
    case kPanasonicAcFanMed:     return kFanSpeedMedium;
    case kPanasonicAcFanLow:     return kFanSpeedLow;
    case kPanasonicAcFanMin:     return kFanSpeedMin;
    default:                     return kFanSpeedAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common horizontal swing position.
swingh_t PanasonicAc_toCommonSwingH(const uint8_t pos) {
  switch (pos) {
    case kPanasonicAcSwingHFullLeft:  return kSwingHLeftMax;
    case kPanasonicAcSwingHLeft:      return kSwingHLeft;
    case kPanasonicAcSwingHMiddle:    return kSwingHMiddle;
    case kPanasonicAcSwingHRight:     return kSwingHRight;
    case kPanasonicAcSwingHFullRight: return kSwingHRightMax;
    default:                          return kSwingHAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
swingv_t PanasonicAc_toCommonSwingV(const uint8_t pos) {
  if (pos >= kPanasonicAcSwingVHighest && pos <= kPanasonicAcSwingVLowest)
    return (swingv_t)pos;
  else
    return kSwingVAuto;
}



