#include <string.h>
#include "ble_gap.h"
#include "app_uart.h"
#include "lib/protocol.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "lib/ampm_list.h"
#include "aes128.h"
#include "app_message_queue.h"
#include "app_mesh.h"
#include "smart_sensor_srv.h"
#include "nrf_log.h"
#include "app_ac_status.h"
#include "app_datalog.h"
#include "data_transmit_fsm.h"



bool bEnableSendingToMesh = false;
bool bSendingToMesh = false;
extern bool gb_is_gateway_alive;

void protocol_send_IR_data(uint8_t* data) {
	if(sysCfg.enable_remote_data_streaming == 0)
	{
		return;
	}
	NRF_LOG_INFO("protocol_send_IR_data\r\n");
	static uint8_t sequence = 0;
	uint8_t buff[PROTOCOL_PAYLOAD_LEN];
	int8_t len = 0;
	ac_control_update_status_to_payload();
	buff[0] = rtcTimeSec;
	buff[1] = (rtcTimeSec >> 8) & 0xFF;
	buff[2] = (rtcTimeSec >> 16) & 0xFF;
	buff[3] = (rtcTimeSec >> 24) & 0xFF;
	buff[4] = ac_status_payload[0];
	buff[5] = ac_status_payload[1];
	buff[6] = ac_status_payload[2];
	buff[7] = ac_status_payload[3];
	buff[8] = ac_status_payload[4];
	buff[9] = ac_status_payload[5];
	
	sysCfg.ac_payload[0] = ac_status_payload[0];
	sysCfg.ac_payload[1] = ac_status_payload[1];
	sysCfg.ac_payload[2] = ac_status_payload[2];
	sysCfg.ac_payload[3] = ac_status_payload[3];
	sysCfg.ac_payload[4] = ac_status_payload[4];
	sysCfg.ac_payload[5] = ac_status_payload[5];
	
	CFG_Saving_Trigger();

//	if(gb_is_gateway_alive)
	{
		ISMARTPACKAGE iSmartPacket;
		transmit_fsm_prepare_payload(buff, 0x12, MESH_MSG_ID_REALTIME_IR, 10);
	}
}


void protocol_send_schedule_event_data(uint8_t* data) {
	if(sysCfg.enable_remote_data_streaming == 0)
	{
		return;
	}
	
	NRF_LOG_INFO("protocol_send_schedule_event_data\r\n");
	static uint8_t sequence = 0;
	uint8_t buff[PROTOCOL_PAYLOAD_LEN];
	int8_t len = 0;
	ac_control_update_status_to_payload();
	buff[0] = rtcTimeSec;
	buff[1] = (rtcTimeSec >> 8) & 0xFF;
	buff[2] = (rtcTimeSec >> 16) & 0xFF;
	buff[3] = (rtcTimeSec >> 24) & 0xFF;
	buff[4] = ac_status_payload[0];
	buff[5] = ac_status_payload[1];
	buff[6] = ac_status_payload[2];
	buff[7] = ac_status_payload[3];
	buff[8] = ac_status_payload[4];
	buff[9] = ac_status_payload[5];
	
	sysCfg.ac_payload[0] = ac_status_payload[0];
	sysCfg.ac_payload[1] = ac_status_payload[1];
	sysCfg.ac_payload[2] = ac_status_payload[2];
	sysCfg.ac_payload[3] = ac_status_payload[3];
	sysCfg.ac_payload[4] = ac_status_payload[4];
	sysCfg.ac_payload[5] = ac_status_payload[5];
	
	CFG_Saving_Trigger();

	{
		transmit_fsm_prepare_payload(buff, 0x12, MESH_MSG_ID_REALTIME_SCHEDULE_EVENT, 10);
	}
}


void protocol_send_timer_event_data(uint8_t* data) {
	if(sysCfg.enable_timer_event_streaming == 0)
	{
		return;
	}
	NRF_LOG_INFO("protocol_send_timer_event_data\r\n");
	static uint8_t sequence = 0;
	uint8_t buff[PROTOCOL_PAYLOAD_LEN];
	int8_t len = 0;
	ac_control_update_status_to_payload();
	buff[0] = rtcTimeSec;
	buff[1] = (rtcTimeSec >> 8) & 0xFF;
	buff[2] = (rtcTimeSec >> 16) & 0xFF;
	buff[3] = (rtcTimeSec >> 24) & 0xFF;
	buff[4] = ac_status_payload[0];
	buff[5] = ac_status_payload[1];
	buff[6] = ac_status_payload[2];
	buff[7] = ac_status_payload[3];
	buff[8] = ac_status_payload[4];
	buff[9] = ac_status_payload[5];
	
	sysCfg.ac_payload[0] = ac_status_payload[0];
	sysCfg.ac_payload[1] = ac_status_payload[1];
	sysCfg.ac_payload[2] = ac_status_payload[2];
	sysCfg.ac_payload[3] = ac_status_payload[3];
	sysCfg.ac_payload[4] = ac_status_payload[4];
	sysCfg.ac_payload[5] = ac_status_payload[5];
	
	CFG_Saving_Trigger();

	transmit_fsm_prepare_payload(buff, 0x12, MESH_MSG_ID_REALTIME_TIMER_EVENT, 10);

}




void protocol_send_feedback_data(uint8_t* data) {
//	NRF_LOG_INFO("protocol_send_feedback_data\r\n");
	static uint8_t sequence = 0;
	uint8_t buff[PROTOCOL_PAYLOAD_LEN];
	int8_t len = 8;

	protocol_send_data_to_gw(data, len, ePROTOCOL_CMD_TYPE_STATUS, 0xCD,false);
	bSendingToMesh = true;
}
