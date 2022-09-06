#include "stdint.h"
#include "nrf_delay.h"

#include "app_led_animation.h"
#include "app_indicator_led.h"

#define LED_BLINK_PERIOD_MS			500
#define LED_BLINK_REPEAT				3
#define LED_BLINK_ON_PERIOD_MS	50
uint32_t led_animation_tick = 0;
uint8_t led_animation_trigger = 0;
volatile uint8_t led_animation_on_type = 0;


uint32_t led_blink_tick = 0;
uint32_t led_blink_trigger = 0;
uint8_t led_blink_type = 0;
uint8_t app_led_number_of_blink = LED_BLINK_REPEAT;

void app_led_blink_trigger(uint8_t type)
{
	NRF_LOG_INFO("app_led_blink_trigger");
	led_blink_trigger = 1;
	app_led_number_of_blink = 0;
	led_blink_tick = LED_BLINK_PERIOD_MS;
	led_blink_type = type;
}

void app_blink_tick()
{
	if(led_blink_tick > 0)
	{
		led_blink_tick--;
	}
}

void app_led_blink_task()
{
	if(led_blink_trigger == 1)
	{
		if(app_led_number_of_blink < LED_BLINK_REPEAT && led_blink_tick == 0)
		{
			app_led_number_of_blink++;
			led_blink_tick = LED_BLINK_PERIOD_MS;
			NRF_LOG_INFO("app_led_blink_task");
			app_led_animation_trigger(LED_BLINK_ON_PERIOD_MS, led_blink_type);
		}
		if(app_led_number_of_blink >= LED_BLINK_REPEAT)
		{
			led_blink_trigger = 0;
			led_blink_tick = 0;
				led_blink_type = 0;
		}
	}
}

void app_led_animation_tick()
{
	if(led_animation_tick > 0)
	{
		led_animation_tick--;
	}
}

void app_led_animation_trigger(uint32_t timeout, uint8_t type)
{
	NRF_LOG_INFO("app_led_animation_trigger %d - %d", timeout, type);
	led_animation_tick = timeout;
	led_animation_on_type = type;
	led_animation_trigger = 1;
}

void app_led_animation_task()
{
	if(led_animation_tick > 0 && led_animation_trigger > 0)
	{
		if(led_animation_on_type > 0)
		{
			if(led_animation_on_type == LED_ANIMATION_TRIGGER_RED)
			{
				//led on
				app_indicator_led_on(RGB_TYPE_R);
			}
			else if(led_animation_on_type == LED_ANIMATION_TRIGGER_GREEN)
			{
				app_indicator_led_on(RGB_TYPE_G);
			}
			else if(led_animation_on_type == LED_ANIMATION_TRIGGER_BLUE)
			{
				app_indicator_led_on(RGB_TYPE_B);
			}
		}
	}
	else
	{
		//led off
		app_indicator_led_off(RGB_TYPE_R);
		app_indicator_led_off(RGB_TYPE_G);
		app_indicator_led_off(RGB_TYPE_B);
		//
		led_animation_trigger = 0;
		led_animation_on_type = 0;
	}
}


