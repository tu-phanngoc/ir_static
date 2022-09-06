/// @file
/// @brief Support for Gree A/C protocols.

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ir_Gree.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"


#include "app_ac_status.h"
#include "IR_DeviceConstructor.h"

union GreeProtocol _GreeProtocol = { .remote_state = {0x00, 0x09, 0x20, 0x50, 0x20, 0x00, 0x00, 0x00}};
gree_ac_remote_model_t _GreeModel;

/* Get values from BLE command then fill to IR protocol */
void encode_Gree(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  Gree_setPower(ac_status.power_status);
  Gree_setTemp(ac_status.temperature, false);
  Gree_setFan(Gree_convertFan(ac_status.fan));
  Gree_setMode(Gree_convertMode(ac_status.mode));

  if(ac_status.swing)
    Gree_setSwingVertical(true, kGreeSwingAuto);
  else
    Gree_setSwingVertical(false, kGreeSwingDown);

  Gree_send(Gree_getRaw(), kGreeStateLength, kGreeRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Gree(int16_t* input, uint8_t* output) {

  initDecodeData(input, GREE_BITS);
  if( Gree_recv(&gDecodeResult, 0, kGreeBits, true) ){
    Gree_setRaw(gDecodeResult.state);
  }

  output[0] = Gree_getPower();
  output[1] = Gree_getTemp();
  output[2] = Gree_toCommonFanSpeed(_GreeProtocol.Fan);
  output[3] = Gree_getSwingVerticalPosition();
  output[4] = Gree_toCommonMode(_GreeProtocol.Mode);

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3] == kGreeSwingAuto);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}

bool isGreeAc(int16_t *irRaw){
  initDecodeData(irRaw, GREE_BITS);
  if( Gree_recv(&gDecodeResult, 0, kGreeBits, true) ){
    Gree_setRaw(gDecodeResult.state);
    return true;
  }
  else
    return false;
}


/// Decode the supplied Gree HVAC message.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Gree_recv(decode_results* results, uint16_t offset, const uint16_t nbits, bool const strict) {
  if (results->rawlen <= 2 * (nbits + kGreeBlockFooterBits) + (kHeader + kFooter + 1) - 1 + offset)
    return false;  // Can't possibly be a valid Gree message.
  if (strict && nbits != kGreeBits)
    return false;  // Not strictly a Gree message.

  // There are two blocks back-to-back in a full Gree IR message
  // sequence.

  uint16_t used;
  // Header + Data Block #1 (32 bits)
  used = matchGeneric_8(results->rawbuf + offset, results->state,
                        results->rawlen - offset, nbits / 2,
                        kGreeHdrMark, kGreeHdrSpace,
                        kGreeBitMark, kGreeOneSpace,
                        kGreeBitMark, kGreeZeroSpace,
                        0, 0, false,
                        kTolerance, kMarkExcess, false);
  if (used == 0) return false;
  offset += used;

  // Block #1 footer (3 bits, B010)
  match_result_t data_result;
  data_result = matchData(&(results->rawbuf[offset]), kGreeBlockFooterBits,
                          kGreeBitMark, kGreeOneSpace, kGreeBitMark, kGreeZeroSpace,
                          kTolerance, kMarkExcess, false, true);
  if (data_result.success == false) return false;
  if (data_result.data != kGreeBlockFooter) return false;
  offset += data_result.used;

  // Inter-block gap + Data Block #2 (32 bits) + Footer
  if (!matchGeneric_8(results->rawbuf + offset, results->state + 4,
                      results->rawlen - offset, nbits / 2,
                      kGreeBitMark, kGreeMsgSpace,
                      kGreeBitMark, kGreeOneSpace,
                      kGreeBitMark, kGreeZeroSpace,
                      kGreeBitMark, kGreeMsgSpace, true,
                      kTolerance,   kMarkExcess,   false)) return false;

  // Compliance
  if (strict) {
    // Verify the message's checksum is correct.
    if (!Gree_validChecksum(results->state, kGreeStateLength)) return false;
  }

  // Success
  results->decode_type = GREE;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}

/// Send a Gree Heat Pump formatted message.
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Gree_send( const uint8_t data[], const uint16_t nbytes,
                const uint16_t repeat, int16_t *irRaw) {
  if (nbytes < kGreeStateLength)
    return;  // Not enough bytes to send a proper message.

  // uint8_t _data[kGreeStateLength] = {0};
  // for(int i=0; i<kGreeStateLength; i++)
  //   _data[i] = data[kGreeStateLength - i -1];

  uint16_t offset = 0;
  for (uint16_t r = 0; r <= repeat; r++) {
    // Block #1
    sendGeneric_8(kGreeHdrMark, kGreeHdrSpace,
                  kGreeBitMark, kGreeOneSpace, kGreeBitMark, kGreeZeroSpace,
                  0, 0,  // No Footer.
                  data, 4,
                  38, false, 0, 50,
                  &irRaw[offset]);
    offset += 2 + 4*8*2; // 2Header + 4bytes data
    // Footer #1
    sendGeneric_64( 0, 0,  // No Header
                    kGreeBitMark, kGreeOneSpace, kGreeBitMark, kGreeZeroSpace,
                    kGreeBitMark, kGreeMsgSpace, 
                    0, // message time
                    kGreeBlockFooter, kGreeBlockFooterBits,
                    38, false, 0, 50,
                    &irRaw[offset]);
    offset += 0 + kGreeBlockFooterBits*2 + 2; // 0Header + 3bits data + 2Footers
    // Block #2
    sendGeneric_8(0, 0,  // No Header for Block #2
                  kGreeBitMark, kGreeOneSpace, kGreeBitMark, kGreeZeroSpace,
                  kGreeBitMark, kGreeMsgGap,
                  data + 4, nbytes - 4,
                  38, false, 0, 50,
                  &irRaw[offset]);
    offset += 0 + (nbytes - 4)*8*2 + 2; // 0Header + (nbytes - 4)bytes data + 2Footers
  }
}



/// Fix up the internal state so it is correct.
/// @note Internal use only.
void Gree_fixup(void) {
  Gree_setPower(Gree_getPower());  // Redo the power bits as they differ between models.
  Gree_checksum(kGreeStateLength);  // Calculate the checksums
}



/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* Gree_getRaw(void) {
  Gree_fixup();  // Ensure correct settings before sending.
  return _GreeProtocol.remote_state;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void Gree_setRaw(const uint8_t new_code[]) {
  memcpy(_GreeProtocol.remote_state, new_code, kGreeStateLength);
  // We can only detect the difference between models when the power is on.
  if (_GreeProtocol.Power) {
    if (_GreeProtocol.ModelA)
      _GreeModel = YAW1F;
    else
      _GreeModel = YBOFB;
  }
}

/// Calculate and set the checksum values for the internal state.
/// @param[in] length The size/length of the state array to fix the checksum of.
void Gree_checksum(const uint16_t length) {
  // Gree uses the same checksum alg. as Kelvinator's block checksum.
  _GreeProtocol.Sum = Gree_calcBlockChecksum(_GreeProtocol.remote_state, length);
}


/// Calculate the checksum for a given block of state.
/// @param[in] block A pointer to a block to calc the checksum of.
/// @param[in] length Length of the block array to checksum.
/// @return The calculated checksum value.
/// @note MANY BOTHANS DIED TO BRING US THIS INFORMATION.
uint8_t Gree_calcBlockChecksum(const uint8_t *block, const uint16_t length) {
  uint8_t sum = kGreeChecksumStart;
  // Sum the lower half of the first 4 bytes of this block.
  for (uint8_t i = 0; i < 4 && i < length - 1; i++, block++)
    sum += (*block & 0xF);
  // then sum the upper half of the next 3 bytes.
  for (uint8_t i = 4; i < length - 1; i++, block++) sum += (*block >> 4);
  // Trim it down to fit into the 4 bits allowed. i.e. Mod 16.
  return sum & 0xF;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Gree_validChecksum(const uint8_t state[], const uint16_t length) {
  // Top 4 bits of the last byte in the state is the state's checksum.
  return GETBITS8(state[length - 1], kHighNibble, kNibbleSize) == Gree_calcBlockChecksum(state, length);
}

/// Set the model of the A/C to emulate.
/// @param[in] model The enum of the appropriate model.
void Gree_setModel(const gree_ac_remote_model_t model) {
  switch (model) {
    case YAW1F:
    case YBOFB: _GreeModel = model; break;
    default:    _GreeModel = YAW1F; break;
  }
}

/// Get/Detect the model of the A/C.
/// @return The enum of the compatible model.
gree_ac_remote_model_t Gree_getModel(void){ return _GreeModel; }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setPower(const bool on) {
  _GreeProtocol.Power = on;
  // May not be needed.
  // _GreeProtocol.ModelA = (on && _GreeModel == YAW1F);
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getPower(void){
  //  See #814. Not checking/requiring: (_GreeProtocol.ModelA)
  return _GreeProtocol.Power;
}

/// Set the default temperature units to use.
/// @param[in] on Use Fahrenheit as the units.
///   true is Fahrenheit, false is Celsius.
void Gree_setUseFahrenheit(const bool on) { _GreeProtocol.UseFahrenheit = on; }

/// Get the default temperature units in use.
/// @return true is Fahrenheit, false is Celsius.
bool Gree_getUseFahrenheit(void){ return _GreeProtocol.UseFahrenheit; }

/// Set the temp. in degrees
/// @param[in] temp Desired temperature in Degrees.
/// @param[in] fahrenheit Use units of Fahrenheit and set that as units used.
///   false is Celsius (Default), true is Fahrenheit.
/// @note The unit actually works in Celsius with a special optional
///   "extra degree" when sending Fahrenheit.
void Gree_setTemp(const uint8_t temp, const bool fahrenheit) {
  float safecelsius = temp;
  if (fahrenheit)
    // Covert to F, and add a fudge factor to round to the expected degree.
    // Why 0.6 you ask?! Because it works. Ya'd thing 0.5 would be good for
    // rounding, but Noooooo!
    safecelsius = fahrenheitToCelsius(temp + 0.6);
  Gree_setUseFahrenheit(fahrenheit);  // Set the correct Temp units.

  // Make sure we have desired temp in the correct range.
  safecelsius = MAX(kGreeMinTempC, safecelsius);
  safecelsius = MIN(kGreeMaxTempC, safecelsius);
  // An operating mode of Auto locks the temp to a specific value. Do so.
  if (_GreeProtocol.Mode == kGreeAuto) safecelsius = 9; // 25C

  // Set the "main" Celsius degrees.
  _GreeProtocol.Temp = safecelsius;
  // Deal with the extra degree fahrenheit difference.
  _GreeProtocol.TempExtraDegreeF = ((uint8_t)(safecelsius * 2) & 1);
}

/// Get the set temperature
/// @return The temperature in degrees in the current units (C/F) set.
uint8_t Gree_getTemp(void){
  uint8_t deg = _GreeProtocol.Temp;
  if (false) { // Current not support UseFahrenheit
  // if (_GreeProtocol.UseFahrenheit) {
    deg = celsiusToFahrenheit(deg);
    // Retrieve the "extra" fahrenheit from elsewhere in the code.
    if (_GreeProtocol.TempExtraDegreeF) deg++;
    deg = MAX(deg, kGreeMinTempF);  // Cover the fact that 61F is < 16C
  }
  return deg;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting. 0 is auto, 1-3 is the speed.
void Gree_setFan(const uint8_t speed) {
  uint8_t fan = MAX(kGreeFanMax, speed);  // Bounds check
  if (_GreeProtocol.Mode == kGreeDry) fan = 1;  // DRY mode is always locked to fan 1.
  // Set the basic fan values.
  _GreeProtocol.Fan = fan;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Gree_getFan(void){ return _GreeProtocol.Fan; }

/// Set the operating mode of the A/C.
/// @param[in] new_mode The desired operating mode.
void Gree_setMode(const uint8_t new_mode) {
  uint8_t mode = new_mode;
  switch (mode) {
    // AUTO is locked to 25C
    case kGreeAuto: Gree_setTemp(9, false); break;
    // DRY always sets the fan to 1.
    case kGreeDry: Gree_setFan(1); break;
    case kGreeCool:
    case kGreeFan:
    case kGreeHeat: break;
    // If we get an unexpected mode, default to AUTO.
    default: mode = kGreeAuto;
  }
  _GreeProtocol.Mode = mode;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Gree_getMode(void){ return _GreeProtocol.Mode; }

/// Set the Light (LED) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setLight(const bool on) { _GreeProtocol.Light = on; }

/// Get the Light (LED) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getLight(void){ return _GreeProtocol.Light; }

/// Set the IFeel setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setIFeel(const bool on) { _GreeProtocol.IFeel = on; }

/// Get the IFeel setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getIFeel(void){ return _GreeProtocol.IFeel; }

/// Set the Wifi (enabled) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setWiFi(const bool on) { _GreeProtocol.WiFi = on; }

/// Get the Wifi (enabled) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getWiFi(void){ return _GreeProtocol.WiFi; }

/// Set the XFan (Mould) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setXFan(const bool on) { _GreeProtocol.Xfan = on; }

/// Get the XFan (Mould) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getXFan(void){ return _GreeProtocol.Xfan; }

/// Set the Sleep setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setSleep(const bool on) { _GreeProtocol.Sleep = on; }

/// Get the Sleep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getSleep(void){ return _GreeProtocol.Sleep; }

/// Set the Turbo setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setTurbo(const bool on) { _GreeProtocol.Turbo = on; }

/// Get the Turbo setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getTurbo(void){ return _GreeProtocol.Turbo; }

/// Set the Econo setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setEcono(const bool on) { _GreeProtocol.Econo = on; }

/// Get the Econo setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getEcono(void){ return _GreeProtocol.Econo; }

/// Set the Vertical Swing mode of the A/C.
/// @param[in] automatic Do we use the automatic setting?
/// @param[in] position The position/mode to set the vanes to.
void Gree_setSwingVertical(const bool automatic, const uint8_t position) {
  _GreeProtocol.SwingAuto = automatic;
  uint8_t new_position = position;
  if (!automatic) {
    switch (position) {
      case kGreeSwingUp:
      case kGreeSwingMiddleUp:
      case kGreeSwingMiddle:
      case kGreeSwingMiddleDown:
      case kGreeSwingDown:
        break;
      default:
        new_position = kGreeSwingLastPos;
    }
  } else {
    switch (position) {
      case kGreeSwingAuto:
      case kGreeSwingDownAuto:
      case kGreeSwingMiddleAuto:
      case kGreeSwingUpAuto:
        break;
      default:
        new_position = kGreeSwingAuto;
    }
  }
  _GreeProtocol.SwingV = new_position;
}

/// Get the Vertical Swing Automatic mode setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getSwingVerticalAuto(void){ return _GreeProtocol.SwingAuto; }

/// Get the Vertical Swing position setting of the A/C.
/// @return The native position/mode.
uint8_t Gree_getSwingVerticalPosition(void){ return _GreeProtocol.SwingV; }

/// Get the Horizontal Swing position setting of the A/C.
/// @return The native position/mode.
uint8_t Gree_getSwingHorizontal(void){ return _GreeProtocol.SwingH; }

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] position The position/mode to set the vanes to.
void Gree_setSwingHorizontal(const uint8_t position) {
  if (position <= kGreeSwingHMaxRight)
    _GreeProtocol.SwingH = position;
  else
    _GreeProtocol.SwingH = kGreeSwingHOff;
}

/// Set the timer enable setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Gree_setTimerEnabled(const bool on) { _GreeProtocol.TimerEnabled = on; }

/// Get the timer enabled setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Gree_getTimerEnabled(void){ return _GreeProtocol.TimerEnabled; }

/// Get the timer time value from the A/C.
/// @return The number of minutes the timer is set for.
uint16_t Gree_getTimer(void){
  uint16_t hrs = bcdToUint8((_GreeProtocol.TimerTensHr << kNibbleSize) |
    _GreeProtocol.TimerHours);
  return hrs * 60 + (_GreeProtocol.TimerHalfHr ? 30 : 0);
}

/// Set the A/C's timer to turn off in X many minutes.
/// @param[in] minutes The number of minutes the timer should be set for.
/// @note Stores time internally in 30 min units.
///  e.g. 5 mins means 0 (& Off), 95 mins is  90 mins (& On). Max is 24 hours.
void Gree_setTimer(const uint16_t minutes) {
  uint16_t mins = MIN(kGreeTimerMax, minutes);  // Bounds check.
  Gree_setTimerEnabled(mins >= 30);  // Timer is enabled when >= 30 mins.
  uint8_t hours = mins / 60;
  // Set the half hour bit.
  _GreeProtocol.TimerHalfHr = (mins % 60) >= 30;
  // Set the "tens" digit of hours.
  _GreeProtocol.TimerTensHr = hours / 10;
  // Set the "units" digit of hours.
  _GreeProtocol.TimerHours = hours % 10;
}

/// Set temperature display mode.
/// i.e. Internal, External temperature sensing.
/// @param[in] mode The desired temp source to display.
/// @note In order for the A/C unit properly accept these settings. You must
///   cycle (send) in the following order:
///   kGreeDisplayTempOff(0) -> kGreeDisplayTempSet(1) ->
///   kGreeDisplayTempInside(2) ->kGreeDisplayTempOutside(3) ->
///   kGreeDisplayTempOff(0).
///   The unit will no behave correctly if the changes of this setting are sent
///   out of order.
void Gree_setDisplayTempSource(const uint8_t mode) {
  _GreeProtocol.DisplayTemp = mode;
}

/// Get the temperature display mode.
/// i.e. Internal, External temperature sensing.
/// @return The current temp source being displayed.
uint8_t Gree_getDisplayTempSource(void){ return _GreeProtocol.DisplayTemp; }

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Gree_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kGreeCool;
    case kOpModeHeat: return kGreeHeat;
    case kOpModeDry:  return kGreeDry;
    case kOpModeFan:  return kGreeFan;
    default:          return kGreeAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Gree_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kGreeFanMin;
    case kFanSpeedLow:    return kGreeFanMax - 1;
    case kFanSpeedMedium:
    case kFanSpeedHigh:
    case kFanSpeedMax:    return kGreeFanMax;
    default:              return kGreeFanAuto;
  }
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] swingv The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Gree_convertSwingV(const swingv_t swingv) {
  switch (swingv) {
    case kSwingVHighest: return kGreeSwingUp;
    case kSwingVHigh:    return kGreeSwingMiddleUp;
    case kSwingVMiddle:  return kGreeSwingMiddle;
    case kSwingVLow:     return kGreeSwingMiddleDown;
    case kSwingVLowest:  return kGreeSwingDown;
    default:             return kGreeSwingAuto;
  }
}

/// Convert a swingh_t enum into it's native setting.
/// @param[in] swingh The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Gree_convertSwingH(const swingh_t swingh) {
  switch (swingh) {
    case kSwingHAuto:      return kGreeSwingHAuto;
    case kSwingHLeftMax:   return kGreeSwingHMaxLeft;
    case kSwingHLeft:      return kGreeSwingHLeft;
    case kSwingHMiddle:    return kGreeSwingHMiddle;
    case kSwingHRight:     return kGreeSwingHRight;
    case kSwingHRightMax:  return kGreeSwingHMaxRight;
    default:               return kGreeSwingHOff;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Gree_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kGreeCool: return kOpModeCool;
    case kGreeHeat: return kOpModeHeat;
    case kGreeDry:  return kOpModeDry;
    case kGreeFan:  return kOpModeFan;
    default:        return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Gree_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kGreeFanMax:     return kFanSpeedMedium;
    case kGreeFanMax - 1: return kFanSpeedLow;
    case kGreeFanMin:     return kFanSpeedMin;
    default:              return kFanSpeedAuto;
  }
}

/// Convert a native Vertical Swing into its stdAc equivalent.
/// @param[in] pos The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
swingv_t Gree_toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kGreeSwingUp:         return kSwingVHighest;
    case kGreeSwingMiddleUp:   return kSwingVHigh;
    case kGreeSwingMiddle:     return kSwingVMiddle;
    case kGreeSwingMiddleDown: return kSwingVLow;
    case kGreeSwingDown:       return kSwingVLowest;
    default:                   return kSwingVAuto;
  }
}

/// Convert a native Horizontal Swing into its stdAc equivalent.
/// @param[in] pos The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
swingh_t Gree_toCommonSwingH(const uint8_t pos) {
  switch (pos) {
    case kGreeSwingHAuto:      return kSwingHAuto;
    case kGreeSwingHMaxLeft:   return kSwingHLeftMax;
    case kGreeSwingHLeft:      return kSwingHLeft;
    case kGreeSwingHMiddle:    return kSwingHMiddle;
    case kGreeSwingHRight:     return kSwingHRight;
    case kGreeSwingHMaxRight:  return kSwingHRightMax;
    default:                   return kSwingHOff;
  }
}

/// Convert the current internal state into its state_t equivalent.
/// @return The stdAc equivalent of the native settings.
state_t Gree_toCommon(void) {
  state_t result 	= {0};
  result.protocol = GREE;
  result.model 		= _GreeModel;
  result.power 		= _GreeProtocol.Power;
  result.mode  		= Gree_toCommonMode(_GreeProtocol.Mode);
  result.celsius  = !_GreeProtocol.UseFahrenheit;
  result.degrees  = Gree_getTemp();
  result.fanspeed = Gree_toCommonFanSpeed(_GreeProtocol.Fan);
  
	if (_GreeProtocol.SwingAuto)
    result.swingv = kSwingVAuto;
  else
    result.swingv = Gree_toCommonSwingV(_GreeProtocol.SwingV);
	
  result.swingh = Gree_toCommonSwingH(_GreeProtocol.SwingH);
  result.turbo = _GreeProtocol.Turbo;
  result.econo = _GreeProtocol.Econo;
  result.light = _GreeProtocol.Light;
  result.clean = _GreeProtocol.Xfan;
  result.sleep = _GreeProtocol.Sleep ? 0 : -1;
  // Not supported.
  result.quiet = false;
  result.filter = false;
  result.beep = false;
  result.clock = -1;
  return result;
}

