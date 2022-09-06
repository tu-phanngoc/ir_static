#include "IR_HAL.h"
#include "ampm_gt999.h"
#include "nrf_log.h"
#define PWM_PERIOD_US		26
#define MODE_LED			//MODE_LED, MODE_GPIO

uint32_t u32Tick = 0;


void HAL_Delay_50us(uint32_t Delay);


void IR_PinSet() {
	IR_LED_PIN_SET;
	
//	IR_LED_PIN_CLEAR;
}

void IR_PinClear() {
	IR_LED_PIN_CLEAR;
	
//	IR_LED_PIN_SET;
}

void IR_GPIO_PinSet() {
//	nrf_gpio_pin_set(IR_GPIO_PIN);
}

void IR_GPIO_PinClear() {
//	nrf_gpio_pin_clear(IR_GPIO_PIN);
}

void IR_SendPulse(uint32_t Interval_us)
{
	#ifdef MODE_GPIO
	// IR_GPIO
	IR_GPIO_PinClear();
	IR_PinClear();
	#endif
	
	#ifdef MODE_LED
	//IR_LED Pulse
	uint32_t pulseCount = Interval_us/PWM_PERIOD_US;
	for(uint32_t i = 0; i<pulseCount; i++)
	{
		IR_PinClear();
		HAL_Delay_us(PWM_PERIOD_US/2);
		IR_PinSet();
		HAL_Delay_us(PWM_PERIOD_US/2);
//		IR_PinSet();
	}
	#endif
	
	#ifdef MODE_GPIO
	//IR_GPIO
	
	#ifndef MODE_LED
//	HAL_Delay_us(Interval_us);
	HAL_Delay_50us(Interval_us/50);
	#endif
	
	IR_GPIO_PinSet();
	IR_PinSet();
	#endif
}

void IR_SendPause(uint32_t Interval_us)
{
	IR_PinSet();
	HAL_Delay_us(Interval_us);
}

void Timer_Inc_Tick()
{
	u32Tick += 16;
}

