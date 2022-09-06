#if(1)
#include "IR_Interface.h"
#include "IR_HAL.h"
#include "ampm_gt999.h"
#include "app_mesh.h"
#include "nrf_log.h"
#include "app_util_platform.h"

#include "ir_Casper.h"
#include "app_ac_status.h"


#include "IRutils.h"
#include "IRsend.h"
#include <stdio.h>
#include <string.h>

CasperProtocol_t _CasperProtocol = { .raw = {0xB2, 0x4D, 0x1F, 0xE0, 0xC0, 0x3F } };
uint8_t _Temp = 0xC;
uint8_t _Mode = kCasperAcCool;
uint8_t _Fan  = kCasperAcPowerAuto;

static int s_IrEnableTx;


/* Get values from BLE command then fill to IR protocol */
void encode_casper(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
  casper_setPower(ac_status.power_status);
  casper_setTemp(ac_status.temperature);
  casper_setMode(ac_status.mode);
  casper_setFan(ac_status.fan);

  

  casper_send(casper_getRaw(), kCasperAcStateLength, 1, OutputIRProtocol);
  SetIrTxState_Casper(1);
}


void decode_casper(int16_t* input, uint8_t* output) {

  initDecodeData(input, DATASET_MAX_INDEX_CASPER);

  if( casper_recv(&gDecodeResult, 0, kCasperAcBits, true) ){
    casper_setRaw(gDecodeResult.state);
  }

  /* ON/OFF */
  output[0] = casper_getPower();
  ac_control_set_power_status(output[0]);

  /* CONTROL TEMPERATURE */
  output[1] = casper_toCommonTemp( casper_getTemp() );
  ac_control_set_temperature(output[1]);

  /* WIND LEVEL */
  output[2] = casper_toCommonFanSpeed(casper_getFan());
  ac_control_set_fan(output[2]);

  /* AUTO/MANUAL SWING */

  /* SET AIRCONDITIONER MODE */
  output[4] = casper_toCommonMode( casper_getMode() );
  ac_control_set_mode(output[4]);

  ac_control_update_status_to_payload();
}

int IsEnableIrTx_Casper(void)
{
	return s_IrEnableTx;
}

void SetIrTxState_Casper(int state)
{
	s_IrEnableTx = state;
}





/// Send a SanyoAc formatted message.
/// Status: STABLE / Reported as working.
/// @param[in] data An array of bytes containing the IR command.
/// @param[in] nbytes Nr. of bytes of data in the array.
/// @param[in] repeat Nr. of times the message is to be repeated.
void casper_send( const uint8_t data[], const uint16_t nbytes, 
                  const uint16_t repeat, int16_t *irRaw) {
  // Header + Data + Footer
    sendGeneric_8( kCasperAcHdrMark, kCasperAcHdrSpace,
                    kCasperAcBitMark, kCasperAcOneSpace,
                    kCasperAcBitMark, kCasperAcZeroSpace,
                    kCasperAcBitMark, kCasperAcGap,
                    data, nbytes, kCasperAcFreq, true, repeat, 50,
                    irRaw);
}


/// Decode the supplied Casper A/C message.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool casper_recv(decode_results *results, uint16_t offset,
                const uint16_t nbits, const bool strict) {

  if (strict && nbits != kCasperAcBits) return false;

  // Match Header + Data + Footer
  uint16_t used = 0;
  for(int i=0; i<=1; i++){
    used = matchGeneric_8(results->rawbuf + offset, results->state + (i*kCasperAcStateLength),
                          results->rawlen - offset, nbits,
                          kCasperAcHdrMark, kCasperAcHdrSpace,
                          kCasperAcBitMark, kCasperAcOneSpace,
                          kCasperAcBitMark, kCasperAcZeroSpace,
                          kCasperAcBitMark, kCasperAcGap,
                          false,            kTolerance,
                          kMarkExcess,      true);
    if (used == 0) return false;
    offset += used;
  }

  // Success
  results->decode_type = UNKNOWN; //CASPER
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}


/// Get a PTR to the internal state/code for this protocol with all integrity
///   checks passing.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* casper_getRaw(void) {
  casper_checksum();
  return _CasperProtocol.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
void casper_setRaw(const uint8_t newState[]) {
  memcpy(_CasperProtocol.raw, newState, kCasperAcStateLength);
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void casper_setPower(const bool on){
  if(on){
    _CasperProtocol.Fan     = kCasperAcPowerOnFan;
    _CasperProtocol.Special = kCasperAcPowerOnSpecial;

    _CasperProtocol.Temp    = _Temp;
    _CasperProtocol.Mode    = _Mode;

    _Fan = _CasperProtocol.Fan;
  }
  else{
    _CasperProtocol.Fan     = kCasperAcPowerOffFan;
    _CasperProtocol.Special = kCasperAcPowerOffSpecial;

    _CasperProtocol.Temp    = kCasperAcPowerOffTemp;
    _CasperProtocol.Mode    = kCasperAcPowerOffMode;
  }
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool casper_getPower(void){
	if(_CasperProtocol.Special == kCasperAcPowerOffFan)
    return false;
  else
    return true;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t casper_getMode(void) {
  return _CasperProtocol.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note If we get an unexpected mode, default to AUTO.
void casper_setMode(const uint8_t mode) {

  _CasperProtocol.Mode = casper_convertMode(mode);
  switch (_CasperProtocol.Mode) {
    case kCasperAcAuto:
    case kCasperAcDry:
      _CasperProtocol.Fan  = kCasperAcPowerOnFan;
      break;
    case kCasperAcCool:
    case kCasperAcHeat:
      _CasperProtocol.Fan  = _Fan;
      break;
    default: // case kCasperAcFan
      _CasperProtocol.Fan  = _Fan;
      _CasperProtocol.Temp = kCasperAcPowerOffTemp;
      break;
  }
  if( _Mode == kCasperAcFan)
    _CasperProtocol.Temp = _Temp;

  _Mode = _CasperProtocol.Mode;
}

/// Set the desired temperature.
/// @param[in] degrees The temperature in degrees celsius.
void casper_setTemp(const uint8_t degrees) {
  if(_Mode == kCasperAcFan){
    _CasperProtocol.Temp = 0xE;
  }
  else{
    _CasperProtocol.Temp = casper_convertTemp(degrees);
    _Temp = _CasperProtocol.Temp;
  }
}

/// Get the current desired temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t casper_getTemp(void) {
  return casper_toCommonTemp(_CasperProtocol.Temp);
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void casper_setFan(const uint8_t speed) {
  switch (_Mode) {
    case kCasperAcAuto:
    case kCasperAcDry:
      if(_CasperProtocol.Temp != kCasperAcPowerOffTemp)
        _CasperProtocol.Fan  = kCasperAcPowerOnFan;
      else
        _CasperProtocol.Fan  = casper_convertFan(speed); // case kCasperAcFan
      break;
    case kCasperAcCool:
    case kCasperAcHeat:
    default: 
      _CasperProtocol.Fan  = casper_convertFan(speed);
      break;
  }
  _Fan = _CasperProtocol.Fan;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t casper_getFan(void) {
  return casper_toCommonFanSpeed(_CasperProtocol.Fan);
}

/// Set the vertical swing setting of the A/C.
/// @param[in] setting The value of the desired setting.
void casper_setSwingV(void) {
  _CasperProtocol.Fan     = kCasperAcSwingVAuto;
  _CasperProtocol.Special = kCasperAcPowerOffSpecial;
  _CasperProtocol.Temp    = kCasperAcPowerOffTemp;
  _CasperProtocol.Mode    = kCasperAcPowerOffMode;
}

void casper_checksum(void){
  _CasperProtocol.raw[1] = _CasperProtocol.raw[0] ^ 0xFF;
  _CasperProtocol.raw[3] = _CasperProtocol.raw[2] ^ 0xFF;
  _CasperProtocol.raw[5] = _CasperProtocol.raw[4] ^ 0xFF;
}




/// Convert a opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t casper_convertMode(const opmode_t mode) {
  switch (mode) {
    case kOpModeCool: return kCasperAcCool;
    case kOpModeHeat: return kCasperAcHeat;
    case kOpModeDry:  return kCasperAcDry;
    case kOpModeFan:  return kCasperAcFan;
    default:          return kCasperAcAuto;
  }
}

/// Convert a fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t casper_convertFan(const fanspeed_t speed) {
  switch (speed) {
    case kFanSpeedMin:    return kCasperAcFanLow;
    case kFanSpeedLow:    return kCasperAcFanMedium;
    case kFanSpeedMedium: return kCasperAcFanHigh;
    case kFanSpeedHigh:
    case kFanSpeedMax:    
    default:              return kCasperAcFanHigh;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
opmode_t casper_toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kCasperAcCool: return kOpModeCool;
    case kCasperAcHeat: return kOpModeHeat;
    case kCasperAcDry:
      if(_CasperProtocol.Temp == 0xE)
        return kOpModeFan;
      else
        return kOpModeDry;
    default:            return kOpModeAuto;
  }
}

/// Convert a swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t casper_convertSwingV(const swingv_t position) {
  switch (position) {
    case kSwingVOff:     return kCasperAcSwingVAuto;
    case kSwingVHighest: return kCasperAcSwingVAuto;
    case kSwingVHigh:    return kCasperAcSwingVAuto;
    case kSwingVMiddle:  return kCasperAcSwingVAuto;
    case kSwingVLow:     return kCasperAcSwingVAuto;
    case kSwingVLowest:  return kCasperAcSwingVAuto;
    default:             return kCasperAcSwingVAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
fanspeed_t casper_toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kCasperAcFanHigh:   return kFanSpeedMax;
    case kCasperAcFanMedium: return kFanSpeedMedium;
    case kCasperAcFanLow:    return kFanSpeedMin;
    default:                 return kFanSpeedMin;
  }
}

uint8_t casper_toCommonTemp(const uint8_t tmp){
  switch (tmp) {
    case 0x0: return 0;  //17;
    case 0x1: return 1;  //18;
    case 0x3: return 2;  //19;
    case 0x2: return 3;  //20;
    case 0x6: return 4;  //21;
    case 0x7: return 5;  //22;
    case 0x5: return 6;  //23;
    case 0x4: return 7;  //24;
    case 0xC: return 8;  //25;
    case 0xD: return 9;  //26;
    case 0x9: return 10; //27;
    case 0x8: return 11; //28;
    case 0xA: return 12; //29;
    case 0xB: return 13; //30;
    default:   return 0;
  }
}


uint8_t casper_convertTemp(const uint8_t tmp){
  switch (tmp) {
    case 0:  return 0x0;//17;
    case 1:  return 0x1;//18;
    case 2:  return 0x3;//19;
    case 3:  return 0x2;//20;
    case 4:  return 0x6;//21;
    case 5:  return 0x7;//22;
    case 6:  return 0x5;//23;
    case 7:  return 0x4;//24;
    case 8:  return 0xC;//25;
    case 9:  return 0xD;//26;
    case 10: return 0x9;//27;
    case 11: return 0x8;//28;
    case 12: return 0xA;//29;
    case 13: return 0xB;//30;
    default:   return 0;
  }
}



#endif


