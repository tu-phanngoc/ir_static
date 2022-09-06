
/// @file
/// @brief Support for Panasonic protocols.

#ifndef IR_PANASONIC_H_
#define IR_PANASONIC_H_

#include "stdint.h"

#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"

// Constants

#define kPanasonicAcAuto                  0
#define kPanasonicAcDry                   2
#define kPanasonicAcCool                  3
#define kPanasonicAcHeat                  4
#define kPanasonicAcFan                   6

#define kPanasonicAcFanMin                3
#define kPanasonicAcFanLow                4
#define kPanasonicAcFanMed                5
#define kPanasonicAcFanHigh               6
#define kPanasonicAcFanMax                7
#define kPanasonicAcFanAuto               0xA

#define kPanasonicAcFanModeTemp           27 // Celsius
#define kPanasonicAcFanTempDelta          0x20 // Celsius
#define kPanasonicAcMinTemp               16 // Celsius
#define kPanasonicAcMaxTemp               30 // Celsius

#define kPanasonicAcPowerOffset           0
#define kPanasonicAcTempOffset            1  // Bits
#define kPanasonicAcTempSize              5  // Bits
#define kPanasonicAcQuietOffset           0
#define kPanasonicAcPowerfulOffset        5

// CKP & RKR models have Powerful and Quiet bits swapped.
#define kPanasonicAcQuietCkpOffset        kPanasonicAcPowerfulOffset
#define kPanasonicAcPowerfulCkpOffset     kPanasonicAcQuietOffset
#define kPanasonicAcSwingVHighest         0x1
#define kPanasonicAcSwingVHigh            0x2
#define kPanasonicAcSwingVMiddle          0x3
#define kPanasonicAcSwingVLow             0x4
#define kPanasonicAcSwingVLowest          0x5
#define kPanasonicAcSwingVAuto            0xF

#define kPanasonicAcSwingHMiddle          0x6
#define kPanasonicAcSwingHFullLeft        0x9
#define kPanasonicAcSwingHLeft            0xA
#define kPanasonicAcSwingHRight           0xB
#define kPanasonicAcSwingHFullRight       0xC
#define kPanasonicAcSwingHAuto            0xD

#define kPanasonicAcChecksumInit          0xF4
#define kPanasonicAcOnTimerOffset         1
#define kPanasonicAcOffTimerOffset        2
#define kPanasonicAcTimeSize              11 // Bits
#define kPanasonicAcTimeOverflowSize      3  // Bits
#define kPanasonicAcTimeMax               23 * 60 + 59  // Mins since midnight.
#define kPanasonicAcTimeSpecial           0x600

#define kPanasonicAcIonFilterByte         22  // Byte
#define kPanasonicAcIonFilterOffset       0  // Bit


typedef union PanasonicAcProtocol_t {
  uint8_t raw[kPanasonicAcStateLength];  ///< State in code form
  struct {
    // Byte 0~7 Pad = 02 20 E0 04 00 00 00 06
    uint8_t Pad0[8];
    // Byte 8-12  Header = 02 20 E0 04 00
    uint8_t Header[5];
    // Byte 13
    uint8_t Power   :1;
    uint8_t rev1300 :3;
    uint8_t Mode    :4;
    // Byte 14
    uint8_t Temp    :8;
    // Byte 15 = 80
    uint8_t rev1500 :8;
    // Byte 16
    uint8_t SwingV  :4;
    uint8_t Fan     :4;
    // Byte 17
    uint8_t SwingH  :4;
    uint8_t rev1800 :4;
    // Byte 18-25   00 0E E0 00 00 89 00 00
    uint8_t Pad1[8];
    // Byte 26
    uint8_t Sum;
  };
} PanasonicAcProtocol_t;

void encode_PanasonicAc(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_PanasonicAc(int16_t* input, uint8_t* output);

bool PanasonicAc_recv(decode_results *results, uint16_t offset,
                      const uint16_t nbits, const bool strict );
void PanasonicAc_send(int16_t *irRaw);

bool isPanasonicAcHeader(int16_t *irRaw);

/// Class for handling detailed Panasonic A/C messages.
panasonic_ac_remote_model_t getModel(void);
void     PanasonicAc_setModel(const panasonic_ac_remote_model_t model);
void     PanasonicAc_setPower(const bool on);
bool     PanasonicAc_getPower(void);
void     PanasonicAc_setTemp(const uint8_t temp, const bool remember );
uint8_t  PanasonicAc_getTemp(void);
void     PanasonicAc_setFan(const uint8_t fan);
uint8_t  PanasonicAc_getFan(void);
void     PanasonicAc_setMode(const uint8_t mode);
uint8_t  PanasonicAc_getMode(void);
void     PanasonicAc_setRaw(const uint8_t state[]);
uint8_t* PanasonicAc_getRaw(void);
void     PanasonicAc_setIon(const bool on);
bool     PanasonicAc_getIon(void);
void     PanasonicAc_setSwingVertical(const uint8_t elevation);
uint8_t  PanasonicAc_getSwingVertical(void);
void     PanasonicAc_setSwingHorizontal(const uint8_t direction);
uint8_t  PanasonicAc_getSwingHorizontal(void);

uint8_t  PanasonicAc_convertMode(const  opmode_t mode);
uint8_t  PanasonicAc_convertFan(const  fanspeed_t speed);
uint8_t  PanasonicAc_convertSwingV(const  swingv_t position);
uint8_t  PanasonicAc_convertSwingH(const  swingh_t position);
opmode_t   PanasonicAc_toCommonMode(const uint8_t mode);
fanspeed_t PanasonicAc_toCommonFanSpeed(const uint8_t speed);
swingv_t   PanasonicAc_toCommonSwingV(const uint8_t pos);
swingh_t   PanasonicAc_toCommonSwingH(const uint8_t pos);

uint8_t PanasonicAc_calcChecksum(const uint8_t *state, const uint16_t length);
void PanasonicAc_Checksum(const uint16_t length);
bool PanasonicAc_validChecksum(const uint8_t *state, const uint16_t length);



#endif  // IR_PANASONIC_H_
