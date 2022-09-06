#include "app_message_queue.h"
#include "sensor_utils.h"
#include "sos_sw_driver.h"
#include "sos_sw_utils.h"
#include "lib/ringbuf.h"
#define SOS_SW_ID	0x99



#if (defined(BOARD_SOS))

/* Static variables */
static bool _sos_sw_status = false;

bool _sos_sw_status_sending = false;
static uint32_t _sos_change_cnt = 0;
extern uint8_t button_pressed_flag;
/* Static function prototypes */
static void _sos_sw_int_event_handler(uint32_t const *  event_pins_low_to_high, uint32_t const *  event_pins_high_to_low);

/* Public functions */
void sos_sw_init() {
//	vt_sensor_sos_init(_sos_sw_int_event_handler);
//	nrf_gpio_cfg_input(BTN_1, BUTTON_PULL);
//	nrf_gpio_cfg_input(IR_RECV_PIN, BUTTON_PULL);
	
	DEBUG_LED_INIT
}

void sos_sw_send_presentation() {
	protocol_send_presentation(SOS_SW_ID, ePROTOCOL_DEVICE_TYPE_SOS);
}

void sos_sw_send_status(uint8_t sequence) {
		_send_sos_sw_info(sequence);
}

uint32_t sos_get_cnt(void)
{
	return _sos_change_cnt;
}

/* Static functions */
static void _sos_sw_int_event_handler(uint32_t const * event_pins_low_to_high, uint32_t const *  event_pins_high_to_low) {
	
//	if((nrf_gpio_pin_read(BUTTON_1) == 1) && (button_pressed_flag == 0))
//	{
//		if (*event_pins_high_to_low) {
//			_sos_sw_status = true;
//		} else {
//			_sos_sw_status = false;
//		}
//		_sos_change_cnt++;
//	}
}

void _set_sos_status(bool status)
{
	_sos_sw_status = status;
}

void _clear_sos_status(void)
{
	_sos_sw_status = false;
}

bool _get_sos_status(void)
{
//	return (nrf_gpio_pin_read(SOS_SW_INPUT_PIN));
	return 0;
}


void _send_sos_sw_info(uint8_t sequence) {
//	uint8_t buff[PROTOCOL_PAYLOAD_LEN];

//	uint8_t len = 0;
//	buff[len++] = SOS_SW_ID;
//	buff[len++] = _sos_sw_status;
//	buff[len++] = sequence;
//	buff[len++] = 0;
//	memcpy(&buff[len], &rtcTimeSec, sizeof(rtcTimeSec));
//	len += 4;
//	if(_sos_sw_status)
//		protocol_send_data_to_gw(buff, len, ePROTOCOL_CMD_TYPE_SOS, 0x01,false);
//	else
//		protocol_send_data_to_gw(buff, len, ePROTOCOL_CMD_TYPE_SOS, 0x00,false);
}

#endif
