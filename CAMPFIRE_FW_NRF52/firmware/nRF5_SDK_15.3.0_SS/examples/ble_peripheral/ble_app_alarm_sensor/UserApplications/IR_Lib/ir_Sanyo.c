

#include "ampm_gt999.h"
#include "app_mesh.h"
#include "nrf_log.h"
#include "app_util_platform.h"
#include "app_ac_status.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "IRsend.h"
#include "IRutils.h"
#include "IRrecv.h"
#include "ir_Sanyo.h"
#include "IR_Common.h"



const uint16_t kSanyoAcHdrMark    = 8500;   ///< uSeconds
const uint16_t kSanyoAcHdrSpace   = 4200;  ///< uSeconds
const uint16_t kSanyoAcBitMark    = 500;    ///< uSeconds
const uint16_t kSanyoAcOneSpace   = 1600;  ///< uSeconds
const uint16_t kSanyoAcZeroSpace  = 550;  ///< uSeconds
const uint16_t kSanyoAcGap        = kSanyoAcHdrSpace;  ///< uSeconds (Guess only)
const uint16_t kSanyoAcFreq       = 38000;  ///< Hz. (Guess only)




static SanyoProtocol_t _Sanyo = { .raw = {0x6A, 0x6D, 0x51, 0x00, 0x10, 0x45, 0x00, 0x00, 0x33}};


void encode_Sanyo(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  Sanyo_setPower(ac_status.power_status);
  Sanyo_setTemp(ac_status.temperature);
  Sanyo_setMode(Sanyo_convertMode(ac_status.mode));
  Sanyo_setFan(Sanyo_convertFan(ac_status.fan));

  if(ac_status.swing)
    Sanyo_setSwingV(kSanyoAcSwingVAuto);
  else
    Sanyo_setSwingV(kSanyoAcSwingVLowest);

  Sanyo_send(Sanyo_getRaw(), kSanyoAcStateLength, 0, OutputIRProtocol);
  setIrTxState(1);
}


void decode_Sanyo(int16_t* input, uint8_t* output) {
  
  // copy raw buf, init data
  // TODO: DATASET_MAX_INDEX_SANYO should be replaced by real data index
  initDecodeData(input, DATASET_MAX_INDEX_SANYO);

  if(Sanyo_recv(&gDecodeResult, 0, kSanyoAcBits, true)){
    Sanyo_setRaw(gDecodeResult.state);
  }
  output[0] = Sanyo_getPower();
  output[1] = Sanyo_getTemp();
  output[2] = Sanyo_toCommonFanSpeed(Sanyo_getFan());
  output[3] = Sanyo_getSwingV();
  output[4] = Sanyo_toCommonMode(Sanyo_getMode());

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan( output[2]);
  ac_control_set_swing(output[3] == kSanyoAcSwingVAuto);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}

// Constants

#ifdef SANYO_AC8650_7641
// Sanyo SA 8650B
const uint16_t kSanyoSa8650bHdrMark = 3500;  // seen range 3500
const uint16_t kSanyoSa8650bHdrSpace = 950;  // seen 950
const uint16_t kSanyoSa8650bOneMark = 2400;  // seen 2400
const uint16_t kSanyoSa8650bZeroMark = 700;  // seen 700
// usually see 713 - not using ticks as get number wrapround
const uint16_t kSanyoSa8650bDoubleSpaceUsecs = 800;
const uint16_t kSanyoSa8650bRptLength = 45000;

// Sanyo LC7461
const uint16_t kSanyoLc7461AddressMask = (1 << kSanyoLC7461AddressBits) - 1;
const uint16_t kSanyoLc7461CommandMask = (1 << kSanyoLC7461CommandBits) - 1;
const uint16_t kSanyoLc7461HdrMark = 9000;
const uint16_t kSanyoLc7461HdrSpace = 4500;
const uint16_t kSanyoLc7461BitMark = 560;    // 1T
const uint16_t kSanyoLc7461OneSpace = 1690;  // 3T
const uint16_t kSanyoLc7461ZeroSpace = 560;  // 1T
const uint32_t kSanyoLc7461MinCommandLength = 108000;

const uint16_t kSanyoLc7461MinGap =
    kSanyoLc7461MinCommandLength -
    (kSanyoLc7461HdrMark + kSanyoLc7461HdrSpace +
     kSanyoLC7461Bits * (kSanyoLc7461BitMark +
                         (kSanyoLc7461OneSpace + kSanyoLc7461ZeroSpace) / 2) +
     kSanyoLc7461BitMark);
#endif  //SANYO_AC8650_7641 

#ifdef SANYO_ACC88
const uint16_t kSanyoAc88HdrMark = 5400;   ///< uSeconds
const uint16_t kSanyoAc88HdrSpace = 2000;  ///< uSeconds
const uint16_t kSanyoAc88BitMark = 500;    ///< uSeconds
const uint16_t kSanyoAc88OneSpace = 1500;  ///< uSeconds
const uint16_t kSanyoAc88ZeroSpace = 750;  ///< uSeconds
const uint32_t kSanyoAc88Gap = 3675;       ///< uSeconds
const uint16_t kSanyoAc88Freq = 38000;     ///< Hz. (Guess only)
const uint8_t  kSanyoAc88ExtraTolerance = 5;  /// (%) Extra tolerance to use.
#endif //#ifdef SANYO_ACC88

#ifdef SANYO_AC8650_7641

uint64_t encodeSanyoLC7461(uint16_t address, uint8_t command) {
  // Mask our input values to ensure the correct bit sizes.
  address &= kSanyoLc7461AddressMask;
  command &= kSanyoLc7461CommandMask;

  uint64_t data = address;
  address ^= kSanyoLc7461AddressMask;  // Invert the 13 LSBs.
  // Append the now inverted address.
  data = (data << kSanyoLC7461AddressBits) | address;
  // Append the command.
  data = (data << kSanyoLC7461CommandBits) | command;
  command ^= kSanyoLc7461CommandMask;  // Invert the command.
  // Append the now inverted command.
  data = (data << kSanyoLC7461CommandBits) | command;

  return data;
}

/// Send a Sanyo LC7461 message.
/// Status: BETA / Probably works.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @note Based on \@marcosamarinho's work.
///   This protocol uses the NEC protocol timings. However, data is
///   formatted as : address(13 bits), !address, command (8 bits), !command.
///   According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
///   Information for this protocol is available at the Sanyo LC7461 datasheet.
///   Repeats are performed similar to the NEC method of sending a special
///   repeat message, rather than duplicating the entire message.
void sendSanyoLC7461(const uint64_t data, const uint16_t nbits,
                             const uint16_t repeat) {
  // This protocol appears to be another 42-bit variant of the NEC protocol.
  sendNEC(data, nbits, repeat);
}

/// Decode the supplied SANYO LC7461 message.
/// Status: BETA / Probably works.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @note Based on \@marcosamarinho's work.
///   This protocol uses the NEC protocol. However, data is
///   formatted as : address(13 bits), !address, command (8 bits), !command.
///   According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
///   Information for this protocol is available at the Sanyo LC7461 datasheet.
bool decodeSanyoLC7461(decode_results *results, uint16_t offset,
                               const uint16_t nbits, const bool strict) {
  if (strict && nbits != kSanyoLC7461Bits)
    return false;  // Not strictly in spec.
  // This protocol is basically a 42-bit variant of the NEC protocol.
  if (!decodeNEC(results, offset, nbits, false))
    return false;  // Didn't match a NEC format (without strict)

  // Bits 30 to 42+.
  uint16_t address =
      results->value >> (kSanyoLC7461Bits - kSanyoLC7461AddressBits);
  // Bits 9 to 16.
  uint8_t command =
      (results->value >> kSanyoLC7461CommandBits) & kSanyoLc7461CommandMask;
  // Compliance
  if (strict) {
    if (results->bits != nbits) return false;
    // Bits 17 to 29.
    uint16_t inverted_address =
        (results->value >> (kSanyoLC7461CommandBits * 2)) &
        kSanyoLc7461AddressMask;
    // Bits 1-8.
    uint8_t inverted_command = results->value & kSanyoLc7461CommandMask;
    if ((address ^ kSanyoLc7461AddressMask) != inverted_address)
      return false;  // Address integrity check failed.
    if ((command ^ kSanyoLc7461CommandMask) != inverted_command)
      return false;  // Command integrity check failed.
  }

  // Success
  results->decode_type = SANYO_LC7461;
  results->address = address;
  results->command = command;
  return true;
}

#endif //SANYO_AC8650_7641

/// Send a SanyoAc formatted message.
/// Status: STABLE / Reported as working.
/// @param[in] data An array of bytes containing the IR command.
/// @param[in] nbytes Nr. of bytes of data in the array.
/// @param[in] repeat Nr. of times the message is to be repeated.
void Sanyo_send(const uint8_t data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irraw) {
  // Header + Data + Footer
  sendGeneric_8(kSanyoAcHdrMark, kSanyoAcHdrSpace,
              kSanyoAcBitMark, kSanyoAcOneSpace,
              kSanyoAcBitMark, kSanyoAcZeroSpace,
              kSanyoAcBitMark, kSanyoAcGap,
              data, nbytes, kSanyoAcFreq, false, repeat, kDutyDefault, 
							irraw);
}

/// Decode the supplied SanyoAc message.
/// Status: STABLE / Reported as working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool Sanyo_recv(decode_results *results, uint16_t offset,
                  const uint16_t nbits, const bool strict) {
  if (strict && nbits != kSanyoAcBits)
    return false;

  // Header + Data + Footer
  if (!matchGeneric_8(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kSanyoAcHdrMark, kSanyoAcHdrSpace,
                      kSanyoAcBitMark, kSanyoAcOneSpace,
                      kSanyoAcBitMark, kSanyoAcZeroSpace,
                      kSanyoAcBitMark, kSanyoAcGap,
                      true,            kTolerance,
                      kMarkExcess, false)) return false;
  // Compliance
  if (strict)
    if (!Sanyo_validChecksum(results->state, nbits / 8)) return false;

  // Success
  results->decode_type = SANYO_AC;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


/// Get a PTR to the internal state/code for this protocol with all integrity
///   checks passing.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* Sanyo_getRaw(void) {
  Sanyo_checksum();
  return _Sanyo.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
void Sanyo_setRaw(const uint8_t newState[]) {
  memcpy(_Sanyo.raw, newState, kSanyoAcStateLength);
}

/// Calculate the checksum for a given state.
/// @param[in] state The array to calc the checksum of.
/// @param[in] length The length/size of the array.
/// @return The calculated checksum value.
uint8_t Sanyo_calcChecksum(const uint8_t state[], const uint16_t length) {
  return length ? sumNibbles_8(state, length - 1, 0) : 0;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool Sanyo_validChecksum(const uint8_t state[], const uint16_t length) {
  return length && state[length - 1] == Sanyo_calcChecksum(state, length);
}

/// Calculate & set the checksum for the current internal state of the remote.
void Sanyo_checksum(void) {
  // Stored the checksum value in the last byte.
  _Sanyo.frame.Sum = Sanyo_calcChecksum(_Sanyo.raw, kSanyoAcStateLength);
}


/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void Sanyo_setPower(const bool on) {
  if(on) {
    _Sanyo.frame.Power = kSanyoAcPowerOn;
  }
  else {
    _Sanyo.frame.Power = kSanyoAcPowerOff;
  }
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool Sanyo_getPower(void) {
  return (_Sanyo.frame.Power == kSanyoAcPowerOn);
}

/// Set the requested power state of the A/C to on.
void Sanyo_on(void) { Sanyo_setPower(true); }

/// Set the requested power state of the A/C to off.
void Sanyo_off(void) { Sanyo_setPower(false); }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t Sanyo_getMode(void) {
  return _Sanyo.frame.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note If we get an unexpected mode, default to AUTO.
void Sanyo_setMode(const uint8_t mode) {
  switch (mode) {
    case kSanyoAcAuto:
    case kSanyoAcCool:
    case kSanyoAcDry:
    case kSanyoAcHeat:
      _Sanyo.frame.Mode = mode;
      break;
    default: _Sanyo.frame.Mode = kSanyoAcAuto;
  }
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Sanyo_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kSanyoAcCool;
    case kOpModeHeat: return kSanyoAcHeat;
    case kOpModeDry:  return kSanyoAcDry;
    default:          return kSanyoAcAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t Sanyo_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kSanyoAcCool: return kOpModeCool;
    case kSanyoAcHeat: return kOpModeHeat;
    case kSanyoAcDry:  return kOpModeDry;
    default:           return kOpModeAuto;
  }
}

/// Set the desired temperature.
/// @param[in] degrees The temperature in degrees celsius.
void Sanyo_setTemp(const uint8_t degrees) {
  uint8_t temp = MAX((uint8_t)0, degrees);
  temp = MIN((uint8_t)14, temp);
  _Sanyo.frame.Temp = temp + kSanyoAcTempMin;
}

/// Get the current desired temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Sanyo_getTemp(void) {
  return _Sanyo.frame.Temp - kSanyoAcTempMin;
}

/// Set the sensor temperature.
/// @param[in] degrees The temperature in degrees celsius.
void Sanyo_setSensorTemp(const uint8_t degrees) {
  uint8_t temp = MAX((uint8_t)kSanyoAcTempMin, degrees);
  temp = MIN((uint8_t)kSanyoAcTempMax, temp);
  _Sanyo.frame.SensorTemp = temp - kSanyoAcTempDelta;
}

/// Get the current sensor temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t Sanyo_getSensorTemp(void) {
  return _Sanyo.frame.SensorTemp + kSanyoAcTempDelta;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void Sanyo_setFan(const uint8_t speed) {
  _Sanyo.frame.Fan = speed;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t Sanyo_getFan(void) {
  return _Sanyo.frame.Fan;
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Sanyo_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin: return kSanyoAcFanLow;
    case kFanSpeedLow: return kSanyoAcFanMedium;
    case kFanSpeedMedium:
    case kFanSpeedHigh:
    case kFanSpeedMax: return kSanyoAcFanHigh;
    default:           return kSanyoAcFanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t Sanyo_toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kSanyoAcFanHigh:   return kFanSpeedHigh;
    case kSanyoAcFanMedium: return kFanSpeedMedium;
    case kSanyoAcFanLow:    return kFanSpeedLow;
    default:                return kFanSpeedAuto;
  }
}

/// Get the vertical swing setting of the A/C.
/// @return The current swing mode setting.
uint8_t Sanyo_getSwingV(void) {
  return _Sanyo.frame.SwingV;
}

/// Set the vertical swing setting of the A/C.
/// @param[in] setting The value of the desired setting.
void Sanyo_setSwingV(const uint8_t setting) {
  if (setting == kSanyoAcSwingVAuto ||
      (setting >= kSanyoAcSwingVLowest && setting <= kSanyoAcSwingVHighest))
    _Sanyo.frame.SwingV = setting;
  else
    _Sanyo.frame.SwingV = kSanyoAcSwingVAuto;
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t Sanyo_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest:  return kSanyoAcSwingVHighest;
    case kSwingVHigh:     return kSanyoAcSwingVHigh;
    case kSwingVMiddle:   return kSanyoAcSwingVUpperMiddle;
    case kSwingVLow:      return kSanyoAcSwingVLow;
    case kSwingVLowest:   return kSanyoAcSwingVLowest;
    default:              return kSanyoAcSwingVAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common vertical swing position.
swingv_t Sanyo_toCommonSwingV(const uint8_t setting) {
  switch (setting) {
    case kSanyoAcSwingVHighest:     return kSwingVHighest;
    case kSanyoAcSwingVHigh:        return kSwingVHigh;
    case kSanyoAcSwingVUpperMiddle:
    case kSanyoAcSwingVLowerMiddle: return kSwingVMiddle;
    case kSanyoAcSwingVLow:         return kSwingVLow;
    case kSanyoAcSwingVLowest:      return kSwingVLowest;
    default:                        return kSwingVAuto;
  }
}

/// Set the Sleep (Night Setback) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Sanyo_setSleep(const bool on) {
  _Sanyo.frame.Sleep = on;
}

/// Get the Sleep (Night Setback) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Sanyo_getSleep(void) {
  return _Sanyo.frame.Sleep;
}

/// Set the Sensor Location setting of the A/C.
/// i.e. Where the ambient temperature is measured.
/// @param[in] location true is Unit/Wall, false is Remote/Room.
void Sanyo_setSensor(const bool location) {
  _Sanyo.frame.Sensor = location;
}

/// Get the Sensor Location setting of the A/C.
/// i.e. Where the ambient temperature is measured.
/// @return true is Unit/Wall, false is Remote/Room.
bool Sanyo_getSensor(void) {
  return _Sanyo.frame.Sensor;
}

/// Set the Beep setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void Sanyo_setBeep(const bool on) {
  _Sanyo.frame.Beep = on;
}

/// Get the Beep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool Sanyo_getBeep(void) {
  return _Sanyo.frame.Beep;
}

/// Get the nr of minutes the Off Timer is set to.
/// @return The timer time expressed as the number of minutes.
///   A value of 0 means the Off Timer is off/disabled.
/// @note The internal precission has a resolution of 1 hour.
uint16_t Sanyo_getOffTimer(void) {
  if (_Sanyo.frame.OffTimer)
    return _Sanyo.frame.OffHour * 60;
  else
    return 0;
}

/// Set the nr of minutes for the Off Timer.
/// @param[in] mins The timer time expressed as nr. of minutes.
///   A value of 0 means the Off Timer is off/disabled.
/// @note The internal precission has a resolution of 1 hour.
void Sanyo_setOffTimer(const uint16_t mins) {
  const uint8_t hours = MIN((uint8_t)(mins / 60), kSanyoAcHourMax);
  _Sanyo.frame.OffTimer = (hours > 0);
  _Sanyo.frame.OffHour = hours;
}

#ifdef SANYO_AC8

SanyoAc88Protocol _SanyoAc88;


#if SEND_SANYO_AC88
/// Send a SanyoAc88 formatted message.
/// Status: ALPHA / Completely untested.
/// @param[in] data An array of bytes containing the IR command.
/// @warning data's bit order may change. It is not yet confirmed.
/// @param[in] nbytes Nr. of bytes of data in the array.
/// @param[in] repeat Nr. of times the message is to be repeated.
void sendSanyoAc88(const uint8_t data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  // (Header + Data + Footer) per repeat
  sendGeneric(kSanyoAc88HdrMark, kSanyoAc88HdrSpace,
              kSanyoAc88BitMark, kSanyoAc88OneSpace,
              kSanyoAc88BitMark, kSanyoAc88ZeroSpace,
              kSanyoAc88BitMark, kSanyoAc88Gap,
              data, nbytes, kSanyoAc88Freq, false, repeat, kDutyDefault);
  space(kDefaultMessageGap);  // Make a guess at a post message gap.
}
#endif  // SEND_SANYO_AC88

#if DECODE_SANYO_AC88
/// Decode the supplied SanyoAc message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @warning data's bit order may change. It is not yet confirmed.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool decodeSanyoAc88(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (strict && nbits != kSanyoAc88Bits)
    return false;

  uint16_t used = 0;
  // Compliance
  const uint16_t expected_repeats = strict ? kSanyoAc88MinRepeat : 0;

  // Handle the expected nr of repeats.
  for (uint16_t r = 0; r <= expected_repeats; r++) {
    // Header + Data + Footer
    used = matchGeneric(results->rawbuf + offset, results->state,
                        results->rawlen - offset, nbits,
                        kSanyoAc88HdrMark, kSanyoAc88HdrSpace,
                        kSanyoAc88BitMark, kSanyoAc88OneSpace,
                        kSanyoAc88BitMark, kSanyoAc88ZeroSpace,
                        kSanyoAc88BitMark,
                        // Expect an inter-message gap, or just the end of msg?
                        (r < expected_repeats) ? kSanyoAc88Gap
                                               : kDefaultMessageGap,
                        r == expected_repeats,
                        _tolerance + kSanyoAc88ExtraTolerance,
                        kMarkExcess, false);
    if (!used) return false;  // No match!
    offset += used;
  }

  // Success
  results->decode_type = decode_type_t::SANYO_AC88;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_SANYO_AC88

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRSanyoAc88::IRSanyoAc88(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Reset the state of the remote to a known good state/sequence.
/// @see https://docs.google.com/spreadsheets/d/1dYfLsnYvpjV-SgO8pdinpfuBIpSzm8Q1R5SabrLeskw/edit?ts=5f0190a5#gid=1050142776&range=A2:B2
void IRSanyoAc88::stateReset(void) {
  static const uint8_t kReset[kSanyoAc88StateLength] = {
    0xAA, 0x55, 0xA0, 0x16, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10};
  memcpy(_Sanyo.raw, kReset, kSanyoAc88StateLength);
}

/// Set up hardware to be able to send a message.
void IRSanyoAc88::begin(void) { begin(); }

#if SEND_SANYO_AC
/// Send the current internal state as IR messages.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRSanyoAc88::send(const uint16_t repeat) {
  sendSanyoAc88(getRaw(), kSanyoAc88StateLength, repeat);
}
#endif  // SEND_SANYO_AC

/// Get a PTR to the internal state/code for this protocol with all integrity
///   checks passing.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IRSanyoAc88::getRaw(void) {
  return _Sanyo.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
void IRSanyoAc88::setRaw(const uint8_t newState[]) {
  memcpy(_Sanyo.raw, newState, kSanyoAc88StateLength);
}

/// Set the requested power state of the A/C to on.
void IRSanyoAc88::on(void) { setPower(true); }

/// Set the requested power state of the A/C to off.
void IRSanyoAc88::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRSanyoAc88::setPower(const bool on) {   _Sanyo.frame.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRSanyoAc88::getPower(void) { return _Sanyo.frame.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRSanyoAc88::getMode(void) { return _Sanyo.frame.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note If we get an unexpected mode, default to AUTO.
void IRSanyoAc88::setMode(const uint8_t mode) {
  switch (mode) {
    case kSanyoAc88Auto:
    case kSanyoAc88FeelCool:
    case kSanyoAc88Cool:
    case kSanyoAc88FeelHeat:
    case kSanyoAc88Heat:
    case kSanyoAc88Fan:
      _Sanyo.frame.Mode = mode;
      break;
    default: _Sanyo.frame.Mode = kSanyoAc88Auto;
  }
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRSanyoAc88::convertMode(const opmode_t mode) {
  switch (mode) {
    case kCool: return kSanyoAc88Cool;
    case kHeat: return kSanyoAc88Heat;
    case kFan:  return kSanyoAc88Fan;
    default:                     return kSanyoAc88Auto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t IRSanyoAc88::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kSanyoAc88FeelCool:
    case kSanyoAc88Cool:
      return kCool;
    case kSanyoAc88FeelHeat:
    case kSanyoAc88Heat:
      return kHeat;
    case kSanyoAc88Fan:
      return kFan;
    default:
      return kAuto;
  }
}

/// Set the desired temperature.
/// @param[in] degrees The temperature in degrees celsius.
void IRSanyoAc88::setTemp(const uint8_t degrees) {
  uint8_t temp = MAX((uint8_t)kSanyoAc88TempMin, degrees);
  _Sanyo.frame.Temp = MIN((uint8_t)kSanyoAc88TempMax, temp);
}

/// Get the current desired temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRSanyoAc88::getTemp(void) { return _Sanyo.frame.Temp; }

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRSanyoAc88::setFan(const uint8_t speed) { _Sanyo.frame.Fan = speed; }

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t IRSanyoAc88::getFan(void) { return _Sanyo.frame.Fan; }

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRSanyoAc88::convertFan(const fanspeed_t speed) {
  switch (speed) {
    case fanspeed_t::kMin:
    case fanspeed_t::kLow:    return kSanyoAc88FanLow;
    case fanspeed_t::kMedium: return kSanyoAc88FanMedium;
    case fanspeed_t::kHigh:
    case fanspeed_t::kMax:    return kSanyoAc88FanHigh;
    default:                         return kSanyoAc88FanAuto;
  }
}

/// Get the current clock time.
/// @return The time as the nr. of minutes past midnight.
uint16_t IRSanyoAc88::getClock(void) {
  return _Sanyo.frame.ClockHrs * 60 + _Sanyo.frame.ClockMins;
}

/// Set the current clock time.
/// @param[in] mins_since_midnight The time as nr. of minutes past midnight.
void IRSanyoAc88::setClock(const uint16_t mins_since_midnight) {
  uint16_t mins = MIN(mins_since_midnight, (uint16_t)(23 * 60 + 59));
  _Sanyo.frame.ClockMins = mins % 60;
  _Sanyo.frame.ClockHrs = mins / 60;
  _Sanyo.frame.ClockSecs = 0;
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t IRSanyoAc88::toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kSanyoAc88FanHigh:   return fanspeed_t::kHigh;
    case kSanyoAc88FanMedium: return fanspeed_t::kMedium;
    case kSanyoAc88FanLow:    return fanspeed_t::kLow;
    default:                  return fanspeed_t::kAuto;
  }
}

/// Change the SwingV setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRSanyoAc88::setSwingV(const bool on) { _Sanyo.frame.SwingV = on; }

/// Get the value of the current SwingV setting.
/// @return true, the setting is on. false, the setting is off.
bool IRSanyoAc88::getSwingV(void) { return _Sanyo.frame.SwingV; }

/// Change the Turbo setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRSanyoAc88::setTurbo(const bool on) { _Sanyo.frame.Turbo = on; }

/// Get the value of the current Turbo setting.
/// @return true, the setting is on. false, the setting is off.
bool IRSanyoAc88::getTurbo(void) { return _Sanyo.frame.Turbo; }

/// Change the Filter setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRSanyoAc88::setFilter(const bool on) { _Sanyo.frame.Filter = on; }

/// Get the value of the current Filter setting.
/// @return true, the setting is on. false, the setting is off.
bool IRSanyoAc88::getFilter(void) { return _Sanyo.frame.Filter; }

/// Change the Sleep setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRSanyoAc88::setSleep(const bool on) { _Sanyo.frame.Sleep = on; }

/// Get the value of the current Sleep setting.
/// @return true, the setting is on. false, the setting is off.
bool IRSanyoAc88::getSleep(void) { return _Sanyo.frame.Sleep; }

/// Convert the current internal state into its state_t equivalent.
/// @return The stdAc equivalent of the native settings.
state_t IRSanyoAc88::toCommon(void) {
  state_t result;
  result.protocol = decode_type_t::SANYO_AC88;
  result.model = -1;  // Not supported.
  result.power = getPower();
  result.mode = toCommonMode(_Sanyo.frame.Mode);
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(_Sanyo.frame.Fan);
  result.swingv = _Sanyo.frame.SwingV ? kAuto : kOff;
  result.filter = _Sanyo.frame.Filter;
  result.turbo = _Sanyo.frame.Turbo;
  result.sleep = _Sanyo.frame.Sleep ? 0 : -1;
  result.clock = getClock();
  // Not supported.
  result.swingh = swingh_t::kOff;
  result.econo = false;
  result.light = false;
  result.quiet = false;
  result.beep = false;
  result.clean = false;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRSanyoAc88::toString(void) {
  String result = "";
  result.reserve(115);
  result += addBoolToString(getPower(), kPowerStr, false);
  result += addModeToString(_Sanyo.frame.Mode, kSanyoAc88Auto, kSanyoAc88Cool,
                            kSanyoAc88Heat, kSanyoAc88Auto, kSanyoAc88Fan);
  result += addTempToString(getTemp());
  result += addFanToString(_Sanyo.frame.Fan, kSanyoAc88FanHigh, kSanyoAc88FanLow,
                           kSanyoAc88FanAuto, kSanyoAc88FanAuto,
                           kSanyoAc88FanMedium);
  result += addBoolToString(_Sanyo.frame.SwingV, kSwingVStr);
  result += addBoolToString(_Sanyo.frame.Turbo, kTurboStr);
  result += addBoolToString(_Sanyo.frame.Sleep, kSleepStr);
  result += addLabeledString(minsToString(getClock()), kClockStr);
  return result;
}

#endif // SANYO_AC8
