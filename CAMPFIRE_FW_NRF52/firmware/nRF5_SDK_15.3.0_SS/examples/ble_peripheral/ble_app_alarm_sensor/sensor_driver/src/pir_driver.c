#include "nrf_gpio.h"
#include "app_gpiote.h"
#include "pir_driver.h"
#if (defined(BOARD_PIR))
/* Static variables */
static app_gpiote_user_id_t pir_sensor_int_gpiote_uid = 0;

/* Public funtions */
uint32_t vt_sensor_pir_init(app_gpiote_event_handler_t handler) {
	nrf_gpio_cfg_input(PIR_SENSOR_PIN, PIR_SENSOR_PIN_PULL);
	
	uint32_t gpiote_pin_low_high_mask = 1 << PIR_SENSOR_PIN;
	uint32_t gpiote_pin_high_low_mask = 1 << PIR_SENSOR_PIN;
	app_gpiote_user_register(&pir_sensor_int_gpiote_uid,
								&gpiote_pin_low_high_mask, 
								&gpiote_pin_high_low_mask,
								handler);
	app_gpiote_user_enable(pir_sensor_int_gpiote_uid);
	
	return 0;
}

#endif
