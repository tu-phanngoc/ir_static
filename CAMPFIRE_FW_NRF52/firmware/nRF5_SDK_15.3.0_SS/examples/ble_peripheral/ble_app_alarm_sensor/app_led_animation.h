#ifndef __APP_LED_ANIMATION__
#define __APP_LED_ANIMATION__

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"

#define LED_ANIMATION_TRIGGER_RED               1
#define LED_ANIMATION_TRIGGER_GREEN             2
#define LED_ANIMATION_TRIGGER_BLUE              4

void app_led_animation_tick();

void app_led_animation_trigger(uint32_t timeout, uint8_t type);

void app_led_animation_task();


void app_led_blink_trigger(uint8_t type);

void app_blink_tick();

void app_led_blink_task();

#endif
