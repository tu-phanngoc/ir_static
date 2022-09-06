
/// @file
/// @brief Support for Toshiba protocols.

#include "ir_Toshiba.h"

#include "string.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "app_ac_status.h"
#include "IR_Common.h"

union ToshibaProtocol _ToshibaProtocol = { .raw = {0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x80, 0x01, 0x00, 0x80, 0x00} };
uint8_t backup[kToshibaAcStateLengthLong];  ///< A backup copy of the state.
uint8_t _prev_mode;  ///< Store of the previously set mode.
bool _send_swing;  ///< Flag indicating if we need to send a swing message.
uint8_t _swing_mode;  ///< The saved swing state/mode/command.

// Constants

// Toshiba A/C
const uint16_t kToshibaAcHdrMark = 4400;
const uint16_t kToshibaAcHdrSpace = 4300;
const uint16_t kToshibaAcBitMark = 580;
const uint16_t kToshibaAcOneSpace = 1600;
const uint16_t kToshibaAcZeroSpace = 490;
// Some models have a different inter-message gap.
const uint16_t kToshibaAcMinGap = 4600;    // WH-UB03NJ remote
const uint16_t kToshibaAcUsualGap = 7400;  // Others




void encode_ToshibaAc(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  if(ac_status.power_status){
    ToshibaAc_setPower(ac_status.power_status);
    ToshibaAc_setTemp(ac_status.temperature);
    ToshibaAc_setMode(ToshibaAc_convertMode(ac_status.mode));
    ToshibaAc_setFan(ToshibaAc_convertFan(ac_status.fan));

    if(ac_status.swing)
      ToshibaAc_setSwing(kToshibaAcSwingOn);
    else
      ToshibaAc_setSwing(kToshibaAcSwingOff);
  }
  else
    ToshibaAc_setPower(ac_status.power_status);
  ToshibaAc_send(ToshibaAc_getRaw(), kToshibaAcStateLength, kToshibaAcMinRepeat, OutputIRProtocol);
  
  setIrTxState(1);
}


void decode_ToshibaAc(int16_t* input, uint8_t* output) {
  
  initDecodeData(input, TOSHIBA_BITS);

  if(ToshibaAc_recv(&gDecodeResult, 0, kToshibaAcBits, true)){
    ToshibaAc_setRaw(gDecodeResult.state, kToshibaAcStateLength);
  }
  output[0] = ToshibaAc_getPower();
  output[1] = ToshibaAc_getTemp();
  output[2] = ToshibaAc_toCommonFanSpeed(ToshibaAc_getFan());
  output[3] = ToshibaAc_getSwing(true);
  output[4] = ToshibaAc_toCommonMode(ToshibaAc_getMode(false));

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan( output[2]);
  ac_control_set_swing(output[3] == kToshibaAcSwingOn);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}


bool isHeader_Toshiba(int16_t *irRaw){

  decode_results results;
  getRawBuf(irRaw, &results, 34); // 34BitLength = 2BytesAddress * 2 + 2BitsHeader

  // Match Header + Data + Footer
  if (!matchGeneric_8(results.rawbuf, results.state,
                      results.rawlen, 16, //2 bytes address
                      kToshibaAcHdrMark, kToshibaAcHdrSpace,
                      kToshibaAcBitMark, kToshibaAcOneSpace,
                      kToshibaAcBitMark, kToshibaAcZeroSpace,
                      0, 0,
                      true, kTolerance, kMarkExcess, true)) return false;

  if(results.state[0] != 0xF2 || results.state[1] != 0x0D) return false;

  return true;
}


/// Send a Toshiba A/C message.
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void ToshibaAc_send(const uint8_t data[], const uint16_t nbytes,
                           const uint16_t repeat, int16_t *irRaw) {
  sendGeneric_8(kToshibaAcHdrMark, kToshibaAcHdrSpace, 
                kToshibaAcBitMark, kToshibaAcOneSpace, 
                kToshibaAcBitMark, kToshibaAcZeroSpace,
                kToshibaAcBitMark, kToshibaAcUsualGap, 
                data, nbytes, 38, true,
                repeat, 50, irRaw);
}


/// Decode the supplied Toshiba A/C message.
/// Status:  STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool ToshibaAc_recv(decode_results* results, uint16_t offset,
                    const uint16_t nbits, const bool strict) {
  // Compliance
  if (strict) {
    switch (nbits) {  // Must be called with the correct nr. of bits.
      case kToshibaAcBits:
      case kToshibaAcBitsShort:
      case kToshibaAcBitsLong:
        break;
      default:
        return false;
    }
  }

  // Match Header + Data + Footer
  if (!matchGeneric_8(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kToshibaAcHdrMark, kToshibaAcHdrSpace,
                      kToshibaAcBitMark, kToshibaAcOneSpace,
                      kToshibaAcBitMark, kToshibaAcZeroSpace,
                      kToshibaAcBitMark, kToshibaAcMinGap,
                      true, kTolerance, kMarkExcess, true)) return false;
  // Compliance
  if (strict) {
    // Check that the checksum of the message is correct.
    if (!ToshibaAc_validChecksum(results->state, nbits / 8)) return false;
  }

  // Success
  results->decode_type = TOSHIBA_AC;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


/// Reset the state of the remote to a known good state/sequence.
void ToshibaAc_stateReset(void) {
  static const uint8_t kReset[kToshibaAcStateLength] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01};
  memcpy(_ToshibaProtocol.raw, kReset, kToshibaAcStateLength);
  ToshibaAc_setTemp(22);  // Remote defaults to 22C after factory reset. So do the same.
  ToshibaAc_setSwing(kToshibaAcSwingOff);
  _prev_mode = ToshibaAc_getMode(false);
}


/// Get the length of the supplied Toshiba state per it's protocol structure.
/// @param[in] state The array to get the built-in length from.
/// @param[in] size The physical size of the state array.
/// @return Nr. of bytes in use for the provided state message.
uint16_t ToshibaAc_getInternalStateLength(const uint8_t state[], const uint16_t size) {
  if (size < kToshibaAcLengthByte) return 0;
  return MIN((uint16_t)(state[kToshibaAcLengthByte] + kToshibaAcMinLength),
                  kToshibaAcStateLengthLong);
}

/// Get the length of the current internal state per the protocol structure.
/// @return Nr. of bytes in use for the current internal state message.
uint16_t ToshibaAc_getStateLength(void){
  return ToshibaAc_getInternalStateLength(_ToshibaProtocol.raw, kToshibaAcStateLengthLong);
}

/// Set the internal length of the current internal state per the protocol.
/// @param[in] size Nr. of bytes in use for the current internal state message.
void ToshibaAc_setStateLength(const uint16_t size) {
  if (size < kToshibaAcMinLength) return;
  _ToshibaProtocol.Length = size - kToshibaAcMinLength;
}

/// Make a copy of the internal code-form A/C state.
void ToshibaAc_backupState(void) {
  memcpy(backup, _ToshibaProtocol.raw, kToshibaAcStateLengthLong);
}

/// Recover the internal code-form A/C state from the backup.
void ToshibaAc_restoreState(void) {
  memcpy(_ToshibaProtocol.raw, backup, kToshibaAcStateLengthLong);
}

/// Get a PTR to the internal state/code for this protocol with all integrity
///   checks passing.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* ToshibaAc_getRaw(void) {
  ToshibaAc_checksum(ToshibaAc_getStateLength());
  return _ToshibaProtocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
/// @param[in] length The length/size of the array.
void ToshibaAc_setRaw(const uint8_t newState[], const uint16_t length) {
  memcpy(_ToshibaProtocol.raw, newState, length);
  _prev_mode = ToshibaAc_getMode(false);
  _send_swing = true;
}

/// Calculate the checksum for a given state.
/// @param[in] state The array to calc the checksum of.
/// @param[in] length The length/size of the array.
/// @return The calculated checksum value.
uint8_t ToshibaAc_calcChecksum(const uint8_t state[],
                                  const uint16_t length) {
  return length ? xorBytes(state, length - 1, 0) : 0;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool ToshibaAc_validChecksum(const uint8_t state[], const uint16_t length) {
  return length >= kToshibaAcMinLength &&
         state[length - 1] == ToshibaAc_calcChecksum(state, length) &&
         checkInvertedBytePairs(state, kToshibaAcInvertedLength) &&
         ToshibaAc_getInternalStateLength(state, length) == length;
}

/// Calculate & set the checksum for the current internal state of the remote.
/// @param[in] length The length/size of the internal array to checksum.
void ToshibaAc_checksum(const uint16_t length) {
  // Stored the checksum value in the last byte.
  if (length >= kToshibaAcMinLength) {
    // Set/clear the short msg bit.
    _ToshibaProtocol.ShortMsg = (ToshibaAc_getStateLength() == kToshibaAcStateLengthShort);
    // Set/clear the long msg bit.
    _ToshibaProtocol.LongMsg = (ToshibaAc_getStateLength() == kToshibaAcStateLengthLong);
    invertBytePairs(_ToshibaProtocol.raw, kToshibaAcInvertedLength);
    // Always do the Xor checksum LAST!
    _ToshibaProtocol.raw[length - 1] = ToshibaAc_calcChecksum(_ToshibaProtocol.raw, length);
  }
}


/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void ToshibaAc_setPower(const bool on) {
  if (on) {  // On
    // If not already on, pick the last non-off mode used
    if (!ToshibaAc_getPower()) ToshibaAc_setMode(_prev_mode);
  } else {  // Off
    ToshibaAc_setMode(kToshibaAcOff);
  }
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool ToshibaAc_getPower(void){
  return (ToshibaAc_getMode(true) != kToshibaAcOff);
}

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
void ToshibaAc_setTemp(const uint8_t degrees) {
  uint8_t temp = kToshibaAcMinTemp;
  if(degrees) // tempVal >= 1 get from app, ac_status.temperature
    temp = MAX(kToshibaAcMinTemp, degrees + 16);
  else       // tempVal == 0(16c) get from app, ac_status.temperature
    temp = kToshibaAcMinTemp;

  temp = MIN(kToshibaAcMaxTemp, temp);
  _ToshibaProtocol.Temp = temp - kToshibaAcMinTemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t ToshibaAc_getTemp(void){ return _ToshibaProtocol.Temp + 1; }

/// Set the speed of the fan.
/// @param[in] speed The desired setting (0 is Auto, 1-5 is the speed, 5 is Max)
void ToshibaAc_setFan(const uint8_t speed) {
  _ToshibaProtocol.Fan = speed;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t ToshibaAc_getFan(void){
  return _ToshibaProtocol.Fan;
}

/// Get the swing setting of the A/C.
/// @param[in] raw Calculate the answer from just the state data.
/// @return The current swing mode setting.
uint8_t ToshibaAc_getSwing(const bool raw){
  return raw ? _ToshibaProtocol.Swing : _swing_mode;
}

/// Set the swing setting of the A/C.
/// @param[in] setting The value of the desired setting.
void ToshibaAc_setSwing(const uint8_t setting) {
  switch (setting) {
    case kToshibaAcSwingStep:
    case kToshibaAcSwingOn:
    case kToshibaAcSwingOff:
    case kToshibaAcSwingToggle:
      _send_swing = true;
      _swing_mode = setting;
      if (ToshibaAc_getStateLength() == kToshibaAcStateLengthShort)
        _ToshibaProtocol.Swing = setting;
  }
}

/// Get the operating mode setting of the A/C.
/// @param[in] raw Get the value without any intelligent processing.
/// @return The current operating mode setting.
uint8_t ToshibaAc_getMode(const bool raw){
  const uint8_t mode = _ToshibaProtocol.Mode;
  if (raw) return mode;
  switch (mode) {
    case kToshibaAcOff: return _prev_mode;
    default:            return mode;
  }
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note If we get an unexpected mode, default to AUTO.
void ToshibaAc_setMode(const uint8_t mode) {
  if (mode != _prev_mode)
      // Changing mode or power turns Econo & Turbo to off on a real remote.
      // Setting the internal message length to "normal" will do that.
    ToshibaAc_setStateLength(kToshibaAcStateLength);
  switch (mode) {
    case kToshibaAcAuto:
    case kToshibaAcCool:
    case kToshibaAcDry:
    case kToshibaAcHeat:
    case kToshibaAcFan:
      _prev_mode = mode;
      // FALL-THRU
    case kToshibaAcOff:
      _ToshibaProtocol.Mode = mode;
      break;
    default:
      _prev_mode = kToshibaAcAuto;
      _ToshibaProtocol.Mode = kToshibaAcAuto;
  }
}

/// Get the Turbo (Powerful) setting of the A/C.
/// @return true, if the current setting is on. Otherwise, false.
bool ToshibaAc_getTurbo(void){
  if (ToshibaAc_getStateLength() == kToshibaAcStateLengthLong)
    return _ToshibaProtocol.EcoTurbo == kToshibaAcTurboOn;
  return false;
}

/// Set the Turbo (Powerful) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// Note: Turbo mode is mutually exclusive with Economy mode.
void ToshibaAc_setTurbo(const bool on) {
  if (on) {
    _ToshibaProtocol.EcoTurbo = kToshibaAcTurboOn;
    ToshibaAc_setStateLength(kToshibaAcStateLengthLong);
  } else {
    if (!ToshibaAc_getEcono()) ToshibaAc_setStateLength(kToshibaAcStateLength);
  }
}

/// Get the Economy mode setting of the A/C.
/// @return true, if the current setting is on. Otherwise, false.
bool ToshibaAc_getEcono(void){
  if (ToshibaAc_getStateLength() == kToshibaAcStateLengthLong)
    return _ToshibaProtocol.EcoTurbo == kToshibaAcEconoOn;
  return false;
}

/// Set the Economy mode setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// Note: Economy mode is mutually exclusive with Turbo mode.
void ToshibaAc_setEcono(const bool on) {
  if (on) {
    _ToshibaProtocol.EcoTurbo = kToshibaAcEconoOn;
    ToshibaAc_setStateLength(kToshibaAcStateLengthLong);
  } else {
    if (!ToshibaAc_getTurbo()) ToshibaAc_setStateLength(kToshibaAcStateLength);
  }
}

/// Get the filter (Pure/Ion Filter) setting of the A/C.
/// @return true, if the current setting is on. Otherwise, false.
bool ToshibaAc_getFilter(void){
  return (ToshibaAc_getStateLength() >= kToshibaAcStateLength) ? _ToshibaProtocol.Filter : false;
}

/// Set the filter (Pure/Ion Filter) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void ToshibaAc_setFilter(const bool on) {
  _ToshibaProtocol.Filter = on;
  if (on) ToshibaAc_setStateLength(MIN(kToshibaAcStateLength, ToshibaAc_getStateLength()));
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t ToshibaAc_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kToshibaAcCool;
    // case kOpModeHeat: return kToshibaAcHeat;  NO HEAT MODE
    case kOpModeDry:  return kToshibaAcDry;
    case kOpModeFan:  return kToshibaAcFan;
    case kOpModeOff:  return kToshibaAcOff;
    default:          return kToshibaAcAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t ToshibaAc_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kToshibaAcFanMin;
    case kFanSpeedLow:    return kToshibaAcFanLow;
    case kFanSpeedMedium: return kToshibaAcFanMed;
    case kFanSpeedHigh:   return kToshibaAcFanHigh;
    case kFanSpeedMax:    return kToshibaAcFanMax;
    default:              return kToshibaAcFanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t ToshibaAc_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kToshibaAcCool: return kOpModeCool;
    case kToshibaAcHeat: return kOpModeHeat;
    case kToshibaAcDry:  return kOpModeDry;
    case kToshibaAcFan:  return kOpModeFan;
    case kToshibaAcOff:  return kOpModeOff;
    default:             return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t ToshibaAc_toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kToshibaAcFanMax:  return kFanSpeedMax;
    case kToshibaAcFanHigh: return kFanSpeedHigh;
    case kToshibaAcFanMed:  return kFanSpeedMedium;
    case kToshibaAcFanLow:  return kFanSpeedLow;
    case kToshibaAcFanMin:  return kFanSpeedMin;
    default:                return kFanSpeedAuto;
  }
}





