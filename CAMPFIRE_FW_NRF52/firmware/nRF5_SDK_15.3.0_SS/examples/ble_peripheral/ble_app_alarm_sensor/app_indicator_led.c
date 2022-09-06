#include "stdint.h"
#include "nrf_delay.h"
#include "app_indicator_led.h"
uint8_t pwm_percent = 1;

uint8_t red_led_state = 0;
uint8_t blue_led_state = 0;
uint8_t green_led_state = 0;



void app_led_task()
{
	static uint16_t local_percent_count = 0;
	
	if(pwm_percent == 0)
	{
			LED_OFF_B;
			LED_OFF_R;
			LED_OFF_G;
	}
	else 
	{
		if(local_percent_count <= pwm_percent)
		{
			//LED_ON
			if(blue_led_state == 1)
			{
				LED_ON_B;
			}
			else
			{
				LED_OFF_B;
			}
			
			if(green_led_state == 1)
			{
				LED_ON_G;
			}
			else
			{
				LED_OFF_G;
			}
			
			if(red_led_state == 1)
			{
				LED_ON_R;
			}
			else
			{
				LED_OFF_R;
			}
		}
		else if(local_percent_count > pwm_percent)
		{
			//LED OFF
			LED_OFF_B;
			LED_OFF_R;
			LED_OFF_G;
		}
		
		local_percent_count++;
		if(local_percent_count >= 20)
		{
			local_percent_count = 0;
		}
	}
}

void set_led_pwm(uint8_t val)
{
	if((val / 5) > 0)
	{
		pwm_percent = val/5;
	}
	else
	{
		pwm_percent = 1;
	}
}

uint8_t get_led_pwm(void)
{
	return pwm_percent;
}

void app_indicator_led_on(uint8_t led_type)
{
	if(led_type == RGB_TYPE_R)
	{
		red_led_state = 1;
	}
	if(led_type == RGB_TYPE_G)
	{
		green_led_state = 1;
	}
	if(led_type == RGB_TYPE_B)
	{
		blue_led_state = 1;
	}
}

void app_indicator_led_off(uint8_t led_type)
{
		if(led_type == RGB_TYPE_R)
	{
		red_led_state = 0;
	}
	if(led_type == RGB_TYPE_G)
	{
		green_led_state = 0;
	}
	if(led_type == RGB_TYPE_B)
	{
		blue_led_state = 0;
	}
}