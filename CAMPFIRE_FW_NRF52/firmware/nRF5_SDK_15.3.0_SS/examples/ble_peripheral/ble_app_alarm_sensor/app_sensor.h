


#ifndef __APP_SENSOR__
#define __APP_SENSOR__
#include <stdint.h>
#include <stdbool.h>

#define FOUND_NEIGHBOR						0xA5A5A5A5
#define FIND_NEIGHBOR							0x5A5A5A5A

//extern uint8_t flag_send_message_asap;
//extern bool sensor_data_sending;
//extern uint8_t send_data_timeout;

void app_sensor_task(void);
extern uint32_t tag_get_ramdom(uint32_t min, uint32_t max);
void app_sensor_init(void);
void app_set_humidity_value(uint8_t val);
uint8_t app_get_humidity_value();
void app_set_temperature_value(uint8_t val);
uint8_t app_get_temperature_value();
void app_meter_init(void);

#endif
