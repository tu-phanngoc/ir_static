#include <string.h>
#include "ble_gap.h"
#include "app_uart.h"
#include "lib/protocol.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "lib/ampm_list.h"
#include "aes128.h"
#include "app_message_queue.h"
#include "app_mesh.h"
#include "smart_sensor_srv.h"
#include "nrf_log.h"
#include "crc32.h"
//DbgCfgPrintf(ALL_LOG,__VA_ARGS__)

#define APP_MSG_QUEUE_ENABLE
#ifdef APP_MSG_QUEUE_ENABLE
#define DBG_APP(...) NRF_LOG_INFO(__VA_ARGS__)
#else
#define DBG_APP(...)
#endif

ble_gap_addr_t _device_addr;
uint16_t u16_sequence = 0;
LIST(iSmartPacketList);
LIST(iSmartMeshPacketList);
LIST(iSmartAppPacketList);
LIST(iSmartEventPacketList);

PROTO_PARSER appProtocol;
uint8_t appProtoBuf[41];
extern bool ble_attempt_to_send(uint8_t * data, uint8_t length);
uint8_t U8CheckSum(uint8_t *buff, uint32_t length);

uint8_t sensor_event_flag = 0;
#define ISMART_PACKET_BOX_SIZE 10

ISMARTPACKAGE iSmartPacketBuf[ISMART_PACKET_BOX_SIZE];
ISMART_PACKET_BOX iSmartPacketBox[ISMART_PACKET_BOX_SIZE];

ISMARTPACKAGE iSmartAppPacketBuf[ISMART_PACKET_BOX_SIZE];
ISMART_PACKET_BOX iSmartAppPacketBox[ISMART_PACKET_BOX_SIZE];

ISMART_MESH_PACKET_LIST iSmartMeshPacketBox[ISMART_PACKET_BOX_SIZE];
transmit_queue_t msg_temp_queue;

ISMART_EVENT_PACKET_LIST iSmartEventPacketBox[ISMART_PACKET_BOX_SIZE];

/* Public functions */
void AppMessageQueueInit(void)
{
	list_init(iSmartPacketList);
	list_init(iSmartMeshPacketList);
	list_init(iSmartAppPacketList);
	list_init(iSmartEventPacketList);
	/* Get device address for packaging packet*/
	#if (NRF_SD_BLE_API_VERSION >= 3)
			sd_ble_gap_addr_get(&_device_addr);
	#else
			sd_ble_gap_address_get(&_device_addr);
	#endif
}

void AppShowiSmartPacket(ISMARTPACKAGE iSmartPacket)
{
	
	DBG_APP("AppShowiSmartPacket:");
	DBG_APP("------------------------------------------------------");
		static uint8_t infoPrintTableCnt = 0;
		infoPrintTableCnt++;
		char buf[128];
		char len = 0;

		DBG_APP("%02X%02X%02X%02X%02X%02X", 
		iSmartPacket.packet.addr[0], 
		iSmartPacket.packet.addr[1], 
		iSmartPacket.packet.addr[2], 
		iSmartPacket.packet.addr[3], 
		iSmartPacket.packet.addr[4], 
		iSmartPacket.packet.addr[5]
		);
		
		DBG_APP("%02X %02X %02X", 
		iSmartPacket.handle,
		iSmartPacket.packet.command,
		iSmartPacket.packet.type
		);
		
		DBG_APP("%02X%02X%02X%02X", 
		iSmartPacket.packet.data[0],
		iSmartPacket.packet.data[1],
		iSmartPacket.packet.data[2],
		iSmartPacket.packet.data[3]
		);
				
		DBG_APP("%02X %02X %02X %02X %02X %02X", 
		iSmartPacket.packet.data[4],
		iSmartPacket.packet.data[5],
		iSmartPacket.packet.data[6],
		iSmartPacket.packet.data[7],
		iSmartPacket.packet.data[8],
		iSmartPacket.packet.data[9]
		);
		
		DBG_APP("%04X", 
		iSmartPacket.packet.sequence
		);
				
		DBG_APP("%08X", 
		iSmartPacket.packet.crc
		);
		
//		uint8_t i;
//		for (i = 0;i < 6;i++)
//		{
//			len += sprintf(&buf[len],"%02X",iSmartPacket.packet.addr[i]);
//		}
//		len += sprintf(&buf[len],"   %02X   ",iSmartPacket.handle);
//		len += sprintf(&buf[len]," %02X ",iSmartPacket.packet.command);
//		len += sprintf(&buf[len]," %02X   ",iSmartPacket.packet.type);
//		for (i = 0;i < PROTOCOL_PAYLOAD_LEN;i++)
//		{
//			len += sprintf(&buf[len],"%02X",iSmartPacket.packet.data[i]);
//		}
//		len += sprintf(&buf[len],"  %08X ",iSmartPacket.packet.crc);
//		len += sprintf(&buf[len],"\r\n");
//		NRF_LOG_INFO("%s",buf);
//		for (i = 0;i < 6;i++)
//		{
//			DBG_APP("%02X",);
//		}
//		char dbgStr[128];
//		uint8_t num = 100;
//		sprintf(dbgStr,"%02X - %02X", num, iSmartPacket.packet.addr[1]);
		DBG_APP("------------------------------------------------------");
}



int8_t App_iSmartMeshPacketGet(uint8_t *packet,uint8_t *len)
{
	ISMART_MESH_PACKET_LIST *pkg;
	if(iSmartMeshPacketList[0] != NULL)
	{
		pkg = list_head(iSmartMeshPacketList);
		if(pkg != NULL)
		{
			list_pop(iSmartMeshPacketList);
			if(pkg->packet.len)
			{
				memcpy(packet,pkg->packet.data,pkg->packet.len);
				*len = pkg->packet.len;
				pkg->packet.len = 0;
				return ISMARTPACKET_OK;
			}
			return ISMARTPACKET_ERROR;
		}
	}
	return ISMARTPACKET_ERROR;
}

void App_iSmartMeshPacketAddToList(uint8_t *packet,uint8_t len)
{
//	DBG_APP("App_iSmartMeshPacketAddToList");
	uint8_t i;
	for(i = 0;i < ISMART_PACKET_BOX_SIZE;i++)
	{
		if(iSmartMeshPacketBox[i].packet.len == 0)
		{
			break;
		}
	}
	if(i >= ISMART_PACKET_BOX_SIZE)
		i = ISMART_PACKET_BOX_SIZE - 1;
	memcpy(iSmartMeshPacketBox[i].packet.data,packet,len);
	iSmartMeshPacketBox[i].packet.len = len;
	list_add(iSmartMeshPacketList, &iSmartMeshPacketBox[i]);
}



int8_t App_iSmartEventPacketGet(uint8_t *packet,uint8_t *cmd,uint8_t *type,uint8_t *len)
{
	
	ISMART_EVENT_PACKET_LIST *pkg;
	if(iSmartEventPacketList[0] != NULL)
	{
		pkg = list_head(iSmartEventPacketList);
		if(pkg != NULL)
		{
			list_pop(iSmartEventPacketList);
			if(pkg->packet.len)
			{
				memcpy(packet,pkg->packet.data,pkg->packet.len);
				*cmd = pkg->packet.cmd;
				*type = pkg->packet.type;
				*len = pkg->packet.len;
				pkg->packet.len = 0;
				return ISMARTPACKET_OK;
			}
			return ISMARTPACKET_ERROR;
		}
	}
	return ISMARTPACKET_ERROR;
}

void App_iSmartEventPacketAddToList(uint8_t *packet,uint8_t cmd,uint8_t type,uint8_t len)
{
	NRF_LOG_INFO("App_iSmartEventPacketAddToList");
	uint8_t i;
	for(i = 0;i < ISMART_PACKET_BOX_SIZE;i++)
	{
		if(iSmartEventPacketBox[i].packet.len == 0)
		{
			break;
		}
	}
	if(i >= ISMART_PACKET_BOX_SIZE)
		i = ISMART_PACKET_BOX_SIZE - 1;
	memcpy(iSmartEventPacketBox[i].packet.data,packet,len);
	iSmartEventPacketBox[i].packet.cmd = cmd;
	iSmartEventPacketBox[i].packet.type = type;
	iSmartEventPacketBox[i].packet.len = len;
	list_add(iSmartEventPacketList, &iSmartEventPacketBox[i]);
}





int8_t App_iSmartPacketGet(ISMARTPACKAGE *packet)
{
	ISMART_PACKET_BOX *pkg;
	if(iSmartPacketList[0] != NULL)
	{
		pkg = list_head(iSmartPacketList);
		if(pkg != NULL)
		{
			*packet = *pkg->packet;
			list_pop(iSmartPacketList);
			if(pkg->packet)
			{
				pkg->packet = NULL;
				return ISMARTPACKET_OK;
			}
			return ISMARTPACKET_ERROR;
		}
	}
	return ISMARTPACKET_ERROR;
}



void App_iSmartPacketAddToList(ISMARTPACKAGE *packet)
{
	uint8_t i;
	
	for(i = 0;i < ISMART_PACKET_BOX_SIZE;i++)
	{
		if(iSmartPacketBox[i].packet == NULL)
		{
			iSmartPacketBox[i].packet = &iSmartPacketBuf[i];
			break;
		}
	}
	if(i >= ISMART_PACKET_BOX_SIZE)
		i = ISMART_PACKET_BOX_SIZE - 1;
	*iSmartPacketBox[i].packet = *packet;
	list_add(iSmartPacketList, &iSmartPacketBox[i]);
	
}


int8_t App_iSmartAppPacketGet(ISMARTPACKAGE *packet)
{
	ISMART_PACKET_BOX *pkg;
	if(iSmartAppPacketList[0] != NULL)
	{
		pkg = list_head(iSmartAppPacketList);
		if(pkg != NULL)
		{
			*packet = *pkg->packet;
			list_pop(iSmartAppPacketList);
			if(pkg->packet)
			{
				pkg->packet = NULL;
				return ISMARTPACKET_OK;
			}
			return ISMARTPACKET_ERROR;
		}
	}
	return ISMARTPACKET_ERROR;
}



void App_iSmartAppPacketAddToList(ISMARTPACKAGE *packet)
{
	uint8_t i;
	for(i = 0;i < ISMART_PACKET_BOX_SIZE;i++)
	{
		if(iSmartAppPacketBox[i].packet == NULL)
		{
			iSmartAppPacketBox[i].packet = &iSmartAppPacketBuf[i];
			break;
		}
	}
	if(i >= ISMART_PACKET_BOX_SIZE)
		i = ISMART_PACKET_BOX_SIZE - 1;
	*iSmartAppPacketBox[i].packet = *packet;
	list_add(iSmartAppPacketList, &iSmartAppPacketBox[i]);
}



uint8_t aes_exp_key[250];
uint32_t AppMakeCrc(uint8_t *packet,uint8_t *customerKey,uint8_t *deviceId,uint32_t rtc)
{				
		rtc = 0x01234567;
		uint8_t buf[16],i,*pt,IV[16];
		uint32_t key[4];
		uint32_t crc;
	
//		memcpy(IV,(uint8_t *)&deviceId[0],6);
//		memcpy(&IV[6],&rtc,4);
//		memcpy(&IV[10],(uint8_t *)&deviceId[0],6);
	
		memcpy((uint8_t *)key,customerKey,16);
		memset(buf,0,16);
	
	for(uint8_t i = 0; i < 16; i++)
	{
		IV[i] = i;
	}
	
		for( i = 0; i < 16; i++ )
		{
				buf[i] = (unsigned char)( packet[i] ^ IV[i] );
		}
		
//	DBG_APP("key: \r\n");
//	for(uint8_t i = 0; i < 4; i++)
//	{
//		DBG_APP("0x%08x  ",key[i]);
//	}
//	DBG_APP("*************\r\n");
		
	
//	DBG_APP("IV:\r\n");
//	for(uint8_t i = 0; i < 16; i++)
//	{
//		DBG_APP("0x%02x  ",IV[i]);
//	}
//	DBG_APP("*************\r\n");
//		
		
		AES_keyschedule_enc((uint32_t *)&key,(uint32_t *)aes_exp_key);
		AES_encrypt((uint32_t *)buf,(uint32_t *)buf,(uint32_t *)aes_exp_key);
		pt = (uint8_t *)&crc;
		pt[0] = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
		pt[1] = buf[4] ^ buf[5] ^ buf[6] ^ buf[7]; 
		pt[2] = buf[8] ^ buf[9] ^ buf[10] ^ buf[11]; 
		pt[3] = buf[12] ^ buf[13] ^ buf[14] ^ buf[15]; 
		
//	DBG_APP("--------------\r\n");
//	for(uint8_t i = 0; i < 16; i++)
//	{
//		DBG_APP("0x%02x  ",buf[i]);
//	}
//	DBG_APP("--------------\r\n");
//	
	
//		DBG_APP(" CRC--------------\r\n");
//	for(uint8_t i = 0; i < 16; i++)
//	{
//		DBG_APP("0x%08x  ",crc);
//	}
//	DBG_APP("--------------\r\n");
//	
		return crc;
}

uint32_t AppMakeOtp(uint8_t *manufactoryKey,uint8_t *deviceId,uint32_t rtc)
{
		uint8_t *pt;
		uint8_t IV[16];
		uint32_t key[4];
		uint32_t crc;
		memcpy(IV,(uint8_t *)&deviceId[0],6);
		memcpy(&IV[6],&rtc,4);
		memcpy(&IV[10],(uint8_t *)&deviceId[0],6);
		memcpy((uint8_t *)key,manufactoryKey,16);
		AES_keyschedule_enc((uint32_t *)&key,(uint32_t *)aes_exp_key);
		AES_encrypt((uint32_t *)IV,(uint32_t *)IV,(uint32_t *)aes_exp_key);
		pt = (uint8_t *)&crc;
		pt[0] = IV[0] ^ IV[1] ^ IV[2] ^ IV[3];
		pt[1] = IV[4] ^ IV[5] ^ IV[6] ^ IV[7];
		pt[2] = IV[8] ^ IV[9] ^ IV[10] ^ IV[11];
		pt[3] = IV[12] ^ IV[13] ^ IV[14] ^ IV[15];
		return crc;
}



uint8_t U8CheckSum(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint8_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}



int8_t ascii2hex(uint8_t *chars,uint8_t *hex,uint8_t len)
{
  uint8_t i = 0,*pt;
  uint8_t c;
  pt = hex;
  for(i = 0;i < len;i++)
  {
    c = chars[i];
    if ((c <= '9') && (c >= '0'))
    {
      if(i & 1)
      {
        pt[i/2] += (c - 0x30);
      }   
      else
      {
        c = (c - 0x30);
        c <<= 4;
        pt[i/2] = c;
      }
    }
    else
    {
      // Invalid 
      return -1;
    }
  }
  return 1;
}

void protocol_send_data_to_gw(uint8_t *data, uint8_t len, uint8_t cmd, uint8_t type,bool is_need_ack) {
	sequence_increment();
	
	msg_temp_queue.u8_cmd = cmd;
	msg_temp_queue.u16_sequence = get_sequence();
	
	ISMARTPACKAGE _packet_temp;
	memset(&_packet_temp, 0, sizeof(ISMARTPACKAGE));
	
	_packet_temp.packet.sequence = get_sequence();
	/* Update user data of sending packet */
	if (len > 0) {
		if (len < PROTOCOL_PAYLOAD_LEN) {
			for(uint8_t i = 0; i < len; i++)
			{
				_packet_temp.packet.data[i] = data[i];
			}
		} else {
			for(uint8_t i = 0; i < PROTOCOL_PAYLOAD_LEN; i++)
			{
				_packet_temp.packet.data[i] = data[i];
			}
		}
	}
	/* Update address */
	memcpy(_packet_temp.packet.addr, _device_addr.addr, BLE_GAP_ADDR_LEN);

	/* Update other fields*/
	_packet_temp.packet.command = cmd;
	_packet_temp.packet.type = type;
	_packet_temp.handle = PROTOCOL_DATA_SEND_TO_MESH;

	uint32_t fakeRTC = 0x12345678;
//	_packet_temp.packet.crc = AppMakeCrc((uint8_t *)&_packet_temp.packet, sysCfg.deviceKey, _device_addr.addr, (fakeRTC) >> 4);
	
	_packet_temp.packet.crc = crc32_compute(&_packet_temp.packet.addr[0], 20, NULL);
	
		if(_packet_temp.packet.data[3] == 0x01 && _packet_temp.packet.data[1] == 0x01 && _packet_temp.packet.data[2] == 0x01)
			{
			}
		else if(_packet_temp.packet.data[3] == 0x02 && _packet_temp.packet.data[1] == 0x02 && _packet_temp.packet.data[2] == 0x02)
			{
			}
				else if(_packet_temp.packet.data[3] == 0x03 && _packet_temp.packet.data[1] == 0x03 && _packet_temp.packet.data[2] == 0x03)
			{
			}
			else if(_packet_temp.packet.data[3] == 0x04 && _packet_temp.packet.data[1] == 0x04 && _packet_temp.packet.data[2] == 0x04)
			{
			}
			else if(_packet_temp.packet.data[3] == 0x05 && _packet_temp.packet.data[1] == 0x05 && _packet_temp.packet.data[2] == 0x05)
			{
			}
			else
			{
				if(_packet_temp.packet.data[0] == 0x17 && _packet_temp.packet.data[3] == 0x00)
				{
					_packet_temp.packet.data[1] = 0x06;
					_packet_temp.packet.data[2] = 0x06;
					_packet_temp.packet.data[3] = 0x06;
				}
			}
	
	NRF_LOG_INFO("******************************************  CRC32: %08X" , _packet_temp.packet.crc);
	{
		AppShowiSmartPacket(_packet_temp);
		App_iSmartAppPacketAddToList(&_packet_temp);
		App_iSmartMeshPacketAddToList((uint8_t *)&_packet_temp, sizeof(ISMARTPACKAGE));
	}
}

void protocol_send_data_to_dev(uint8_t* p_addr, uint8_t *data, uint8_t len, uint8_t cmd, uint8_t type, bool is_need_ack) {
	ISMARTPACKAGE _packet_temp;
	memset(&_packet_temp, 0, sizeof(ISMARTPACKAGE));

	/* Update user data of sending packet */
	if (len > 0) {
		if (len < PROTOCOL_PAYLOAD_LEN) {
			memcpy(_packet_temp.packet.data, data, len);
		} else {
			memcpy(_packet_temp.packet.data, data, PROTOCOL_PAYLOAD_LEN);
		}
	}

	/* Update address */
	memcpy(_packet_temp.packet.addr, p_addr, BLE_GAP_ADDR_LEN);

	/* Update other fields*/
	_packet_temp.packet.command = cmd | (is_need_ack ? PROTOCOL_PACKET_ACK_MASK : 0);
	_packet_temp.packet.type = type;
	_packet_temp.handle = PROTOCOL_DATA_SEND_TO_MESH;
	_packet_temp.packet.crc = AppMakeCrc((uint8_t *)&_packet_temp.packet, sysCfg.deviceKey, _packet_temp.packet.addr, (rtcTimeSec) >> 4);
	App_iSmartMeshPacketAddToList((uint8_t *)&_packet_temp, sizeof(ISMARTPACKAGE));
	App_iSmartAppPacketAddToList(&_packet_temp);
}

void protocol_generate_packet(ISMARTPACKAGE* p_packet, uint8_t* p_addr, uint8_t *data, uint8_t len, uint8_t cmd, uint8_t type, bool is_need_ack) {
	memset(p_packet, 0, sizeof(ISMARTPACKAGE));

	/* Update address */
	if (p_addr) {
		memcpy(p_packet->packet.addr, p_addr, BLE_GAP_ADDR_LEN);
	} else {
		memcpy(p_packet->packet.addr, _device_addr.addr, BLE_GAP_ADDR_LEN);
	}
	/* Update user data of sending packet */
	if (len > 0) {
		if (len < PROTOCOL_PAYLOAD_LEN) {
			memcpy(p_packet->packet.data, data, len);
		} else {
			memcpy(p_packet->packet.data, data, PROTOCOL_PAYLOAD_LEN);
		}
	} else {
		return;
	}
	/* Update other fields*/
	p_packet->packet.command = cmd | (is_need_ack ? PROTOCOL_PACKET_ACK_MASK : 0);
	p_packet->packet.type = type;
	p_packet->handle = PROTOCOL_DATA_SEND_TO_MESH;
	p_packet->packet.crc = 0;
}

void protocol_send_packet_to_dev(ISMARTPACKAGE* p_packet, uint32_t device_time) {
	if (!device_time) {
		device_time = rtcTimeSec;
	}
	p_packet->packet.crc = AppMakeCrc((uint8_t *)&p_packet->packet, sysCfg.deviceKey, p_packet->packet.addr, (device_time) >> 4);
	App_iSmartAppPacketAddToList(p_packet);
	App_iSmartMeshPacketAddToList((uint8_t *)p_packet, sizeof(ISMARTPACKAGE));
}

void protocol_send_presentation(uint8_t sensor_id, protocol_device_type_e dev_type) {
	protocol_send_data_to_gw(&sensor_id, 1, ePROTOCOL_CMD_TYPE_PRESENTATION, dev_type,false);
}


void protocol_configure_time(uint8_t* target_addr, uint32_t target_time) {
	ISMARTPACKAGE packet;
	uint8_t buff[PROTOCOL_PAYLOAD_LEN];
	time_get(buff);

	protocol_generate_packet(&packet, target_addr, buff, TIME_ARRAY_SIZE, ePROTOCOL_CMD_TYPE_INTERNAL, ePROTOCOL_INTERAL_TYPE_TIME, false);
	packet.packet.crc = AppMakeCrc((uint8_t *)&packet, sysCfg.deviceKey, target_addr, (target_time) >> 4);
	App_iSmartMeshPacketAddToList((uint8_t *)&packet, sizeof(ISMARTPACKAGE));
}

void set_sequence_increment(uint16_t val)
{
	u16_sequence = val;
	DBG_APP("set_sequence_increment %04X..................... ", val);
}

void sequence_increment()
{
	u16_sequence++;
	DBG_APP("sequence_increment %04X..................... ", u16_sequence);
	NRF_LOG_INFO("sequence_increment %04X..................... ", u16_sequence);
}

uint16_t get_sequence()
{
	return u16_sequence;
}
