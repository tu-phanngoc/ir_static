// Copyright 2018 David Conran

/// @file
/// @brief Support for Casper343 protocols.

#include "ir_Casper343.h"
#include <string.h>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"
#include "IRcommon.h"

#include "app_ac_status.h"
#include "IR_Common.h"


Casper343Protocol_t _Casper343Protocol = { .raw = { 0x83, 0x06, 0x00, 0x82, 0x00, 0x00,
                                                    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                                                    0x00, 0x06, 0x00, 0x00, 0x38, 0x00, 0x3E } };
uint8_t _desiredtemp;  ///< The last user explicitly set temperature.

ac_status_t _saveAcStatus;
static bool _isDecoded = false;

// Constants
const uint16_t kCasper343HdrMark    = 9000;
const uint16_t kCasper343HdrSpace   = 4500;
const uint16_t kCasper343OneSpace   = 1725;
const uint16_t kCasper343BitMark    = 570;
const uint16_t kCasper343ZeroSpace  = 570;
const uint16_t kCasper343Gap        = 8000;
const uint32_t kCasper343MinGap     = kDefaultMessageGap;  // Just a guess.
const uint8_t kCasper343Sections    = 3;


/* Get values from BLE command then fill to IR protocol */
void encode_Casper343(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  Casper343_setPower(ac_status.power_status);
  Casper343_setTemp(ac_status.temperature, true);
  Casper343_setMode(Casper343_convertMode(ac_status.mode));
  Casper343_setFan(ac_status.fan);
  Casper343_setSwingV(ac_status.swing);
  Casper343_setCommand();
  Casper343_send(Casper343_getRaw(true), kCasper343StateLength, 0, OutputIRProtocol);
  Casper343_ClearData();
  
  _saveAcStatus = ac_status;

  setIrTxState(1);
}


void decode_Casper343(int16_t* input, uint8_t* output) {

  initDecodeData(input, CASPER343_BITS);

  if( Casper343_recv(&gDecodeResult, 0, kCasper343Bits, true) ){
    Casper343_setRaw(gDecodeResult.state, kCasper343StateLength);
  }

  output[0] = Casper343_getPower();
  output[1] = Casper343_getTemp();
  output[2] = Casper343_getFan();
  output[3] = Casper343_getSwingV();
  output[4] = Casper343_toCommonMode( Casper343_getMode() );

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3]);
  ac_control_set_mode(output[4]);

  _saveAcStatus.power_status =  Casper343_getPower();
  _saveAcStatus.temperature  =  Casper343_getTemp();
  _saveAcStatus.fan          =  Casper343_getFan();
  _saveAcStatus.swing        =  Casper343_getSwingV();
  _saveAcStatus.mode         =  Casper343_toCommonMode( Casper343_getMode() );

  ac_control_update_status_to_payload();
}


/// Send a Casper343 A/C message.
/// Status: BETA / Probably works.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Casper343_send(const unsigned char data[], const uint16_t nbytes,
                             const uint16_t repeat, int16_t *irRaw) {
  uint16_t offset = 0;
  if (nbytes < kCasper343StateLength)
    return;  // Not enough bytes to send a proper message.

  // Section 1
  sendGeneric_8(kCasper343HdrMark, kCasper343HdrSpace,
                kCasper343BitMark, kCasper343OneSpace, kCasper343BitMark, kCasper343ZeroSpace,
                kCasper343BitMark, kCasper343Gap,
                data, 6,  // 6 bytes == 48 bits
                38000,    // Complete guess of the modulation frequency.
                false, 0, 50, &irRaw[offset]);
  offset += 48*2 + 4; //index = 100
  // Section 2
  sendGeneric_8(0, 0, 
                kCasper343BitMark, kCasper343OneSpace,
                kCasper343BitMark, kCasper343ZeroSpace,
                kCasper343BitMark, kCasper343Gap,
                data + 6, 8,  // 8 bytes == 64 bits
                38000,  // Complete guess of the modulation frequency.
                false, 0, 50, &irRaw[offset]);
  // Section 3
  offset += 64*2 + // data bitcounts
            2;     // header bitcounts
            //index = 100 + 130
  sendGeneric_8(0, 0, kCasper343BitMark, kCasper343OneSpace,
              kCasper343BitMark, kCasper343ZeroSpace, kCasper343BitMark,
              kCasper343MinGap, data + 14, 7,  // 7 bytes == 56 bits
              38000,  // Complete guess of the modulation frequency.
              false, 0, 50, &irRaw[offset]);
}

/// Decode the supplied Casper343 A/C message.
/// Status: STABLE / Working as intended.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool Casper343_recv(decode_results *results, uint16_t offset,
                               const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + 4 + kHeader + kFooter - 1 + offset)
    return false;  // Can't possibly be a valid Casper343 A/C message.
  if (strict) {
    if (nbits != kCasper343Bits) return false;
  }

  const uint8_t sectionSize[kCasper343Sections] = {6, 8, 7};

  // Header
  if (!matchMark(results->rawbuf[offset++], kCasper343HdrMark, kTolerance)) return false;
  if (!matchSpace(results->rawbuf[offset++], kCasper343HdrSpace, kTolerance))
    return false;

  // Data Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kCasper343Sections; section++) {
    uint16_t used;
    // Section Data
    used = matchGeneric_8(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, sectionSize[section] * 8,
                        0, 0,
                        kCasper343BitMark, kCasper343OneSpace,
                        kCasper343BitMark, kCasper343ZeroSpace,
                        kCasper343BitMark, kCasper343Gap,
                        section >= kCasper343Sections - 1,
                        kTolerance, kMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += sectionSize[section];
  }

  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    if (pos * 8 != nbits) return false;
    if (!Casper343_validChecksum(results->state, nbits / 8))
      return false;
  }

  // Success
  results->decode_type = CASPER;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}



bool isCapser343(int16_t *irRaw){
  initDecodeData(irRaw, CASPER343_BITS);

  if( Casper343_recv(&gDecodeResult, 0, kCasper343Bits, true) ){
    if(gDecodeResult.state[0] != 0x83 && gDecodeResult.state[1] != 0x06)
      return false;
    else
      return true;
  }
  else{
    return false;
  }
}


// Class for emulating a Casper343 A/C remote.


/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Casper343_validChecksum(const uint8_t state[],
                                  const uint16_t length) {
  if (length > kCasper343ChecksumByte1 &&
      state[kCasper343ChecksumByte1] != xorBytes(state + 2, kCasper343ChecksumByte1 - 1 - 2, 0)) {
    return false;
  }
  if (length > kCasper343ChecksumByte2 &&
      state[kCasper343ChecksumByte2] != xorBytes(state + kCasper343ChecksumByte1 + 1, 
      kCasper343ChecksumByte2 - kCasper343ChecksumByte1 - 1, 0)) {
    return false;
  }
  // State is too short to have a checksum or everything checked out.
  return true;
}

/// Calculate & set the checksum for the current internal state of the remote.
/// @param[in] length The length/size of the internal state array.
void Casper343_checksum(uint16_t length) {
  if (length >= kCasper343ChecksumByte1)
    _Casper343Protocol.Sum1 = xorBytes(_Casper343Protocol.raw + 2, kCasper343ChecksumByte1 - 1 - 2, 0);
  if (length >= kCasper343ChecksumByte2)
    _Casper343Protocol.Sum2 = xorBytes(_Casper343Protocol.raw + kCasper343ChecksumByte1 + 1,
                 kCasper343ChecksumByte2 - kCasper343ChecksumByte1 - 1, 0);
}


/// Get a copy of the internal state/code for this protocol.
/// @param[in] calcchecksum Do we need to calculate the checksum?.
/// @return A code for this protocol based on the current internal state.
uint8_t *Casper343_getRaw(const bool calcchecksum) {
  if (calcchecksum) Casper343_checksum(kCasper343StateLength);
  return _Casper343Protocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] length The length/size of the new_code array.
void Casper343_setRaw(const uint8_t new_code[], const uint16_t length) {
  memcpy(_Casper343Protocol.raw, new_code, MIN(length, kCasper343StateLength));
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
/// @param[in] remember Do we save this temperature?
/// @note Internal use only.
void Casper343_setTemp(const uint8_t temp, const bool remember) {
  if (remember) _desiredtemp = temp;
  _Casper343Protocol.Temp = temp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Casper343_getTemp(void) {
  return _Casper343Protocol.Temp;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note Internal use only.
void Casper343_setMode(const uint8_t mode) {
  switch (mode) {
    
    // case kCasper343Auto: No Auto Mode
    case kCasper343Heat:
    case kCasper343Cool:
    case kCasper343Dry:
    case kCasper343Fan:
      _Casper343Protocol.Mode = mode;
      break;
    default:
      return;
  }
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Casper343_getMode(void) {
  return _Casper343Protocol.Mode;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Casper343_setFan(const uint8_t speed) {
  switch (speed) {
    case kFanSpeedMin:
      _Casper343Protocol.Fan = kCasper343FanLow;
      _Casper343Protocol.FanAngel = false;
      break;
    case kFanSpeedLow: 
      _Casper343Protocol.Fan = kCasper343FanLow;
      _Casper343Protocol.FanAngel = true;
      break;
    case kFanSpeedMedium:
      _Casper343Protocol.Fan = kCasper343FanMedium;
      _Casper343Protocol.FanAngel = false;
      break;
    case kFanSpeedHigh:
      _Casper343Protocol.Fan = kCasper343FanHigh;
      _Casper343Protocol.FanAngel = false;
      break;
    case kFanSpeedMax:
      _Casper343Protocol.Fan = kCasper343FanHigh;
      _Casper343Protocol.FanAngel = true;
      break;
    default:
      _Casper343Protocol.Fan = kCasper343FanAuto;
      _Casper343Protocol.FanAngel = false;
      break;
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Casper343_getFan(void) {
  switch (_Casper343Protocol.Fan) {
    case kCasper343FanMedium: 
      return kFanSpeedMedium;
    case kCasper343FanHigh:
      if(_Casper343Protocol.FanAngel == true)
        return kFanSpeedMax;
      else
        return kFanSpeedHigh;
    case kCasper343FanLow: 
      if(_Casper343Protocol.FanAngel == true)
        return kFanSpeedLow;
      else
        return kFanSpeedMin;
    default:
      return kFanSpeedAuto;
  }
}

/// Set the (vertical) swing setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper343_setSwingV(const bool on) {
  _Casper343Protocol.Swing1 = on;
  _Casper343Protocol.Swing2 = on;
}

/// Get the (vertical) swing setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper343_getSwingV(void) {
  return _Casper343Protocol.Swing1 && _Casper343Protocol.Swing2;
}

/// Set the (vertical) swing setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper343_setSwingH(const bool on) {
  _Casper343Protocol.SwingH = on;
}

/// Get the (vertical) swing setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper343_getSwingH(void) {
  return _Casper343Protocol.SwingH;
}

/// Set the Light (Display/LED) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper343_setLight(const bool on) {
  // Cleared when on.
  _Casper343Protocol.LightOff = on;
}

/// Change the power toggle setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper343_setPower(const bool on) {
  if(on){
    _Casper343Protocol.Power  = on;
    _Casper343Protocol.Power1 = on;
  }
  else{
    _Casper343Protocol.Power  = on;
    _Casper343Protocol.Power1 = false;
  }
}

/// Get the value of the current power toggle setting.
/// @return true, the setting is on. false, the setting is off.
bool Casper343_getPower(void) {
  return _Casper343Protocol.Power1;
}


/// Get the Command (Button) setting of the A/C.
/// @return The current Command (Button) of the A/C.
void Casper343_setCommand(void) {
  if     (_saveAcStatus.power_status != ac_status.power_status) _Casper343Protocol.Cmd = kCasper343CommandPower;
  else if(_saveAcStatus.fan          != ac_status.fan)          _Casper343Protocol.Cmd = kCasper343CommandFanSpeed;
  else if(_saveAcStatus.mode         != ac_status.mode)         _Casper343Protocol.Cmd = kCasper343CommandMode;
  else if(_saveAcStatus.temperature  != ac_status.temperature)  _Casper343Protocol.Cmd = kCasper343CommandTemp;
  else if(_saveAcStatus.swing        != ac_status.temperature)  _Casper343Protocol.Cmd = kCasper343CommandSwingV;
}

/// Get the Command (Button) setting of the A/C.
/// @return The current Command (Button) of the A/C.
void Casper343_ClearData(void) {
  if     (_saveAcStatus.power_status != ac_status.power_status) _Casper343Protocol.Power = false;
  else if(_saveAcStatus.swing        != ac_status.swing){
      Casper343_setSwingV(false);
  }
}

uint8_t Casper343_getCommand(void) {
  return _Casper343Protocol.Cmd;
}


/// Set the Sleep setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper343_setSleep(const uint8_t _sleep) {
  _Casper343Protocol.Sleep = true;
  switch(_sleep){
    case 0:  _Casper343Protocol.SleepLvl = kCasper343SleepLv0; break;
    case 1:  _Casper343Protocol.SleepLvl = kCasper343SleepLv1; break;
    case 2:  _Casper343Protocol.SleepLvl = kCasper343SleepLv2; break;
    case 3:  _Casper343Protocol.SleepLvl = kCasper343SleepLv3; break;
    default: 
      _Casper343Protocol.Sleep = false;
      _Casper343Protocol.SleepLvl = kCasper343SleepLv0; 
      break;
  }
  Casper343_setFan(kFanSpeedMin);
}

/// Get the Sleep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper343_getSleep(void) {
  return _Casper343Protocol.Sleep;
}

/// Set the Super (Turbo/Jet) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Casper343_setSuper(const bool on) {
  if (on) {
    Casper343_setFan(kFanSpeedHigh);
    switch (_Casper343Protocol.Mode) {
      case kCasper343Heat:
        Casper343_setTemp(kCasper343MaxTemp, true);
        break;
      case kCasper343Cool:
      default:
        Casper343_setTemp(kCasper343MinTemp, true);
        Casper343_setMode(kCasper343Cool);
        break;
    }
    _Casper343Protocol.Super1 = true;
    _Casper343Protocol.Super2 = true;
  } else {
    _Casper343Protocol.Super1 = false;
    _Casper343Protocol.Super2 = false;
  }
}

/// Get the Super (Turbo/Jet) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Casper343_getSuper(void) {
  return _Casper343Protocol.Super1 && _Casper343Protocol.Super2;
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Casper343_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kCasper343Cool: return kOpModeCool;
    case kCasper343Heat: return kOpModeHeat;
    case kCasper343Dry:  return kOpModeDry;
    case kCasper343Fan:  return kOpModeFan;
    default:             return kOpModeAuto;
  }
}



/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Casper343_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeHeat: return kCasper343Heat;
    case kOpModeDry:  return kCasper343Dry;
    case kOpModeFan:  return kCasper343Fan;
    default:          return kCasper343Cool;
  }
}



