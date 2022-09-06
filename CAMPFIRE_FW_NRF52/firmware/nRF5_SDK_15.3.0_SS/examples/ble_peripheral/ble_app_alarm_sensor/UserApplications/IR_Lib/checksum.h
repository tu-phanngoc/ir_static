
#ifndef CHECKSUM_H_
#define CHECKSUM_H_

#include "stdbool.h"
#include "stdint.h"
#include "string.h"



bool    validChecksum_sumBytes(const uint8_t *state, const uint16_t length, const uint8_t init);
uint8_t  calcChecksum_sumBytes(const uint8_t *state, const uint16_t length, const uint8_t init);



#endif  // CHECKSUM_H_
