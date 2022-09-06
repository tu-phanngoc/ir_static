// Copyright 2009 Ken Shirriff
// Copyright 2017, 2019 David Conran

/// @file
/// @brief Support for Sharp protocols.

#include <string.h>
#include <stdint.h>

#include "ir_Sharp.h"
#include "IRcommon.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "app_ac_status.h"
#include "IR_Common.h"

SharpProtocol _SharpProtocol = { .raw = { 0xAA, 0x5A, 0xCF, 0x10, 0x00, 0x01, 0x00, 0x00, 0x08, 0x80, 0x00, 0xE0, 0x01 } };
static uint8_t _temp;  ///< Saved copy of the desired temp.
static uint8_t _mode;  ///< Saved copy of the desired mode.
static uint8_t _fan;  ///< Saved copy of the desired fan speed.
static bool _power = false;
sharp_ac_remote_model_t _model;  ///< Saved copy of the model.


// Constants
// period time = 1/38000Hz = 26.316 microseconds.
const uint16_t kSharpTick           = 26;
const uint16_t kSharpBitMarkTicks   = 10;
const uint16_t kSharpBitMark        = kSharpBitMarkTicks * kSharpTick;
const uint16_t kSharpOneSpaceTicks  = 70;
const uint16_t kSharpOneSpace       = kSharpOneSpaceTicks * kSharpTick;
const uint16_t kSharpZeroSpaceTicks = 30;
const uint16_t kSharpZeroSpace      = kSharpZeroSpaceTicks * kSharpTick;
const uint16_t kSharpGapTicks       = 1677;
const uint16_t kSharpGap            = kSharpGapTicks * kSharpTick;
// Address(5) + Command(8) + Expansion(1) + Check(1)
const uint64_t kSharpToggleMask  = ((uint64_t)1 << (kSharpBits - kSharpAddressBits)) - 1;
const uint64_t kSharpAddressMask = ((uint64_t)1 << kSharpAddressBits) - 1;
const uint64_t kSharpCommandMask = ((uint64_t)1 << kSharpCommandBits) - 1;




void encode_Sharp(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  Sharp_setPower(ac_status.power_status, _power);
  Sharp_setTemp(ac_status.temperature, true);
  Sharp_setMode(Sharp_convertMode(ac_status.mode), true);
  Sharp_setFan(Sharp_convertFan(ac_status.fan, Sharp_getModel(false)), true);
  if(ac_status.swing)
    Sharp_setSwingV(kSharpAcSwingVToggle, true);
  else
    Sharp_setSwingV(kSharpAcSwingVOff, true);

  Sharp_send(Sharp_getRaw(), kSharpAcStateLength, kSharpAcDefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Sharp(int16_t* input, uint8_t* output) {
  // copy raw buf, init data
  initDecodeData(input, SHARP_BITS);
  
  if( Sharp_recv(&gDecodeResult, 0, kSharpAcBits, false) ){
    Sharp_setRaw(gDecodeResult.state, kSharpAcStateLength);
  }

  output[0] = Sharp_getPower();
  output[1] = Sharp_getTemp();
  output[2] = Sharp_toCommonFanSpeed(Sharp_getFan());
  output[3] = Sharp_getSwingV();
  output[4] = Sharp_toCommonMode(Sharp_getMode());

  ac_control_set_power_status(Sharp_getPower());
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3] == kSharpAcSwingVToggle);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}




/// Send a Sharp A/C message.
/// Status: Alpha / Untested.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Sharp_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw) {
  if (nbytes < kSharpAcStateLength)
    return;  // Not enough bytes to send a proper message.

  sendGeneric_8( kSharpAcHdrMark, kSharpAcHdrSpace,
                  kSharpAcBitMark, kSharpAcOneSpace,
                  kSharpAcBitMark, kSharpAcZeroSpace,
                  kSharpAcBitMark, kSharpAcGap,
                  data, nbytes, 38000, false, repeat, 50,
                  irRaw);
}

/// Decode the supplied Sharp A/C message.
/// Status: STABLE / Known working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool Sharp_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  // Compliance
  if (strict && nbits != kSharpAcBits) return false;

  // Match Header + Data + Footer
  uint16_t used;
  used = matchGeneric_8(results->rawbuf + offset, results->state,
                        results->rawlen - offset, nbits,
                        kSharpAcHdrMark, kSharpAcHdrSpace,
                        kSharpAcBitMark, kSharpAcOneSpace,
                        kSharpAcBitMark, kSharpAcZeroSpace,
                        kSharpAcBitMark, kSharpAcGap,
                        true,            kTolerance,
                        kMarkExcess,     false);
  if (used == 0) return false;
  offset += used;
  // Compliance
  if (strict) {
    if (!Sharp_validChecksum(results->state, kSharpAcStateLength))
      return false;
  }

  // Success
  results->decode_type = SHARP_AC;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


/// Calculate the checksum for a given state.
/// @param[in] state The array to calc the checksum of.
/// @param[in] length The length/size of the array.
/// @return The calculated 4-bit checksum value.
uint8_t Sharp_calcChecksum(uint8_t state[], const uint16_t length) {
  uint8_t xorsum = xorBytes(state, length - 1, 0);
  xorsum ^= GETBITS8(state[length - 1], kLowNibble, kNibbleSize);
  xorsum ^= GETBITS8(xorsum, kHighNibble, kNibbleSize);
  return GETBITS8(xorsum, kLowNibble, kNibbleSize);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Sharp_validChecksum(uint8_t state[], const uint16_t length) {
  return GETBITS8(state[length - 1], kHighNibble, kNibbleSize) == Sharp_calcChecksum(state, length);
}

/// Calculate and set the checksum values for the internal state.
void Sharp_checksum(void) {
  _SharpProtocol.Sum = Sharp_calcChecksum(_SharpProtocol.raw, kSharpAcStateLength);
}


/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Sharp_getRaw(void) {
  Sharp_checksum();  // Ensure correct settings before sending.
  return _SharpProtocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] length The length/size of the new_code array.
void Sharp_setRaw(const uint8_t new_code[], const uint16_t length) {
  memcpy(_SharpProtocol.raw, new_code, MIN(length, kSharpAcStateLength));
  _model = Sharp_getModel(true);
}

/// Set the model of the A/C to emulate.
/// @param[in] model The enum of the appropriate model.
void Sharp_setModel(const sharp_ac_remote_model_t model) {
  switch (model) {
    case A705:
    case A903:
      _model = model;
      _SharpProtocol.Model = true;
      break;
    default:
      _model = A907;
      _SharpProtocol.Model = false;
  }
  _SharpProtocol.Model2 = (_model != A907);
  // Redo the operating mode as some models don't support all modes.
  Sharp_setMode(_SharpProtocol.Mode, true);
}

/// Get/Detect the model of the A/C.
/// @param[in] raw Try to determine the model from the raw code only.
/// @return The enum of the compatible model.
sharp_ac_remote_model_t Sharp_getModel(const bool raw) {
  if (raw) {
    if (_SharpProtocol.Model2) {
      if (_SharpProtocol.Model)
        return A705;
      else
        return A903;
    } else {
      return A907;
    }
  }
  return _model;
}

/// Set the value of the Power Special setting without any checks.
/// @param[in] value The value to set Power Special to.
inline void Sharp_setPowerSpecial(const uint8_t value) {
  _SharpProtocol.PowerSpecial = value;
}

/// Get the value of the Power Special setting.
/// @return The setting's value.
uint8_t Sharp_getPowerSpecial(void) {
  return _SharpProtocol.PowerSpecial;
}

/// Clear the "special"/non-normal bits in the power section.
/// e.g. for normal/common command modes.
void Sharp_clearPowerSpecial(void) {
  Sharp_setPowerSpecial(_SharpProtocol.PowerSpecial & kSharpAcPowerOn);
}

/// Is one of the special power states in use?
/// @return true, it is. false, it isn't.
bool Sharp_isPowerSpecial(void) {
  switch (_SharpProtocol.PowerSpecial) {
    case kSharpAcPowerSetSpecialOff:
    case kSharpAcPowerSetSpecialOn:
    case kSharpAcPowerTimerSetting: return true;
    default: return false;
  }
}

/// Change the power setting, including the previous power state.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @param[in] prev_on true, the setting is on. false, the setting is off.
void Sharp_setPower(const bool on, const bool prev_on) {
  Sharp_setPowerSpecial(on ? (prev_on ? kSharpAcPowerOn : kSharpAcPowerOnFromOff)
                     : kSharpAcPowerOff);
  // Power operations are incompatible with clean mode.
  if (_SharpProtocol.Clean) Sharp_setClean(false);
  _SharpProtocol.Special = kSharpAcSpecialPower;
  _power = Sharp_getPower();
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Sharp_getPower(void) {
  switch (_SharpProtocol.PowerSpecial) {
    case kSharpAcPowerUnknown:
    case kSharpAcPowerOff: return false;
    default: return true;  // Everything else is "probably" on.
  }
}

/// Set the value of the Special (button/command?) setting.
/// @param[in] mode The value to set Special to.
void Sharp_setSpecial(const uint8_t mode) {
  switch (mode) {
    case kSharpAcSpecialPower:
    case kSharpAcSpecialTurbo:
    case kSharpAcSpecialTempEcono:
    case kSharpAcSpecialFan:
    case kSharpAcSpecialSwing:
    case kSharpAcSpecialTimer:
    case kSharpAcSpecialTimerHalfHour:
      _SharpProtocol.Special = mode;
      break;
    default:
      _SharpProtocol.Special = kSharpAcSpecialPower;
  }
}

/// Get the value of the Special (button/command?) setting.
/// @return The setting's value.
uint8_t Sharp_getSpecial(void) { return _SharpProtocol.Special; }

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
/// @param[in] save Do we save this setting as a user set one?
void Sharp_setTemp(const uint8_t temp, const bool save) {

  switch (_SharpProtocol.Mode) {
    // Auto & Dry don't allow temp changes and have a special temp.
    case kSharpAcAuto:
    case kSharpAcDry:
      _SharpProtocol.raw[kSharpAcByteTemp] = 0;
      return;
    default:
      switch (Sharp_getModel(true)) {
        case A705:
          _SharpProtocol.raw[kSharpAcByteTemp] = 0xD0;
          break;
        case A907:
          _SharpProtocol.raw[kSharpAcByteTemp] = 0xC0;
          break;
        default: //A903
          _SharpProtocol.raw[kSharpAcByteTemp] = 0x00;
          break;
      }
  }

  uint8_t degrees = MAX(temp + 1, kSharpAcMinTemp);
  degrees = MIN(degrees, kSharpAcMaxTemp);
  if (save) _temp = degrees;
  _SharpProtocol.Temp = degrees;
  _SharpProtocol.Special = kSharpAcSpecialTempEcono;
  Sharp_clearPowerSpecial();
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Sharp_getTemp(void) {
  uint8_t tempDelta = 0;
  switch (_SharpProtocol.Mode) {
    // Auto & Dry don't allow temp changes and have a special temp.
    case kSharpAcAuto:
    case kSharpAcDry:
      return 0;
    default:
      switch (Sharp_getModel(true))
      {
        case A705:
          return _SharpProtocol.Temp + 1;
        case A907:
          return _SharpProtocol.Temp - 1;
        default:
          return _SharpProtocol.Temp + 1 ;
      }
  }
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Sharp_getMode(void) {
  return _SharpProtocol.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @param[in] save Do we save this setting as a user set one?
void Sharp_setMode(const uint8_t mode, const bool save) {
  uint8_t realMode = mode;
  if (mode == kSharpAcHeat) {
    switch (Sharp_getModel(true)) {
      case A705:
      case A903:
        // These models have no heat mode, use Fan mode instead.
        realMode = kSharpAcFan;
        break;
      default:
        break;
    }
  }

  switch (realMode) {
    case kSharpAcAuto:  // Also kSharpAcFan
    case kSharpAcDry:
      // When Dry or Auto, Fan always 2(Auto)
      Sharp_setFan(kSharpAcFanAuto, false);
      // FALLTHRU
    case kSharpAcCool:
    case kSharpAcHeat:
      _SharpProtocol.Mode = realMode;
      break;
    default:
      Sharp_setFan(kSharpAcFanAuto, false);
      _SharpProtocol.Mode = kSharpAcAuto;
  }
  // Dry/Auto have no temp setting. This step will enforce it.
  Sharp_setTemp(_temp, false);
  // Save the mode in case we need to revert to it. eg. Clean
  if (save) _mode = _SharpProtocol.Mode;

  _SharpProtocol.Special = kSharpAcSpecialPower;
  Sharp_clearPowerSpecial();
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
/// @param[in] save Do we save this setting as a user set one?
void Sharp_setFan(const uint8_t speed, const bool save) {
  switch (speed) {
    case kSharpAcFanAuto:
    case kSharpAcFanMin:
    case kSharpAcFanMed:
    case kSharpAcFanHigh:
    case kSharpAcFanMax:
      _SharpProtocol.Fan = speed;
      if (save) _fan = speed;
      break;
    default:
      _SharpProtocol.Fan = kSharpAcFanAuto;
      _fan = kSharpAcFanAuto;
  }
  _SharpProtocol.Special = kSharpAcSpecialFan;
  Sharp_clearPowerSpecial();
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Sharp_getFan(void) {
  return _SharpProtocol.Fan;
}

/// Get the Turbo setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Sharp_getTurbo(void) {
  return (_SharpProtocol.PowerSpecial == kSharpAcPowerSetSpecialOn) &&
         (_SharpProtocol.Special == kSharpAcSpecialTurbo);
}

/// Set the Turbo setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @note If you use this method, you will need to send it before making
///   other changes to the settings, as they may overwrite some of the bits
///   used by this setting.
void Sharp_setTurbo(const bool on) {
  if (on) Sharp_setFan(kSharpAcFanMax, true);
  Sharp_setPowerSpecial(on ? kSharpAcPowerSetSpecialOn : kSharpAcPowerSetSpecialOff);
  _SharpProtocol.Special = kSharpAcSpecialTurbo;
}

/// Get the Vertical Swing setting of the A/C.
/// @return The position of the Vertical Swing setting.
uint8_t Sharp_getSwingV(void) { return _SharpProtocol.Swing; }

/// Set the Vertical Swing setting of the A/C.
/// @note Some positions may not work on all models.
/// @param[in] position The desired position/setting.
/// @note `setSwingV(kSharpAcSwingVLowest)` will only allow the Lowest setting
/// in Heat mode, it will default to `kSharpAcSwingVLow` otherwise.
/// If you want to set this value in other modes e.g. Cool, you must
/// use `setSwingV`s optional `force` parameter.
/// @param[in] force Do we override the safety checks and just do it?
void Sharp_setSwingV(const uint8_t position, const bool force) {
  switch (position) {
    case kSharpAcSwingVCoanda:
      // Only allowed in Heat mode.
      if (!force && Sharp_getMode() != kSharpAcHeat) {
        Sharp_setSwingV(kSharpAcSwingVLow, false);  // Use the next lowest setting.
        return;
      }
      // FALLTHRU
    case kSharpAcSwingVHigh:
    case kSharpAcSwingVMid:
    case kSharpAcSwingVLow:
    case kSharpAcSwingVToggle:
    case kSharpAcSwingVOff:
    case kSharpAcSwingVLast:  // Technically valid, but we don't use it.
      // All expected non-positions set the special bits.
      _SharpProtocol.Special = kSharpAcSpecialSwing;
      // FALLTHRU
    case kSharpAcSwingVIgnore:
      _SharpProtocol.Swing = position;
  }
}

/// Convert a standard A/C vertical swing into its native setting.
/// @param[in] position A swingv_t position to convert.
/// @return The equivalent native horizontal swing position.
uint8_t Sharp_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest:
    case kSwingVHigh:    return kSharpAcSwingVHigh;
    case kSwingVMiddle:  return kSharpAcSwingVMid;
    case kSwingVLow:     return kSharpAcSwingVLow;
    case kSwingVLowest:  return kSharpAcSwingVCoanda;
    case kSwingVAuto:    return kSharpAcSwingVToggle;
    case kSwingVOff:     return kSharpAcSwingVOff;
    default:                 return kSharpAcSwingVIgnore;
  }
}

/// Get the (vertical) Swing Toggle setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Sharp_getSwingToggle(void) {
  return Sharp_getSwingV() == kSharpAcSwingVToggle;
}

/// Set the (vertical) Swing Toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Sharp_setSwingToggle(const bool on) {
  Sharp_setSwingV(on ? kSharpAcSwingVToggle : kSharpAcSwingVIgnore, false);
  if (on) _SharpProtocol.Special = kSharpAcSpecialSwing;
}

/// Get the Ion (Filter) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Sharp_getIon(void) { return _SharpProtocol.Ion; }

/// Set the Ion (Filter) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Sharp_setIon(const bool on) {
  _SharpProtocol.Ion = on;
  Sharp_clearPowerSpecial();
  if (on) _SharpProtocol.Special = kSharpAcSpecialSwing;
}

/// Get the Economical mode toggle setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
/// @note Shares the same location as the Light setting on A705.
static bool getEconoToggle(void) {
  return (_SharpProtocol.PowerSpecial == kSharpAcPowerSetSpecialOn) &&
         (_SharpProtocol.Special == kSharpAcSpecialTempEcono);
}

/// Set the Economical mode toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @warning Probably incompatible with `setTurbo()`
/// @note Shares the same location as the Light setting on A705.
static void setEconoToggle(const bool on) {
  if (on) _SharpProtocol.Special = kSharpAcSpecialTempEcono;
  Sharp_setPowerSpecial(on ? kSharpAcPowerSetSpecialOn : kSharpAcPowerSetSpecialOff);
}

/// Set the Economical mode toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @warning Probably incompatible with `setTurbo()`
/// @note Available on the A907 models.
void Sharp_setEconoToggle(const bool on) {
  if (_model == A907) setEconoToggle(on);
}

/// Get the Economical mode toggle setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
/// @note Available on the A907 models.
bool Sharp_getEconoToggle(void) {
  return _model == A907 && getEconoToggle();
}

/// Set the Light mode toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @warning Probably incompatible with `setTurbo()`
/// @note Not available on the A907 model.
void Sharp_setLightToggle(const bool on) {
  if (_model != A907) Sharp_setEconoToggle(on);
}

/// Get the Light toggle setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
/// @note Not available on the A907 model.
bool Sharp_getLightToggle(void) {
  return _model != A907 && Sharp_getEconoToggle();
}

/// Get how long the timer is set for, in minutes.
/// @return The time in nr of minutes.
uint16_t Sharp_getTimerTime(void) {
  return _SharpProtocol.TimerHours * kSharpAcTimerIncrement * 2 +
      ((_SharpProtocol.Special == kSharpAcSpecialTimerHalfHour) ? kSharpAcTimerIncrement : 0);
}

/// Is the Timer enabled?
/// @return true, the setting is on. false, the setting is off.
bool Sharp_getTimerEnabled(void) { return _SharpProtocol.TimerEnabled; }

/// Get the current timer type.
/// @return true, It's an "On" timer. false, It's an "Off" timer.
bool Sharp_getTimerType(void) { return _SharpProtocol.TimerType; }

/// Set or cancel the timer function.
/// @param[in] enable Is the timer to be enabled (true) or canceled(false)?
/// @param[in] timer_type An On (true) or an Off (false). Ignored if canceled.
/// @param[in] mins Nr. of minutes the timer is to be set to.
/// @note Rounds down to 30 MIN increments. (MAX: 720 mins (12h), 0 is Off)
void Sharp_setTimer(bool enable, bool timer_type, uint16_t mins) {
  uint8_t half_hours = MIN(mins / kSharpAcTimerIncrement,
                                kSharpAcTimerHoursMax * 2);
  if (half_hours == 0) enable = false;
  if (!enable) {
    half_hours = 0;
    timer_type = kSharpAcOffTimerType;
  }
  _SharpProtocol.TimerEnabled = enable;
  _SharpProtocol.TimerType = timer_type;
  _SharpProtocol.TimerHours = half_hours / 2;
  // Handle non-round hours.
  _SharpProtocol.Special = (half_hours % 2) ? kSharpAcSpecialTimerHalfHour
                               : kSharpAcSpecialTimer;
  Sharp_setPowerSpecial(kSharpAcPowerTimerSetting);
}

/// Get the Clean setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Sharp_getClean(void) {
  return _SharpProtocol.Clean;
}

/// Set the Economical mode toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @note Officially A/C unit needs to be "Off" before clean mode can be entered
void Sharp_setClean(const bool on) {
  // Clean mode appears to be just default dry mode, with an extra bit set.
  if (on) {
    Sharp_setMode(kSharpAcDry, false);
    Sharp_setPower(true, false);
  } else {
    // Restore the previous operation mode & fan speed.
    Sharp_setMode(_mode, false);
    Sharp_setFan(_fan, false);
  }
  _SharpProtocol.Clean = on;
  Sharp_clearPowerSpecial();
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Sharp_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kSharpAcCool;
    case kOpModeHeat: return kSharpAcHeat;
    case kOpModeDry:  return kSharpAcDry;
    // No Fan mode.
    default:          return kSharpAcAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @param[in] model The enum of the appropriate model.
/// @return The native equivalent of the enum.
uint8_t Sharp_convertFan(const fanspeed_t speed,
                              const sharp_ac_remote_model_t model) {
  switch (model) {
    case A705:
    case A903:
      switch (speed) {
        case kFanSpeedLow:    return kSharpAcFanA705Low;
        case kFanSpeedMedium: return kSharpAcFanA705Med;
        default: {};  // Fall thru to the next/default clause if not the above
                      // special cases.
      }
    // FALL THRU
    default:
      switch (speed) {
        case kFanSpeedMin:
        case kFanSpeedLow:    return kSharpAcFanMin;
        case kFanSpeedMedium: return kSharpAcFanMed;
        case kFanSpeedHigh:   return kSharpAcFanHigh;
        case kFanSpeedMax:    return kSharpAcFanMax;
        default:              return kSharpAcFanAuto;
      }
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Sharp_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kSharpAcCool: return kOpModeCool;
    case kSharpAcHeat: return kOpModeHeat;
    case kSharpAcDry:  return kOpModeDry;
    case kSharpAcAuto:  // Also kSharpAcFan
      switch (Sharp_getModel(false)) {
        case A705: return kOpModeFan;
        default:   return kOpModeAuto;
      }
    default:           return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Sharp_toCommonFanSpeed(const uint8_t speed) {
  switch (Sharp_getModel(false)) {
    case A705:
    case A903:
      switch (speed) {
        case kSharpAcFanA705Low:  return kFanSpeedLow;
        case kSharpAcFanA705Med:  return kFanSpeedMedium;
      }
      // FALL-THRU
    default:
      switch (speed) {
        case kSharpAcFanMax:  return kFanSpeedHigh;
        case kSharpAcFanHigh: return kFanSpeedMedium;
        case kSharpAcFanMed:  return kFanSpeedLow;
        case kSharpAcFanMin:  return kFanSpeedMin;
        default:              return kFanSpeedAuto;
      }
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @param[in] mode What operating mode are we in?
/// @return The common vertical swing position.
swingv_t Sharp_toCommonSwingV(const uint8_t pos, const opmode_t mode) {
  switch (pos) {
    case kSharpAcSwingVHigh:   return kSwingVHighest;
    case kSharpAcSwingVMid:    return kSwingVMiddle;
    case kSharpAcSwingVLow:    return kSwingVLow;
    case kSharpAcSwingVCoanda:  // Coanda has mode dependent positionss
      switch (mode) {
        case kOpModeCool: return kSwingVHighest;
        case kOpModeHeat: return kSwingVLowest;
        default:                     return kSwingVOff;
      }
    case kSharpAcSwingVToggle: return kSwingVAuto;
    default:                   return kSwingVOff;
  }
}





