/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
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
#ifndef AMPM_GT999_H
#define AMPM_GT999_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"



#define APP_GPIOTE_MAX_USERS            4

#if defined(NRF52811_PIN)

#define BUTTON_1      15

#define BUTTON_PULL   GPIO_PIN_CNF_PULL_Pullup
#define IR_RECV_PIN 	14 //sensor alarm pin

#define IR_LED_PIN		12	//10 //4
#define IR_GPIO_PIN		10
#elif defined (NRF52832_XXAA) || defined (NRF52832_XXAB)

#ifdef BENKON_HW_V1
//V1
#define BTN_1      15
#define BUTTON_PULL   GPIO_PIN_CNF_PULL_Pullup
#define IR_RECV_PIN 	14 //sensor alarm pin
#define IR_LED_PIN		12	//10 //4
#define DEBUG_LED_PIN	11
#define IR_GPIO_PIN		10
#define LED_R						11
#define LED_B						11
#define LED_G						11
#endif

#ifdef BENKON_HW_V2
//V2
#define BTN_1      		10
#define BUTTON_PULL   GPIO_PIN_CNF_PULL_Pullup		//GPIO_PIN_CNF_PULL_Pulldown /GPIO_PIN_CNF_PULL_Pullup
#define IR_RECV_PIN 	15 //sensor alarm pin
#define IR_LED_PIN		11		//11
#define DEBUG_LED_PIN	14
#define LED_R						16
#define LED_B						17
#define LED_G						18
#endif

#ifdef BENKON_HW_V3
//V3
#define BTN_1      		10
#define BUTTON_PULL   GPIO_PIN_CNF_PULL_Pullup		//GPIO_PIN_CNF_PULL_Pulldown /GPIO_PIN_CNF_PULL_Pullup
#define IR_RECV_PIN 	15 //sensor alarm pin
#define IR_LED_PIN		11		//11
#define DEBUG_LED_PIN	14
#define LED_R						16
#define LED_B						17
#define LED_G						18
#endif

#endif

#define IR_LED_PIN_INIT	nrf_gpio_cfg_output(IR_LED_PIN)
#define IR_LED_PIN_DEINIT	nrf_gpio_cfg_default(IR_LED_PIN)

#define IR_LED_PIN_SET 		nrf_gpio_pin_set(IR_LED_PIN)
#define IR_LED_PIN_CLEAR	nrf_gpio_pin_clear(IR_LED_PIN)

#define IR_LED_DEINIT	nrf_gpio_cfg_input(IR_LED_PIN, BUTTON_PULL);
#define IR_LED_INIT		{nrf_gpio_cfg_output(IR_LED_PIN); nrf_gpio_pin_clear(IR_LED_PIN);}

#define DEBUG_LED_INIT 		{nrf_gpio_cfg_output(DEBUG_LED_PIN); nrf_gpio_pin_set(DEBUG_LED_PIN);}
#define DEBUG_LED_DEINIT	nrf_gpio_cfg_input(DEBUG_LED_PIN, BUTTON_PULL);
#define DEBUG_LED_SET 		nrf_gpio_pin_set(DEBUG_LED_PIN)
#define DEBUG_LED_CLEAR				nrf_gpio_pin_clear(DEBUG_LED_PIN)
#define DEBUG_LED_TOGGLE	nrf_gpio_pin_toggle(DEBUG_LED_PIN)

#define LED_ON_G			nrf_gpio_pin_set(LED_G);
#define LED_OFF_G			nrf_gpio_pin_clear(LED_G);
#define LED_ON_R			nrf_gpio_pin_set(LED_R);
#define LED_OFF_R			nrf_gpio_pin_clear(LED_R);
#define LED_ON_B			nrf_gpio_pin_set(LED_B);
#define LED_OFF_B			nrf_gpio_pin_clear(LED_B);
// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

bool getTestMode();														 
#ifdef __cplusplus
}
#endif

#endif // AMPM_GT999_H
