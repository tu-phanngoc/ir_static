/**************************************************************************************************
/@ Project   :  Smart Home      
/@ File      :  sht2x_utils.c
/@ Author    :  
/@ Brief     :  SHT2x Sensors Utils functions for Application
**************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
*                                           Includes
* -------------------------------------------------------------------------------------------------
*/
#include "app_message_queue.h"
#include "sht2x_driver.h"
#include "sht2x_utils.h"


/* ------------------------------------------------------------------------------------------------
*                                           Defines
* -------------------------------------------------------------------------------------------------
*/
#if (defined(BOARD_STH2x))
#define TEMP_SENSOR_ID	0x00
#define HUMID_SENSOR_ID	0x01

#define TIMEOUT_TO_SEND_NEXT_INFO	1

/* ------------------------------------------------------------------------------------------------
*                                           Local Variables
* -------------------------------------------------------------------------------------------------
*/
static bool _is_need_to_send_humid_sensor_status = true;    // Keeps user register value
int16_t temperature;
int16_t humidity;
static uint8_t _remain_time_to_send_next_info = 0;
extern uint8_t sequence;
/* ------------------------------------------------------------------------------------------------
*                                           Private Functions
* -------------------------------------------------------------------------------------------------
*/

static void _send_humid_sensor_info(int16_t sensor_value, uint8_t mode,uint8_t sequence) {
	uint8_t buf[PROTOCOL_PAYLOAD_LEN];
	uint8_t len = 0;

	switch (mode) {
		case 0 :
			buf[len++] = TEMP_SENSOR_ID;
			buf[len++] = (sensor_value >> 8) & 0xFF;
			buf[len++] = sensor_value & 0xFF;
			buf[len++] = sequence;
			protocol_send_data_to_gw(buf, len, ePROTOCOL_CMD_TYPE_SET, ePROTOCOL_VALUE_TYPE_TEMP, true);
			break;
		case 1 :
			buf[len++] = HUMID_SENSOR_ID;
			buf[len++] = (sensor_value >> 8) & 0xFF;
			buf[len++] = sensor_value & 0xFF;
			buf[len++] = sequence;
			protocol_send_data_to_gw(buf, len, ePROTOCOL_CMD_TYPE_SET, ePROTOCOL_VALUE_TYPE_HUMID, true);
			break;
	}
}
/* ------------------------------------------------------------------------------------------------
*                                           Public Functions
* -------------------------------------------------------------------------------------------------
*/

void humidity_sensor_init() {
	vt_sensor_humidity_init();
}

void humidity_sensor_send_presentation() {
	protocol_send_presentation(TEMP_SENSOR_ID, ePROTOCOL_DEVICE_TYPE_TEMP);
	protocol_send_presentation(HUMID_SENSOR_ID, ePROTOCOL_DEVICE_TYPE_HUMID);
}
extern bool twi_master_init(void);

void humidity_sensor_send_data(uint8_t sequence) {
	
	vt_sensor_humidity_read(&temperature, &humidity);
	if (_is_need_to_send_humid_sensor_status) 
	{
		_is_need_to_send_humid_sensor_status = false;
		_send_humid_sensor_info(temperature, 0,sequence);
	}
	else
	{
		_is_need_to_send_humid_sensor_status = true;
		_send_humid_sensor_info(humidity, 1,sequence);
	}
//		_remain_time_to_send_next_info = TIMEOUT_TO_SEND_NEXT_INFO;
//	}
//	temperature = 0;
//	humidity = 0;
	twi_master_init();
}

void humidity_sensor_tick_1s_handler(void) {
//	if (_remain_time_to_send_next_info) {
//		_remain_time_to_send_next_info--;
//		if (!_remain_time_to_send_next_info) {
			_send_humid_sensor_info(humidity, 1,sequence);
//		}
//	}
}

void humidity_sensor_trigger_to_send_data() {
	_is_need_to_send_humid_sensor_status = true;
}

bool humidity_sensor_is_need_to_send(void) {
	return _is_need_to_send_humid_sensor_status;
}
#endif
