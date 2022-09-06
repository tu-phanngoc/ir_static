#ifndef __APP_ERROR_CODE_H__
#define __APP_ERROR_CODE_H__

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"

#define BENKON_ERR_CODE_SYSTEM_RESET        1

typedef struct error_list
{
    uint8_t err_system_reset;
}error_list_t;

uint8_t error_code_task(void);
uint8_t error_code_set(uint8_t err);
uint8_t error_code_clear(uint8_t err);
void error_code_report_reset(void);
void error_code_init(void);

extern uint32_t gu32_reset_reason;
#endif
