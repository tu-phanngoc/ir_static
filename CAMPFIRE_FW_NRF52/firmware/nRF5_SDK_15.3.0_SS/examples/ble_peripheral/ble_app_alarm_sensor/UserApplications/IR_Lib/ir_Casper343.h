// Copyright 2018 David Conran

/// @file
/// @brief Support for Casper343 protocols.

#ifndef IR_Casper343_H_
#define IR_Casper343_H_

#include <stdint.h>

#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"

/// Native representation of a Casper343 A/C message.
typedef union Casper343Protocol_t{
  uint8_t raw[kCasper343StateLength];  ///< The state in IR code form
  struct {
    // Byte 0~1
    uint8_t pad0[2];
    // Byte 2
    uint8_t Fan     :2;
    uint8_t Power   :1;
    uint8_t Sleep   :1;
    uint8_t pad1:3;
    uint8_t Swing1  :1;
    // Byte 3
    uint8_t Mode  :3;
    uint8_t pad2:1;
    uint8_t Temp  :4;
    // Byte 4
    uint8_t pad3:8;
    // Byte 5
    uint8_t pad4:4;
    uint8_t Super1  :1;
    uint8_t pad5:2;
    uint8_t Super2  :1;
    // Byte 6
    uint8_t ClockHours  :5;
    uint8_t LightOff    :1;
    uint8_t pad6:2;
    // Byte 7
    uint8_t ClockMins       :6;
    uint8_t pad7:1;
    uint8_t OffTimerEnabled :1;
    // Byte 8
    uint8_t OffHours  :5;
    uint8_t rev0800   :1;
    uint8_t Swing2    :1;
    uint8_t SwingH    :1;
    // Byte 9
    uint8_t OffMins         :6;
    uint8_t pad10:1;
    uint8_t OnTimerEnabled  :1;
    // Byte 10
    uint8_t OnHours :5;
    uint8_t pad11:3;
    // Byte 11
    uint8_t OnMins  :6;
    uint8_t pad12:2;
    // Byte 12
    uint8_t pad13:8;
    // Byte 13
    uint8_t Sum1  :8;
    // Byte 14
    uint8_t rev1400:4;
    uint8_t SleepLvl:4;
    // Byte 15
    uint8_t Cmd   :8;
    // Byte 16
    uint8_t rev160:8;
    // Byte 17
    uint8_t rev1700:6;
    uint8_t FanAngel:1;
    uint8_t rev1701:1;
    // Byte 18
    uint8_t rev1800 :4;
    uint8_t Power1  :1;
    uint8_t rev1801 :3;
    // Byte 19
    uint8_t pad17:8;
    // Byte 20
    uint8_t Sum2  :8;
  };
}Casper343Protocol_t;

// Constants
#define kCasper343ChecksumByte1         13
#define kCasper343ChecksumByte2         kCasper343StateLength - 1

#define kCasper343Heat                  0
#define kCasper343Auto                  1
#define kCasper343Cool                  2
#define kCasper343Dry                   3
#define kCasper343Fan                   4

#define kCasper343FanAuto               0
#define kCasper343FanHigh               1
#define kCasper343FanMedium             2
#define kCasper343FanLow                3

#define kCasper343MinTemp               0       // 16C
#define kCasper343MaxTemp               0xE     // 30C
#define kCasper343AutoTemp              7       // 23C

#define kCasper343SleepLv0              0
#define kCasper343SleepLv1              4
#define kCasper343SleepLv2              8
#define kCasper343SleepLv3              0xC

#define kCasper343CommandLight          0x00
#define kCasper343CommandPower          0x01
#define kCasper343CommandTemp           0x02
#define kCasper343CommandSleep          0x03
#define kCasper343CommandSuper          0x04
#define kCasper343CommandOnTimer        0x05
#define kCasper343CommandMode           0x06
#define kCasper343CommandSwingV         0x07
#define kCasper343CommandSwingH         0x08
#define kCasper343CommandIFeel          0x0D
#define kCasper343CommandFanSpeed       0x11
#define kCasper343Command6thSense       0x17
#define kCasper343CommandOffTimer       0x1D



void encode_Casper343(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Casper343(int16_t* input, uint8_t* output);
bool isCapser343(int16_t *irRaw);


bool Casper343_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);
void Casper343_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);


/// Class for handling detailed Casper343 A/C messages.
void     Casper343_setPower(const bool on);
bool     Casper343_getPower(void);
void     Casper343_setSleep(const uint8_t _sleep);
bool     Casper343_getSleep(void);
void     Casper343_setSuper(const bool on);
bool     Casper343_getSuper(void);
void     Casper343_setTemp(const uint8_t temp, const bool remember);
uint8_t  Casper343_getTemp(void);
void     Casper343_setFan(const uint8_t speed);
uint8_t  Casper343_getFan(void);
void     Casper343_setMode(const uint8_t mode);
uint8_t  Casper343_getMode(void);
void     Casper343_setSwingV(const bool on);
bool     Casper343_getSwingV(void);
void     Casper343_setSwingH(const bool on);
bool     Casper343_getSwingH(void);
void     Casper343_setLight(const bool on);
void     Casper343_setCommand(void);
uint8_t  Casper343_getCommand(void);
uint8_t* Casper343_getRaw(const bool calcchecksum);
void     Casper343_setRaw(const uint8_t new_code[], const uint16_t length );
bool     Casper343_validChecksum(const uint8_t state[], const uint16_t length );
void     Casper343_checksum(const uint16_t length );
void     Casper343_ClearData(void);


uint8_t    Casper343_convertMode(const opmode_t mode);
opmode_t   Casper343_toCommonMode(const uint8_t mode);



#endif  // IR_Casper343_H_


