#ifndef IRUTILS_H_
#define IRUTILS_H_

// Copyright 2017 David Conran

#include <stdint.h>
#include "IRrecv.h"

#define kNibbleSize             4
#define kLowNibble              0
#define kHighNibble             4
#define kModeBitsSize           3



/** Leaves the minimum of the two 32-bit arguments */
/*lint -emacro(506, MIN) */ /* Suppress "Constant value Boolean */
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif //MIN
/** Leaves the maximum of the two 32-bit arguments */
/*lint -emacro(506, MAX) */ /* Suppress "Constant value Boolean */
#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif //MAX



uint64_t reverseBits(uint64_t input, uint16_t nbits);
uint8_t  sumBytes(const uint8_t * const start, const uint16_t length, const uint8_t init);
uint8_t  xorBytes(const uint8_t * const start, const uint16_t length, const uint8_t init);
uint16_t countBits_8(const uint8_t * const start, const uint16_t length, const bool ones, const uint16_t init);
uint16_t countBits_64(const uint64_t data, const uint8_t length, const bool ones, const uint16_t init);
uint64_t invertBits(const uint64_t data, const uint16_t nbits);
decode_type_t strToDecodeType(const char *str);
float   celsiusToFahrenheit(const float deg);
float   fahrenheitToCelsius(const float deg);
uint8_t sumNibbles_8(const uint8_t * const start, const uint16_t length, const uint8_t init);
uint8_t sumNibbles_64(const uint64_t data, const uint8_t count, const uint8_t init, const bool nibbleonly);
uint8_t bcdToUint8(const uint8_t bcd);
uint8_t uint8ToBcd(const uint8_t integer);
bool    getBit_64(const uint64_t data, const uint8_t position, const uint8_t size);
bool    getBit_8(const uint8_t data, const uint8_t position);

#define GETBIT8(a, b) ((a) & ((uint8_t)1 << (b)))
#define GETBIT16(a, b) ((a) & ((uint16_t)1 << (b)))
#define GETBIT32(a, b) ((a) & ((uint32_t)1 << (b)))
#define GETBIT64(a, b) ((a) & ((uint64_t)1 << (b)))
#define GETBITS8(data, offset, size) \
    (((data) & (((uint8_t)UINT8_MAX >> (8 - (size))) << (offset))) >> (offset))
#define GETBITS16(data, offset, size) \
    (((data) & (((uint16_t)UINT16_MAX >> (16 - (size))) << (offset))) >> \
     (offset))
#define GETBITS32(data, offset, size) \
    (((data) & (((uint32_t)UINT32_MAX >> (32 - (size))) << (offset))) >> \
     (offset))
#define GETBITS64(data, offset, size) \
    (((data) & (((uint64_t)UINT64_MAX >> (64 - (size))) << (offset))) >> \
     (offset))

uint64_t setBit_64(const uint64_t data, const uint8_t position, const bool on, const uint8_t size);
uint8_t setBit_8(const uint8_t data, const uint8_t position, const bool on);
void setBit_p8(uint8_t * const data, const uint8_t position, const bool on);
void setBit_p32(uint32_t * const data, const uint8_t position, const bool on);
void setBit_p64(uint64_t * const data, const uint8_t position, const bool on);
void setBits_8(uint8_t * const dst, const uint8_t offset, const uint8_t nbits, const uint8_t data);
void setBits_32(uint32_t * const dst, const uint8_t offset, const uint8_t nbits, const uint32_t data);
void setBits_64(uint64_t * const dst, const uint8_t offset, const uint8_t nbits, const uint64_t data);
uint64_t u8tou64(uint8_t const u8[8]);
uint8_t * invertBytePairs(uint8_t *ptr, const uint16_t length);
bool checkInvertedBytePairs(const uint8_t * const ptr, const uint16_t length);
uint8_t lowLevelSanityCheck(void);
uint64_t swapLong(uint64_t input, uint8_t nbytes);

#endif  // IRUTILS_H_
