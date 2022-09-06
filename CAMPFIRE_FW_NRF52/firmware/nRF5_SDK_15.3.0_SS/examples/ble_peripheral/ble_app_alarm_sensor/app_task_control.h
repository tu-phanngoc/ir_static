#ifndef __APP_TASK_CONTROL_H_
#define __APP_TASK_CONTROL_H_

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

#define TASK_IDX_DEFAULT                        0
#define TASK_IDX_CONTROL_AC                     1
#define TASK_IDX_IR_RECV                        2
#define TASK_IDX_DATALOG                        3

uint32_t task_control_get_current_task(void);
void task_control_set_current_task(uint8_t idx);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_HAL_H */