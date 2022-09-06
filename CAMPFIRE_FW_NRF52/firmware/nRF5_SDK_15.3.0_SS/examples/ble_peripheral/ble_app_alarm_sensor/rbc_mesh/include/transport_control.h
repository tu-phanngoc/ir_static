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

#ifndef _TRANSPORT_CONTROL_H__
#define _TRANSPORT_CONTROL_H__
#include <stdint.h>
#include "mesh_srv.h"
/**
* @file This module takes care of all lower level packet processing and 
*   schedules the radio for transmission. Acts as the link between the radio
*   and the mesh service.
*/

/** 
* @brief Called at the beginning of a timeslot with a timestamp in order to let
*   the system catch up with any lost time between timeslots
* 
* @param[in] global_timer_value The timestamp to use as reference for whether
*   there is anything to process.
*/


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

#define PACKET_DATA_MAX_LEN         (28)
#define PACKET_MAX_CHAIN_LEN        (1) /**@TODO: May be increased when RX 
                                        callback packet chain handling is implemented.*/


void transport_control_timeslot_begin(uint64_t global_timer_value);
void packet_create_from_data(uint8_t* data, packet_t* packet);
/**
* @brief Force a check for timed out values
*/
void transport_control_step(void);

#endif /* _TRANSPORT_CONTROL_H__ */
