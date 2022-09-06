#include "app_ac_status.h"
#include "nrf_log.h"

#define PRINTF NRF_LOG_INFO
//#define PRINTF(...)

ac_status_t ac_status;
uint8_t ac_status_payload[AC_STATUS_PAYLOAD_LEN];

static uint8_t IsNumberInRange(uint8_t number,uint8_t min, uint8_t max);
static uint8_t GetValueFromBitPos(uint8_t u8Num, uint8_t LsbPos, uint8_t MsbPos);
/* Function 0.1 */
static uint8_t IsNumberInRange(uint8_t number,uint8_t min, uint8_t max)
{
	if(number >= min && number <= max)
		return 1;
	else
		return 0;
}

/* Function 0.2 */
static uint8_t GetValueFromBitPos(uint8_t u8Num, uint8_t LsbPos, uint8_t MsbPos)
{	
	uint8_t ret = u8Num >> LsbPos;
	MsbPos = MsbPos - LsbPos;
	switch(MsbPos)
	{
		case 0:
			ret = ret & 0x01;
			break;
		case 1:
			ret = ret & 0x03;
			break;
		case 2:
			ret = ret & 0x07;
			break;
		case 3:
			ret = ret & 0x0F;
			break;
		case 4:
			ret = ret & 0x1F;
			break;
		case 5:
			ret = ret & 0x3F;
			break;
		case 6:
			ret = ret & 0x7F;
			break;
		case 7:
			ret = ret & 0xFF;
			break;
		default:
			return 0;
			break;
	}
	return ret;
}

static uint8_t SetValueToPos(uint8_t* data, uint8_t lsbPos, uint8_t msbPos, uint8_t val)
{
    if(lsbPos > msbPos)
        return 1;
    for(uint8_t i = lsbPos; i <= msbPos; i++)
    {
        CLR_BIT(*data, i);
    }
    *data |= val << lsbPos;
    return 0;
}

void ac_control_init()
{
	memset((void*)&ac_status, 0, sizeof(ac_status_t));
}

uint8_t ac_control_set_power_status(uint8_t val)
{
	PRINTF("ac_control_set_power_status: %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!", val);
  ac_status.power_status = val;
  return 0;
}

uint8_t ac_control_set_temperature(uint8_t val)
{
    ac_status.temperature = val;
    return 0;
}

uint8_t ac_control_set_mode(uint8_t val)
{
    ac_status.mode = val;
    return 0;
}

uint8_t ac_control_set_fan(uint8_t val)
{
    ac_status.fan = val;
    return 0;
}

uint8_t ac_control_set_swing(uint8_t val)
{
    ac_status.swing = val;
    return 0;
}

uint8_t ac_control_set_timer_override(uint8_t val)
{
    ac_status.timer_override = val;
    return 0;
}

uint8_t ac_control_get_timer_override(uint8_t val)
{
    return ac_status.timer_override;
}

uint8_t ac_control_set_timer_enable(uint8_t val)
{
    ac_status.timer_enable = val;
    return 0;
}

uint8_t ac_control_set_timer_val(uint8_t val)
{
    ac_status.timer_val = val;
    return 0;
}

uint8_t ac_control_set_air_dir(uint8_t val)
{
    ac_status.air_dir = val;
    return 0;
}

uint8_t ac_control_set_comfort(uint8_t val)
{
    ac_status.comfort = val;
    return 0;
}

uint8_t ac_control_set_quiet_sleep(uint8_t val)
{
    ac_status.quiet_sleep = val;
    return 0;
}

uint8_t ac_control_set_kill_bacteria(uint8_t val)
{
    ac_status.kill_bacteria = val;
    return 0;
}

uint8_t ac_control_set_turbo_mode(uint8_t val)
{
    ac_status.turbo_mode = val;
    return 0;
}

uint8_t ac_control_set_eco(uint8_t val)
{
    ac_status.eco = val;
    return 0;
}
uint8_t ac_control_set_human_detect(uint8_t val)
{
    ac_status.human_detect = val;
    return 0;
}
uint8_t ac_control_set_anti_mold(uint8_t val)
{
    ac_status.anti_mold = val;
    return 0;
}
uint8_t ac_control_set_self_care(uint8_t val)
{
    ac_status.self_care = val;
    return 0;
}

uint8_t ac_control_update_status_to_payload()
{
	PRINTF("ac_control_update_status_to_payload:");
	PRINTF("power: %d", ac_status.power_status);
    SetValueToPos(
                    &ac_status_payload[AC_POWER_STATUS_BYTE_IDX], 
                    AC_POWER_STATUS_LSB, 
                    AC_POWER_STATUS_MSB, 
                    ac_status.power_status);
	
	PRINTF("TEMP: %d ......................",ac_status.temperature + 16);
    SetValueToPos(
                    &ac_status_payload[AC_TEMPERATURE_BYTE_IDX], 
                    AC_TEMPERATURE_LSB, 
                    AC_TEMPERATURE_MSB, 
                    ac_status.temperature);
	
		PRINTF("mode: %d",ac_status.mode);
    SetValueToPos(
                    &ac_status_payload[AC_MODE_BYTE_IDX], 
                    AC_MODE_LSB, 
                    AC_MODE_MSB, 
                    ac_status.mode);
	
//    PRINTF("fan: %d",ac_status.fan);
    SetValueToPos(
                    &ac_status_payload[AC_FAN_BYTE_IDX], 
                    AC_FAN_LSB, 
                    AC_FAN_MSB, 
                    ac_status.fan);
										
//		PRINTF("swing: %d",ac_status.swing);
    SetValueToPos(
                    &ac_status_payload[AC_SWING_BYTE_IDX], 
                    AC_SWING_LSB, 
                    AC_SWING_MSB, 
                    ac_status.swing);
										
		PRINTF("timer_override: %d",ac_status.timer_override);
    SetValueToPos(
                    &ac_status_payload[AC_TIMER_ENABLE_OVERRIDE_BYTE_IDX], 
                    AC_TIMER_ENABLE_OVERRIDE_LSB, 
                    AC_TIMER_ENABLE_OVERRIDE_LSB, 
                    ac_status.timer_override);							
		PRINTF("timer_enable: %d",ac_status.timer_enable);
    SetValueToPos(
                    &ac_status_payload[AC_TIMER_ENABLE_BYTE_IDX], 
                    AC_TIMER_ENABLE_LSB, 
                    AC_TIMER_ENABLE_MSB, 
                    ac_status.timer_enable);
										
		PRINTF("timer_event_type: %d",ac_status.timer_event_type);
    SetValueToPos(
                    &ac_status_payload[AC_TIMER_TYPE_EVENT_BYTE_IDX], 
                    AC_TIMER_TYPE_EVENT_LSB, 
                    AC_TIMER_TYPE_EVENT_MSB, 
                    ac_status.timer_event_type);
										
		PRINTF("timer_val: %d",ac_status.timer_val);
    SetValueToPos(
                    &ac_status_payload[AC_TIMER_VAL_BYTE_LSB_IDX], 
                    AC_TIMER_VAL_LSB, 
                    AC_TIMER_VAL_MSB, 
                    ac_status.timer_val & 0xFF);
		SetValueToPos(
                    &ac_status_payload[AC_TIMER_VAL_BYTE_MSB_IDX], 
                    AC_TIMER_VAL_LSB, 
                    AC_TIMER_VAL_MSB, 
                    (ac_status.timer_val >> 8) & 0xFF);
										
//		PRINTF("air_dir: %d",ac_status.air_dir);
    SetValueToPos(
                    &ac_status_payload[AC_AIR_DIR_BYTE_IDX], 
                    AC_AIR_DIR_LSB, 
                    AC_AIR_DIR_MSB, 
                    ac_status.air_dir);
										
//		PRINTF("comfort: %d",ac_status.comfort);
    SetValueToPos(
                    &ac_status_payload[AC_COMFORT_BYTE_IDX], 
                    AC_COMFORT_LSB, 
                    AC_COMFORT_MSB, 
                    ac_status.comfort);
										
//		PRINTF("quiet_sleep: %d",ac_status.quiet_sleep);
		
    SetValueToPos(
                    &ac_status_payload[AC_QUIET_SLEEP_BYTE_IDX], 
                    AC_QUIET_SLEEP_LSB, 
                    AC_QUIET_SLEEP_MSB, 
                    ac_status.quiet_sleep);
										
//		PRINTF("kill_bacteria: %d",ac_status.kill_bacteria);
    SetValueToPos(
                    &ac_status_payload[AC_KILL_BACTERIA_BYTE_IDX], 
                    AC_KILL_BACTERIA_LSB, 
                    AC_KILL_BACTERIA_MSB, 
                    ac_status.kill_bacteria);
										
//		PRINTF("turbo_mode: %d",ac_status.turbo_mode);
    SetValueToPos(
                    &ac_status_payload[AC_TURBO_BYTE_IDX], 
                    AC_TURBO_LSB, 
                    AC_TURBO_MSB, 
                    ac_status.turbo_mode);
										
//		PRINTF("eco: %d",ac_status.eco);
    SetValueToPos(
                    &ac_status_payload[AC_ECO_BYTE_IDX], 
                    AC_ECO_LSB, 
                    AC_ECO_MSB, 
                    ac_status.eco);
										
//		PRINTF("human_detect: %d",ac_status.human_detect);
    SetValueToPos(
                    &ac_status_payload[AC_HUMAN_DETECT_BYTE_IDX], 
                    AC_HUMAN_DETECT_LSB, 
                    AC_HUMAN_DETECT_MSB, 
                    ac_status.human_detect);
										
//		PRINTF("anti_mold: %d",ac_status.anti_mold);
    SetValueToPos(
                    &ac_status_payload[AC_ANTI_MOLD_BYTE_IDX], 
                    AC_ANTI_MOLD_LSB, 
                    AC_ANTI_MOLD_MSB, 
                    ac_status.anti_mold); 
										
//		PRINTF("self_care: %d",ac_status.self_care);
		
    SetValueToPos(
                    &ac_status_payload[AC_SELF_CARE_BYTE_IDX], 
                    AC_SELF_CARE_LSB, 
                    AC_SELF_CARE_MSB, 
                    ac_status.self_care); 

    return 0;
}

uint8_t ac_control_get_status_from_payload(uint8_t* data)
{
	for(uint8_t i = 0; i < 6; i++)
	{
		PRINTF("%02X", data[i]);
	}
		PRINTF("ac_control_get_status_from_payload:");
    ac_status.power_status = GetValueFromBitPos(
                                                    data[AC_POWER_STATUS_BYTE_IDX], 
                                                    AC_POWER_STATUS_LSB, 
                                                    AC_POWER_STATUS_MSB);
		PRINTF("power: %d..................................................", ac_status.power_status);
	
	
    ac_status.temperature = GetValueFromBitPos(
                                                    data[AC_TEMPERATURE_BYTE_IDX], 
                                                    AC_TEMPERATURE_LSB, 
                                                    AC_TEMPERATURE_MSB);
	
		PRINTF("TEMP: %d   ..........................", ac_status.temperature + 16);
	
	
    ac_status.mode = GetValueFromBitPos(
                                                    data[AC_MODE_BYTE_IDX], 
                                                    AC_MODE_LSB, 
                                                    AC_MODE_MSB);
	
		PRINTF("mode: %d", ac_status.mode);
	
	
    ac_status.fan = GetValueFromBitPos(
                                                    data[AC_FAN_BYTE_IDX], 
                                                    AC_FAN_LSB, 
                                                    AC_FAN_MSB);
//		PRINTF("fan: %d", ac_status.fan);
		
		
    ac_status.swing = GetValueFromBitPos(
                                                    data[AC_SWING_BYTE_IDX], 
                                                    AC_SWING_LSB, 
                                                    AC_SWING_MSB);
//		PRINTF("swing: %d", ac_status.swing);

    ac_status.timer_override = GetValueFromBitPos(
                                                    data[AC_TIMER_ENABLE_OVERRIDE_BYTE_IDX], 
                                                    AC_TIMER_ENABLE_OVERRIDE_LSB, 
                                                    AC_TIMER_ENABLE_OVERRIDE_MSB);
																										
		PRINTF("timer_override: %d", ac_status.timer_override);		
		
		if(ac_status.timer_override == 1)
		{
			ac_status.timer_override = 0;
			ac_status.timer_enable = GetValueFromBitPos(
                                                    data[AC_TIMER_ENABLE_BYTE_IDX], 
                                                    AC_TIMER_ENABLE_LSB, 
                                                    AC_TIMER_ENABLE_MSB);
			PRINTF("timer_enable: %d", ac_status.timer_enable);
			
			ac_status.timer_event_type = GetValueFromBitPos(
																											data[AC_TIMER_ENABLE_BYTE_IDX], 
																											AC_TIMER_TYPE_EVENT_LSB, 
																											AC_TIMER_TYPE_EVENT_MSB);
			PRINTF("timer_event_type: %d", ac_status.timer_event_type);

			ac_status.timer_val = GetValueFromBitPos(data[AC_TIMER_VAL_BYTE_LSB_IDX], 0, 7) | GetValueFromBitPos(data[AC_TIMER_VAL_BYTE_MSB_IDX], 0, 7) << 8;
			PRINTF("timer_val: %d", ac_status.timer_val);	
		}		
								
    ac_status.air_dir = GetValueFromBitPos(
                                                    data[AC_AIR_DIR_BYTE_IDX], 
                                                    AC_AIR_DIR_LSB, 
                                                    AC_AIR_DIR_MSB);
//		PRINTF("air_dir: %d", ac_status.air_dir);
		

    ac_status.comfort = GetValueFromBitPos(
                                                    data[AC_COMFORT_BYTE_IDX], 
                                                    AC_COMFORT_LSB, 
                                                    AC_COMFORT_MSB);
//		PRINTF("comfort: %d", ac_status.comfort);
		
		
    ac_status.quiet_sleep = GetValueFromBitPos(
                                                    data[AC_QUIET_SLEEP_BYTE_IDX], 
                                                    AC_QUIET_SLEEP_LSB, 
                                                    AC_QUIET_SLEEP_MSB);	
//		PRINTF("quiet_sleep: %d", ac_status.quiet_sleep);
		
		
    ac_status.kill_bacteria = GetValueFromBitPos(
                                                    data[AC_KILL_BACTERIA_BYTE_IDX], 
                                                    AC_KILL_BACTERIA_LSB, 
                                                    AC_KILL_BACTERIA_MSB);
																										
//		PRINTF("kill_bacteria: %d", ac_status.kill_bacteria);
		
		
    ac_status.turbo_mode = GetValueFromBitPos(
                                                    data[AC_TURBO_BYTE_IDX], 
                                                    AC_TURBO_LSB, 
                                                    AC_TURBO_MSB);
//		PRINTF("turbo_mode: %d", ac_status.turbo_mode);																		
																								

    ac_status.eco = GetValueFromBitPos(
                                                    data[AC_ECO_BYTE_IDX], 
                                                    AC_ECO_LSB,
                                                    AC_ECO_MSB);
//		PRINTF("eco: %d", ac_status.eco);

    ac_status.human_detect = GetValueFromBitPos(
                                                    data[AC_HUMAN_DETECT_BYTE_IDX], 
                                                    AC_HUMAN_DETECT_LSB, 
                                                    AC_HUMAN_DETECT_MSB);
//		PRINTF("human_detect: %d", ac_status.human_detect);
																										

    ac_status.anti_mold = GetValueFromBitPos(
                                                    data[AC_ANTI_MOLD_BYTE_IDX], 
                                                    AC_ANTI_MOLD_LSB, 
                                                    AC_ANTI_MOLD_MSB);
//		PRINTF("anti_mold: %d", ac_status.anti_mold);
		
		
    ac_status.self_care = GetValueFromBitPos(
                                                    data[AC_SELF_CARE_BYTE_IDX], 
                                                    AC_SELF_CARE_LSB, 
                                                    AC_SELF_CARE_MSB);
//		PRINTF("self_care: %d", ac_status.self_care);
	return 0;
}

uint8_t app_ac_init(uint8_t* data)
{
	if(data != NULL)
	{
		ac_status_payload[0] = data[0]; 
		ac_status_payload[1] = data[1]; 
		ac_status_payload[2] = data[2]; 
		ac_status_payload[3] = data[3]; 
		ac_status_payload[4] = data[4]; 
		ac_status_payload[5] = data[5];
		ac_control_get_status_from_payload(ac_status_payload);	
		return 0;
	}
	else
	{
		return 1;
	}
}