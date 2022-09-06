
/// @file
/// @brief Support for LG protocols.

// Supports:
//   Brand: LG,  Model: 6711A20083V remote (LG)
//   Brand: LG,  Model: AKB74395308 remote (LG2)
//   Brand: LG,  Model: S4-W12JA3AA A/C (LG2)
//   Brand: LG,  Model: AKB75215403 remote (LG2)
//   Brand: LG,  Model: AKB74955603 remote (LG2 - AKB74955603)
//   Brand: LG,  Model: A4UW30GFA2 A/C (LG2 - AKB74955603 & AKB73757604)
//   Brand: LG,  Model: AMNW09GSJA0 A/C (LG2 - AKB74955603)
//   Brand: LG,  Model: AMNW24GTPA1 A/C (LG2 - AKB73757604)
//   Brand: LG,  Model: AKB73757604 remote (LG2 - AKB73757604)
//   Brand: LG,  Model: AKB73315611 remote (LG2 - AKB74955603)
//   Brand: LG,  Model: MS05SQ NW0 A/C (LG2 - AKB74955603)
//   Brand: General Electric,  Model: AG1BH09AW101 Split A/C (LG)
//   Brand: General Electric,  Model: 6711AR2853M A/C Remote (LG)

#ifndef IR_LG_H_
#define IR_LG_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include "IRcommon.h"
#include "IRsend.h"
#include "IRutils.h"

/// Native representation of a LG A/C message.
typedef union LGProtocol{
  uint32_t raw;  ///< The state of the IR remote in IR code form.
  struct {
    uint32_t Sum  :4;
    uint32_t Fan  :4;
    uint32_t Temp :4;
    uint32_t Mode :3;
    uint32_t reverse :3;
    uint32_t Power:2;
    uint32_t Sign :8;
  };
} LGProtocol_t;

#define kLgAcFanLowest                  0
#define kLgAcFanLow                     1
#define kLgAcFanMedium                  2
#define kLgAcFanMax                     4
#define kLgAcFanAuto                    5
#define kLgAcFanLowAlt                  9
#define kLgAcFanHigh                    10
// Nr. of slots in the look-up table
#define kLgAcFanEntries                 kLgAcFanHigh + 1;
#define kLgAcTempAdjust                 15
#define kLgAcMinTemp                    16  // Celsius
#define kLgAcMaxTemp                    30  // Celsius
#define kLgAcCool                       0
#define kLgAcDry                        1
#define kLgAcFan                        2
#define kLgAcAuto                       3
#define kLgAcHeat                       4
#define kLgAcPowerOff                   3
#define kLgAcPowerOn                    0
#define kLgAcSignature                  0x88

#define kLgAcOffCommand                0x88C0051
#define kLgAcLightToggle               0x88C00A6

#define kLgAcSwingSignature            0x8813
#define kLgAcSwingVLowest              0x8813048
#define kLgAcSwingVLow                 0x8813059
#define kLgAcSwingVMiddle              0x881306A
#define kLgAcSwingVUpperMiddle         0x881307B
#define kLgAcSwingVHigh                0x881308C
#define kLgAcSwingVHighest             0x881309D
#define kLgAcSwingVSwing               0x8813149
#define kLgAcSwingVAuto                kLgAcSwingVSwing
#define kLgAcSwingVOff                 0x881315A
#define kLgAcSwingVLowest_Short        0x04
#define kLgAcSwingVLow_Short           0x05
#define kLgAcSwingVMiddle_Short        0x06
#define kLgAcSwingVUpperMiddle_Short   0x07
#define kLgAcSwingVHigh_Short          0x08
#define kLgAcSwingVHighest_Short       0x09
#define kLgAcSwingVSwing_Short         0x14
#define kLgAcSwingVAuto_Short          kLgAcSwingVSwing_Short
#define kLgAcSwingVOff_Short           0x15

// AKB73757604 Constants
// SwingH
#define kLgAcSwingHAuto                0x881316B
#define kLgAcSwingHOff                 0x881317C
// SwingV
#define kLgAcVaneSwingVHighest         1  ///< 0b001
#define kLgAcVaneSwingVHigh            2  ///< 0b010
#define kLgAcVaneSwingVUpperMiddle     3  ///< 0b011
#define kLgAcVaneSwingVMiddle          4  ///< 0b100
#define kLgAcVaneSwingVLow             5  ///< 0b101
#define kLgAcVaneSwingVLowest          6  ///< 0b110
#define kLgAcVaneSwingVSize            8
#define kLgAcSwingVMaxVanes            4  ///< Max Nr. of Vanes


void encode_LG(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_LG(int16_t* input, uint8_t* output);

void LG_send(uint64_t data, uint16_t nbits, uint16_t repeat, int16_t *irRaw);
void LG_send2(uint64_t data, uint16_t nbits, uint16_t repeat, int16_t *irRaw);
bool LG_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);


// Classes
/// Class for handling detailed LG A/C messages.
static uint8_t LG_calcChecksum(const uint32_t state);
static bool LG_validChecksum(const uint32_t state);
static void LG_checksum(void);

bool     LG_isValidLgAc(void);
void     LG_begin(void);
void     LG_setPower(const bool on);
bool     LG_getPower(void);
bool     LG_isOffCommand(void);
void     LG_setTemp(const uint8_t degrees);
uint8_t  LG_getTemp(void);
void     LG_setFan(const uint8_t speed);
uint8_t  LG_getFan(void);
void     LG_setMode(const uint8_t mode);
uint8_t  LG_getMode(void);
void     LG_setLight(const bool on);
bool     LG_getLight(void);
bool     LG_isLightToggle(void);
bool     LG_isSwing(void);
void     LG_setSwingH(const bool on);
bool     LG_getSwingH(void);
bool     LG_isSwingV(void);
bool     LG_isVaneSwingV(void);
void     LG_setSwingV(const uint32_t position);
uint32_t LG_getSwingV(void);
void     LG_setVaneSwingV(const uint8_t vane, const uint8_t position);
uint8_t  LG_getVaneSwingV(const uint8_t vane);
bool     LG_isSwingH(void);
void     LG_updateSwingPrev(void);
uint32_t LG_getRaw(void);
void     LG_setRaw(const uint32_t new_code, const decode_type_t protocol);
void     LG_setModel(const lg_ac_remote_model_t model);
lg_ac_remote_model_t LG_getModel(void);

static uint32_t   LG_calcVaneSwingV(const uint8_t vane, const uint8_t position);
static uint8_t    LG_getVaneCode(const uint32_t raw);
static opmode_t   LG_toCommonMode(const uint8_t mode);
static fanspeed_t LG_toCommonFanSpeed(const uint8_t speed);
static swingv_t   LG_toCommonSwingV(const uint32_t code);
static swingv_t   LG_toCommonVaneSwingV(const uint8_t pos);
static uint8_t    LG_convertMode(const opmode_t mode);
static uint8_t    LG_convertFan(const fanspeed_t speed);
static uint32_t   LG_convertSwingV(const swingv_t swingv);
static uint8_t    LG_convertVaneSwingV(const swingv_t swingv);

void _setTemp(const uint8_t value);
static bool _isAKB74955603(void);
static bool _isAKB73757604(void);
static bool _isNormal(void);


#endif  // IR_LG_H_
