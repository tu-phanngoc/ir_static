

#ifndef IR_CASPER104_H_
#define IR_CASPER104_H_


#include "string.h"
#include "stdint.h"

#include "IRcommon.h"
#include "IRsend.h"
#include "IRrecv.h"

/// Native representation of a Electra A/C message.
typedef union Casper104Protocol {
  uint8_t raw[kCasper104StateLength];   ///< The state of the IR remote
  struct {
    // Byte 0
    uint8_t Address :8;
    // Byte 1
    uint8_t SwingV  :3;
    uint8_t Temp    :5;
    // Byte 2
    uint8_t rev20   :5;
    uint8_t SwingH  :3;
    // Byte 3
    uint8_t rev30        :6;
    uint8_t SensorUpdate :1;
    uint8_t rev31        :1;
    // Byte 4
    uint8_t rev40   :5;
    uint8_t Fan     :3;
    // Byte 5
    uint8_t rev50   :6;
    uint8_t Turbo   :1;
    uint8_t Silent  :1;
    // Byte 6
    uint8_t Light   :1;
    uint8_t rev60   :1;
    uint8_t Sleep   :1;
    uint8_t IFeel   :1;
    uint8_t rev61   :1;
    uint8_t Mode    :3;
    // Byte 7
    uint8_t SensorTemp :8;
    // Byte 8
    uint8_t rev0800 :8;
    // Byte 9
    uint8_t rev90   :1;
    uint8_t Heath   :1;
    uint8_t Clean   :1;
    uint8_t Eco     :1;
    uint8_t Heat    :1;
    uint8_t Power   :1;
    uint8_t rev91   :2;
    // Byte 10
    uint8_t rev1000 :8;
    // Byte 11
    uint8_t Button  :5;
    uint8_t revB1   :3; // fixed = 0b010
    // Byte 12
    uint8_t Sum     :8;
  };
} Casper104Protocol_t;

// Constants
#define kCasper104HdrMark           9000
#define kCasper104HdrSpace          4500
#define kCasper104BitMark           560
#define kCasper104OneSpace          1690
#define kCasper104ZeroSpace         560
#define kCasper104MessageGap        kDefaultMessageGap  // Just a guess.

#define kCasper104MinTemp           16   // 16C
#define kCasper104MaxTemp           32   // 32C
#define kCasper104TempDelta         8

#define kCasper104SwingOn           0
#define kCasper104SwingOff          7

#define kCasper104FanAuto           5
#define kCasper104FanLow            3
#define kCasper104FanMed            2
#define kCasper104FanHigh           1

#define kCasper104Auto              0
#define kCasper104Cool              1
#define kCasper104Dry               2
#define kCasper104Heat              4
#define kCasper104Fan               6

// #define kCasper104LightToggleOn     0x15
// #define kCasper104LightToggleMask   0x11
// #define kCasper104LightToggleOff    0x08

#define kCasper104ButtonTempUp          0
#define kCasper104ButtonTempDown        1
#define kCasper104ButtonRightSwing      2
#define kCasper104ButtonLeftSwing       3
#define kCasper104ButtonFan             4
#define kCasper104ButtonOnOff           5
#define kCasper104ButtonMode            6
#define kCasper104ButtonHeath           7
#define kCasper104ButtonSilent          8
#define kCasper104ButtonLight           9
#define kCasper104ButtonSleep           0xB
#define kCasper104ButtonEco             0x13
#define kCasper104ButtonIFeel           0x1E
#define kCasper104ButtonDisplay         0x15


// Re: Byte[7]. Or Delta == 0xA and Temperature are stored in last 6 bits,
// and bit 7 stores Unknown flag
#define kCasper104SensorTempDelta   0x4A
#define kCasper104SensorMinTemp     0    // 0C
#define kCasper104SensorMaxTemp     50   // 50C

void encode_Casper104(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Casper104(int16_t* input, uint8_t* output);


bool Casper104_isHeaderMatch(int16_t *rawbuf);


void Casper104_send(const uint8_t data[], const uint16_t nbytes,
                           const uint16_t repeat, int16_t *irRaw);
bool Casper104_recv(decode_results *results, uint16_t offset,
                             const uint16_t nbits,
                             const bool strict);

/// Class for handling detailed Electra A/C messages.
void     Casper104_begin(void);
void     Casper104_setPower(const bool on);
bool     Casper104_getPower(void);
void     Casper104_setMode(const uint8_t mode);
uint8_t  Casper104_getMode(void);
void     Casper104_setTemp(const uint8_t temp);
uint8_t  Casper104_getTemp(void);
void     Casper104_setFan(const uint8_t speed);
uint8_t  Casper104_getFan(void);
void     Casper104_setSwingV(const bool on);
bool     Casper104_getSwingV(void);
void     Casper104_setSwingH(const bool on);
bool     Casper104_getSwingH(void);
void     Casper104_setClean(const bool on);
bool     Casper104_getClean(void);
void     Casper104_setLightToggle(const bool on);
bool     Casper104_getLightToggle(void);
void     Casper104_setTurbo(const bool on);
bool     Casper104_getTurbo(void);
void     Casper104_setSleep(const bool on);
bool     Casper104_getSleep(void);
void     Casper104_setHeath(const bool on);
bool     Casper104_getHeath(void);
void     Casper104_setIFeel(const bool on);
bool     Casper104_getIFeel(void);
void     Casper104_setSensorUpdate(const bool on);
bool     Casper104_getSensorUpdate(void);
void     Casper104_setSensorTemp(const uint8_t temp);
uint8_t  Casper104_getSensorTemp(void);
uint8_t* Casper104_getRaw(void);
void     Casper104_setRaw(const uint8_t new_code[]);
void     Casper104_setButton(void);
uint8_t  Casper104_getButton(void);

void    Casper104_checksum();
bool    Casper104_validChecksum(const uint8_t state[]);
uint8_t Casper104_calcChecksum(const uint8_t state[]);

uint8_t Casper104_convertMode(const opmode_t mode);
uint8_t Casper104_convertFan(const fanspeed_t speed);
opmode_t   Casper104_toCommonMode(const uint8_t mode);
fanspeed_t Casper104_toCommonFanSpeed(const uint8_t speed);


#endif  // IR_CASPER104_H_
