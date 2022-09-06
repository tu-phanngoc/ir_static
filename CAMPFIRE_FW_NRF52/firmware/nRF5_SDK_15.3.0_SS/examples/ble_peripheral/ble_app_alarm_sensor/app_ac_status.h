#ifndef __APP_AC_STATUS_H
#define __APP_AC_STATUS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"



#define AC_STATUS_PAYLOAD_LEN           6

#define AC_PAYLOAD_BYTE_IDX_0           0
#define AC_PAYLOAD_BYTE_IDX_1           1
#define AC_PAYLOAD_BYTE_IDX_2           2
#define AC_PAYLOAD_BYTE_IDX_3           3
#define AC_PAYLOAD_BYTE_IDX_4           4
#define AC_PAYLOAD_BYTE_IDX_5           5

#define BIT0                            0
#define BIT1                            1
#define BIT2                            2
#define BIT3                            3
#define BIT4                            4
#define BIT5                            5
#define BIT6                            6
#define BIT7                            7

#define AC_POWER_STATUS_BYTE_IDX        					AC_PAYLOAD_BYTE_IDX_0
#define AC_TEMPERATURE_BYTE_IDX        						AC_PAYLOAD_BYTE_IDX_0
#define AC_MODE_BYTE_IDX                					AC_PAYLOAD_BYTE_IDX_1
#define AC_FAN_BYTE_IDX                 					AC_PAYLOAD_BYTE_IDX_1
#define AC_SWING_BYTE_IDX               					AC_PAYLOAD_BYTE_IDX_2
#define AC_TIMER_ENABLE_OVERRIDE_BYTE_IDX       	AC_PAYLOAD_BYTE_IDX_5
#define AC_TIMER_ENABLE_BYTE_IDX        					AC_PAYLOAD_BYTE_IDX_5
#define AC_TIMER_TYPE_EVENT_BYTE_IDX    					AC_PAYLOAD_BYTE_IDX_5
#define AC_TIMER_VAL_BYTE_LSB_IDX           			AC_PAYLOAD_BYTE_IDX_3
#define AC_TIMER_VAL_BYTE_MSB_IDX           			AC_PAYLOAD_BYTE_IDX_4
#define AC_AIR_DIR_BYTE_IDX             					AC_PAYLOAD_BYTE_IDX_2
#define AC_COMFORT_BYTE_IDX             					AC_PAYLOAD_BYTE_IDX_0
#define AC_QUIET_SLEEP_BYTE_IDX         					AC_PAYLOAD_BYTE_IDX_0
#define AC_KILL_BACTERIA_BYTE_IDX       					AC_PAYLOAD_BYTE_IDX_1
#define AC_TURBO_BYTE_IDX               					AC_PAYLOAD_BYTE_IDX_1
#define AC_ECO_BYTE_IDX                 					AC_PAYLOAD_BYTE_IDX_2
#define AC_HUMAN_DETECT_BYTE_IDX        					AC_PAYLOAD_BYTE_IDX_2
#define AC_ANTI_MOLD_BYTE_IDX           					AC_PAYLOAD_BYTE_IDX_2
#define AC_SELF_CARE_BYTE_IDX           					AC_PAYLOAD_BYTE_IDX_2



#define AC_POWER_STATUS_LSB             BIT0 
#define AC_POWER_STATUS_MSB             BIT0   

#define AC_TEMPERATURE_LSB              BIT1    
#define AC_TEMPERATURE_MSB              BIT5   

#define AC_MODE_LSB                     BIT0
#define AC_MODE_MSB                     BIT2

#define AC_FAN_LSB                      BIT3
#define AC_FAN_MSB                      BIT5

#define AC_SWING_LSB                    BIT7
#define AC_SWING_MSB                    BIT7 

#define AC_TIMER_ENABLE_OVERRIDE_LSB    BIT0
#define AC_TIMER_ENABLE_OVERRIDE_MSB    BIT0 

#define AC_TIMER_ENABLE_LSB             BIT1
#define AC_TIMER_ENABLE_MSB             BIT1 

#define AC_TIMER_TYPE_EVENT_LSB         BIT2 
#define AC_TIMER_TYPE_EVENT_MSB         BIT2

#define AC_TIMER_VAL_LSB                BIT0
#define AC_TIMER_VAL_MSB                BIT7

#define AC_AIR_DIR_LSB                  BIT0
#define AC_AIR_DIR_MSB                  BIT2

#define AC_COMFORT_LSB                  BIT6
#define AC_COMFORT_MSB                  BIT6 

#define AC_QUIET_SLEEP_LSB              BIT7
#define AC_QUIET_SLEEP_MSB              BIT7

#define AC_KILL_BACTERIA_LSB            BIT6
#define AC_KILL_BACTERIA_MSB            BIT6

#define AC_TURBO_LSB                    BIT7
#define AC_TURBO_MSB                    BIT7

#define AC_ECO_LSB                      BIT3           
#define AC_ECO_MSB                      BIT3

#define AC_HUMAN_DETECT_LSB             BIT4 
#define AC_HUMAN_DETECT_MSB             BIT4

#define AC_ANTI_MOLD_LSB                BIT5     
#define AC_ANTI_MOLD_MSB                BIT5

#define AC_SELF_CARE_LSB                BIT6        
#define AC_SELF_CARE_MSB                BIT7


#define AC_MODE_AUTO										7
#define AC_MODE_DRY											1
#define AC_MODE_FAN_ONLY								2
#define AC_MODE_HEAT										3
#define AC_MODE_COOL										4

#define AC_FAN_LVL_AUTO									7
#define AC_FAN_LVL_1										1
#define AC_FAN_LVL_2										2
#define AC_FAN_LVL_3										3
#define AC_FAN_LVL_4										4
#define AC_FAN_LVL_5										5

//#define SET_BIT(val, bitIndex) val |= (1 << bitIndex)
//#define CLEAR_BIT(val, bitIndex) val &= ~(1 << bitIndex)
//#define TOGGLE_BIT(val, bitIndex) val ^= (1 << bitIndex)
//#define BIT_IS_SET(val, bitIndex) (val & (1 << bitIndex))

typedef struct ac_stt
{
    uint8_t power_status;
    uint8_t temperature;
    uint8_t mode;
    uint8_t fan;
    uint8_t swing;
		uint8_t timer_override;
    uint8_t timer_enable;
    uint8_t timer_event_type;
    uint16_t timer_val;
    uint8_t air_dir;
    uint8_t comfort;
    uint8_t quiet_sleep;
    uint8_t kill_bacteria;
    uint8_t turbo_mode;
    uint8_t eco;
    uint8_t human_detect;
    uint8_t anti_mold;
    uint8_t self_care;
}ac_status_t;
extern ac_status_t ac_status;
extern uint8_t ac_status_payload[AC_STATUS_PAYLOAD_LEN];
void ac_control_init();
uint8_t ac_control_update_status_to_payload();
uint8_t ac_control_set_power_status(uint8_t val);
uint8_t ac_control_set_temperature(uint8_t val);
uint8_t ac_control_set_mode(uint8_t val);
uint8_t ac_control_set_fan(uint8_t val);
uint8_t ac_control_set_swing(uint8_t val);
uint8_t ac_control_get_timer_override(uint8_t val);
uint8_t ac_control_set_timer_enable(uint8_t val);
uint8_t ac_control_set_timer_val(uint8_t val);
uint8_t ac_control_set_air_dir(uint8_t val);
uint8_t ac_control_set_comfort(uint8_t val);
uint8_t ac_control_set_quiet_sleep(uint8_t val);
uint8_t ac_control_set_kill_bacteria(uint8_t val);
uint8_t ac_control_set_turbo_mode(uint8_t val);
uint8_t ac_control_set_eco(uint8_t val);
uint8_t ac_control_set_human_detect(uint8_t val);
uint8_t ac_control_set_anti_mold(uint8_t val);
uint8_t ac_control_set_self_care(uint8_t val);

uint8_t ac_control_update_status_to_payload();
uint8_t ac_control_get_status_from_payload(uint8_t* data);

uint8_t app_ac_init(uint8_t* data);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_HAL_H */