
/// @file
/// @brief Support for Mitsubishi protocols.
/// @see GlobalCache's Control Tower's Mitsubishi TV data.

#include "stdint.h"
#include "string.h"
#include "stdint.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "ir_Mitsubishi144.h"
#include "app_ac_status.h"
#include "IR_Common.h"

Mitsubishi144Protocol_t _Mitsubishi144 = { .raw = {0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x06, 0x30, 0x45, 0x67} };


// Mitsubishi 144 A/C
const uint16_t kMitsubishi144HdrMark         = 3400;
const uint16_t kMitsubishi144HdrSpace        = 1750;
const uint16_t kMitsubishi144BitMark         = 450;
const uint16_t kMitsubishi144OneSpace        = 1300;
const uint16_t kMitsubishi144ZeroSpace       = 420;
const uint16_t kMitsubishi144RptMark         = 440;
const uint16_t kMitsubishi144RptSpace        = 17100;
const uint8_t  kMitsubishi144ExtraTolerance  = 5;


void encode_Mitsubishi144(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  Mitsubishi144_setPower(ac_status.power_status);
  Mitsubishi144_setTemp(ac_status.temperature);
  Mitsubishi144_setMode(Mitsubishi144_convertMode(ac_status.mode));
  Mitsubishi144_setFan(Mitsubishi144_convertFan(ac_status.fan));

  if(ac_status.swing)
    Mitsubishi144_setVane(kMitsubishi144VaneAuto);
  else
    Mitsubishi144_setVane(kMitsubishi144VaneLowest);

  Mitsubishi144_send(OutputIRProtocol);
  setIrTxState(1);
}


void decode_Mitsubishi144(int16_t* input, uint8_t* output) {
  
  bool isDecodeOk = false;
  // copy raw buf, init data
  initDecodeData(input, MITSUBISHI_144_MAX_INDEX + 1);
  
  if( Mitsubishi144_recv(&gDecodeResult, 0, kMitsubishiACBits, false) ){
    Mitsubishi144_setRaw(gDecodeResult.state);
  }

  output[0] = Mitsubishi144_getPower();
  output[1] = Mitsubishi144_getTemp() - kMitsubishi144MinTemp;
  output[2] = Mitsubishi144_toCommonFanSpeed(Mitsubishi144_getFan());
  output[3] = Mitsubishi144_getVane();
  output[4] = Mitsubishi144_toCommonMode(Mitsubishi144_getMode());

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan( output[2]);
  ac_control_set_temperature(output[3] == kMitsubishi144VaneAuto);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}



/// Send a Mitsubishi 144-bit A/C formatted message. (MITSUBISHI_AC)
void Mitsubishi144_send(int16_t *irRaw) {

  sendGeneric_8( kMitsubishi144HdrMark, kMitsubishi144HdrSpace,
                  kMitsubishi144BitMark, kMitsubishi144OneSpace,
                  kMitsubishi144BitMark, kMitsubishi144ZeroSpace,
                  kMitsubishi144RptMark, kMitsubishi144RptSpace,
                  Mitsubishi144_getRaw(),  kMitsubishiACStateLength,
                  38, false, kMitsubishiACMinRepeat, 50,
                  irRaw);
}

/// Decode the supplied Mitsubish 144-bit A/C message.
/// Status: BETA / Probably works
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @see https://www.analysir.com/blog/2015/01/06/reverse-engineering-mitsubishi-ac-infrared-protocol/
bool Mitsubishi144_recv(decode_results *results, uint16_t offset,
                        const uint16_t nbits,
                        const bool strict ) {
  // Compliance
  if (strict && nbits != kMitsubishiACBits) return false;  // Out of spec.
  // Do we need to look for a repeat?
  const uint16_t expected_repeats = strict ? kMitsubishiACMinRepeat : kNoRepeat;
  // Enough data?
  if (results->rawlen <= (nbits * 2 + kHeader + kFooter) *
                         (expected_repeats + 1) + offset - 1) return false;
  uint16_t save[kStateSizeMax];
  // Handle repeats if we need too.
  for (uint16_t r = 0; r <= expected_repeats; r++) {
    // Header + Data + Footer
    uint16_t used = matchGeneric_8(results->rawbuf + offset, results->state,
                                 results->rawlen - offset, nbits,
                                 kMitsubishi144HdrMark, kMitsubishi144HdrSpace,
                                 kMitsubishi144BitMark, kMitsubishi144OneSpace,
                                 kMitsubishi144BitMark, kMitsubishi144ZeroSpace,
                                 kMitsubishi144RptMark, kMitsubishi144RptSpace,
                                 r < expected_repeats,  // At least?
                                 kTolerance + kMitsubishi144ExtraTolerance,
                                 0, false);
    if (!used) return false;  // No match.
    offset += used;
    if (r) {  // Is this a repeat?
      // Repeats are expected to be exactly the same.
      if (memcmp(save, results->state, nbits / 8) != 0) return false;
    } else {  // It is the first message.
      // Compliance
      if (strict) {
        // Data signature check.
        static const uint8_t signature[5] = {0x23, 0xCB, 0x26, 0x01, 0x00};
        if (memcmp(results->state, signature, 5) != 0) return false;
        // Checksum verification.
        if (!Mitsubishi144_validChecksum(results->state)) return false;
      }
      // Save a copy of the state to compare with.
      memcpy(save, results->state, nbits / 8);
    }
  }

  // Success.
  results->decode_type = MITSUBISHI_AC;
  results->bits = nbits;
  return true;
}



/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Mitsubishi144_getRaw(void) {
  Mitsubishi144_checksum();
  return _Mitsubishi144.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] data A valid code for this protocol.
void Mitsubishi144_setRaw(const uint8_t *data) {
  memcpy(_Mitsubishi144.raw, data, kMitsubishiACStateLength);
}

/// Calculate and set the checksum values for the internal state.
void Mitsubishi144_checksum(void) {
  _Mitsubishi144.frame.Sum = Mitsubishi144_calculateChecksum(_Mitsubishi144.raw);
}

/// Verify the checksum is valid for a given state.
/// @param[in] data The array to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
static bool Mitsubishi144_validChecksum(const uint8_t *data) {
  return Mitsubishi144_calculateChecksum(data) == data[kMitsubishiACStateLength - 1];
}

/// Calculate the checksum for a given state.
/// @param[in] data The value to calc the checksum of.
/// @return The calculated checksum value.
static uint8_t Mitsubishi144_calculateChecksum(const uint8_t *data) {
  return sumBytes(data, kMitsubishiACStateLength - 1, 0);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi144_setPower(bool on) {
  _Mitsubishi144.frame.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi144_getPower(void) {
  return _Mitsubishi144.frame.Power;
}

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
/// @note The temperature resolution is 0.5 of a degree.
void Mitsubishi144_setTemp(const float degrees) {
  // Make sure we have desired temp in the correct range.
  float celsius = MAX(degrees + kMitsubishi144MinTemp, kMitsubishi144MinTemp);
  celsius = MIN(celsius, kMitsubishi144MaxTemp);
  // Convert to integer nr. of half degrees.
  uint8_t nrHalfDegrees = celsius * 2;
  // Do we have a half degree celsius?
  _Mitsubishi144.frame.HalfDegree = nrHalfDegrees & 1;
  _Mitsubishi144.frame.Temp = (uint8_t)(nrHalfDegrees / 2 - kMitsubishi144MinTemp);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
/// @note The temperature resolution is 0.5 of a degree.
float Mitsubishi144_getTemp(void) {
  return _Mitsubishi144.frame.Temp + kMitsubishi144MinTemp + (_Mitsubishi144.frame.HalfDegree ? 0.5 : 0);
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting. 0 is auto, 1-5 is speed, 6 is silent.
void Mitsubishi144_setFan(const uint8_t speed) {
  uint8_t fan = speed;
  // Bounds check
  if (fan > kMitsubishi144FanSilent)
    fan = kMitsubishi144FanMax;        // Set the fan to maximum if out of range.
  // Auto has a special bit.
  _Mitsubishi144.frame.FanAuto = (fan == kMitsubishi144FanAuto);
  if (fan >= kMitsubishi144FanMax)
    fan--;  // There is no spoon^H^H^Heed 5 (MAX), pretend it doesn't exist.
  _Mitsubishi144.frame.Fan = fan;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Mitsubishi144_getFan(void) {
  uint8_t fan = _Mitsubishi144.frame.Fan;
  if (fan == kMitsubishi144FanMax) return kMitsubishi144FanSilent;
  return fan;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Mitsubishi144_getMode(void) {
  return _Mitsubishi144.frame.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Mitsubishi144_setMode(const uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  switch (mode) {
    case kMitsubishi144Auto: _Mitsubishi144.raw[8] = 0x30; break;
    case kMitsubishi144Cool: _Mitsubishi144.raw[8] = 0x36; break;
    case kMitsubishi144Dry:  _Mitsubishi144.raw[8] = 0x32; break;
    case kMitsubishi144Heat: _Mitsubishi144.raw[8] = 0x30; break;
    case kMitsubishi144Fan:  _Mitsubishi144.raw[8] = 0x37; break;
    default:
      _Mitsubishi144.raw[8] = 0x30;
      _Mitsubishi144.frame.Mode = kMitsubishi144Auto;
      return;
  }
  _Mitsubishi144.frame.Mode = mode;
}

/// Set the requested vane (Vertical Swing) operation mode of the a/c unit.
/// @note On some models, this represents the Right vertical vane.
/// @param[in] position The position/mode to set the vane to.
void Mitsubishi144_setVane(const uint8_t position) {
  uint8_t pos = MIN(position, kMitsubishi144VaneAutoMove);  // bounds check
  _Mitsubishi144.frame.VaneBit = 1;
  _Mitsubishi144.frame.Vane = pos;
}

/// Set the requested wide-vane (Horizontal Swing) operation mode of the a/c.
/// @param[in] position The position/mode to set the wide vane to.
void Mitsubishi144_setWideVane(const uint8_t position) {
  _Mitsubishi144.frame.WideVane = MIN(position, kMitsubishi144WideVaneAuto);
}

/// Get the Vane (Vertical Swing) mode of the A/C.
/// @note On some models, this represents the Right vertical vane.
/// @return The native position/mode setting.
uint8_t Mitsubishi144_getVane(void) {
  return _Mitsubishi144.frame.Vane;
}

/// Get the Wide Vane (Horizontal Swing) mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi144_getWideVane(void) {
  return _Mitsubishi144.frame.WideVane;
}

/// Set the requested Left Vane (Vertical Swing) operation mode of the a/c unit.
/// @param[in] position The position/mode to set the vane to.
void Mitsubishi144_setVaneLeft(const uint8_t position) {
  _Mitsubishi144.frame.VaneLeft = MIN(position, kMitsubishi144VaneAutoMove);  // bounds check
}

/// Get the Left Vane (Vertical Swing) mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi144_getVaneLeft(void) { return _Mitsubishi144.frame.VaneLeft; }

/// Get the clock time of the A/C unit.
/// @return Nr. of 10 minute increments past midnight.
/// @note 1 = 1/6 hour (10 minutes). e.g. 4pm = 48.
uint8_t Mitsubishi144_getClock(void) { return _Mitsubishi144.frame.Clock; }

/// Set the clock time on the A/C unit.
/// @param[in] clock Nr. of 10 minute increments past midnight.
/// @note 1 = 1/6 hour (10 minutes). e.g. 6am = 36.
void Mitsubishi144_setClock(const uint8_t clock) {
  _Mitsubishi144.frame.Clock = clock;
}

/// Get the desired start time of the A/C unit.
/// @return Nr. of 10 minute increments past midnight.
/// @note 1 = 1/6 hour (10 minutes). e.g. 4pm = 48.
uint8_t Mitsubishi144_getStartClock(void) { return _Mitsubishi144.frame.StartClock; }

/// Set the desired start time of the A/C unit.
/// @param[in] clock Nr. of 10 minute increments past midnight.
/// @note 1 = 1/6 hour (10 minutes). e.g. 8pm = 120.
void Mitsubishi144_setStartClock(const uint8_t clock) {
  _Mitsubishi144.frame.StartClock = clock;
}

/// Get the desired stop time of the A/C unit.
/// @return Nr. of 10 minute increments past midnight.
/// @note 1 = 1/6 hour (10 minutes). e.g. 10pm = 132.
uint8_t Mitsubishi144_getStopClock(void) { return _Mitsubishi144.frame.StopClock; }

/// Set the desired stop time of the A/C unit.
/// @param[in] clock Nr. of 10 minute increments past midnight.
/// @note 1 = 1/6 hour (10 minutes). e.g. 10pm = 132.
void Mitsubishi144_setStopClock(const uint8_t clock) {
  _Mitsubishi144.frame.StopClock = clock;
}

/// Get the timers active setting of the A/C.
/// @return The current timers enabled.
/// @note Possible values: kMitsubishi144NoTimer,
///   kMitsubishi144StartTimer, kMitsubishi144StopTimer,
///   kMitsubishi144StartStopTimer
uint8_t Mitsubishi144_getTimer(void) {
  return _Mitsubishi144.frame.Timer;
}

/// Set the timers active setting of the A/C.
/// @param[in] timer The timer code indicating which ones are active.
/// @note Possible values: kMitsubishi144NoTimer,
///   kMitsubishi144StartTimer, kMitsubishi144StopTimer,
///   kMitsubishi144StartStopTimer
void Mitsubishi144_setTimer(const uint8_t timer) {
  _Mitsubishi144.frame.Timer = timer;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi144_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kMitsubishi144Cool;
    case kOpModeHeat: return kMitsubishi144Heat;
    case kOpModeDry:  return kMitsubishi144Dry;
    case kOpModeFan:  return kMitsubishi144Fan;
    default:          return kMitsubishi144Auto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi144_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kMitsubishi144FanSilent;
    case kFanSpeedLow:    return kMitsubishi144FanRealMax - 3;
    case kFanSpeedMedium: return kMitsubishi144FanRealMax - 2;
    case kFanSpeedHigh:   return kMitsubishi144FanRealMax - 1;
    case kFanSpeedMax:    return kMitsubishi144FanRealMax;
    default:              return kMitsubishi144FanAuto;
  }
}


/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi144_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest: return kMitsubishi144VaneHighest;
    case kSwingVHigh:    return kMitsubishi144VaneHigh;
    case kSwingVMiddle:  return kMitsubishi144VaneMiddle;
    case kSwingVLow:     return kMitsubishi144VaneLow;
    case kSwingVLowest:  return kMitsubishi144VaneLowest;
    // These model Mitsubishi A/C have two automatic settings.
    // 1. A typical up & down oscillation. (Native Swing)
    // 2. The A/C determines where the best placement for the vanes, outside of
    //    user control. (Native Auto)
    // Native "Swing" is what we consider "Auto" in stdAc. (Case 1)
    case kSwingVAuto:    return kMitsubishi144VaneSwing;
    // Native "Auto" doesn't have a good match for this in stdAc. (Case 2)
    // So we repurpose stdAc's "Off" (and anything else) to be Native Auto.
    default:             return kMitsubishi144VaneAuto;
  }
}

/// Convert a swingh_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi144_convertSwingH(const swingh_t position) {
  switch (position) {
    case kSwingHLeftMax:  return kMitsubishi144WideVaneLeftMax;
    case kSwingHLeft:     return kMitsubishi144WideVaneLeft;
    case kSwingHMiddle:   return kMitsubishi144WideVaneMiddle;
    case kSwingHRight:    return kMitsubishi144WideVaneRight;
    case kSwingHRightMax: return kMitsubishi144WideVaneRightMax;
    case kSwingHWide:     return kMitsubishi144WideVaneWide;
    case kSwingHAuto:     return kMitsubishi144WideVaneAuto;
    default:              return kMitsubishi144WideVaneMiddle;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Mitsubishi144_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kMitsubishi144Cool: return kOpModeCool;
    case kMitsubishi144Heat: return kOpModeHeat;
    case kMitsubishi144Dry:  return kOpModeDry;
    case kMitsubishi144Fan:  return kOpModeFan;
    default:                 return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Mitsubishi144_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kMitsubishi144FanRealMax:     return kFanSpeedMax;
    case kMitsubishi144FanRealMax - 1: return kFanSpeedHigh;
    case kMitsubishi144FanRealMax - 2: return kFanSpeedMedium;
    case kMitsubishi144FanRealMax - 3: return kFanSpeedLow;
    case kMitsubishi144FanSilent:      return kFanSpeedMin;
    default:                           return kFanSpeedAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
swingv_t Mitsubishi144_toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kMitsubishi144VaneHighest: return kSwingVHighest;
    case kMitsubishi144VaneHigh:    return kSwingVHigh;
    case kMitsubishi144VaneMiddle:  return kSwingVMiddle;
    case kMitsubishi144VaneLow:     return kSwingVLow;
    case kMitsubishi144VaneLowest:  return kSwingVLowest;
    // These model Mitsubishi A/C have two automatic settings.
    // 1. A typical up & down oscillation. (Native Swing)
    // 2. The A/C determines where the best placement for the vanes, outside of
    //    user control. (Native Auto)
    // Native "Auto" doesn't have a good match for this in stdAc. (Case 2)
    // So we repurpose stdAc's "Off" to be Native Auto.
    case kMitsubishi144VaneAuto:    return kSwingVOff;
    // Native "Swing" is what we consider "Auto" in stdAc. (Case 1)
    default:                       return kSwingVAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common horizontal swing position.
swingh_t Mitsubishi144_toCommonSwingH(const uint8_t pos) {
  switch (pos) {
    case kMitsubishi144WideVaneLeftMax:  return kSwingHLeftMax;
    case kMitsubishi144WideVaneLeft:     return kSwingHLeft;
    case kMitsubishi144WideVaneMiddle:   return kSwingHMiddle;
    case kMitsubishi144WideVaneRight:    return kSwingHRight;
    case kMitsubishi144WideVaneRightMax: return kSwingHRightMax;
    case kMitsubishi144WideVaneWide:     return kSwingHWide;
    default:                            return kSwingHAuto;
  }
}

/// Change the Weekly Timer Enabled setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi144_setWeeklyTimerEnabled(const bool on) {
  _Mitsubishi144.frame.WeeklyTimer = on;
}

/// Get the value of the WeeklyTimer Enabled setting.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi144_getWeeklyTimerEnabled(void) { return _Mitsubishi144.frame.WeeklyTimer; }
