/// @file
/// @brief Support for Mitsubishi protocols.

#ifndef IR_MITSUBISHI_H_
#define IR_MITSUBISHI_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRsend.h"
#include "IRrecv.h"

// Constants
#define kMitsubishi144Auto                4
#define kMitsubishi144Cool                3
#define kMitsubishi144Dry                 2
#define kMitsubishi144Heat                1
#define kMitsubishi144Fan                 7
#define kMitsubishi144FanAuto             0
#define kMitsubishi144FanMax              5
#define kMitsubishi144FanRealMax          4
#define kMitsubishi144FanSilent           6
#define kMitsubishi144FanQuiet            kMitsubishi144FanSilent
#define kMitsubishi144MinTemp             16.0  // 16C
#define kMitsubishi144MaxTemp             31.0  // 31C
#define kMitsubishi144VaneAuto            0  // Vanes move when AC wants to.
#define kMitsubishi144VaneHighest         1
#define kMitsubishi144VaneHigh            2
#define kMitsubishi144VaneMiddle          3
#define kMitsubishi144VaneLow             4
#define kMitsubishi144VaneLowest          5
#define kMitsubishi144VaneSwing           7  // Vanes move all the time.
#define kMitsubishi144VaneAutoMove        kMitsubishi144VaneSwing  // Deprecated
#define kMitsubishi144WideVaneLeftMax     1
#define kMitsubishi144WideVaneLeft        2
#define kMitsubishi144WideVaneMiddle      3
#define kMitsubishi144WideVaneRight       4
#define kMitsubishi144WideVaneRightMax    5
#define kMitsubishi144WideVaneWide        6
#define kMitsubishi144WideVaneAuto        8
#define kMitsubishi144NoTimer             0
#define kMitsubishi144StartTimer          5
#define kMitsubishi144StopTimer           3
#define kMitsubishi144StartStopTimer      7



// Legacy defines (Deprecated)
#define MITSUBISHI_144_MAX_INDEX            583 // 292data

#define MITSUBISHI_AC_VANE_AUTO_MOVE        kMitsubishi144VaneAutoMove
#define MITSUBISHI_AC_VANE_AUTO             kMitsubishi144VaneAuto
#define MITSUBISHI_AC_MIN_TEMP              kMitsubishi144MinTemp
#define MITSUBISHI_AC_MAX_TEMP              kMitsubishi144MaxTemp
#define MITSUBISHI_AC_HEAT                  kMitsubishi144Heat
#define MITSUBISHI_AC_FAN_SILENT            kMitsubishi144FanSilent
#define MITSUBISHI_AC_FAN_REAL_MAX          kMitsubishi144FanRealMax
#define MITSUBISHI_AC_FAN_MAX               kMitsubishi144FanMax
#define MITSUBISHI_AC_FAN_AUTO              kMitsubishi144FanAuto
#define MITSUBISHI_AC_DRY                   kMitsubishi144Dry
#define MITSUBISHI_AC_COOL                  kMitsubishi144Cool
#define MITSUBISHI_AC_AUTO                  kMitsubishi144Auto


/// Native representation of a Mitsubishi 144-bit A/C message.
typedef union Mitsubishi144Protocol{
  uint8_t raw[kMitsubishiACStateLength];  ///< The state in code form.
  struct {
    // Byte 0~4
    uint8_t pad0[5];
    // Byte 5
    uint8_t rev50 :5;
    uint8_t Power :1;
    uint8_t rev51 :2;
    // Byte 6
    uint8_t rev60 :3;
    uint8_t Mode  :3;
    uint8_t rev61 :2;
    // Byte 7
    uint8_t Temp       :4;
    uint8_t HalfDegree :1;
    uint8_t rev70      :3;
    // Byte 8
    uint8_t rev80   :4;
    uint8_t WideVane:4;  // SwingH
    // Byte 9
    uint8_t Fan     :3;
    uint8_t Vane    :3;  // SwingV
    uint8_t VaneBit :1;
    uint8_t FanAuto :1;
    // Byte 10
    uint8_t Clock   :8;
    // Byte 11
    uint8_t StopClock :8;
    // Byte 12
    uint8_t StartClock:8;
    // Byte 13
    uint8_t Timer       :3;
    uint8_t WeeklyTimer :1;
    uint8_t rev13       :4;
    // Byte 14
    uint8_t rev14       :8;
    // Byte 15
    uint8_t rev15    :8;
    // Byte 16
    uint8_t rev160   :3;
    uint8_t VaneLeft :3;  // SwingV(Left)
    uint8_t rev161   :2;
    // Byte 17
    uint8_t Sum   :8;
  } frame;
} Mitsubishi144Protocol_t;



void encode_Mitsubishi144(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Mitsubishi144(int16_t* input, uint8_t* output);

void Mitsubishi144_send(int16_t *irRaw);
bool Mitsubishi144_recv(decode_results *results, uint16_t offset,
                        const uint16_t nbits,
                        const bool strict);



void Mitsubishi144_setPower(const bool on);
bool Mitsubishi144_getPower(void);
void Mitsubishi144_setTemp(const float degrees);
float Mitsubishi144_getTemp(void);
void Mitsubishi144_setFan(const uint8_t speed);
uint8_t Mitsubishi144_getFan(void);
void Mitsubishi144_setMode(const uint8_t mode);
uint8_t Mitsubishi144_getMode(void);
void Mitsubishi144_setVane(const uint8_t position);
void Mitsubishi144_setWideVane(const uint8_t position);
uint8_t Mitsubishi144_getVane(void);
uint8_t Mitsubishi144_getWideVane(void);
void Mitsubishi144_setVaneLeft(const uint8_t position);
uint8_t Mitsubishi144_getVaneLeft(void);
uint8_t* Mitsubishi144_getRaw(void);
void Mitsubishi144_setRaw(const uint8_t* data);
uint8_t Mitsubishi144_getClock(void);
void Mitsubishi144_setClock(const uint8_t clock);
uint8_t Mitsubishi144_getStartClock(void);
void Mitsubishi144_setStartClock(const uint8_t clock);
uint8_t Mitsubishi144_getStopClock(void);
void Mitsubishi144_setStopClock(const uint8_t clock);
uint8_t Mitsubishi144_getTimer(void);
void Mitsubishi144_setTimer(const uint8_t timer);
bool Mitsubishi144_getWeeklyTimerEnabled(void);
void Mitsubishi144_setWeeklyTimerEnabled(const bool on);
static uint8_t Mitsubishi144_convertMode(const opmode_t mode);
static uint8_t Mitsubishi144_convertFan(const fanspeed_t speed);
static uint8_t Mitsubishi144_convertSwingV(const swingv_t position);
static uint8_t Mitsubishi144_convertSwingH(const swingh_t position);
static opmode_t Mitsubishi144_toCommonMode(const uint8_t mode);
static fanspeed_t Mitsubishi144_toCommonFanSpeed(const uint8_t speed);
static swingv_t Mitsubishi144_toCommonSwingV(const uint8_t pos);
static swingh_t Mitsubishi144_toCommonSwingH(const uint8_t pos);

void Mitsubishi144_checksum(void);
static bool Mitsubishi144_validChecksum(const uint8_t* data);
static uint8_t Mitsubishi144_calculateChecksum(const uint8_t* data);


#endif  // IR_MITSUBISHI_H_
