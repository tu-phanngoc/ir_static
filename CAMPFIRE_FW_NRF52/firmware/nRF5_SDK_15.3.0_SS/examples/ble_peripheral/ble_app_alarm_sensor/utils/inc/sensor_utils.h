#ifndef __SENSOR_UTILS_H__
#define __SENSOR_UTILS_H__

#include "lib/ringbuf.h"
extern RINGBUF sensorsRingBuf;
extern uint8_t sequence;
#if defined(BOARD_SHT2X)
#define SENSOR_DELAY_TIME_TO_SEND_DATA	60
#else
#define SENSOR_DELAY_TIME_TO_SEND_DATA	10
#endif
/* Function prototypes */
void sensor_init(void);

void sensor_send_info(void);

void sensor_trigger_to_send_data(void);
uint32_t sensor_get_change_cnt(void);
void set_sensor_status(bool status);
void sensor_tick_1s_handler(void);
bool get_sensor_status(void);
uint8_t sensor_got_sequence(void);
void sensor_sequence_inc(void);
void clear_sensor_status(void);
#endif
