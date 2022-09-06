#ifndef __AC_TIMER_TASK__
#define __AC_TIMER_TASK__

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"

void ac_timer_task();

void ac_timer_tick();

void ac_timer_task_trigger(uint32_t timeout);

void ac_timer_task_init();

void ac_timer_task_deinit();

#endif
