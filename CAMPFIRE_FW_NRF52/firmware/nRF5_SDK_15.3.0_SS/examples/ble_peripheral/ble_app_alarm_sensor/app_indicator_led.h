#ifndef __APP_INDICATOR_LED__
#define __APP_INDICATOR_LED__

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"

#define RGB_TYPE_R			1
#define RGB_TYPE_B			2
#define RGB_TYPE_G			3
void app_led_task();

void set_led_pwm(uint8_t val);
uint8_t get_led_pwm(void);
void app_indicator_led_on(uint8_t led_type);
void app_indicator_led_off(uint8_t led_type);

#endif
