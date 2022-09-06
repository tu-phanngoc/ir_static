// Copyright 2019 David Conran

/// @file
/// @brief Support for Mitsubishi Heavy Industry protocols.
/// Code to emulate Mitsubishi Heavy Industries A/C IR remote control units.
/// @note This code was *heavily* influenced by ToniA's great work & code,
///   but it has been written from scratch.
///   Nothing was copied other than constants and message analysis.

#include "ir_MitsubishiHeavy.h"
#include "IRutils.h"

#include "app_ac_status.h"
#include "IR_Common.h"

Mitsubishi152Protocol_t _Mitsubishi152Protocol = { .raw = {0xAD, 0x51, 0x3C, 0xE5, 0x1A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };
Mitsubishi88Protocol_t  _Mitsubishi88Protocol  = { .raw = {0xAD, 0x51, 0x3C, 0xD9, 0x26, 0, 0, 0, 0, 0, 0}};

const uint8_t kMitsubishiHeavyZmsSig[kMitsubishiHeavySigLength] = { 0xAD, 0x51, 0x3C, 0xE5, 0x1A}; // ZMS (152 bit)
const uint8_t kMitsubishiHeavyZjsSig[kMitsubishiHeavySigLength] = { 0xAD, 0x51, 0x3C, 0xD9, 0x26}; // ZJS (88 bit)

static bool isHeavy152 = false;

/* Get values from BLE command then fill to IR protocol */
void encode_MitsubishiHeavy(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  if(isHeavy152){
    Mitsubishi152_setPower(ac_status.power_status);
    Mitsubishi152_setTemp(ac_status.temperature);
    Mitsubishi152_setMode(Mitsubishi152_convertMode(ac_status.mode));
    Mitsubishi152_setFan(Mitsubishi152_convertFan(ac_status.fan));
    if(ac_status.swing)
      Mitsubishi152_setSwingVertical(kMitsubishiHeavy152SwingVAuto);
    else
      Mitsubishi152_setSwingVertical(kMitsubishiHeavy152SwingVOff);

    MitsubishiHeavy_send(Mitsubishi152_getRaw(), kMitsubishi152StateLength, kMitsubishi152MinRepeat, OutputIRProtocol);
  }
  else{
    Mitsubishi88_setPower(ac_status.power_status);
    Mitsubishi88_setTemp(ac_status.temperature);
    Mitsubishi88_setMode(Mitsubishi88_convertMode(ac_status.mode));
    Mitsubishi88_setFan(Mitsubishi88_convertFan(ac_status.fan));
    
    if(ac_status.swing)
      Mitsubishi88_setSwingVertical(kMitsubishiHeavy88SwingVAuto);
    else
      Mitsubishi88_setSwingVertical(kMitsubishiHeavy88SwingVOff);
    
    MitsubishiHeavy_send(Mitsubishi88_getRaw(), kMitsubishi88StateLength, kMitsubishi88MinRepeat, OutputIRProtocol);
  }
  setIrTxState(1);
}


void decode_MitsubishiHeavy(int16_t* input, uint8_t* output) {

  if(isHeavy152){
    initDecodeData(input, MITSUBISHI152_BITS);
    if( MitsubishiHeavy_recv(&gDecodeResult, 0, kMitsubishi152Bits, true) ){
      Mitsubishi152_setRaw(gDecodeResult.state);
    }
    else return;

    output[0] = Mitsubishi152_getPower();
    output[1] = Mitsubishi152_getTemp();
    output[2] = Mitsubishi152_toCommonFanSpeed(Mitsubishi152_getFan());
    output[3] = (Mitsubishi152_getSwingVertical() == kMitsubishiHeavy152SwingVAuto);
    output[4] = Mitsubishi152_toCommonMode(Mitsubishi152_getMode());
  }
  else{
    initDecodeData(input, MITSUBISHI88_BITS);
    if( MitsubishiHeavy_recv(&gDecodeResult, 0, kMitsubishi88Bits, true) ){
      Mitsubishi88_setRaw(gDecodeResult.state);
    }
    else return;

    output[0] = Mitsubishi88_getPower();
    output[1] = Mitsubishi88_getTemp();
    output[2] = Mitsubishi88_toCommonFanSpeed(Mitsubishi88_getFan());
    output[3] = (Mitsubishi88_getSwingVertical() == kMitsubishiHeavy152SwingVAuto);
    output[4] = Mitsubishi152_toCommonMode(Mitsubishi88_getMode());
  }

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3]);
  ac_control_set_mode(output[4]);
  ac_control_update_status_to_payload();
}


void MitsubishiHeavy_set152AcTypes(bool on){
  isHeavy152 = on;
}


/// Decode the supplied Mitsubishi Heavy Industries A/C message.
/// Status: BETA / Appears to be working. Needs testing against a real device.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
///   Typically kMitsubishi88Bits or kMitsubishi152Bits (def).
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool MitsubishiHeavy_recv(decode_results* results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (strict) {
    switch (nbits) {
      case kMitsubishi88Bits:
      case kMitsubishi152Bits:
        break;
      default:
        return false;  // Not what is expected
    }
  }

  uint16_t used;
  used = matchGeneric_8(results->rawbuf + offset, results->state,
                        results->rawlen - offset, nbits,
                        kMitsubishiHeavyHdrMark, kMitsubishiHeavyHdrSpace,
                        kMitsubishiHeavyBitMark, kMitsubishiHeavyOneSpace,
                        kMitsubishiHeavyBitMark, kMitsubishiHeavyZeroSpace,
                        kMitsubishiHeavyBitMark, kMitsubishiHeavyGap, true,
                        kTolerance, 0, false);
  if (used == 0) return false;
  offset += used;
  // Compliance
  switch (nbits) {
    case kMitsubishi88Bits:
      if (strict && !(Mitsubishi88_checkZjsSig(results->state) &&
                      Mitsubishi88_validChecksum(results->state, kMitsubishi88StateLength)))
        return false;
      results->decode_type = MITSUBISHI_HEAVY_88;
      break;
    case kMitsubishi152Bits:
      if (strict && !(Mitsubishi152_checkZmsSig(results->state) &&
                      Mitsubishi152_validChecksum(results->state, kMitsubishi152StateLength)))
        return false;
      results->decode_type = MITSUBISHI_HEAVY_152;
      break;
    default:
      return false;
  }

  // Success
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}

/// Status: BETA / Appears to be working. Needs testing against a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void MitsubishiHeavy_send( const unsigned char data[],
                        const uint16_t nbytes,
                        const uint16_t repeat,
                        int16_t *irRaw) {
  if (nbytes != kMitsubishi88StateLength && nbytes != kMitsubishi152StateLength)
    return;  // Not enough bytes to send a proper message.
  sendGeneric_8(kMitsubishiHeavyHdrMark, kMitsubishiHeavyHdrSpace,
                kMitsubishiHeavyBitMark, kMitsubishiHeavyOneSpace,
                kMitsubishiHeavyBitMark, kMitsubishiHeavyZeroSpace,
                kMitsubishiHeavyBitMark, kMitsubishiHeavyGap,
                data, nbytes, 38000, false, repeat, kDutyDefault,
                irRaw);
}

// Class for decoding and constructing MitsubishiHeavy152 AC messages.

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Mitsubishi152_getRaw(void) {
  Mitsubishi152_checksum();
  return _Mitsubishi152Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] data A valid code for this protocol.
void Mitsubishi152_setRaw(const uint8_t *data) {
  memcpy(_Mitsubishi152Protocol.raw, data, kMitsubishi152StateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi152_setPower(const bool on) {
  _Mitsubishi152Protocol.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi152_getPower(void){
  return _Mitsubishi152Protocol.Power;
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Mitsubishi152_setTemp(const uint8_t temp) {
  uint8_t newtemp = (temp < 2) ? kMitsubishiHeavyMinTemp : (temp + 16);
  newtemp = MIN(newtemp, kMitsubishiHeavyMaxTemp);
  newtemp = MAX(newtemp, kMitsubishiHeavyMinTemp);
  _Mitsubishi152Protocol.Temp = newtemp - kMitsubishiHeavyDeltaTemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Mitsubishi152_getTemp(void){
  // return _Mitsubishi152Protocol.Temp + kMitsubishiHeavyMinTemp;
  return _Mitsubishi152Protocol.Temp + 1;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Mitsubishi152_setFan(const uint8_t speed) {
  uint8_t newspeed = speed;
  switch (speed) {
    case kMitsubishiHeavy152FanLow:
    case kMitsubishiHeavy152FanMed:
    case kMitsubishiHeavy152FanHigh:
    case kMitsubishiHeavy152FanMax:
    case kMitsubishiHeavy152FanEcono:
    case kMitsubishiHeavy152FanTurbo: break;
    default: newspeed = kMitsubishiHeavy152FanAuto;
  }
  _Mitsubishi152Protocol.Fan = newspeed;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Mitsubishi152_getFan(void){
  return _Mitsubishi152Protocol.Fan;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Mitsubishi152_setMode(const uint8_t mode) {
  uint8_t newmode = mode;
  switch (mode) {
    case kMitsubishiHeavyCool:
    case kMitsubishiHeavyDry:
    case kMitsubishiHeavyFan:
    case kMitsubishiHeavyHeat:
      break;
    default:
      newmode = kMitsubishiHeavyAuto;
  }
  _Mitsubishi152Protocol.Mode = newmode;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Mitsubishi152_getMode(void){
  return _Mitsubishi152Protocol.Mode;
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] pos The position/mode to set the swing to.
void Mitsubishi152_setSwingVertical(const uint8_t pos) {
  _Mitsubishi152Protocol.SwingV = MIN(pos, kMitsubishiHeavy152SwingVOff);
}

/// Get the Vertical Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi152_getSwingVertical(void){
  return _Mitsubishi152Protocol.SwingV;
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] pos The position/mode to set the swing to.
void Mitsubishi152_setSwingHorizontal(const uint8_t pos) {
  _Mitsubishi152Protocol.SwingH = MIN(pos, kMitsubishiHeavy152SwingHOff);
}

/// Get the Horizontal Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi152_getSwingHorizontal(void){
  return _Mitsubishi152Protocol.SwingH;
}

/// Set the Night (Sleep) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi152_setNight(const bool on) {
  _Mitsubishi152Protocol.Night = on;
}

/// Get the Night (Sleep) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi152_getNight(void){
  return _Mitsubishi152Protocol.Night;
}

/// Set the 3D mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi152_set3D(const bool on) {
  if (on)
    { _Mitsubishi152Protocol.Three = 1; _Mitsubishi152Protocol.D = 1; }
  else
    { _Mitsubishi152Protocol.Three = 0; _Mitsubishi152Protocol.D = 0; }
}

/// Get the 3D mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi152_get3D(void){
  return _Mitsubishi152Protocol.Three && _Mitsubishi152Protocol.D;
}

/// Set the Silent (Quiet) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi152_setSilent(const bool on) {
  _Mitsubishi152Protocol.Silent = on;
}

/// Get the Silent (Quiet) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi152_getSilent(void){
  return _Mitsubishi152Protocol.Silent;
}

/// Set the Filter mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi152_setFilter(const bool on) {
  _Mitsubishi152Protocol.Filter = on;
}

/// Get the Filter mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi152_getFilter(void){
  return _Mitsubishi152Protocol.Filter;
}

/// Set the Clean mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi152_setClean(const bool on) {
  _Mitsubishi152Protocol.Filter = on;
  _Mitsubishi152Protocol.Clean = on;
}

/// Get the Clean mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi152_getClean(void){
  return _Mitsubishi152Protocol.Clean && _Mitsubishi152Protocol.Filter;
}

/// Set the Turbo mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi152_setTurbo(const bool on) {
  if (on)
    Mitsubishi152_setFan(kMitsubishiHeavy152FanTurbo);
  else if (Mitsubishi152_getTurbo()) Mitsubishi152_setFan(kMitsubishiHeavy152FanAuto);
}

/// Get the Turbo mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi152_getTurbo(void){
  return _Mitsubishi152Protocol.Fan == kMitsubishiHeavy152FanTurbo;
}

/// Set the Economical mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi152_setEcono(const bool on) {
  if (on)
    Mitsubishi152_setFan(kMitsubishiHeavy152FanEcono);
  else if (Mitsubishi152_getEcono()) Mitsubishi152_setFan(kMitsubishiHeavy152FanAuto);
}

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi152_getEcono(void){
  return _Mitsubishi152Protocol.Fan == kMitsubishiHeavy152FanEcono;
}

/// Verify the given state has a ZM-S signature.
/// @param[in] state A ptr to a state to be checked.
/// @return true, the check passed. Otherwise, false.
bool Mitsubishi152_checkZmsSig(const uint8_t *state) {
  for (uint8_t i = 0; i < kMitsubishiHeavySigLength; i++)
    if (state[i] != kMitsubishiHeavyZmsSig[i]) return false;
  return true;
}

/// Calculate the checksum for the current internal state of the remote.
/// Note: Technically it has no checksum, but does have inverted byte pairs.
void Mitsubishi152_checksum(void) {
  const uint8_t kOffset = kMitsubishiHeavySigLength - 2;
  invertBytePairs(_Mitsubishi152Protocol.raw + kOffset, kMitsubishi152StateLength - kOffset);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
/// Note: Technically it has no checksum, but does have inverted byte pairs.
bool Mitsubishi152_validChecksum(const uint8_t *state, const uint16_t length) {
  // Assume anything too short is fine.
  if (length < kMitsubishiHeavySigLength) return true;
  const uint8_t kOffset = kMitsubishiHeavySigLength - 2;
  return checkInvertedBytePairs(state + kOffset, length - kOffset);
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi152_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kMitsubishiHeavyCool;
    case kOpModeHeat: return kMitsubishiHeavyHeat;
    case kOpModeDry:  return kMitsubishiHeavyDry;
    case kOpModeFan:  return kMitsubishiHeavyFan;
    default:          return kMitsubishiHeavyAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi152_convertFan(const fanspeed_t speed) {
  switch (speed) {
    // Assumes Econo is slower than Low.
    case kFanSpeedMin:    return kMitsubishiHeavy152FanLow;
    case kFanSpeedLow:    return kMitsubishiHeavy152FanMed;
    case kFanSpeedMedium: return kMitsubishiHeavy152FanHigh;
    // case kFanSpeedHigh:   return kMitsubishiHeavy152FanEcono;  // Currently not support
    // case kFanSpeedMax:    return kMitsubishiHeavy152FanMax;    // Currently not support
    default:              return kMitsubishiHeavy152FanAuto;
  }
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi152_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVAuto:    return kMitsubishiHeavy152SwingVAuto;
    case kSwingVHighest: return kMitsubishiHeavy152SwingVHighest;
    case kSwingVHigh:    return kMitsubishiHeavy152SwingVHigh;
    case kSwingVMiddle:  return kMitsubishiHeavy152SwingVMiddle;
    case kSwingVLow:     return kMitsubishiHeavy152SwingVLow;
    case kSwingVLowest:  return kMitsubishiHeavy152SwingVLowest;
    default:             return kMitsubishiHeavy152SwingVOff;
  }
}

/// Convert a swingh_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi152_convertSwingH(const swingh_t position) {
  switch (position) {
    case kSwingHAuto:     return kMitsubishiHeavy152SwingHAuto;
    case kSwingHLeftMax:  return kMitsubishiHeavy152SwingHLeftMax;
    case kSwingHLeft:     return kMitsubishiHeavy152SwingHLeft;
    case kSwingHMiddle:   return kMitsubishiHeavy152SwingHMiddle;
    case kSwingHRight:    return kMitsubishiHeavy152SwingHRight;
    case kSwingHRightMax: return kMitsubishiHeavy152SwingHRightMax;
    default:              return kMitsubishiHeavy152SwingHOff;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Mitsubishi152_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kMitsubishiHeavyCool: return kOpModeCool;
    case kMitsubishiHeavyHeat: return kOpModeHeat;
    case kMitsubishiHeavyDry:  return kOpModeDry;
    case kMitsubishiHeavyFan:  return kOpModeFan;
    default:                   return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Mitsubishi152_toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kMitsubishiHeavy152FanMax:   return kFanSpeedMax;
    case kMitsubishiHeavy152FanHigh:  return kFanSpeedHigh;
    case kMitsubishiHeavy152FanMed:   return kFanSpeedMedium;
    case kMitsubishiHeavy152FanLow:   return kFanSpeedLow;
    case kMitsubishiHeavy152FanEcono: return kFanSpeedMin;
    default:                          return kFanSpeedAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common horizontal swing position.
swingh_t Mitsubishi152_toCommonSwingH(const uint8_t pos) {
  switch (pos) {
    case kMitsubishiHeavy152SwingHLeftMax:  return kSwingHLeftMax;
    case kMitsubishiHeavy152SwingHLeft:     return kSwingHLeft;
    case kMitsubishiHeavy152SwingHMiddle:   return kSwingHMiddle;
    case kMitsubishiHeavy152SwingHRight:    return kSwingHRight;
    case kMitsubishiHeavy152SwingHRightMax: return kSwingHRightMax;
    case kMitsubishiHeavy152SwingHOff:      return kSwingHOff;
    default:                                return kSwingHAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
swingv_t Mitsubishi152_toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kMitsubishiHeavy152SwingVHighest: return kSwingVHighest;
    case kMitsubishiHeavy152SwingVHigh:    return kSwingVHigh;
    case kMitsubishiHeavy152SwingVMiddle:  return kSwingVMiddle;
    case kMitsubishiHeavy152SwingVLow:     return kSwingVLow;
    case kMitsubishiHeavy152SwingVLowest:  return kSwingVLowest;
    case kMitsubishiHeavy152SwingVOff:     return kSwingVOff;
    default:                               return kSwingVAuto;
  }
}











// Class for decoding and constructing MitsubishiHeavy88 AC messages.

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Mitsubishi88_getRaw(void) {
  Mitsubishi88_checksum();
  return _Mitsubishi88Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] data A valid code for this protocol.
void Mitsubishi88_setRaw(const uint8_t *data) {
  memcpy(_Mitsubishi88Protocol.raw, data, kMitsubishi88StateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi88_setPower(const bool on) {
  _Mitsubishi88Protocol.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi88_getPower(void){
  return _Mitsubishi88Protocol.Power;
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Mitsubishi88_setTemp(const uint8_t temp) {
  uint8_t newtemp = (temp < 2) ? kMitsubishiHeavyMinTemp : (temp + 16);
  newtemp = MIN(newtemp, kMitsubishiHeavyMaxTemp);
  newtemp = MAX(newtemp, kMitsubishiHeavyMinTemp);
  _Mitsubishi88Protocol.Temp = newtemp - kMitsubishiHeavyDeltaTemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Mitsubishi88_getTemp(void){
  return _Mitsubishi88Protocol.Temp + 1;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Mitsubishi88_setFan(const uint8_t speed) {
  uint8_t newspeed = speed;
  switch (speed) {
    case kMitsubishiHeavy88FanLow:
    case kMitsubishiHeavy88FanMed:
    case kMitsubishiHeavy88FanHigh:
    case kMitsubishiHeavy88FanTurbo:
    case kMitsubishiHeavy88FanEcono: break;
    default: newspeed = kMitsubishiHeavy88FanAuto;
  }
  _Mitsubishi88Protocol.Fan = newspeed;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Mitsubishi88_getFan(void){
  return _Mitsubishi88Protocol.Fan;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Mitsubishi88_setMode(const uint8_t mode) {
  uint8_t newmode = mode;
  switch (mode) {
    case kMitsubishiHeavyCool:
    case kMitsubishiHeavyDry:
    case kMitsubishiHeavyFan:
    case kMitsubishiHeavyHeat:
      break;
    default:
      newmode = kMitsubishiHeavyAuto;
  }
  _Mitsubishi88Protocol.Mode = newmode;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Mitsubishi88_getMode(void){
  return _Mitsubishi88Protocol.Mode;
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] pos The position/mode to set the swing to.
void Mitsubishi88_setSwingVertical(const uint8_t pos) {
  uint8_t newpos;
  switch (pos) {
    case kMitsubishiHeavy88SwingVAuto:
    case kMitsubishiHeavy88SwingVHighest:
    case kMitsubishiHeavy88SwingVHigh:
    case kMitsubishiHeavy88SwingVMiddle:
    case kMitsubishiHeavy88SwingVLow:
    case kMitsubishiHeavy88SwingVLowest: newpos = pos; break;
    default: newpos = kMitsubishiHeavy88SwingVOff;
  }
  _Mitsubishi88Protocol.SwingV5 = newpos;
  _Mitsubishi88Protocol.SwingV7 = (newpos >> kMitsubishiHeavy88SwingVByte5Size);
}

/// Get the Vertical Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi88_getSwingVertical(void){
  return _Mitsubishi88Protocol.SwingV5 | (_Mitsubishi88Protocol.SwingV7 << kMitsubishiHeavy88SwingVByte5Size);
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] pos The position/mode to set the swing to.
void Mitsubishi88_setSwingHorizontal(const uint8_t pos) {
  uint8_t newpos;
  switch (pos) {
    case kMitsubishiHeavy88SwingHAuto:
    case kMitsubishiHeavy88SwingHLeftMax:
    case kMitsubishiHeavy88SwingHLeft:
    case kMitsubishiHeavy88SwingHMiddle:
    case kMitsubishiHeavy88SwingHRight:
    case kMitsubishiHeavy88SwingHRightMax:
    case kMitsubishiHeavy88SwingHLeftRight:
    case kMitsubishiHeavy88SwingHRightLeft:
    case kMitsubishiHeavy88SwingH3D: newpos = pos; break;
    default:                         newpos = kMitsubishiHeavy88SwingHOff;
  }
  _Mitsubishi88Protocol.SwingH1 = newpos;
  _Mitsubishi88Protocol.SwingH2 = (newpos >> kMitsubishiHeavy88SwingHSize);
}

/// Get the Horizontal Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi88_getSwingHorizontal(void){
  return _Mitsubishi88Protocol.SwingH1 | (_Mitsubishi88Protocol.SwingH2 << kMitsubishiHeavy88SwingHSize);
}

/// Set the Turbo mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi88_setTurbo(const bool on) {
  if (on)
    Mitsubishi88_setFan(kMitsubishiHeavy88FanTurbo);
  else if (Mitsubishi88_getTurbo()) Mitsubishi88_setFan(kMitsubishiHeavy88FanAuto);
}

/// Get the Turbo mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi88_getTurbo(void){
  return _Mitsubishi88Protocol.Fan == kMitsubishiHeavy88FanTurbo;
}

/// Set the Economical mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi88_setEcono(const bool on) {
  if (on)
    Mitsubishi88_setFan(kMitsubishiHeavy88FanEcono);
  else if (Mitsubishi88_getEcono()) Mitsubishi88_setFan(kMitsubishiHeavy88FanAuto);
}

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi88_getEcono(void){
  return _Mitsubishi88Protocol.Fan == kMitsubishiHeavy88FanEcono;
}

/// Set the 3D mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi88_set3D(const bool on) {
  if (on)
    Mitsubishi88_setSwingHorizontal(kMitsubishiHeavy88SwingH3D);
  else if (Mitsubishi88_get3D())
    Mitsubishi88_setSwingHorizontal(kMitsubishiHeavy88SwingHOff);
}

/// Get the 3D mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi88_get3D(void){
  return Mitsubishi88_getSwingHorizontal() == kMitsubishiHeavy88SwingH3D;
}

/// Set the Clean mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi88_setClean(const bool on) {
  _Mitsubishi88Protocol.Clean = on;
}

/// Get the Clean mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi88_getClean(void){
  return _Mitsubishi88Protocol.Clean;
}

/// Verify the given state has a ZJ-S signature.
/// @param[in] state A ptr to a state to be checked.
/// @return true, the check passed. Otherwise, false.
bool Mitsubishi88_checkZjsSig(const uint8_t *state) {
  for (uint8_t i = 0; i < kMitsubishiHeavySigLength; i++)
    if (state[i] != kMitsubishiHeavyZjsSig[i]) return false;
  return true;
}

/// Calculate the checksum for the current internal state of the remote.
/// Note: Technically it has no checksum, but does have inverted byte pairs.
void Mitsubishi88_checksum(void) {
  const uint8_t kOffset = kMitsubishiHeavySigLength - 2;
  invertBytePairs(_Mitsubishi88Protocol.raw + kOffset, kMitsubishi88StateLength - kOffset);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
/// Note: Technically it has no checksum, but does have inverted byte pairs.
bool Mitsubishi88_validChecksum(const uint8_t *state, const uint16_t length) {
  return Mitsubishi152_validChecksum(state, length);
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi88_convertMode(const opmode_t mode) {
  return Mitsubishi152_convertMode(mode);
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi88_convertFan(const fanspeed_t speed) {
  switch (speed) {
    // Assumes Econo is slower than Low.
    case kFanSpeedMin:    return kMitsubishiHeavy88FanLow;
    case kFanSpeedLow:    return kMitsubishiHeavy88FanMed;
    case kFanSpeedMedium: return kMitsubishiHeavy88FanHigh;
    // case kFanSpeedHigh:   return kMitsubishiHeavy88FanMed;     // Current not support
    // case kFanSpeedMax:    return kMitsubishiHeavy88FanTurbo;   // Current not support
    default:              return kMitsubishiHeavy88FanAuto;
  }
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi88_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVAuto:    return kMitsubishiHeavy88SwingVAuto;
    case kSwingVHighest: return kMitsubishiHeavy88SwingVHighest;
    case kSwingVHigh:    return kMitsubishiHeavy88SwingVHigh;
    case kSwingVMiddle:  return kMitsubishiHeavy88SwingVMiddle;
    case kSwingVLow:     return kMitsubishiHeavy88SwingVLow;
    case kSwingVLowest:  return kMitsubishiHeavy88SwingVLowest;
    default:             return kMitsubishiHeavy88SwingVOff;
  }
}

/// Convert a swingh_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi88_convertSwingH(const swingh_t position) {
  switch (position) {
    case kSwingHAuto:     return kMitsubishiHeavy88SwingHAuto;
    case kSwingHLeftMax:  return kMitsubishiHeavy88SwingHLeftMax;
    case kSwingHLeft:     return kMitsubishiHeavy88SwingHLeft;
    case kSwingHMiddle:   return kMitsubishiHeavy88SwingHMiddle;
    case kSwingHRight:    return kMitsubishiHeavy88SwingHRight;
    case kSwingHRightMax: return kMitsubishiHeavy88SwingHRightMax;
    default:              return kMitsubishiHeavy88SwingHOff;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Mitsubishi88_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kMitsubishiHeavy88FanTurbo: return kFanSpeedMax;
    case kMitsubishiHeavy88FanHigh:  return kFanSpeedHigh;
    case kMitsubishiHeavy88FanMed:   return kFanSpeedMedium;
    case kMitsubishiHeavy88FanLow:   return kFanSpeedLow;
    case kMitsubishiHeavy88FanEcono: return kFanSpeedMin;
    default:                         return kFanSpeedAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common horizontal swing position.
swingh_t Mitsubishi88_toCommonSwingH(const uint8_t pos) {
  switch (pos) {
    case kMitsubishiHeavy88SwingHLeftMax:  return kSwingHLeftMax;
    case kMitsubishiHeavy88SwingHLeft:     return kSwingHLeft;
    case kMitsubishiHeavy88SwingHMiddle:   return kSwingHMiddle;
    case kMitsubishiHeavy88SwingHRight:    return kSwingHRight;
    case kMitsubishiHeavy88SwingHRightMax: return kSwingHRightMax;
    case kMitsubishiHeavy88SwingHOff:      return kSwingHOff;
    default:                               return kSwingHAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
swingv_t Mitsubishi88_toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kMitsubishiHeavy88SwingVHighest: return kSwingVHighest;
    case kMitsubishiHeavy88SwingVHigh:    return kSwingVHigh;
    case kMitsubishiHeavy88SwingVMiddle:  return kSwingVMiddle;
    case kMitsubishiHeavy88SwingVLow:     return kSwingVLow;
    case kMitsubishiHeavy88SwingVLowest:  return kSwingVLowest;
    case kMitsubishiHeavy88SwingVOff:     return kSwingVOff;
    default:                              return kSwingVAuto;
  }
}







