// Copyright 2018-2021 David Conran
/// @file
/// @brief Support for Casper104 A/C protocols.

#include "ir_Casper104.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "app_ac_status.h"
#include "IR_Common.h"


Casper104Protocol_t _Casper104Protocol = { .raw = {0xC3, 0x07, 0xE0, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x45, 0xAF} };
ac_status_t _SavedAcStatus;

static int s_IrEnableTx;

/* Get values from BLE command then fill to IR protocol */
void encode_Casper104(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  Casper104_setPower(ac_status.power_status);
  Casper104_setMode(Casper104_convertMode(ac_status.mode));
  Casper104_setTemp(ac_status.temperature); // setTemp must to perform after setMode
  Casper104_setFan(Casper104_convertFan(ac_status.fan));
  Casper104_setSwingV(ac_status.swing);

  Casper104_setButton();

  _SavedAcStatus = ac_status;
  Casper104_send(Casper104_getRaw(), kCasper104StateLength, 0, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Casper104(int16_t* input, uint8_t* output) {

  initDecodeData(input, CASPER104_BITS);
  if( Casper104_recv(&gDecodeResult, 0, kCasper104Bits, true) ){
    Casper104_setRaw(gDecodeResult.state);
  }

  /* ON/OFF */
  output[0] = Casper104_getPower();
  output[1] = Casper104_getTemp();
  output[2] = Casper104_toCommonFanSpeed(Casper104_getFan());
  output[3] = Casper104_getSwingV();
  output[4] = Casper104_toCommonMode( Casper104_getMode() );

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3]);
  ac_control_set_mode(output[4]);

  _SavedAcStatus = ac_status;
  ac_control_update_status_to_payload();
}



/// Send a Casper104 A/C formatted message.
/// Status: Alpha / Needs testing against a real device.
/// @param[in] data The message to be sent.
/// @note Guessing MSBF order.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Casper104_send(const uint8_t data[], const uint16_t nbytes,
                           const uint16_t repeat, int16_t *irRaw) {
    sendGeneric_8( kCasper104HdrMark, kCasper104HdrSpace,
                    kCasper104BitMark, kCasper104OneSpace, kCasper104BitMark, kCasper104ZeroSpace,
                    kCasper104BitMark, kCasper104MessageGap,
                    data, nbytes,
                    38000,  // Complete guess of the modulation frequency.
                    false,  // Send data in LSB order per byte
                    0, 50,
                    irRaw);
}

/// Decode the supplied Casper104 A/C message.
/// Status: STABLE / Known working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Casper104_recv(decode_results *results, uint16_t offset,
                             const uint16_t nbits,
                             const bool strict) {
  if (strict) {
    if (nbits != kCasper104Bits)
      return false;  // Not strictly
  }

  // Match Header + Data + Footer
  if (!matchGeneric_8(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kCasper104HdrMark, kCasper104HdrSpace,
                      kCasper104BitMark, kCasper104OneSpace,
                      kCasper104BitMark, kCasper104ZeroSpace,
                      kCasper104BitMark, kCasper104MessageGap, true,
                      kTolerance, 0, false)) return false;

  // Compliance
  if (strict) {
    // Verify the checksum.
    if (!Casper104_validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = CASPER;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


bool Casper104_isHeaderMatch(int16_t *rawbuf){
  initDecodeData(rawbuf, CASPER104_BITS);
  if(Casper104_recv(&gDecodeResult, 0, kCasper104Bits, true))
    return (gDecodeResult.state[0] == 0xC3);
  else
    return false;
}

/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @param[in] length The length of the state array.
/// @return The calculated checksum stored in a uint_8.
uint8_t Casper104_calcChecksum(const uint8_t state[]) {
  return sumBytes(state, kCasper104StateLength - 1, 0);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The state to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Casper104_validChecksum(const uint8_t state[]) {
  return (state[kCasper104StateLength - 1] == Casper104_calcChecksum(state));
}

/// Calculate and set the checksum values for the internal state.
/// @param[in] length The length of the state array.
void Casper104_checksum() {
  _Casper104Protocol.Sum = Casper104_calcChecksum(_Casper104Protocol.raw);
}


/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Casper104_getRaw(void) {
  Casper104_checksum();
  return _Casper104Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] length The length of the code array.
void Casper104_setRaw(const uint8_t new_code[]) {
  memcpy(_Casper104Protocol.raw, new_code, kCasper104StateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper104_setPower(const bool on) {
  _Casper104Protocol.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getPower(void){
  return _Casper104Protocol.Power;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Casper104_setMode(const uint8_t mode) {
  _Casper104Protocol.Heat = 0;
  switch (mode) {
    case kCasper104Heat:
      _Casper104Protocol.Heat = 1;
      _Casper104Protocol.Mode = mode;
      break;
    case kCasper104Auto:
    case kCasper104Fan:
    case kCasper104Dry:
    case kCasper104Cool:
    default:
      // If we get an unexpected mode, default to AUTO.
      _Casper104Protocol.Mode = mode;
      break;
  }
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Casper104_getMode(void){
  return _Casper104Protocol.Mode;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Casper104_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kCasper104Cool;
    case kOpModeHeat: return kCasper104Heat;
    case kOpModeDry:  return kCasper104Dry;
    case kOpModeFan:  return kCasper104Fan;
    default:          return kCasper104Auto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Casper104_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kCasper104Cool: return kOpModeCool;
    case kCasper104Heat: return kOpModeHeat;
    case kCasper104Dry:  return kOpModeDry;
    case kCasper104Fan:  return kOpModeFan;
    default:             return kOpModeAuto;
  }
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void Casper104_setTemp(const uint8_t temp) {
  uint8_t newtemp = temp + kCasper104MinTemp;
  newtemp = MAX(kCasper104MinTemp, newtemp);
  newtemp = MIN(kCasper104MaxTemp, newtemp) - kCasper104TempDelta;

  if(_Casper104Protocol.Mode == kCasper104Auto || _Casper104Protocol.Mode == kCasper104Fan)
    _Casper104Protocol.Temp = 0;
  else{
    _Casper104Protocol.Temp = newtemp;
  }
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Casper104_getTemp(void){
  if(_Casper104Protocol.Mode == kCasper104Auto || _Casper104Protocol.Mode == kCasper104Fan)
    return 0;
  else
    return _Casper104Protocol.Temp + kCasper104TempDelta - kCasper104MinTemp;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
/// @note 0 is auto, 1-3 is the speed
void Casper104_setFan(const uint8_t speed) {

  switch (Casper104_getMode())
  {
  case kCasper104Auto:
    _Casper104Protocol.Fan = kCasper104FanAuto;
    break;
  case kCasper104Dry:
    _Casper104Protocol.Fan = kCasper104FanLow;
    break;
  default:
    switch (speed) {
      case kCasper104FanAuto:
      case kCasper104FanHigh:
      case kCasper104FanMed:
      case kCasper104FanLow:
        _Casper104Protocol.Fan = speed;
        break;
      default:
        // If we get an unexpected speed, default to Auto.
        _Casper104Protocol.Fan = kCasper104FanAuto;
    }
    break;
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Casper104_getFan(void){
  return _Casper104Protocol.Fan;
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Casper104_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kCasper104FanLow;
    case kFanSpeedLow:    return kCasper104FanMed;
    case kFanSpeedMedium:
    case kFanSpeedHigh:
    case kFanSpeedMax:    return kCasper104FanHigh;
    default:              return kCasper104FanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Casper104_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kCasper104FanHigh: return kFanSpeedMedium; // kFanSpeedMax
    case kCasper104FanMed:  return kFanSpeedLow;
    case kCasper104FanLow:  return kFanSpeedMin;
    default:                return kFanSpeedAuto;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper104_setSwingV(const bool on) {
  _Casper104Protocol.SwingV = (on ? kCasper104SwingOn : kCasper104SwingOff);
}

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getSwingV(void){
  return !_Casper104Protocol.SwingV;
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper104_setSwingH(const bool on) {
  _Casper104Protocol.SwingH = (on ? kCasper104SwingOn : kCasper104SwingOff);
}

/// Get the Horizontal Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getSwingH(void){
  return !_Casper104Protocol.SwingH;
}

/// Set the Light (LED) Toggle mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper104_setLightToggle(const bool on) {
  _Casper104Protocol.Light = on;
}

/// Get the Light (LED) Toggle mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getLightToggle(void){
  return _Casper104Protocol.Light;
}

/// Set the Clean mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper104_setClean(const bool on) {
  _Casper104Protocol.Clean = on;
}

/// Get the Clean mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getClean(void){
  return _Casper104Protocol.Clean;
}

/// Set the Turbo mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper104_setTurbo(const bool on) {
  _Casper104Protocol.Turbo = on;
}

/// Get the Turbo mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getTurbo(void){
  return _Casper104Protocol.Turbo;
}

/// Set the Sleep mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
void Casper104_setSleep(const bool on){
  _Casper104Protocol.Sleep = on;
}

/// Get the Sleep mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getSleep(void){
  return _Casper104Protocol.Sleep;
}

/// Set the Sleep mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
void Casper104_setHeath(const bool on){
  _Casper104Protocol.Heath = on;
}

/// Get the Sleep mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getHeath(void){
  return _Casper104Protocol.Heath;
}

/// Get the IFeel mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getIFeel(void){ return _Casper104Protocol.IFeel; }

/// Set the IFeel mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper104_setIFeel(const bool on) {
  _Casper104Protocol.IFeel = on;
  if (_Casper104Protocol.IFeel)
    // Make sure there is a reasonable value in _Casper104Protocol.SensorTemp
    Casper104_setSensorTemp(Casper104_getSensorTemp());
  else
    // Clear any previous stored temp..
    _Casper104Protocol.SensorTemp = kCasper104SensorMinTemp;
}

/// Get the silent Sensor Update setting of the message.
/// i.e. Is this _just_ a sensor temp update message from the remote?
/// @note The A/C just takes the sensor temp value from the message and
/// will not follow any of the other settings in the message.
/// @return true, the setting is on. false, the setting is off.
bool Casper104_getSensorUpdate(void){ return _Casper104Protocol.SensorUpdate; }

/// Set the silent Sensor Update setting of the message.
/// i.e. Is this _just_ a sensor temp update message from the remote?
/// @note The A/C will just take the sensor temp value from the message and
/// will not follow any of the other settings in the message. If set, the A/C
/// unit will also not beep in response to the message.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper104_setSensorUpdate(const bool on) { _Casper104Protocol.SensorUpdate = on; }

/// Set the Sensor temperature for the IFeel mode.
/// @param[in] temp The temperature in degrees celsius.
void Casper104_setSensorTemp(const uint8_t temp) {
  _Casper104Protocol.SensorTemp = MIN(kCasper104SensorMaxTemp, MAX(kCasper104SensorMinTemp, temp)) +
                                    kCasper104SensorTempDelta;
}

/// Get the current sensor temperature setting for the IFeel mode.
/// @return The current setting for temp. in degrees celsius.
uint8_t Casper104_getSensorTemp(void){
  return MAX(kCasper104SensorTempDelta, _Casper104Protocol.SensorTemp) -
        kCasper104SensorTempDelta;
}


uint8_t Casper104_getButton(void){
  if(     _SavedAcStatus.mode         != ac_status.mode)         return kCasper104ButtonMode;
  else if(_SavedAcStatus.fan          != ac_status.fan)          return kCasper104ButtonFan;
  else if(_SavedAcStatus.temperature  >  ac_status.temperature)  return kCasper104ButtonTempDown;
  else if(_SavedAcStatus.temperature  <  ac_status.temperature)  return kCasper104ButtonTempUp;
  else if(_SavedAcStatus.power_status != ac_status.power_status) return kCasper104ButtonOnOff;
}


void Casper104_setButton(void){
  _Casper104Protocol.Button = Casper104_getButton();
}



