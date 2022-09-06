/// @file
/// @brief Support for Samsung protocols.
/// @see http://elektrolab.wz.cz/katalog/samsung_protocol.pdf

// Supports:
//   Brand: Samsung,  Model: UA55H6300 TV (SAMSUNG);
//   Brand: Samsung,  Model: BN59-01178B TV remote (SAMSUNG);
//   Brand: Samsung,  Model: DB63-03556X003 remote
//   Brand: Samsung,  Model: DB93-16761C remote
//   Brand: Samsung,  Model: IEC-R03 remote
//   Brand: Samsung,  Model: AK59-00167A Bluray remote (SAMSUNG36);
//   Brand: Samsung,  Model: AH59-02692E Soundbar remote (SAMSUNG36);
//   Brand: Samsung,  Model: HW-J551 Soundbar (SAMSUNG36);
//   Brand: Samsung,  Model: AR09FSSDAWKNFA A/C (SAMSUNG_AC);
//   Brand: Samsung,  Model: AR09HSFSBWKN A/C (SAMSUNG_AC);
//   Brand: Samsung,  Model: AR12KSFPEWQNET A/C (SAMSUNG_AC);
//   Brand: Samsung,  Model: AR12HSSDBWKNEU A/C (SAMSUNG_AC);
//   Brand: Samsung,  Model: AR12NXCXAWKXEU A/C (SAMSUNG_AC);
//   Brand: Samsung,  Model: AR12TXEAAWKNEU A/C (SAMSUNG_AC);
//   Brand: Samsung,  Model: DB93-14195A remote (SAMSUNG_AC);
//   Brand: Samsung,  Model: DB96-24901C remote (SAMSUNG_AC);

#ifndef IR_SAMSUNG_H_
#define IR_SAMSUNG_H_

#include <stdint.h>
#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"


extern bool g_samsungIsPowerTriggerred;

/// Native representation of a Samsung A/C message.
union SamsungProtocol{
  uint8_t raw[kSamsungAcExtendedStateLength];  ///< State in code form.
  struct {  // Standard message map
    // Byte 0
    uint8_t         :8;
    // Byte 1
    uint8_t         :4;
    uint8_t         :4;  // Sum1Lower
    // Byte 2
    uint8_t         :4;  // Sum1Upper
    uint8_t         :4;
    // Byte 3
    uint8_t         :8;
    // Byte 4
    uint8_t         :8;
    // Byte 5
    uint8_t         :4;
    uint8_t Sleep5  :1;
    uint8_t Quiet   :1;
    uint8_t         :2;
    // Byte 6
    uint8_t         :4;
    uint8_t Power1  :2;
    uint8_t         :2;
    // Byte 7
    uint8_t         :8;
    // Byte 8
    uint8_t         :4;
    uint8_t         :4;  // Sum2Lower
    // Byte 9
    uint8_t         :4;  // Sum1Upper
    uint8_t Swing   :3;
    uint8_t         :1;
    // Byte 10
    uint8_t               :1;
    uint8_t FanSpecial    :3;  // Powerful, Breeze/WindFree, Econo
    uint8_t Display       :1;
    uint8_t               :2;
    uint8_t CleanToggle10 :1;
    // Byte 11
    uint8_t Ion           :1;
    uint8_t CleanToggle11 :1;
    uint8_t               :2;
    uint8_t Temp          :4;
    // Byte 12
    uint8_t       :1;
    uint8_t Fan   :3;
    uint8_t Mode  :3;
    uint8_t       :1;
    // Byte 13
    uint8_t            :2;
    uint8_t BeepToggle :1;
    uint8_t            :1;
    uint8_t Power2     :2;
    uint8_t            :2;
  };
  struct {  // Extended message map
    // 1st Section
    // Byte 0
    uint8_t                :8;
    // Byte 1
    uint8_t                :4;
    uint8_t Sum1Lower      :4;
    // Byte 2
    uint8_t Sum1Upper      :4;
    uint8_t                :4;
    // Byte 3
    uint8_t                :8;
    // Byte 4
    uint8_t                :8;
    // Byte 5
    uint8_t                :8;
    // Byte 6
    uint8_t                :8;
    // 2nd Section
    // Byte 7
    uint8_t                :8;
    // Byte 8
    uint8_t                :4;
    uint8_t Sum2Lower      :4;
    // Byte 9
    uint8_t Sum2Upper      :4;
    uint8_t OffTimeMins    :3;  // In units of 10's of mins
    uint8_t OffTimeHrs1    :1;  // LSB of the number of hours.
    // Byte 10
    uint8_t OffTimeHrs2    :4;  // MSBs of the number of hours.
    uint8_t OnTimeMins     :3;  // In units of 10's of mins
    uint8_t OnTimeHrs1     :1;  // LSB of the number of hours.
    // Byte 11
    uint8_t OnTimeHrs2     :4;  // MSBs of the number of hours.
    uint8_t                :4;
    // Byte 12
    uint8_t OffTimeDay     :1;
    uint8_t OnTimerEnable  :1;
    uint8_t OffTimerEnable :1;
    uint8_t Sleep12        :1;
    uint8_t OnTimeDay      :1;
    uint8_t                :3;
    // Byte 13
    uint8_t                :8;
    // 3rd Section
    // Byte 14
    uint8_t                :8;
    // Byte 15
    uint8_t                :4;
    uint8_t Sum3Lower      :4;
    // Byte 16
    uint8_t Sum3Upper      :4;
    uint8_t                :4;
    // Byte 17
    uint8_t                :8;
    // Byte 18
    uint8_t                :8;
    // Byte 19
    uint8_t                :8;
    // Byte 20
    uint8_t                :8;
  };
};

// Constants
#define kSamsungAcPowerOn               3
#define kSamsungAcPowerOff              0
#define kSamsungAcMinTemp               0    // 16C   Mask 0b11110000
#define kSamsungAcMaxTemp               0xE  // 30C   Mask 0b11110000
#define kSamsungAcAutoTemp              9    // 25C   Mask 0b11110000
#define kSamsungAcAuto                  0
#define kSamsungAcCool                  1
#define kSamsungAcDry                   2
#define kSamsungAcFan                   3
#define kSamsungAcHeat                  4
#define kSamsungAcFanAuto               0
#define kSamsungAcFanLow                2
#define kSamsungAcFanMed                4
#define kSamsungAcFanHigh               5
#define kSamsungAcFanAuto2              6
#define kSamsungAcFanTurbo              7
#define kSamsungAcSectionLength         7
#define kSamsungAcPowerSection          0x1D20F00000000

// Classes
/// Class for handling detailed Samsung A/C messages.
void encode_Samsung(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Samsung(int16_t* input, uint8_t* output);

void Samsung_send(const uint8_t data[], const uint16_t nbytes,
                  const uint16_t repeat, int16_t *irRaw);
bool Samsung_recv(decode_results *results, uint16_t offset,
                  const uint16_t nbits, const bool strict);

bool isSamsung(int16_t *irRaw);

// void Samsung_send(const uint16_t repeat);
void Samsung_sendExtended(const uint16_t repeat, int16_t *irRaw);
void Samsung_sendOn(const uint16_t repeat, int16_t *irRaw);
void Samsung_sendOff(const uint16_t repeat, int16_t *irRaw);
void Samsung_setPower(const bool on);
bool Samsung_getPower(void);
void Samsung_setTemp(const uint8_t temp);
uint8_t Samsung_getTemp(void);
void    Samsung_setFan(const uint8_t speed);
uint8_t Samsung_getFan(void);
void    Samsung_setMode(const uint8_t mode);
uint8_t Samsung_getMode(void);
void Samsung_setSwing(const bool on);
bool Samsung_getSwing(void);
void Samsung_setSwingH(const bool on);
bool Samsung_getSwingH(void);
void Samsung_setBeep(const bool on);
bool Samsung_getBeep(void);
void Samsung_setClean(const bool on);
bool Samsung_getClean(void);
void Samsung_setQuiet(const bool on);
bool Samsung_getQuiet(void);
void Samsung_setPowerful(const bool on);
bool Samsung_getPowerful(void);
void Samsung_setBreeze(const bool on);
bool Samsung_getBreeze(void);
void Samsung_setEcono(const bool on);
bool Samsung_getEcono(void);
void Samsung_setDisplay(const bool on);
bool Samsung_getDisplay(void);
void Samsung_setIon(const bool on);
bool Samsung_getIon(void);
uint16_t Samsung_getOnTimer(void);
void     Samsung_setOnTimer(const uint16_t nr_of_mins);
uint16_t Samsung_getOffTimer(void);
void     Samsung_setOffTimer(const uint16_t nr_of_mins);
uint16_t Samsung_getSleepTimer(void);
void     Samsung_setSleepTimer(const uint16_t nr_of_mins);
uint8_t* Samsung_getRaw(void);
void     Samsung_setRaw(const uint8_t new_code[], const uint16_t length);
uint8_t  Samsung_calcSectionChecksum(const uint8_t *section);
uint8_t  Samsung_getSectionChecksum(const uint8_t *section);
bool     Samsung_validChecksum(const uint8_t state[], const uint16_t length );
uint8_t  Samsung_convertMode(const opmode_t mode);
uint8_t  Samsung_convertFan(const fanspeed_t speed);
opmode_t   Samsung_toCommonMode(const uint8_t mode);
fanspeed_t Samsung_toCommonFanSpeed(const uint8_t speed);

state_t Samsung_toCommon(void);
void Samsung_checksum(void);

uint16_t _Samsung_getOnTimer(void);
uint16_t _Samsung_getOffTimer(void);
void     _Samsung_setOnTimer(void);
void     _Samsung_setOffTimer(void);
void     _Samsung_setSleepTimer(void);

#endif  // IR_SAMSUNG_H_
