
#include "app_ble.h"
#include "lib/sys_time.h"
#include "rbc_mesh.h"
#include "timeslot_handler.h"
#include "mesh_utils.h"
#include "sensor_process.h"
#include "system_config.h"
#include "app_message_queue.h"
#include "sensor_utils.h"
#include "app_mesh.h"
#include "smart_sensor_srv.h"
#include "app_mesh.h"
#include "app_timer.h"
#include "app_sensor.h"
#include "adc_task.h"
#include "data_transmit_fsm.h"
#include "dataLogger_Interface.h"
#define DBG_APP_SENSOR(...) NRF_LOG_INFO(__VA_ARGS__)
#define SENSOR_ACK_TIMEOUT				2
//APP_TIMER_DEF(m_sensor_event_timer_id);
//APP_TIMER_DEF(m_sensor_recv_event_timer_id);

extern const uint8_t broadcast_addr[6];
ISMARTPACKAGE iSmartPacket;
static uint8_t flag_send_message_asap = 0;
static bool sensor_data_sending = false;
static uint8_t send_data_timeout = 0;

extern uint32_t sht_temperature;
extern uint8_t sht_humidity;

uint16_t u16_temperature = 0;
uint8_t u8_humidity = 0;

uint32_t gu32_emeter_energy = 0;
double gdouble_emeter_energy = 0;


extern tSMARTHOME_DATA mlib;
extern tSMARTHOME_RESULT result;
extern tSMARTHOME_RESULT meter_result;
extern double            umax;         /*!< maximal voltage measurement range      */
extern double            imax; 
extern uint32_t wh_cnt;
extern bool gb_is_gateway_alive;

uint16_t msg_cnt = 0;

static uint32_t tag_get_ramdom(uint32_t min, uint32_t max)
{
	return  min + (rand() % (max - min));
}

static void sensor_event_timeout_handler(void * p_context)
{
	NRF_LOG_INFO("sensor_event_timeout_handler");
}

static void sensor_recv_event_timeout_handler(void * p_context)
{

}


void app_sensor_init(void)
{
}
uint32_t u32_cnt = 0;
void app_sensor_task(void)
{
	if(sysCfg.enable_sensor_streaming == 0)
	{
		return;
	}
	msg_cnt++;
	uint32_t err_code;
	NRF_LOG_INFO("Send Sensor data..................................., u32_cnt: %d", u32_cnt);
	uint8_t data[10];
	#ifdef BENKON_HW_V1
	static uint16_t fakeTemp = 250;
	fakeTemp++;
	if(fakeTemp > 300)
	{
		fakeTemp = 250;
	}
	#endif
	#ifdef BENKON_HW_V2
	uint16_t fakeTemp = sht_temperature/10;
	uint8_t fakeHumi = sht_humidity;
	#endif
	#ifdef BENKON_HW_V3
	uint16_t fakeTemp = sht_temperature/10;
	uint8_t fakeHumi = sht_humidity;
	#endif
	/* */
	data[0] = rtcTimeSec;
	data[1] = (rtcTimeSec >> 8) & 0xFF;
	data[2] = (rtcTimeSec >> 16) & 0xFF;
	data[3] = (rtcTimeSec >> 24) & 0xFF;
	
	data[4] = (uint8_t)(fakeTemp & 0xFF);
	data[5] = (uint8_t)((fakeTemp >> 8) & 0xFF);
	
	#ifdef BENKON_HW_V1
	static uint16_t fakeHumi = 50;
	fakeHumi++;
	if(fakeHumi > 85)
	{
		fakeHumi = 50;
	}
	#endif
	
	data[6] = (uint8_t)fakeHumi;
	data[7] = 0;
	data[8] = u32_cnt & 0xFF;
	data[9] = (u32_cnt >> 8) & 0xFF;
	
	if(data[0] == 0x17 && data[3] == 0x00)
	{
		data[1] = 0x01;
		data[2] = 0x01;
		data[3] = 0x01;
	}
	transmit_fsm_prepare_payload(data, 0x12, 0x01, 10);
	u32_cnt++;
	/* */
}

double d_cosphi = 0.8;

void app_emeter_task(void)
{
	if(sysCfg.enable_power_streaming == 0)
	{
		return;
	}
	
	NRF_LOG_INFO("app_emeter_task...................................");
	/* */
	uint32_t u32Power = 0;
	uint32_t u32V = 0;
	uint32_t u32I = 0;
	uint8_t data[10];
	uint8_t tempBuff[32];
	/* */
	data[0] = rtcTimeSec;
	data[1] = (rtcTimeSec >> 8) & 0xFF;
	data[2] = (rtcTimeSec >> 16) & 0xFF;
	data[3] = (rtcTimeSec >> 24) & 0xFF;

	SMARTHOME_ReadResults((tSMARTHOME_DATA *)&mlib, &meter_result, &umax, &imax);
	NRF_LOG_INFO("u_rms: %d, i_rms: %d, P: %d W, E: %d", meter_result.urms * 1000, meter_result.irms[0] * 1000, meter_result.p[0] * 1000, wh_cnt); 
	
	u32Power = meter_result.p[0];
	data[4] = u32Power & 0xFF;
	data[5] = (u32Power >> 8) & 0xFF;
	
	data[6] = (wh_cnt) & 0xFF;
	data[7] = (wh_cnt >> 8) & 0xFF;
	data[8] = (wh_cnt >> 16) & 0xFF;
	data[9] = (wh_cnt >> 24) & 0xFF;

	transmit_fsm_prepare_payload(data, 0x12, 0x02, 10);
	
}

void app_meter_init(void)
{
	uint32_t res = dataLogger_GetLatestValid(METER_ENERGY_RECORD_LOG, &wh_cnt);
	res = dataLogger_GetLatestValid(METER_ENERGY_RECORD_LOG, &wh_cnt);
	if(res == DATA_LOGGER_STT_SUCCESS) 
	{
		NRF_LOG_INFO("GET ENERGY SUCCESS %d.........", wh_cnt);
	}
	else
	{
		NRF_LOG_INFO("GET ENERGY FAILED ..........");
	}

	nrf_delay_ms(300);
	uint32_t u32Rtc = 0;
	res = dataLogger_GetLatestValid(RTC_RECORD_LOG, &u32Rtc);
	rtcTimeSec = u32Rtc;
	if(res == DATA_LOGGER_STT_SUCCESS) 
	{
		NRF_LOG_INFO("GET RTC SUCCESS %d .........", rtcTimeSec);
	}
	else
	{
		NRF_LOG_INFO("GET RTC FAILED ..........");
	}
	
	SMARTHOME_ReadResults((tSMARTHOME_DATA *)&mlib, &meter_result, &umax, &imax);
}

void app_set_humidity_value(uint8_t val)
{
	u8_humidity = val;
}

uint8_t app_get_humidity_value()
{
	return u8_humidity;
}

void app_set_temperature_value(uint8_t val)
{
	u16_temperature = val;
}

uint8_t app_get_temperature_value()
{
	return u16_temperature;
}
