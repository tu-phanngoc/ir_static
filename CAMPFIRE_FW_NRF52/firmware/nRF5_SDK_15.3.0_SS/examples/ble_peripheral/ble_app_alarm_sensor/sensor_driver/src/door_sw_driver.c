#include "nrf_gpio.h"
#include "app_gpiote.h"

#if (defined(BOARD_DOOR))
#include "door_sw_driver.h"

/* Static variables */
static app_gpiote_user_id_t _door_sw_int_gpiote_uid = 0;

/* Public funtions */
uint32_t vt_sensor_door_init(app_gpiote_event_handler_t handler) {
	nrf_gpio_cfg_input(DOOR_SW_INPUT_PIN, BUTTON_PULL);
	
	uint32_t gpiote_pin_low_high_mask = 1 << DOOR_SW_INPUT_PIN;
	uint32_t gpiote_pin_high_low_mask = 1 << DOOR_SW_INPUT_PIN;
	app_gpiote_user_register(&_door_sw_int_gpiote_uid,
								&gpiote_pin_low_high_mask, 
								&gpiote_pin_high_low_mask,
								handler);
	app_gpiote_user_enable(_door_sw_int_gpiote_uid);
	
	return 0;
}

#endif
