
/// @file
/// @brief Support for Daikin A/C protocols.

#ifndef IR_DAIKIN_H_
#define IR_DAIKIN_H_

#include "IRrecv.h"
#include "IRsend.h"
#include "IRcommon.h"


/// Native representation of a Daikin A/C message.
typedef union DaikinESPProtocol_t{
  uint8_t raw[kDaikinStateLength];  ///< The state of the IR remote.
  struct {
    // Byte 0~5
    uint8_t rev0[6];
    // Byte 6
    uint8_t rev060   :4;
    uint8_t Comfort  :1;
    uint8_t rev061   :3;
    // Byte 7
    uint8_t Sum1     :8;  // checksum of the first part

    // Byte 8~12
    uint8_t rev1[5];
    // Byte 13~14
    uint8_t CurrentTime0 :8;  // Current time, mins past midnight
    uint8_t CurrentDay   :3;  // Day of the week (SUN=1, MON=2, ..., SAT=7)
    uint8_t rev14        :2;
    uint8_t CurrentTime1 :3;  // Current time, mins past midnight
    // Byte 15
    uint8_t Sum2         :8;  // checksum of the second part

    // Byte 16~20
    uint8_t rev2[5];
    // Byte 21
    uint8_t Power    :1;
    uint8_t OnTimer  :1;
    uint8_t OffTimer :1;
    uint8_t rev210   :1;  // always 1
    uint8_t Mode     :3;
    uint8_t rev211   :1;
    // Byte 22
    uint8_t rev22    :1;
    uint8_t Temp     :7;  // Temp should be between 10 - 32
    // Byte 23
    uint8_t rev23   :8;

    // Byte 24
    uint8_t SwingV   :4;  // 0000 =  off, 1111 = on
    uint8_t Fan      :4;
    // Byte 25
    uint8_t SwingH   :4;  // 0000 =  off, 1111 = on
    uint8_t rev25    :4;
    // Byte 26~28
    // uint64_t OnTime   :12;  // timer mins past midnight
    // uint64_t OffTime  :12;  // timer mins past midnight
    uint8_t on_off_time[3];
    // Byte 29
    uint8_t Powerful :1;
    uint8_t rev290   :4;
    uint8_t Quiet    :1;
    uint8_t rev291   :2;
    // Byte 30~31
    uint8_t rev3[2];

    // Byte 32
    uint8_t rev320      :1;
    uint8_t Sensor      :1;
    uint8_t Econo       :1;
    uint8_t rev321      :4;
    uint8_t WeeklyTimer :1;
    // Byte 33
    uint8_t rev330:1;
    uint8_t Mold  :1;
    uint8_t rev331:6;
    // Byte 34
    uint8_t Sum3  :8;  // checksum of the third part
  };
} DaikinESPProtocol_t;

// Constants
#define kDaikinAuto                   0  // temp 25
#define kDaikinDry                    2  // temp 0xc0 = 96 degrees c
#define kDaikinCool                   3
#define kDaikinHeat                   4  // temp 23
#define kDaikinFan                    6  // temp not shown, but 25
#define kDaikinMinTemp                10  // Celsius
#define kDaikinMaxTemp                32  // Celsius
#define kDaikinFanMin                 1
#define kDaikinFanMed                 3
#define kDaikinFanMax                 5
#define kDaikinFanAuto                10  // 10 / 0xA
#define kDaikinFanQuiet               11  // 11 / 0xB
#define kDaikinSwingOn                15
#define kDaikinSwingOff               0
#define kDaikinHeaderLength           5
#define kDaikinSections               3
#define kDaikinSection1Length         8
#define kDaikinSection2Length         8
#define kDaikinSection3Length         kDaikinStateLength - kDaikinSection1Length - kDaikinSection2Length
#define kDaikinByteChecksum1          7
#define kDaikinByteChecksum2          15
// const uint8_t kDaikinBitEye = 0b10000000;
#define kDaikinUnusedTime             0x600
#define kDaikinBeepQuiet              1
#define kDaikinBeepLoud               2
#define kDaikinBeepOff                3
#define kDaikinLightBright            1
#define kDaikinLightDim               2
#define kDaikinLightOff               3
#define kDaikinCurBit                 kDaikinStateLength
#define kDaikinCurIndex               kDaikinStateLength + 1
#define kDaikinTolerance              35
#define kDaikinMarkExcess             kMarkExcess
#define kDaikinHdrMark                3650   // kDaikinBitMark * 8
#define kDaikinHdrSpace               1623  // kDaikinBitMark * 4
#define kDaikinBitMark                428
#define kDaikinZeroSpace              428
#define kDaikinOneSpace               1280
#define kDaikinGap                    29000
// Note bits in each octet swapped so can be sent as a single value
#define kDaikinFirstHeader64          0xD70000C50027DA11

/// Native representation of a Daikin2 A/C message.
typedef union Daikin2Protocol{
  struct{
    uint8_t pad[3];
    uint8_t raw[kDaikin2StateLength];  ///< The state of the IR remote.
  };
  struct {
    // Byte -3~4
    uint8_t rev0 [8]; //:64;

    // Byte 5~6
    uint8_t CurrentTime1 :8;
    uint8_t rev06        :3;
    uint8_t Power2       :1;
    uint8_t CurrentTime2 :4;
    // Byte 7
    uint8_t rev07        :4;
    uint8_t Light        :2;
    uint8_t Beep         :2;
    // Byte 8
    uint8_t FreshAir     :1;
    uint8_t rev080       :2;
    uint8_t Mold         :1;
    uint8_t rev081       :1;
    uint8_t Clean        :1;
    uint8_t rev082       :1;
    uint8_t FreshAirHigh :1;
    // Byte 9~12
    uint8_t rev1[4];

    // Byte 13
    uint8_t rev13    :7;
    uint8_t EyeAuto  :1;
    // Byte 14~16
    uint8_t rev2[3];
    // Byte 17
    uint8_t SwingH   :8;
    // Byte 18
    uint8_t SwingV   :4;
    uint8_t rev180   :4;
    // Byte 19
    uint8_t Sum1     :8;
    // Byte 20
    uint8_t rev20    :8;

    // Byte 21~24
    uint8_t rev3[4];
    // Byte 25
    uint8_t Power    :1;
    uint8_t OnTimer  :1;
    uint8_t OffTimer :1;
    uint8_t rev250   :1;
    uint8_t Mode     :3;
    uint8_t rev251   :1;
    // Byte 26
    uint8_t rev26    :1;
    uint8_t Temp     :6;
    uint8_t HumidOn  :1;
    // Byte 27
    uint8_t Humidity :8;
    // Byte 28
    uint8_t rev280   :4;
    uint8_t Fan      :4;

    // Byte 29
    uint8_t rev29      :8;
    // Byte 30~32
    uint8_t OnTime0    :8;
    uint8_t OffTime0   :4;
    uint8_t OnTime1    :4;
    uint8_t OffTime1   :4;
    // Byte 33
    uint8_t Powerful   :1;
    uint8_t rev330     :4;
    uint8_t Quiet      :1;
    uint8_t rev331     :2;
    // Byte 34~35
    uint8_t rev4[2];
    // Byte 36
    uint8_t rev360     :1;
    uint8_t Eye        :1;
    uint8_t Econo      :1;
    uint8_t rev361     :1;
    uint8_t Purify     :1;
    uint8_t SleepTimer :1;
    uint8_t rev362     :2;

    // Byte 37
    uint8_t rev37 :8;
    // Byte 38
    uint8_t Sum2  :8;
  };
} Daikin2Protocol_t;

#define kDaikin2Freq                    36700  // Modulation Frequency in Hz.
#define kDaikin2LeaderMark              10024
#define kDaikin2LeaderSpace             25180
#define kDaikin2Gap                     kDaikin2LeaderMark + kDaikin2LeaderSpace
#define kDaikin2HdrMark                 3500
#define kDaikin2HdrSpace                1728
#define kDaikin2BitMark                 460
#define kDaikin2OneSpace                1270
#define kDaikin2ZeroSpace               420
#define kDaikin2Sections                2
#define kDaikin2Section1Length          20
#define kDaikin2Section2Length          19
#define kDaikin2Tolerance               5  // Extra percentage tolerance
#define kDaikin2SwingVHighest           0x1
#define kDaikin2SwingVHigh              0x2
#define kDaikin2SwingVUpperMiddle       0x3
#define kDaikin2SwingVLowerMiddle       0x4
#define kDaikin2SwingVLow               0x5
#define kDaikin2SwingVLowest            0x6
#define kDaikin2SwingVBreeze            0xC
#define kDaikin2SwingVCirculate         0xD
#define kDaikin2SwingVOff               0xE
#define kDaikin2SwingVAuto              0xF  // A.k.a "Swing"
#define kDaikin2SwingVSwing             kDaikin2SwingVAuto


#define kDaikin2SwingHWide            0xA3
#define kDaikin2SwingHLeftMax         0xA8
#define kDaikin2SwingHLeft            0xA9
#define kDaikin2SwingHMiddle          0xAA
#define kDaikin2SwingHRight           0xAB
#define kDaikin2SwingHRightMax        0xAC
#define kDaikin2SwingHAuto            0xBE  // A.k.a "Swing"
#define kDaikin2SwingHOff             0xBF
#define kDaikin2SwingHSwing           kDaikin2SwingHAuto

#define kDaikin2HumidityOff           0x00
#define kDaikin2HumidityHeatLow       0x28  // Humidify (Heat) only (40%?)
#define kDaikin2HumidityHeatMedium    0x2D  // Humidify (Heat) only (45%?)
#define kDaikin2HumidityHeatHigh      0x32  // Humidify (Heat) only (50%?)
#define kDaikin2HumidityDryLow        0x32  // Dry only (50%?)
#define kDaikin2HumidityDryMedium     0x37  // Dry only (55%?)
#define kDaikin2HumidityDryHigh       0x3C  // Dry only (60%?)
#define kDaikin2HumidityAuto          0xFF

#define kDaikin2MinCoolTemp          18  // Min temp (in C) when in Cool mode.

/// Native representation of a Daikin216 A/C message.
typedef union Daikin216Protocol{
  uint8_t raw[kDaikin216StateLength];  ///< The state of the IR remote.
  struct {
    // Byte 0~6
    uint8_t pad0[7];
    // Byte 7
    uint8_t Sum1  :8;
    // Byte 8~12
    uint8_t pad1[5];
    // Byte 13
    uint8_t Power :1;
    uint8_t rev130:3;
    uint8_t Mode  :3;
    uint8_t rev131:1;
    // Byte 14
    uint8_t rev140  :1;
    uint8_t Temp    :6;
    uint8_t rev141  :1;
    // Byte 15
    uint8_t rev15   :8;
    // Byte 16
    uint8_t SwingV  :4;
    uint8_t Fan     :4;
    // Byte 17
    uint8_t SwingH  :4;
    uint8_t rev17   :4;
    // Byte 18~20
    uint8_t pad2[3];
    // Byte 21
    uint8_t Powerful  :1;
    uint8_t rev21     :7;
    // Byte 22~25
    uint8_t pad3[4];
    // Byte 26
    uint8_t Sum2      :8;
  };
} Daikin216Protocol_t;

#define kDaikin216Freq                38000  // Modulation Frequency in Hz.
#define kDaikin216HdrMark             3440
#define kDaikin216HdrSpace            1750
#define kDaikin216BitMark             420
#define kDaikin216OneSpace            1300
#define kDaikin216ZeroSpace           450
#define kDaikin216Gap                 29650
#define kDaikin216Sections            2
#define kDaikin216Section1Length      8
#define kDaikin216Section2Length      kDaikin216StateLength - kDaikin216Section1Length

#define kDaikin216SwingOn             15
#define kDaikin216SwingOff            0

/// Native representation of a Daikin160 A/C message.
typedef union Daikin160Protocol{
  uint8_t raw[kDaikin160StateLength];  ///< The state of the IR remote.
  struct {
    // Byte 0~5
    uint8_t pad0[6];
    // Byte 6
    uint8_t Sum1 :8;
    // Byte 7~11
    uint8_t pad1[5];
    // Byte 12
    uint8_t Power   :1;
    uint8_t rev1    :3;
    uint8_t Mode    :3;
    uint8_t rev2    :1;
    // Byte 13
    uint8_t rev3    :4;
    uint8_t SwingV  :4;
    // Byte 14~15
    uint8_t pad2[2];
    // Byte 16
    uint8_t rev4  :1;
    uint8_t Temp  :6;
    uint8_t rev5  :1;
    // Byte 17
    uint8_t Fan   :4;
    uint8_t rev6  :4;
    // Byte 18
    uint8_t rev7  :8;
    // Byte 19
    uint8_t Sum2  :8;
  };
} Daikin160Protocol_t;

#define kDaikin160Freq                38000  // Modulation Frequency in Hz.
#define kDaikin160HdrMark             5000
#define kDaikin160HdrSpace            2145
#define kDaikin160BitMark             342
#define kDaikin160OneSpace            1786
#define kDaikin160ZeroSpace           700
#define kDaikin160Gap                 29650
#define kDaikin160Sections            2
#define kDaikin160Section1Length      7
#define kDaikin160Section2Length      kDaikin160StateLength - kDaikin160Section1Length
#define kDaikin160SwingVLowest        0x1
#define kDaikin160SwingVLow           0x2
#define kDaikin160SwingVMiddle        0x3
#define kDaikin160SwingVHigh          0x4
#define kDaikin160SwingVHighest       0x5
#define kDaikin160SwingVAuto          0xF

/// Native representation of a Daikin176 A/C message.
typedef union Daikin176Protocol{
  uint8_t raw[kDaikin176StateLength];  ///< The state of the IR remote.
  struct {
    // Byte 0~2
    uint8_t rev00 :8;
    uint8_t rev01 :8;
    uint8_t rev02 :8;
    // Byte 3
    uint8_t Id1   :1;
    uint8_t rev03 :7;
    // Byte 4
    uint8_t rev04:8;
    // Byte 5
    uint8_t rev05:8;
    // Byte 6
    uint8_t Sum1 :8;
    // Byte 7-9
    uint8_t rev07:8;
    uint8_t rev08:8;
    uint8_t rev09:8;
    // Byte 10
    uint8_t Id2  :1;
    uint8_t rev10:7;
    // Byte 11
    uint8_t rev11:8;
    // Byte 12
    uint8_t rev120 :4;
    uint8_t AltMode :3;
    uint8_t rev121 :1;
    // Byte 13
    uint8_t ModeButton  :8;
    // Byte 14
    uint8_t Power :1;
    uint8_t rev140:3;
    uint8_t Mode  :3;
    uint8_t rev141:1;
    // Byte 15~16
    uint8_t pad2[2];
    // Byte 17
    uint8_t rev170:1;
    uint8_t Temp  :6;
    uint8_t rev171:1;
    // Byte 18
    uint8_t SwingH  :4;
    uint8_t Fan     :4;
    // Byte 19~20
    uint8_t pad3[2];
    // Byte 21
    uint8_t Sum2  :8;
  };
} Daikin176Protocol_t;

#define kDaikin176Freq                38000  // Modulation Frequency in Hz.
#define kDaikin176HdrMark             5070
#define kDaikin176HdrSpace            2140
#define kDaikin176BitMark             370
#define kDaikin176OneSpace            1780
#define kDaikin176ZeroSpace           710
#define kDaikin176Gap                 29410
#define kDaikin176Sections            2
#define kDaikin176Section1Length      7
#define kDaikin176Section2Length      kDaikin176StateLength - kDaikin176Section1Length
#define kDaikin176Fan                 0
#define kDaikin176Heat                1
#define kDaikin176Cool                2
#define kDaikin176Auto                3
#define kDaikin176Dry                 7
#define kDaikin176ModeButton          4
#define kDaikin176DryFanTemp          17  // Dry/Fan mode is always 17 Celsius.
#define kDaikin176FanMax              3
#define kDaikin176SwingHAuto          0x5
#define kDaikin176SwingHOff           0x6

/// Native representation of a Daikin128 A/C message.
typedef union Daikin128Protocol{
  uint8_t raw[kDaikin128StateLength];  ///< The state of the IR remote.
  struct {
    // Byte 0
    uint8_t pad0  :8;
    // Byte 1
    uint8_t Mode  :4;
    uint8_t Fan   :4;
    // Byte 2
    uint8_t ClockMins   :8;
    // Byte 3
    uint8_t ClockHours  :8;
    // Byte 4
    uint8_t OnHours     :6;
    uint8_t OnHalfHour  :1;
    uint8_t OnTimer     :1;
    // Byte 5
    uint8_t OffHours    :6;
    uint8_t OffHalfHour :1;
    uint8_t OffTimer    :1;
    // Byte 6
    uint8_t Temp    :8;
    // Byte 7
    uint8_t SwingV  :1;
    uint8_t Sleep   :1;
    uint8_t rev07   :1;  // always 1
    uint8_t Power   :1;
    uint8_t Sum1    :4;
    // Byte 8
    uint8_t rev08   :8;
    // Byte 9
    uint8_t Ceiling :1;
    uint8_t rev090  :1;
    uint8_t Econo   :1;
    uint8_t Wall    :1;
    uint8_t rev091  :4;
    // Byte 10~14
    uint8_t pad[5];
    // Byte 15
    uint8_t Sum2    :8;
  };
} Daikin128Protocol_t;

#define kDaikin128Freq                  38000  // Modulation Frequency in Hz.
#define kDaikin128LeaderMark            9800
#define kDaikin128LeaderSpace           9800
#define kDaikin128HdrMark               4600
#define kDaikin128HdrSpace              2500
#define kDaikin128BitMark               350
#define kDaikin128OneSpace              954
#define kDaikin128ZeroSpace             382
#define kDaikin128Gap                   20300
#define kDaikin128FooterMark            kDaikin128HdrMark
#define kDaikin128Sections              2
#define kDaikin128SectionLength         8
#define kDaikin128Dry                   1
#define kDaikin128Cool                  2
#define kDaikin128Fan                   4
#define kDaikin128Heat                  8
#define kDaikin128Auto                  10
#define kDaikin128FanAuto               1
#define kDaikin128FanHigh               2
#define kDaikin128FanMed                4
#define kDaikin128FanLow                8
#define kDaikin128FanPowerful           3
#define kDaikin128FanQuiet              9
#define kDaikin128MinTemp               16  // C
#define kDaikin128MaxTemp               30  // C
#define kDaikin128BitWall               8
#define kDaikin128BitCeiling            1

/// Native representation of a Daikin152 A/C message.
typedef union Daikin152Protocol{
  uint8_t raw[kDaikin152StateLength];  ///< The state of the IR remote.
  struct {
    // Byte 0~4
    uint8_t pad0[5];
    // Byte 5
    uint8_t Power :1;
    uint8_t rev050:3;
    uint8_t Mode  :3;
    uint8_t rev051:1;
    // Byte 6
    uint8_t rev06 :1;
    uint8_t Temp  :7;
    // Byte 7
    uint8_t rev07 :8;
    // Byte 8
    uint8_t SwingV  :4;
    uint8_t Fan     :4;
    // Byte 9~12
    uint8_t pad1[4];
    // Byte 13
    uint8_t Powerful  :1;
    uint8_t rev130    :4;
    uint8_t Quiet     :1;
    uint8_t rev131    :2;
    // Byte 14~15
    uint8_t pad2[2];
    // Byte 16
    uint8_t rev160  :1;
    uint8_t Comfort :1;
    uint8_t Econo   :1;
    uint8_t Sensor  :1;
    uint8_t rev161  :4;
    // Byte 17
    uint8_t rev17   :8;
    // Byte 18
    uint8_t Sum     :8;
  };
} Daikin152Protocol_t;

#define kDaikin152Freq              38000  // Modulation Frequency in Hz.
#define kDaikin152LeaderBits        5
#define kDaikin152HdrMark           3368
#define kDaikin152HdrSpace          1684
#define kDaikin152BitMark           421
#define kDaikin152OneSpace          1263
#define kDaikin152ZeroSpace         kDaikin152BitMark
#define kDaikin152Gap               25182

#define kDaikin152DryTemp           kDaikin2MinCoolTemp  // Celsius
#define kDaikin152FanTemp           0x60  // 96 Celsius

/// Native representation of a Daikin64 A/C message.
typedef union Daikin64Protocol{
  uint64_t raw;  ///< The state of the IR remote.
  struct {
    uint8_t rev0        :8;

    uint8_t Mode        :4;
    uint8_t Fan         :4;

    uint8_t ClockMins   :8;
    uint8_t ClockHours  :8;

    uint8_t OnHours     :6;
    uint8_t OnHalfHour  :1;
    uint8_t OnTimer     :1;

    uint8_t OffHours    :6;
    uint8_t OffHalfHour :1;
    uint8_t OffTimer    :1;

    uint8_t Temp        :8;

    uint8_t SwingV      :1;
    uint8_t Sleep       :1;
    uint8_t rev1        :1;
    uint8_t Power       :1;
    uint8_t Sum         :4;
  };
} Daikin64Protocol_t;

#define kDaikin64HdrMark              kDaikin128HdrMark
#define kDaikin64BitMark              kDaikin128BitMark
#define kDaikin64HdrSpace             kDaikin128HdrSpace
#define kDaikin64OneSpace             kDaikin128OneSpace
#define kDaikin64ZeroSpace            kDaikin128ZeroSpace
#define kDaikin64LdrMark              kDaikin128LeaderMark
#define kDaikin64Gap                  kDaikin128Gap
#define kDaikin64LdrSpace             kDaikin128LeaderSpace
#define kDaikin64Freq                 kDaikin128Freq  // Hz.
#define kDaikin64Overhead             9
#define kDaikin64ToleranceDelta       5  // +5%

#define kDaikin64KnownGoodState       0xEC24101000001216
#define kDaikin64Dry                  1
#define kDaikin64Cool                 2
#define kDaikin64Fan                  4
#define kDaikin64Heat                 8
#define kDaikin64FanAuto              1
#define kDaikin64FanLow               8
#define kDaikin64FanMed               4
#define kDaikin64FanHigh              2
#define kDaikin64FanQuiet             9
#define kDaikin64FanTurbo             3
#define kDaikin64MinTemp              16  // Celsius
#define kDaikin64MaxTemp              30  // Celsius
#define kDaikin64ChecksumOffset       60
#define kDaikin64ChecksumSize         4  // Mask 0b1111 << 59

// Legacy defines.
#define DAIKIN_COOL                       kDaikinCool
#define DAIKIN_HEAT                       kDaikinHeat
#define DAIKIN_FAN                        kDaikinFan
#define DAIKIN_AUTO                       kDaikinAuto
#define DAIKIN_DRY                        kDaikinDry
#define DAIKIN_MIN_TEMP                   kDaikinMinTemp
#define DAIKIN_MAX_TEMP                   kDaikinMaxTemp
#define DAIKIN_FAN_MIN                    kDaikinFanMin
#define DAIKIN_FAN_MAX                    kDaikinFanMax
#define DAIKIN_FAN_AUTO                   kDaikinFanAuto
#define DAIKIN_FAN_QUIET                  kDaikinFanQuiet



/*
  Generic Function

*/

void setAcStatus(uint8_t* output);


/// Class for handling detailed Daikin 280-bit A/C messages.
void encode_DaikinESP(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_DaikinESP(int16_t* input, uint8_t* output);

void DaikinESP_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool DaikinESP_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);

bool     DaikinESP_isHeaderMatch(int16_t *rawbuf);
void     DaikinESP_setPower(const bool on);
bool     DaikinESP_getPower(void);
void     DaikinESP_setTemp(const uint8_t temp);
uint8_t  DaikinESP_getTemp(void);
void     DaikinESP_setFan(const uint8_t fan);
uint8_t  DaikinESP_getFan(void);
void     DaikinESP_setMode(const uint8_t mode);
uint8_t  DaikinESP_getMode(void);
void     DaikinESP_setSwingVertical(const bool on);
bool     DaikinESP_getSwingVertical(void);
void     DaikinESP_setSwingHorizontal(const bool on);
bool     DaikinESP_getSwingHorizontal(void);
bool     DaikinESP_getQuiet(void);
void     DaikinESP_setQuiet(const bool on);
bool     DaikinESP_getPowerful(void);
void     DaikinESP_setPowerful(const bool on);
void     DaikinESP_setSensor(const bool on);
bool     DaikinESP_getSensor(void);
void     DaikinESP_setEcono(const bool on);
bool     DaikinESP_getEcono(void);
void     DaikinESP_setMold(const bool on);
bool     DaikinESP_getMold(void);
void     DaikinESP_setComfort(const bool on);
bool     DaikinESP_getComfort(void);
void     DaikinESP_enableOnTimer(const uint16_t starttime);
void     DaikinESP_disableOnTimer(void);
uint16_t DaikinESP_getOnTime(void);
bool     DaikinESP_getOnTimerEnabled(void);
void     DaikinESP_enableOffTimer(const uint16_t endtime);
void     DaikinESP_disableOffTimer(void);
uint16_t DaikinESP_getOffTime(void);
bool     DaikinESP_getOffTimerEnabled(void);
void     DaikinESP_setCurrentTime(const uint16_t mins_since_midnight);
uint16_t DaikinESP_getCurrentTime(void);
void     DaikinESP_setCurrentDay(const uint8_t day_of_week);
uint8_t  DaikinESP_getCurrentDay(void);
void     DaikinESP_setWeeklyTimerEnable(const bool on);
bool     DaikinESP_getWeeklyTimerEnable(void);
uint8_t* DaikinESP_getRaw(void);
void     DaikinESP_setRaw(const uint8_t new_code[], const uint16_t length);

static bool       DaikinESP_validChecksum(uint8_t state[], const uint16_t length);
static uint8_t    DaikinESP_convertMode(const opmode_t mode);
static uint8_t    DaikinESP_convertFan(const fanspeed_t speed);
static opmode_t   DaikinESP_toCommonMode(const uint8_t mode);
static fanspeed_t DaikinESP_toCommonFanSpeed(const uint8_t speed);

void DaikinESP_stateReset(void);
void DaikinESP_checksum(void);




/// Class for handling detailed Daikin 312-bit A/C messages.
void encode_Daikin2(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Daikin2(int16_t* input, uint8_t* output);
void Daikin2_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool Daikin2_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);

void Daikin2_setPower(const bool state);
bool Daikin2_getPower(void);
void Daikin2_setTemp(const uint8_t temp);
uint8_t Daikin2_getTemp(void);
void Daikin2_setFan(const uint8_t fan);
uint8_t Daikin2_getFan(void);
uint8_t Daikin2_getMode(void);
void Daikin2_setMode(const uint8_t mode);
void Daikin2_setSwingVertical(const uint8_t position);
uint8_t Daikin2_getSwingVertical(void);
void Daikin2_setSwingHorizontal(const uint8_t position);
uint8_t Daikin2_getSwingHorizontal(void);
bool Daikin2_getQuiet(void);
void Daikin2_setQuiet(const bool on);
bool Daikin2_getPowerful(void);
void Daikin2_setPowerful(const bool on);
void Daikin2_setEcono(const bool on);
bool Daikin2_getEcono(void);
void Daikin2_setEye(const bool on);
bool Daikin2_getEye(void);
void Daikin2_setEyeAuto(const bool on);
bool Daikin2_getEyeAuto(void);
void Daikin2_setPurify(const bool on);
bool Daikin2_getPurify(void);
void Daikin2_setMold(const bool on);
bool Daikin2_getMold(void);
void Daikin2_enableOnTimer(const uint16_t starttime);
void Daikin2_disableOnTimer(void);
uint16_t Daikin2_getOnTime(void);
bool Daikin2_getOnTimerEnabled(void);
void Daikin2_enableSleepTimer(const uint16_t sleeptime);
void Daikin2_disableSleepTimer(void);
uint16_t Daikin2_getSleepTime(void);
bool Daikin2_getSleepTimerEnabled(void);
void Daikin2_enableOffTimer(const uint16_t endtime);
void Daikin2_disableOffTimer(void);
uint16_t Daikin2_getOffTime(void);
bool Daikin2_getOffTimerEnabled(void);
void Daikin2_setCurrentTime(const uint16_t time);
uint16_t Daikin2_getCurrentTime(void);
void Daikin2_setBeep(const uint8_t beep);
uint8_t Daikin2_getBeep(void);
void Daikin2_setLight(const uint8_t light);
uint8_t Daikin2_getLight(void);
void Daikin2_setClean(const bool on);
bool Daikin2_getClean(void);
void Daikin2_setFreshAir(const bool on);
bool Daikin2_getFreshAir(void);
void Daikin2_setFreshAirHigh(const bool on);
bool Daikin2_getFreshAirHigh(void);
uint8_t Daikin2_getHumidity(void);
void Daikin2_setHumidity(const uint8_t percent);
uint8_t* Daikin2_getRaw(void);
void Daikin2_setRaw(const uint8_t new_code[]);

void Daikin2_stateReset(void);
void Daikin2_checksum(void);
void Daikin2_clearOnTimerFlag(void);
void Daikin2_clearSleepTimerFlag(void);

// static
bool    Daikin2_validChecksum(uint8_t state[], const uint16_t length);
uint8_t Daikin2_convertMode(const opmode_t mode);
uint8_t Daikin2_convertFan(const fanspeed_t speed);
uint8_t Daikin2_convertSwingV(const swingv_t position);
uint8_t Daikin2_convertSwingH(const swingh_t position);
swingv_t Daikin2_toCommonSwingV(const uint8_t setting);
swingh_t Daikin2_toCommonSwingH(const uint8_t setting);
  



/// Class for handling detailed Daikin 216-bit A/C messages.
void encode_Daikin216(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Daikin216(int16_t* input, uint8_t* output);
bool Daikin216_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);
void Daikin216_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);

bool Daikin216_isHeaderMatch(int16_t *rawbuf);


uint8_t* Daikin216_getRaw(void);
void Daikin216_setRaw(const uint8_t new_code[]);
void Daikin216_on(void);
void Daikin216_off(void);
void Daikin216_setPower(const bool on);
bool Daikin216_getPower(void);
void Daikin216_setTemp(const uint8_t temp);
uint8_t Daikin216_getTemp(void);
void Daikin216_setMode(const uint8_t mode);
uint8_t Daikin216_getMode(void);
void Daikin216_setFan(const uint8_t fan);
uint8_t Daikin216_getFan(void);
void Daikin216_setSwingVertical(const bool on);
bool Daikin216_getSwingVertical(void);
void Daikin216_setSwingHorizontal(const bool on);
bool Daikin216_getSwingHorizontal(void);
void Daikin216_setQuiet(const bool on);
bool Daikin216_getQuiet(void);
void Daikin216_setPowerful(const bool on);
bool Daikin216_getPowerful(void);

void Daikin216_stateReset(void);
void Daikin216_checksum(void);

//static
uint8_t Daikin216_convertFan(const fanspeed_t speed);
uint8_t Daikin216_convertMode(const opmode_t mode);
bool Daikin216_validChecksum(uint8_t state[], const uint16_t length);
// # of bytes per command



/// Class for handling detailed Daikin 160-bit A/C messages.
void encode_Daikin160(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Daikin160(int16_t* input, uint8_t* output);
void Daikin160_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool Daikin160_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);


uint8_t* Daikin160_getRaw(void);
void Daikin160_setRaw(const uint8_t new_code[]);
void Daikin160_setPower(const bool on);
bool Daikin160_getPower(void);
void Daikin160_setTemp(const uint8_t temp);
uint8_t Daikin160_getTemp(void);
void Daikin160_setMode(const uint8_t mode);
uint8_t Daikin160_getMode(void);
void Daikin160_setFan(const uint8_t fan);
uint8_t Daikin160_getFan(void);
void Daikin160_setSwingVertical(const uint8_t position);
uint8_t Daikin160_getSwingVertical(void);
void Daikin160_stateReset(void);
void Daikin160_checksum(void);

// static
uint8_t Daikin160_convertFan(const fanspeed_t speed);
uint8_t Daikin160_convertMode(const opmode_t mode);
uint8_t Daikin160_convertSwingV(const swingv_t position);
swingv_t Daikin160_toCommonSwingV(const uint8_t setting);
bool Daikin160_validChecksum(uint8_t state[], const uint16_t length);






/// Class for handling detailed Daikin 176-bit A/C messages.
void encode_Daikin176(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Daikin176(int16_t* input, uint8_t* output);
void Daikin176_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool Daikin176_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);


uint8_t* Daikin176_getRaw(void);
void Daikin176_setRaw(const uint8_t new_code[]);
void Daikin176_setPower(const bool on);
bool Daikin176_getPower(void);
void Daikin176_setTemp(const uint8_t temp);
uint8_t Daikin176_getTemp(void);
void Daikin176_setMode(const uint8_t mode);
uint8_t Daikin176_getMode(void);
void Daikin176_setFan(const uint8_t fan);
uint8_t Daikin176_getFan(void);
void Daikin176_setSwingHorizontal(const uint8_t position);
uint8_t Daikin176_getSwingHorizontal(void);
uint8_t Daikin176_getId(void);
void Daikin176_setId(const uint8_t num);

void Daikin176_checksum(void);

void Daikin176_stateReset(void);

// static
static uint8_t Daikin176_convertFan(const fanspeed_t speed);
static uint8_t Daikin176_convertSwingH(const swingh_t position);
static uint8_t Daikin176_convertMode(const opmode_t mode);
static fanspeed_t Daikin176_toCommonFanSpeed(const uint8_t speed);
static opmode_t   Daikin176_toCommonMode(const uint8_t mode);
static swingh_t   Daikin176_toCommonSwingH(const uint8_t setting);
static bool Daikin176_validChecksum(uint8_t state[], const uint16_t length);






/// Class for handling detailed Daikin 128-bit A/C messages.
void encode_Daikin128(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Daikin128(int16_t* input, uint8_t* output);
void Daikin128_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool Daikin128_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);


void Daikin128_setPowerToggle(const bool toggle);
bool Daikin128_getPowerToggle(void);
void Daikin128_setTemp(const uint8_t temp);
uint8_t Daikin128_getTemp(void);
void Daikin128_setFan(const uint8_t fan);
uint8_t Daikin128_getFan(void);
uint8_t Daikin128_getMode(void);
void Daikin128_setMode(const uint8_t mode);
void Daikin128_setSwingVertical(const bool on);
bool Daikin128_getSwingVertical(void);
bool Daikin128_getSleep(void);
void Daikin128_setSleep(const bool on);
bool Daikin128_getQuiet(void);
void Daikin128_setQuiet(const bool on);
bool Daikin128_getPowerful(void);
void Daikin128_setPowerful(const bool on);
void Daikin128_setEcono(const bool on);
bool Daikin128_getEcono(void);
void Daikin128_setOnTimer(const uint16_t mins_since_midnight);
uint16_t Daikin128_getOnTimer(void);
bool Daikin128_getOnTimerEnabled(void);
void Daikin128_setOnTimerEnabled(const bool on);
void Daikin128_setOffTimer(const uint16_t mins_since_midnight);
uint16_t Daikin128_getOffTimer(void);
bool Daikin128_getOffTimerEnabled(void);
void Daikin128_setOffTimerEnabled(const bool on);
void Daikin128_setClock(const uint16_t mins_since_midnight);
uint16_t Daikin128_getClock(void);
void Daikin128_setLightToggle(const uint8_t unit_type);
uint8_t Daikin128_getLightToggle(void);
uint8_t* Daikin128_getRaw(void);
void Daikin128_setRaw(const uint8_t new_code[]);

void Daikin128_checksum(void);
void Daikin128_stateReset(void);
//static
bool Daikin128_validChecksum(uint8_t state[]);
uint8_t Daikin128_convertMode(const opmode_t mode);
uint8_t Daikin128_convertFan(const fanspeed_t speed);
opmode_t Daikin128_toCommonMode(const uint8_t mode);
fanspeed_t Daikin128_toCommonFanSpeed(const uint8_t speed);
uint8_t Daikin128_calcFirstChecksum(const uint8_t state[]);
uint8_t Daikin128_calcSecondChecksum(const uint8_t state[]);
  




/// Class for handling detailed Daikin 152-bit A/C messages.
void encode_Daikin152(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Daikin152(int16_t* input, uint8_t* output);
void Daikin152_send(const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);
bool Daikin152_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);

bool Daikin152_isHeaderMatch(int16_t *rawbuf);
uint8_t* Daikin152_getRaw(void);
void Daikin152_setRaw(const uint8_t new_code[]);
void Daikin152_setPower(const bool on);
bool Daikin152_getPower(void);
void Daikin152_setTemp(const uint8_t temp);
uint8_t Daikin152_getTemp(void);
void Daikin152_setFan(const uint8_t fan);
uint8_t Daikin152_getFan(void);
void Daikin152_setMode(const uint8_t mode);
uint8_t Daikin152_getMode(void);
void Daikin152_setSwingV(const bool on);
bool Daikin152_getSwingV(void);
bool Daikin152_getQuiet(void);
void Daikin152_setQuiet(const bool on);
bool Daikin152_getPowerful(void);
void Daikin152_setPowerful(const bool on);
void Daikin152_setSensor(const bool on);
bool Daikin152_getSensor(void);
void Daikin152_setEcono(const bool on);
bool Daikin152_getEcono(void);
void Daikin152_setComfort(const bool on);
bool Daikin152_getComfort(void);

void Daikin152_checksum(void);
void Daikin152_stateReset(void);
//static
static bool Daikin152_validChecksum(uint8_t state[], const uint16_t length);
static uint8_t Daikin152_convertMode(const opmode_t mode);
static uint8_t Daikin152_convertFan(const fanspeed_t speed);






/// Class for handling detailed Daikin 64-bit A/C messages.
void encode_Daikin64(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_Daikin64(int16_t* input, uint8_t* output);
void Daikin64_send(const uint64_t data, const uint16_t nbits, const uint16_t repeat, int16_t *irRaw);
bool Daikin64_recv(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict);


void Daikin64_begin(void);
uint64_t Daikin64_getRaw(void);
void Daikin64_setRaw(const uint64_t new_state);
void Daikin64_setPowerToggle(const bool on);
bool Daikin64_getPowerToggle(void);
void Daikin64_setTemp(const uint8_t temp);
uint8_t Daikin64_getTemp(void);
void Daikin64_setFan(const uint8_t fan);
uint8_t Daikin64_getFan(void);
void Daikin64_setMode(const uint8_t mode);
uint8_t Daikin64_getMode(void);
void Daikin64_setSwingVertical(const bool on);
bool Daikin64_getSwingVertical(void);
void Daikin64_setSleep(const bool on);
bool Daikin64_getSleep(void);
bool Daikin64_getQuiet(void);
void Daikin64_setQuiet(const bool on);
bool Daikin64_getTurbo(void);
void Daikin64_setTurbo(const bool on);
void Daikin64_setClock(const uint16_t mins_since_midnight);
uint16_t Daikin64_getClock(void);
void Daikin64_setOnTimeEnabled(const bool on);
bool Daikin64_getOnTimeEnabled(void);
void Daikin64_setOnTime(const uint16_t mins_since_midnight);
uint16_t Daikin64_getOnTime(void);
void Daikin64_setOffTimeEnabled(const bool on);
bool Daikin64_getOffTimeEnabled(void);
void Daikin64_setOffTime(const uint16_t mins_since_midnight);
uint16_t Daikin64_getOffTime(void);
void Daikin64_checksum(void);
//static
uint8_t Daikin64_convertMode(const opmode_t mode);
uint8_t Daikin64_convertFan(const fanspeed_t speed);
opmode_t Daikin64_toCommonMode(const uint8_t mode);
fanspeed_t Daikin64_toCommonFanSpeed(const uint8_t speed);
uint8_t Daikin64_calcChecksum(const uint64_t state);
bool Daikin64_validChecksum(const uint64_t state);


#endif

