/***********************************************************************************
Copyright (c) Nordic Semiconductor ASA
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  3. Neither the name of Nordic Semiconductor ASA nor the names of other
  contributors to this software may be used to endorse or promote products
  derived from this software without specific prior written permission.

  4. This software must only be used in a processor manufactured by Nordic
  Semiconductor ASA, or in a processor manufactured by a third party that
  is used in combination with a processor manufactured by Nordic Semiconductor.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************/

#include "ts_controller.h"

#include <string.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "nrf_assert.h"
#include "app_error.h"	
#include "btle.h"
#include "ts_peripheral.h"
#include "nrf_advertiser.h"

/*****************************************************************************
* Local Definitions
*****************************************************************************/


/* Disable this flag to disable listening for scan requests, and jump straight to 
* next advertisement 
*/
#define TS_SEND_SCAN_RSP (1)

/* Quick macro to scale the interval to the timeslot scale */
#define ADV_INTERVAL_TRANSLATE(interval) (625 * (interval))

/* short macro to check whether the given event is triggered */
#define RADIO_EVENT(x) (NRF_RADIO->x != 0)

/*****************************************************************************
* Static Globals
*****************************************************************************/


ts_packet_t radio_rx_packet;
ts_packet_t radio_tx_packet;

/* Store the radio channel */
static uint8_t channel;
static uint8_t timeslot_cnt = 0;
static uint8_t channel_use_cnt = 0;
/* running flag for advertiser. Decision point: end of timeslot (adv event) */
static bool sm_adv_run;

/* Channel map for advertisement */
static btle_dd_channel_map_t channel_map;
	
/* min advertisement interval */
static btle_adv_interval_t adv_int_min;

/* max advertisement interval */
static btle_adv_interval_t adv_int_max;

/* statemachine state */
static ts_state_t sm_state;

/* Pool of 255 (the range of the RNG-peripheral) psedorandomly generated values */
static uint8_t rng_pool[255];

/* A pointer into our pool. Will wrap around upon overflow */
static uint8_t pool_index = 0;

/* Packet counter */
static uint16_t packet_count_valid;

/* Faulty packets counter */
static uint16_t packet_count_invalid;

static uint8_t is_in_timeslot = 0;

static  uint8_t m_rssi;

/*****************************************************************************
* Globals
*****************************************************************************/

/* return param in signal handler */
nrf_radio_signal_callback_return_param_t g_signal_callback_return_param;


/* timeslot request EARLIEST. Used to send the first timeslot request */
static nrf_radio_request_t g_timeslot_req_earliest = 
			{NRF_RADIO_REQ_TYPE_EARLIEST, 
			.params.earliest = {
						HFCLK, 
						NRF_RADIO_PRIORITY_NORMAL, 
						TIMESLOT_LENGTH, 		
						10000}
			};

/* timeslot request NORMAL. Used to request a periodic timeslot, i.e. advertisement events */
static nrf_radio_request_t g_timeslot_req_normal =
			{NRF_RADIO_REQ_TYPE_NORMAL, 
			.params.normal = {
						HFCLK, 
						NRF_RADIO_PRIORITY_NORMAL, 
						TIMESLOT_INTERVAL_100MS, 		
						TIMESLOT_LENGTH}
			};




/*****************************************************************************
* Static Functions
*****************************************************************************/
void (*ble_tx_data_calback)(ts_packet_t *packet);
uint32_t (*ble_rx_data_calback)(ts_packet_t *packet,uint8_t rssi);
			
/**
* Get next channel to use. Affected by the tsa adv channels.
*/			
static __INLINE void channel_iterate(void)
{
	//while (((channel_map & (1 << (++channel - 37))) == 0) && channel < 40);	
	//channel++;
	//if(channel > 39) 
		channel = 38;
	channel_use_cnt++;	
}
			


/**
* Send initial time slot request to API. 
*/
static __INLINE void timeslot_req_initial(void)
{	
	DEBUG_PIN_POKE(7);
	/* send to sd: */
	uint8_t error_code = sd_radio_request(&g_timeslot_req_earliest);
	APP_ERROR_CHECK(error_code);
}


/**
* Short check to verify that the incomming 
* message was a scan request for this unit
*/
static bool have_message_to_send(void)
{	
	/* check CRC. Add to number of CRC faults if wrong */
	if (0 == NRF_RADIO->CRCSTATUS) 
	{
		++packet_count_invalid;
		return false;
	}
	++packet_count_valid;
	if(ble_rx_data_calback(&radio_rx_packet,m_rssi))
	{
		radio_tx_packet = radio_rx_packet;
		return true;
	}
	return false;
}

uint32_t ts_get_ramdom(uint32_t min, uint32_t max)
{
	return  min + (rng_pool[pool_index++]) % (max - min);
}

/**
* Request a new timeslot for the next advertisement event
* If the advertiser has been told to stop, the function will not 
* schedule any more, but just end the timeslot.
*/
static __INLINE void next_timeslot_schedule(void)
{
	//if (sm_adv_run)
	if(0)
	{
		g_timeslot_req_normal.params.normal.distance_us = ADV_INTERVAL_TRANSLATE(adv_int_min) 
																										+ 1000 * ((rng_pool[pool_index++]) % (ADV_INTERVAL_TRANSLATE(adv_int_max - adv_int_min)));
		g_signal_callback_return_param.params.request.p_next = &g_timeslot_req_normal;
		g_signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
		NRF_TIMER0->TASKS_STOP = 1;
	}
	else
	{
		NRF_TIMER0->TASKS_STOP = 1;
		g_signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_END;
	}
}

/**
* Doing the setup actions before the first adv_send state actions
*/
static __INLINE void adv_evt_setup(void)
{
	periph_radio_setup();
	
	/* set channel to first in sequence */
	//channel = 36; /* will be iterated by channel_iterate() */
	channel_use_cnt = 0;
	//channel_use_cnt = 1;
	channel = 37 + (timeslot_cnt % 3);
}	
	

uint8_t ts_is_in_timeslot(void)
{
	return is_in_timeslot;
}



/******************************************
* Functions for start/end of adv_send state 
******************************************/
static void sm_enter_adv_send(void)
{
	sm_state = STATE_ADV_SEND;
	periph_radio_ch_set(channel);
	
	/* trigger task early, the rest of the setup can be done in RXRU */
	PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_TXEN);
	ble_tx_data_calback(&radio_tx_packet);
	//ble_adv_data[11] = 'A' + (cnt++ % 60);
	periph_radio_packet_ptr_set((uint8_t *)&radio_tx_packet);
	
	periph_radio_shorts_set(	RADIO_SHORTS_READY_START_Msk | 
														RADIO_SHORTS_END_DISABLE_Msk |
														RADIO_SHORTS_DISABLED_RXEN_Msk);
	
	periph_radio_intenset(RADIO_INTENSET_DISABLED_Msk);
}

static void sm_exit_adv_send(void)
{
	/* wipe events and interrupts triggered by this state */
	periph_radio_intenclr(RADIO_INTENCLR_DISABLED_Msk);
	PERIPHERAL_EVENT_CLR(NRF_RADIO->EVENTS_DISABLED);
}


/******************************************
* Functions for start/end of SCAN_REQ_RSP
******************************************/
static void sm_enter_scan_req_rsp(void)
{
	sm_state = STATE_SCAN_REQ_RSP;
	periph_radio_packet_ptr_set((uint8_t *)&radio_rx_packet);
	
	periph_radio_shorts_set(	RADIO_SHORTS_READY_START_Msk | 
														RADIO_SHORTS_END_DISABLE_Msk |
														RADIO_SHORTS_DISABLED_TXEN_Msk |
														RADIO_SHORTS_ADDRESS_RSSISTART_Msk);
	
	periph_radio_intenset(	RADIO_INTENSET_DISABLED_Msk);
	
	/* change the tifs in order to be able to capture all packets */
	periph_radio_tifs_set(200);
	
	/* start the timer that aborts the RX if no address is received. */
	periph_timer_start(0, 300, true);
	
	/* set PPI pipe to stop the timer as soon as an address is received */
	periph_ppi_set(0, &(NRF_TIMER0->TASKS_STOP), &(NRF_RADIO->EVENTS_ADDRESS));
}

static void sm_exit_scan_req_rsp(void)
{
	periph_timer_abort(0);
	periph_radio_intenclr(RADIO_INTENCLR_DISABLED_Msk);
	PERIPHERAL_EVENT_CLR(NRF_RADIO->EVENTS_DISABLED);
	periph_ppi_clear(0);
}

/*****************************************
* Functions for start/end of WAIT_FOR_IDLE
******************************************/
static void sm_enter_wait_for_idle(bool req_rx_accepted)
{
	sm_state = STATE_WAIT_FOR_IDLE;
	/* enable disabled interrupt to avoid race conditions */
	periph_radio_intenset(RADIO_INTENSET_DISABLED_Msk);
	
	/* different behaviour depending on whether we actually 
	received a scan request or not */
	if (req_rx_accepted)
	{
		/* need to answer request, set scan_rsp packet and 
		let radio continue to send */
		
		periph_radio_packet_ptr_set((uint8_t *)&radio_tx_packet);
		periph_radio_shorts_set(RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk);
		
		/* wait exactly 200us to send response. NOTE: the Reference manual is wrong */
		periph_radio_tifs_set(200);
	}
	else
	{
		/* remove shorts and disable radio */
		periph_radio_shorts_set(0);
		PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_DISABLE);
	}
}


static bool sm_exit_wait_for_idle(void)
{
	periph_radio_intenclr(RADIO_INTENCLR_DISABLED_Msk);
	PERIPHERAL_EVENT_CLR(NRF_RADIO->EVENTS_DISABLED);
	
	channel_iterate();
	
	/* return whether the advertisement event is done */
	return (channel_use_cnt > 2);
}

/*****************************************************************************
* Interface Functions
*****************************************************************************/

void ctrl_init(void)
{
	/* generate rng sequence */
	adv_rng_init(rng_pool);
	
}




__INLINE void ctrl_signal_handler(uint8_t sig)
{
	switch (sig)
	{
		case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:	
			is_in_timeslot = 1;
			timeslot_cnt++;
			DEBUG_PIN_POKE(3);
			adv_evt_setup();
			sm_enter_adv_send();
			break;
		
		case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
		{
			DEBUG_PIN_POKE(0);
			
			/* check state, and act accordingly */
			switch (sm_state)
			{
				case STATE_ADV_SEND:
					if (RADIO_EVENT(EVENTS_DISABLED))
					{
						DEBUG_PIN_SET(12);
						
						sm_exit_adv_send();
#if TS_SEND_SCAN_RSP
						sm_enter_scan_req_rsp();
#else 
						sm_enter_wait_for_idle(false);
#endif
					}
					break;
					
				case STATE_SCAN_REQ_RSP:
					
					if (RADIO_EVENT(EVENTS_DISABLED))
					{
						sm_exit_scan_req_rsp();
						sm_enter_wait_for_idle(have_message_to_send());
					}
					break;
					
				case STATE_WAIT_FOR_IDLE:
					if (RADIO_EVENT(EVENTS_DISABLED))
					{
						/* state exit function returns whether the adv event is complete */
						bool adv_evt_done = sm_exit_wait_for_idle();
						
						if (adv_evt_done)
						{
							next_timeslot_schedule();
							is_in_timeslot = 0;
						}
						else
						{
							sm_enter_adv_send();
						}
					}
					break;							
			
				default:
					/* Shouldn't happen */
					ASSERT(false);
				}
		}
			break;
		case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
			DEBUG_PIN_POKE(5);
			
			sm_exit_scan_req_rsp();
		
			/* go to wait for idle, no packet was accepted */
			sm_enter_wait_for_idle(false);
		
			PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_DISABLE);
		
			break;
		
		default:
			/* shouldn't happen in this advertiser. */

			DEBUG_PIN_SET(LED_0);
			DEBUG_PIN_POKE(13);		
			DEBUG_PIN_POKE(14);		
	}
	
}	


bool ctrl_adv_param_set(btle_cmd_param_le_write_advertising_parameters_t* adv_params)
{
	ASSERT(adv_params != NULL);
	/* Checks for error */
	
	/* channel map */
	if (0x00 == adv_params->channel_map || 0x07 < adv_params->channel_map)
	{
		return false;
	}
	
	/* address */
	if (NULL == adv_params->direct_address)
	{
		return false;
	}
	
	/* set channel map */
	channel_map = adv_params->channel_map;
	
	/* Advertisement interval */
	adv_int_min = adv_params->interval_min;
	adv_int_max = adv_params->interval_max;
	
	
	return true;
}

void ctrl_timeslot_order(void)
{
	sm_adv_run = true;
	timeslot_req_initial();
}

void ctrl_timeslot_abort(void)
{
	sm_adv_run = false;
}


void ctrl_tx_data_callback_set(void (*callback)(ts_packet_t *packet))//added by thienhaiblue@gmail.com#date:26/10/2015
{
	ble_tx_data_calback = callback;
}
void ctrl_rx_data_callback_set(uint32_t (*callback)(ts_packet_t *packet,uint8_t rssi))//added by thienhaiblue@gmail.com#date:26/10/2015
{
	ble_rx_data_calback = callback;
}

