

#ifndef __BTN_H__
#define __BTN_H__
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf52811_bitfields.h"
#include "boards.h"
#include "app_timer.h"

#define BUTTON_PRESS_2S	2
#define BUTTON_PRESS_3S	3
#define BUTTON_PRESS_4S	4
#define BUTTON_PRESS_5S	5
#define BUTTON_1_CLICK	1
#define BUTTON_2_CLICK	2
#define BUTTON_3_CLICK	3
#define BUTTON_4_CLICK	4
#define BUTTON_5_CLICK	5
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)    /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define TAG_BUTTON_PROCESS_INTERVAL			APP_TIMER_TICKS(500) /**< Battery level measurement interval (ticks). */

extern uint8_t button_click_times;
extern uint32_t button_press_time;
extern uint8_t button_pressed_flag;
void btn_init(void);
void btn_set_input_sense(void);
void btn_set_output_and_clear_pin(void);
#endif
