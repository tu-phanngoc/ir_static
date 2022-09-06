#ifndef __IR_CASPER_H
#define __IR_CASPER_H



#include <cstdint>
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include <stdbool.h>
#include "IRsend.h"
#include "IRrecv.h"


void IR_EncodeUserCmdToIRProtocol_Casper(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_Casper(int16_t* input, uint8_t* output);
int IsEnableIrTx_Casper(void);
void SetIrTxState_Casper(int state);


// Constants

#define kCasperAcTempMin                 17      ///< Celsius
#define kCasperAcTempMax                 30      ///< Celsius
#define kCasperAcTempMedium              25      ///< Celsius

// #define kCasperAcTempDelta               4       ///< Celsius to Native Temp difference.
#define kCasperAcCool                    0       // 
#define kCasperAcAuto                    8       // 
#define kCasperAcDry                     4       // 
#define kCasperAcFan                     4       // 
#define kCasperAcHeat                    0xC       // 

#define kCasperAcModeAppend_B            0xB       // 
#define kCasperAcModeAppend_1            1       // 

#define kCasperAcFanLow                  9       ///<            0b10
#define kCasperAcFanHigh                 5       ///<            0b01
#define kCasperAcFanMedium               3       ///<            0b11
#define kCasperAcPowerAuto               0xB // default on

#define kCasperAcPowerOffFan             0x7 // default on
#define kCasperAcPowerOffSpecial         0xB // 
#define kCasperAcPowerOnFan              0x1 // default on
#define kCasperAcPowerOnSpecial          0xF //

#define kCasperAcPowerOffTemp             0xE // default on
#define kCasperAcPowerOffMode             0x0 // 

#define kCasperAcSwingVAuto              0x6

#define kCasperAcHdrMark                 4500    ///< uSeconds
#define kCasperAcHdrSpace                4500    ///< uSeconds
#define kCasperAcBitMark                 550     ///< uSeconds
#define kCasperAcOneSpace                1650    ///< uSeconds
#define kCasperAcZeroSpace               550     ///< uSeconds
#define kCasperAcGap                     5600    ///< uSeconds 
#define kCasperAcFreq                    38000   ///< Hz.

#define kCasperAcStateLength             6
#define kCasperAcBits                    kCasperAcStateLength * 8

#define DATASET_MAX_INDEX_CASPER         199

/// Native representation of a Sanyo A/C message.

typedef union CasperProtocol{
  uint8_t raw[kCasperAcStateLength];  ///< The state in IR code form.
  struct{
    // Byte 0
    uint8_t Address :8;   // 0xB2 (Fixed?)
    // Byte 1
    uint8_t rev0100 :8;   // exor of 0xB2 = 0x4D
    // Byte 2
    uint8_t Fan     :4;
    uint8_t Special :4;
    // Byte 3
    uint8_t rev0300 :8;   // exor of Special
    // Byte 4
    uint8_t Temp  :4;
		uint8_t Mode  :4;
    // Byte 5
    uint8_t rev0500 :8;   // exor of Mode
  };
} CasperProtocol_t;

/// Class for handling detailed Sanyo A/C messages.
void decode_casper(int16_t* input, uint8_t* output);
void encode_casper(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);

void casper_send( const uint8_t data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool casper_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);

void casper_begin(void);
void casper_setPower(const bool on);
bool casper_getPower(void);
void casper_setTemp(const uint8_t degrees);
uint8_t casper_getTemp(void);
void casper_setFan(const uint8_t speed);
uint8_t casper_getFan(void);
void casper_setMode(const uint8_t mode);
uint8_t casper_getMode(void);
void casper_setSwingV(void);
uint8_t casper_getSwingV(void);
void casper_setRaw(const uint8_t newState[]);
uint8_t* casper_getRaw(void);
void casper_checksum(void);

uint8_t casper_convertMode(const opmode_t mode);
uint8_t casper_convertFan(const fanspeed_t speed);
uint8_t casper_convertSwingV(const swingv_t position);

opmode_t   casper_toCommonMode(const uint8_t mode);
fanspeed_t casper_toCommonFanSpeed(const uint8_t speed);
uint8_t    casper_toCommonTemp(const uint8_t tmp);
uint8_t    casper_convertTemp(const uint8_t tmp);

#endif

