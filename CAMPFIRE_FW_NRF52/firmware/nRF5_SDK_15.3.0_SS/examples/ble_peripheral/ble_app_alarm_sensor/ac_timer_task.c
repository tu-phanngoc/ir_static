#include "stdint.h"
#include "ac_timer_task.h"
#include "app_ac_status.h"
#include "IR_Interface.h"
#include "nrf_delay.h"
#define AC_TIMER_STATE_STANDBY 				0
#define AC_TIMER_STATE_RUNNING				1
#define AC_TIMER_STATE_FINISH					2

uint8_t ac_current_onoff_state = 0;
uint8_t ac_timer_task_triggered = 0;
uint32_t ac_timer_task_tick = 0;

uint8_t ac_timer_state = AC_TIMER_STATE_STANDBY;
static uint8_t is_timer_enabled = 0;

void protocol_send_timer_event_data(uint8_t* data);

void ac_timer_task()
{
	if(ac_timer_state == AC_TIMER_STATE_STANDBY)
	{
		if(is_timer_enabled == 1)
		{
			is_timer_enabled = 0;
			ac_control_update_status_to_payload();
			
			if(ac_status.timer_event_type == 0)
			{
				NRF_LOG_INFO("ac_timer, OFF TIMER..........................");

//				uint8_t data[12];
//				ac_control_set_power_status(0);
//				ac_control_update_status_to_payload();
//				IRInterface_EncodeBLEToIR(data, g_i16RawIRBits);
//				IRInterface_PrepareDataToSend(g_i16RawIRBits);
//				gu32IRTxTrigger = 1;
//				nrf_delay_ms(50);
//				protocol_send_IR_data(data);
			}
			else
			{
				NRF_LOG_INFO("ac_tiimer, ON TIMER...........................");
//				uint8_t data[12];
//				ac_control_set_power_status(1);
//				ac_control_update_status_to_payload();
//				IRInterface_EncodeBLEToIR(data, g_i16RawIRBits);
//				IRInterface_PrepareDataToSend(g_i16RawIRBits);
//				gu32IRTxTrigger = 1;
//				nrf_delay_ms(50);
//				protocol_send_IR_data(data);
			}
			
			is_timer_enabled = 0;
			ac_control_update_status_to_payload();
		}
		
		if(ac_timer_task_triggered == 1)
		{
			ac_timer_task_triggered = 0;
			ac_timer_state = AC_TIMER_STATE_RUNNING;
		}
	}
	else if(ac_timer_state == AC_TIMER_STATE_RUNNING)
	{
		if(ac_timer_task_tick == 0)
		{
			ac_timer_state = AC_TIMER_STATE_FINISH;
		}
		else
		{
			NRF_LOG_INFO("ac_tiimer: %d - %d", ac_timer_task_tick, ac_status.power_status);
		}
		if(ac_timer_task_tick % 60 == 0)
		{
			ac_control_update_status_to_payload();
			sysCfg.ac_payload[0] = ac_status_payload[0];
			sysCfg.ac_payload[1] = ac_status_payload[1];
			sysCfg.ac_payload[2] = ac_status_payload[2];
			sysCfg.ac_payload[3] = ac_status_payload[3];
			sysCfg.ac_payload[4] = ac_status_payload[4];
			sysCfg.ac_payload[5] = ac_status_payload[5];
			
			CFG_Saving_Trigger();
		}
	}
	else if(ac_timer_state == AC_TIMER_STATE_FINISH)
	{
		ac_timer_state = AC_TIMER_STATE_STANDBY;
		ac_status.timer_enable = 0;
		//Change state here
		NRF_LOG_INFO("AC_TIMER_STATE_FINISH, REVERSE STATE: %d", ac_status.power_status);
		if(ac_status.timer_event_type == 1)
		{
			NRF_LOG_INFO("ac_tiimer, TURN ON");
			ac_control_set_power_status(1);
			uint8_t data[12];
			ac_control_update_status_to_payload();
			IRInterface_EncodeBLEToIR(data, g_i16RawIRBits);
			IRInterface_PrepareDataToSend(g_i16RawIRBits);
			gu32IRTxTrigger = 1;
			nrf_delay_ms(50);
			protocol_send_timer_event_data(data);
		}
		else
		{
			NRF_LOG_INFO("ac_tiimer, TURN OFF");
			uint8_t data[12];
			ac_control_set_power_status(0);
			ac_control_update_status_to_payload();
			IRInterface_EncodeBLEToIR(data, g_i16RawIRBits);
			IRInterface_PrepareDataToSend(g_i16RawIRBits);
			gu32IRTxTrigger = 1;
			nrf_delay_ms(50);
			protocol_send_timer_event_data(data);
		}
		
		sysCfg.ac_payload[0] = ac_status_payload[0];
		sysCfg.ac_payload[1] = ac_status_payload[1];
		sysCfg.ac_payload[2] = ac_status_payload[2];
		sysCfg.ac_payload[3] = ac_status_payload[3];
		sysCfg.ac_payload[4] = ac_status_payload[4];
		sysCfg.ac_payload[5] = ac_status_payload[5];
		CFG_Saving_Trigger();
	}
	ac_timer_tick();

	//Always check the condition
	if(ac_status.timer_enable == 0)
	{
		ac_timer_task_trigger(0);
	}
}

void ac_timer_tick()
{
	if(ac_timer_task_tick > 0)
	{
		ac_timer_task_tick--;
		ac_status.timer_val = ac_timer_task_tick;
	}
}

void ac_timer_task_trigger(uint32_t timeout)
{
//	NRF_LOG_INFO("ac_timer_task_trigger %d", timeout);
	if(timeout > 0)
	{
		is_timer_enabled = 1;
		ac_timer_state = AC_TIMER_STATE_STANDBY;
		ac_timer_task_triggered = 1;
		ac_timer_task_tick = timeout;
	}
	else 
	{
		//
		ac_timer_task_tick = 0;
		ac_timer_state = AC_TIMER_STATE_STANDBY;
		is_timer_enabled = 0;
	}
}

void ac_timer_task_init()
{
	if(ac_status.timer_enable == 1)
	{
		ac_timer_task_trigger(ac_status.timer_val);
	}
}

void ac_timer_task_deinit()
{
}
