/// @file
/// @brief Support for Gree A/C protocols.

// Supports:
//   Brand: Ultimate,  Model: Heat Pump
//   Brand: EKOKAI,  Model: A/C
//   Brand: RusClimate,  Model: EACS/I-09HAR_X/N3 A/C
//   Brand: RusClimate,  Model: YAW1F remote
//   Brand: Green,  Model: YBOFB remote
//   Brand: Green,  Model: YBOFB2 remote
//   Brand: Gree,  Model: YAA1FBF remote
//   Brand: Gree,  Model: YB1F2F remote
//   Brand: Gree,  Model: YAN1F1 remote
//   Brand: Gree,  Model: VIR09HP115V1AH A/C
//   Brand: Gree,  Model: VIR12HP230V1AH A/C
//   Brand: Amana,  Model: PBC093G00CC A/C
//   Brand: Amana,  Model: YX1FF remote
//   Brand: Cooper & Hunter,  Model: YB1F2 remote
//   Brand: Cooper & Hunter,  Model: CH-S09FTXG A/C
//   Brand: Vailland,  Model: YACIFB remote
//   Brand: Vailland,  Model: VAI5-035WNI A/C

#ifndef IR_GREE_H_
#define IR_GREE_H_

#include <stdint.h>

#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"
#include "IRutils.h"

/// Native representation of a Gree A/C message.
union GreeProtocol{
  uint8_t remote_state[kGreeStateLength];  ///< The state in native IR code form
  struct {
    // Byte 0
    uint8_t Mode      :3;
    uint8_t Power     :1;
    uint8_t Fan       :2;
    uint8_t SwingAuto :1;
    uint8_t Sleep     :1;
    // Byte 1
    uint8_t Temp        :4;
    uint8_t TimerHalfHr :1;
    uint8_t TimerTensHr :2;
    uint8_t TimerEnabled:1;
    // Byte 2
    uint8_t TimerHours:4;
    uint8_t Turbo     :1;
    uint8_t Light     :1;
    uint8_t ModelA    :1;  // model==YAW1F
    uint8_t Xfan      :1;
    // Byte 3
    uint8_t rev0300         :2;
    uint8_t TempExtraDegreeF:1;
    uint8_t UseFahrenheit   :1;
    uint8_t unknown1        :4;  // value=0b0101
    // Byte 4
    uint8_t SwingV      :4;
    uint8_t SwingH      :3;
    uint8_t rev0400     :1;
    // Byte 5
    uint8_t DisplayTemp :2;
    uint8_t IFeel       :1;
    uint8_t unknown2    :3;  // value = 0b100
    uint8_t WiFi        :1;
    uint8_t rev0500     :1;
    // Byte 6
    uint8_t rev0600     :8;
    // Byte 7
    uint8_t rev0700     :2;
    uint8_t Econo       :1;
    uint8_t rev0701     :1;
    uint8_t Sum         :4;
  };
};

// Constants

#define kGreeAuto   0
#define kGreeCool   1
#define kGreeDry    2
#define kGreeFan    3
#define kGreeHeat   4

#define kGreeFanAuto   0
#define kGreeFanMin    1
#define kGreeFanMed    2
#define kGreeFanMax    3

#define kGreeMinTempC   00// 16 Celsius
#define kGreeMaxTempC   14// 30 Celsius
#define kGreeMinTempF   61  // Fahrenheit
#define kGreeMaxTempF   86  // Fahrenheit
#define kGreeTimerMax   1440 // 24 * 60

#define kGreeSwingLastPos      0  // 0b0000
#define kGreeSwingAuto         1  // 0b0001
#define kGreeSwingUp           2  // 0b0010
#define kGreeSwingMiddleUp     3  // 0b0011
#define kGreeSwingMiddle       4  // 0b0100
#define kGreeSwingMiddleDown   5  // 0b0101
#define kGreeSwingDown         6  // 0b0110
#define kGreeSwingDownAuto     7  // 0b0111
#define kGreeSwingMiddleAuto   9  // 0b1001
#define kGreeSwingUpAuto       11 // 0b1011

#define kGreeSwingHOff          0 // 0b000
#define kGreeSwingHAuto         1 // 0b001
#define kGreeSwingHMaxLeft      2 // 0b010
#define kGreeSwingHLeft         3 // 0b011
#define kGreeSwingHMiddle       4 // 0b100
#define kGreeSwingHRight        5 // 0b101
#define kGreeSwingHMaxRight     6 // 0b110

#define kGreeDisplayTempOff       0 // 0b00
#define kGreeDisplayTempSet       1 // 0b01
#define kGreeDisplayTempInside    2 // 0b10
#define kGreeDisplayTempOutside   3 // 0b11


#define kGreeChecksumStart        10

// Legacy defines.

void encode_Gree(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Gree(int16_t* input, uint8_t* output);
bool isGreeAc(int16_t *irRaw);
bool Gree_recv(decode_results* results, uint16_t offset, const uint16_t nbits, bool const strict);
void Gree_send( const uint8_t data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);

// Classes
/// Class for handling detailed Gree A/C messages.
void Gree_setModel(const gree_ac_remote_model_t model);
gree_ac_remote_model_t Gree_getModel(void);
void Gree_setPower(const bool on);
bool Gree_getPower(void);
void    Gree_setTemp(const uint8_t temp, const bool fahrenheit);
uint8_t Gree_getTemp(void);
void Gree_setUseFahrenheit(const bool on);
bool Gree_getUseFahrenheit(void);
void Gree_setFan(const uint8_t speed);
uint8_t Gree_getFan(void);
void    Gree_setMode(const uint8_t new_mode);
uint8_t Gree_getMode(void);
void Gree_setLight(const bool on);
bool Gree_getLight(void);
void Gree_setXFan(const bool on);
bool Gree_getXFan(void);
void Gree_setSleep(const bool on);
bool Gree_getSleep(void);
void Gree_setTurbo(const bool on);
bool Gree_getTurbo(void);
void Gree_setEcono(const bool on);
bool Gree_getEcono(void);
void Gree_setIFeel(const bool on);
bool Gree_getIFeel(void);
void Gree_setWiFi(const bool on);
bool Gree_getWiFi(void);
void Gree_setSwingVertical(const bool automatic, const uint8_t position);
bool Gree_getSwingVerticalAuto(void);
uint8_t  Gree_getSwingVerticalPosition(void);
void     Gree_setSwingHorizontal(const uint8_t position);
uint8_t  Gree_getSwingHorizontal(void);
uint16_t Gree_getTimer(void);
void    Gree_setTimer(const uint16_t minutes);
void    Gree_setDisplayTempSource(const uint8_t mode);
uint8_t Gree_getDisplayTempSource(void);
uint8_t Gree_convertMode(const opmode_t mode);
uint8_t Gree_convertFan(const fanspeed_t speed);
uint8_t Gree_convertSwingV(const swingv_t swingv);
uint8_t Gree_convertSwingH(const swingh_t swingh);

opmode_t   Gree_toCommonMode(const uint8_t mode);
fanspeed_t Gree_toCommonFanSpeed(const uint8_t speed);
swingv_t   Gree_toCommonSwingV(const uint8_t pos);
swingh_t   Gree_toCommonSwingH(const uint8_t pos);
state_t    Gree_toCommon(void);

uint8_t* Gree_getRaw(void);
void Gree_setRaw(const uint8_t new_code[]);
bool Gree_validChecksum(const uint8_t state[],const uint16_t length);

void Gree_checksum(const uint16_t length);
uint8_t Gree_calcBlockChecksum(const uint8_t *block, const uint16_t length);
void Gree_fixup(void);
void Gree_setTimerEnabled(const bool on);
bool Gree_getTimerEnabled(void);

#endif  // IR_GREE_H_
