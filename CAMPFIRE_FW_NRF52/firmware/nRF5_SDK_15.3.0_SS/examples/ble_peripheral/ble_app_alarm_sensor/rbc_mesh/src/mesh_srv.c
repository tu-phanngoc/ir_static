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

#include "mesh_srv.h"
#include "rbc_mesh.h"
#include "timeslot_handler.h"
#include "trickle.h"
#include "rbc_mesh_common.h"
#include "mesh_aci.h"
#include "transport_control.h"
#include "nrf_soc.h"
#include "nrf_error.h"
#include "ble.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

extern ble_gap_addr_t _device_addr;


/*****************************************************************************
* Local Type Definitions
*****************************************************************************/

typedef struct
{
    uint8_t value_count;
    mesh_char_metadata_t* char_metadata;
} mesh_srv_t;



/*****************************************************************************
* Static globals
*****************************************************************************/
static mesh_srv_t g_mesh_service = {0, NULL};
static bool is_initialized = false;
                                         

/*****************************************************************************
* Static functions
*****************************************************************************/


/*****************************************************************************
* Interface functions
*****************************************************************************/

uint32_t mesh_srv_init(uint8_t mesh_value_count, 
    uint32_t access_address, uint8_t channel, uint32_t adv_int_ms)
{
    if (mesh_value_count > MAX_VALUE_COUNT)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    if (is_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    is_initialized = true;
    
    g_mesh_service.value_count = mesh_value_count;
    
    uint32_t md_len = sizeof(mesh_char_metadata_t) * g_mesh_service.value_count;
    
    /* allocate metadata array */
    g_mesh_service.char_metadata = (mesh_char_metadata_t*) malloc(md_len);    
    memset(g_mesh_service.char_metadata, 0, md_len);
    
		
    trickle_setup(1000 * adv_int_ms, 600, 3);
    
    return NRF_SUCCESS;
}


uint32_t mesh_srv_char_val_set(uint8_t index, uint8_t* data, uint16_t len, bool update_sender)
{
    if (!is_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    if (index > g_mesh_service.value_count || index == 0)
    {
        return NRF_ERROR_INVALID_ADDR;
    }
    
    if (len > MAX_VALUE_LENGTH)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    
    
    mesh_char_metadata_t* ch_md = &g_mesh_service.char_metadata[index - 1];
    
    /* this is now a new version of this data, signal to the rest of the mesh */
    ++ch_md->version_number;
    
    bool first_time = (ch_md->flags & (1 << MESH_MD_FLAGS_USED_POS)) == 0;
    
    if (first_time)
    {
        ch_md->flags |= 
            (1 << MESH_MD_FLAGS_INITIALIZED_POS) |
            (1 << MESH_MD_FLAGS_USED_POS);
        trickle_init(&ch_md->trickle);
    }
    else
    {
        trickle_rx_inconsistent(&ch_md->trickle);
    }
    
    if (update_sender || first_time)
    {
        memcpy(&ch_md->last_sender_addr, &_device_addr, sizeof(ble_gap_addr_t));
        ch_md->flags |= (1 << MESH_MD_FLAGS_IS_ORIGIN_POS);
    }
    memcpy(&ch_md->data,data,len);
		ch_md->data_len = len;
		
		rbc_mesh_event_t update_evt;
		update_evt.event_type = ((first_time)? 
				RBC_MESH_EVENT_TYPE_NEW_VAL :
				RBC_MESH_EVENT_TYPE_UPDATE_VAL);
		update_evt.data_len = ch_md->data_len;
		update_evt.value_handle = index;
		update_evt.data = ch_md->data;
		memcpy(&update_evt.originator_address, &ch_md->last_sender_addr, sizeof(ble_gap_addr_t));
//		rbc_mesh_event_handler(&update_evt);
    return NRF_SUCCESS;
}

uint32_t mesh_srv_char_val_get(uint8_t index, uint8_t* data, uint16_t* len, ble_gap_addr_t* origin_addr)
{
    if (!is_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    if (index > g_mesh_service.value_count || index == 0)
    {
        return NRF_ERROR_INVALID_ADDR;
    }
    mesh_char_metadata_t* md_ch = &g_mesh_service.char_metadata[index - 1];
		if(data)
			memcpy(data,md_ch->data,md_ch->data_len);
		*len = md_ch->data_len;
   
    if (origin_addr != NULL)
    {
        memcpy(origin_addr, 
            &g_mesh_service.char_metadata[index - 1].last_sender_addr, 
            sizeof(ble_gap_addr_t));
    }
    
    return NRF_SUCCESS;
}


uint32_t mesh_srv_get_next_processing_time(uint64_t* time)
{
    if (!is_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    bool anything_to_process = false;
    *time = UINT64_MAX;
    
    for (uint8_t i = 0; i < g_mesh_service.value_count; ++i)
    {
        if ((g_mesh_service.char_metadata[i].flags & (1 << MESH_MD_FLAGS_USED_POS)) == 0)
            continue;
            
        uint64_t temp_time = trickle_next_processing_get(&g_mesh_service.char_metadata[i].trickle);
        
        if (temp_time < *time)
        {
            anything_to_process = true;
            *time = temp_time;
        }
    }
    if (!anything_to_process)
    {
        return NRF_ERROR_NOT_FOUND;
    }
        
    return NRF_SUCCESS;
}

uint32_t mesh_srv_packet_process(packet_t* packet)
{
	DBG_MESH_SRV("mesh_srv_packet_process \r\n");
    if (!is_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    uint32_t error_code;
		char buf[128],len = 0;
    static uint8_t printTableCnt = 0;
    uint8_t handle = packet->data[MESH_PACKET_HANDLE_OFFSET];
    uint16_t version = (packet->data[MESH_PACKET_VERSION_OFFSET] | 
                    (((uint16_t) packet->data[MESH_PACKET_VERSION_OFFSET + 1]) << 8));
    uint8_t* data = &packet->data[MESH_PACKET_DATA_OFFSET];
    uint16_t data_len = packet->length - MESH_PACKET_DATA_OFFSET;
		printTableCnt++;
		if(printTableCnt >= 10)
		{
			printTableCnt = 0;
											//packet->1F4B7DEEF8F1   01      06   014B0100010000004C01002000000000BF010000
			DBG_MESH_SRV("MESH SENDER_ADDR |HANDLE|VERSION|*******DATA**************|\r\n");
		}
	
		uint8_t i;
		for (i = 0;i < 6;i++)
		{
			len += sprintf(&buf[len],"%02X",packet->sender.addr[i]);
		}
		len += sprintf(&buf[len],"   %02X   ",handle);
		len += sprintf(&buf[len]," %04X ",version);
		for (i = 0;i < data_len;i++)
		{
			len += sprintf(&buf[len],"%02X",data[i]);
		}
		len += sprintf(&buf[len],"\r\n");
		DBG_MESH_SRV("%s",(uint32_t)buf);
    if (data_len > MAX_VALUE_LENGTH)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    
    if (handle > g_mesh_service.value_count || handle == 0)
    {
        return NRF_ERROR_INVALID_ADDR;
    }
    
    
    mesh_char_metadata_t* ch_md = &g_mesh_service.char_metadata[handle - 1];
    
    bool uninitialized = !(ch_md->flags & (1 << MESH_MD_FLAGS_INITIALIZED_POS));
    
    if (uninitialized)
    {
        trickle_init(&ch_md->trickle);
				DBG_MESH_SRV_PUTS("\r\nMESH_SRV:trickle_init()");
    }
   // DBG_MESH_SRV("\r\nMESH_SRV:Version:%d",ch_md->version_number);
    if (ch_md->version_number != version)
    {
        trickle_rx_inconsistent(&ch_md->trickle);
				DBG_MESH_SRV_PUTS("\r\nMESH_SRV:inconsistent");
    }
    
    /* new version */  
    uint16_t separation = (version > ch_md->version_number)?
        (version - ch_md->version_number) : 
        (-(ch_md->version_number - MESH_VALUE_LOLLIPOP_LIMIT) + (version - MESH_VALUE_LOLLIPOP_LIMIT) - MESH_VALUE_LOLLIPOP_LIMIT);

    if ((ch_md->version_number < MESH_VALUE_LOLLIPOP_LIMIT && version > ch_md->version_number) || 
        (ch_md->version_number >= MESH_VALUE_LOLLIPOP_LIMIT && separation < (UINT16_MAX - MESH_VALUE_LOLLIPOP_LIMIT)/2) || 
        uninitialized)
    {
        /* update value */
				DBG_MESH_SRV("\r\nMESH_SRV:%d#",data_len);
				uint8_t i;
				for (i = 0;i < data_len;i++)
				{
					DBG_MESH_SRV("%02X",data[i]);
				}
        mesh_srv_char_val_set(handle, data, data_len, false);
        ch_md->flags |= (1 << MESH_MD_FLAGS_INITIALIZED_POS);
        ch_md->flags &= ~(1 << MESH_MD_FLAGS_IS_ORIGIN_POS);
        ch_md->version_number = version;
        
        /* Manually set originator address */
        memcpy(&ch_md->last_sender_addr, &packet->sender, sizeof(ble_gap_addr_t));
        
        rbc_mesh_event_t update_evt;
        update_evt.event_type = ((uninitialized)? 
            RBC_MESH_EVENT_TYPE_NEW_VAL :
            RBC_MESH_EVENT_TYPE_UPDATE_VAL);
        update_evt.data_len = data_len;
        update_evt.value_handle = handle;
        
        update_evt.data = data;
        memcpy(&update_evt.originator_address, &packet->sender, sizeof(ble_gap_addr_t));
        
        rbc_mesh_event_handler(&update_evt);
#ifdef RBC_MESH_SERIAL
				mesh_aci_rbc_event_handler(&update_evt);
#endif
    }
    else if (version == ch_md->version_number)
    {
        /* check for conflicting data */
        uint16_t old_len = MAX_VALUE_LENGTH;
        
        error_code = mesh_srv_char_val_get(handle, NULL, &old_len, NULL);
        if (error_code != NRF_SUCCESS)
        {
            return error_code;
        }
        
        volatile bool conflicting = false;
        
        if (packet->rx_crc != ch_md->crc && 
            !(ch_md->flags & (1 << MESH_MD_FLAGS_IS_ORIGIN_POS)))
        {
            conflicting = true;
        }
        else if (old_len != data_len)
        {
            conflicting = true;
        }
        
        
        if (conflicting)
        {
//            TICK_PIN(7);
            rbc_mesh_event_t conflicting_evt;
            
            conflicting_evt.event_type = RBC_MESH_EVENT_TYPE_CONFLICTING_VAL;
            
            conflicting_evt.data_len = data_len;
            conflicting_evt.value_handle = handle;
            
            conflicting_evt.data = data;
            memcpy(&conflicting_evt.originator_address, &packet->sender, sizeof(ble_gap_addr_t));
            
            trickle_rx_inconsistent(&ch_md->trickle);
            
            rbc_mesh_event_handler(&conflicting_evt);
#ifdef RBC_MESH_SERIAL
            mesh_aci_rbc_event_handler(&conflicting_evt);
#endif
        }
        else
        { 
            trickle_rx_consistent(&ch_md->trickle);
        }
        
    }
    
    
    ch_md->crc = packet->rx_crc;
    
    return NRF_SUCCESS;
}



uint32_t mesh_srv_packet_assemble(packet_t* packet, 
    uint16_t packet_max_len, 
    bool* has_anything_to_send)
{
    *has_anything_to_send = false;
    if (!is_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t error_code;
    //DBG_MESH_SRV("\r\nMESH_SRV:CNT#%d#",g_mesh_service.value_count);
    for (uint8_t i = 0; i < g_mesh_service.value_count; ++i)
    {
        mesh_char_metadata_t* md_ch = &g_mesh_service.char_metadata[i];
        
        if ((md_ch->flags & (1 << MESH_MD_FLAGS_USED_POS)) == 0)
            continue;
        
        bool do_trickle_tx = false;
        trickle_step(&md_ch->trickle, &do_trickle_tx);
        

        if (do_trickle_tx && !(*has_anything_to_send))
        {
            trickle_register_tx(&md_ch->trickle); 
            uint8_t data[MAX_VALUE_LENGTH];
            uint16_t len = MAX_VALUE_LENGTH;

            error_code = mesh_srv_char_val_get(i + 1, data, &len, NULL);

            if (error_code != NRF_SUCCESS)
            {
                return error_code;
            }

            packet->data[MESH_PACKET_HANDLE_OFFSET] = i + 1;
            packet->data[MESH_PACKET_VERSION_OFFSET] = 
                (md_ch->version_number & 0xFF); 
            packet->data[MESH_PACKET_VERSION_OFFSET + 1] = 
                ((md_ch->version_number >> 8) & 0xFF);
            
            memcpy(&packet->data[MESH_PACKET_DATA_OFFSET], data, len);
            packet->length = len + MESH_PACKET_DATA_OFFSET;
            
            memcpy(&packet->sender, &md_ch->last_sender_addr, sizeof(md_ch->last_sender_addr));
           
            /**@TODO: Add multiple trickle messages in one packet */

            *has_anything_to_send = true;
            //break;
        }
    }
    
    return NRF_SUCCESS;
}


uint32_t mesh_srv_get_packet(packet_t* packet,uint8_t handle,uint16_t *version)
{
    if (!is_initialized || (handle > g_mesh_service.value_count) || (handle == 0))
    {
        return NRF_ERROR_INVALID_STATE;
    }
		handle--;
		mesh_char_metadata_t* md_ch = &g_mesh_service.char_metadata[handle];
		uint16_t len;
		len = md_ch->data_len;
		if(len > MAX_VALUE_LENGTH)
			len = MAX_VALUE_LENGTH;
		packet->data[MESH_PACKET_HANDLE_OFFSET] = handle + 1;
		packet->data[MESH_PACKET_VERSION_OFFSET] = (md_ch->version_number & 0xFF); 
		packet->data[MESH_PACKET_VERSION_OFFSET + 1] = ((md_ch->version_number >> 8) & 0xFF);
		*version = md_ch->version_number;
		memcpy(&packet->data[MESH_PACKET_DATA_OFFSET], md_ch->data, len);
		packet->length = len + MESH_PACKET_DATA_OFFSET;
		memcpy(&packet->sender, &md_ch->last_sender_addr, sizeof(md_ch->last_sender_addr));
    return NRF_SUCCESS;
}


uint32_t mesh_srv_char_val_enable(uint8_t index)
{
    if (!is_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    if (index > g_mesh_service.value_count || index == 0)
    {
        return NRF_ERROR_INVALID_ADDR;
    }
    
    trickle_init(&g_mesh_service.char_metadata[index - 1].trickle);
    
    g_mesh_service.char_metadata[index - 1].flags |= 
        (1 << MESH_MD_FLAGS_INITIALIZED_POS) |
        (1 << MESH_MD_FLAGS_USED_POS);
    
    
    return NRF_SUCCESS;
}

uint32_t mesh_srv_char_val_disable(uint8_t index)
{
    if (!is_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    if (index > g_mesh_service.value_count || index == 0)
    {
        return NRF_ERROR_INVALID_ADDR;
    }
    
    g_mesh_service.char_metadata[index - 1].flags &=
        ~(1 << MESH_MD_FLAGS_USED_POS);
    
    return NRF_SUCCESS;
}
