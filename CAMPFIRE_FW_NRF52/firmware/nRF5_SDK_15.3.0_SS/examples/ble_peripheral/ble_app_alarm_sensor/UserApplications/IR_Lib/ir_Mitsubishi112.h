/// @file
/// @brief Support for Mitsubishi protocols.

// Supports:
//   Brand: Mitsubishi,  Model: TV (MITSUBISHI)
//   Brand: Mitsubishi,  Model: HC3000 Projector (MITSUBISHI2)
//   Brand: Mitsubishi,  Model: MS-GK24VA A/C
//   Brand: Mitsubishi,  Model: KM14A 0179213 remote
//   Brand: Mitsubishi Electric,  Model: PEAD-RP71JAA Ducted A/C (MITSUBISHI136)
//   Brand: Mitsubishi Electric,  Model: 001CP T7WE10714 remote (MITSUBISHI136)
//   Brand: Mitsubishi Electric,  Model: MSH-A24WV A/C (MITSUBISHI112)
//   Brand: Mitsubishi Electric,  Model: MUH-A24WV A/C (MITSUBISHI112)
//   Brand: Mitsubishi Electric,  Model: KPOA remote (MITSUBISHI112)
//   Brand: Mitsubishi Electric,  Model: MLZ-RX5017AS A/C (MITSUBISHI_AC)
//   Brand: Mitsubishi Electric,  Model: SG153/M21EDF426 remote (MITSUBISHI_AC)
//   Brand: Mitsubishi Electric,  Model: MSZ-GV2519 A/C (MITSUBISHI_AC)
//   Brand: Mitsubishi Electric,  Model: RH151/M21ED6426 remote (MITSUBISHI_AC)
//   Brand: Mitsubishi Electric,  Model: MSZ-SF25VE3 A/C (MITSUBISHI_AC)
//   Brand: Mitsubishi Electric,  Model: SG15D remote (MITSUBISHI_AC)
//   Brand: Mitsubishi Electric,  Model: MSZ-ZW4017S A/C (MITSUBISHI_AC)

#ifndef IR_MITSUBISHI112_H_
#define IR_MITSUBISHI112_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRsend.h"
#include "IRrecv.h"

// Constants
/// Native representation of a Mitsubishi 112-bit A/C message.
typedef union Mitsubishi112Protocol{
  uint8_t raw[kMitsubishi112StateLength];  ///< The state in code form.
  struct {
    // Byte 0~4
    uint8_t pad0[5];
    // Byte 5
    uint8_t rev50 :2;
    uint8_t Power :1;
    uint8_t rev51 :5;
    // Byte 6
    uint8_t Mode  :4;
    uint8_t rev60 :4;
    // Byte 7
    uint8_t Temp  :4;
    uint8_t rev70 :4;
    // Byte 8
    uint8_t Fan     :3;
    uint8_t SwingV  :3;
    uint8_t rev80   :2;
    // Byte 9~11
    uint8_t pad1[3];
    // Byte 12
    uint8_t rev120  :2;
    uint8_t SwingH  :4;
    uint8_t rev121   :2;
    // Byte 13
    uint8_t Sum :8;
  } frame;
} Mitsubishi112Protocol_t;

#define kMitsubishi112Cool                          3
#define kMitsubishi112Heat                          1
#define kMitsubishi112Auto                          8
#define kMitsubishi112Dry                           2
#define kMitsubishi112Fan                           7

#define kMitsubishi112MinTemp                       16  // 16C
#define kMitsubishi112MaxTemp                       31  // 31C

#define kMitsubishi112FanMin                        2
#define kMitsubishi112FanMed                        3
#define kMitsubishi112FanMax                        5
#define kMitsubishi112FanAuto                       0

#define kMitsubishi112FanQuiet                      kMitsubishi112FanMin
#define kMitsubishi112SwingVLowest                  5
#define kMitsubishi112SwingVLow                     4
#define kMitsubishi112SwingVMiddle                  3
#define kMitsubishi112SwingVHigh                    2
#define kMitsubishi112SwingVHighest                 1
#define kMitsubishi112SwingVAuto                    7

#define kMitsubishi112SwingHLeftMax                 1
#define kMitsubishi112SwingHLeft                    2
#define kMitsubishi112SwingHMiddle                  3
#define kMitsubishi112SwingHRight                   4
#define kMitsubishi112SwingHRightMax                5
#define kMitsubishi112SwingHWide                    8
#define kMitsubishi112SwingHAuto                    12

// Legacy defines (Deprecated)
#define MITSUBISHI_112_MAX_INDEX                    226


void encode_Mitsubishi112(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Mitsubishi112(int16_t* input, uint8_t* output);

void Mitsubishi112_send(int16_t *irRaw);
bool Mitsubishi112_recv(decode_results *results, uint16_t offset,
                        const uint16_t nbits,
                        const bool strict);

void      Mitsubishi112_setPower(const bool on);
bool      Mitsubishi112_getPower(void);
void      Mitsubishi112_setTemp(const uint8_t degrees);
uint8_t   Mitsubishi112_getTemp(void);
void      Mitsubishi112_setFan(const uint8_t speed);
uint8_t   Mitsubishi112_getFan(void);
void      Mitsubishi112_setMode(const uint8_t mode);
uint8_t   Mitsubishi112_getMode(void);
void      Mitsubishi112_setSwingV(const uint8_t position);
uint8_t   Mitsubishi112_getSwingV(void);
void      Mitsubishi112_setSwingH(const uint8_t position);
uint8_t   Mitsubishi112_getSwingH(void);
void      Mitsubishi112_setQuiet(const bool on);
bool      Mitsubishi112_getQuiet(void);
uint8_t*  Mitsubishi112_getRaw(void);
void      Mitsubishi112_setRaw(const uint8_t* data);

static void    Mitsubishi112_checksum(void);
static uint8_t Mitsubishi112_calcChecksum(uint8_t state[], const uint16_t length );
static bool    Mitsubishi112_validChecksum(uint8_t state[], const uint16_t length );

static uint8_t Mitsubishi112_convertMode(const opmode_t mode);
static uint8_t Mitsubishi112_convertFan(const fanspeed_t speed);
static uint8_t Mitsubishi112_convertSwingV(const swingv_t position);
static uint8_t Mitsubishi112_convertSwingH(const swingh_t position);

static opmode_t   Mitsubishi112_toCommonMode(const uint8_t mode);
static fanspeed_t Mitsubishi112_toCommonFanSpeed(const uint8_t speed);
static swingv_t   Mitsubishi112_toCommonSwingV(const uint8_t pos);
static swingh_t   Mitsubishi112_toCommonSwingH(const uint8_t pos);




#endif  // IR_MITSUBISHI112_H_
