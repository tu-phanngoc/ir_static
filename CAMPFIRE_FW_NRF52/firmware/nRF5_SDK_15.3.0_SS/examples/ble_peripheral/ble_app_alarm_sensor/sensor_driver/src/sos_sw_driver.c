#include "nrf_gpio.h"
#include "app_gpiote.h"
#include "sos_sw_driver.h"
#if (defined(BOARD_SOS))
/* Static variables */
static app_gpiote_user_id_t _sos_sw_int_gpiote_uid = 0;

/* Public funtions */
uint32_t vt_sensor_sos_init(app_gpiote_event_handler_t handler) {
	
	#if defined(USE_PCA10040)
	nrf_gpio_cfg_input(SOS_SW_INPUT_PIN, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_input(BAT_STATUS, NRF_GPIO_PIN_PULLUP);
	#else
//	nrf_gpio_cfg_input(BTN_1, BUTTON_PULL);
//	nrf_gpio_cfg_input(IR_RECV_PIN, NRF_GPIO_PIN_PULLDOWN);

	#endif
	
//	uint32_t gpiote_pin_low_high_mask = 1 << SOS_SW_INPUT_PIN;
//	uint32_t gpiote_pin_high_low_mask = 1 << SOS_SW_INPUT_PIN;
//	app_gpiote_user_register(&_sos_sw_int_gpiote_uid,
//								&gpiote_pin_low_high_mask, 
//								&gpiote_pin_high_low_mask,
//								handler);
//	app_gpiote_user_enable(_sos_sw_int_gpiote_uid);
	
//	uint32_t pin_number = SOS_SW_INPUT_PIN;
//	NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&pin_number);
//	reg->PIN_CNF[pin_number] &= ~GPIO_PIN_CNF_PULL_Msk;
//	reg->PIN_CNF[pin_number] |= ((uint32_t)GPIO_PIN_CNF_PULL_Pullup<<GPIO_PIN_CNF_PULL_Pos);
	return 0;
}

#endif
