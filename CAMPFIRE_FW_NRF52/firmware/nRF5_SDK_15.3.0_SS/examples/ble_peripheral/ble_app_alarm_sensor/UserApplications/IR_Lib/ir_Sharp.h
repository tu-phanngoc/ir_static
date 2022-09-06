// Copyright 2019 crankyoldgit

/// @file
/// @brief Support for Sharp protocols.

// Supports:
//   Brand: Sharp,  Model: LC-52D62U TV
//   Brand: Sharp,  Model: AY-ZP40KR A/C (A907)
//   Brand: Sharp,  Model: AH-AxSAY A/C (A907)
//   Brand: Sharp,  Model: CRMC-A907 JBEZ remote (A907)
//   Brand: Sharp,  Model: CRMC-A950 JBEZ (A907)
//   Brand: Sharp,  Model: AH-PR13-GL A/C (A903)
//   Brand: Sharp,  Model: CRMC-A903JBEZ remote (A903)
//   Brand: Sharp,  Model: AH-XP10NRY A/C (A903)
//   Brand: Sharp,  Model: CRMC-820 JBEZ remote (A903)
//   Brand: Sharp,  Model: CRMC-A705 JBEZ remote (A705)
//   Brand: Sharp,  Model: AH-A12REVP-1 A/C (A903)
//   Brand: Sharp,  Model: CRMC-A863 JBEZ remote (A903)

#ifndef IR_SHARP_H_
#define IR_SHARP_H_

#include "stdint.h"
#include "string.h"

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"
#include "IRcommon.h"

/// Native representation of a Sharp A/C message.
typedef union SharpProtocol{
  uint8_t raw[kSharpAcStateLength];  ///< State of the remote in IR code form
  struct {
    // Byte 0~3
    uint8_t pad[4];
    // Byte 4
    uint8_t Temp  :4;
    uint8_t Model :1;
    uint8_t rev0400:3;
    // Byte 5
    uint8_t rev1 :4;
    uint8_t PowerSpecial  :4;
    // Byte 6
    uint8_t Mode  :2;
    uint8_t rev2  :1;
    uint8_t Clean :1;
    uint8_t Fan   :3;
    uint8_t rev3  :1;
    // Byte 7
    uint8_t TimerHours  :4;
    uint8_t rev4        :2;
    uint8_t TimerType   :1;
    uint8_t TimerEnabled:1;
    // Byte 8
    uint8_t Swing :3;
    uint8_t rev5  :5;
    // Byte 9
    uint8_t rev6  :8;
    // Byte 10
    uint8_t Special :8;
    // Byte 11
    uint8_t rev7  :2;
    uint8_t Ion    :1;
    uint8_t rev8  :1;
    uint8_t Model2 :1;
    uint8_t rev9  :3;
    // Byte 12
    uint8_t  rev10  :4;
    uint8_t Sum    :4;
  };
} SharpProtocol;

// Constants
#define kSharpAcHdrMark                     3800
#define kSharpAcHdrSpace                    1900
#define kSharpAcBitMark                     470
#define kSharpAcZeroSpace                   500
#define kSharpAcOneSpace                    1400
#define kSharpAcGap                         kDefaultMessageGap

#define kSharpAcByteTemp                    4
#define kSharpAcTempDelta                   1  // Celsius
#define kSharpAcMinTemp                     1  // 15 Celsius
#define kSharpAcMaxTemp                     16  // 30 Celsius

#define kSharpAcPowerUnknown                0 
#define kSharpAcPowerOnFromOff              1 
#define kSharpAcPowerOff                    2 
#define kSharpAcPowerOn                     3 // Normal 
#define kSharpAcPowerSetSpecialOn           6 
#define kSharpAcPowerSetSpecialOff          7 
#define kSharpAcPowerTimerSetting           8 

#define kSharpAcAuto                        0  // A907 only
#define kSharpAcFan                         0  // A705 only
#define kSharpAcDry                         3
#define kSharpAcCool                        2
#define kSharpAcHeat                        1  // A907 only
#define kSharpAcFanAuto                     2  // 2
#define kSharpAcFanMin                      4  // 4 (FAN1)
#define kSharpAcFanMed                      3  // 3 (FAN2)
#define kSharpAcFanA705Low                  3  // 3 (A903 too)
#define kSharpAcFanHigh                     5  // 5 (FAN3)
#define kSharpAcFanA705Med                  5  // 5 (A903 too)
#define kSharpAcFanMax                      7  // 7 (FAN4)

#define kSharpAcTimerIncrement              30  // Mins
#define kSharpAcTimerHoursOff               0
#define kSharpAcTimerHoursMax               12  // 12
#define kSharpAcOffTimerType                0
#define kSharpAcOnTimerType                 1

#define kSharpAcSwingVIgnore                0  // Don't change the swing setting.
#define kSharpAcSwingVHigh                  1  // 0° down. Similar to Cool Coanda.
#define kSharpAcSwingVOff                   2  // Stop & Go to last fixed pos.
#define kSharpAcSwingVMid                   3  // 30° down
#define kSharpAcSwingVLow                   4  // 45° down
#define kSharpAcSwingVLast                  5  // Same as kSharpAcSwingVOff.
// Toggles between last fixed pos & either 75° down (Heat) or 0° down (Cool)
// i.e. alternate between last pos <-> 75° down if in Heat mode, AND
//      alternate between last pos <-> 0° down if in Cool mode.
// Note: `setSwingV(kSharpAcSwingVLowest)` will only allow the Lowest setting in
//       Heat mode, it will default to `kSharpAcSwingVLow` otherwise.
//       If you want to set this value in other modes e.g. Cool, you must
//       use `setSwingV`s optional `force` parameter.
#define kSharpAcSwingVLowest                6
#define kSharpAcSwingVCoanda                kSharpAcSwingVLowest
#define kSharpAcSwingVToggle                7  // Toggle Constant swinging on/off.

#define kSharpAcSpecialPower                0x00
#define kSharpAcSpecialTurbo                0x01
#define kSharpAcSpecialTempEcono            0x04
#define kSharpAcSpecialFan                  0x05
#define kSharpAcSpecialSwing                0x06
#define kSharpAcSpecialTimer                0xC0
#define kSharpAcSpecialTimerHalfHour        0xDE


void encode_Sharp(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Sharp(int16_t* input, uint8_t* output);

void Sharp_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool Sharp_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);

// Classes
/// Class for handling detailed Sharp A/C messages.

  sharp_ac_remote_model_t Sharp_getModel(const bool raw);
  void     Sharp_setModel(const sharp_ac_remote_model_t model);
  void     Sharp_setPower(const bool on, const bool prev_on);
  bool     Sharp_getPower(void);
  bool     Sharp_isPowerSpecial(void);
  void     Sharp_setTemp(const uint8_t temp, const bool save);
  uint8_t  Sharp_getTemp(void);
  void     Sharp_setFan(const uint8_t fan, const bool save);
  uint8_t  Sharp_getFan(void);
  void     Sharp_setMode(const uint8_t mode, const bool save);
  uint8_t  Sharp_getMode(void);
  void     Sharp_setSpecial(const uint8_t mode);
  uint8_t  Sharp_getSpecial(void);
  bool     Sharp_getTurbo(void);
  void     Sharp_setTurbo(const bool on);
  bool     Sharp_getSwingToggle(void);
  void     Sharp_setSwingToggle(const bool on);
  uint8_t  Sharp_getSwingV(void);
  void     Sharp_setSwingV(const uint8_t position, const bool force);
  bool     Sharp_getIon(void);
  void     Sharp_setIon(const bool on);
  bool     Sharp_getEconoToggle(void);
  void     Sharp_setEconoToggle(const bool on);
  bool     Sharp_getLightToggle(void);
  void     Sharp_setLightToggle(const bool on);
  uint16_t Sharp_getTimerTime(void);
  bool     Sharp_getTimerEnabled(void);
  bool     Sharp_getTimerType(void);
  void     Sharp_setTimer(bool enable, bool timer_type, uint16_t mins);
  bool     Sharp_getClean(void);
  void     Sharp_setClean(const bool on);
  uint8_t* Sharp_getRaw(void);
  void     Sharp_setRaw(const uint8_t new_code[], const uint16_t length);
  void     Sharp_setPowerSpecial(const uint8_t value);
  uint8_t  Sharp_getPowerSpecial(void);
  void     Sharp_clearPowerSpecial(void);


  static bool getEconoToggle(void);
  static void setEconoToggle(const bool on);
  static void Sharp_checksum(void);
	static bool Sharp_validChecksum(uint8_t state[], const uint16_t length);
	static uint8_t Sharp_calcChecksum(uint8_t state[], const uint16_t length);
	

  static uint8_t Sharp_convertMode(const opmode_t mode);
  static uint8_t Sharp_convertFan(const fanspeed_t speed, const sharp_ac_remote_model_t model);
  static uint8_t Sharp_convertSwingV(const swingv_t position);

  opmode_t   Sharp_toCommonMode(const uint8_t mode);
  fanspeed_t Sharp_toCommonFanSpeed(const uint8_t speed);
  swingv_t   Sharp_toCommonSwingV( const uint8_t pos, const opmode_t mode);



#endif  // IR_SHARP_H_
