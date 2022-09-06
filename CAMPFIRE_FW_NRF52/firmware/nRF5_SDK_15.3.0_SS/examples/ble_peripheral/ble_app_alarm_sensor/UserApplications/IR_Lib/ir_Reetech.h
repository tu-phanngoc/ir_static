

#ifndef IR_REETECH_H_
#define IR_REETECH_H_


#include "string.h"
#include "stdint.h"
#include "stdio.h"

#include "IRcommon.h"
#include "IRsend.h"
#include "IRrecv.h"





/// Native representation of a Reetech A/C message.

// LSB BYTE ORDER
typedef union ReetechProtocol_t {
  uint8_t raw[kReetechAcStateLength];   ///< The state of the IR remote
  struct {
    // Byte 0
    uint8_t Address :8; // Fixed = 0xD5
    // Byte 1 = ~Byte0
    uint8_t rev0100 :8;
    // Byte 2
    uint8_t Temp    :5;
    uint8_t Mode    :3;
    // Byte 3 = ~Byte2
    uint8_t rev0300 :8;
    // Byte 4
    uint8_t Sleep   :1;
    uint8_t Power   :1;
    uint8_t SwingV  :2;
    uint8_t SwingH  :1;
    uint8_t Fan     :3;
    // Byte 5 = ~Byte4
    uint8_t rev0500 :8;
    // Byte 6
    uint8_t Button  :8;
    // Byte 7 = ~Byte6
    uint8_t rev0700 :8;
    // Byte 8
    uint8_t Light   :2;
    uint8_t Lock    :1;
    uint8_t Strong  :1;
    uint8_t rev0800 :4;
    // Byte 9 = ~Byte8
    uint8_t rev0900 :8;
    // Byte 10
    uint8_t Timer   :8;
    // Byte 11 = ~Byte 10
    uint8_t rev1100 :8;
  };
} ReetechProtocol_t;

// Constants
#define kReetechAcMinTemp           0      // 16C
#define kReetechAcMaxTemp           0x10   // 32C

#define kReetechAcAuto              0
#define kReetechAcCool              1
#define kReetechAcDry               2
#define kReetechAcFan               3
#define kReetechAcHeat              4

#define kReetechAcFanAuto           0
#define kReetechAcFanLow            3
#define kReetechAcFanMed            2
#define kReetechAcFanHigh           1

#define kReetechAcSwingMax          0
#define kReetechAcSwingAuto         1
#define kReetechAcSwingOff          2

#define kReetechAcButtonPower       0x00
#define kReetechAcButtonMode        0x01
#define kReetechAcButtonTempUp      0x02
#define kReetechAcButtonTempDown    0x03
#define kReetechAcButtonSwingV      0x04
#define kReetechAcButtonFan         0x05
#define kReetechAcButtonTimer       0x06
#define kReetechAcButtonSwingH      0x07
#define kReetechAcButtonLock        0x08
#define kReetechAcButtonSleep       0x09
#define kReetechAcButtonStrong      0x0A
#define kReetechAcButtonLight       0x0B


void encode_ReetechAc(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_ReetechAc(int16_t* input, uint8_t* output);

bool isHeader_Reetech(int16_t *irRaw);
void ReetechAc_send(const uint8_t data[], const uint16_t nbytes,const uint16_t repeat, int16_t *irRaw);
bool ReetechAc_recv(decode_results *results, uint16_t offset,const uint16_t nbits, const bool strict);

/// Class for handling detailed Reetech A/C messages.
void     ReetechAc_setPower(const bool on);
bool     ReetechAc_getPower(void);
void     ReetechAc_setMode(const uint8_t mode);
uint8_t  ReetechAc_getMode(void);
void     ReetechAc_setTemp(const uint8_t temp);
uint8_t  ReetechAc_getTemp(void);
void     ReetechAc_setFan(const uint8_t speed);
uint8_t  ReetechAc_getFan(void);
void     ReetechAc_setSwingV(const swingv_t swing);
bool     ReetechAc_getSwingV(void);
void     ReetechAc_setSwingH(const bool on);
bool     ReetechAc_getSwingH(void);
void     ReetechAc_setLight(const bool on);
bool     ReetechAc_getLight(void);
void     ReetechAc_setStrong(const bool on);
bool     ReetechAc_getStrong(void);
void     ReetechAc_setButton(void);


uint8_t* ReetechAc_getRaw(void);
void     ReetechAc_setRaw(const uint8_t new_code[]);

void    ReetechAc_checksum();
bool    ReetechAc_validChecksum(const uint8_t state[]);

uint8_t ReetechAc_convertMode(const opmode_t mode);
uint8_t ReetechAc_convertFan(const fanspeed_t speed);
uint8_t  ReetechAc_convertSwing(const swingv_t swing);

opmode_t   ReetechAc_toCommonMode(const uint8_t mode);
fanspeed_t ReetechAc_toCommonFanSpeed(const uint8_t speed);
swingv_t   ReetechAc_toCommonSwing(const uint8_t swing);



#endif  // IR_REETECH_H_


