// Copyright 2018-2021 David Conran
/// @file
/// @brief Support for Reetech A/C protocols.

#include "ir_Reetech.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "stdint.h"
#include "app_ac_status.h"
#include "IR_Common.h"

ReetechProtocol_t _ReetechProtocol = { .raw = {0xD5, 0x2A, 0x29, 0xD6, 0x6A, 0x95, 0x02, 0xFD, 0x00, 0xFF, 0x00, 0xFF} };
// ReetechProtocol_t _ReetechProtocol = { .raw = {0xD5, 0x2A, 0x29, 0xD6, 0x6A, 0x95, 0x02, 0xFD, 0x00, 0xFF, 0x00, 0xFF} };
static uint8_t _rawMSB[kReetechAcStateLength] = { 0xFF, 0x00, 0xFF, 0x00, 0xFD, 0x02, 0x95, 0x6A, 0xD6, 0x29, 0x2A, 0xD5}; 
bool isDecoded=false;
ac_status_t _savedStatus;

// Constants
const uint16_t kReetechAcHdrMark    = 6160;
const uint16_t kReetechAcHdrSpace   = 7400;
const uint16_t kReetechAcBitMark    = 560;
const uint16_t kReetechAcOneSpace   = 1640;
const uint16_t kReetechAcZeroSpace  = 560;
const uint16_t kReetechAcMessageGap = 7400;  // Just a guess.



void encode_ReetechAc(uint8_t* InputBleCommands, int16_t* OutputIRProtocol){
  ReetechAc_setPower(ac_status.power_status);
  ReetechAc_setTemp(ac_status.temperature);
  ReetechAc_setMode(ReetechAc_convertMode(ac_status.mode));
  ReetechAc_setFan(ReetechAc_convertFan(ac_status.fan));
  if(ac_status.swing)
    ReetechAc_setSwingV(kReetechAcSwingAuto);
  else
    ReetechAc_setSwingV(kReetechAcSwingOff);

  ReetechAc_setButton();
  ReetechAc_send(ReetechAc_getRaw(), kReetechAcStateLength, kReetechAcMinRepeat, OutputIRProtocol);
  
  setIrTxState(1);
  _savedStatus = ac_status;
}


void decode_ReetechAc(int16_t* input, uint8_t* output) {
  initDecodeData(input, REETECH_BITS);

  if(!isDecoded){
    if(ReetechAc_recv(&gDecodeResult, 0, kReetechAcBits, true)){
      ReetechAc_setRaw(gDecodeResult.state);
    }
  }
  else{
    isDecoded = false;
  }
  output[0] = ReetechAc_getPower();
  output[1] = ReetechAc_getTemp();
  output[2] = ReetechAc_toCommonFanSpeed(ReetechAc_getFan());
  output[3] = ReetechAc_getSwingV();
  output[4] = ReetechAc_toCommonMode(ReetechAc_getMode());

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3] == kReetechAcSwingAuto);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
  _savedStatus = ac_status;
}


bool isHeader_Reetech(int16_t *irRaw){

  initDecodeData(irRaw, REETECH_BITS);

  if(ReetechAc_recv(&gDecodeResult, 0, kReetechAcBits, true)){
    if( gDecodeResult.state[kReetechAcStateLength-1] == 0xD5 && 
        gDecodeResult.state[kReetechAcStateLength-2] == 0x2A){
      ReetechAc_setRaw(gDecodeResult.state);
      isDecoded = true;
      return true;
    }
  }
  isDecoded = false;
  return false;
}



/// Send a Reetech A/C formatted message.
/// Status: Alpha / Needs testing against a real device.
/// @param[in] data The message to be sent.
/// @note Guessing MSBF order.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void ReetechAc_send(const uint8_t data[], const uint16_t nbytes,
                           const uint16_t repeat, int16_t *irRaw) {
  sendGeneric_8(kReetechAcHdrMark, kReetechAcHdrSpace,
                kReetechAcBitMark, kReetechAcOneSpace,
                kReetechAcBitMark, kReetechAcZeroSpace,
                kReetechAcBitMark, kReetechAcMessageGap,
                data, nbytes,
                38000,  // Complete guess of the modulation frequency.
                false,  // Send data in LSB order per byte
                0, 50,
                irRaw);
  *(irRaw + REETECH_BITS - 1) = kReetechAcBitMark;
  *(irRaw + REETECH_BITS)     = -10000; // guess gap
}


/// Decode the supplied Reetech A/C message.
/// Status: STABLE / Known working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool ReetechAc_recv(decode_results *results, uint16_t offset,
                             const uint16_t nbits,
                             const bool strict) {
  if (strict) {
    if (nbits != kReetechAcBits)
      return false;  // Not strictly a ELECTRA_AC message.
  }

  // Match Header + Data + Footer
if (!matchGeneric_8(results->rawbuf, results->state,
                    results->rawlen, nbits,
                    kReetechAcHdrMark, kReetechAcHdrSpace,
                    kReetechAcBitMark, kReetechAcOneSpace,
                    kReetechAcBitMark, kReetechAcZeroSpace,
                    kReetechAcBitMark, kReetechAcMessageGap,
                    true, kTolerance, kMarkExcess, false)) return false;

  // Compliance
  if (strict) {
    // Verify the checksum.
    if (!ReetechAc_validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = CHIGO96AC;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


/// Verify the checksum is valid for a given state.
/// @param[in] state The state to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool ReetechAc_validChecksum(const uint8_t state[]) {
  for(int i=0; i<kReetechAcStateLength; i+=2){
    if(state[i] != (state[i+1] ^ 0xFF))
      return false;
  }
  return true;
}

/// Calculate and set the checksum values for the internal state.
/// @param[in] length The length of the state array.
void ReetechAc_checksum() {
  for(int i=0; i<kReetechAcStateLength; i+=2){
    _ReetechProtocol.raw[i+1] =_ReetechProtocol.raw[i] ^ 0xFF;
  }
}


/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *ReetechAc_getRaw(void) {
  ReetechAc_checksum();
	for(int i=0; i<kReetechAcStateLength; i++) _rawMSB[i] = _ReetechProtocol.raw[kReetechAcStateLength -i-1];
  return _rawMSB;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] length The length of the code array.
void ReetechAc_setRaw(const uint8_t new_code[]) {
  for(int i=0; i<kReetechAcStateLength; i++) _ReetechProtocol.raw[i] = new_code[kReetechAcStateLength-i-1];
  // memcpy(_ReetechProtocol.raw, new_code, kReetechAcStateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void ReetechAc_setPower(const bool on) {
  _ReetechProtocol.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool ReetechAc_getPower(void){
  return _ReetechProtocol.Power;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void ReetechAc_setMode(const uint8_t mode) {
  switch (mode) {
    case kReetechAcAuto:
    case kReetechAcDry:
    case kReetechAcCool:
    case kReetechAcHeat:
    case kReetechAcFan:
      _ReetechProtocol.Mode = mode;
      break;
    default:
      // If we get an unexpected mode, default to AUTO.
      _ReetechProtocol.Mode = kReetechAcAuto;
  }
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t ReetechAc_getMode(void){
  return _ReetechProtocol.Mode;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t ReetechAc_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kReetechAcCool;
    case kOpModeHeat: return kReetechAcHeat;
    case kOpModeDry:  return kReetechAcDry;
    case kOpModeFan:  return kReetechAcFan;
    default:          return kReetechAcAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t ReetechAc_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kReetechAcCool: return kOpModeCool;
    case kReetechAcHeat: return kOpModeHeat;
    case kReetechAcDry:  return kOpModeDry;
    case kReetechAcFan:  return kOpModeFan;
    default:             return kOpModeAuto;
  }
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void ReetechAc_setTemp(const uint8_t temp) {
  uint8_t newtemp = MAX(kReetechAcMinTemp, temp);
  newtemp = MIN(kReetechAcMaxTemp, newtemp);
  _ReetechProtocol.Temp = newtemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t ReetechAc_getTemp(void){
  return _ReetechProtocol.Temp;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
/// @note 0 is auto, 1-3 is the speed
void ReetechAc_setFan(const uint8_t speed) {
  switch (speed) {
    case kReetechAcFanAuto:
    case kReetechAcFanHigh:
    case kReetechAcFanMed:
    case kReetechAcFanLow:
      _ReetechProtocol.Fan = speed;
      break;
    default:
      // If we get an unexpected speed, default to Auto.
      _ReetechProtocol.Fan = kReetechAcFanAuto;
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t ReetechAc_getFan(void){
  return _ReetechProtocol.Fan;
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t ReetechAc_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kReetechAcFanLow;
    case kFanSpeedLow:    return kReetechAcFanMed;
    case kFanSpeedMedium:
    case kFanSpeedHigh:
    case kFanSpeedMax:    return kReetechAcFanHigh;
    default:              return kReetechAcFanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t ReetechAc_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kReetechAcFanHigh: return kFanSpeedMedium; // kFanSpeedMax
    case kReetechAcFanMed:  return kFanSpeedLow;
    case kReetechAcFanLow:  return kFanSpeedMin;
    default:                return kFanSpeedAuto;
  }
}

uint8_t  ReetechAc_convertSwing(const swingv_t swing){
  switch (swing){
    case kSwingVOff:     return kReetechAcSwingOff;
    case kSwingVAuto:    return kReetechAcSwingAuto;
    case kSwingVHighest:
    case kSwingVHigh:
    case kSwingVMiddle:
    case kSwingVLow:
    case kSwingVLowest: return kReetechAcSwingMax;
    default:            return kReetechAcSwingOff;
  }
}

swingv_t ReetechAc_toCommonSwing(const uint8_t swing){
  switch(swing){
    case kReetechAcSwingOff:  return kSwingVOff;
    case kReetechAcSwingAuto: return kSwingVAuto;
    case kReetechAcSwingMax:  return kSwingVHighest;
    default:                  return kSwingVOff;
  }
}
/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void ReetechAc_setSwingV(const swingv_t swing) {
  switch (swing){
    case kReetechAcSwingMax:
    case kReetechAcSwingAuto:
    case kReetechAcSwingOff:
      _ReetechProtocol.SwingV = swing;
      break;
    default:
      _ReetechProtocol.SwingV = kReetechAcSwingOff;
      break;
  }
}

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool ReetechAc_getSwingV(void){
  return _ReetechProtocol.SwingV;
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void ReetechAc_setSwingH(const bool on) {
  _ReetechProtocol.SwingH = on;
}

/// Get the Horizontal Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool ReetechAc_getSwingH(void){
  return _ReetechProtocol.SwingH;
}

/// Set the Light (LED) Toggle mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void ReetechAc_setLight(const bool on) {
  _ReetechProtocol.Light = on;
}

/// Get the Light (LED) Toggle mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool ReetechAc_getLight(void){
  return _ReetechProtocol.Light;
}


/// Set the Turbo mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void ReetechAc_setStrong(const bool on) {
  _ReetechProtocol.Strong = on;
}

/// Get the Turbo mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool ReetechAc_getStrong(void){
  return _ReetechProtocol.Strong;
}


void ReetechAc_setButton(void){

  uint8_t button_id = 0;

  if(     ac_status.power_status!= _savedStatus.power_status)button_id = kReetechAcButtonPower;
  else if(ac_status.mode        != _savedStatus.mode)        button_id = kReetechAcButtonMode;
  else if(ac_status.temperature >  _savedStatus.temperature) button_id = kReetechAcButtonTempUp;
  else if(ac_status.temperature <  _savedStatus.temperature) button_id = kReetechAcButtonTempDown;
  else if(ac_status.fan         != _savedStatus.fan)         button_id = kReetechAcButtonFan;
  else if(ac_status.swing       != _savedStatus.swing)       button_id = kReetechAcButtonSwingV;

  _ReetechProtocol.Button = button_id;
}


