/// @file
/// @brief Support for Mitsubishi Heavy Industry protocols.
/// Code to emulate Mitsubishi Heavy Industries A/C IR remote control units.
/// @note This code was *heavily* influenced by ToniA's great work & code,
///   but it has been written from scratch.
///   Nothing was copied other than constants and message analysis.

// Supports:
//   Brand: Mitsubishi Heavy Industries,  Model: RLA502A700B remote (152 bit)
//   Brand: Mitsubishi Heavy Industries,  Model: SRKxxZM-S A/C (152 bit)
//   Brand: Mitsubishi Heavy Industries,  Model: SRKxxZMXA-S A/C (152 bit)
//   Brand: Mitsubishi Heavy Industries,  Model: RKX502A001C remote (88 bit)
//   Brand: Mitsubishi Heavy Industries,  Model: SRKxxZJ-S A/C (88 bit)

#ifndef IR_MITSUBISHIHEAVY_H_
#define IR_MITSUBISHIHEAVY_H_

#include "IRsend.h"
#include "IRrecv.h"
#include "IRcommon.h"


#define kMitsubishiHeavyHdrMark     3172
#define kMitsubishiHeavyHdrSpace    1586
#define kMitsubishiHeavyBitMark     394
#define kMitsubishiHeavyOneSpace    394
#define kMitsubishiHeavyZeroSpace   1182
#define kMitsubishiHeavyGap         kDefaultMessageGap  // Just a guess.


/// Native representation of a Mitsubishi Heavy 152-bit A/C message.
typedef union Mitsubishi152Protocol_t{
  uint8_t raw[kMitsubishi152StateLength];  ///< State in code form
  struct {
    // Byte 0~4
    uint8_t Sig[5];
    // Byte 5
    uint8_t Mode  :3;
    uint8_t Power :1;
    uint8_t rev0500:1;
    uint8_t Clean :1;
    uint8_t Filter:1;
    uint8_t rev0501 :1;
    // Byte 6
    uint8_t rev0600 :8;
    // Byte 7
    uint8_t Temp  :4;
    uint8_t __70  :4;
    // Byte 8
    uint8_t rev0800:8;
    // Byte 9
    uint8_t Fan   :4;
    uint8_t __90  :4;
    // Byte 10
    uint8_t rev1000:8;
    // Byte 11
    uint8_t __B0    :1;
    uint8_t Three   :1;
    uint8_t __B1    :2;
    uint8_t D       :1;  // binding with "Three"
    uint8_t SwingV  :3;
    // Byte 12
    uint8_t rev1200:8;
    // Byte 13
    uint8_t SwingH  :4;
    uint8_t __D0    :4;
    // Byte 14
    uint8_t rev1400:8;
    // Byte 15
    uint8_t __F0    :6;
    uint8_t Night   :1;
    uint8_t Silent  :1;
  };
} Mitsubishi152Protocol_t;

// Constants.

#define kMitsubishiHeavySigLength             5

#define kMitsubishiHeavyAuto                  0 // 0b000
#define kMitsubishiHeavyCool                  1 // 0b001
#define kMitsubishiHeavyDry                   2 // 0b010
#define kMitsubishiHeavyFan                   3 // 0b011
#define kMitsubishiHeavyHeat                  4 // 0b100

#define kMitsubishiHeavyDeltaTemp             17
#define kMitsubishiHeavyMinTemp               18   // 18C
#define kMitsubishiHeavyMaxTemp               30   // 30C

#define kMitsubishiHeavy152FanAuto            0x0  // 0b0000
#define kMitsubishiHeavy152FanLow             0x1  // 0b0001
#define kMitsubishiHeavy152FanMed             0x2  // 0b0010
#define kMitsubishiHeavy152FanHigh            0x3  // 0b0011
#define kMitsubishiHeavy152FanMax             0x4  // 0b0100
#define kMitsubishiHeavy152FanEcono           0x6  // 0b0110
#define kMitsubishiHeavy152FanTurbo           0x8  // 0b1000

#define kMitsubishiHeavy152SwingVAuto         0  // 0b000
#define kMitsubishiHeavy152SwingVHighest      1  // 0b001
#define kMitsubishiHeavy152SwingVHigh         2  // 0b010
#define kMitsubishiHeavy152SwingVMiddle       3  // 0b011
#define kMitsubishiHeavy152SwingVLow          4  // 0b100
#define kMitsubishiHeavy152SwingVLowest       5  // 0b101
#define kMitsubishiHeavy152SwingVOff          6  // 0b110

#define kMitsubishiHeavy152SwingHAuto         0  // 0b0000
#define kMitsubishiHeavy152SwingHLeftMax      1  // 0b0001
#define kMitsubishiHeavy152SwingHLeft         2  // 0b0010
#define kMitsubishiHeavy152SwingHMiddle       3  // 0b0011
#define kMitsubishiHeavy152SwingHRight        4  // 0b0100
#define kMitsubishiHeavy152SwingHRightMax     5  // 0b0101
#define kMitsubishiHeavy152SwingHRightLeft    6  // 0b0110
#define kMitsubishiHeavy152SwingHLeftRight    7  // 0b0111
#define kMitsubishiHeavy152SwingHOff          8  // 0b1000

/// Native representation of a Mitsubishi Heavy 88-bit A/C message.
typedef union Mitsubishi88Protocol_t {
  uint8_t raw[kMitsubishi88StateLength];  ///< State in code form
  struct {
    // Byte 0~4
    uint8_t Sig[5];
    // Byte 5
    uint8_t __50    :1;
    uint8_t SwingV5 :1;
    uint8_t SwingH1 :2;
    uint8_t __51    :1;
    uint8_t Clean   :1;
    uint8_t SwingH2 :2;
    // Byte 6
    uint8_t __06    :8;
    // Byte 7
    uint8_t __70    :3;
    uint8_t SwingV7 :2;
    uint8_t Fan     :3;
    // Byte 8
    uint8_t __08    :8;
    // Byte 9
    uint8_t Mode  :3;
    uint8_t Power :1;
    uint8_t Temp  :4;
  };
} Mitsubishi88Protocol_t;



#define kMitsubishiHeavy88MinTemp               18   // 17C
#define kMitsubishiHeavy88MaxTemp               30   // 31C

#define kMitsubishiHeavy88SwingHSize          2   // Bits (per offset)
#define kMitsubishiHeavy88SwingHOff           0   // 0b0000
#define kMitsubishiHeavy88SwingHAuto          8   // 0b1000
#define kMitsubishiHeavy88SwingHLeftMax       1   // 0b0001
#define kMitsubishiHeavy88SwingHLeft          5   // 0b0101
#define kMitsubishiHeavy88SwingHMiddle        9   // 0b1001
#define kMitsubishiHeavy88SwingHRight         13  // 0b1101
#define kMitsubishiHeavy88SwingHRightMax      2   // 0b0010
#define kMitsubishiHeavy88SwingHRightLeft     10  // 0b1010
#define kMitsubishiHeavy88SwingHLeftRight     6   // 0b0110
#define kMitsubishiHeavy88SwingH3D            14  // 0b1110

#define kMitsubishiHeavy88FanAuto             0  // 0b000
#define kMitsubishiHeavy88FanLow              2  // 0b010
#define kMitsubishiHeavy88FanMed              3  // 0b011
#define kMitsubishiHeavy88FanHigh             4  // 0b100
#define kMitsubishiHeavy88FanTurbo            6  // 0b110
#define kMitsubishiHeavy88FanEcono            7  // 0b111
#define kMitsubishiHeavy88SwingVByte5Size     1

#define kMitsubishiHeavy88SwingVOff           0 // 0b000
#define kMitsubishiHeavy88SwingVAuto          4 // 0b100
#define kMitsubishiHeavy88SwingVHighest       6 // 0b110
#define kMitsubishiHeavy88SwingVHigh          1 // 0b001
#define kMitsubishiHeavy88SwingVMiddle        3 // 0b011
#define kMitsubishiHeavy88SwingVLow           5 // 0b101
#define kMitsubishiHeavy88SwingVLowest        7 // 0b111







void encode_MitsubishiHeavy(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void decode_MitsubishiHeavy(int16_t* input, uint8_t* output);

void MitsubishiHeavy_set152AcTypes(bool on);
bool MitsubishiHeavy_recv(decode_results* results, uint16_t offset, const uint16_t nbits, const bool strict);
void MitsubishiHeavy_send( const unsigned char data[], const uint16_t nbytes, const uint16_t repeat, int16_t *irRaw);



/// Class for handling detailed Mitsubishi Heavy 152-bit A/C messages.

  void    Mitsubishi152_setPower(const bool on);
  bool    Mitsubishi152_getPower(void);
  void    Mitsubishi152_setTemp(const uint8_t temp);
  uint8_t Mitsubishi152_getTemp(void);
  void    Mitsubishi152_setFan(const uint8_t fan);
  uint8_t Mitsubishi152_getFan(void);
  void    Mitsubishi152_setMode(const uint8_t mode);
  uint8_t Mitsubishi152_getMode(void);
  void    Mitsubishi152_setSwingVertical(const uint8_t pos);
  uint8_t Mitsubishi152_getSwingVertical(void);
  void    Mitsubishi152_setSwingHorizontal(const uint8_t pos);
  uint8_t Mitsubishi152_getSwingHorizontal(void);
  void Mitsubishi152_setNight(const bool on);
  bool Mitsubishi152_getNight(void);
  void Mitsubishi152_set3D(const bool on);
  bool Mitsubishi152_get3D(void);
  void Mitsubishi152_setSilent(const bool on);
  bool Mitsubishi152_getSilent(void);
  void Mitsubishi152_setFilter(const bool on);
  bool Mitsubishi152_getFilter(void);
  void Mitsubishi152_setClean(const bool on);
  bool Mitsubishi152_getClean(void);
  void Mitsubishi152_setTurbo(const bool on);
  bool Mitsubishi152_getTurbo(void);
  void Mitsubishi152_setEcono(const bool on);
  bool Mitsubishi152_getEcono(void);
  uint8_t* Mitsubishi152_getRaw(void);
  void Mitsubishi152_setRaw(const uint8_t* data);

  bool Mitsubishi152_checkZmsSig(const uint8_t *state);
  bool Mitsubishi152_validChecksum( const uint8_t *state, const uint16_t length);
  void Mitsubishi152_checksum(void);

  uint8_t Mitsubishi152_convertMode(const opmode_t mode);
  uint8_t Mitsubishi152_convertFan(const fanspeed_t speed);
  uint8_t Mitsubishi152_convertSwingV(const swingv_t position);
  uint8_t Mitsubishi152_convertSwingH(const swingh_t position);
  opmode_t Mitsubishi152_toCommonMode(const uint8_t mode);
  fanspeed_t Mitsubishi152_toCommonFanSpeed(const uint8_t speed);
  swingv_t Mitsubishi152_toCommonSwingV(const uint8_t pos);
  swingh_t Mitsubishi152_toCommonSwingH(const uint8_t pos);




/// Class for handling detailed Mitsubishi Heavy 88-bit A/C messages.
  void    Mitsubishi88_setPower(const bool on);
  bool    Mitsubishi88_getPower(void);
  void    Mitsubishi88_setTemp(const uint8_t temp);
  uint8_t Mitsubishi88_getTemp(void);
  void    Mitsubishi88_setFan(const uint8_t fan);
  uint8_t Mitsubishi88_getFan(void);
  void    Mitsubishi88_setMode(const uint8_t mode);
  uint8_t Mitsubishi88_getMode(void);
  void    Mitsubishi88_setSwingVertical(const uint8_t pos);
  uint8_t Mitsubishi88_getSwingVertical(void);
  void    Mitsubishi88_setSwingHorizontal(const uint8_t pos);
  uint8_t Mitsubishi88_getSwingHorizontal(void);
  void Mitsubishi88_setTurbo(const bool on);
  bool Mitsubishi88_getTurbo(void);
  void Mitsubishi88_setEcono(const bool on);
  bool Mitsubishi88_getEcono(void);
  void Mitsubishi88_set3D(const bool on);
  bool Mitsubishi88_get3D(void);
  void Mitsubishi88_setClean(const bool on);
  bool Mitsubishi88_getClean(void);
  uint8_t* Mitsubishi88_getRaw(void);
  void Mitsubishi88_setRaw(const uint8_t* data);

  void Mitsubishi88_checksum(void);
  bool Mitsubishi88_checkZjsSig(const uint8_t *state);
  bool Mitsubishi88_validChecksum( const uint8_t *state, const uint16_t length);

  uint8_t Mitsubishi88_convertMode(const opmode_t mode);
  uint8_t Mitsubishi88_convertFan(const fanspeed_t speed);
  uint8_t Mitsubishi88_convertSwingV(const swingv_t position);
  uint8_t Mitsubishi88_convertSwingH(const swingh_t position);
  fanspeed_t Mitsubishi88_toCommonFanSpeed(const uint8_t speed);
  swingv_t   Mitsubishi88_toCommonSwingV(const uint8_t pos);
  swingh_t   Mitsubishi88_toCommonSwingH(const uint8_t pos);




#endif  // IR_MITSUBISHIHEAVY_H_




