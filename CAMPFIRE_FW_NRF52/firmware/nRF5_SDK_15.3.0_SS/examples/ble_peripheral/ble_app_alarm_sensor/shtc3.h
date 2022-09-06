// Copyright (c) Konstantin Belyalov. All rights reserved.
// Licensed under the MIT license.

#ifndef __SHTC3_H
#define __SHTC3_H

#include "nrf_drv_twi.h"
#include "nrf_delay.h"

#ifdef __cplusplus
#define EXPORT extern "C"
#else
#define EXPORT
#endif


// The shtc3 provides a serial number individualized for each device
// Params:
//  - `hi2c` I2C bus
// Returns device id or 0 in case of error.
EXPORT uint16_t shtc3_read_id(nrf_drv_twi_t const *p_instance);

// Put sensor into sleep mode
// Params:
//  - `hi2c` I2C bus
// Returns device id or 0 in case of error.
EXPORT uint32_t shtc3_sleep(nrf_drv_twi_t const *p_instance);

// Wake up sensor.
// You must wait for 240us to let sensor enter into IDLE mode.
// Params:
//  - `hi2c` I2C bus
// Returns zero in case of error
EXPORT uint32_t shtc3_wakeup(nrf_drv_twi_t const *p_instance);

// Performs full cycle: starts temperature/humidity measurements using "clock stretch" method.
// Params:
//  - `hi2c` I2C bus
//  - `temp` measured temperature, in C multiplied by 100 (e.g. 24.1C -> 2410)
//  - `hum` measured relative humidity, in percents
// Returns zero in case of error
EXPORT uint32_t shtc3_perform_measurements(nrf_drv_twi_t const *p_instance, int32_t* temp, int32_t* hum);

// Start temperature/humidity measurements using "clock stretch" approach, in low power mode.
// After completed - values can be obtained by shtc3_read_measurements()
// Params:
//  - `hi2c` I2C bus
//  - `temp` measured temperature, in C multiplied by 100 (e.g. 24.1C -> 2410)
//  - `hum` measured relative humidity, in percents
// Returns zero in case of error
EXPORT uint32_t shtc3_perform_measurements_low_power(nrf_drv_twi_t const *p_instance, int32_t* out_temp, int32_t* out_hum);

uint8_t shtc3_init();
uint32_t shtc3_softreset(nrf_drv_twi_t const *p_instance);
void shtc3_loop();
void app_shtc3_sleep();
#endif