#include "nrf_delay.h"
#include "app_alarm.h"
#include "sys_time.h"
#include "IR_Interface.h"
#include "app_ac_status.h"
#include "app_indicator_led.h"
#include "nrf_use_flash.h"


extern DATE_TIME sysTime;
schedule_control_t schedule_control[NUMBER_OF_SCHEDULE];

void protocol_send_IR_data(uint8_t* data);
void app_alarm_task()
{
	uint8_t is_cmd_triggered = 0;
	bool cfg_saving = false;
	/* Get current rtc */
	for(uint8_t schedule_idx = 0; schedule_idx < NUMBER_OF_SCHEDULE; schedule_idx ++)
	{
		schedule_control_t schedule = schedule_control[schedule_idx];
		/* Check with schedule setting */
		if(schedule.alarm_config.isEnabled && 				
				schedule.alarm_config.hour == sysTime.hour &&
				schedule.alarm_config.min == sysTime.min &&
				sysTime.sec == 0 &&
				schedule.alarm_config.isRepeatAvailable == 0)
		{
			NRF_LOG_INFO("TRIGGER SCHEDULE............... 0");
			is_cmd_triggered = 1;
		}
		if(schedule.alarm_config.isEnabled)
		{
			/* 1. Check sunday repeat */
			/* If match, send event */
			if(schedule.alarm_config.isSundayRepeated && 
					schedule.alarm_config.hour == sysTime.hour &&
					schedule.alarm_config.min == sysTime.min &&
					sysTime.wday == 1 &&
					sysTime.sec == 0)
			{
				NRF_LOG_INFO("TRIGGER SCHEDULE............... 1");
				is_cmd_triggered = 1;
			}
			
			/* 2. Check sunday repeat */
			/* If match, send event */
			if(schedule.alarm_config.isMondayRepeated && 
					schedule.alarm_config.hour == sysTime.hour &&
					schedule.alarm_config.min == sysTime.min &&
					sysTime.wday == 2 &&
					sysTime.sec == 0)
			{
				NRF_LOG_INFO("TRIGGER SCHEDULE............... 2");
				is_cmd_triggered = 1;
			}
			
			/* 3. Check sunday repeat */
			/* If match, send event */
			if(schedule.alarm_config.isTuesdayRepeated && 
					schedule.alarm_config.hour == sysTime.hour &&
					schedule.alarm_config.min == sysTime.min &&
					sysTime.wday == 3 &&
					sysTime.sec == 0)
			{
				NRF_LOG_INFO("TRIGGER SCHEDULE............... 3");
				is_cmd_triggered = 1;
			}
			
			/* 4. Check sunday repeat */
			/* If match, send event */
			if(schedule.alarm_config.isWednesdayRepeated && 
					schedule.alarm_config.hour == sysTime.hour &&
					schedule.alarm_config.min == sysTime.min &&
					sysTime.wday == 4 &&
					sysTime.sec == 0)
			{
				NRF_LOG_INFO("TRIGGER SCHEDULE............... 4");
				is_cmd_triggered = 1;

			}
			
			/* 5. Check sunday repeat */
			/* If match, send event */
			if(schedule.alarm_config.isThursdayRepeated && 
					schedule.alarm_config.hour == sysTime.hour &&
					schedule.alarm_config.min == sysTime.min &&
					sysTime.wday == 5 &&
					sysTime.sec == 0)
			{
				NRF_LOG_INFO("TRIGGER SCHEDULE............... 5");
				is_cmd_triggered = 1;
			}
			
			/* 6. Check sunday repeat */
			/* If match, send event */
			if(schedule.alarm_config.isFridayRepeated && 
					schedule.alarm_config.hour == sysTime.hour &&
					schedule.alarm_config.min == sysTime.min &&
					sysTime.wday == 6 &&
					sysTime.sec == 0)
			{
				is_cmd_triggered = 1;
			}
			
			/* 7. Check sunday repeat */
			/* If match, send event */
			if(schedule.alarm_config.isSaturdayRepeated && 
					schedule.alarm_config.hour == sysTime.hour &&
					schedule.alarm_config.min == sysTime.min &&
					sysTime.wday == 7 &&
					sysTime.sec == 0)
			{
				NRF_LOG_INFO("TRIGGER SCHEDULE............... 7");
				is_cmd_triggered = 1;
			}
		}
			
		if(is_cmd_triggered == 1)
		{
			if(schedule.alarm_config.isRepeatAvailable == 0)
			{
				schedule.alarm_config.isEnabled = 0;
			}
			NRF_LOG_INFO("ACTION.............................");
			uint8_t data[12];
			ac_control_get_status_from_payload(schedule.ac_status);
			IRInterface_EncodeBLEToIR(data, g_i16RawIRBits);
			IRInterface_PrepareDataToSend(g_i16RawIRBits);
			gu32IRTxTrigger = 1;
			protocol_send_schedule_event_data(data);
			cfg_saving = true;
			is_cmd_triggered = 0;
		}
		schedule_control[schedule_idx] = schedule;
	}
	
	if(cfg_saving == true)
	{
		NRF_LOG_INFO("SAVE schedule");
		for(uint8_t i = 0; i < NUMBER_OF_SCHEDULE; i++)
		{				
			sysCfg.schedule_control[i] = schedule_control[i];
		}
		CFG_Saving_Trigger();
		cfg_saving = false;
	}

}

void app_alarm_set_config(uint8_t* data, uint8_t len, uint8_t index)
{
	NRF_LOG_INFO("app_alarm_set_config idx %d:...............................", index);
	if(index >= NUMBER_OF_SCHEDULE)
	{
		NRF_LOG_INFO("index too large %d", index);
		return;
	}
	NRF_LOG_INFO("isEnabled = %d", data[0]);
	NRF_LOG_INFO("hour = %d", data[1]);
	NRF_LOG_INFO("min = %d", data[2]);
	NRF_LOG_INFO("repeat = 0x%02X", data[3]);
	schedule_control[index].alarm_config.isEnabled = data[0];

	schedule_control[index].alarm_config.hour = data[1];
	schedule_control[index].alarm_config.min = data[2];
	if(data[3] == 0) {
		schedule_control[index].alarm_config.isRepeatAvailable = 0;
		schedule_control[index].alarm_config.isSundayRepeated = 0;
		schedule_control[index].alarm_config.isMondayRepeated = 0;
		schedule_control[index].alarm_config.isTuesdayRepeated = 0;
		schedule_control[index].alarm_config.isWednesdayRepeated = 0;
		schedule_control[index].alarm_config.isThursdayRepeated = 0;
		schedule_control[index].alarm_config.isFridayRepeated = 0;
		schedule_control[index].alarm_config.isSaturdayRepeated = 0;
	}
	else {
		schedule_control[index].alarm_config.isRepeatAvailable = 1;
		schedule_control[index].alarm_config.isSundayRepeated = IS_SET(data[3], 1);
		schedule_control[index].alarm_config.isMondayRepeated = IS_SET(data[3], 2);
		schedule_control[index].alarm_config.isTuesdayRepeated = IS_SET(data[3], 3);
		schedule_control[index].alarm_config.isWednesdayRepeated = IS_SET(data[3], 4);
		schedule_control[index].alarm_config.isThursdayRepeated = IS_SET(data[3], 5);
		schedule_control[index].alarm_config.isFridayRepeated = IS_SET(data[3], 6);
		schedule_control[index].alarm_config.isSaturdayRepeated = IS_SET(data[3], 7);
	}
	for(uint8_t i = 4; i < 10; i++)
	{
		NRF_LOG_INFO("%02X", data[i]);
	}
	schedule_control[index].ac_status[0] = data[4];
	schedule_control[index].ac_status[1] = data[5];
	schedule_control[index].ac_status[2] = data[6];
	schedule_control[index].ac_status[3] = data[7];
	schedule_control[index].ac_status[4] = data[8];
	schedule_control[index].ac_status[5] = data[9];
}


void app_alarm_get_config(uint8_t* data, uint8_t len, uint8_t index)
{
	NRF_LOG_INFO("app_alarm_get_config idx %d:...............................", index);
	if(index >= NUMBER_OF_SCHEDULE)
	{
		NRF_LOG_INFO("index too large %d", index);
		return;
	}

	data[0] = schedule_control[index].alarm_config.isEnabled;

	data[1] = schedule_control[index].alarm_config.hour;
	data[2] = schedule_control[index].alarm_config.min;
	data[3] = schedule_control[index].alarm_config.isSundayRepeated << 1 |
						schedule_control[index].alarm_config.isMondayRepeated << 2 |
						schedule_control[index].alarm_config.isTuesdayRepeated << 3 |
						schedule_control[index].alarm_config.isWednesdayRepeated << 4 |
						schedule_control[index].alarm_config.isThursdayRepeated << 5 |
						schedule_control[index].alarm_config.isFridayRepeated << 6 |
						schedule_control[index].alarm_config.isSaturdayRepeated << 7;
	
	NRF_LOG_INFO("isEnabled = %d", data[0]);
	NRF_LOG_INFO("hour = %d", data[1]);
	NRF_LOG_INFO("min = %d", data[2]);
	NRF_LOG_INFO("repeat = 0x%02X", data[3]);
	for(uint8_t i = 0; i < 6; i++)
	{
		data[i+4] = schedule_control[index].ac_status[i];
		NRF_LOG_INFO("%02X", data[i+4]);
	}
}

void app_alarm_init()
{
	for(uint8_t i = 0; i < NUMBER_OF_SCHEDULE; i++)
	{
		schedule_control[i] = sysCfg.schedule_control[i];
	}
}
