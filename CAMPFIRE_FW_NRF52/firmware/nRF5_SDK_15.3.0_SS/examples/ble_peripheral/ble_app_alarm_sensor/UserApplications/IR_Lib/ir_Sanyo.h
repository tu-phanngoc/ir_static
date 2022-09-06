
// Supports:
//   Brand: Sanyo,  Model: SA 8650B - disabled
//   Brand: Sanyo,  Model: LC7461 transmitter IC (SANYO_LC7461)
//   Brand: Sanyo,  Model: SAP-K121AHA A/C (SANYO_AC)
//   Brand: Sanyo,  Model: RCS-2HS4E remote (SANYO_AC)
//   Brand: Sanyo,  Model: SAP-K242AH A/C (SANYO_AC)
//   Brand: Sanyo,  Model: RCS-2S4E remote (SANYO_AC)




#ifndef IR_SANYO_H_
#define IR_SANYO_H_

	 
#include "stdint.h"
#include "stdbool.h"
#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"

/// Native representation of a Sanyo A/C message.
typedef union{
  uint8_t raw[kSanyoAcStateLength];  ///< The state in IR code form.
  struct {
    // Byte 0
    uint8_t Address :8;  // 0x6A (Fixed?)
    // Byte 1
    uint8_t Temp :5;
    uint8_t rev0100 :3;
    // Byte 2
    uint8_t SensorTemp :5;
    uint8_t Sensor     :1;  ///< Sensor location (0 = remote, 1 = A/C)
    uint8_t Beep       :1;
    uint8_t rev0200    :1;
    // Byte 3
    uint8_t OffHour :4;
    uint8_t rev0300 :4;
    // Byte 4
    uint8_t Fan      :2;
    uint8_t OffTimer :1;
    uint8_t rev0400  :1;
    uint8_t Mode     :3;
    uint8_t rev0401  :1;
    // Byte 5
    uint8_t SwingV :3;
    uint8_t rev0500:3;
    uint8_t Power  :2;
    // Byte 6
    uint8_t rev0600:3;
    uint8_t Sleep  :1;
    uint8_t rev0601:4;
    // Byte 7
    uint8_t rev0700:8;
    // Byte 8
    uint8_t Sum :8;
  } frame;
} SanyoProtocol_t;




// Constants

#define DATASET_MAX_INDEX_SANYO         147

#define kSanyoAcTempMin          16    ///< Celsius
#define kSanyoAcTempMax          30    ///< Celsius
#define kSanyoAcTempDelta          4   ///< Celsius to Native Temp difference.

#define kSanyoAcHourMax          15       ///<          0b1111

#define kSanyoAcHeat          1        ///<       0b001
#define kSanyoAcCool          2        ///<       0b010
#define kSanyoAcDry          3        ///<       0b011
#define kSanyoAcAuto          4        ///<       0b100
#define kSanyoAcFanAuto          0   ///<            0b00
#define kSanyoAcFanHigh          1   ///<            0b01
#define kSanyoAcFanLow          2   ///<            0b10
#define kSanyoAcFanMedium          3   ///<            0b11

// const uint8_t kSanyoAcPowerStandby =           0b00;  ///< Standby?
#define kSanyoAcPowerOff    1  ///< Off
#define kSanyoAcPowerOn     2  ///< On
#define kSanyoAcSwingVAuto          0  ///<     0b000
#define kSanyoAcSwingVLowest          2  ///<     0b010
#define kSanyoAcSwingVLow          3  ///<     0b011
#define kSanyoAcSwingVLowerMiddle          4  ///<     0b100
#define kSanyoAcSwingVUpperMiddle          5  ///<     0b101
#define kSanyoAcSwingVHigh          6  ///<     0b110
#define kSanyoAcSwingVHighest          7  ///<     0b111


void encode_Sanyo(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Sanyo(int16_t* input, uint8_t* output);

void Sanyo_send(const uint8_t data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irraw);
bool Sanyo_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict); 

void Sanyo_begin(void);
void Sanyo_on(void);
void Sanyo_off(void);
void Sanyo_setPower(const bool on);
bool Sanyo_getPower(void);
void Sanyo_setTemp(const uint8_t degrees);
uint8_t Sanyo_getTemp(void);
void Sanyo_setSensorTemp(const uint8_t degrees);
uint8_t Sanyo_getSensorTemp(void);
void Sanyo_setFan(const uint8_t speed);
uint8_t Sanyo_getFan(void);
void Sanyo_setMode(const uint8_t mode);
uint8_t Sanyo_getMode(void);
void Sanyo_setSleep(const bool on);
bool Sanyo_getSleep(void);
void Sanyo_setSensor(const bool location);
bool Sanyo_getSensor(void);
void Sanyo_setBeep(const bool on);
bool Sanyo_getBeep(void);
void Sanyo_setSwingV(const uint8_t setting);
uint8_t Sanyo_getSwingV(void);
void Sanyo_setRaw(const uint8_t newState[]);
uint8_t* Sanyo_getRaw(void);
uint16_t Sanyo_getOffTimer(void);
void Sanyo_setOffTimer(const uint16_t mins);
static uint8_t Sanyo_convertFan(const fanspeed_t speed);
static uint8_t Sanyo_convertSwingV(const swingv_t position);
static uint8_t Sanyo_convertMode(const opmode_t mode);
static opmode_t Sanyo_toCommonMode(const uint8_t mode);
static fanspeed_t Sanyo_toCommonFanSpeed(const uint8_t speed);
static swingv_t Sanyo_toCommonSwingV(const uint8_t setting);
state_t Sanyo_toCommon(void);

void Sanyo_checksum(void);
static bool Sanyo_validChecksum(const uint8_t state[], const uint16_t length);
static uint8_t Sanyo_calcChecksum(const uint8_t state[], const uint16_t length);



#ifdef SANYO_AC8

#define kSanyoAc88Auto          0  ///< 0b000
#define kSanyoAc88FeelCool          1  ///< 0b001
#define kSanyoAc88Cool          2  ///< 0b010
#define kSanyoAc88FeelHeat          3  ///< 0b011
#define kSanyoAc88Heat          4  ///< 0b100
#define kSanyoAc88Fan          5  ///< 0b101

#define kSanyoAc88TempMin          10  ///< Celsius
#define kSanyoAc88TempMax          30  ///< Celsius

#define kSanyoAc88FanAuto          0  ///< 0b00
#define kSanyoAc88FanLow          1  ///< 0b11
#define kSanyoAc88FanMedium          2  ///< 0b10
#define kSanyoAc88FanHigh          3  ///< 0b11

/// Native representation of a Sanyo 88-bit A/C message.
union SanyoAc88Protocol{
  uint8_t raw[kSanyoAc88StateLength];  ///< The state in IR code form.
  struct {
    // Byte 0-1
    uint8_t                  :8;  // 0xAA (Fixed?)
    uint8_t                  :8;  // 0x55 (Fixed?)
    // Byte 2
    uint8_t Fan              :2;
    uint8_t                  :2;
    uint8_t Mode             :3;
    uint8_t Power            :1;
    // Byte 3
    uint8_t Temp             :5;
    uint8_t Filter           :1;
    uint8_t SwingV           :1;
    uint8_t                  :1;
    // Byte 4
    uint8_t ClockSecs        :8;  // Nr. of Seconds
    // Byte 5
    uint8_t ClockMins        :8;  // Nr. of Minutes
    // Byte 6
    uint8_t ClockHrs         :8;  // Nr. of Hours
    // Byte 7-9  (Timer times?)
    uint8_t                  :8;
    uint8_t                  :8;
    uint8_t                  :8;
    // Byte 10
    uint8_t                  :3;
    uint8_t Turbo            :1;
    uint8_t EnableStartTimer :1;
    uint8_t EnableStopTimer  :1;
    uint8_t Sleep            :1;
    uint8_t                  :1;
  } frame;
};

/// Handling detailed Sanyo A/C messages.
  void Sanyo88_begin(void);
  void Sanyo88_on(void);
  void Sanyo88_off(void);
  void Sanyo88_setPower(const bool on);
  bool Sanyo88_getPower(void);
  void Sanyo88_setTemp(const uint8_t degrees);
  uint8_t Sanyo88_getTemp(void);
  void Sanyo88_setFan(const uint8_t speed);
  uint8_t Sanyo88_getFan(void);
  void Sanyo88_setMode(const uint8_t mode);
  uint8_t Sanyo88_getMode(void);
  void Sanyo88_setSleep(const bool on);
  bool Sanyo88_getSleep(void);
  void Sanyo88_setTurbo(const bool on);
  bool Sanyo88_getTurbo(void);
  void Sanyo88_setFilter(const bool on);
  bool Sanyo88_getFilter(void);
  void Sanyo88_setSwingV(const bool on);
  bool Sanyo88_getSwingV(void);
  uint16_t Sanyo88_getClock(void);
  void Sanyo88_setClock(const uint16_t mins_since_midnight);
  void Sanyo88_setRaw(const uint8_t newState[]);
  uint8_t* Sanyo88_getRaw(void);

  void Sanyo88_checksum(void);
  static uint8_t Sanyo88_calcChecksum(const uint8_t state[],const uint16_t length);
#endif // #ifdef SANYO_AC8

#endif  // IR_SANYO_H_
