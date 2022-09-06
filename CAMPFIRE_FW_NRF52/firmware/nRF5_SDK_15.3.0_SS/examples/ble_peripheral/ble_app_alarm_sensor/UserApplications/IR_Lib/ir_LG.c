
/// @file
/// @brief Support for LG protocols.

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"
#include "IRcommon.h"

#include "ir_LG.h"

#include "app_ac_status.h"
#include "IR_Common.h"

// Constants
// Common timings
const uint16_t kLgBitMark           = 550;     ///< uSeconds.
const uint16_t kLgOneSpace          = 1600;    ///< uSeconds.
const uint16_t kLgZeroSpace         = 550;     ///< uSeconds.
const uint16_t kLgRptSpace          = 2250;    ///< uSeconds.
const uint16_t kLgMinGap            = 39750;   ///< uSeconds.
const uint32_t kLgMinMessageLength  = 108050;  ///< uSeconds.
// LG (28 Bit)
const uint16_t kLgHdrMark           = 8500; ///< uSeconds.
const uint16_t kLgHdrSpace          = 4250; ///< uSeconds.
// LG (32 Bit)
const uint16_t kLg32HdrMark         = 4500; ///< uSeconds.
const uint16_t kLg32HdrSpace        = 4450; ///< uSeconds.
const uint16_t kLg32RptHdrMark      = 8950; ///< uSeconds.
// LG2 (28 Bit)
const uint16_t kLg2HdrMark          = 3200; ///< uSeconds.
const uint16_t kLg2HdrSpace         = 9900; ///< uSeconds.
const uint16_t kLg2BitMark          = 480;  ///< uSeconds.

const uint32_t kLgAcAKB74955603DetectionMask  = 0x0000080;
const uint8_t  kLgAcChecksumSize              = 4;  ///< Size in bits.
// Signature has the checksum removed, and another bit to match both Auto & Off.
const uint8_t  kLgAcSwingHOffsetSize  = kLgAcChecksumSize + 1;
const uint32_t kLgAcSwingHSignature   = kLgAcSwingHOff >> kLgAcSwingHOffsetSize;
const uint32_t kLgAcVaneSwingVBase    = 0x8813200;


LGProtocol_t _LGProtocol = { .raw = kLgAcOffCommand };
static uint8_t _temp;
static bool _light = true;
static uint32_t _swingv = kLgAcSwingVOff;
static uint32_t _swingv_prev;
static uint8_t _vaneswingv[kLgAcSwingVMaxVanes] = {0};
static uint8_t _vaneswingv_prev[kLgAcSwingVMaxVanes];
static bool _swingh = false;
static bool _swingh_prev;
static decode_type_t _protocol = LG2;  ///< Protocol version
static lg_ac_remote_model_t _model = GE6711AR2853M;  ///< Model type

#ifdef VANESWINGVPOS
#undef VANESWINGVPOS
#endif
#define VANESWINGVPOS(code) (code % kLgAcVaneSwingVSize)





void encode_LG(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  if(ac_status.power_status){
    LG_setPower(ac_status.power_status);
    LG_setTemp(ac_status.temperature);
    LG_setMode(LG_convertMode(ac_status.mode));
    LG_setFan(LG_convertFan(ac_status.fan));
  }
  else{
    _LGProtocol.raw = kLgAcOffCommand;
  }

  if(_protocol == LG)
    LG_send(LG_getRaw(), kLgBits, kNoRepeat, OutputIRProtocol);
  else
    LG_send2(LG_getRaw(), kLgBits, kNoRepeat, OutputIRProtocol);
  setIrTxState(1);
}


void decode_LG(int16_t* input, uint8_t* output) {
  
  // copy raw buf, init data
  initDecodeData(input, LG_BITS);
  
  if( LG_recv(&gDecodeResult, 0, kLgBits, false) ){
    LG_setRaw( gDecodeResult.state[0]        |
              (gDecodeResult.state[1] << 8)  |
              (gDecodeResult.state[2] << 16) |
              (gDecodeResult.state[3] << 24) ,
               gDecodeResult.decode_type);
  }

  /* ON/OFF */
  output[0] = LG_getPower();
  ac_control_set_power_status(LG_getPower());

  /* CONTROL TEMPERATURE */
  output[1] = LG_getTemp();
  ac_control_set_temperature(LG_getTemp());

  /* WIND LEVEL */
  output[2] = LG_toCommonFanSpeed(LG_getFan());
  ac_control_set_fan( output[2]);

  /* AUTO/MANUAL SWING */
  output[3] = LG_getSwingV();
  ac_control_set_swing(output[3]);

  /* SET AIRCONDITIONER MODE */
  output[4] = LG_toCommonMode(LG_getMode());
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}






/// Send an LG formatted message. (LG)
/// Status: Beta / Should be working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
///   Typically kLgBits or kLg32Bits.
/// @param[in] repeat The number of times the command is to be repeated.
/// @note LG has a separate message to indicate a repeat, like NEC does.
void LG_send(uint64_t data, uint16_t nbits, uint16_t repeat, int16_t *irRaw) {
  uint16_t repeatHeaderMark = 0;
  uint8_t duty = kDutyDefault;
  int16_t *ptr = irRaw;

  if (nbits >= kLg32Bits) {
    // LG 32bit protocol is near identical to Samsung except for repeats.
    sendGeneric_64(kLg32HdrMark, kLg32HdrSpace, 
                    kLgBitMark, kLgOneSpace,
                    kLgBitMark, kLgZeroSpace,
                    kLgBitMark, kLgMinGap, 
                    kLgMinMessageLength, data,
                    nbits, 38, true, 0, 33,
                    ptr);
    ptr += (8 + nbits * 2);
    repeatHeaderMark = kLg32RptHdrMark;
    duty = 33;
    repeat++;
  } else {
    // LG (28-bit) protocol.
    repeatHeaderMark = kLgHdrMark;
    sendGeneric_64(kLgHdrMark, kLgHdrSpace, kLgBitMark, kLgOneSpace, kLgBitMark,
                    kLgZeroSpace, kLgBitMark, kLgMinGap, kLgMinMessageLength, data,
                    nbits, 38, true, 0,  // Repeats are handled later.
                    duty,
                    ptr);
    ptr += (8 + nbits * 2);
  }

  // Repeat
  // Protocol has a mandatory repeat-specific code sent after every command.
  if (repeat){
    sendGeneric_64(repeatHeaderMark, kLgRptSpace, 0, 0, 0, 0,  // No data is sent.
                    kLgBitMark, kLgMinGap, kLgMinMessageLength, 0, 0,  // No data.
                    38, true, repeat - 1, duty,
                    ptr);
    ptr+=4;
  }
}

/// Send an LG Variant-2 formatted message. (LG2)
/// Status: Beta / Should be working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
///   Typically kLgBits or kLg32Bits.
/// @param[in] repeat The number of times the command is to be repeated.
/// @note LG has a separate message to indicate a repeat, like NEC does.
void LG_send2(uint64_t data, uint16_t nbits, uint16_t repeat, int16_t *irRaw) {
  int16_t *ptr = irRaw;
  if (nbits >= kLg32Bits) {
    // Let the original routine handle it.
    LG_send(data, nbits, repeat, ptr);  // Send it as a single Samsung message.
    return;
  }

  // LGv2 (28-bit) protocol.
  sendGeneric_64(kLg2HdrMark, kLg2HdrSpace, kLg2BitMark, kLgOneSpace, kLg2BitMark,
                  kLgZeroSpace, kLg2BitMark, kLgMinGap, kLgMinMessageLength, data,
                  nbits, 38, true, 0,  // Repeats are handled later.
                  33,
                  ptr);  // Use a duty cycle of 33% (Testing)
  ptr += (8 + nbits*2);
  // TODO(crackn): Verify the details of what repeat messages look like.
  // Repeat
  // Protocol has a mandatory repeat-specific code sent after every command.
  if (repeat){
    sendGeneric_64(kLg2HdrMark, kLgRptSpace, 0, 0, 0, 0,  // No data is sent.
                    kLgBitMark, kLgMinGap, kLgMinMessageLength, 0, 0,  // No data.
                    38, true, repeat - 1, 50, 
                    ptr);
    ptr+=4;
  }
}

/// Decode the supplied LG message.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
///   Typically kLgBits or kLg32Bits.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @note LG protocol has a repeat code which is 4 items long.
///   Even though the protocol has 28/32 bits of data, only 24/28 bits are
///   distinct.
///   In transmission order, the 28/32 bits are constructed as follows:
///     8/12 bits of address + 16 bits of command + 4 bits of checksum.
/// @note LG 32bit protocol appears near identical to the Samsung protocol.
///   They possibly differ on how they repeat and initial HDR mark.
bool LG_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (nbits >= kLg32Bits) {
    if (results->rawlen <= 2 * nbits + 2 * (kHeader + kFooter) - 1 + offset)
      return false;  // Can't possibly be a valid LG32 message.
  } else {
    if (results->rawlen <= 2 * nbits + kHeader - 1 + offset)
      return false;  // Can't possibly be a valid LG message.
  }
  // Compliance
  if (strict && nbits != kLgBits && nbits != kLg32Bits)
    return false;  // Doesn't comply with expected LG protocol.

  // Header (Mark)
  uint32_t kHdrSpace;
  if (matchMark(results->rawbuf[offset], kLgHdrMark, kTolerance))
    kHdrSpace = kLgHdrSpace;
  else if (matchMark(results->rawbuf[offset], kLg2HdrMark, kTolerance))
    kHdrSpace = kLg2HdrSpace;
  else if (matchMark(results->rawbuf[offset], kLg32HdrMark, kTolerance))
    kHdrSpace = kLg32HdrSpace;
  else
    return false;
  offset++;

  // Set up the expected data section values.
  const uint16_t kBitmark = (kHdrSpace == kLg2HdrSpace) ? kLg2BitMark
                                                        : kLgBitMark;
  // Header Space + Data + Footer
  uint64_t data = 0;
  uint16_t used = matchGeneric_64(results->rawbuf + offset, &data,
                                  results->rawlen - offset, nbits,
                                  0,  // Already matched the Header mark.
                                  kHdrSpace,
                                  kBitmark, kLgOneSpace, kBitmark, kLgZeroSpace,
                                  kBitmark, kLgMinGap, true, kTolerance, 0, true);
  if (!used) return false;
    offset += used;

  // Repeat
  if (nbits >= kLg32Bits) {
    // If we are expecting the LG 32-bit protocol, there is always
    // a repeat message. So, check for it.
    uint64_t unused;
    if (!matchGeneric_64( results->rawbuf + offset, &unused,
                          results->rawlen - offset, 0,  // No Data bits to match.
                          kLg32RptHdrMark, kLgRptSpace,
                          kBitmark, kLgOneSpace, kBitmark, kLgZeroSpace,
                          kBitmark, kLgMinGap, true, kTolerance, kMarkExcess, true)) return false;
  }

  // The 16 bits before the checksum.
  uint16_t command = (data >> kLgAcChecksumSize);

  // Compliance
  if (strict && (data & 0xF) != sumNibbles_64(command, 4, 0, true))
    return false;  // The last 4 bits sent are the expected checksum.
  // Success
  if (kHdrSpace == kLg2HdrSpace)  // Was it an LG2 message?
    results->decode_type = LG2;
  else
    results->decode_type = LG;
  results->bits = nbits;
  results->value = data;
  results->command = command;
  results->address = data >> 20;  // The bits before the command.
  return true;
}


#if SEND_LG_GENERIC
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void LG_sendGeneric(const uint16_t repeat) {
  if (LG_getPower()) {
    if(LG)
      LG_send(LG_getRaw(), kLgBits, repeat);
    else
      LG_send2(LG_getRaw(), kLgBits, repeat);
    // Some models have extra/special settings & controls
    switch (LG_getModel()) {
      case AKB74955603:
        // Only send the swing setting if we need to.
        if (_swingv != _swingv_prev){
          if(LG)
            LG_send(_swingv, kLgBits, repeat);
          else
            LG_send2(_swingv, kLgBits, repeat);
        }
        // Any "normal" command sent will always turn the light on, thus we only
        // send it when we want it off. Must be sent last!
        if (!_light){
          if(LG)
            LG_send(kLgAcLightToggle, kLgBits, repeat);
          else
            LG_send2(kLgAcLightToggle, kLgBits, repeat);
        }
        break;
      case AKB73757604:
        // Check if we need to send any vane specific swingv's.
        for (uint8_t i = 0; i < kLgAcSwingVMaxVanes; i++)  // For all vanes
          if (_vaneswingv[i] != _vaneswingv_prev[i]){  // Only send if we must.
            if(LG)
              LG_send(LG_calcVaneSwingV(i, _vaneswingv[i]), kLgBits, repeat);
            else
              LG_send2(LG_calcVaneSwingV(i, _vaneswingv[i]), kLgBits, repeat);
          } 
        // and if we need to send a swingh message.
        if (_swingh != _swingh_prev){
           if(LG)
              LG_send(_swingh ? kLgAcSwingHAuto : kLgAcSwingHOff, kLgBits, repeat);
            else
              LG_send2(_swingh ? kLgAcSwingHAuto : kLgAcSwingHOff, kLgBits, repeat);
        }
        break;
      default:
        break;
    }
    LG_updateSwingPrev();  // Swing changes will have been sent, so make them prev.
  } else {
    // Always send the special Off command if the power is set to off.
    if(LG)
      LG_send(kLgAcOffCommand, kLgBits, repeat);
    else
      LG_send2(kLgAcOffCommand, kLgBits, repeat);
  }
}
#endif  // SEND_LG_GENERIC


/// Is the current message a normal (non-special) message?
/// @return True, if it is a normal message, False, if it is special.
bool _isNormal(void) {
  switch (_LGProtocol.raw) {
    case kLgAcOffCommand:
    case kLgAcLightToggle:
      return false;
  }
  if (LG_isSwing()) return false;
  return true;
}

/// Set the model of the A/C to emulate.
/// @param[in] model The enum of the appropriate model.
void LG_setModel(const lg_ac_remote_model_t model) {
  switch (model) {
    case AKB75215403:
    case AKB74955603:
    case AKB73757604:
      _protocol = LG2;
      break;
    case GE6711AR2853M:
      _protocol = LG;
      break;
    default:
      return;
  }
  _model = model;
}

/// Get the model of the A/C.
/// @return The enum of the compatible model.
lg_ac_remote_model_t LG_getModel(void) {
  return _model;
}

/// Check if the stored code must belong to a AKB74955603 model.
/// @return true, if it is AKB74955603 message. Otherwise, false.
/// @note Internal use only.
static bool _isAKB74955603(void) {
  return ((_LGProtocol.raw & kLgAcAKB74955603DetectionMask) && _isNormal()) ||
      LG_isSwingV() || LG_isLightToggle();
}

/// Check if the stored code must belong to a AKB73757604 model.
/// @return true, if it is AKB73757604 message. Otherwise, false.
/// @note Internal use only.
static bool _isAKB73757604(void) {
  return LG_isSwingH() || LG_isVaneSwingV();
}

/// Get a copy of the internal state/code for this protocol.
/// @return The code for this protocol based on the current internal state.
uint32_t LG_getRaw(void) {
  LG_checksum();
  return _LGProtocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] protocol A valid decode protocol type to use.
void LG_setRaw(const uint32_t new_code, const decode_type_t protocol) {
  _LGProtocol.raw = new_code;
  // Set the default model for this protocol, if the protocol is supplied.
  switch (protocol) {
    case LG:
      LG_setModel(GE6711AR2853M);
      break;
    case LG2:
      LG_setModel(AKB75215403);
      break;
    default:
      // Don't change anything if it isn't an expected protocol.
      break;
  }
  // Look for model specific settings/features to improve model detection.
  if (_isAKB74955603()) {
    LG_setModel(AKB74955603);
    if (LG_isSwingV()) _swingv = new_code;
  }
  if (_isAKB73757604()) {
    LG_setModel(AKB73757604);
    if (LG_isVaneSwingV()) {
      // Extract just the vane nr and position part of the message.
      const uint32_t vanecode = LG_getVaneCode(_LGProtocol.raw);
      _vaneswingv[vanecode / kLgAcVaneSwingVSize] = VANESWINGVPOS(vanecode);
    } else if (LG_isSwingH()) {
      _swingh = (_LGProtocol.raw == kLgAcSwingHAuto);
    }
  }
  _temp = 15;  // Ensure there is a "sane" previous temp.
  _temp = LG_getTemp();
}

/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @return The calculated checksum value.
uint8_t LG_calcChecksum(const uint32_t state) {
  return sumNibbles_64(state >> kLgAcChecksumSize, 4, 0, true);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The value to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool LG_validChecksum(const uint32_t state) {
  LGProtocol_t LGp;
  LGp.raw = state;
  return LG_calcChecksum(state) == LGp.Sum;
}

/// Calculate and set the checksum values for the internal state.
void LG_checksum(void) {
  _LGProtocol.Sum = LG_calcChecksum(_LGProtocol.raw);
}


/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void LG_setPower(const bool on) {
  _LGProtocol.Power = (on ? kLgAcPowerOn : kLgAcPowerOff);
  if (on)
    LG_setTemp(_temp);  // Reset the temp if we are on.
  else
    _setTemp(0);  // Off clears the temp.
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool LG_getPower(void) {
  return _LGProtocol.Power == kLgAcPowerOn;
}

/// Is the message a Power Off message?
/// @return true, if it is. false, if not.
bool LG_isOffCommand(void) { return _LGProtocol.raw == kLgAcOffCommand; }

/// Change the light/led/display setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void LG_setLight(const bool on) { _light = on; }

/// Get the value of the current light setting.
/// @return true, the setting is on. false, the setting is off.
bool LG_getLight(void) { return _light; }

/// Is the message a Light Toggle message?
/// @return true, if it is. false, if not.
bool LG_isLightToggle(void) { return _LGProtocol.raw == kLgAcLightToggle; }

/// Set the temperature.
/// @param[in] value The native temperature.
/// @note Internal use only.
inline void _setTemp(const uint8_t value) { _LGProtocol.Temp = value; }

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
void LG_setTemp(const uint8_t degrees) {
  uint8_t temp = MAX(kLgAcMinTemp, degrees + kLgAcMinTemp);
  temp = MIN(kLgAcMaxTemp, temp);
  _temp = temp;
  _setTemp(temp - kLgAcTempAdjust);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t LG_getTemp(void) {
  return _isNormal() ? _LGProtocol.Temp -1 : _temp;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void LG_setFan(const uint8_t speed) {
  uint8_t _speed = speed;
  // Only model AKB74955603 has these speeds, so convert if we have to.
  if (LG_getModel() != AKB74955603) {
    switch (speed) {
      case kLgAcFanLowAlt:
        _LGProtocol.Fan = kLgAcFanLow;
        return;
      case kLgAcFanHigh:
        _LGProtocol.Fan = kLgAcFanMax;
        return;
    }
  }
  switch (speed) {
    case kLgAcFanLow:
    case kLgAcFanLowAlt:
      _speed = (LG_getModel() != AKB74955603)
          ? kLgAcFanLow : kLgAcFanLowAlt;
      break;
    case kLgAcFanHigh:
      _speed = (LG_getModel() != AKB74955603)
          ? kLgAcFanMax : speed;
      break;
    case kLgAcFanAuto:
    case kLgAcFanLowest:
    case kLgAcFanMedium:
    case kLgAcFanMax:
      _speed = speed;
      break;
    default:
      _speed = kLgAcFanAuto;
  }
  _LGProtocol.Fan = _speed;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t LG_getFan(void) { return _LGProtocol.Fan; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t LG_getMode(void) { return _LGProtocol.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void LG_setMode(const uint8_t mode) {
  switch (mode) {
    case kLgAcAuto:
    case kLgAcDry:
    case kLgAcHeat:
    case kLgAcCool:
    case kLgAcFan:
      _LGProtocol.Mode = mode;
      break;
    default:
      _LGProtocol.Mode = kLgAcAuto;
  }
}

/// Check if the stored code is a Swing message.
/// @return true, if it is. Otherwise, false.
bool LG_isSwing(void) {
  return (_LGProtocol.raw >> 12) == kLgAcSwingSignature;
}

/// Check if the stored code is a non-vane SwingV message.
/// @return true, if it is. Otherwise, false.
bool LG_isSwingV(void) {
  const uint32_t code = _LGProtocol.raw >> kLgAcChecksumSize;
  return code >= (kLgAcSwingVLowest >> kLgAcChecksumSize) &&
      code < (kLgAcSwingHAuto >> kLgAcChecksumSize);
}

/// Check if the stored code is a SwingH message.
/// @return true, if it is. Otherwise, false.
bool LG_isSwingH(void) {
  return (_LGProtocol.raw >> kLgAcSwingHOffsetSize) == kLgAcSwingHSignature;
}

/// Get the Horizontal Swing position setting of the A/C.
/// @return true, if it is. Otherwise, false.
bool LG_getSwingH(void) { return _swingh; }

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void LG_setSwingH(const bool on) { _swingh = on; }

/// Check if the stored code is a vane specific SwingV message.
/// @return true, if it is. Otherwise, false.
bool LG_isVaneSwingV(void) {
  return _LGProtocol.raw > kLgAcVaneSwingVBase &&
      _LGProtocol.raw < (kLgAcVaneSwingVBase +
               ((kLgAcSwingVMaxVanes *
                 kLgAcVaneSwingVSize) << kLgAcChecksumSize));
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] position The position/mode to set the vanes to.
void LG_setSwingV(const uint32_t position) {
  // Is it a valid position code?
  if (position == kLgAcSwingVOff ||
      LG_toCommonSwingV(position) != kSwingVOff) {
    if (position <= 0xFF) {  // It's a short code, convert it.
      _swingv = (kLgAcSwingSignature << 8 | position) << kLgAcChecksumSize;
      _swingv |= LG_calcChecksum(_swingv);
    } else {
      _swingv = position;
    }
  }
}

// Copy the previous swing settings from the current ones.
void LG_updateSwingPrev(void) {
  _swingv_prev = _swingv;
  for (uint8_t i = 0; i < kLgAcSwingVMaxVanes; i++)
    _vaneswingv_prev[i] = _vaneswingv[i];
}

/// Get the Vertical Swing position setting of the A/C.
/// @return The native position/mode.
uint32_t LG_getSwingV(void) { return _swingv; }

/// Set the per Vane Vertical Swing mode of the A/C.
/// @param[in] vane The nr. of the vane to control.
/// @param[in] position The position/mode to set the vanes to.
void LG_setVaneSwingV(const uint8_t vane, const uint8_t position) {
  if (vane < kLgAcSwingVMaxVanes)  // It's a valid vane nr.
    if (position && position <= kLgAcVaneSwingVLowest)  // Valid position
      _vaneswingv[vane] = position;
}

/// Get the Vertical Swing position for the given vane of the A/C.
/// @return The native position/mode.
uint8_t LG_getVaneSwingV(const uint8_t vane ) {
  return (vane < kLgAcSwingVMaxVanes) ? _vaneswingv[vane] : 0;
}

/// Get the vane code of a Vane Vertical Swing message.
/// @param[in] raw A raw number representing a native LG message.
/// @return A number containing just the vane nr, and the position.
uint8_t LG_getVaneCode(const uint32_t raw) {
  return (raw - kLgAcVaneSwingVBase) >> kLgAcChecksumSize;
}

/// Calculate the Vane specific Vertical Swing code for the A/C.
/// @return The native raw code.
uint32_t LG_calcVaneSwingV(const uint8_t vane, const uint8_t position) {
  uint32_t result = kLgAcVaneSwingVBase;
  if (vane < kLgAcSwingVMaxVanes)  // It's a valid vane nr.
    if (position && position <= kLgAcVaneSwingVLowest)  // Valid position
      result += ((vane * kLgAcVaneSwingVSize + position) << kLgAcChecksumSize);
  return result | LG_calcChecksum(result);
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t LG_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kLgAcCool;
    case kOpModeHeat: return kLgAcHeat;
    case kOpModeFan:  return kLgAcFan;
    case kOpModeDry:  return kLgAcDry;
    default:          return kLgAcAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t LG_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kLgAcCool: return kOpModeCool;
    case kLgAcHeat: return kOpModeHeat;
    case kLgAcDry:  return kOpModeDry;
    case kLgAcFan:  return kOpModeFan;
    default:        return kOpModeAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t LG_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kLgAcFanLowest;
    case kFanSpeedLow:    return kLgAcFanLow;
    case kFanSpeedMedium: return kLgAcFanMedium;
    case kFanSpeedHigh:   return kLgAcFanHigh;
    case kFanSpeedMax:    return kLgAcFanMax;
    default:                         return kLgAcFanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t LG_toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kLgAcFanMax:     return kFanSpeedMax;
    case kLgAcFanHigh:    return kFanSpeedHigh;
    case kLgAcFanMedium:  return kFanSpeedMedium;
    case kLgAcFanLow:
    case kLgAcFanLowAlt:  return kFanSpeedLow;
    case kLgAcFanLowest:  return kFanSpeedMin;
    default:              return kFanSpeedAuto;
  }
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] swingv The enum to be converted.
/// @return The native equivalent of the enum.
uint32_t LG_convertSwingV(const swingv_t swingv) {
  switch (swingv) {
    case kSwingVHighest: return kLgAcSwingVHighest;
    case kSwingVHigh:    return kLgAcSwingVHigh;
    case kSwingVMiddle:  return kLgAcSwingVMiddle;
    case kSwingVLow:     return kLgAcSwingVLow;
    case kSwingVLowest:  return kLgAcSwingVLowest;
    case kSwingVAuto:    return kLgAcSwingVSwing;
    default:                        return kLgAcSwingVOff;
  }
}

/// Convert a native Vertical Swing into its stdAc equivalent.
/// @param[in] code The native code to be converted.
/// @return The stdAc equivalent of the native setting.
swingv_t LG_toCommonSwingV(const uint32_t code) {
  switch (code) {
    case kLgAcSwingVHighest_Short:
    case kLgAcSwingVHighest: return kSwingVHighest;
    case kLgAcSwingVHigh_Short:
    case kLgAcSwingVHigh:    return kSwingVHigh;
    case kLgAcSwingVUpperMiddle_Short:
    case kLgAcSwingVUpperMiddle:
    case kLgAcSwingVMiddle_Short:
    case kLgAcSwingVMiddle:  return kSwingVMiddle;
    case kLgAcSwingVLow_Short:
    case kLgAcSwingVLow:     return kSwingVLow;
    case kLgAcSwingVLowest_Short:
    case kLgAcSwingVLowest:  return kSwingVLowest;
    case kLgAcSwingVSwing_Short:
    case kLgAcSwingVSwing:   return kSwingVAuto;
    default:                 return kSwingVOff;
  }
}

/// Convert a native Vane specific Vertical Swing into its stdAc equivalent.
/// @param[in] pos The native position to be converted.
/// @return The stdAc equivalent of the native setting.
swingv_t LG_toCommonVaneSwingV(const uint8_t pos) {
  switch (pos) {
    case kLgAcVaneSwingVHigh:    return kSwingVHigh;
    case kLgAcVaneSwingVUpperMiddle:
    case kLgAcVaneSwingVMiddle:  return kSwingVMiddle;
    case kLgAcVaneSwingVLow:     return kSwingVLow;
    case kLgAcVaneSwingVLowest:  return kSwingVLowest;
    default:                     return kSwingVHighest;
  }
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] swingv The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t LG_convertVaneSwingV(const swingv_t swingv) {
  switch (swingv) {
    case kSwingVHigh:    return kLgAcVaneSwingVHigh;
    case kSwingVMiddle:  return kLgAcVaneSwingVMiddle;
    case kSwingVLow:     return kLgAcVaneSwingVLow;
    case kSwingVLowest:  return kLgAcVaneSwingVLowest;
    default:                        return kLgAcVaneSwingVHighest;
  }
}

/// Check if the internal state looks like a valid LG A/C message.
/// @return true, the internal state is a valid LG A/C mesg. Otherwise, false.
bool LG_isValidLgAc(void) {
  return LG_validChecksum(_LGProtocol.raw) && (_LGProtocol.Sign == kLgAcSignature);
}


