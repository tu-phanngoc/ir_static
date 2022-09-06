
#ifndef __APP_MESSAGE_QUEUE_H__
#define __APP_MESSAGE_QUEUE_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "lib/sys_tick.h"
#include "lib/ringbuf.h"
#include "lib/protocol.h"
#include "system_config.h"

#define TIME_ARRAY_SIZE	(7)

#define SENSOR_TIMEOUT_EVENT 0x01
#define SENSOR_CHANGED_EVENT 0x02
#define SENSOR_RECV_TIMEOUT_EVENT 0x04
#define SENSOR_SOS_EVENT 0x08
#define SENSOR_BAT_EVENT 0x10

#define ISMARTPACKET_OK	0
#define ISMARTPACKET_ERROR	-1
#define ISMARTPACKET_MANUFACTORY_KEY_BIT_MASK	(0x40)

#define PROTOCOL_PAYLOAD_LEN 10
#define PROTOCOL_PACKET_SEND_TO_APP_MASK (0x80)
#define PROTOCOL_PACKET_ACK_MASK 0x08
#define PROTOCOL_PACKET_CMD_MASK (0x07)

#define PROTOCOL_DATA_SEND_TO_MESH 0xFF

#define MESH_MSG_ID_REALTIME_IR									0xCD
#define MESH_MSG_ID_REALTIME_SENSOR							0x01
#define MESH_MSG_ID_REALTIME_PWR								0x02	
#define MESH_MSG_ID_REALTIME_SCHEDULE_EVENT			0xCA
#define MESH_MSG_ID_REALTIME_TIMER_EVENT				0xCB

#define MESH_MSG_ID_HISTORY_IR									0xDD
#define MESH_MSG_ID_HISTORY_SENSOR							0xD1
#define MESH_MSG_ID_HISTORY_PWR									0xD2	
#define MESH_MSG_ID_HISTORY_SCHEDULE_EVENT			0xDA
#define MESH_MSG_ID_HISTORY_TIMER_EVENT					0xDB

typedef enum _protocol_cmd_type_e {
	ePROTOCOL_CMD_TYPE_PRESENTATION = 0x00,
	ePROTOCOL_CMD_TYPE_SET = 0x01,
	ePROTOCOL_CMD_TYPE_REQ = 0x02,
	ePROTOCOL_CMD_TYPE_INTERNAL = 0x03,
	ePROTOCOL_CMD_TYPE_STREAM = 0x04,
	ePROTOCOL_CMD_TYPE_CFG = 0x05,
	ePROTOCOL_CMD_TYPE_SOS = 0x06,
	ePROTOCOL_CMD_TYPE_STATUS = 0x07,
	ePROTOCOL_CMD_ALL = 0x0F
} protocol_cmd_type_e;



typedef enum _protocol_device_type_e {
	ePROTOCOL_DEVICE_TYPE_DOOR = 0x00,
	ePROTOCOL_DEVICE_TYPE_MOTION = 0x01,
	ePROTOCOL_DEVICE_TYPE_BINARY = 0x03,
	ePROTOCOL_DEVICE_TYPE_DIMMER = 0x04,
	ePROTOCOL_DEVICE_TYPE_TEMP   = 0x06,
	ePROTOCOL_DEVICE_TYPE_HUMID  = 0x07,
	ePROTOCOL_DEVICE_TYPE_BARO   = 0x08,
	ePROTOCOL_DEVICE_TYPE_SOS		= 0x09
} protocol_device_type_e;

typedef enum _protocol_value_type_e {
	ePROTOCOL_VALUE_TYPE_TEMP       = 0x00,
	ePROTOCOL_VALUE_TYPE_HUMID      = 0x01,
	ePROTOCOL_VALUE_TYPE_STATUS     = 0x02,
	ePROTOCOL_VALUE_TYPE_PERCENTAGE = 0x03,
	ePROTOCOL_VALUE_TYPE_PRESSURE   = 0x04,
} protocol_value_type_e;

typedef enum _protocol_internal_type_e {
	ePROTOCOL_INTERAL_TYPE_TIME = 0x01,
	ePROTOCOL_INTERAL_TYPE_SET = 0x02,
	ePROTOCOL_INTERAL_TYPE_ERROR = 29,
	ePROTOCOL_INTERAL_TYPE_CONTEXT = 30,
} protocol_internal_type_e;


typedef enum smarthome_context_type_e {
	SMARTHOME_CONTEXT_IR = 0x01,
} smarthome_context_type_e;

typedef enum _protocol_error_code_e {
	ePROTOCOL_ERROR_CODE_NONE = 0,
	ePROTOCOL_ERROR_CODE_CRC_ERROR = 0x01,
	ePROTOCOL_ERROR_CODE_CMD_ERROR = 0x02,
	ePROTOCOL_ERROR_CODE_SENSOR_ID_NON_EXIST = 0x03,
} protocol_error_code_e;

typedef struct __attribute__((packed)){
	uint8_t data[28];
	uint8_t len;
}ISMART_MESH_PACKET;

typedef struct {
	struct ISMART_MESH_PACKET_LIST *next;
	ISMART_MESH_PACKET packet;
} ISMART_MESH_PACKET_LIST;


typedef struct __attribute__((packed)){
	uint8_t data[PROTOCOL_PAYLOAD_LEN];
	uint8_t cmd;
	uint8_t type;
	uint8_t len;
}ISMART_EVENT_PACKET;

typedef struct {
	struct ISMART_MESH_PACKET_LIST *next;
	ISMART_EVENT_PACKET packet;
} ISMART_EVENT_PACKET_LIST;


typedef struct __attribute__((packed)) {
	uint8_t addr[6];
	uint8_t command;
	uint8_t type;
	uint8_t data[PROTOCOL_PAYLOAD_LEN];
	uint16_t sequence;
	uint32_t crc;
} ISMARTPACKAGE_TYPE_BLE;

typedef struct __attribute__((packed)) {
	uint8_t handle; //not use to make crc
	ISMARTPACKAGE_TYPE_BLE packet;
} ISMARTPACKAGE;

typedef struct {
	struct ISMART_PACKET_BOX *next;
	ISMARTPACKAGE *packet;
} ISMART_PACKET_BOX;

typedef enum _msg_stt_e {
	MSG_STANDBY = 0x01,
	MSG_SENT = 0x02,
	MSG_WAIT = 0x03,
	MSG_OK = 0x04,
	MSG_FAIL = 0x05,
} msg_status_e;

typedef struct transmit_queue
{
	uint8_t u8_cmd;
	uint16_t u16_sequence;
	uint8_t u8_wait_time;
	msg_status_e msg_status;
}transmit_queue_t;


extern transmit_queue_t msg_temp_queue;
extern uint8_t sensor_event_flag;

/* Public functions */
void AppConfigTaskInit(void);
void App_iSmartPacketAddToList(ISMARTPACKAGE *packet);
int8_t App_iSmartPacketGet(ISMARTPACKAGE *packet);
void App_iSmartAppPacketAddToList(ISMARTPACKAGE *packet);
int8_t App_iSmartAppPacketGet(ISMARTPACKAGE *packet);

void AppConfigTask(void);
uint8_t U8CheckSum(uint8_t *buff, uint32_t length);
uint32_t AppMakeCrc(uint8_t *packet,uint8_t *customerKey,uint8_t *deviceId,uint32_t rtc);
extern void AppConfigCallback(PROTO_PARSER *parser);
int8_t App_iSmartMeshPacketGet(uint8_t *packet,uint8_t *len);
void App_iSmartMeshPacketAddToList(uint8_t *packet,uint8_t len);
int8_t App_iSmartEventPacketGet(uint8_t *packet,uint8_t *cmd,uint8_t *type,uint8_t *len);
void App_iSmartEventPacketAddToList(uint8_t *packet,uint8_t cmd,uint8_t type,uint8_t len);

uint32_t AppMakeOtp(uint8_t *manufactoryKey,uint8_t *deviceId,uint32_t rtc);
void protocol_send_data_to_gw(uint8_t *data, uint8_t len, uint8_t cmd, uint8_t type,bool is_need_ack);
void protocol_send_presentation(uint8_t sensor_id, protocol_device_type_e dev_type);

void protocol_send_data_to_dev(uint8_t* addr, uint8_t *data, uint8_t len, uint8_t cmd, uint8_t type, bool is_need_ack);

void protocol_send_packet_to_dev(ISMARTPACKAGE* p_packet, uint32_t device_time);

void protocol_generate_packet(ISMARTPACKAGE* p_packet, uint8_t* p_addr, uint8_t *data, uint8_t len, uint8_t cmd, uint8_t type, bool is_need_ack);
void AppMessageQueueInit(void);
void protocol_configure_time(uint8_t* target_addr, uint32_t target_time);
void AppShowiSmartPacket(ISMARTPACKAGE iSmartPacket);
void protocol_send_sos(bool status);
void protocol_send_bat(bool status);
void sequence_increment();
uint16_t get_sequence();
void set_sequence_increment(uint16_t val);

void protocol_send_IR_data(uint8_t* data);
void protocol_send_schedule_event_data(uint8_t* data);
void protocol_send_timer_event_data(uint8_t* data);

#endif
