#include "app_message_queue.h"
#include "sensor_utils.h"
#include "smarttouch_driver.h"
#include "smarttouch_utils.h"
#include "lib/ringbuf.h"
#define SMARTTOUCH_ID	0x00

/* Static variables */
static bool _smarttouch_status = true;
bool _smarttouch_status_sending = false;
/* Static function prototypes */
static void _smarttouch_int_event_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low);

/* Public functions */
void smarttouch_init() {
	vt_sensor_smarttouch_init(_smarttouch_int_event_handler);
}

void smarttouch_send_presentation() {
	protocol_send_presentation(SMARTTOUCH_ID, ePROTOCOL_DEVICE_TYPE_DOOR);
}

void smarttouch_send_status(uint8_t sequence) {
		_send_smarttouch_info(sequence);
}

/* Static functions */
static void _smarttouch_int_event_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low) {
	if (event_pins_low_to_high) {
		//sequence++;
		_smarttouch_status_sending ^= 1;
		_smarttouch_status = true;
	} else {
		_smarttouch_status = false;
	}
	sensor_event_flag |= SENSOR_CHANGED_EVENT;
	uint8_t status;
	if(RINGBUF_GetFill(&sensorsRingBuf) >= sensorsRingBuf.size)
		RINGBUF_Get(&sensorsRingBuf,&status);
	RINGBUF_Put(&sensorsRingBuf,(uint8_t)_smarttouch_status);
}

void _set_smarttouch_status(bool status)
{
	_smarttouch_status_sending = status;
}

bool _get_smarttouch_status(void)
{
	return _smarttouch_status_sending;
}

void _send_smarttouch_info(uint8_t sequence) {
	uint8_t buff[PROTOCOL_PAYLOAD_LEN];
	uint8_t len = 0;
	buff[len++] = SMARTTOUCH_ID;
	buff[len++] = _smarttouch_status_sending;
	buff[len++] = sequence;
	protocol_send_data_to_gw(buff, len, ePROTOCOL_CMD_TYPE_SET, ePROTOCOL_VALUE_TYPE_STATUS,true);
}
