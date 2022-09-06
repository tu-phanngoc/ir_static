
#ifndef IR_COOLIX_H_
#define IR_COOLIX_H_

#include "stdint.h"
#include "stdio.h"
#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"



// Constants
#define kCoolixHdrMark            4500
#define kCoolixHdrSpace           4500
#define kCoolixBitMark            550
#define kCoolixZeroSpace          550
#define kCoolixOneSpace           1650
#define kCoolixMinGap             5644
#define kCoolix48ExtraTolerance   5  // Percent

// Modes
#define kCoolixCool    0
#define kCoolixDry     1
#define kCoolixAuto    2
#define kCoolixHeat    3
#define kCoolixFan     4
// Fan Control
#define kCoolixFanAuto0       0
#define kCoolixFanMax         1
#define kCoolixFanMed         2
#define kCoolixFanMin         4
#define kCoolixFanAuto        5
#define kCoolixFanZoneFollow  6
#define kCoolixFanFixed       7

// Temperature
#define kCoolixTempMin        17  // Celsius
#define kCoolixTempMax        30  // Celsius
#define kCoolixTempRange      kCoolixTempMax - kCoolixTempMin + 1
#define kCoolixFanTempCode    0xE  // Part of Fan Mode.

#define kCoolixSensorTempMax   30  // Celsius
#define kCoolixSensorTempIgnoreCode   0x1F  // 0x1F / 31 (DEC)
// Fixed states/messages.
#define kCoolixOff      0xB27BE0
#define kCoolixSwing    0xB26BE0
#define kCoolixSwingH   0xB5F5A2
#define kCoolixSwingV   0xB20FE0
#define kCoolixSleep    0xB2E003
#define kCoolixTurbo    0xB5F5A2
#define kCoolixLed      0xB5F5A5
#define kCoolixClean    0xB5F5AA
#define kCoolixCmdFan   0xB2BFE4
// On, 25C, Mode: Auto, Fan: Auto, Zone Follow: Off, Sensor Temp: Ignore.
#define kCoolixDefaultState   0xB21FC8

/// Native representation of a Coolix A/C message.
union CoolixProtocol_t {
  uint32_t raw;  ///< The state in IR code form.
  struct {  // Only 24 bits are used.
    // Byte
    uint32_t rev00:1;  // Unknown
    uint32_t ZoneFollow1:1;  ///< Control bit for Zone Follow mode.
    uint32_t Mode       :2;  ///< Operation mode.
    uint32_t Temp       :4;  ///< Desired temperature (Celsius)
    // Byte
    uint32_t SensorTemp :5;  ///< The temperature sensor in the IR remote.
    uint32_t Fan        :3;  ///< Fan speed
    // Byte
    uint32_t rev01:3;  // Unknown
    uint32_t ZoneFollow2:1;  ///< Additional control bit for Zone Follow mode.
    uint32_t rev02:4;  ///< Fixed value 0b1011 / 0xB.
  };
};


void encode_Coolix(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Coolix(int16_t* input, uint8_t* output);

void Coolix_send(uint64_t data, uint16_t nbits, uint16_t repeat, int16_t *irRaw);
bool Coolix_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);

bool isHeader_Coolix(int16_t *irRaw);


/// Class for handling detailed Coolix A/C messages.
void    Coolix_setPower(const bool on);
bool    Coolix_getPower(void);
void    Coolix_setTemp(const uint8_t temp);
uint8_t Coolix_getTemp(void);
void    Coolix_setSensorTemp(const uint8_t temp);
uint8_t Coolix_getSensorTemp(void);
void    Coolix_clearSensorTemp(void);
void    Coolix_setFan(const uint8_t speed, const bool modecheck);
uint8_t Coolix_getFan(void);
void    Coolix_setMode(const uint8_t mode);
uint8_t Coolix_getMode(void);
void Coolix_setSwing(void);
bool Coolix_getSwing(void);
void Coolix_setSwingVStep(void);
bool Coolix_getSwingVStep(void);
void Coolix_setSleep(void);
bool Coolix_getSleep(void);
void Coolix_setTurbo(void);
bool Coolix_getTurbo(void);
void Coolix_setLed(void);
bool Coolix_getLed(void);
void Coolix_setClean(void);
bool Coolix_getClean(void);
bool Coolix_getZoneFollow(void);
uint32_t Coolix_getRaw(void);
void     Coolix_setRaw(const uint32_t new_code);

uint8_t Coolix_convertMode(const opmode_t mode);
uint8_t Coolix_convertFan(const fanspeed_t speed);
opmode_t Coolix_toCommonMode(const uint8_t mode);
fanspeed_t Coolix_toCommonFanSpeed(const uint8_t speed);


void Coolix_setTempRaw(const uint8_t code);
uint8_t Coolix_getTempRaw(void);

void Coolix_setSensorTempRaw(const uint8_t code);
void Coolix_setZoneFollow(const bool on);
bool Coolix_isSpecialState(void);
bool Coolix_handleSpecialState(const uint32_t data);
void Coolix_updateAndSaveState(const uint32_t raw_state);
void Coolix_recoverSavedState(void);
uint32_t Coolix_getNormalState(void);


#endif  // IR_COOLIX_H_
