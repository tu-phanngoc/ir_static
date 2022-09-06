#ifndef __AC_TYPE_DETECTION_TASK__
#define __AC_TYPE_DETECTION_TASK__

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"

void ac_type_detection_task();

void ac_type_detection_tick();

void ac_type_detection_task_trigger(uint32_t timeout);

void ac_type_detection_task_init();

void ac_type_detection_task_deinit();
void ac_type_detection_set_signal(uint8_t data);
uint8_t is_ac_type_detection_enabled();

#endif
