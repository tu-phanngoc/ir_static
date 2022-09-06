/// @file


#ifndef IR_VESTEL_H_
#define IR_VESTEL_H_

#include <stdint.h>
#include <stdio.h>
#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"

/// Native representation of a Vestel A/C message.
union VestelProtocol{
  struct {
    uint64_t cmdState;
    uint64_t timeState;
  };
  struct {
    // Command
    uint64_t Signature  :12;  // 0x201
    uint64_t CmdSum     :8;
    uint64_t Swing      :4;  // auto 0xA, stop 0xF
    uint64_t TurboSleep :4;  // normal 0x1, sleep 0x3, turbo 0x7
    uint64_t rev_00     :8;  // 00
    uint64_t Temp       :4;
    uint64_t Fan        :4;
    uint64_t Mode       :3;
    uint64_t rev_01     :3;
    uint64_t Ion        :1;
    uint64_t rev_02     :1;
    uint64_t Power      :2;
    uint64_t UseCmd     :1;
    uint64_t rev_03     :1;
    // Time
    uint64_t rev_04     :12;
    uint64_t TimeSum    :8;
    uint64_t OffTenMins :3;
    uint64_t OffHours   :5;
    uint64_t OnTenMins  :3;
    uint64_t OnHours    :5;
    uint64_t Hours      :5;
    uint64_t OnTimer    :1;
    uint64_t OffTimer   :1;
    uint64_t Timer      :1;
    uint64_t Minutes    :8;
    uint64_t rev_05     :4;
  };
};

// Constants
#define kVestelAcMinTemp      0  // 16c
#define kVestelAcMaxTemp      16 // 32c

#define kVestelAcAuto         0
#define kVestelAcCool         1
#define kVestelAcDry          2
#define kVestelAcFan          3
#define kVestelAcHeat         4

#define kVestelAcFanAuto      1
#define kVestelAcFanLow       5
#define kVestelAcFanMed       9
#define kVestelAcFanHigh      0xB
#define kVestelAcFanAutoCool  0xC
#define kVestelAcFanAutoHot   0xD

#define kVestelAcNormal       1
#define kVestelAcSleep        3
#define kVestelAcTurbo        7
#define kVestelAcIon          4
#define kVestelAcSwing        0xA

// Default states
#define kVestelAcStateDefault           0xC00D8001FF2201ULL
#define kVestelAcTimeStateDefaultOff    0x036140000F7201ULL
#define kVestelAcTimeStateDefaultOn     0xF00000000F9202ULL



void encode_VestelAc(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_VestelAc(int16_t* input, uint8_t* output);
bool isVestelAc(int16_t *irRaw);

void VestelAc_send(const uint64_t data, const uint16_t nbits, const uint16_t repeat, int16_t *irRaw);
bool VestelAc_recv(decode_results* results, uint16_t offset, const uint16_t nbits, const bool strict);

/// Class for handling detailed Vestel A/C messages.
void VestelAc_setPower(const bool on);
bool VestelAc_getPower(void);
void VestelAc_setAuto(const int8_t autoLevel);
void VestelAc_setTimer(const uint16_t minutes);
uint16_t VestelAc_getTimer(void);
void     VestelAc_setTime(const uint16_t minutes);
uint16_t VestelAc_getTime(void);
void     VestelAc_setOnTimer(const uint16_t minutes);
uint16_t VestelAc_getOnTimer(void);
void     VestelAc_setOffTimer(const uint16_t minutes);
uint16_t VestelAc_getOffTimer(void);
void     VestelAc_setTemp(const uint8_t temp);
uint8_t  VestelAc_getTemp(void);
void     VestelAc_setFan(const uint8_t fan);
uint8_t  VestelAc_getFan(void);
void     VestelAc_setMode(const uint8_t mode);
uint8_t  VestelAc_getMode(void);
void     VestelAc_setRaw_p8(const uint8_t* newState);
void     VestelAc_setRaw(const uint64_t newState);
uint64_t VestelAc_getRaw(void);
bool VestelAc_validChecksum(const uint64_t state);
void VestelAc_setSwing(const bool on);
bool VestelAc_getSwing(void);
void VestelAc_setSleep(const bool on);
bool VestelAc_getSleep(void);
void VestelAc_setTurbo(const bool on);
bool VestelAc_getTurbo(void);
void VestelAc_setIon(const bool on);
bool VestelAc_getIon(void);
bool VestelAc_isTimeCommand(void);
bool VestelAc_isOnTimerActive(void);
void VestelAc_setOnTimerActive(const bool on);
bool VestelAc_isOffTimerActive(void);
void VestelAc_setOffTimerActive(const bool on);
bool VestelAc_isTimerActive(void);
void VestelAc_setTimerActive(const bool on);

void    VestelAc_checksum(void);
uint8_t VestelAc_calcChecksum(const uint64_t state);
uint8_t VestelAc_convertMode(const opmode_t mode);
uint8_t VestelAc_convertFan(const fanspeed_t speed);
opmode_t VestelAc_toCommonMode(const uint8_t mode);
fanspeed_t VestelAc_toCommonFanSpeed(const uint8_t speed);
state_t VestelAc_toCommon(void);

static uint64_t swapData(uint64_t input);

#endif  // IR_VESTEL_H_
