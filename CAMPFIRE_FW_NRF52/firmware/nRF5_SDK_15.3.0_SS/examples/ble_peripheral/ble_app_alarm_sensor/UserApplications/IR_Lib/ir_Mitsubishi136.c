
/// @file
/// @brief Support for Mitsubishi protocols.
/// Mitsubishi (TV) sending & Mitsubishi A/C support added by David Conran
/// @see GlobalCache's Control Tower's Mitsubishi TV data.

#include "stdint.h"
#include "string.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "ir_Mitsubishi136.h"
#include "ir_Mitsubishi144.h"
#include "app_ac_status.h"
#include "IR_Common.h"

Mitsubishi136Protocol_t _Mitsubishi136 = { .raw = {0x23, 0xCB, 0x26, 0x21, 0x00, 0x40, 0xC2, 0xC7, 0x04} };


// Mitsubishi 136 bit A/C
const uint16_t kMitsubishi136HdrMark    = 3324;
const uint16_t kMitsubishi136HdrSpace   = 1474;
const uint16_t kMitsubishi136BitMark    = 467;
const uint16_t kMitsubishi136OneSpace   = 1137;
const uint16_t kMitsubishi136ZeroSpace  = 351;
const uint32_t kMitsubishi136Gap        = kDefaultMessageGap;


void encode_Mitsubishi136(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  Mitsubishi136_setPower(ac_status.power_status);
  Mitsubishi136_setTemp(ac_status.temperature);
  Mitsubishi136_setMode(Mitsubishi136_convertMode(ac_status.mode));
  Mitsubishi136_setFan(Mitsubishi136_convertFan(ac_status.fan));

  if(ac_status.swing)
    Mitsubishi136_setSwingV(kMitsubishi136SwingVAuto);
  else
    Mitsubishi136_setSwingV(kMitsubishi136SwingVLowest);

  Mitsubishi136_send(OutputIRProtocol);
  setIrTxState(1);
}


void decode_Mitsubishi136(int16_t* input, uint8_t* output) {
  
  // copy raw buf, init data
  initDecodeData(input, MITSUBISHI_136_MAX_INDEX + 1);
  
  if( Mitsubishi136_recv(&gDecodeResult, 0, kMitsubishi136Bits, false) ){
    Mitsubishi136_setRaw(gDecodeResult.state);
  }

  output[0] = Mitsubishi136_getPower();
  output[1] = Mitsubishi136_getTemp();
  output[2] = Mitsubishi136_toCommonFanSpeed(Mitsubishi136_getFan());
  output[3] = Mitsubishi136_getSwingV();
  output[4] = Mitsubishi136_toCommonMode(Mitsubishi136_getMode());

  ac_control_set_power_status(Mitsubishi136_getPower());
  ac_control_set_temperature(Mitsubishi136_getTemp());
  ac_control_set_fan( output[2]);
  ac_control_set_swing(output[3] == kMitsubishi136SwingVAuto);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}



/// Send a Mitsubishi 136-bit A/C message. (MITSUBISHI136)
/// Status: BETA / Probably working. Needs to be tested against a real device.
void Mitsubishi136_send(int16_t *irRaw) {

  sendGeneric_8( kMitsubishi136HdrMark,  kMitsubishi136HdrSpace,
                  kMitsubishi136BitMark,  kMitsubishi136OneSpace,
                  kMitsubishi136BitMark,  kMitsubishi136ZeroSpace,
                  kMitsubishi136BitMark,  kMitsubishi136Gap,
                  Mitsubishi136_getRaw(), kMitsubishi136StateLength, 
                  38, false, 0, 50,
                  irRaw);
}

/// Decode the supplied Mitsubishi 136-bit A/C message. (MITSUBISHI136)
/// Status: STABLE / Reported as working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
bool Mitsubishi136_recv(decode_results *results, uint16_t offset,
                                 const uint16_t nbits,
                                 const bool strict) {
  if (nbits % 8 != 0) return false;  // Not a multiple of an 8 bit byte.
  if (strict) {  // Do checks to see if it matches the spec.
    if (nbits != kMitsubishi136Bits) return false;
  }
  uint16_t used = matchGeneric_8( results->rawbuf + offset, results->state,
                                  results->rawlen - offset, nbits,
                                  kMitsubishi136HdrMark, kMitsubishi136HdrSpace,
                                  kMitsubishi136BitMark, kMitsubishi136OneSpace,
                                  kMitsubishi136BitMark, kMitsubishi136ZeroSpace,
                                  kMitsubishi136BitMark, kMitsubishi136Gap,
                                  true, kTolerance, 0, false);
  if (!used) return false;
  if (strict) {
    // Header validation: Codes start with 0x23CB26
    if (results->state[0] != 0x23 || results->state[1] != 0xCB ||
        results->state[2] != 0x26) return false;
    if (!Mitsubishi136_validChecksum(results->state, kMitsubishi136StateLength))
      return false;
  }
  results->decode_type = MITSUBISHI136;
  results->bits = nbits;
  return true;
}


// Code to emulate Mitsubishi 136bit A/C IR remote control unit.


/// Calculate the checksum for the current internal state of the remote.
void Mitsubishi136_checksum(void) {
  for (uint8_t i = 0; i < 6; i++)
    _Mitsubishi136.raw[kMitsubishi136PowerByte + 6 + i] =
        ~_Mitsubishi136.raw[kMitsubishi136PowerByte + i];
}

/// Verify the checksum is valid for a given state.
/// @param[in] data The array to verify the checksum of.
/// @param[in] len The length of the data array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Mitsubishi136_validChecksum(const uint8_t *data, const uint16_t len) {
  if (len < kMitsubishi136StateLength) return false;
  const uint16_t half = (len - kMitsubishi136PowerByte) / 2;
  for (uint8_t i = 0; i < half; i++) {
    // This variable is needed to avoid the warning: (known compiler issue)
    // warning: comparison of promoted ~unsigned with unsigned [-Wsign-compare]
    const uint8_t inverted = ~data[kMitsubishi136PowerByte + half + i];
    if (data[kMitsubishi136PowerByte + i] != inverted) return false;
  }
  return true;
}


/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Mitsubishi136_getRaw(void) {
  Mitsubishi136_checksum();
  return _Mitsubishi136.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] data A valid code for this protocol.
void Mitsubishi136_setRaw(const uint8_t *data) {
  memcpy(_Mitsubishi136.raw, data, kMitsubishi136StateLength);
}


/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi136_setPower(bool on) {
  _Mitsubishi136.frame.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi136_getPower(void) {
  return _Mitsubishi136.frame.Power;
}

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
void Mitsubishi136_setTemp(const uint8_t degrees) {
  uint8_t temp;
  if(degrees)
    temp = MAX((uint8_t)kMitsubishi136MinTemp, degrees - 1 + kMitsubishi136MinTemp);
  else
    temp = kMitsubishi136MinTemp;

  temp = MIN((uint8_t)kMitsubishi136MaxTemp, temp);
  _Mitsubishi136.frame.Temp = temp - kMitsubishi136MinTemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Mitsubishi136_getTemp(void) {
  return _Mitsubishi136.frame.Temp;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Mitsubishi136_setFan(const uint8_t speed) {
  _Mitsubishi136.frame.Fan = MIN(speed, kMitsubishi136FanMax);
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Mitsubishi136_getFan(void) {
  return _Mitsubishi136.frame.Fan;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Mitsubishi136_getMode(void) {
  return _Mitsubishi136.frame.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Mitsubishi136_setMode(const uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  switch (mode) {
    case kMitsubishi136Fan:
    case kMitsubishi136Cool:
    case kMitsubishi136Heat:
    case kMitsubishi136Auto:
    case kMitsubishi136Dry:
      _Mitsubishi136.frame.Mode = mode;
      break;
    default:
      _Mitsubishi136.frame.Mode = kMitsubishi136Auto;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void Mitsubishi136_setSwingV(const uint8_t position) {
  // If we get an unexpected mode, default to auto.
  switch (position) {
    case kMitsubishi136SwingVLowest:
    case kMitsubishi136SwingVLow:
    case kMitsubishi136SwingVHigh:
    case kMitsubishi136SwingVHighest:
    case kMitsubishi136SwingVAuto:
      _Mitsubishi136.frame.SwingV = position;
      break;
    default:
      _Mitsubishi136.frame.SwingV = kMitsubishi136SwingVAuto;
  }
}

/// Get the Vertical Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi136_getSwingV(void) {
  return _Mitsubishi136.frame.SwingV;
}

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi136_setQuiet(bool on) {
  if (on) 
    Mitsubishi136_setFan(kMitsubishi136FanQuiet);
  else if (Mitsubishi136_getQuiet())
    Mitsubishi136_setFan(kMitsubishi136FanLow);
}


/// Get the Quiet mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi136_getQuiet(void) {
  return _Mitsubishi136.frame.Fan == kMitsubishi136FanQuiet;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi136_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kMitsubishi136Cool;
    case kOpModeHeat: return kMitsubishi136Heat;
    case kOpModeDry:  return kMitsubishi136Dry;
    case kOpModeFan:  return kMitsubishi136Fan;
    default:                     return kMitsubishi136Auto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi136_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin: return kMitsubishi136FanMin;
    case kFanSpeedLow: return kMitsubishi136FanLow;
    case kFanSpeedHigh:
    case kFanSpeedMax: return kMitsubishi136FanMax;
    default:           return kMitsubishi136FanMed;
  }
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi136_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest: return kMitsubishi136SwingVHighest;
    case kSwingVHigh:
    case kSwingVMiddle:  return kMitsubishi136SwingVHigh;
    case kSwingVLow:     return kMitsubishi136SwingVLow;
    case kSwingVLowest:  return kMitsubishi136SwingVLowest;
    default:             return kMitsubishi136SwingVAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Mitsubishi136_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kMitsubishi136Cool: return kOpModeCool;
    case kMitsubishi136Heat: return kOpModeHeat;
    case kMitsubishi136Dry:  return kOpModeDry;
    case kMitsubishi136Fan:  return kOpModeFan;
    default:                 return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Mitsubishi136_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kMitsubishi136FanMax: return kFanSpeedMax;
    case kMitsubishi136FanMed: return kFanSpeedMedium;
    case kMitsubishi136FanLow: return kFanSpeedLow;
    case kMitsubishi136FanMin: return kFanSpeedMin;
    default:                   return kFanSpeedMedium;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
swingv_t Mitsubishi136_toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kMitsubishi136SwingVHighest: return kSwingVHighest;
    case kMitsubishi136SwingVHigh:    return kSwingVHigh;
    case kMitsubishi136SwingVLow:     return kSwingVLow;
    case kMitsubishi136SwingVLowest:  return kSwingVLowest;
    default:                          return kSwingVAuto;
  }
}

