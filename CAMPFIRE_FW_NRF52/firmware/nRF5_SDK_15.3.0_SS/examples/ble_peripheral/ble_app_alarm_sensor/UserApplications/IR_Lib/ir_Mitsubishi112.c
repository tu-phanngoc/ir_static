
/// @file
/// @brief Support for Mitsubishi protocols.
/// Mitsubishi (TV) sending & Mitsubishi A/C support added by David Conran
/// @see GlobalCache's Control Tower's Mitsubishi TV data.

#include "stdint.h"
#include "string.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "ir_Mitsubishi112.h"
#include "app_ac_status.h"
#include "IR_Common.h"

Mitsubishi112Protocol_t _Mitsubishi112 = { .raw = {0x23, 0xCB, 0x26, 0x01, 0x00, 0x24, 0x03, 0x0B, 0x10, 0x00, 0x00, 0x00, 0x30} };


// Mitsubishi 112 bit A/C
const uint16_t kMitsubishi112HdrMark            = 3450;
const uint16_t kMitsubishi112HdrSpace           = 1696;
const uint16_t kMitsubishi112BitMark            = 450;
const uint16_t kMitsubishi112OneSpace           = 1250;
const uint16_t kMitsubishi112ZeroSpace          = 385;
const uint32_t kMitsubishi112Gap                = kMitsubishi112ZeroSpace;
const uint8_t  kMitsubishi112HdrMarkTolerance   = 5; // Total tolerance percentage to use for matching the header mark.


void encode_Mitsubishi112(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  Mitsubishi112_setPower(ac_status.power_status);
  Mitsubishi112_setTemp(ac_status.temperature);
  Mitsubishi112_setMode(Mitsubishi112_convertMode(ac_status.mode));
  Mitsubishi112_setFan(Mitsubishi112_convertFan(ac_status.fan));

  if(ac_status.swing)
    Mitsubishi112_setSwingV(kMitsubishi112SwingVAuto);
  else
    Mitsubishi112_setSwingV(kMitsubishi112SwingVLowest);

  Mitsubishi112_send(OutputIRProtocol);
  setIrTxState(1);
}


void decode_Mitsubishi112(int16_t* input, uint8_t* output) {
  
  bool isDecodeOk = false;
  // copy raw buf, init data
  initDecodeData(input, MITSUBISHI_112_MAX_INDEX + 1);
  
  if( Mitsubishi112_recv(&gDecodeResult, 0, kMitsubishi112Bits, false) ){
    Mitsubishi112_setRaw(gDecodeResult.state);
  }

  output[0] = Mitsubishi112_getPower();
  output[1] = Mitsubishi112_getTemp();
  output[2] = Mitsubishi112_toCommonFanSpeed(Mitsubishi112_getFan());
  output[3] = Mitsubishi112_getSwingV();
  output[4] = Mitsubishi112_toCommonMode(Mitsubishi112_getMode());

  ac_control_set_power_status(Mitsubishi112_getPower());
  ac_control_set_temperature(output[1]);
  ac_control_set_fan(output[2]);
  ac_control_set_swing(output[3] == kMitsubishi112SwingVAuto);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}



/// Send a Mitsubishi 112-bit A/C formatted message. (MITSUBISHI112)
/// Status: Stable / Reported as working.
void Mitsubishi112_send(int16_t *irRaw) {
  sendGeneric_8( kMitsubishi112HdrMark, kMitsubishi112HdrSpace,
                  kMitsubishi112BitMark, kMitsubishi112OneSpace,
                  kMitsubishi112BitMark, kMitsubishi112ZeroSpace,
                  kMitsubishi112BitMark, kMitsubishi112Gap,
                  Mitsubishi112_getRaw(),   kMitsubishi112StateLength,
                  38, false, 0, 50,
                  irRaw);
}

/// Decode the supplied Mitsubishi/TCL 112-bit A/C message.
///   (MITSUBISHI112, TCL112AC)
/// Status: STABLE / Reported as working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @note Note Mitsubishi112 & Tcl112Ac are basically the same protocol.
///   The only significant difference I can see is Mitsubishi112 has a
///   slightly longer header mark. We will use that to determine which
///   variant it should be. The other differences require full decoding and
///   only only with certain settings.
///   There are some other timing differences too, but the tolerances will
///   overlap.
bool Mitsubishi112_recv(decode_results *results, uint16_t offset,
                          const uint16_t nbits, const bool strict) {
  if (results->rawlen < (2 * nbits) + kHeader + kFooter - 1 + offset)
    return false;
  if (nbits % 8 != 0) return false;  // Not a multiple of an 8 bit byte.
  if (strict) {  // Do checks to see if it matches the spec.
    if (nbits != kMitsubishi112Bits && nbits != kTcl112AcBits) return false; /* Xem lai logic, Tu Phan */
  }
  decode_type_t typeguess = UNKNOWN;
  uint16_t hdrspace;
  uint16_t bitmark;
  uint16_t onespace;
  uint16_t zerospace;
  uint32_t gap;
  uint8_t  tolerance = kTolerance;

  // Header
	/* TODO: Neu ko match thi sao? Tu Phan */
  if (matchMark(results->rawbuf[offset], kMitsubishi112HdrMark, kTolerance)) {
    typeguess = MITSUBISHI112;
    hdrspace = kMitsubishi112HdrSpace;
    bitmark = kMitsubishi112BitMark;
    onespace = kMitsubishi112OneSpace;
    zerospace = kMitsubishi112ZeroSpace;
    gap = kMitsubishi112Gap;
  }
  if (typeguess == UNKNOWN) return false;  // No header matched.
  offset++;

  uint16_t used = matchGeneric_8( results->rawbuf + offset, results->state,
                                  results->rawlen - offset, nbits,
                                  0,  // Skip the header as we matched it earlier.
                                  hdrspace, bitmark, onespace, bitmark, zerospace,
                                  bitmark, gap,
                                  true, tolerance, 0, false);
  if (!used) return false;
  if (strict) {
    // Header validation: Codes start with 0x23CB26
    if (results->state[0] != 0x23 || results->state[1] != 0xCB ||
        results->state[2] != 0x26) return false;

    if ( !Mitsubishi112_validChecksum(results->state, nbits / 8) )
      return false;
  }
  // Success
  results->decode_type = typeguess;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


/// Calculate the checksum for the current internal state of the remote.
void Mitsubishi112_checksum(void) {
  _Mitsubishi112.frame.Sum = Mitsubishi112_calcChecksum(_Mitsubishi112.raw, kMitsubishi112StateLength);
}

/// Calculate the checksum for a given state.
/// @param[in] state The array to calc the checksum of.
/// @param[in] length The length/size of the array.
/// @return The calculated checksum value.
uint8_t Mitsubishi112_calcChecksum(uint8_t state[], const uint16_t length) {
  if (length) {
    if (length > 4 && state[3] == 0x02) {  // Special nessage?
      return sumBytes(state, length - 1, 0xF);  // Checksum needs an offset.
    } else {
      return sumBytes(state, length - 1, 0);
    }
  } else {
    return 0;
  }
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Mitsubishi112_validChecksum(uint8_t state[], const uint16_t length) {
  return (length > 1 && state[length - 1] == Mitsubishi112_calcChecksum(state, length));
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *Mitsubishi112_getRaw(void) {
  Mitsubishi112_checksum();
  return _Mitsubishi112.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] data A valid code for this protocol.
void Mitsubishi112_setRaw(const uint8_t *data) {
  memcpy(_Mitsubishi112.raw, data, kMitsubishi112StateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Mitsubishi112_setPower(bool on) {
  _Mitsubishi112.frame.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Mitsubishi112_getPower(void) {
  return _Mitsubishi112.frame.Power;
}

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
void Mitsubishi112_setTemp(const uint8_t degrees) {
  uint8_t temp = MAX((uint8_t)kMitsubishi112MinTemp, degrees + kMitsubishi112MinTemp);
  temp = MIN((uint8_t)kMitsubishi112MaxTemp, temp);
  _Mitsubishi112.frame.Temp = kMitsubishi112MaxTemp - temp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Mitsubishi112_getTemp(void) {
  return kMitsubishi112MaxTemp - _Mitsubishi112.frame.Temp - kMitsubishi112MinTemp;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Mitsubishi112_setFan(const uint8_t speed) {
  switch (speed) {
    case kMitsubishi112FanMin:
    case kMitsubishi112FanMed:
    case kMitsubishi112FanMax:
      _Mitsubishi112.frame.Fan = speed;
      break;
    default:
      _Mitsubishi112.frame.Fan = kMitsubishi112FanMax;
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Mitsubishi112_getFan(void) {
  return _Mitsubishi112.frame.Fan;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Mitsubishi112_getMode(void) {
  return _Mitsubishi112.frame.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void Mitsubishi112_setMode(const uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  switch (mode) {
    // Note: No Fan Only mode.
    case kMitsubishi112Cool:
    case kMitsubishi112Heat:
    case kMitsubishi112Auto:
    case kMitsubishi112Fan:
    case kMitsubishi112Dry:
      _Mitsubishi112.frame.Mode = mode;
      break;
    default:
      _Mitsubishi112.frame.Mode = kMitsubishi112Auto;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void Mitsubishi112_setSwingV(const uint8_t position) {
  // If we get an unexpected mode, default to auto.
  switch (position) {
    case kMitsubishi112SwingVLowest:
    case kMitsubishi112SwingVLow:
    case kMitsubishi112SwingVMiddle:
    case kMitsubishi112SwingVHigh:
    case kMitsubishi112SwingVHighest:
    case kMitsubishi112SwingVAuto:
      _Mitsubishi112.frame.SwingV = position;
      break;
    default:
      _Mitsubishi112.frame.SwingV = kMitsubishi112SwingVAuto;
  }
}

/// Get the Vertical Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi112_getSwingV(void) {
  return _Mitsubishi112.frame.SwingV;
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void Mitsubishi112_setSwingH(const uint8_t position) {
  // If we get an unexpected mode, default to auto.
  switch (position) {
    case kMitsubishi112SwingHLeftMax:
    case kMitsubishi112SwingHLeft:
    case kMitsubishi112SwingHMiddle:
    case kMitsubishi112SwingHRight:
    case kMitsubishi112SwingHRightMax:
    case kMitsubishi112SwingHWide:
    case kMitsubishi112SwingHAuto:
      _Mitsubishi112.frame.SwingH = position;
      break;
    default:
      _Mitsubishi112.frame.SwingH = kMitsubishi112SwingHAuto;
  }
}


/// Get the Horizontal Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t Mitsubishi112_getSwingH(void) {
  return _Mitsubishi112.frame.SwingH;
}

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @note There is no true quiet setting on this A/C.
void Mitsubishi112_setQuiet(bool on) {
  if (on)
    Mitsubishi112_setFan(kMitsubishi112FanQuiet);
  else if (Mitsubishi112_getQuiet())
    Mitsubishi112_setFan(kMitsubishi112FanMed);
}


/// Get the Quiet mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
/// @note There is no true quiet setting on this A/C.
bool Mitsubishi112_getQuiet(void) {
  return _Mitsubishi112.frame.Fan == kMitsubishi112FanQuiet;
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi112_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kMitsubishi112Cool;
    case kOpModeHeat: return kMitsubishi112Heat;
    case kOpModeDry:  return kMitsubishi112Dry;
    // Note: No Fan Only mode.
    default:          return kMitsubishi112Auto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi112_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kMitsubishi112FanMin;
    case kFanSpeedLow:    return kMitsubishi112FanMed;
    case kFanSpeedMedium:
    case kFanSpeedHigh:
    case kFanSpeedMax:    return kMitsubishi112FanMax;
    default:              return kMitsubishi112FanAuto;
  }
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi112_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest: return kMitsubishi112SwingVHighest;
    case kSwingVHigh:    return kMitsubishi112SwingVHigh;
    case kSwingVMiddle:  return kMitsubishi112SwingVMiddle;
    case kSwingVLow:     return kMitsubishi112SwingVLow;
    case kSwingVLowest:  return kMitsubishi112SwingVLowest;
    default:             return kMitsubishi112SwingVAuto;
  }
}

/// Convert a swingh_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Mitsubishi112_convertSwingH(const swingh_t position) {
  switch (position) {
    case kSwingHLeftMax:  return kMitsubishi112SwingHLeftMax;
    case kSwingHLeft:     return kMitsubishi112SwingHLeft;
    case kSwingHMiddle:   return kMitsubishi112SwingHMiddle;
    case kSwingHRight:    return kMitsubishi112SwingHRight;
    case kSwingHRightMax: return kMitsubishi112SwingHRightMax;
    case kSwingHWide:     return kMitsubishi112SwingHWide;
    case kSwingHAuto:     return kMitsubishi112SwingHAuto;
    default:              return kMitsubishi112SwingHAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Mitsubishi112_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kMitsubishi112Cool: return kOpModeCool;
    case kMitsubishi112Heat: return kOpModeHeat;
    case kMitsubishi112Dry:  return kOpModeDry;
    case kMitsubishi112Fan:  return kOpModeFan;
    default:                 return kOpModeAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Mitsubishi112_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kMitsubishi112FanMax: return kFanSpeedMedium;
    case kMitsubishi112FanMed: return kFanSpeedLow;
    case kMitsubishi112FanMin: return kFanSpeedMin;
    default:                   return kFanSpeedAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
swingv_t Mitsubishi112_toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kMitsubishi112SwingVHighest: return kSwingVHighest;
    case kMitsubishi112SwingVHigh:    return kSwingVHigh;
    case kMitsubishi112SwingVMiddle:  return kSwingVMiddle;
    case kMitsubishi112SwingVLow:     return kSwingVLow;
    case kMitsubishi112SwingVLowest:  return kSwingVLowest;
    default:                          return kSwingVAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common horizontal swing position.
swingh_t Mitsubishi112_toCommonSwingH(const uint8_t pos) {
  switch (pos) {
    case kMitsubishi112SwingHLeftMax:  return kSwingHLeftMax;
    case kMitsubishi112SwingHLeft:     return kSwingHLeft;
    case kMitsubishi112SwingHMiddle:   return kSwingHMiddle;
    case kMitsubishi112SwingHRight:    return kSwingHRight;
    case kMitsubishi112SwingHRightMax: return kSwingHRightMax;
    case kMitsubishi112SwingHWide:     return kSwingHWide;
    default:                           return kSwingHAuto;
  }
}


