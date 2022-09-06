
#ifndef __M_NRF_SPI_H__
#define __M_NRF_SPI_H__
#include <stdint.h>
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"

void spi1_Init(void);
uint8_t halSpi1_WriteByte(uint8_t data);

#endif

