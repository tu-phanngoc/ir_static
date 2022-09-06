#include "btn.h"
//#include "ble_error_log.h"
#include "app_error.h"
#include "app_gpiote.h"
#include "app_util_platform.h"
#include "app_mesh.h"
#include "nrf_delay.h"
APP_TIMER_DEF(m_tag_button_timer_id);												/**< button process timer. */

static void tag_button_timeout_handler(void * p_context);
void gpiote_btn0_event_handler(uint32_t const * event_pins_low_to_high, uint32_t const * event_pins_high_to_low);
uint8_t button_click_cnt = 0;
uint32_t button_click_interval = 0;
uint32_t button_click_start_time = 0;

uint8_t button_click_times;
uint32_t button_press_time;
uint8_t button_pressed_flag;

uint8_t button_enable = 1;
static app_gpiote_user_id_t        btn0_gpiote_uid;
void btn_set_input_sense(void);
void btn_set_output_and_clear_pin(void);
void btn_init(void)
{
	uint32_t err_code;
	uint32_t gpiote_pin_low_high_mask = 0;
	uint32_t gpiote_pin_high_low_mask = 0;
	// Create timers.
	err_code = app_timer_create(&m_tag_button_timer_id,
								APP_TIMER_MODE_SINGLE_SHOT,
								tag_button_timeout_handler);
	APP_ERROR_CHECK(err_code); 
	
//	nrf_gpio_cfg_input(BTN_1, NRF_GPIO_PIN_NOPULL);
	
	gpiote_pin_low_high_mask = (1 << BTN_1);
	gpiote_pin_high_low_mask = (1 << BTN_1);
	app_gpiote_user_register(&btn0_gpiote_uid,
									&gpiote_pin_low_high_mask,
									&gpiote_pin_high_low_mask,
									gpiote_btn0_event_handler);
	
	app_gpiote_user_enable(btn0_gpiote_uid);
	#if defined(USE_PCA10040)
		uint32_t pin_number = BTN0;
		NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
		reg->PIN_CNF[pin_number] &= ~GPIO_PIN_CNF_PULL_Msk;
		reg->PIN_CNF[pin_number] |= ((uint32_t)GPIO_PIN_CNF_PULL_Pullup<<GPIO_PIN_CNF_PULL_Pos);
	#endif
}

void btn_set_output_and_clear_pin(void)
{
	button_enable = 0;
	app_gpiote_user_disable(btn0_gpiote_uid);
	nrf_gpio_pin_clear(BTN_1);
	nrf_gpio_cfg_output(BTN_1);
	nrf_gpio_pin_clear(BTN_1);
}

void btn_set_input_sense(void)
{
	//event_handler = gpiote_btn_event_handler;
	if(button_enable == 0)
	{
//		nrf_gpio_cfg_input(BTN_1, NRF_GPIO_PIN_NOPULL);
		app_gpiote_user_enable(btn0_gpiote_uid);
		#if defined(USE_PCA10040)
		uint32_t pin_number = BTN0;
		NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
		reg->PIN_CNF[pin_number] &= ~GPIO_PIN_CNF_PULL_Msk;
		reg->PIN_CNF[pin_number] |= ((uint32_t)GPIO_PIN_CNF_PULL_Pullup<<GPIO_PIN_CNF_PULL_Pos);
		#endif
		nrf_delay_us(100);
		button_click_cnt = 0;
		button_pressed_flag = 0;
		button_click_start_time = app_timer_cnt_get();
		button_enable = 1;
	}
}
volatile uint32_t btnIntCount = 0;

void gpiote_btn0_event_handler(uint32_t const * event_pins_low_to_high, uint32_t const * event_pins_high_to_low)
{
		uint32_t rtc_time,err_code;
		uint32_t button_click_interval_ms;
		rtc_time = app_timer_cnt_get();
		static bool _start_press = false;
		if(app_get_mode() != MESH_MODE)
		{
			app_set_mode(MESH_MODE);
		}
		if(button_enable)
		{
			if(*event_pins_high_to_low)
			{
				_start_press = true;
				button_click_cnt++;
				button_click_start_time = rtc_time;	
				err_code = app_timer_stop(m_tag_button_timer_id);
				APP_ERROR_CHECK(err_code);
			}
			else if(*event_pins_low_to_high)
			{
				if(!_start_press)
				{
						return;
				}
				
				err_code = app_timer_start(m_tag_button_timer_id, TAG_BUTTON_PROCESS_INTERVAL, NULL);
				APP_ERROR_CHECK(err_code);
				if(button_click_start_time > rtc_time)
				{
						button_click_interval = 0xFFFFFF - button_click_start_time + rtc_time;
				}
				else
				{
						button_click_interval = rtc_time - button_click_start_time;
				}
				button_click_interval_ms = button_click_interval/33;
				if(button_click_interval_ms < 100)
				{
					if(button_click_cnt > 0)
						button_click_cnt--;
				}
				_start_press = false;
			}
		}
}

static void tag_button_timeout_handler(void * p_context)
{
//		uint32_t err_code;
    UNUSED_PARAMETER(p_context);
		
	if(button_pressed_flag == 0 && button_enable)
	{
		button_click_times = button_click_cnt;
		button_press_time = button_click_interval>>15;//in sec
		button_pressed_flag = 1;
	}
	button_click_cnt = 0;		// number of button clicks
	button_click_interval = 0;	// button press time
}	
