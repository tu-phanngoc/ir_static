
#include "app_ble.h"
#include "lib/sys_time.h"
#include "rbc_mesh.h"
#include "timeslot_handler.h"
#include "mesh_utils.h"
#include "sensor_utils.h"
#include "sensor_process.h"
#include "system_config.h"
#include "app_message_queue.h"
#include "btn.h"
#include "app_mesh.h"
#include "nrf_delay.h"

#include "IR_Interface.h"
#include "IR_Common.h"

#include "app_ac_status.h"
#include "data_transmit_fsm.h"
/* Static function prototypes */
#include "app_alarm.h"
#include "ac_timer_task.h"
#include "app_indicator_led.h"
#include "adc_task.h"
#include "app_error_code.h"
#include "shtc3.h"
#include "app_task_control.h"

#include "app_led_animation.h"
#include "crc32.h"
extern ble_gap_addr_t _device_addr;
const uint8_t broadcast_addr[6] = {0x99,0x99,0x99,0x99,0x99,0x99};
extern uint8_t alarmResetTime;
extern bool gb_is_gateway_alive;
extern uint8_t datalog_enabled;

#define MESH_DEBUG
#ifdef MESH_DEBUG
#define DBG_APP_MESH(...) NRF_LOG_INFO(__VA_ARGS__)
#else
#define DBG_APP_MESH(...)
#endif
uint32_t sysMode __attribute__((at(0x20005ff0)));//added by thienhaiblue@gmail.com#date:23/11/2015
uint8_t sysModeTimeout __attribute__((at(0x20005ff4)));//added by thienhaiblue@gmail.com#date:23/11/2015
uint8_t fired_flag = 0;
uint8_t	fired_timeout = 0;
uint8_t waitingForNewCustomerKey = 0;
uint8_t flag_waiting_for_confirm = 0;

//APP_TIMER_DEF(m_sensor_event_timer_id);
//APP_TIMER_DEF(m_sensor_recv_event_timer_id);
static uint8_t flag_send_message_asap = 0;
static bool sensor_data_sending = false;
static uint8_t send_data_timeout = 0;

uint8_t g_u8MeshPackRecv = 0;
uint32_t gu32MeshTimeout = 0;
extern uint32_t msg_from_fw_tick;

extern volatile bool gbBleCommandRecv;
extern uint8_t gBleData[24];
extern uint32_t sht_temperature;
extern uint8_t sht_humidity;
extern uint32_t wh_cnt;
extern tSMARTHOME_RESULT meter_result;
extern uint16_t datalog_number;
extern uint32_t ir_lock;
void protocol_send_feedback_data(uint8_t* data);

void IR_EncodeBLECommandToIRProtocol(uint8_t* InputBleCommands, uint8_t* OutputIRProtocol);

uint32_t app_get_mode(void)
{
	return sysMode;
}

void app_set_mode(uint32_t mode)
{
	if(mode == MESH_MODE) {
		NRF_LOG_INFO("Set mode to Mesh");
	}
	else
	{
		NRF_LOG_INFO("Set mode to Sensor");
	}
	sysMode =  mode;
	if(sysMode == MESH_MODE)
		sysModeTimeout = MESH_MODE_TIMEOUT;
}

void app_tick_task(void)
{

}


static uint32_t tag_get_ramdom(uint32_t min, uint32_t max)
{
	return  min + (rand() % (max - min));
}

static void sensor_event_timeout_handler(void * p_context)
{
	sensor_event_flag |= SENSOR_TIMEOUT_EVENT;
}

static void sensor_recv_event_timeout_handler(void * p_context)
{
	sensor_event_flag |= SENSOR_RECV_TIMEOUT_EVENT;
}

void mesh_app_init(void)
{
	// Create timers.
//	uint32_t err_code = app_timer_create(&m_sensor_event_timer_id,
//										APP_TIMER_MODE_SINGLE_SHOT,
//										sensor_event_timeout_handler);
//	APP_ERROR_CHECK(err_code);
//	// Create timers.
//	err_code = app_timer_create(&m_sensor_recv_event_timer_id,
//								APP_TIMER_MODE_SINGLE_SHOT,
//								sensor_recv_event_timeout_handler);
//	APP_ERROR_CHECK(err_code);
//	uint32_t next_timeslot;
//	next_timeslot = APP_TIMER_TICKS(tag_get_ramdom(20,30));
//	err_code = app_timer_start(m_sensor_event_timer_id,next_timeslot , NULL);
//	APP_ERROR_CHECK(err_code);
	#ifdef USE_MESH
	rbc_mesh_init_params_t init_params;
	uint32_t error_code;
	init_params.access_addr = sysCfg.radio_access_addr;
	init_params.adv_int_ms = 300;
	init_params.channel = sysCfg.radio_channel;
	init_params.handle_count = MESH_HANDLE_CNT;
	init_params.packet_format = RBC_MESH_PACKET_FORMAT_ORIGINAL;
	init_params.radio_mode = RBC_MESH_RADIO_MODE_BLE_1MBIT;
	error_code = rbc_mesh_init(init_params);
	APP_ERROR_CHECK(error_code);
	mesh_enable_all_handle();

	AppMessageQueueInit();
	if(sysModeTimeout >= MESH_MODE_TIMEOUT)
	{
		sysModeTimeout = MESH_MODE_TIMEOUT;
	}
	#endif
}

uint32_t gu32Count = 0;
void app_mesh_task(void)
{
//	gu32Count++;
//	if(gu32Count < 100)
//	{
//		return;
//	}
//	gu32Count = 0;
	
	uint32_t err_code;
	uint32_t next_timeslot = 0;
	uint8_t status;
	ISMARTPACKAGE iSmartPacket;
	ISMART_MESH_PACKET Mesh2BlePacket;

	//check package
	if(App_iSmartPacketGet(&iSmartPacket) == ISMARTPACKET_OK)
	{
		AppShowiSmartPacket(iSmartPacket);
		uint32_t crc32 = crc32_compute(&iSmartPacket.packet.addr[0], 20, NULL);
		if(iSmartPacket.packet.crc != crc32) {
			DBG_APP_MESH("****************** CHECK CRC FAIL *");
			return;
		}
		else
		{
			DBG_APP_MESH("******************* CHECK CRC SUCCESS *");
		}
		gu32IRCmdTimeout = IR_CMD_TIMEOUT;
		meter_timeout = METER_TIMEOUT;
		DBG_APP_MESH("App_iSmartPacketGet OK, cmd = %02X,  queue cmd = %02X",iSmartPacket.packet.command, msg_temp_queue.u8_cmd);
		
//		else if(1) 
		{
//			DBG_APP_MESH("PROCESSING PACKET..............");
	//		if(iSmartPacket.handle != 0xFF)
			{
				if((iSmartPacket.packet.sequence == msg_temp_queue.u16_sequence) && (memcmp(_device_addr.addr,iSmartPacket.packet.addr,BLE_GAP_ADDR_LEN) == NULL)/*|| (iSmartPacket.packet.sequence == msg_temp_queue.u16_sequence + 1)*/)
				{
					notify_data_transmit_fsm(1);
				}
			}
		
			if( !(IS_SET(iSmartPacket.packet.command, 4)))
			{
//				DBG_APP_MESH("reset msg_from_fw_tick..............................");
				msg_from_fw_tick = 0;
			}
			
	//		if(iSmartPacket.packet.crc == AppMakeCrc((uint8_t *)&iSmartPacket.packet,sysCfg.deviceKey,_device_addr.addr,(rtcTimeSec)>>4))
			{
				//CMD = 0x11/0x12/0x13/0x14
				if(( IS_SET(iSmartPacket.packet.command, 4)) && (memcmp(_device_addr.addr,iSmartPacket.packet.addr,BLE_GAP_ADDR_LEN) != NULL))
				{
//					DBG_APP_MESH("A report msg another device to app..............................");
					App_ProcessPacket(&iSmartPacket);
					App_iSmartAppPacketAddToList(&iSmartPacket);
//					App_iSmartMeshPacketAddToList((uint8_t *)&iSmartPacket,sizeof(ISMARTPACKAGE));
				}	
				
				//CMD = 0x01/0x02/0x03/0x04
				else if( !(IS_SET(iSmartPacket.packet.command, 4)) && (memcmp(_device_addr.addr,iSmartPacket.packet.addr,BLE_GAP_ADDR_LEN) != NULL))
				{
//					DBG_APP_MESH("A control msg from app to another device..............................");
				}
				//CMD = 0x11/0x12/0x13/0x14
				else if(( IS_SET(iSmartPacket.packet.command, 4)) && (memcmp(_device_addr.addr,iSmartPacket.packet.addr,BLE_GAP_ADDR_LEN) == NULL))
				{
					AppShowiSmartPacket(iSmartPacket);
//					DBG_APP_MESH("A report msg from this device to app....................");
					App_iSmartAppPacketAddToList(&iSmartPacket);
					App_iSmartMeshPacketAddToList((uint8_t *)&iSmartPacket,sizeof(ISMARTPACKAGE));
				}
				else 
					if(memcmp(_device_addr.addr,iSmartPacket.packet.addr,BLE_GAP_ADDR_LEN) == NULL && (iSmartPacket.packet.command == 0x01))
					{
						gb_is_gateway_alive = true;
						
						DBG_APP_MESH("A control msg from app to this device..............................");
						
//						NRF_LOG_INFO("A control msg from app to this device..............................");
						App_ProcessPacket(&iSmartPacket);
						iSmartPacket.packet.command = 0x11;
//						iSmartPacket.packet.crc = AppMakeCrc((uint8_t *)&iSmartPacket.packet,sysCfg.deviceKey,_device_addr.addr,(rtcTimeSec)>>4);
						iSmartPacket.packet.crc = crc32_compute(&iSmartPacket.packet.addr[0], 20, NULL);
						App_iSmartAppPacketAddToList(&iSmartPacket);
						App_iSmartMeshPacketAddToList((uint8_t *)&iSmartPacket,sizeof(ISMARTPACKAGE));
//						DBG_APP_MESH("Sending feedback data \r\n");
					}
					
				else 
					if(memcmp(_device_addr.addr,iSmartPacket.packet.addr,BLE_GAP_ADDR_LEN) == NULL && (iSmartPacket.packet.command == 0x02))
					{
//						DBG_APP_MESH("Recv feedback data from app to this device \r\n");
						App_ProcessPacket(&iSmartPacket);
//						App_iSmartMeshPacketAddToList((uint8_t *)&iSmartPacket,sizeof(ISMARTPACKAGE));
					}
				else 
					if(memcmp(_device_addr.addr,iSmartPacket.packet.addr,BLE_GAP_ADDR_LEN) == NULL && (iSmartPacket.packet.command == 0x03))
					{
//						DBG_APP_MESH("App request to get settings from this device \r\n");
						iSmartPacket.packet.data[0] = rtcTimeSec & 0xFF;
						iSmartPacket.packet.data[1] = (rtcTimeSec >> 8) & 0xFF;
						iSmartPacket.packet.data[2] = (rtcTimeSec >> 16) & 0xFF;
						iSmartPacket.packet.data[3] = (rtcTimeSec >> 24) & 0xFF;
						App_ProcessPacket(&iSmartPacket);
						iSmartPacket.packet.command = 0x13;
//						iSmartPacket.packet.crc = AppMakeCrc((uint8_t *)&iSmartPacket.packet,sysCfg.deviceKey,_device_addr.addr,(rtcTimeSec)>>4);
						iSmartPacket.packet.crc = crc32_compute(&iSmartPacket.packet.addr[0], 20, NULL);
						App_iSmartAppPacketAddToList(&iSmartPacket);
						App_iSmartMeshPacketAddToList((uint8_t *)&iSmartPacket,sizeof(ISMARTPACKAGE));
					}
				else 
					if(memcmp(_device_addr.addr,iSmartPacket.packet.addr,BLE_GAP_ADDR_LEN) == NULL && (iSmartPacket.packet.command == 0x0F))
					{
//						if(g_u8MeshPackRecv == 1)
//							return;
						DBG_APP_MESH("Control all device");
						App_ProcessPacket(&iSmartPacket);
//						App_iSmartMeshPacketAddToList((uint8_t *)&iSmartPacket,sizeof(ISMARTPACKAGE));
					}

				else if(iSmartPacket.handle == PROTOCOL_DATA_SEND_TO_MESH)//send to mesh net
				{
					App_iSmartMeshPacketAddToList((uint8_t *)&iSmartPacket,sizeof(ISMARTPACKAGE));
				}
				else 
				{
//					DBG_APP_MESH("just fwd");
					App_iSmartAppPacketAddToList(&iSmartPacket);
				}				
			}
			g_u8MeshPackRecv = 1;
			
			
		}
//		else
//		{
////			DBG_APP_MESH("OLD PACKET, IGNORE..............");
//		}
		
	}
	
	if(ble_check_status() != NRF_ERROR_BUSY)
	{
		if(App_iSmartAppPacketGet(&iSmartPacket) == ISMARTPACKET_OK)
		{
			DBG_APP_MESH("Sending message to BLE...");
			ble_send_data((uint8_t *)&iSmartPacket.packet,sizeof(ISMARTPACKAGE_TYPE_BLE));
		}
	}

	//Send message to mesh network
	if(App_iSmartMeshPacketGet(Mesh2BlePacket.data,&Mesh2BlePacket.len) == ISMARTPACKET_OK)
	{
		AppShowiSmartPacket(*((ISMARTPACKAGE *)&Mesh2BlePacket));
//		DBG_APP_MESH("Sending message to MESH network ...");
//		NRF_LOG_INFO("Sending message to MESH network ...");
		uint32_t ret = mesh_add_packet(Mesh2BlePacket.data[0], &Mesh2BlePacket.data[1], Mesh2BlePacket.len - 1);
		if(ret == NRF_SUCCESS)
		{
//			NRF_LOG_INFO("Sending message to MESH network SUCCESS: %d ...", ret);
		}
		else
		{
//			NRF_LOG_INFO("Sending message to MESH network FAIL: %d ...", ret);
		}
	}
}

/* Static functions */
static uint64_t _convert_ble_addr_to_long(uint8_t* p_addr)
{
	uint64_t ll_addr = 0;
	uint8_t *pt = (uint8_t *)&ll_addr;
	pt[0] = p_addr[0];
	pt[1] = p_addr[1];
	pt[2] = p_addr[2];
	pt[3] = p_addr[3];
	pt[4] = p_addr[4];
	pt[5] = p_addr[5];

	return ll_addr;
}

uint8_t getHandleGateway(uint8_t *addr_Gateway)
{
	uint64_t addr_long = _convert_ble_addr_to_long(addr_Gateway);
	uint8_t m_handle = (addr_long % MESH_HANDLE_CNT) + 1;
	if(m_handle == 0) m_handle = 1;
	return m_handle;
}



uint8_t App_ProcessPacket(ISMARTPACKAGE *iSmartPacket)
{
	DBG_APP_MESH("App_ProcessPacket");
	uint8_t schedule_flag = 0;
	gbBleCommandRecv = true;
	uint8_t ret_status = 0;
	DBG_APP_MESH("%02X",iSmartPacket->packet.command);
	switch(iSmartPacket->packet.command)
	{
		case 0x03:
			if(iSmartPacket->packet.type == 0xCD)
			{
				DBG_APP_MESH("TYPE ===================>>> 0xCD");
				for(uint8_t i = 0; i < AC_STATUS_PAYLOAD_LEN; i++)
				{
					iSmartPacket->packet.data[i+4] = ac_status_payload[i];
				}
			}
			else if (iSmartPacket->packet.type == 0x01)
			{
				uint16_t u16Temp = sht_temperature/10;
				uint8_t u16Humi = sht_humidity;

				uint16_t fakeTemp = sht_temperature/10;
				uint8_t fakeHumi = sht_humidity;
				/* */
				iSmartPacket->packet.data[4] = (uint8_t)(u16Temp & 0xFF);
				iSmartPacket->packet.data[5] = (uint8_t)((u16Temp >> 8) & 0xFF);
				iSmartPacket->packet.data[6] = (uint8_t)u16Humi;
			}
			else if (iSmartPacket->packet.type == 0x02)
			{
				uint32_t u32P = meter_result.p[0];
				
				iSmartPacket->packet.data[4] = u32P & 0xFF;
				iSmartPacket->packet.data[5] = (u32P >> 8) & 0xFF;
				
				iSmartPacket->packet.data[6] = (wh_cnt) & 0xFF;
				iSmartPacket->packet.data[7] = (wh_cnt >> 8) & 0xFF;
				iSmartPacket->packet.data[8] = (wh_cnt >> 16) & 0xFF;
				iSmartPacket->packet.data[9] = (wh_cnt >> 24) & 0xFF;
			}
			else if (iSmartPacket->packet.type == 0x06)
			{
				iSmartPacket->packet.data[0] = get_led_pwm();
			}
			else if (iSmartPacket->packet.type == 0x0E)
			{
				iSmartPacket->packet.data[0] = datalog_number & 0xFF;
				iSmartPacket->packet.data[1] = (datalog_number >> 8) & 0xFF;
			}
			
			else if((iSmartPacket->packet.type >= 0x11) && (iSmartPacket->packet.type <= 0x1A))
			{
				app_alarm_get_config(iSmartPacket->packet.data, 10, iSmartPacket->packet.type - 0x11);
			}
			ret_status = 1;
			break;
		case 0x01:
			if(iSmartPacket->packet.type == 0xCD)
			{
				if(ir_lock == 1)
				{
					DBG_APP_MESH("IR_BUSY");
					return 0;
				}
				else
				{
					DBG_APP_MESH("TYPE ===================>>> 0xCD");
					memcpy(ac_status_payload, &iSmartPacket->packet.data[4], 6); 

					memcpy(sysCfg.ac_payload, ac_status_payload, 6); 
					
					ac_control_get_status_from_payload(ac_status_payload);
					
					uint8_t data[12];
					IRInterface_EncodeBLEToIR(data, g_i16RawIRBits);
					IRInterface_PrepareDataToSend(g_i16RawIRBits);
					gu32IRTxTrigger = 1;
					memcpy(gBleData, iSmartPacket->packet.data, 24);
					
					if(ac_status.timer_enable == 1)
					{
						ac_timer_task_trigger(ac_status.timer_val);
					}
					CFG_Saving_Trigger();
				}
			}
			else if (iSmartPacket->packet.type == 0x01)
			{
			}
			else if (iSmartPacket->packet.type == 0x04)
			{
				DBG_APP_MESH("TYPE 04, SETTIME");			
				rtcTimeSec = iSmartPacket->packet.data[0] | 
										iSmartPacket->packet.data[1] << 8 | 
										iSmartPacket->packet.data[2] << 16 | 
										iSmartPacket->packet.data[3] << 24;
				rtcUpdateFlag = 1;
			}
			else if(iSmartPacket->packet.type == 0x06)
			{
				set_led_pwm(iSmartPacket->packet.data[0]);
			}
			else if(iSmartPacket->packet.type == 0x08)
			{
				app_led_blink_trigger(iSmartPacket->packet.data[1]);
			}
			else if(iSmartPacket->packet.type == 0x0B)
			{
				sysCfg.enable_sensor_streaming = iSmartPacket->packet.data[1];
				sysCfg.enable_power_streaming = iSmartPacket->packet.data[2];
				sysCfg.enable_remote_data_streaming = iSmartPacket->packet.data[3];
				sysCfg.enable_schedule_event_streaming = iSmartPacket->packet.data[4];
				sysCfg.enable_timer_event_streaming = iSmartPacket->packet.data[5];
				CFG_Saving_Trigger();
			}
			else if (iSmartPacket->packet.type == 0x0D)
			{
				sysCfg.enable_sensor_datalog = iSmartPacket->packet.data[0];
				datalog_enabled = iSmartPacket->packet.data[0];
				CFG_Saving_Trigger();
			}
			else if((iSmartPacket->packet.type >= 0x11) && (iSmartPacket->packet.type <= 0x1A))
			{
				app_alarm_set_config(iSmartPacket->packet.data, 10, iSmartPacket->packet.type - 0x11);
				schedule_flag = 1;
			}
			ret_status = 1;
			
			//Save scheduler to flash
			if(schedule_flag == 1)
			{
				schedule_flag = 0;
				for(uint8_t i = 0; i < NUMBER_OF_SCHEDULE; i++)
				{				
					sysCfg.schedule_control[i] = schedule_control[i];
				}
				CFG_Saving_Trigger();
			}
			
		break;
		case 0x02:
			if (iSmartPacket->packet.type == 0x04)
			{
				DBG_APP_MESH("TYPE 04, SETTIME");			
				rtcTimeSec = iSmartPacket->packet.data[0] | 
										iSmartPacket->packet.data[1] << 8 | 
										iSmartPacket->packet.data[2] << 16 | 
										iSmartPacket->packet.data[3] << 24;
				rtcUpdateFlag = 1;
			}
			else if(iSmartPacket->packet.type == 0x07)
			{
				error_code_clear(iSmartPacket->packet.data[4]);
			}
			break;
		case ePROTOCOL_CMD_ALL:
				ac_status_payload[0] = iSmartPacket->packet.data[4];
				ac_status_payload[1] = iSmartPacket->packet.data[5];
				ac_status_payload[2] = iSmartPacket->packet.data[6];
				ac_status_payload[3] = iSmartPacket->packet.data[7];
				ac_status_payload[4] = iSmartPacket->packet.data[8];
				ac_status_payload[5] = iSmartPacket->packet.data[9];

				ac_control_get_status_from_payload(ac_status_payload);
//				nrf_delay_ms(20);
				IRInterface_EncodeBLEToIR(iSmartPacket->packet.data, g_i16RawIRBits);
				IRInterface_PrepareDataToSend(g_i16RawIRBits);
				gu32IRTxTrigger = 1;
				memcpy(gBleData, iSmartPacket->packet.data, 24);
//				nrf_delay_ms(50);
				ret_status = 1;
			break;
		default:
			break;
	}

	return ret_status;
}

/**
* @brief RBC_MESH framework event handler. Defined in rbc_mesh.h. Handles
*   events coming from the mesh. Sets LEDs according to data
*
* @param[in] evt RBC event propagated from framework
*/
void rbc_mesh_event_handler(rbc_mesh_event_t* evt)
{
    switch (evt->event_type)
    {
			case RBC_MESH_EVENT_TYPE_CONFLICTING_VAL:
					DBG_APP_MESH("RBC_MESH_EVENT_TYPE_CONFLICTING_VAL");
			break;
        case RBC_MESH_EVENT_TYPE_NEW_VAL:
					DBG_APP_MESH("RBC_MESH_EVENT_TYPE_NEW_VAL");
					ISMARTPACKAGE iSmartPacket;
					iSmartPacket.handle = evt->value_handle;
					memcpy(&iSmartPacket.packet,evt->data,sizeof(ISMARTPACKAGE_TYPE_BLE));
					AppShowiSmartPacket(iSmartPacket);
			break;
        case RBC_MESH_EVENT_TYPE_UPDATE_VAL:
					DBG_APP_MESH("RBC_MESH_EVENT_TYPE_UPDATE_VAL, handle: %d", evt->value_handle);
			if(evt->data_len == sizeof(ISMARTPACKAGE_TYPE_BLE))
			{
				if (evt->value_handle > MESH_HANDLE_CNT) {
					break;
				}
				ISMARTPACKAGE iSmartPacket;
				iSmartPacket.handle = evt->value_handle;
				memcpy(&iSmartPacket.packet,evt->data,sizeof(ISMARTPACKAGE_TYPE_BLE));
//				AppShowiSmartPacket(iSmartPacket);
				App_iSmartPacketAddToList(&iSmartPacket);
			}
			break;
		default:
			break;
    }
}

bool mesk_task_enabled = false;
void enable_mesh_task(void)
{
	mesk_task_enabled = true;
}
void disable_mesh_task(void)
{
	mesk_task_enabled = false;
}

bool is_mesh_task_enable(void)
{
	return mesk_task_enabled;
}
