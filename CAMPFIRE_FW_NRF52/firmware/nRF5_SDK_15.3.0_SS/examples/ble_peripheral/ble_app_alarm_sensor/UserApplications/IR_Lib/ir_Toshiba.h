
/// @file
/// @brief Support for Toshiba protocols.

// Supports:
//   Brand: Toshiba,  Model: RAS-B13N3KV2
//   Brand: Toshiba,  Model: Akita EVO II
//   Brand: Toshiba,  Model: RAS-B13N3KVP-E
//   Brand: Toshiba,  Model: RAS 18SKP-ES
//   Brand: Toshiba,  Model: WH-TA04NE
//   Brand: Toshiba,  Model: WC-L03SE
//   Brand: Toshiba,  Model: WH-UB03NJ remote
//   Brand: Toshiba,  Model: RAS-2558V A/C
//   Brand: Toshiba,  Model: WH-TA01JE remote
//   Brand: Toshiba,  Model: RAS-25SKVP2-ND A/C
//   Brand: Carrier,  Model: 42NQV060M2 / 38NYV060M2 A/C
//   Brand: Carrier,  Model: 42NQV050M2 / 38NYV050M2 A/C
//   Brand: Carrier,  Model: 42NQV035M2 / 38NYV035M2 A/C
//   Brand: Carrier,  Model: 42NQV025M2 / 38NYV025M2 A/C

#ifndef IR_TOSHIBA_H_
#define IR_TOSHIBA_H_

#include <stdint.h>
#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"

/// Native representation of a Toshiba A/C message.
union ToshibaProtocol{
  uint8_t raw[kToshibaAcStateLengthLong];  ///< The state in code form.
  struct {
    // Byte[0] - 0xF2
    uint8_t Address:8;
    // Byte[1] - 0x0D (inverted previous byte's value)
    uint8_t rev0100:8;
    // Byte[2] - The expected payload length (in bytes) past the Byte[4].
    ///< Known lengths are:
    ///<   1 (56 bit message)
    ///<   3 (72 bit message)
    ///<   4 (80 bit message)
    uint8_t Length   :8;
    // Byte[3] - The bit-inverted value of the "length" byte.
    uint8_t rev0300  :8;
    // Byte[4]
    uint8_t rev0400  :3;
    uint8_t LongMsg  :1;
    uint8_t rev0401  :1;
    uint8_t ShortMsg :1;
    uint8_t rev0402  :2;
    // Byte[5]
    uint8_t Swing    :3;
    uint8_t rev0500  :1;
    uint8_t Temp     :4;
    // Byte[6]
    uint8_t Mode     :3;
    uint8_t rev0600  :2;
    uint8_t Fan      :3;
    // Byte[7]
    uint8_t rev0700  :4;
    uint8_t Filter   :1;
    uint8_t rev0701  :3;

    // Byte[8]
    // (Checksum for 72 bit messages, Eco/Turbo for long 80 bit messages)
    uint8_t EcoTurbo :8;
  };
};

// Constants

#define kToshibaAcLengthByte        2  ///< Byte pos of the "length" attribute
#define kToshibaAcMinLength         6  ///< Min Nr. of bytes in a message.

#define kToshibaAcInvertedLength    4  ///< Nr. of leading bytes in
                                       ///< inverted pairs.

#define kToshibaAcSwingStep         0 ///< 0b000
#define kToshibaAcSwingOn           1 ///< 0b001
#define kToshibaAcSwingOff          2 ///< 0b010
#define kToshibaAcSwingToggle       4 ///< 0b100

#define kToshibaAcMinTemp   17  ///< 17C
#define kToshibaAcMaxTemp   30  ///< 30C

#define kToshibaAcAuto      0 // 0b000
#define kToshibaAcCool      1 // 0b001
#define kToshibaAcDry       2 // 0b010
#define kToshibaAcHeat      3 // 0b011
#define kToshibaAcFan       4 // 0b100
#define kToshibaAcOff       7 // 0b111

#define kToshibaAcFanAuto   0
#define kToshibaAcFanMin    2
#define kToshibaAcFanLow    3
#define kToshibaAcFanMed    4
#define kToshibaAcFanHigh   5
#define kToshibaAcFanMax    6

#define kToshibaAcTurboOn   1 // 0b01
#define kToshibaAcEconoOn   3 // 0b11

// Legacy defines. (Deprecated)
#define TOSHIBA_AC_AUTO       kToshibaAcAuto
#define TOSHIBA_AC_COOL       kToshibaAcCool
#define TOSHIBA_AC_DRY        kToshibaAcDry
#define TOSHIBA_AC_HEAT       kToshibaAcHeat
#define TOSHIBA_AC_POWER      kToshibaAcPower
#define TOSHIBA_AC_FAN_AUTO   kToshibaAcFanAuto
#define TOSHIBA_AC_FAN_MAX    kToshibaAcFanMax
#define TOSHIBA_AC_MIN_TEMP   kToshibaAcMinTemp
#define TOSHIBA_AC_MAX_TEMP   kToshibaAcMaxTemp




void encode_ToshibaAc(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_ToshibaAc(int16_t* input, uint8_t* output);

bool isHeader_Toshiba(int16_t *irRaw);
void ToshibaAc_send(const uint8_t data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool ToshibaAc_recv(decode_results* results, uint16_t offset,const uint16_t nbits, const bool strict);


// Classes
/// Class for handling detailed Toshiba A/C messages.
void ToshibaAc_setPower(const bool on);
bool ToshibaAc_getPower(void);
void ToshibaAc_setTemp(const uint8_t degrees);
uint8_t ToshibaAc_getTemp(void);
void    ToshibaAc_setFan(const uint8_t speed);
uint8_t ToshibaAc_getFan(void);
void ToshibaAc_setTurbo(const bool on);
bool ToshibaAc_getTurbo(void);
void ToshibaAc_setEcono(const bool on);
bool ToshibaAc_getEcono(void);
void ToshibaAc_setFilter(const bool on);
bool ToshibaAc_getFilter(void);
void ToshibaAc_setMode(const uint8_t mode);
uint8_t  ToshibaAc_getMode(const bool raw);
void     ToshibaAc_setRaw(const uint8_t newState[], const uint16_t length);
uint8_t* ToshibaAc_getRaw(void);
uint16_t ToshibaAc_getInternalStateLength(const uint8_t state[], const uint16_t size);
uint16_t ToshibaAc_getStateLength(void);
uint8_t ToshibaAc_getSwing(const bool raw);
void    ToshibaAc_setSwing(const uint8_t setting);
uint8_t ToshibaAc_convertMode(const opmode_t mode);
uint8_t ToshibaAc_convertFan(const fanspeed_t speed);
opmode_t   ToshibaAc_toCommonMode(const uint8_t mode);
fanspeed_t ToshibaAc_toCommonFanSpeed(const uint8_t speed);



bool    ToshibaAc_validChecksum(const uint8_t state[], const uint16_t length);
void    ToshibaAc_checksum(const uint16_t length);
uint8_t ToshibaAc_calcChecksum(const uint8_t state[], const uint16_t length);

void ToshibaAc_setStateLength(const uint16_t size);
void ToshibaAc_backupState(void);
void ToshibaAc_restoreState(void);




#endif  // IR_TOSHIBA_H_



