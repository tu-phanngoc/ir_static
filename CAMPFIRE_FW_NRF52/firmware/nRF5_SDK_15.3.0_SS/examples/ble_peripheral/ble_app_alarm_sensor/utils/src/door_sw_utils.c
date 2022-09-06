#include "app_message_queue.h"
#include "app_config_task.h"
#include "sensor_utils.h"
#include "door_sw_driver.h"
#include "door_sw_utils.h"
#include "lib/ringbuf.h"
#define DOOR_SW_ID	0x00

#if (defined(BOARD_DOOR))
/* Static variables */
static bool _door_sw_status = true;
bool _door_sw_status_sending = false;
/* Static function prototypes */
static void _door_sw_int_event_handler(uint32_t const * event_pins_low_to_high, uint32_t const * event_pins_high_to_low);

/* Public functions */
void door_sw_init() {
	vt_sensor_door_init(_door_sw_int_event_handler);
}

void door_sw_send_presentation() {
	protocol_send_presentation(DOOR_SW_ID, ePROTOCOL_DEVICE_TYPE_DOOR);
}

void door_sw_send_status(uint8_t sequence) {
		_send_door_sw_info(sequence);
}

/* Static functions */
static void _door_sw_int_event_handler(uint32_t const * event_pins_low_to_high, uint32_t const * event_pins_high_to_low) {
	if (*event_pins_low_to_high) {
		_door_sw_status = true;
	} else {
		_door_sw_status = false;
	}
	sensor_event_flag |= SENSOR_CHANGED_EVENT;
	RINGBUF_Put(&sensorsRingBuf,(uint8_t)_door_sw_status);
}

void _set_door_status(bool status)
{
	_door_sw_status_sending = status;
}

void _clear_door_status(void)
{
	_door_sw_status = false;
}

void _send_door_sw_info(uint8_t sequence){
	uint8_t buff[PROTOCOL_PAYLOAD_LEN];

	uint8_t len = 0;
	buff[len++] = DOOR_SW_ID;
	buff[len++] = _door_sw_status_sending;
	buff[len++] = sequence;
	buff[len++] = 0;
	memcpy(&buff[len], &rtcTimeSec, sizeof(rtcTimeSec));
	len += 4;
	protocol_send_data_to_gw(buff, len, ePROTOCOL_CMD_TYPE_STATUS, ePROTOCOL_VALUE_TYPE_STATUS,true);
}

#endif
