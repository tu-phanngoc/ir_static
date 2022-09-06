/// @file
/// @brief Support for Mitsubishi protocols.


#ifndef IR_MITSUBISHI136_H_
#define IR_MITSUBISHI136_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRsend.h"
#include "IRrecv.h"


/// Native representation of a Mitsubishi 136-bit A/C message.
typedef union Mitsubishi136Protocol{
  uint8_t raw[kMitsubishi136StateLength];  ///< The state in code form.
  struct {
    // Byte 0~4
    uint8_t pad[5];
    // Byte 5
    uint8_t _rev50  :6;
    uint8_t Power   :1;
    uint8_t _rev51  :1;
    // Byte 6
    uint8_t Mode    :3;
    uint8_t _rev60  :1;
    uint8_t Temp    :4;
    // Byte 7
    uint8_t _rev70  :1;
    uint8_t Fan     :2;
    uint8_t _rev71  :1;
    uint8_t SwingV  :4;
  } frame;
} Mitsubishi136Protocol_t;

#define kMitsubishi136PowerByte         5
#define kMitsubishi136MinTemp           17  // 17C
#define kMitsubishi136MaxTemp           30  // 30C
#define kMitsubishi136Fan               0
#define kMitsubishi136Cool              1
#define kMitsubishi136Heat              2
#define kMitsubishi136Auto              3
#define kMitsubishi136Dry               5
#define kMitsubishi136SwingVLowest      0
#define kMitsubishi136SwingVLow         1
#define kMitsubishi136SwingVHigh        2
#define kMitsubishi136SwingVHighest     3
#define kMitsubishi136SwingVAuto        12
#define kMitsubishi136FanMin            0
#define kMitsubishi136FanLow            1
#define kMitsubishi136FanMed            2
#define kMitsubishi136FanMax            3
#define kMitsubishi136FanQuiet          kMitsubishi136FanMin

// Legacy defines (Deprecated)
#define MITSUBISHI_136_MAX_INDEX        274


void encode_Mitsubishi136(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Mitsubishi136(int16_t* input, uint8_t* output);

void Mitsubishi136_send(int16_t *irRaw);
bool Mitsubishi136_recv(decode_results *results, uint16_t offset,
                        const uint16_t nbits,
                        const bool strict);


void      Mitsubishi136_setPower(const bool on);
bool      Mitsubishi136_getPower(void);
void      Mitsubishi136_setTemp(const uint8_t degrees);
uint8_t   Mitsubishi136_getTemp(void);
void      Mitsubishi136_setFan(const uint8_t speed);
uint8_t   Mitsubishi136_getFan(void);
void      Mitsubishi136_setMode(const uint8_t mode);
uint8_t   Mitsubishi136_getMode(void);
void      Mitsubishi136_setSwingV(const uint8_t position);
uint8_t   Mitsubishi136_getSwingV(void);
void      Mitsubishi136_setQuiet(const bool on);
bool      Mitsubishi136_getQuiet(void);
uint8_t*  Mitsubishi136_getRaw(void);
void      Mitsubishi136_setRaw(const uint8_t* data);

static uint8_t Mitsubishi136_convertMode(const opmode_t mode);
static uint8_t Mitsubishi136_convertFan(const fanspeed_t speed);
static uint8_t Mitsubishi136_convertSwingV(const swingv_t position);

static opmode_t   Mitsubishi136_toCommonMode(const uint8_t mode);
static fanspeed_t Mitsubishi136_toCommonFanSpeed(const uint8_t speed);
static swingv_t   Mitsubishi136_toCommonSwingV(const uint8_t pos);

static bool Mitsubishi136_validChecksum(const uint8_t* data, const uint16_t len);
static void Mitsubishi136_checksum(void);



#endif  // IR_MITSUBISHI136_H_
