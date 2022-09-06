#include "app_message_queue.h"
#include "pir_driver.h"
#include "pir_sensor_utils.h"
#include "sensor_utils.h"
#include "lib/ringbuf.h"
#define PIR_SENSOR_ID	0x01
#if (defined(BOARD_PIR))

/* Static variables */
static bool _pir_sensor_status = false;
bool _pir_sw_status_sending = false;
/* Static function prototypes */
static void _pir_sensor_int_event_handler(uint32_t const * event_pins_low_to_high, uint32_t const * event_pins_high_to_low);
static void _send_pir_sensor_info(uint8_t sequence);

/* Public functions */
void pir_sensor_init() {
	vt_sensor_pir_init(_pir_sensor_int_event_handler);
}

void pir_sensor_send_presentation() {
	protocol_send_presentation(PIR_SENSOR_ID, ePROTOCOL_DEVICE_TYPE_MOTION);
}

void pir_sensor_send_status(uint8_t sequence) {
		_send_pir_sensor_info(sequence);
}

void pir_sensor_trigger_to_send_status() {

}

/* Static functions */
static void _pir_sensor_int_event_handler(uint32_t const * event_pins_low_to_high, uint32_t const * event_pins_high_to_low) {
	if (*event_pins_low_to_high) {
		_pir_sensor_status = true;
	} else {
		_pir_sensor_status = false;
	}
	sensor_event_flag |= SENSOR_CHANGED_EVENT;
	uint8_t status;
	if(RINGBUF_GetFill(&sensorsRingBuf) >= sensorsRingBuf.size)
		RINGBUF_Get(&sensorsRingBuf,&status);
	RINGBUF_Put(&sensorsRingBuf,(uint8_t)_pir_sensor_status);
}

void _set_pir_status(bool status)
{
	_pir_sw_status_sending = status;
}


static void _send_pir_sensor_info(uint8_t sequence) {
	uint8_t buff[PROTOCOL_PAYLOAD_LEN];

	uint8_t len = 0;
	buff[len++] = PIR_SENSOR_ID;
	buff[len++] = _pir_sw_status_sending;
	buff[len++] = sequence;
	buff[len++] = 0;
	memcpy(&buff[len], &rtcTimeSec, sizeof(rtcTimeSec));
	len += 4;
	protocol_send_data_to_gw(buff, len, ePROTOCOL_CMD_TYPE_STATUS, ePROTOCOL_VALUE_TYPE_STATUS,true);
}

#endif

