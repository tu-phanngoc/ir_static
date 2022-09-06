#include <stdbool.h>
#include "app_gpiote.h"
#include "app_message_queue.h"
#if defined(BOARD_PIR)
#include "pir_sensor_utils.h"
#endif
#if defined(BOARD_SOS)
#include "sos_sw_utils.h"
#endif
#if defined(BOARD_DOOR)
#include "door_sw_utils.h"
#endif
#include "sensor_utils.h"
#if defined(BOARD_SHT2X)
#include "sht2x_utils.h"
#endif
#if defined(BOARD_SMARTTOUCH)
#include "smarttouch_utils.h"
#endif
#include "lib/ringbuf.h"

RINGBUF sensorsRingBuf;
uint8_t sensorsBuf[10];
uint8_t sequence;
/* Static functions */

/* Public functions */
void sensor_init() {
	/* Initialize sensors */
	RINGBUF_Init(&sensorsRingBuf,sensorsBuf,sizeof(sensorsBuf));
#if defined(BOARD_PIR)
	pir_sensor_init();
#endif
#if defined(BOARD_SOS)
	sos_sw_init();
#endif
	
#if defined(BOARD_DOOR)
	door_sw_init();
#endif
#if defined(BOARD_SHT2X)
	humidity_sensor_init();
#endif
#if defined(BOARD_SMARTTOUCH)
	smarttouch_init();
#endif
}

uint8_t sensor_got_sequence(void) {
	return sequence;
}
void sensor_sequence_inc(void) {
	sequence++;
}

uint32_t sensor_get_change_cnt(void) {
	#if defined(BOARD_PIR)

#endif
#if defined(BOARD_SOS)
	return sos_get_cnt();
#endif
#if defined(BOARD_DOOR)
	return 0;
	//return door_get_cnt();
#endif
#if defined(BOARD_SHT2X)

#endif
#if defined(BOARD_SMARTTOUCH)

#endif
	
}



void sensor_send_info(void) {
	
#if defined(BOARD_PIR)
	pir_sensor_send_status(sequence);
#endif
#if defined(BOARD_SOS)
	sos_sw_send_status(sequence);
#endif
#if defined(BOARD_DOOR)
	door_sw_send_status(sequence);
#endif
#if defined(BOARD_SHT2X)
	humidity_sensor_send_data(sequence);
#endif
#if defined(BOARD_SMARTTOUCH)
	smarttouch_send_status(sequence);
#endif
}

void set_sensor_status(bool status)
{
#if defined(BOARD_PIR)
	_set_pir_status(status);
#endif
#if defined(BOARD_SOS)
	_set_sos_status(status);
#endif
#if defined(BOARD_DOOR)
	_set_door_status(status);
#endif
#if defined(BOARD_SMARTTOUCH)
	_set_smarttouch_status(status);
#endif
}


void clear_sensor_status(void)
{
#if defined(BOARD_PIR)

#endif
#if defined(BOARD_SOS)
	_clear_sos_status();
#endif
#if defined(BOARD_DOOR)
	//_clear_door_status();
#endif
#if defined(BOARD_SMARTTOUCH)
	
#endif
}


bool get_sensor_status(void)
{
#if defined(BOARD_SMARTTOUCH)
	return _get_smarttouch_status();
#endif
#if defined(BOARD_SOS)
	return _get_sos_status();
#endif
#if defined(BOARD_DOOR)
	//return _get_door_status();
#endif
	return 0;

}



void sensor_trigger_to_send_data() {

}

void sensor_tick_1s_handler(void) {
#if defined(BOARD_SHT2X)
	humidity_sensor_tick_1s_handler();
#endif
}

/* Static functions */
