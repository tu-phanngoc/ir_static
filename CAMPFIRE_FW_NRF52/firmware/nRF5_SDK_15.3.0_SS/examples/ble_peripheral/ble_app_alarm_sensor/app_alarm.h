#ifndef __APP_ALARM_H
#define __APP_ALARM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"

#include "data_transmit_fsm.h"



#define MEMORY_SCHEDULE_ADDRESS			0x0006FA00UL

extern schedule_control_t schedule_control[NUMBER_OF_SCHEDULE];

void app_alarm_task();
void app_alarm_set_config(uint8_t* data, uint8_t len, uint8_t index);
void app_alarm_get_config(uint8_t* data, uint8_t len, uint8_t index);
void app_alarm_init();
#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_HAL_H */