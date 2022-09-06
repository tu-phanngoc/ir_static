#include <stdint.h>
#include <stdbool.h>
#include "nrf_gpio.h"
#include "boards.h"
#include "touch_utils.h"

typedef struct _touch_btn_info_t {
	uint32_t red_led_pin;
	uint32_t blue_led_pin;
	uint32_t relay_pin;
} touch_btn_info_t;

static const touch_btn_info_t btn_info_a[] = {
	{
		.red_led_pin = LED_TS0_RED,
		.blue_led_pin = LED_TS0_BLUE,
		.relay_pin = RELAY1_PIN
	},
	{
		.red_led_pin = LED_TS1_RED,
		.blue_led_pin = LED_TS1_BLUE,
		.relay_pin = RELAY2_PIN
	},
	{
		.red_led_pin = LED_TS2_RED,
		.blue_led_pin = LED_TS2_BLUE,
		.relay_pin = RELAY3_PIN
	},
	{
		.red_led_pin = LED_TS3_RED,
		.blue_led_pin = LED_TS3_BLUE,
		.relay_pin = RELAY4_PIN
	}
};

/* Function prototype for private functions*/
static void _init_touch_button(touch_btn_no_e btn);
static void _set_default_look(touch_btn_no_e btn);

/* Public functions */
void touch_btn_init() {
#if (NO_TOUCH_BUTTON >= 1)
	_init_touch_button(eTOUCH_BUTTON_NO_0);
#endif
#if (NO_TOUCH_BUTTON >= 2)
	_init_touch_button(eTOUCH_BUTTON_NO_1);
#endif
#if (NO_TOUCH_BUTTON >= 3)
	_init_touch_button(eTOUCH_BUTTON_NO_2);
#endif
#if (NO_TOUCH_BUTTON == 4)
	_init_touch_button(eTOUCH_BUTTON_NO_3);
#endif
}

void touch_btn_update_look(uint8_t status) {
#if (NO_TOUCH_BUTTON >= 1)
	touch_btn_update_single_look(eTOUCH_BUTTON_NO_0, status & (1 << eTOUCH_BUTTON_NO_0));
#endif
#if (NO_TOUCH_BUTTON >= 2)
	touch_btn_update_single_look(eTOUCH_BUTTON_NO_1, status & (1 << eTOUCH_BUTTON_NO_1));
#endif
#if (NO_TOUCH_BUTTON >= 3)
	touch_btn_update_single_look(eTOUCH_BUTTON_NO_2, status & (1 << eTOUCH_BUTTON_NO_2));
#endif
#if (NO_TOUCH_BUTTON == 4)
	touch_btn_update_single_look(eTOUCH_BUTTON_NO_3, status & (1 << eTOUCH_BUTTON_NO_3));
#endif
}

void touch_btn_set_default_look() {
#if (NO_TOUCH_BUTTON >= 1)
	_set_default_look(eTOUCH_BUTTON_NO_0);
#endif
#if (NO_TOUCH_BUTTON >= 2)
	_set_default_look(eTOUCH_BUTTON_NO_1);
#endif
#if (NO_TOUCH_BUTTON >= 3)
	_set_default_look(eTOUCH_BUTTON_NO_2);
#endif
#if (NO_TOUCH_BUTTON == 4)
	_set_default_look(eTOUCH_BUTTON_NO_3);
#endif
}

void touch_btn_update_single_look(touch_btn_no_e btn, bool status) {
	if (status) {
		nrf_gpio_pin_set(btn_info_a[btn].relay_pin);
		nrf_gpio_pin_clear(btn_info_a[btn].red_led_pin);
		nrf_gpio_pin_set(btn_info_a[btn].blue_led_pin);
	} else {
		nrf_gpio_pin_clear(btn_info_a[btn].relay_pin);
		nrf_gpio_pin_clear(btn_info_a[btn].blue_led_pin);
		nrf_gpio_pin_set(btn_info_a[btn].red_led_pin);
	}
}

/* Private functions*/
static void _init_touch_button(touch_btn_no_e btn) {
	nrf_gpio_cfg_output(btn_info_a[btn].red_led_pin);
	nrf_gpio_cfg_output(btn_info_a[btn].blue_led_pin);
	nrf_gpio_cfg_output(btn_info_a[btn].relay_pin);
}

static void _set_default_look(touch_btn_no_e btn) {
	nrf_gpio_pin_clear(btn_info_a[btn].relay_pin);
	nrf_gpio_pin_clear(btn_info_a[btn].blue_led_pin);
	nrf_gpio_pin_clear(btn_info_a[btn].red_led_pin);
}
