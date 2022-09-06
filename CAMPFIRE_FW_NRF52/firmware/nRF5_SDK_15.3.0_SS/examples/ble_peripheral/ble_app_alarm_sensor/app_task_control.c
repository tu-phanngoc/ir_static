#include "app_task_control.h"
#include "nrf_log.h"

uint8_t gu8_current_task_ctl = TASK_IDX_DEFAULT;

uint32_t task_control_get_current_task(void)
{
    return gu8_current_task_ctl;
}

void task_control_set_current_task(uint8_t idx)
{
    gu8_current_task_ctl = idx;
}
