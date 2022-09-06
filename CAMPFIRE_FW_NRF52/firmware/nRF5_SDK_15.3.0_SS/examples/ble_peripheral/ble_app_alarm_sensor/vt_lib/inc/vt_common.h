#ifndef _VT_COMMON_H_
#define _VT_COMMON_H_
#include <stdint.h>
#include <stdbool.h>
#include "app_gpiote.h"

typedef void (*vt_sensor_handler_t) (bool status);

/* Relative Humidity Sensor API */
/*
 * Description: Initialize the driver for Humidity sensor SHT21
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_humidity_init(void);

/*
 * Description: Read humidity value from sensor SHT21
 * Param: p_value - pointer to store humidity value
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_humidity_read(uint16_t *p_value);

/* Temparature Sensor API */
/*
 * Description: Initialize the driver for Temperature sensor TMP006
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_temp_init(void);

/*
 * Description: Read temparature value from sensor TMP006
 * Param: p_value - pointer to store temperature value
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_temp_read(uint16_t *p_value);

/* Pressure Sensor API */
/*
 * Description: Initialize the driver for Pressure sensor T5400
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_pressure_init(void);

/*
 * Description: Read pressure value from sensor T5400
 * Param: p_value - pointer to store temperature value
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_pressure_read(uint32_t *p_value);

/* Door Sensor API */
/*
 * Description: Initialize the driver for Door Sensor
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_door_init(app_gpiote_event_handler_t handler);

/* Sos Sensor API */
/*
 * Description: Initialize the driver for Door Sensor
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_sos_init(app_gpiote_event_handler_t handler);
/* SmartTouch Sensor API */
/*
 * Description: Initialize the driver for Door Sensor
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_smarttouch_init(app_gpiote_event_handler_t handler);
/* IR Sensor API */
/*
 * Description: Initialize the driver for IR Sensor
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_ir_init(void);

/*
 * Description: Register handler for IR Sensor; the handler will be called when IR sensor signal change state.
 * Param: handler - function pointer to handle IR sensor state change.
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_ir_register_handler(const vt_sensor_handler_t handler);

/* PIR Sensor API */
/*
 * Description: Initialize the driver for PIR Sensor
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_pir_init(app_gpiote_event_handler_t handler);

/* Gas Sensor API */
/*
 * Description: Initialize the driver for Gas Sensor
 * Param: NULL
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_gas_init(void);

/*
 * Description: Set threshold for Gas Sensor
 * Param: threshold - threshold value
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_gas_set_threshold(uint32_t threshold);

/*
 * Description: Register handler for Gas Sensor; the handler will be called when read value is larger than threshold.
 * Param: handler - function pointer to handle when gas value larger than threhold.
 * Return: 0 - Success
 *         1 - Failed
 */
uint32_t vt_sensor_gas_register_handler(const vt_sensor_handler_t handler);

#endif
