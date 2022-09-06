
/// @file
/// @brief Support for Panasonic protocols.

#include "string.h"

#include "ir_Panasonic32.h"
#include "ir_Panasonic.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

#include "app_ac_status.h"
#include "IR_Common.h"

PanasonicAc32Protocol_t _PanasonicAc32Protocol = { .raw = kPanasonicAc32KnownGood };  ///< The state in code form.

bool _isLongCode = true; // default is long code

// Constants
const uint16_t kPanasonicAc32HdrMark          = 3543;         ///< uSeconds.
const uint16_t kPanasonicAc32BitMark          = 920;          ///< uSeconds.
const uint16_t kPanasonicAc32HdrSpace         = 3450;        ///< uSeconds.
const uint16_t kPanasonicAc32OneSpace         = 2575;        ///< uSeconds.
const uint16_t kPanasonicAc32ZeroSpace        = 828;        ///< uSeconds.
const uint16_t kPanasonicAc32SectionGap       = 13946;     ///< uSeconds.
const uint8_t  kPanasonicAc32Sections         = 2;
const uint8_t  kPanasonicAc32BlocksPerSection = 2;




void encode_PanasonicAc32(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  PanasonicAc32_setPowerToggle(ac_status.power_status);
  PanasonicAc32_setTemp(ac_status.temperature);
  PanasonicAc32_setMode(PanasonicAc32_convertMode(ac_status.mode));
  PanasonicAc32_setFan(PanasonicAc32_convertFan(ac_status.fan));

  if(ac_status.swing)
    PanasonicAc32_setSwingVertical(kPanasonicAc32SwingVAuto);
  else
    PanasonicAc32_setSwingVertical(kPanasonicAcSwingVLowest);

  if(_isLongCode){
    PanasonicAc32_send(kPanasonicAc32Bits, OutputIRProtocol);
  }
  else{
    PanasonicAc32_send(kPanasonicAc32Bits/2, OutputIRProtocol);
  }
  setIrTxState(1);
}


void decode_PanasonicAc32(int16_t* input, uint8_t* output) {
  
  // copy raw buf, init data
  initDecodeData(input, DATASET_MAX_INDEX_PANASONICAC32_LONG);
  
  if( PanasonicAc32_recv(&gDecodeResult, 0, kPanasonicAc32Bits, false) ){
    _isLongCode = true;
    PanasonicAc32_setRaw(  gDecodeResult.state[0]        |
                          (gDecodeResult.state[1] << 8)  |
                          (gDecodeResult.state[2] << 16) |
                          (gDecodeResult.state[3] << 24) );
  }
  else if(PanasonicAc32_recv(&gDecodeResult, 0, kPanasonicAc32Bits/2, false)){
    _isLongCode = false;
    PanasonicAc32_setRaw(  gDecodeResult.state[0]        |
                          (gDecodeResult.state[1] << 8)  |
                          (gDecodeResult.state[2] << 16) |
                          (gDecodeResult.state[3] << 24) );
  }

  output[0] = PanasonicAc32_getPowerToggle();
  output[1] = PanasonicAc32_getTemp();
  output[2] = PanasonicAc32_toCommonFanSpeed(PanasonicAc32_getFan());
  output[3] = PanasonicAc32_getSwingVertical();
  output[4] = PanasonicAc32_toCommonMode(PanasonicAc32_getMode());

  ac_control_set_power_status(output[0]);
  ac_control_set_temperature(output[1]);
  ac_control_set_fan( output[2]);
  ac_control_set_swing(output[3] == kPanasonicAc32SwingVAuto);
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}




/// Send a Panasonic AC 32/16bit formatted message.
/// Status: STABLE / Confirmed working.
/// @param[in] data containing the IR command.
/// @param[in] nbits Nr. of bits to send. Usually kPanasonicAc32Bits
/// @param[in] repeat Nr. of times the message is to be repeated.
void PanasonicAc32_send(const uint16_t nbits, int16_t *irRaw) {
  uint16_t section_bits;
  uint16_t sections;
  uint16_t blocks;
  int16_t *ptr = irRaw;
  // Calculate the section, block, and bit sizes based on the requested bit size
  // Calculate the section, block, and bit sizes based on the requested bit size
  if (nbits > kPanasonicAc32Bits / 2) {  // A long message
    section_bits = nbits / kPanasonicAc32Sections;
    sections = kPanasonicAc32Sections;
    blocks = kPanasonicAc32BlocksPerSection;
  } else {  // A short message
    section_bits = nbits;
    sections = kPanasonicAc32Sections - 1;
    blocks = kPanasonicAc32BlocksPerSection + 1;
  }

  for (uint16_t r = 0; r <= kPanasonicAcDefaultRepeat; r++) {
    for (uint8_t section = 0; section < sections;  section++) {
      uint64_t section_data;
      section_data = GETBITS64(PanasonicAc32_getRaw(), section_bits * (sections - section - 1), section_bits);

      // Duplicate bytes in the data.
      uint64_t expanded_data = 0;
      for (uint8_t i = 0; i < sizeof(expanded_data); i++) {
       const uint8_t first_byte = section_data >> 56;
       for (uint8_t i = 0; i < 2; i++)
         expanded_data = (expanded_data << 8) | first_byte;
       section_data <<= 8;
      }
      // Two data blocks per section (i.e. 1 + a repeat)
      sendGeneric_64(kPanasonicAc32HdrMark, kPanasonicAc32HdrSpace,  // Header
                      kPanasonicAc32BitMark, kPanasonicAc32OneSpace,  // Data
                      kPanasonicAc32BitMark, kPanasonicAc32ZeroSpace,
                      0, 0,  // No Footer
                      0, //message time
                      expanded_data, section_bits * 2, 
                      kPanasonicFreq, false,
                      blocks - 1,  // Repeat
                      50,
                      ptr);
      ptr += ((section_bits * 2 * 2) + 2) * 2;  // (raw indexes for data + 2 raw indexes for header) * repeat
      // Section Footer
      sendGeneric_64(kPanasonicAc32HdrMark, kPanasonicAc32HdrSpace,  // Header
                      0, 0, 0, 0,  // No Data
                      kPanasonicAc32BitMark, section ? 0 : kPanasonicAc32SectionGap,  // Footer
                      0, //message time
                      PanasonicAc32_getRaw(), 0,  // No data (bits)
                      kPanasonicFreq, true, 0, 50,
                      ptr);
      ptr += 4; // incresing indexes for header + footer
    }
  }
}

/// Decode the supplied Panasonic AC 32/16bit message.
/// Status: STABLE / Confirmed working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
///   Typically: kPanasonicAc32Bits or kPanasonicAc32Bits/2
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @note Protocol has two known configurations:
///   (long)
///   Two sections of identical 32 bit data block pairs. ie. (32+32)+(32+32)=128
///   or
///   (short)
///   A single section of 3 x identical 32 bit data blocks i.e. (32+32+32)=96
/// Each data block also has a pair of 8 bits repeated identical bits.
/// e.g. (8+8)+(8+8)=32
///
/// So each long version really only has 32 unique bits, and the short version
/// really only has 16 unique bits.
bool PanasonicAc32_recv(decode_results *results, uint16_t offset,
                                 const uint16_t nbits, const bool strict) {
  if (strict && (nbits != kPanasonicAc32Bits &&
                 nbits != kPanasonicAc32Bits / 2))
    return false;  // Not strictly a valid bit size.

  // Determine if this is a long or a short message we are looking for.
  const bool is_long = (nbits > kPanasonicAc32Bits / 2);
  const uint16_t min_length = is_long ?
      kPanasonicAc32Sections * kPanasonicAc32BlocksPerSection *
      ((2 * nbits) + kHeader + kFooter) - 1 + offset :
      (kPanasonicAc32BlocksPerSection + 1) * ((4 * nbits) + kHeader) +
      kFooter - 1 + offset;

  if (results->rawlen < min_length)
    return false;  // Can't possibly be a valid message.

  // Calculate the parameters for the decode based on it's length.
  uint16_t sections;
  uint16_t blocks_per_section;
  if (is_long) {
    sections = kPanasonicAc32Sections;
    blocks_per_section = kPanasonicAc32BlocksPerSection;
  } else {
    sections = kPanasonicAc32Sections - 1;
    blocks_per_section = kPanasonicAc32BlocksPerSection + 1;
  }
  const uint16_t bits_per_block = nbits / sections;

  uint64_t data = 0;
  uint64_t section_data = 0;
  uint32_t prev_section_data;

  // Match all the expected data blocks.
  for (uint16_t block = 0; block < sections * blocks_per_section; block++) {
    prev_section_data = section_data;
    uint16_t used = matchGeneric_64(results->rawbuf + offset, &section_data,
                                    results->rawlen - offset, bits_per_block * 2,
                                    kPanasonicAc32HdrMark, kPanasonicAc32HdrSpace,
                                    kPanasonicAc32BitMark, kPanasonicAc32OneSpace,
                                    kPanasonicAc32BitMark, kPanasonicAc32ZeroSpace,
                                    0, 0,  // No Footer
                                    false, kTolerance, kMarkExcess, false);
    if (!used) return false;
    offset += used;
    // Is it the first block of the section?
    if (block % blocks_per_section == 0) {
      // The protocol repeats each byte twice, so to shrink the code we
      // remove the duplicate bytes in the collected data. We only need to do
      // this for the first block in a section.
      uint64_t shrunk_data = 0;
      uint64_t data_copy = section_data;
      for (uint8_t i = 0; i < sizeof(data_copy); i += 2) {
        const uint8_t first_byte = GETBITS64(data_copy,
                                             (sizeof(data_copy) - 1) * 8, 8);
        shrunk_data = (shrunk_data << 8) | first_byte;
        // Compliance
        if (strict) {
          // Every second byte must be a duplicate of the previous.
          const uint8_t next_byte = GETBITS64(data_copy,
                                              (sizeof(data_copy) - 2) * 8, 8);
          if (first_byte != next_byte) return false;
        }
        data_copy <<= 16;
      }
      // Keep the data from the first of the block in the section.
      data = (data << bits_per_block) | shrunk_data;
    } else {  // Not the first block in a section.
      // Compliance
      if (strict)
        // Compare the data from the blocks in pairs.
        if (section_data != prev_section_data) return false;
      // Look for the section footer at the end of the blocks.
      if ((block + 1) % blocks_per_section == 0) {
        uint64_t junk;
        used = matchGeneric_64(results->rawbuf + offset, &junk,
                            results->rawlen - offset, 0,
                            // Header
                            kPanasonicAc32HdrMark, kPanasonicAc32HdrSpace,
                            // No Data
                            0, 0,
                            0, 0,
                            // Footer
                            kPanasonicAc32BitMark, kPanasonicAc32SectionGap,
                            true, kTolerance, kMarkExcess, true);
        if (!used) return false;
        offset += used;
      }
    }
  }

  // Success
  results->value = data;
  results->address = 0;
  results->command = 0;
  results->decode_type = PANASONIC_AC32;
  results->bits = nbits;
  return true;
}

/// Class constructor


/// Get a copy of the internal state/code for this protocol.
/// @return The code for this protocol based on the current internal state.
uint32_t PanasonicAc32_getRaw(void) { return _PanasonicAc32Protocol.raw; }

/// Set the internal state from a valid code for this protocol.
/// @param[in] state A valid code for this protocol.
void PanasonicAc32_setRaw(const uint32_t state) {  _PanasonicAc32Protocol.raw = state; }

/// Reset the state of the remote to a known good state/sequence.
void PanasonicAc32_stateReset(void) { PanasonicAc32_setRaw(kPanasonicAc32KnownGood); }

/// Set the Power Toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void PanasonicAc32_setPowerToggle(const bool on) { _PanasonicAc32Protocol.frame.PowerToggle = !on; }

/// Get the Power Toggle setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool PanasonicAc32_getPowerToggle(void) { return !_PanasonicAc32Protocol.frame.PowerToggle; }

/// Set the desired temperature.
/// @param[in] degrees The temperature in degrees celsius.
void PanasonicAc32_setTemp(const uint8_t degrees) {
  uint8_t temp = MAX((uint8_t)kPanasonicAcMinTemp, degrees + kPanasonicAcMinTemp);
  temp = MIN((uint8_t)kPanasonicAcMaxTemp, temp);
  _PanasonicAc32Protocol.frame.Temp = temp - (kPanasonicAcMinTemp - 1);
}

/// Get the current desired temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t PanasonicAc32_getTemp(void) {
  return _PanasonicAc32Protocol.frame.Temp - 1;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t PanasonicAc32_getMode(void) { return _PanasonicAc32Protocol.frame.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note If we get an unexpected mode, default to AUTO.
void PanasonicAc32_setMode(const uint8_t mode) {
  switch (mode) {
    case kPanasonicAc32Auto:
    case kPanasonicAc32Cool:
    case kPanasonicAc32Dry:
    case kPanasonicAc32Heat:
    case kPanasonicAc32Fan:
      _PanasonicAc32Protocol.frame.Mode = mode;
      break;
    default: _PanasonicAc32Protocol.frame.Mode = kPanasonicAc32Auto;
  }
}

/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t PanasonicAc32_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kPanasonicAc32Cool;
    case kOpModeHeat: return kPanasonicAc32Heat;
    case kOpModeDry:  return kPanasonicAc32Dry;
    case kOpModeFan:  return kPanasonicAc32Fan;
    default:              return kPanasonicAc32Auto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t PanasonicAc32_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kPanasonicAc32Cool: return kOpModeCool;
    case kPanasonicAc32Heat: return kOpModeHeat;
    case kPanasonicAc32Dry:  return kOpModeDry;
    case kPanasonicAc32Fan:  return kOpModeFan;
    default:                 return kOpModeAuto;
  }
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void PanasonicAc32_setFan(const uint8_t speed) {
  switch (speed) {
    case kPanasonicAc32FanMin:
    case kPanasonicAc32FanLow:
    case kPanasonicAc32FanMed:
    case kPanasonicAc32FanHigh:
    case kPanasonicAc32FanMax:
    case kPanasonicAc32FanAuto:
      _PanasonicAc32Protocol.frame.Fan = speed;
      break;
    default: _PanasonicAc32Protocol.frame.Fan = kPanasonicAc32FanAuto;
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t PanasonicAc32_getFan(void) { return _PanasonicAc32Protocol.frame.Fan; }

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t PanasonicAc32_toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kPanasonicAc32FanMax:     return kFanSpeedMax;
    case kPanasonicAc32FanHigh:    return kFanSpeedHigh;
    case kPanasonicAc32FanMed:     return kFanSpeedMedium;
    case kPanasonicAc32FanLow:     return kFanSpeedLow;
    case kPanasonicAc32FanMin:     return kFanSpeedMin;
    default:                       return kFanSpeedAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t PanasonicAc32_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kPanasonicAc32FanMin;
    case kFanSpeedLow:    return kPanasonicAc32FanLow;
    case kFanSpeedMedium: return kPanasonicAc32FanMed;
    case kFanSpeedHigh:   return kPanasonicAc32FanHigh;
    case kFanSpeedMax:    return kPanasonicAc32FanMax;
    default:                         return kPanasonicAc32FanAuto;
  }
}

/// Get the current horizontal swing setting.
/// @return The current position it is set to.
bool PanasonicAc32_getSwingHorizontal(void) { return _PanasonicAc32Protocol.frame.SwingH; }

/// Control the horizontal swing setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void PanasonicAc32_setSwingHorizontal(const bool on) { _PanasonicAc32Protocol.frame.SwingH = on; }

/// Get the current vertical swing setting.
/// @return The current position it is set to.
uint8_t PanasonicAc32_getSwingVertical(void) { return _PanasonicAc32Protocol.frame.SwingV; }

/// Control the vertical swing setting.
/// @param[in] pos The position to set the vertical swing to.
void PanasonicAc32_setSwingVertical(const uint8_t pos) {
  uint8_t elevation = pos;
  if (elevation != kPanasonicAc32SwingVAuto) {
    elevation = MAX(elevation, kPanasonicAcSwingVHighest);
    elevation = MIN(elevation, kPanasonicAcSwingVLowest);
  }
  _PanasonicAc32Protocol.frame.SwingV = elevation;
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
swingv_t PanasonicAc32_toCommonSwingV(const uint8_t pos) {
  if (pos >= kPanasonicAcSwingVHighest && pos <= kPanasonicAcSwingVLowest)
    return (swingv_t)pos;
  else
    return kSwingVAuto;
}

/// Convert a standard A/C vertical swing into its native setting.
/// @param[in] position A swingv_t position to convert.
/// @return The equivalent native horizontal swing position.
uint8_t PanasonicAc32_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVHighest:
    case kSwingVHigh:
    case kSwingVMiddle:
    case kSwingVLow:
    case kSwingVLowest: return (uint8_t)position;
    default:            return kPanasonicAc32SwingVAuto;
  }
}




