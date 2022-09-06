/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include "boards.h"
#include <stdint.h>

#if defined (BOARD_AMPM_GT999)
#include <stdio.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf52811_bitfields.h"
#include "boards.h"
#include "app_gpiote.h"
//#include "sensor_utils.h"
#include "app_message_queue.h"
#include "app_timer.h"

const uint32_t UICR_ADDR_0x20C    __attribute__((at(0x1000120C))) __attribute__((used)) = 0xFFFFFFFE;

uint8_t lowpower_flag;
uint32_t findNeighborFlag __attribute__((at(0x20005fec)));
uint32_t rtcTimeSec __attribute__((at(0x20005ff8)));//added by thienhaiblue@gmail.com
uint32_t low_bat_cnt_time_save __attribute__((at(0x20005ffc)));//added by thienhaiblue@gmail.com
extern void app_rtc_timeout_handler(void * p_context);
APP_TIMER_DEF(m_app_rtc_timer_id);                  /**< Connection parameters timer. */

uint8_t createOtpFlag = 0;
uint32_t my_otp_crc;
uint32_t broadcast_crc;
uint32_t default_crc;
uint8_t alarm_source = FROM_SENSOR;
static void gpiote_init(void)
{
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}

/**@brief   Function for Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
	uint32_t err_code;
	// Initialize timer module
	err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);
 // Create timers.
	err_code = app_timer_create(&m_app_rtc_timer_id,
								APP_TIMER_MODE_REPEATED,
								app_rtc_timeout_handler);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting application timers.
 */
void application_timers_start(void)
{
    uint32_t err_code;
    // Start application timers.
    err_code = app_timer_start(m_app_rtc_timer_id, TAG_PROCESS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

void boards_init(void)
{
//	gpiote_init();
	timers_init();
}


#if LEDS_NUMBER > 0
bool bsp_board_led_state_get(uint32_t led_idx)
{
    ASSERT(led_idx < LEDS_NUMBER);
    bool pin_set = nrf_gpio_pin_out_read(m_board_led_list[led_idx]) ? true : false;
    return (pin_set == (LEDS_ACTIVE_STATE ? true : false));
}

void bsp_board_led_on(uint32_t led_idx)
{
        ASSERT(led_idx < LEDS_NUMBER);
        nrf_gpio_pin_write(m_board_led_list[led_idx], LEDS_ACTIVE_STATE ? 1 : 0);
}

void bsp_board_led_off(uint32_t led_idx)
{
    ASSERT(led_idx < LEDS_NUMBER);
    nrf_gpio_pin_write(m_board_led_list[led_idx], LEDS_ACTIVE_STATE ? 0 : 1);
}

void bsp_board_leds_off(void)
{
    uint32_t i;
    for(i = 0; i < LEDS_NUMBER; ++i)
    {
        bsp_board_led_off(i);
    }
}

void bsp_board_leds_on(void)
{
    uint32_t i;
    for(i = 0; i < LEDS_NUMBER; ++i)
    {
        bsp_board_led_on(i);
    }
}

void bsp_board_led_invert(uint32_t led_idx)
{
    ASSERT(led_idx < LEDS_NUMBER);
    nrf_gpio_pin_toggle(m_board_led_list[led_idx]);
}

void bsp_board_leds_init(void)
{
    uint32_t i;
    for(i = 0; i < LEDS_NUMBER; ++i)
    {
        nrf_gpio_cfg_output(m_board_led_list[i]);
    }
    bsp_board_leds_off();
}

uint32_t bsp_board_led_idx_to_pin(uint32_t led_idx)
{
    ASSERT(led_idx < LEDS_NUMBER);
    return m_board_led_list[led_idx];
}

uint32_t bsp_board_pin_to_led_idx(uint32_t pin_number)
{
    uint32_t ret = 0xFFFFFFFF;
    uint32_t i;
    for(i = 0; i < LEDS_NUMBER; ++i)
    {
        if (m_board_led_list[i] == pin_number)
        {
            ret = i;
            break;
        }
    }
    return ret;
}
#endif //LEDS_NUMBER > 0

#endif
