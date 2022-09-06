
#ifndef IRSEND_H_
#define IRSEND_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "stdbool.h"
#include "IRcommon.h"

#define kPeriodOffset               -5
#define kDutyDefault                50          // Percentage
#define kDutyMax                    100         // Percentage
#define kMaxAccurateUsecDelay       16383
#define kDefaultMessageGap          INT16_MAX

/// Enumerators and Structures for the Common A/C API.
typedef enum {
  kOpModeOff  = -1,
  kOpModeDry  =  1,
  kOpModeFan  =  2,
  kOpModeHeat =  3,
  kOpModeCool =  4,
  kOpModeAuto =  7,
  // Add new entries before this one, and update it to point to the last entry
  kLastOpmodeEnum = kOpModeAuto,
} opmode_t;

/// Common A/C settings for Fan Speeds.
typedef enum {
  kFanSpeedMin =    1,
  kFanSpeedLow =    2,
  kFanSpeedMedium = 3,
  kFanSpeedHigh =   4,
  kFanSpeedMax =    5,
  kFanSpeedAuto =   7,
  // Add new entries before this one, and update it to point to the last entry
  kLastFanspeedEnum = kFanSpeedMax,
} fanspeed_t;

/// Common A/C settings for Vertical Swing.
typedef enum {
  kSwingVOff =    -1,
  kSwingVAuto =    0,
  kSwingVHighest = 1,
  kSwingVHigh =    2,
  kSwingVMiddle =  3,
  kSwingVLow =     4,
  kSwingVLowest =  5,
  // Add new entries before this one, and update it to point to the last entry
  kLastSwingvEnum = kSwingVLowest,
} swingv_t;

/// Common A/C settings for Horizontal Swing.
typedef enum {
  kSwingHOff =     -1,
  kSwingHAuto =     0,  // a.k.a. On.
  kSwingHLeftMax =  1,
  kSwingHLeft =     2,
  kSwingHMiddle =   3,
  kSwingHRight =    4,
  kSwingHRightMax = 5,
  kSwingHWide =     6,  // a.k.a. left & right at the same time.
  // Add new entries before this one, and update it to point to the last entry
  kLastSwinghEnum = kSwingHWide,
}swingh_t;

/// Structure to hold a common A/C state.
typedef struct {
  decode_type_t protocol;
  int16_t model;
  bool power;
  opmode_t mode;
  float degrees;
  bool celsius;
  fanspeed_t fanspeed;
  swingv_t swingv;
  swingh_t swingh;
  bool quiet;
  bool turbo;
  bool econo;
  bool light;
  bool filter;
  bool clean;
  bool beep;
  int16_t sleep;
  int16_t clock;
} state_t;

/// Fujitsu A/C model numbers
enum fujitsu_ac_remote_model_t {
  ARRAH2E = 1,  ///< (1) AR-RAH2E, AR-RAC1E, AR-RAE1E, AR-RCE1E (Default)
                ///< Warning: Use on incorrect models can cause the A/C to lock
                ///< up, requring the A/C to be physically powered off to fix.
                ///< e.g. AR-RAH1U may lock up with a Swing command.
  ARDB1,        ///< (2) AR-DB1, AR-DL10 (AR-DL10 swing doesn't work)
  ARREB1E,      ///< (3) AR-REB1E, AR-RAH1U (Similar to ARRAH2E but no horiz
                ///<     control)
  ARJW2,        ///< (4) AR-JW2  (Same as ARDB1 but with horiz control)
  ARRY4,        ///< (5) AR-RY4 (Same as AR-RAH2E but with clean & filter)
  ARREW4E,      ///< (6) Similar to ARRAH2E, but with different temp config.
};

/// HAIER_AC176 A/C model numbers
enum haier_ac176_remote_model_t {
  V9014557_A = 1,  // (1) V9014557 Remote in "A" setting. (Default)
  V9014557_B,      // (2) V9014557 Remote in "B" setting.
};

/// HITACHI_AC1 A/C model numbers
enum hitachi_ac1_remote_model_t {
  R_LT0541_HTA_A = 1,  // (1) R-LT0541-HTA Remote in "A" setting. (Default)
  R_LT0541_HTA_B,      // (2) R-LT0541-HTA Remote in "B" setting.
};

/// MIRAGE A/C model numbers
enum mirage_ac_remote_model_t {
  KKG9AC1 = 1,  // (1) KKG9A-C1 Remote. (Default)
  KKG29AC1,     // (2) KKG29A-C1 Remote.
};


/// TCL A/C model numbers
enum tcl_ac_remote_model_t {
  TAC09CHSD = 1,
  GZ055BE1 = 2,
};

/// Voltas A/C model numbers
enum voltas_ac_remote_model_t {
  kVoltasUnknown = 0,  // Full Function
  kVoltas122LZF = 1,   // (1) 122LZF (No SwingH support) (Default)
};



uint16_t dataToRaw(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
                      uint32_t zerospace, uint64_t data, uint16_t nbits,
                      bool MSBfirst, 
                      int16_t *IR_raw);

void sendGeneric_8(  const uint16_t headermark,  const uint16_t headerspace,
                      const uint16_t onemark,     const uint16_t onespace,
                      const uint16_t zeromark,    const uint16_t zerospace,
                      const uint16_t footermark,  const uint16_t gap,
                      const uint8_t *dataptr,     const uint16_t nbytes,
                      const uint16_t frequency,   const bool     MSBfirst,
                      const uint16_t repeat,      const uint8_t  dutycycle,
                      int16_t *IR_raw);

void sendGeneric_64( const uint16_t headermark,  const uint32_t headerspace,
                      const uint16_t onemark,     const uint32_t onespace,
                      const uint16_t zeromark,    const uint32_t zerospace,
                      const uint16_t footermark,  const uint32_t gap,
                      const uint32_t mesgtime,    const uint64_t data,
                      const uint16_t nbits,       const uint16_t frequency,
                      const bool MSBfirst,        const uint16_t repeat,
                      const uint8_t dutycycle,
                      int16_t *IR_raw);

void sendManchesterData(const uint16_t half_period, const uint64_t data,
                        const uint16_t nbits, const bool MSBfirst,
                        const bool GEThomas);
void sendManchester(const uint16_t headermark, const uint32_t headerspace,
                    const uint16_t half_period, const uint16_t footermark,
                    const uint32_t gap, const uint64_t data,
                    const uint16_t nbits, const uint16_t frequency,
                    const bool MSBfirst,
                    const uint16_t repeat,
                    const uint8_t dutycycle,
                    const bool GEThomas);

#endif



