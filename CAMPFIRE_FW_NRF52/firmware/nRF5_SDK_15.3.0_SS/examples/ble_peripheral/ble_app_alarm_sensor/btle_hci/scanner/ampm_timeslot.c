
/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/

#include "ampm_timeslot.h"

#include "ampm_ll.h"
#include "radio.h"

#include "app_error.h"
#include "nrf_soc.h"
#include "nrf_gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "boards.h"
#include "timer_control.h"

#define TIMESLOT_END_SAFETY_MARGIN_US   (200)
#define TIMESLOT_SLOT_LENGTH            (100000)
#define TIMESLOT_SLOT_EMERGENCY_LENGTH  (3000) /* will fit between two conn events */
#define TIMESLOT_MAX_LENGTH             (1000000) /* 1s */


/*****************************************************************************
* Static globals
*****************************************************************************/

/**
* Timeslot request structures
*/


static nrf_radio_request_t radio_request_normal = 
                {
                    .request_type = NRF_RADIO_REQ_TYPE_NORMAL,
                    .params.normal = 
                    {
                        .hfclk = NRF_RADIO_HFCLK_CFG_DEFAULT,
                        .priority = NRF_RADIO_PRIORITY_NORMAL,
                        .distance_us = 10000,
                        .length_us = TIMESLOT_SLOT_LENGTH
                    }
                };
                
static nrf_radio_request_t radio_request_earliest = 
                {
                    .request_type = NRF_RADIO_REQ_TYPE_EARLIEST,
                    .params.earliest = 
                    {
                        .hfclk = NRF_RADIO_HFCLK_CFG_DEFAULT,
                        .priority = NRF_RADIO_PRIORITY_NORMAL,
                        .length_us = TIMESLOT_SLOT_LENGTH,
                        .timeout_us = 10000 /* 10ms */
                    }
                };
                
                
                  
static nrf_radio_signal_callback_return_param_t g_ret_param;
//static nrf_radio_signal_callback_return_param_t g_final_ret_param;

static bool g_is_in_callback = true;
                
static uint64_t g_timeslot_length;      
static uint32_t g_timeslot_end_timer;      
static uint64_t g_next_timeslot_length;       
static bool g_is_in_timeslot = false;               
static uint32_t g_negotiate_timeslot_length = TIMESLOT_SLOT_LENGTH;

void  timeslot_sys_evt_handler(uint32_t evt)
{
    switch (evt)
    {
        case NRF_EVT_RADIO_SESSION_IDLE:
            timeslot_order_earliest(TIMESLOT_SLOT_LENGTH, true);
            break;

        case NRF_EVT_RADIO_BLOCKED:
            timeslot_order_earliest(TIMESLOT_SLOT_EMERGENCY_LENGTH, true);
            break;

        case NRF_EVT_RADIO_CANCELED:
            timeslot_order_earliest(TIMESLOT_SLOT_LENGTH, true);
            break;

        default:
            break;
    }
}

								
static void end_timer_handler(void)
{
    timeslot_order_earliest(((g_timeslot_length > 100000)? 100000 : g_timeslot_length), true);
}


nrf_radio_signal_callback_return_param_t *radio_signal_callback (uint8_t sig)
{

	g_ret_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
	g_is_in_callback = true;
	static uint32_t requested_extend_time = 0;
	static uint32_t successful_extensions = 0;  


  switch (sig)
  {
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:
			g_is_in_timeslot = true;
			timer_init();
			successful_extensions = 0;
			g_negotiate_timeslot_length = g_timeslot_length;
			g_timeslot_length = g_next_timeslot_length;
			g_timeslot_end_timer = 
					timer_order_cb_sync_exec(g_timeslot_length - TIMESLOT_END_SAFETY_MARGIN_US, 
							end_timer_handler);
			
			/* attempt to extend our time right away */
			timeslot_extend(g_negotiate_timeslot_length);
		
      ampm_ll_start ();
      break;

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
      radio_event_cb ();
      break;

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
			timer_event_handler();
      break;

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_SUCCEEDED:
			g_timeslot_length += requested_extend_time;
			requested_extend_time = 0;
			++successful_extensions;
			g_ret_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
	
			timer_abort(g_timeslot_end_timer);
	
			g_timeslot_end_timer = 
					timer_order_cb_sync_exec(g_timeslot_length - TIMESLOT_END_SAFETY_MARGIN_US, 
							end_timer_handler);
			
			//TICK_PIN(1);
			if (g_timeslot_length + g_negotiate_timeslot_length < TIMESLOT_MAX_LENGTH)
			{
					timeslot_extend(g_negotiate_timeslot_length);   
			}
			else
			{
					/* done extending, check for new trickle event */
					
			}
      break;

    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_FAILED:
			 g_negotiate_timeslot_length >>= 2;
			if (g_negotiate_timeslot_length > 1000)
			{
					timeslot_extend(g_negotiate_timeslot_length);        
			}
			else
			{
					/* done extending, check for new trickle event */
					
			}
      break;
		default:
			 APP_ERROR_CHECK(NRF_ERROR_INVALID_STATE);
  }
	
	g_is_in_callback = false;
	if (g_ret_param.callback_action == NRF_RADIO_SIGNAL_CALLBACK_ACTION_EXTEND)
	{
			requested_extend_time = g_ret_param.params.extend.length_us;
	}
	else if (g_ret_param.callback_action == NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END)
	{
			g_is_in_timeslot = false;
//			nrf_gpio_pin_set(LED_0);
	}
	else
	{
			requested_extend_time = 0;
	}
	
	return &g_ret_param;
}




/*****************************************************************************
* Interface Functions
*****************************************************************************/

void timeslot_handler_init (void)
{
	 uint32_t error;
    
    g_is_in_callback = false;

    
    error = sd_nvic_EnableIRQ(SD_EVT_IRQn);
    APP_ERROR_CHECK(error);
    
    error = sd_radio_session_open(&radio_signal_callback);
    APP_ERROR_CHECK(error);

    g_timeslot_length = TIMESLOT_SLOT_LENGTH;
    timeslot_order_earliest(g_timeslot_length, true); 
}

void timeslot_order_earliest(uint32_t length_us, bool immediately)
{
    if (immediately)
    {
        radio_request_earliest.params.earliest.length_us = length_us;
        g_ret_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
        g_ret_param.params.request.p_next = &radio_request_earliest;
        
        g_next_timeslot_length = length_us;
        
        if (!g_is_in_callback)
        {
            sd_radio_request(&radio_request_earliest);
        }
    }
    else
    {
        radio_request_earliest.params.earliest.length_us = length_us;
        //g_final_ret_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
        //g_final_ret_param.params.request.p_next = &radio_request_earliest;
        
        g_next_timeslot_length = length_us;
    }
}


void timeslot_order_normal(uint32_t length_us, uint32_t distance_us, bool immediately)
{
    if (immediately)
    {
        radio_request_normal.params.normal.length_us = length_us;
        radio_request_normal.params.normal.distance_us = distance_us;
        g_ret_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
        g_ret_param.params.request.p_next = &radio_request_normal;
        
        g_next_timeslot_length = length_us;
        
        if (!g_is_in_callback)
        {
            sd_radio_request(&radio_request_normal);
        }
    }
    else
    {
        radio_request_normal.params.normal.length_us = length_us;
        radio_request_normal.params.normal.distance_us = distance_us;
        //g_final_ret_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
        //g_final_ret_param.params.request.p_next = &radio_request_normal;
        
        g_next_timeslot_length = length_us;
    }
}

void timeslot_extend(uint32_t extra_time_us)
{
    if (g_is_in_callback)
    {
        if (g_timeslot_length + extra_time_us > TIMESLOT_MAX_LENGTH)
        {
            extra_time_us = TIMESLOT_MAX_LENGTH - g_timeslot_length;
        }
        g_ret_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_EXTEND;
        g_ret_param.params.extend.length_us = extra_time_us;
    }
}

uint32_t timeslot_get_remaining_time(void)
{
    if (!g_is_in_timeslot)
    {
        return 0;
    }
    
    uint32_t timestamp = timer_get_timestamp();
    if (timestamp > g_timeslot_length - TIMESLOT_END_SAFETY_MARGIN_US)
    {
        return 0;
    }
    else
    {
        return (g_timeslot_length - TIMESLOT_END_SAFETY_MARGIN_US - timestamp);
    }
}

uint64_t timeslot_get_end_time(void)
{
    if (!g_is_in_timeslot)
    {
        return 0;
    }
    
    return g_timeslot_length;
}

