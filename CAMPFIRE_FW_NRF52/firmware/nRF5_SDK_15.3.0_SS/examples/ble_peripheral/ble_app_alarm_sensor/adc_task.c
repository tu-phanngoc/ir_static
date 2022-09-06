/**
  ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "adc_task.h"
#include "app_ble.h"
#include "user_ringbuf.h"
#include "app_common.h"
#include "math.h"

#include "fraclib.h"
#include "meterlib.h"

#include "system_config.h"
#include "smarthome_cfg.h"
#include "smarthome_meter.h"
#include "pingpong.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
nrf_saadc_value_t adc_result;
uint16_t          batt_lvl_in_milli_volts = 3300;
uint8_t           percentage_batt_lvl;
//static bool adc_xfer_done = false;
#define SAADC_SAMPLES_IN_BUFFER         2
#define ADC_GAIN  											6
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS   600                                     /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_PRE_SCALING_COMPENSATION    1                                       /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define ADC_RES_10BIT                   1024                                    /**< Maximum digital value for 10-bit ADC conversion. */
#define ADC_RES_12BIT                   4096
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE)\
        ((ADC_VALUE * ADC_REF_VOLTAGE_IN_MILLIVOLTS * ADC_GAIN * ADC_PRE_SCALING_COMPENSATION) / ADC_RES_12BIT)

#define SAADC_SAMPLE_RATE               1                                         /**< SAADC sample rate in ms. */               


#define ADC_PPI_CH         6

#define ADC_PPI_GROUP        2

nrf_saadc_value_t adc_buf[2][SAADC_SAMPLES_IN_BUFFER];

uint8_t is_adc_enabled = 0;

uint16_t I_rms = 0;
uint16_t V_rms = 0;
uint32_t I_gain = 10;
uint32_t I_offset = 2500;
static uint32_t              m_adc_evt_counter;
int16_t temp_I;
int16_t temp_U;
//
pingpong_t measurement;
tSMARTHOME_DATA mlib = SMARTHOME_CFG;
tSMARTHOME_RESULT result;

//
SER_BUF_T adc_ring_buf;
static const nrf_drv_timer_t   m_timer = NRF_DRV_TIMER_INSTANCE(1);
static nrf_ppi_channel_t     m_ppi_channel = NRF_PPI_CHANNEL15;
extern volatile uint32_t gStateTimer;


uint32_t meter_timeout = 0;
void afeCallback();
void calcCallback(void);
void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);
    APP_ERROR_CHECK(err_code);
}

void timer_handler(nrf_timer_event_t event_type, void* p_context)
{

}
/**@brief Function for handling the ADC interrupt.
 *
 * @details  This function will fetch the conversion result from the ADC, convert the value into
 *           percentage and send it to peer.
 */

 void saadc_sampling_event_init(void)
{
    ret_code_t err_code;
    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_timer_config_t timer_config = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_config.frequency = NRF_TIMER_FREQ_16MHz;
    err_code = nrf_drv_timer_init(&m_timer, &timer_config, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer,SAADC_SAMPLE_RATE);
    nrf_drv_timer_extended_compare(&m_timer, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer, NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_event_addr = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel, timer_compare_event_addr, saadc_sample_event_addr);
    APP_ERROR_CHECK(err_code);

}
 
void saadc_event_handler(nrf_drv_saadc_evt_t const * p_event)
{
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
		int32_t offset = 1877; //(1.65 * 2^12 / 3.6 = 1877)
		int32_t adc_val;
		uint32_t          err_code;
		uint16_t current_adc_val;
		err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAADC_SAMPLES_IN_BUFFER);
		APP_ERROR_CHECK(err_code);
		
		adc_result = p_event->data.done.p_buffer[0];
		current_adc_val = ADC_RESULT_IN_MILLI_VOLTS((uint32_t)adc_result);
		
		adc_val = adc_result;
		adc_val = adc_val - offset;
		adc_val = adc_val << 12;
		
		measurement.current[0] = adc_val;
		measurement.current[1] = adc_val;
		measurement.current[2] = adc_val;
		measurement.current[3] = adc_val;
		
		//Get voltage_adc
		uint16_t u16_adc_res_vol = p_event->data.done.p_buffer[1];

		adc_val = u16_adc_res_vol;
		adc_val = adc_val - offset;
		adc_val = adc_val << 12;
		
		measurement.current[4] = adc_val;

		pingpong_swap(&measurement);
		
		afeCallback();
		calcCallback();
		m_adc_evt_counter++;
		//		NRF_LOG_INFO("ADC event number: %d\r\n",(int)m_adc_evt_counter);
//		NRF_LOG_INFO("t: %d us, I_adc: %d, V_adc: %d, adc_0: %d, adc_1: %d", gStateTimer, p_event->data.done.p_buffer[0], p_event->data.done.p_buffer[1], p_event->data.done.p_buffer[2], p_event->data.done.p_buffer[3]);
	}
}


/**@brief Function for configuring ADC to do battery level conversion.
 */
#define ACQ_TIME NRF_SAADC_ACQTIME_3US
void saadc_init(void)
{
	pingpong_init(&measurement, 5);
	ramcfg.correct.u_gain = FRAC32(1.0);
	ramcfg.correct.i_gain[0] = FRAC32(1.0);
	ramcfg.correct.p_gain[0] = FRAC32(1.0);
	
	  nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
    saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;
		saadc_config.oversample = 0;
    ret_code_t err_code = nrf_drv_saadc_init(&saadc_config, saadc_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_saadc_channel_config_t config_channel_1 =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);
	config_channel_1.acq_time = ACQ_TIME;
	
	  nrf_saadc_channel_config_t config_channel_2 =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN3);
	config_channel_2.acq_time = ACQ_TIME;
	
	    nrf_saadc_channel_config_t config_channel_3 =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);
	config_channel_3.acq_time = ACQ_TIME;
	
	    nrf_saadc_channel_config_t config_channel_4 =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);
	config_channel_4.acq_time = ACQ_TIME;	
	
		err_code = nrf_drv_saadc_channel_init(0, &config_channel_1);
    APP_ERROR_CHECK(err_code);
	  
		err_code = nrf_drv_saadc_channel_init(1, &config_channel_2);
    APP_ERROR_CHECK(err_code);	

    err_code = nrf_drv_saadc_buffer_convert(adc_buf[0], SAADC_SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(adc_buf[1], SAADC_SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}

void adc_init(void)
{
//		saadc_sampling_event_init();
    saadc_init();
//    saadc_sampling_event_enable();
		adc_task_trigger();
}

void adc_measure_vdd(void)
{
//	adc_xfer_done = false;
//	NRF_LOG_INFO("adc_measure:");
//	adc_init();
	nrf_drv_saadc_sample();
}

void adc_deinit(void)
{
	nrf_drv_saadc_uninit();
}


void afeCallback()
{
}
uint32_t wh_cnt;
long outResPuls;
void calcCallback(void)
{
	int i = 0;
	if (measurement.ready) {
		measurement.prev[i] = L_mul(measurement.prev[i],
						    ramcfg.correct.i_gain[i]);
		SMARTHOME_RemoveDcBias((tSMARTHOME_DATA *)&mlib,
				       L_mul(measurement.prev[NUM_CHAN],
					     ramcfg.correct.u_gain),
				       measurement.prev);
		SMARTHOME_CalcWattHours((tSMARTHOME_DATA *)&mlib,
					(int *)&wh_cnt,
					(Frac64 *)outResPuls,
					(Frac32 *)ramcfg.correct.p_gain);
		SMARTHOME_CalcAuxiliary((tSMARTHOME_DATA *)&mlib);
		measurement.ready = 0;
	}
}

tSMARTHOME_RESULT meter_result;
double            umax;         /*!< maximal voltage measurement range      */
double            imax; 
void adc_task()
{
	
	/* */
	if(is_adc_task_enabled())
//	if(1)
	{
		static uint32_t u8Count = 0;
		static uint32_t u8_adc_sampling_count = 0;
		u8Count++;
//		if(u8Count >= 10) //10
		{
			/* Trigger each 1ms */
			adc_measure_vdd();
			u8Count = 0;
		}
	}

}

void set_I_rms_mA(uint16_t val)
{
	I_rms = val;
}
uint16_t get_I_rms_mA(void)
{
	return I_rms;
}

uint8_t is_adc_task_enabled(void)
{
	return is_adc_enabled;
}

void reset_adc_task_trigger()
{
	is_adc_enabled = 0;
}

void adc_task_trigger(void)
{
	is_adc_enabled = 1;
}
