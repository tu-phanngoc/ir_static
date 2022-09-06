#include "stdint.h"
#include "ac_type_detection_task.h"
#include "app_ble.h"
#define AC_TYPE_DETECTION_STANDBY 				0
#define AC_TYPE_DETECTION_RUNNING					1
#define AC_TYPE_DETECTION_OK							2
#define AC_TYPE_DETECTION_TIMEOUT					3
#define AC_TYPE_DETECTION_FAIL						4

uint8_t ac_type_detection_task_triggered = 0;
uint32_t ac_type_detection_task_tick = 0;
uint8_t ac_type_detection_signal = 251;
uint8_t ac_type_detection_task_state = AC_TYPE_DETECTION_STANDBY;

void ac_type_detection_task()
{
	if(ac_type_detection_task_state == AC_TYPE_DETECTION_STANDBY)
	{
		if(ac_type_detection_task_triggered == 1)
		{
			ac_type_detection_task_triggered = 0;
			ac_type_detection_task_state = AC_TYPE_DETECTION_RUNNING;
		}
	}
	else if(ac_type_detection_task_state == AC_TYPE_DETECTION_RUNNING)
	{
		NRF_LOG_INFO("AC_TYPE_DETECTION_RUNNING: %d", ac_type_detection_task_tick);
//		ac_type_detection_signal = 1; //Hardcode
//		if(ac_type_detection_task_tick == 3)
//		{
//			ac_type_detection_task_state = AC_TYPE_DETECTION_OK;
//			ac_type_detection_signal = 1;
//		}
		
		if(ac_type_detection_task_tick == 0)
		{
			ac_type_detection_task_state = AC_TYPE_DETECTION_TIMEOUT;
		}
		else if(ac_type_detection_signal != 255 && ac_type_detection_signal < 250)
		{
			ac_type_detection_task_state = AC_TYPE_DETECTION_OK;
		}
		else if(ac_type_detection_signal == 255)
		{
			ac_type_detection_task_state = AC_TYPE_DETECTION_FAIL;
		}
	}
	else if(ac_type_detection_task_state == AC_TYPE_DETECTION_TIMEOUT)
	{
		if(ble_check_status() != NRF_ERROR_BUSY)
		{
			uint8_t data[16] = "TIMEOUT";
			ble_send_data(data,sizeof(data));
		}
		ac_type_detection_task_state = AC_TYPE_DETECTION_STANDBY;
		ac_type_detection_set_signal(255);
		//Change state here
		NRF_LOG_INFO("AC_TYPE_DETECTION_TIMEOUT");
	}
	else if(ac_type_detection_task_state == AC_TYPE_DETECTION_OK)
	{
		NRF_LOG_INFO("AC_TYPE_DETECTION_OK: %d",ac_type_detection_signal);
		if(ble_check_status() != NRF_ERROR_BUSY)
		{
			uint8_t data[16];
			sprintf((char*)data,"TYPE=%u",ac_type_detection_signal);
			ble_send_data(data,strlen((char *)data));
		}
		ac_type_detection_task_state = AC_TYPE_DETECTION_STANDBY;
		ac_type_detection_set_signal(251);
		//Change state here
	}
	else if(ac_type_detection_task_state == AC_TYPE_DETECTION_FAIL)
	{
		NRF_LOG_INFO("AC_TYPE_DETECTION_FAIL: %d",ac_type_detection_signal);
		if(ble_check_status() != NRF_ERROR_BUSY)
		{
			uint8_t data[16];
			sprintf((char*)data,"TYPE=-1");
			ble_send_data(data,strlen((char *)data));
		}
		ac_type_detection_task_state = AC_TYPE_DETECTION_STANDBY;
		ac_type_detection_set_signal(251);
		//Change state here
	}
	ac_type_detection_tick();
}

void ac_type_detection_tick()
{
	if(ac_type_detection_task_tick > 0)
	{
		ac_type_detection_task_tick--;
	}
}

void ac_type_detection_task_trigger(uint32_t timeout)
{
	if(timeout > 0)
	{
		NRF_LOG_INFO("ac_type_detection_task_trigger: %d", timeout);
		ac_type_detection_task_state = AC_TYPE_DETECTION_STANDBY;
		ac_type_detection_task_triggered = 1;
		ac_type_detection_task_tick = timeout;
	}
	else 
	{
		//
	}
}

void ac_type_detection_task_init()
{
}

void ac_type_detection_task_deinit()
{
}

void ac_type_detection_set_signal(uint8_t data)
{
	ac_type_detection_signal = data;
}

uint8_t is_ac_type_detection_enabled()
{
	if(ac_type_detection_task_state == AC_TYPE_DETECTION_RUNNING)
	{
		return 1;
	}
	return 0;
}
