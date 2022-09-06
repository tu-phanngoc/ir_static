
#include "string.h"
#include "stdint.h"
#include "stdio.h"
#include "ir_Coolix.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"
#include "IRcommon.h"

#include "IR_Common.h"
#include "app_ac_status.h"


// Constants
const uint8_t kCoolixTempMap[kCoolixTempRange] = {
    0x0,  // 17C
    0x1,  // 18c
    0x3,  // 19C
    0x2,  // 20C
    0x6,  // 21C
    0x7,  // 22C
    0x5,  // 23C
    0x4,  // 24C
    0xC,  // 25C
    0xD,  // 26C
    0x9,  // 27C
    0x8,  // 28C
    0xA,  // 29C
    0xB   // 30C
};


// Internal State settings
union CoolixProtocol_t _CoolixProtocol = {.raw = kCoolixDefaultState};  ///< The state of the IR remote in IR code form.
union CoolixProtocol_t _saved = {.raw = kCoolixDefaultState};   ///< Copy of the state if we required a special mode.
bool powerFlag;
bool turboFlag;
bool ledFlag;
bool cleanFlag;
bool sleepFlag;
bool swingFlag;
uint8_t savedFan;

static bool isCoolixDecoded = false;


/* Get values from BLE command then fill to IR protocol */
void encode_Coolix(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  if(ac_status.power_status){
    Coolix_setPower(ac_status.power_status);
    Coolix_setMode(Coolix_convertMode(ac_status.mode));
    Coolix_setFan(Coolix_convertFan(ac_status.fan), true);
    if(ac_status.mode != kOpModeFan)
      Coolix_setTemp(ac_status.temperature);
  }
  else{
    Coolix_setPower(ac_status.power_status);
  }

  Coolix_send(Coolix_getRaw(), kCoolixBits, kCoolixDefaultRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Coolix(int16_t* input, uint8_t* output) {

  if(isCoolixDecoded){
    isCoolixDecoded = false;
  }
  else{
    initDecodeData(input, COOLIX_BITS);
    if( Coolix_recv(&gDecodeResult, 0, kCoolixBits, true) ){
      Coolix_setRaw(gDecodeResult.value);
    }
  }

  output[0] = Coolix_getPower();
  output[1] = Coolix_getTemp();
  output[2] = Coolix_toCommonFanSpeed(Coolix_getFan());
  output[4] = Coolix_toCommonMode( Coolix_getMode() );
  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}




/// Send a Coolix 24-bit message
/// Status: STABLE / Confirmed Working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void Coolix_send(uint64_t data, uint16_t nbits, uint16_t repeat, int16_t *irRaw) {

  uint8_t offset = 0;
  if (nbits % 8 != 0) return;  // nbits is required to be a multiple of 8.


  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    irRaw[offset++] = kCoolixHdrMark;
    irRaw[offset++] = -kCoolixHdrSpace;

    // Data
    //   Break data into byte segments, starting at the Most Significant
    //   Byte. Each byte then being sent normal, then followed inverted.
    for (uint16_t i = 8; i <= nbits; i += 8) {
      // Grab a bytes worth of data.
      uint8_t segment = (data >> (nbits - i)) & 0xFF;
      // Normal
      dataToRaw(kCoolixBitMark, kCoolixOneSpace, kCoolixBitMark, kCoolixZeroSpace,
                segment, 8, true, &irRaw[offset]);
      offset += 8*2;
      // Inverted.
      dataToRaw(kCoolixBitMark, kCoolixOneSpace, kCoolixBitMark, kCoolixZeroSpace,
                (segment ^ 0xFF), 8, true, &irRaw[offset]);
      offset += 8*2;
    }

    // Footer
    irRaw[offset++] = kCoolixBitMark;
    irRaw[offset++] = -kCoolixMinGap;
  }
}




/// Decode the supplied  24-bit A/C message.
/// Status: STABLE / Known Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Coolix_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  // The protocol sends the data normal + inverted, alternating on
  // each byte. Hence twice the number of expected data bits.
  if (results->rawlen < 2 * 2 * nbits + kHeader + kFooter - 1 + offset)
    return false;  // Can't possibly be a valid COOLIX message.
  if (strict && nbits != kCoolixBits)
    return false;      // Not strictly a COOLIX message.
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;

  uint64_t data = 0;
  uint64_t inverted = 0;

  if (nbits > sizeof(data) * 8)
    return false;  // We can't possibly capture a Coolix packet that big.

  // Header
  if (!matchMark(results->rawbuf[offset++], kCoolixHdrMark, kTolerance)) return false;
  if (!matchSpace(results->rawbuf[offset++], kCoolixHdrSpace, kTolerance)) return false;

  // Data
  // Twice as many bits as there are normal plus inverted bits.
  for (uint16_t i = 0; i < nbits * 2; i++, offset++) {
    bool flip = (i / 8) % 2;
    if (!matchMark(results->rawbuf[offset++], kCoolixBitMark, kTolerance))
      return false;
    if (matchSpace(results->rawbuf[offset], kCoolixOneSpace, kTolerance)) {
      if (flip) inverted = (inverted << 1) | 1;
      else data = (data << 1) | 1;
    }
    else if (matchSpace(results->rawbuf[offset], kCoolixZeroSpace, kTolerance)) {
      if (flip) inverted <<= 1;
      else data <<= 1;
    } 
    else {
      return false;
    }
  }

  // Footer
  if (!matchMark(results->rawbuf[offset++], kCoolixBitMark, kTolerance))
    return false;

  // Compliance
  uint64_t orig = data;  // Save a copy of the data.
  if (strict) {
    for (uint16_t i = 0; i < nbits; i += 8, data >>= 8, inverted >>= 8)
      if ((data & 0xFF) != ((inverted & 0xFF) ^ 0xFF)) return false;
  }

  // Success
  results->decode_type = COOLIX;
  results->bits = nbits;
  results->value = orig;
  results->address = 0;
  results->command = 0;
  return true;
}


bool isHeader_Coolix(int16_t *irRaw){
  uint8_t Address = 0;

  initDecodeData(irRaw, COOLIX_BITS);
  if( Coolix_recv(&gDecodeResult, 0, kCoolixBits, true) ){
    Coolix_setRaw(gDecodeResult.value);

    if(gDecodeResult.state[2] == 0xB2){
      isCoolixDecoded = true;
      return true;
    }
  }

  isCoolixDecoded = false;
  return false;
}

/// Reset the internal state to a fixed known good state.
void Coolix_stateReset(void) {
  Coolix_setRaw(kCoolixDefaultState);
  savedFan = Coolix_getFan();
  Coolix_clearSensorTemp();
  powerFlag = false;
  turboFlag = false;
  ledFlag = false;
  cleanFlag = false;
  sleepFlag = false;
  swingFlag = false;
}

/// Get a copy of the internal state as a valid code for this protocol.
/// @return A valid code for this protocol based on the current internal state.
uint32_t Coolix_getRaw(void) { return _CoolixProtocol.raw; }

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void Coolix_setRaw(const uint32_t new_code) {
  powerFlag = true;  // Everything that is not the special power off mesg is On.
  if (!Coolix_handleSpecialState(new_code)) {
    // it isn`t special so might affect Temp|mode|Fan
    if (new_code == kCoolixCmdFan) {
      Coolix_setMode(kCoolixFan);
      return;
    }
  }
  // must be a command changing Temp|Mode|Fan
  // it is safe to just copy to remote var
  _CoolixProtocol.raw = new_code;
}

/// Is the current state is a special state?
/// @return true, if it is. false if it isn't.
bool Coolix_isSpecialState(void) {
  switch (_CoolixProtocol.raw) {
    case kCoolixClean:
    case kCoolixLed:
    case kCoolixOff:
    case kCoolixSwing:
    case kCoolixSwingV:
    case kCoolixSleep:
    case kCoolixTurbo: return true;
    default: return false;
  }
}

/// Adjust any internal settings based on the type of special state we are
///   supplied. Does nothing if it isn't a special state.
/// @param[in] data The state we need to act upon.
/// @note Special state means commands that are not affecting
/// Temperature/Mode/Fan, and they toggle a setting.
/// e.g. Swing Step is not a special state by this definition.
/// @return true, if it is a special state. false if it isn't.
bool Coolix_handleSpecialState(const uint32_t data) {
  switch (data) {
    case kCoolixClean:
      cleanFlag = !cleanFlag;
      break;
    case kCoolixLed:
      ledFlag = !ledFlag;
      break;
    case kCoolixOff:
      powerFlag = false;
      break;
    case kCoolixSwing:
      swingFlag = !swingFlag;
      break;
    case kCoolixSleep:
      sleepFlag = !sleepFlag;
      break;
    case kCoolixTurbo:
      turboFlag = !turboFlag;
      break;
    default:
      return false;
  }
  return true;
}

/// Backup the current internal state as long as it isn't a special state and
/// set the new state.
/// @note: Must be called before every special state to make sure the
/// internal state is safe.
/// @param[in] raw_state A valid raw state/code for this protocol.
void Coolix_updateAndSaveState(const uint32_t raw_state) {
  if (!Coolix_isSpecialState()) _saved = _CoolixProtocol;
  _CoolixProtocol.raw = raw_state;
}

/// Restore the current internal state from backup as long as it isn't a
/// special state.
void Coolix_recoverSavedState(void) {
  // If the current state is a special one, last known normal one.
  if (Coolix_isSpecialState()) _CoolixProtocol = _saved;
  // If the saved state was also a special state, reset as we expect a normal
  // state out of all this.
  if (Coolix_isSpecialState()) Coolix_stateReset();
}


/// Set the raw (native) temperature value.
/// @note Bypasses any checks.
/// @param[in] code The desired native temperature.
void Coolix_setTempRaw(const uint8_t code) { _CoolixProtocol.Temp = code; }

/// Get the raw (native) temperature value.
/// @return The native temperature value.
uint8_t Coolix_getTempRaw(void) { return _CoolixProtocol.Temp; }


/// Set the temperature.
/// @param[in] desired The temperature in degrees celsius.
void Coolix_setTemp(const uint8_t desired) {
  // Range check.
  uint8_t temp;
  if(desired == 0)
    temp = 17;
  else
    temp = desired + 16;
  temp = MIN(temp, kCoolixTempMax);
  temp = MAX(temp, kCoolixTempMin);
  Coolix_setTempRaw(kCoolixTempMap[temp - kCoolixTempMin]);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Coolix_getTemp(void) {
  const uint8_t code = Coolix_getTempRaw();
  for (uint8_t i = 0; i < kCoolixTempRange; i++)
    if (kCoolixTempMap[i] == code) return (i + 1);
  return kCoolixTempMax;  // Not a temp we expected.
}

/// Set the raw (native) sensor temperature value.
/// @note Bypasses any checks or additional actions.
/// @param[in] code The desired native sensor temperature.
void Coolix_setSensorTempRaw(const uint8_t code) { _CoolixProtocol.SensorTemp = code; }

/// Set the sensor temperature.
/// @param[in] temp The temperature in degrees celsius.
/// @warning Do not send messages with a Sensor Temp more frequently than once
///   per minute, otherwise the A/C unit will ignore them.
void Coolix_setSensorTemp(const uint8_t temp) {
  Coolix_setSensorTempRaw(MIN(temp, kCoolixSensorTempMax));
  Coolix_setZoneFollow(true);  // Setting a Sensor temp means you want to Zone Follow.
}

/// Get the sensor temperature setting.
/// @return The current setting for sensor temp. in degrees celsius.
uint8_t Coolix_getSensorTemp(void) { return _CoolixProtocol.SensorTemp; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
/// @note There is only an "off" state. Everything else is "on".
bool Coolix_getPower(void) { return powerFlag; }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Coolix_setPower(const bool on) {
  if (!on)
    Coolix_updateAndSaveState(kCoolixOff);
  else if (!powerFlag)
    // at this point state must be ready
    // to be transmitted
    Coolix_recoverSavedState();
  powerFlag = on;
}

/// Get the Swing setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Coolix_getSwing(void) { return swingFlag; }

/// Toggle the Swing mode of the A/C.
void Coolix_setSwing(void) {
  // Assumes that repeated sending "swing" toggles the action on the device.
  Coolix_updateAndSaveState(kCoolixSwing);
  swingFlag = !swingFlag;
}

/// Get the Vertical Swing Step setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Coolix_getSwingVStep(void) { return _CoolixProtocol.raw == kCoolixSwingV; }

/// Set the Vertical Swing Step setting of the A/C.
void Coolix_setSwingVStep(void) {
  Coolix_updateAndSaveState(kCoolixSwingV);
}

/// Get the Sleep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Coolix_getSleep(void) { return sleepFlag; }

/// Toggle the Sleep mode of the A/C.
void Coolix_setSleep(void) {
  Coolix_updateAndSaveState(kCoolixSleep);
  sleepFlag = !sleepFlag;
}

/// Get the Turbo setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Coolix_getTurbo(void) { return turboFlag; }

/// Toggle the Turbo mode of the A/C.
void Coolix_setTurbo(void) {
  // Assumes that repeated sending "turbo" toggles the action on the device.
  Coolix_updateAndSaveState(kCoolixTurbo);
  turboFlag = !turboFlag;
}

/// Get the Led (light) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Coolix_getLed(void) { return ledFlag; }

/// Toggle the Led (light) mode of the A/C.
void Coolix_setLed(void) {
  // Assumes that repeated sending "Led" toggles the action on the device.
  Coolix_updateAndSaveState(kCoolixLed);
  ledFlag = !ledFlag;
}

/// Get the Clean setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Coolix_getClean(void) { return cleanFlag; }

/// Toggle the Clean mode of the A/C.
void Coolix_setClean(void) {
  Coolix_updateAndSaveState(kCoolixClean);
  cleanFlag = !cleanFlag;
}

/// Get the Zone Follow setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Coolix_getZoneFollow(void) {
  return _CoolixProtocol.ZoneFollow1 && _CoolixProtocol.ZoneFollow2;
}

/// Change the Zone Follow setting.
/// @note Internal use only.
/// @param[in] on true, the setting is on. false, the setting is off.
void Coolix_setZoneFollow(const bool on) {
  _CoolixProtocol.ZoneFollow1 = on;
  _CoolixProtocol.ZoneFollow2 = on;
  Coolix_setFan(on ? kCoolixFanZoneFollow : savedFan, true);
}

/// Clear the Sensor Temperature setting..
void Coolix_clearSensorTemp(void) {
  Coolix_setZoneFollow(false);
  Coolix_setSensorTempRaw(kCoolixSensorTempIgnoreCode);
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Coolix_setMode(const uint8_t mode) {
  uint32_t actualmode = mode;
  switch (actualmode) {
    case kCoolixAuto:
    case kCoolixDry:
      Coolix_setFan(kCoolixFanAuto0, false);
      break;
    case kCoolixCool:
    case kCoolixHeat:
    case kCoolixFan:
      Coolix_setFan(kCoolixFanAuto, false);
      break;
    default:  // Anything else, go with Auto mode.
      Coolix_setMode(kCoolixAuto);
      Coolix_setFan(kCoolixFanAuto0, false);
      return;
  }
  Coolix_setTemp(Coolix_getTemp());
  // Fan mode is a special case of Dry.
  if (mode == kCoolixFan) {
    actualmode = kCoolixDry;
    Coolix_setTempRaw(kCoolixFanTempCode);
  }
  _CoolixProtocol.Mode = actualmode;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Coolix_getMode(void) {
  const uint8_t mode = _CoolixProtocol.Mode;
  if (mode == kCoolixDry)
    if (Coolix_getTempRaw() == kCoolixFanTempCode) return kCoolixFan;
  return mode;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t Coolix_getFan(void) { return _CoolixProtocol.Fan; }

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
/// @param[in] modecheck Do we enforce any mode limitations before setting?
void Coolix_setFan(const uint8_t speed, const bool modecheck) {
  uint8_t newspeed = speed;
  switch (speed) {
    case kCoolixFanAuto:  // Dry & Auto mode can't have this speed.
      if (modecheck) {
        switch (Coolix_getMode()) {
          case kCoolixAuto:
          case kCoolixDry:
            newspeed = kCoolixFanAuto0;
          break;
        }
      }
      break;
    case kCoolixFanAuto0:  // Only Dry & Auto mode can have this speed.
      if (modecheck) {
        switch (Coolix_getMode()) {
          case kCoolixAuto:
          case kCoolixDry: break;
          default: newspeed = kCoolixFanAuto;
        }
      }
      break;
    case kCoolixFanMin:
    case kCoolixFanMed:
    case kCoolixFanMax:
    case kCoolixFanZoneFollow:
    case kCoolixFanFixed:
      break;
    default:  // Unknown speed requested.
      newspeed = kCoolixFanAuto;
      break;
  }
  // Keep a copy of the last non-ZoneFollow fan setting.
  savedFan = (_CoolixProtocol.Fan == kCoolixFanZoneFollow) ? savedFan : _CoolixProtocol.Fan;
  _CoolixProtocol.Fan = newspeed;
}

/// Convert a standard A/C mode into its native mode.
/// @param[in] mode A opmode_t to be converted to it's native equivalent.
/// @return The corresponding native mode.
uint8_t Coolix_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kCoolixCool;
    case kOpModeHeat: return kCoolixHeat;
    case kOpModeDry:  return kCoolixDry;
    case kOpModeFan:  return kCoolixFan;
    default:          return kCoolixAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Coolix_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kCoolixFanMin;
    case kFanSpeedLow:    return kCoolixFanMed;
    case kFanSpeedMedium: 
    case kFanSpeedHigh:
    case kFanSpeedMax:    return kCoolixFanMax;
    default:              return kCoolixFanAuto;
  }
}

/// Convert a native mode to it's common opmode_t equivalent.
/// @param[in] mode A native operation mode to be converted.
/// @return The corresponding common opmode_t mode.
opmode_t Coolix_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kCoolixCool: return kOpModeCool;
    case kCoolixHeat: return kOpModeHeat;
    case kCoolixDry:  return kOpModeDry;
    case kCoolixFan:  return kOpModeFan;
    default:          return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Coolix_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kCoolixFanMax: return kFanSpeedMax;
    case kCoolixFanMed: return kFanSpeedMedium;
    case kCoolixFanMin: return kFanSpeedMin;
    default:            return kFanSpeedAuto;
  }
}



