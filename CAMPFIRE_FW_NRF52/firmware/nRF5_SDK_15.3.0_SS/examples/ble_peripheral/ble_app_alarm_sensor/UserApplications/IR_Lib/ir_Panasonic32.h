
/// @file
/// @brief Support for Panasonic protocols.


#ifndef IR_PANASONIC32_H_
#define IR_PANASONIC32_H_

#define __STDC_LIMIT_MACROS
#include "stdint.h"
#include "stdbool.h"

#include "IRsend.h"
#include "IRrecv.h"

// Constants


/// Native representation of a Panasonic 32-bit A/C message.
typedef union PanasonicAc32Protocol {
  uint32_t raw;  ///< The state in IR code form.
  struct {
    // Byte 0
    uint8_t rev01       :3;
    uint8_t SwingH      :1;
    uint8_t SwingV      :3;
    uint8_t rev02       :1;  ///< Always appears to be set. (1)
    // Byte 1
    uint8_t rev1x       :8;  // Always seems to be 0x36.
    // Byte 2
    uint8_t Temp        :4;
    uint8_t Fan         :4;
    // Byte 3
    uint8_t Mode        :3;
    uint8_t PowerToggle :1;  // 0 means toggle, 1 = keep the same.
    uint8_t rev3x       :4;
  } frame;
} PanasonicAc32Protocol_t;

#define kPanasonicAc32Fan                         1
#define kPanasonicAc32Cool                        2
#define kPanasonicAc32Dry                         3
#define kPanasonicAc32Heat                        4
#define kPanasonicAc32Auto                        6

#define kPanasonicAc32FanMin                      2
#define kPanasonicAc32FanLow                      3
#define kPanasonicAc32FanMed                      4
#define kPanasonicAc32FanHigh                     5
#define kPanasonicAc32FanMax                      6
#define kPanasonicAc32FanAuto                     0xF
#define kPanasonicAc32SwingVAuto                  0x7
#define kPanasonicAc32KnownGood                   0x0AF136FC  ///< Cool, Auto, 16C

#define DATASET_MAX_INDEX_PANASONICAC32_LONG     271
#define DATASET_MAX_INDEX_PANASONICAC32_SHORT    201



void encode_PanasonicAc32(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_PanasonicAc32(int16_t* input, uint8_t* output);

void PanasonicAc32_send(const uint16_t nbits, int16_t *irRaw);
bool PanasonicAc32_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);


/// Class for handling detailed Panasonic 32bit A/C messages.
void     PanasonicAc32_begin(void);
void     PanasonicAc32_setPowerToggle(const bool on);
bool     PanasonicAc32_getPowerToggle(void);
void     PanasonicAc32_setTemp(const uint8_t temp);
uint8_t  PanasonicAc32_getTemp(void);
void     PanasonicAc32_setFan(const uint8_t fan);
uint8_t  PanasonicAc32_getFan(void);
void     PanasonicAc32_setMode(const uint8_t mode);
uint8_t  PanasonicAc32_getMode(void);
void     PanasonicAc32_setRaw(const uint32_t state);
uint32_t PanasonicAc32_getRaw(void);
void     PanasonicAc32_setSwingVertical(const uint8_t pos);
uint8_t  PanasonicAc32_getSwingVertical(void);
void     PanasonicAc32_setSwingHorizontal(const bool on);
bool     PanasonicAc32_getSwingHorizontal(void);

static uint8_t PanasonicAc32_convertMode(const opmode_t mode);
static uint8_t PanasonicAc32_convertFan(const fanspeed_t speed);
static uint8_t PanasonicAc32_convertSwingV(const swingv_t position);

static opmode_t   PanasonicAc32_toCommonMode(const uint8_t mode);
static fanspeed_t PanasonicAc32_toCommonFanSpeed(const uint8_t speed);
static swingv_t   PanasonicAc32_toCommonSwingV(const uint8_t pos);

#endif  // IR_PANASONIC32_H_
