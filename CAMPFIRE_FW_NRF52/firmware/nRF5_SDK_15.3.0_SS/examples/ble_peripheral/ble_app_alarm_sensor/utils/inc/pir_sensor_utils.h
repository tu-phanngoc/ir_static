#ifndef __PIR_SENSOR_UTILS_H__
#define __PIR_SENSOR_UTILS_H__
#include <stdint.h>
/* Module APIs */
void pir_sensor_init(void);

void pir_sensor_send_presentation(void);

void pir_sensor_send_status(uint8_t sequence);

void pir_sensor_trigger_to_send_status(void);

void _set_pir_status(bool status);

#endif
