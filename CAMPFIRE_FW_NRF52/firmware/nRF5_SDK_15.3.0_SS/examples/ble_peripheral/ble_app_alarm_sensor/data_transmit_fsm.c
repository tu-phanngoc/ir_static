#include "data_transmit_fsm.h"
#include "dataLogger_Interface.h"
#include "app_datalog.h"
#include "app_led_animation.h"

#define DATA_TRANSMIT_DEBUG_ENABLE
#ifdef DATA_TRANSMIT_DEBUG_ENABLE
#define DATA_TRANSMIT_DEBUG				NRF_LOG_INFO
#else
#define DATA_TRANSMIT_DEBUG(...)
#endif

#define DATA_TRANSMIT_TIMEOUT			2
/* transitions from end state aren't needed */
data_transmit_transition_t data_transmit_state_transitions[] = {
		{data_transmit_fsm_standby, repeat,   data_transmit_fsm_standby},
    {data_transmit_fsm_standby, ok,     data_transmit_fsm_start},
    {data_transmit_fsm_start,   ok,     data_transmit_fsm_transmit},
		{data_transmit_fsm_start,   fail,     data_transmit_fsm_exit},
    {data_transmit_fsm_transmit,   ok,   data_transmit_fsm_wait},
    {data_transmit_fsm_wait,   repeat, data_transmit_fsm_wait},
    {data_transmit_fsm_wait,   ok,     data_transmit_fsm_exit},
    {data_transmit_fsm_wait,   fail,   data_transmit_fsm_exit},
		{data_transmit_fsm_exit,   ok, data_transmit_fsm_standby}
};
/* array and enum below must be in sync! */
static int (* state[])(void) = { standby_state, start_state, transmit_state, wait_state, exit_state};
static data_transmit_state_codes_e transmit_state_codes = 0;
static enum ret_codes rc = 2;
static int (* state_fun)(void);
static int is_triggered = 0;
uint32_t data_transmit_tick = 0;
uint8_t data_transmit_notification = 0;

static uint8_t data_transmit_fsm_payload[DATA_TRANSMIT_FSM_PAYLOAD_LEN];
static uint8_t u8_cmd;
static uint8_t u8_type;

extern bool gb_is_gateway_alive;
extern uint8_t datalog_enabled;

void data_transmit_fsm_init()
{
	for(uint8_t i = 0; i < DATA_TRANSMIT_FSM_PAYLOAD_LEN; i++)
	{
		 data_transmit_fsm_payload[i] = 0;
	}
	u8_cmd = 0;
	u8_type = 0;
}

static int standby_state()
{
//	NRF_LOG_INFO("standby_state");
	uint8_t len;
	if(App_iSmartEventPacketGet(data_transmit_fsm_payload, &u8_cmd, &u8_type, &len) == ISMARTPACKET_OK)
	{
		//
		
				if(data_transmit_fsm_payload[3] == 0x01 && data_transmit_fsm_payload[1] == 0x01 && data_transmit_fsm_payload[2] == 0x01)
			{
			}
		else if(data_transmit_fsm_payload[3] == 0x02 && data_transmit_fsm_payload[1] == 0x02 && data_transmit_fsm_payload[2] == 0x02)
			{
			}
				else if(data_transmit_fsm_payload[3] == 0x03 && data_transmit_fsm_payload[1] == 0x03 && data_transmit_fsm_payload[2] == 0x03)
			{
			}
			else if(data_transmit_fsm_payload[3] == 0x04 && data_transmit_fsm_payload[1] == 0x04 && data_transmit_fsm_payload[2] == 0x04)
			{
			}
			else
			{
				if(data_transmit_fsm_payload[0] == 0x17 && data_transmit_fsm_payload[3] == 0x00)
				{
					data_transmit_fsm_payload[1] = 0x05;
					data_transmit_fsm_payload[2] = 0x05;
					data_transmit_fsm_payload[3] = 0x05;
				}
			}
			
		NRF_LOG_INFO("App_iSmartEventPacketGet OK...........");
		return ok;
	}
	else
		return repeat;
}

static int start_state()
{
//	NRF_LOG_INFO("start_state");
	return ok;
}

static int transmit_state()
{
//	DATA_TRANSMIT_DEBUG("transmit_state");
	uint8_t len;
	data_transmit_tick = DATA_TRANSMIT_TIMEOUT;
	data_transmit_notification = 0;
	

	
	protocol_send_data_to_gw(data_transmit_fsm_payload, DATA_TRANSMIT_FSM_PAYLOAD_LEN, u8_cmd, u8_type,false);
	return ok;
}

static int wait_state()
{
//	NRF_LOG_INFO("wait_state");
	if(data_transmit_tick > 0)
	{
		if (data_transmit_notification == 1) 
		{
			data_transmit_notification = 0;
			return ok;
		}
		else
		{
			return repeat;
		}
	}
	return fail;
}

static int exit_state()
{
//	NRF_LOG_INFO("exit_state: %d", rc);
	if(rc == ok)
	{
		/* Pop data */
		DATA_TRANSMIT_DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!! ACK Successs !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		is_app_alive = true;
		gb_is_gateway_alive = true;
	}
	else if(rc == fail)
	{
		//gui lai data
		/* */
		
		DATA_TRANSMIT_DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Send MSG failed, cmd %02X, type %02X !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!", u8_cmd, u8_type);
		is_app_alive = false;
		gb_is_gateway_alive = false;
		datalog_enabled = 0;
		sysCfg.enable_sensor_datalog = datalog_enabled;
//		CFG_Saving_Trigger();
		
		for(uint8_t i = 0; i < 10; i++)
		{
			saveDB.payload[i] = data_transmit_fsm_payload[i];
		}
		//Save to DB
		saveDB.u8cmd = u8_cmd;
		
		if(u8_type == MESH_MSG_ID_REALTIME_IR)
		{
			saveDB.u8type = MESH_MSG_ID_HISTORY_IR;
		}
		else if(u8_type == MESH_MSG_ID_REALTIME_SENSOR)
		{
			saveDB.u8type = MESH_MSG_ID_HISTORY_SENSOR;
		}
		else if(u8_type == MESH_MSG_ID_REALTIME_PWR)
		{
			if(saveDB.payload[3] == 0x01 && saveDB.payload[1] == 0x01 && saveDB.payload[2] == 0x01)
			{
			}
			else
			{
				if(saveDB.payload[0] == 0x17 && saveDB.payload[3] == 0x00)
				{
					saveDB.payload[1] = 0x02;
					saveDB.payload[2] = 0x02;
					saveDB.payload[3] = 0x02;
				}
			}

			saveDB.u8type = MESH_MSG_ID_HISTORY_PWR;
		}
		else if(u8_type == MESH_MSG_ID_REALTIME_SCHEDULE_EVENT)
		{
			saveDB.u8type = MESH_MSG_ID_HISTORY_SCHEDULE_EVENT;
		}
		else if(u8_type == MESH_MSG_ID_REALTIME_TIMER_EVENT)
		{
			saveDB.u8type = MESH_MSG_ID_HISTORY_TIMER_EVENT;
		}


		is_datalog_save_enabled = true;
	}
	
	notify_data_transmit_fsm(0);
	return ok;
}


int data_transmit_lookup_transitions(data_transmit_state_codes_e cur_state,enum ret_codes rc, data_transmit_transition_t* state_trans, uint8_t transition_len) {
    data_transmit_state_codes_e ret = 0;
    for(int i = 0; i < transition_len; i++) {
        if(transmit_state_codes == state_trans[i].src_state && rc == state_trans[i].ret_code) {
            ret = state_trans[i].dst_state;
        }
    }
    return ret;
}

int data_transmit_fsm_main_loop()
{
	state_fun = state[transmit_state_codes];
	rc = state_fun();
	transmit_state_codes = data_transmit_lookup_transitions(transmit_state_codes, rc, data_transmit_state_transitions, sizeof(data_transmit_state_transitions)/sizeof(data_transmit_state_transitions[0]));
}


void data_transmit_tick_loop(void)
{
	if(data_transmit_tick > 0)
		data_transmit_tick--;
}

void notify_data_transmit_fsm(uint8_t value)
{
	data_transmit_notification = value;
//	NRF_LOG_INFO("notify_data_transmit_fsm: %d", data_transmit_notification);
}

void transmit_fsm_prepare_payload(uint8_t* data, uint8_t cmd, uint8_t type, uint8_t len)
{
	
	NRF_LOG_INFO("transmit_fsm_prepare_payload %02X, %02X", cmd, type );
	App_iSmartEventPacketAddToList(data,cmd,type,len);
}


