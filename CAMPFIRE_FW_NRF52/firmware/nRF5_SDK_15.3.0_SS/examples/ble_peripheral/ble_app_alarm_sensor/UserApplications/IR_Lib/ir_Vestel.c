
/// @file
/// @brief Support for Vestel protocols.

#include "stdio.h"
#include "stdint.h"
#include "string.h"

#include "ir_Vestel.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "app_ac_status.h"
#include "IR_DeviceConstructor.h"

union VestelProtocol _VestelProtocol = {
  .cmdState  = kVestelAcStateDefault,
  .timeState = kVestelAcTimeStateDefaultOff,
};

/* Get values from BLE command then fill to IR protocol */
void encode_VestelAc(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  VestelAc_setPower(ac_status.power_status);
  VestelAc_setTemp(ac_status.temperature);
  VestelAc_setMode(VestelAc_convertMode(ac_status.mode));
  VestelAc_setFan(VestelAc_convertFan(ac_status.fan));
  VestelAc_setSwing(ac_status.swing);

  VestelAc_send(VestelAc_getRaw(), kVestelAcBits, kNoRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_VestelAc(int16_t* input, uint8_t* output) {

  initDecodeData(input, VESTELAC_BITS);
  if( VestelAc_recv(&gDecodeResult, 0, kVestelAcBits, true) ){
    VestelAc_setRaw(gDecodeResult.value);
  }

  output[0] = VestelAc_getPower();
  output[1] = VestelAc_getTemp();
  output[2] = VestelAc_toCommonFanSpeed(_VestelProtocol.Fan);
  output[3] = VestelAc_getSwing();
  output[4] = VestelAc_toCommonMode(_VestelProtocol.Mode);

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3]);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}

/// Send a Vestel message
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void VestelAc_send(const uint64_t data, const uint16_t nbits, const uint16_t repeat, int16_t *irRaw){
  if (nbits % 8 != 0) return;  // nbits is required to be a multiple of 8.

  uint64_t temp;

  if(VestelAc_getPower())
    temp = swapData(kVestelAcTimeStateDefaultOn);
  else
    temp = swapData(kVestelAcTimeStateDefaultOff);
  sendGeneric_64( kVestelAcHdrMark, kVestelAcHdrSpace,   // Header
                  kVestelAcBitMark, kVestelAcOneSpace,   // Data
                  kVestelAcBitMark, kVestelAcZeroSpace,  // Data
                  kVestelAcBitMark, kVestelAcGap,        // Footer + repeat gap
                  0, temp,
                  nbits, 38, false, repeat, 50,
                  irRaw);

  temp = swapData(data);
  sendGeneric_64( kVestelAcHdrMark, kVestelAcHdrSpace,   // Header
                  kVestelAcBitMark, kVestelAcOneSpace,   // Data
                  kVestelAcBitMark, kVestelAcZeroSpace,  // Data
                  kVestelAcBitMark, 10000,        // Footer + repeat gap
                  0, temp,
                  nbits, 38, false, repeat, 50,
                  &irRaw[116]); // 116 = kVestelAcBits*2 + 2header +2footer
}


bool isVestelAc(int16_t *irRaw){
  initDecodeData(irRaw, VESTELAC_BITS);

  if( VestelAc_recv(&gDecodeResult, 0, kVestelAcBits*2, true) ){
    VestelAc_setRaw(gDecodeResult.value);

    if( (gDecodeResult.state[6] == 0xF0 || gDecodeResult.state[6] == 0xC0)
        &&
        (gDecodeResult.state[0] == 0x01 || gDecodeResult.state[0] == 0x01) /* Confirm logic co dung khong? Tu Phan */
        &&
        ((gDecodeResult.state[1] & 0x0F) == 0x02) )
      return true;
  }
  return false;
}

/// Decode the supplied Vestel message.
/// Status: Alpha / Needs testing against a real device.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool VestelAc_recv(decode_results* results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;

  if (strict)
    if (nbits != kVestelAcBits && nbits != kVestelAcBits*2)
      return false;  // Not strictly a Vestel AC message.
 
  uint64_t data[2] = {0};
  uint16_t used = 0;

  if (nbits > sizeof(data) * 8)
    return false;  // We can't possibly capture a Vestel packet that big.

  // Match Header + Data + Footer
  for(int i=0; i<nbits/kVestelAcBits; i++){
    used = matchGeneric_64( results->rawbuf + offset + used, &data[i],
                            results->rawlen - offset - used, kVestelAcBits,
                            kVestelAcHdrMark, kVestelAcHdrSpace,
                            kVestelAcBitMark, kVestelAcOneSpace,
                            kVestelAcBitMark, kVestelAcZeroSpace,
                            kVestelAcBitMark, i?0:kVestelAcGap,
                            false, kTolerance, kMarkExcess, false);
    if(used==0) return 0;
  }
  // Compliance
  if (strict)
    if (!VestelAc_validChecksum(data[0])) return false;
    if ((nbits/kVestelAcBits == 2) && !VestelAc_validChecksum(data[1])) return false; 

  // Success
  results->decode_type = VESTEL_AC;
  results->bits = nbits;
  results->value = data[1];
  // results->value = data[0]; // Timer Frame not support yet
  results->address = 0;
  results->command = 0;

  return true;
}


/// Get a copy of the internal state/code for this protocol.
/// @return A code for this protocol based on the current internal state.
uint64_t VestelAc_getRaw(void) {
  VestelAc_checksum();
  // temporary disable timer frame
  // if (!_VestelProtocol.UseCmd) return _VestelProtocol.timeState;
  return _VestelProtocol.cmdState;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
void VestelAc_setRaw_p8(const uint8_t* newState) {
  uint64_t upState = 0;
  for (int i = 0; i < 7; i++)
    upState |= (uint64_t)(newState[i]) << (i * 8);
  VestelAc_setRaw(upState);
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
void VestelAc_setRaw(const uint64_t newState) {
  _VestelProtocol.cmdState = newState;
  _VestelProtocol.timeState = newState;
  if (VestelAc_isTimeCommand()) {
    _VestelProtocol.cmdState = kVestelAcStateDefault;
    _VestelProtocol.UseCmd = false;
  } else {
    _VestelProtocol.timeState = kVestelAcTimeStateDefaultOff;
  }
}


/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void VestelAc_setPower(const bool on) {
  _VestelProtocol.Power = (on ? 3 : 0);
  _VestelProtocol.UseCmd = true;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool VestelAc_getPower(void){
  return _VestelProtocol.Power;
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void VestelAc_setTemp(const uint8_t temp) {
  uint8_t new_temp = MAX(kVestelAcMinTemp, temp);
  new_temp = MIN(kVestelAcMaxTemp, new_temp);
  _VestelProtocol.Temp = new_temp;
  _VestelProtocol.UseCmd = true;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t VestelAc_getTemp(void){
  return _VestelProtocol.Temp;
}

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
void VestelAc_setFan(const uint8_t fan) {
  switch (fan) {
    case kVestelAcFanLow:
    case kVestelAcFanMed:
    case kVestelAcFanHigh:
    case kVestelAcFanAutoCool:
    case kVestelAcFanAutoHot:
    case kVestelAcFanAuto:
      _VestelProtocol.Fan = fan;
      break;
    default:
      _VestelProtocol.Fan = kVestelAcFanAuto;
  }
  _VestelProtocol.UseCmd = true;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t VestelAc_getFan(void){
  return _VestelProtocol.Fan;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t VestelAc_getMode(void){
  return _VestelProtocol.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note If we get an unexpected mode, default to AUTO.
void VestelAc_setMode(const uint8_t mode) {
  switch (mode) {
    case kVestelAcAuto:
    case kVestelAcCool:
    case kVestelAcHeat:
    case kVestelAcDry:
    case kVestelAcFan:
      _VestelProtocol.Mode = mode;
      break;
    default:
      _VestelProtocol.Mode = kVestelAcAuto;
  }
  _VestelProtocol.UseCmd = true;
}

/// Set Auto mode/level of the A/C.
/// @param[in] autoLevel The auto mode/level setting.
void VestelAc_setAuto(const int8_t autoLevel) {
  if (autoLevel < -2 || autoLevel > 2) return;
  _VestelProtocol.Mode = kVestelAcAuto;
  _VestelProtocol.Fan = (autoLevel < 0 ? kVestelAcFanAutoCool : kVestelAcFanAutoHot);
  if (autoLevel == 2)
    VestelAc_setTemp(30);
  else if (autoLevel == 1)
    VestelAc_setTemp(31);
  else if (autoLevel == 0)
    VestelAc_setTemp(25);
  else if (autoLevel == -1)
    VestelAc_setTemp(16);
  else if (autoLevel == -2)
    VestelAc_setTemp(17);
}

/// Set the timer to be active on the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void VestelAc_setTimerActive(const bool on) {
  _VestelProtocol.Timer = on;
  _VestelProtocol.UseCmd = false;
}

/// Get if the Timer is active on the A/C.
/// @return true, the setting is on. false, the setting is off.
bool VestelAc_isTimerActive(void){
  return _VestelProtocol.Timer;
}

/// Set Timer option of A/C.
/// @param[in] minutes Nr of minutes the timer is to be set for.
/// @note Valid arguments are 0, 0.5, 1, 2, 3 and 5 hours (in minutes).
///   0 disables the timer.
void VestelAc_setTimer(const uint16_t minutes) {
  // Clear both On & Off timers.
  _VestelProtocol.OnHours = 0;
  _VestelProtocol.OnTenMins = 0;
  // Set the "Off" time with the nr of minutes before we turn off.
  _VestelProtocol.OffHours = minutes / 60;
  _VestelProtocol.OffTenMins = (minutes % 60) / 10;
  VestelAc_setOffTimerActive(false);
  // Yes. On Timer instead of Off timer active.
  VestelAc_setOnTimerActive(minutes != 0);
  VestelAc_setTimerActive(minutes != 0);
}

/// Get the Timer time of A/C.
/// @return The number of minutes of time on the timer.
uint16_t VestelAc_getTimer(void){ return VestelAc_getOffTimer(); }

/// Set the A/C's internal clock.
/// @param[in] minutes The time expressed in nr. of minutes past midnight.
void VestelAc_setTime(const uint16_t minutes) {
  _VestelProtocol.Hours = minutes / 60;
  _VestelProtocol.Minutes = minutes % 60;
  _VestelProtocol.UseCmd = false;
}

/// Get the A/C's internal clock's time.
/// @return The time expressed in nr. of minutes past midnight.
uint16_t VestelAc_getTime(void){
  return _VestelProtocol.Hours * 60 + _VestelProtocol.Minutes;
}

/// Set the On timer to be active on the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void VestelAc_setOnTimerActive(const bool on) {
  _VestelProtocol.OnTimer = on;
  _VestelProtocol.UseCmd = false;
}

/// Get if the On Timer is active on the A/C.
/// @return true, the setting is on. false, the setting is off.
bool VestelAc_isOnTimerActive(void){
  return _VestelProtocol.OnTimer;
}

/// Set the On timer time on the A/C.
/// @param[in] minutes Time in nr. of minutes.
void VestelAc_setOnTimer(const uint16_t minutes) {
  VestelAc_setOnTimerActive(minutes);
  _VestelProtocol.OnHours = minutes / 60;
  _VestelProtocol.OnTenMins = (minutes % 60) / 10;
  VestelAc_setTimerActive(false);
}

/// Get the A/C's On Timer time.
/// @return The time expressed in nr. of minutes.
uint16_t VestelAc_getOnTimer(void){
  return _VestelProtocol.OnHours * 60 + _VestelProtocol.OnTenMins * 10;
}

/// Set the Off timer to be active on the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void VestelAc_setOffTimerActive(const bool on) {
  _VestelProtocol.OffTimer = on;
  _VestelProtocol.UseCmd = false;
}

/// Get if the Off Timer is active on the A/C.
/// @return true, the setting is on. false, the setting is off.
bool VestelAc_isOffTimerActive(void){
  return _VestelProtocol.OffTimer;
}

/// Set the Off timer time on the A/C.
/// @param[in] minutes Time in nr. of minutes.
void VestelAc_setOffTimer(const uint16_t minutes) {
  VestelAc_setOffTimerActive(minutes);
  _VestelProtocol.OffHours = minutes / 60;
  _VestelProtocol.OffTenMins = (minutes % 60) / 10;
  VestelAc_setTimerActive(false);
}

/// Get the A/C's Off Timer time.
/// @return The time expressed in nr. of minutes.
uint16_t VestelAc_getOffTimer(void){
  return _VestelProtocol.OffHours * 60 + _VestelProtocol.OffTenMins * 10;
}

/// Set the Sleep setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void VestelAc_setSleep(const bool on) {
  _VestelProtocol.TurboSleep = (on ? kVestelAcSleep : kVestelAcNormal);
  _VestelProtocol.UseCmd = true;
}

/// Get the Sleep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool VestelAc_getSleep(void){
  return _VestelProtocol.TurboSleep == kVestelAcSleep;
}

/// Set the Turbo setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void VestelAc_setTurbo(const bool on) {
  _VestelProtocol.TurboSleep = (on ? kVestelAcTurbo : kVestelAcNormal);
  _VestelProtocol.UseCmd = true;
}

/// Get the Turbo setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool VestelAc_getTurbo(void){
  return _VestelProtocol.TurboSleep == kVestelAcTurbo;
}

/// Set the Ion (Filter) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void VestelAc_setIon(const bool on) {
  _VestelProtocol.Ion = on;
  _VestelProtocol.UseCmd = true;
}

/// Get the Ion (Filter) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool VestelAc_getIon(void){
  return _VestelProtocol.Ion;
}

/// Set the Swing Roaming setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void VestelAc_setSwing(const bool on) {
  _VestelProtocol.Swing = (on ? kVestelAcSwing : 0xF);
  _VestelProtocol.UseCmd = true;
}

/// Get the Swing Roaming setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool VestelAc_getSwing(void){
  return _VestelProtocol.Swing == kVestelAcSwing;
}

/// Calculate the checksum for a given state.
/// @param[in] state The state to calc the checksum of.
/// @return The calculated checksum value.
uint8_t VestelAc_calcChecksum(const uint64_t state) {
  // Just counts the set bits +1 on stream and take inverse after mask
  return 0xFF - countBits_64(GETBITS64(state, 20, 44), 44, true, 2);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The state to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool VestelAc_validChecksum(const uint64_t state) {
  union VestelProtocol vp;
  vp.cmdState = state;
  return vp.CmdSum == VestelAc_calcChecksum(state);
}

/// Calculate & set the checksum for the current internal state of the remote.
void VestelAc_checksum(void) {
  // Stored the checksum value in the last byte.
  _VestelProtocol.CmdSum =  VestelAc_calcChecksum(_VestelProtocol.cmdState);
  _VestelProtocol.TimeSum = VestelAc_calcChecksum(_VestelProtocol.timeState);
}

/// Is the current state a time command?
/// @return true, if the state is a time message. Otherwise, false.
bool VestelAc_isTimeCommand(void){
  return !_VestelProtocol.UseCmd;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t VestelAc_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kVestelAcCool;
    case kOpModeHeat: return kVestelAcHeat;
    case kOpModeDry:  return kVestelAcDry;
    case kOpModeFan:  return kVestelAcFan;
    default:          return kVestelAcAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t VestelAc_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kVestelAcFanLow;
    case kFanSpeedLow:    return kVestelAcFanMed;
    case kFanSpeedMedium:
    case kFanSpeedHigh:
    case kFanSpeedMax:    return kVestelAcFanHigh;
    default:              return kVestelAcFanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t VestelAc_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kVestelAcCool: return kOpModeCool;
    case kVestelAcHeat: return kOpModeHeat;
    case kVestelAcDry:  return kOpModeDry;
    case kVestelAcFan:  return kOpModeFan;
    default:            return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t VestelAc_toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kVestelAcFanHigh: return kFanSpeedMax;
    case kVestelAcFanMed:  return kFanSpeedMedium;
    case kVestelAcFanLow:  return kFanSpeedMin;
    default:               return kFanSpeedAuto;
  }
}

/// Convert the current internal state into its state_t equivalent.
/// @return The stdAc equivalent of the native settings.
state_t VestelAc_toCommon(void){
  state_t result = {0};
  result.protocol = VESTEL_AC;
  result.model    = -1;  // Not supported.
  result.power    = _VestelProtocol.Power;
  result.mode     = VestelAc_toCommonMode(_VestelProtocol.Mode);
  result.celsius  = true;
  result.degrees  = VestelAc_getTemp();
  result.fanspeed = VestelAc_toCommonFanSpeed(_VestelProtocol.Fan);
  result.swingv   = (VestelAc_getSwing() ? kSwingVAuto : kSwingVOff);
  result.turbo    = VestelAc_getTurbo();
  result.filter   = _VestelProtocol.Ion;
  result.sleep    = (VestelAc_getSleep() ? 0 : -1);
  // Not supported.
  result.swingh   = kSwingHOff;
  result.light    = false;
  result.econo    = false;
  result.quiet    = false;
  result.clean    = false;
  result.beep     = false;
  result.clock    = -1;
  return result;
}



static uint64_t swapData(uint64_t input){
  uint64_t x = input;
  uint8_t *ptr = (uint8_t *)&x;
  uint64_t ret = 0;

  for(int i=1; i<=7; i++){
    ret |= (uint64_t)((uint64_t)(*ptr) << (56 - i*8));
    ptr++;
  }

  return x;
}


