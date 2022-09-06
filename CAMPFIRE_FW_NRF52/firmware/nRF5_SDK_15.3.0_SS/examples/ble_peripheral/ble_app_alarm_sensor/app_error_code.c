#include "stdint.h"
#include "app_error_code.h"
#include "app_ble.h"
#include "nrf_delay.h"
#include "app_mesh.h"
#include "crc32.h"
error_list_t err_list;
uint32_t gu32_reset_reason = 0;

#define ERROR_CODE_LOG		NRF_LOG_INFO
extern ble_gap_addr_t _device_addr;


void error_code_init(void)
{
//	for(uint8_t i = 0; i < 32;i++)
//	{
//		if((gu32_reset_reason >> i) && 0x01 == 1)
//		{
//			err_list.err_system_reset = i + 1;
//		}
//	}
	
	err_list.err_system_reset = (gu32_reset_reason & 0x0F) | ((gu32_reset_reason >> 12) & 0xF0);
	ERROR_CODE_LOG("Reset reason: %d................!!!", err_list.err_system_reset);
}

uint8_t error_code_set(uint8_t err)
{
	
}

uint8_t error_code_clear(uint8_t err)
{
	ERROR_CODE_LOG("error_code_clear: %d", err);	
	if(err > 0)
	{
		err_list.err_system_reset = 0;
	}
}

uint8_t error_code_task(void)
{
	if(err_list.err_system_reset != 0)
	{
		error_code_report_reset();
	}
}


void error_code_report_reset(void)
{
	{
		NRF_LOG_INFO("error_code_report_reset");
		uint8_t data[10];
		for(uint8_t i = 0; i < 10; i++)
		{
			data[i] = 0;
		}
		uint32_t resetRtc = rtcTimeSec;
		data[0] = resetRtc & 0xFF;
		data[1] = (resetRtc >> 8) & 0xFF;
		data[2] = (resetRtc >> 16) & 0xFF;
		data[3] = (resetRtc >> 24) & 0xFF;

		data[4] = err_list.err_system_reset;
		
		
		
		ISMARTPACKAGE iSmartPacket;
		iSmartPacket.handle = 0xFF;// "This message from app";

		memcpy(&iSmartPacket.packet.addr, _device_addr.addr, 6);
		memcpy(&iSmartPacket.packet.data, data, 10);
		iSmartPacket.packet.command = 0x12;
		iSmartPacket.packet.type = 0x07;
		iSmartPacket.packet.sequence = get_sequence() + 1;
		iSmartPacket.packet.crc = crc32_compute(&iSmartPacket.packet.addr[0], 20, NULL);
		App_iSmartPacketAddToList(&iSmartPacket);

	//	App_iSmartAppPacketAddToList(&iSmartPacket);
	}
}
