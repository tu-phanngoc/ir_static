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

#ifndef _TS_CONTROLLER_H__
#define _TS_CONTROLLER_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_soc.h"
#include "btle.h"



#if defined (NRF51)
#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "nrf51_deprecated.h"
#elif defined (NRF52810_XXAA)
#include "nrf52810.h"
#include "nrf52810_bitfields.h"
#include "nrf51_to_nrf52810.h"
#include "nrf52_to_nrf52810.h"
#elif defined (NRF52811_XXAA)
#include "nrf52811.h"
#include "nrf52811_bitfields.h"
#elif defined (NRF52832_XXAA) || defined (NRF52832_XXAB)
#include "nrf52.h"
//#include "nrf52832.h"
//#include "nrf52832_bitfields.h"
#elif defined (NRF52840_XXAA)
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf51_to_nrf52840.h"
#include "nrf52_to_nrf52840.h"
#elif defined (NRF9160_XXAA)
#else
#error "Device must be defined. See nrf.h."
#endif /* NRF51, NRF52810_XXAA, NRF52811_XXAA, NRF52832_XXAA, NRF52832_XXAB, NRF52840_XXAA, NRF9160_XXAA */

#define PACKET_TYPE_LEN             (1)
#define PACKET_LENGTH_LEN           (1)
#define PACKET_ADDR_LEN             (BLE_GAP_ADDR_LEN)

#define PACKET_TYPE_POS             (0)
#define PACKET_LENGTH_POS           (1)
#define PACKET_PADDING_POS          (2)
#define PACKET_ADDR_POS             (3)
#define PACKET_DATA_POS             (PACKET_ADDR_POS + PACKET_ADDR_LEN)

#define PACKET_TYPE_ADV_NONCONN     (0x02)
#define PACKET_TYPE_VTSMART     		(0x07)

#define PACKET_TYPE_MASK            (0x0F)
#define PACKET_LENGTH_MASK          (0x3F)
#define PACKET_ADDR_TYPE_MASK       (0x40)

#define PACKET_DATA_MAX_LEN         (31)
#define PACKET_MAX_CHAIN_LEN        (1)

typedef struct __attribute__((packed))
{
    uint8_t header;
		uint8_t pkg_len;
		uint8_t padding;
		uint8_t addr[6];
    uint8_t pkg[PACKET_DATA_MAX_LEN];
    uint32_t rx_crc;
} ts_packet_t;


#define HFCLK 									NRF_RADIO_HFCLK_CFG_XTAL_GUARANTEED
#define TIMESLOT_LENGTH 				4300	
#define TIMESLOT_INTERVAL_100MS 100000

															
#define BLE_TYPE_OFFSET			(0)
#define BLE_SIZE_OFFSET 		(1)
#define BLE_ADDR_OFFSET 		(3)
#define BLE_PAYLOAD_OFFSET 	(9)
#define BLE_TXADD_OFFSET		(1)

#define BLE_ADDR_TYPE_MASK	(0x40)
#define BLE_TYPE_MASK				(0x3F)
#define BLE_ADDR_LEN				(6)
#define BLE_PAYLOAD_MAXLEN	(31)


/* State machine states */
typedef enum
{
	STATE_ADV_SEND,
	STATE_SCAN_REQ_RSP,
	STATE_WAIT_FOR_IDLE,
} ts_state_t; 



/*****************************************************************************
* Global variables
*****************************************************************************/

extern nrf_radio_signal_callback_return_param_t g_signal_callback_return_param;

/*****************************************************************************
* Interface Functions
*****************************************************************************/

void ctrl_init(void);

void ctrl_signal_handler(uint8_t sig);

bool ctrl_adv_param_set(btle_cmd_param_le_write_advertising_parameters_t* adv_params);

void ctrl_timeslot_order(void);

void ctrl_timeslot_abort(void);

void ctrl_tx_data_callback_set(void (*callback)(ts_packet_t *packet));
void ctrl_rx_data_callback_set(uint32_t (*callback)(ts_packet_t *packet,uint8_t rssi));
uint8_t ts_is_in_timeslot(void);
uint32_t ts_get_ramdom(uint32_t min, uint32_t max);
#endif /* _TS_CONTROLLER_H__ */
