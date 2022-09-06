#include "nrf_gpio.h"
#include "app_gpiote.h"
#include "smarttouch_driver.h"

/* Static variables */
static app_gpiote_user_id_t _smarttouch_int_gpiote_uid = 0;

/* Public funtions */
uint32_t vt_sensor_smarttouch_init(app_gpiote_event_handler_t handler) {
	nrf_gpio_cfg_input(SMARTTOUCH_INPUT_PIN, NRF_GPIO_PIN_NOPULL);
	
	uint32_t gpiote_pin_low_high_mask = 1 << SMARTTOUCH_INPUT_PIN;
	uint32_t gpiote_pin_high_low_mask = 1 << SMARTTOUCH_INPUT_PIN;
	app_gpiote_user_register(&_smarttouch_int_gpiote_uid,
								gpiote_pin_low_high_mask, 
								gpiote_pin_high_low_mask,
								handler);
	app_gpiote_user_enable(_smarttouch_int_gpiote_uid);
	return 0;
}
