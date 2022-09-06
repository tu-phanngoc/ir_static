
#ifndef IRCOMMON_H_
#define IRCOMMON_H_


#include "stdbool.h"
#include "stdint.h"
#include "string.h"


#pragma anon_unions

/// Enumerator for defining and numbering of supported IR protocol.
/// @note Always add to the end of the list and should never remove entries
///  or change order. Projects may save the type number for later usage
///  so numbering should always stay the same.

typedef enum {
  UNKNOWN = -1,
  UNUSED = 0,
  RC5,
  RC6,
  NEC,
  SONY,
  PANASONIC,  // (5)
  JVC,
  SAMSUNG,
  WHYNTER,
  AIWA_RC_T501,
  LG,  // (10)
  SANYO,
  MITSUBISHI,
  DISH,
  SHARP,
  COOLIX,  // (15)
  DAIKIN,
  DENON,
  KELVINATOR,
  SHERWOOD,
  MITSUBISHI_AC,  // (20)
  RCMM,
  SANYO_LC7461,
  RC5X,
  GREE,
  PRONTO,  // Technically not a protocol, but an encoding. (25)
  NEC_LIKE,
  ARGO,
  TROTEC,
  NIKAI,
  RAW,  // Technically not a protocol, but an encoding. (30)
  GLOBALCACHE,  // Technically not a protocol, but an encoding.
  TOSHIBA_AC,
  FUJITSU_AC,
  MIDEA,
  MAGIQUEST,  // (35)
  LASERTAG,
  CARRIER_AC,
  HAIER_AC,
  MITSUBISHI2,
  HITACHI_AC,  // (40)
  HITACHI_AC1,
  HITACHI_AC2,
  GICABLE,
  HAIER_AC_YRW02,
  WHIRLPOOL_AC,  // (45)
  SAMSUNG_AC,
  LUTRON,
  ELECTRA_AC,
  PANASONIC_AC,
  PIONEER,  // (50)
  LG2,
  MWM,
  DAIKIN2,
  VESTEL_AC,
  TECO,  // (55)
  SAMSUNG36,
  TCL112AC,
  LEGOPF,
  MITSUBISHI_HEAVY_88,
  MITSUBISHI_HEAVY_152,  // 60
  DAIKIN216,
  SHARP_AC,
  GOODWEATHER,
  INAX,
  DAIKIN160,  // 65
  NEOCLIMA,
  DAIKIN176,
  DAIKIN128,
  AMCOR,
  DAIKIN152,  // 70
  MITSUBISHI136,
  MITSUBISHI112,
  HITACHI_AC424,
  SONY_38K,
  EPSON,  // 75
  SYMPHONY,
  HITACHI_AC3,
  DAIKIN64,
  AIRWELL,
  DELONGHI_AC,  // 80
  DOSHISHA,
  MULTIBRACKETS,
  CARRIER_AC40,
  CARRIER_AC64,
  HITACHI_AC344,  // 85
  CORONA_AC,
  MIDEA24,
  ZEPEAL,
  SANYO_AC,
  VOLTAS,  // 90
  METZ,
  TRANSCOLD,
  TECHNIBEL_AC,
  MIRAGE,
  ELITESCREENS,  // 95
  PANASONIC_AC32,
  MILESTAG2,
  ECOCLIM,
  XMP,
  TRUMA,  // 100
  HAIER_AC176,
  TEKNOPOINT,
  KELON,
  TROTEC_3550,
  SANYO_AC88,  // 105
  BOSE,
  ARRIS,
  RHOSS,
  CASPER,
  CHIGO96AC,
  // Add new entries before this one, and update it to point to the last entry.
  kLastDecodeType = RHOSS,
} decode_type_t;
// Message lengths & required repeat values
#define kNoRepeat                     0
#define kSingleRepeat                 1
#define kAirwellBits                  34
#define kAirwellMinRepeats            2
#define kAiwaRcT501Bits               15
#define kAiwaRcT501MinRepeats         kSingleRepeat
#define kAlokaBits                    32
#define kAmcorStateLength             8
#define kAmcorBits        kAmcorStateLength * 8
#define kAmcorDefaultRepeat        kSingleRepeat
#define kArgoStateLength        12
#define kArgoBits        kArgoStateLength * 8
#define kArgoDefaultRepeat        kNoRepeat
#define kArrisBits        32

#define kCarrierAcBits        32
#define kCarrierAcMinRepeat        kNoRepeat
#define kCarrierAc40Bits        40
#define kCarrierAc40MinRepeat        2
#define kCarrierAc64Bits        64
#define kCarrierAc64MinRepeat        kNoRepeat
#define kCoronaAcStateLengthShort        7
#define kCoronaAcStateLength        kCoronaAcStateLengthShort * 3
#define kCoronaAcBitsShort        kCoronaAcStateLengthShort * 8
#define kCoronaAcBits        kCoronaAcStateLength * 8

#define kDelonghiAcBits        64
#define kDelonghiAcDefaultRepeat        kNoRepeat
#define kTechnibelAcBits        56
#define kTechnibelAcDefaultRepeat        kNoRepeat
#define kDenonBits        15
#define kDenon48Bits        48
#define kDenonLegacyBits        14
#define kDishBits        16
#define kDishMinRepeat        3
#define kDoshishaBits        40
#define kEcoclimBits        56
#define kEcoclimShortBits        15
#define kEpsonBits        32
#define kEpsonMinRepeat        2

#define kEliteScreensBits        32
#define kEliteScreensDefaultRepeat        kSingleRepeat
#define kFujitsuAcMinRepeat        kNoRepeat
#define kFujitsuAcStateLength        16
#define kFujitsuAcStateLengthShort        7
#define kFujitsuAcBits        kFujitsuAcStateLength * 8
#define kFujitsuAcMinBits        (kFujitsuAcStateLengthShort - 1) * 8
#define kGicableBits        16
#define kGicableMinRepeat        kSingleRepeat
#define kGoodweatherBits        48
#define kGoodweatherMinRepeat        kNoRepeat

#define kHaierACStateLength        9
#define kHaierACBits        kHaierACStateLength * 8
#define kHaierAcDefaultRepeat        kNoRepeat
#define kHaierACYRW02StateLength        14
#define kHaierACYRW02Bits        kHaierACYRW02StateLength * 8
#define kHaierAcYrw02DefaultRepeat        kNoRepeat
#define kHaierAC176StateLength        22
#define kHaierAC176Bits        kHaierAC176StateLength * 8
#define kHaierAc176DefaultRepeat        kNoRepeat
#define kHitachiAcStateLength        28
#define kHitachiAcBits        kHitachiAcStateLength * 8
#define kHitachiAcDefaultRepeat        kNoRepeat
#define kHitachiAc1StateLength        13
#define kHitachiAc1Bits        kHitachiAc1StateLength * 8
#define kHitachiAc2StateLength        53
#define kHitachiAc2Bits        kHitachiAc2StateLength * 8
#define kHitachiAc3StateLength        27
#define kHitachiAc3Bits        kHitachiAc3StateLength * 8
#define kHitachiAc3MinStateLength        15
#define kHitachiAc3MinBits        kHitachiAc3MinStateLength * 8
#define kHitachiAc344StateLength        43
#define kHitachiAc344Bits        kHitachiAc344StateLength * 8
#define kHitachiAc424StateLength        53
#define kHitachiAc424Bits        kHitachiAc424StateLength * 8
#define kInaxBits        24
#define kInaxMinRepeat        kSingleRepeat
#define kJvcBits        16
#define kKelonBits        48
#define kKelvinatorStateLength        16
#define kKelvinatorBits        kKelvinatorStateLength * 8
#define kKelvinatorDefaultRepeat        kNoRepeat
#define kLasertagBits        13
#define kLasertagMinRepeat        kNoRepeat
#define kLegoPfBits        16
#define kLegoPfMinRepeat        kNoRepeat



#define kLutronBits        35
#define kMagiquestBits        56
#define kMetzBits        19
#define kMetzMinRepeat        kNoRepeat
#define kMideaBits        48
#define kMideaMinRepeat        kNoRepeat
#define kMidea24Bits        24
#define kMidea24MinRepeat        kSingleRepeat
#define kMirageStateLength        15
#define kMirageBits        kMirageStateLength * 8
#define kMirageMinRepeat        kNoRepeat
#define kMitsubishiBits        16



// TODO(anyone): Verify that the Mitsubishi repeat is really needed.
//               Based on marcosamarinho's code.
#define kMitsubishiMinRepeat              kSingleRepeat
#define kMitsubishiACStateLength          18
#define kMitsubishiACBits                 kMitsubishiACStateLength * 8
#define kMitsubishiACMinRepeat            kSingleRepeat
#define kMitsubishi136StateLength         17
#define kMitsubishi136Bits                kMitsubishi136StateLength * 8
#define kMitsubishi136MinRepeat           kNoRepeat
#define kMitsubishi112StateLength         14
#define kMitsubishi112Bits                kMitsubishi112StateLength * 8
#define kMitsubishi112MinRepeat           kNoRepeat
#define kMitsubishi88StateLength          11
#define kMitsubishi88Bits                 kMitsubishi88StateLength * 8
#define kMitsubishi88MinRepeat            kNoRepeat
#define kMitsubishi152StateLength         19
#define kMitsubishi152Bits                kMitsubishi152StateLength * 8
#define kMitsubishi152MinRepeat           kNoRepeat

#define MITSUBISHI88_BITS                 179
#define MITSUBISHI152_BITS                307

/// Panasonic A/C model numbers
typedef enum panasonic_ac_remote_model_t {
  kPanasonicUnknown = 0,
  kPanasonicLke = 1,
  kPanasonicNke = 2,
  kPanasonicDke = 3,  // PKR too.
  kPanasonicJke = 4,
  kPanasonicCkp = 5,
  kPanasonicRkr = 6,
} panasonic_ac_remote_model_t;

#define kPanasonicFreq                    36700
#define kPanasonicAcExcess                0
#define kPanasonicAcTolerance             40
#define kPanasonicBits                    48
#define kPanasonicManufacturer            0x4004
#define kPanasonicAcStateLength           27
#define kPanasonicAcStateShortLength      16
#define kPanasonicAcBits                  kPanasonicAcStateLength * 8
#define kPanasonicAcShortBits             kPanasonicAcStateShortLength * 8
#define kPanasonicAcDefaultRepeat         kNoRepeat
#define kPanasonicAc32Bits                32

#define PANASONICAC_BITS                  439


/// Sharp A/C model numbers
typedef enum sharp_ac_remote_model_t {
  A907 = 1,
  A705 = 2,
  A903 = 3,  // 820 too
} sharp_ac_remote_model_t;

#define kSharpAddressBits                 5
#define kSharpCommandBits                 8
#define kSharpBits                        kSharpAddressBits + kSharpCommandBits + 2      // 15
#define kSharpAcStateLength               13
#define kSharpAcBits                      kSharpAcStateLength * 8      // 104
#define kSharpAcDefaultRepeat             kNoRepeat
#define SHARP_BITS                        211 // kSharpAcBits*2 + 2 + 1 =  2 for header, 1 for footer


/// LG A/C model numbers
typedef enum lg_ac_remote_model_t {
  GE6711AR2853M = 1,  // (1) LG 28-bit Protocol (default)
  AKB75215403,        // (2) LG2 28-bit Protocol
  AKB74955603,        // (3) LG2 28-bit Protocol variant
  AKB73757604,        // (4) LG2 Variant of AKB74955603
} lg_ac_remote_model_t;
#define kLgBits                       28
#define kLg32Bits                     32
#define kLgDefaultRepeat              kNoRepeat
#define LG_BITS                       kLgBits * 2 + 2 + 1
#define LG32_BITS                     kLg32Bits * 2 + 2 + 1


/// DAIKIN A/C model numbers
#define kDaikinStateLength            35
#define kDaikinBits                   kDaikinStateLength * 8
#define kDaikinStateLengthShort       kDaikinStateLength - 8
#define kDaikinBitsShort              kDaikinStateLengthShort * 8
#define kDaikinDefaultRepeat          kNoRepeat
#define kDaikin2StateLength           39
#define kDaikin2Bits                  kDaikin2StateLength * 8
#define kDaikin2DefaultRepeat         kNoRepeat
#define kDaikin64Bits                 64
#define kDaikin64DefaultRepeat        kNoRepeat
#define kDaikin160StateLength         20
#define kDaikin160Bits                kDaikin160StateLength * 8
#define kDaikin160DefaultRepeat       kNoRepeat
#define kDaikin128StateLength         16
#define kDaikin128Bits                kDaikin128StateLength * 8
#define kDaikin128DefaultRepeat       kNoRepeat
#define kDaikin152StateLength         19
#define kDaikin152Bits                kDaikin152StateLength * 8
#define kDaikin152DefaultRepeat       kNoRepeat
#define kDaikin176StateLength         22
#define kDaikin176Bits                kDaikin176StateLength * 8
#define kDaikin176DefaultRepeat       kNoRepeat
#define kDaikin216StateLength         27
#define kDaikin216Bits                kDaikin216StateLength * 8
#define kDaikin216DefaultRepeat       kNoRepeat
#define DAIKIN_BITS                   583
#define DAIKIN2_BITS                  633
#define DAIKIN216_BITS                439
#define DAIKIN160_BITS                327
#define DAIKIN176_BITS                359 // 2 + 2 + 7*8*2 + 4 + 15*8*2
#define DAIKIN128_BITS                265
#define DAIKIN152_BITS                319
#define DAIKIN64_BITS                 137

#define kElectraAcStateLength         13
#define kElectraAcBits                kElectraAcStateLength * 8
#define kElectraAcMinRepeat           kNoRepeat

#define kCasper104StateLength         13
#define kCasper104Bits                kCasper104StateLength * 8
#define kCasper104MinRepeat           kNoRepeat
#define kCasper343StateLength         21
#define kCasper343Bits                kWhirlpoolAcStateLength * 8
#define kCasper343DefaultRepeat       kNoRepeat
#define CASPER104_BITS                211
#define CASPER343_BITS                343

#define kReetechAcStateLength         12
#define kReetechAcBits                kReetechAcStateLength * 8
#define kReetechAcMinRepeat           kNoRepeat
#define REETECH_BITS                  197

#define kToshibaAcStateLength         9
#define kToshibaAcBits                kToshibaAcStateLength * 8
#define kToshibaAcMinRepeat           kSingleRepeat
#define kToshibaAcStateLengthShort    7 // kToshibaAcStateLength - 2
#define kToshibaAcBitsShort           kToshibaAcStateLengthShort * 8
#define kToshibaAcStateLengthLong     kToshibaAcStateLength + 1
#define kToshibaAcBitsLong            kToshibaAcStateLengthLong * 8
#define TOSHIBA_BITS                  295

#define kCoolixBits                   24
#define kCoolixDefaultRepeat          kSingleRepeat
#define kCoolix48Bits                 kCoolixBits * 2;
#define COOLIX_BITS                   199

#define kSanyoAcStateLength           9
#define kSanyoAcBits                  kSanyoAcStateLength * 8
#define kSanyoAc88StateLength         11
#define kSanyoAc88Bits                kSanyoAc88StateLength * 8
#define kSanyoAc88MinRepeat           2
#define kSanyoSA8650BBits             12
#define kSanyoLC7461AddressBits       13
#define kSanyoLC7461CommandBits       8
#define kSanyoLC7461Bits              (kSanyoLC7461AddressBits + kSanyoLC7461CommandBits) * 2

#define kVestelAcHdrMark              3030
#define kVestelAcHdrSpace             9090
#define kVestelAcBitMark              530
#define kVestelAcZeroSpace            530
#define kVestelAcOneSpace             1530
#define kVestelAcGap                  1530
#define kVestelAcTolerance            30
#define kVestelAcStateLength          7
#define kVestelAcBits                 56
#define VESTELAC_BITS                 231

#define kWhirlpoolAcStateLength       21
#define kWhirlpoolAcBits              kWhirlpoolAcStateLength * 8
#define kWhirlpoolAcDefaultRepeat     kNoRepeat

#define kTcl112AcStateLength          14
#define kTcl112AcBits                 kTcl112AcStateLength * 8
#define kTcl112AcDefaultRepeat        kNoRepeat


/// Gree A/C model numbers
typedef enum gree_ac_remote_model_t {
  YAW1F = 1,  // (1) Ultimate, EKOKAI, RusClimate (Default)
  YBOFB,     // (2) Green, YBOFB2, YAPOF3
}gree_ac_remote_model_t;
#define kGreeHdrMark                  9000
#define kGreeHdrSpace                 4500
#define kGreeBitMark                  600
#define kGreeZeroSpace                600
#define kGreeOneSpace                 1600
#define kGreeMsgSpace                 19980
#define kGreeMsgGap                   25800
#define kGreeBlockFooter              2
#define kGreeBlockFooterBits          3
#define kGreeRepeat                   3
#define kGreeStateLength              8
#define kGreeBits                     kGreeStateLength * 8
#define kGreeDefaultRepeat            kNoRepeat
#define GREE_BITS                     419

// Samsung AC
#define kSamsungBits                    32
#define kSamsung36Bits                  36
#define kSamsungAcStateLength           14
#define kSamsungAcBits                  kSamsungAcStateLength * 8
#define kSamsungAcExtendedStateLength   21
#define kSamsungAcExtendedBits          kSamsungAcExtendedStateLength * 8
#define kSamsungAcDefaultRepeat         kNoRepeat

#define SAMSUNG_BITS                    233
#define SAMSUNG_EXT_BITS                349

#endif  // IRCOMMON_H_


