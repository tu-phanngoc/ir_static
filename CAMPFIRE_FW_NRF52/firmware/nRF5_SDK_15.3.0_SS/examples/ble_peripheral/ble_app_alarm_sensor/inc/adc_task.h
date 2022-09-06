

#ifndef __ADC_TASK_H__
#define __ADC_TASK_H__
#include "stdint.h"

#define VDD_DISABLE_LEVEL	2450

#define METER_TIMEOUT										4
void adc_measure_vdd(void);
void adc_config_inteval(uint16_t rtc_time);
extern uint16_t	batt_lvl_in_milli_volts;
extern uint32_t meter_timeout;
void adc_task();

void set_I_rms_mA(uint16_t val);

uint16_t get_I_rms_mA(void);

uint8_t is_adc_task_enabled(void);

void reset_adc_task_trigger();

void adc_task_trigger(void);

#endif


