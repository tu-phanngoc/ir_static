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

#ifndef _RBC_MESH_COMMON_H__
#define _RBC_MESH_COMMON_H__
#include <stdint.h>
#include "nrf_log.h"
#if defined(DEBUG_MODE ) && (defined(USE_PCA10040) || defined(USE_PCA10028))
#define RBC_MESH_DEBUG  (0)
#else
#define RBC_MESH_DEBUG  (0)
#endif

#if RBC_MESH_DEBUG
#define DBG_MESH_SRV(...) NRF_LOG_INFO(__VA_ARGS__)
#define DBG_MESH_SRV_PUTS //dbg_puts
#else
#define DBG_MESH_SRV(...) //dbg_printf(__VA_ARGS__)
#define DBG_MESH_SRV_PUTS 
#endif
/******************************************************************************
* Debug related defines
******************************************************************************/

#define TICK_PIN(x) 
#define SET_PIN(x) 
#define CLEAR_PIN(x) 


#define PIN_MESH_TX         (11)
#define PIN_SEARCHING       (12)
#define PIN_CPU_IN_USE      (13)
#define PIN_CONSISTENT      (14)
#define PIN_INCONSISTENT    (15)
#define PIN_RX              (16)
#define PIN_BUTTON          (17)
#define PIN_ABORTED         (18)

#define PIN_INT0            (25)
#define PIN_INT1            (26)
#define PIN_TX0             (27)
#define PIN_TX1             (28)
#define PIN_SYNC_TIME       (29)

#define PIN_RADIO_SIGNAL    (3)
#define PIN_TIMER_SIGNAL    (4)
#define PIN_IN_TIMESLOT     (6)

#define DEBUG_RADIO         (0)

#define PIN_RADIO_STATE_RX  (3)
#define PIN_RADIO_STATE_TX  (4)
#define PIN_RADIO_STATE_IDLE (6)

#if DEBUG_RADIO
    #define DEBUG_RADIO_SET_PIN(x) NRF_GPIO->OUTSET = (1 << (x))
    #define DEBUG_RADIO_CLEAR_PIN(x) NRF_GPIO->OUTCLR = (1 << (x))
#else
    #define DEBUG_RADIO_SET_PIN(x)
    #define DEBUG_RADIO_CLEAR_PIN(x)
#endif


#define PIN_BIT_H           (25)
#define PIN_BIT_L           (28)

#if RBC_MESH_DEBUG
    #define PIN_OUT(val,bitcount)      //for (uint8_t i = 0; i < (bitcount); ++i){ if (((val) >> ((bitcount) - 1 - i) & 0x01)) { TICK_PIN(PIN_BIT_H); } else { TICK_PIN(PIN_BIT_L); } }
#else
    #define PIN_OUT(val,bitcount)   
#endif

#endif /* _RBC_MESH_COMMON_H__ */
