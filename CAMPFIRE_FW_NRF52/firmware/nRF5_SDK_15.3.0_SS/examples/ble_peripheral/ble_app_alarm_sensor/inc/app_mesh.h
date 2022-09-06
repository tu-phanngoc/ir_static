#ifndef __APP_MESH__
#define __APP_MESH__
#include <stdint.h>
#include "app_message_queue.h"


#define  SENSOR_MODE	0x55555555
#define  MESH_MODE		0xAAAAAAAA
#define  MESH_MODE_TIMEOUT		60 //60s

extern bool sensor_data_sending;
extern uint8_t sysModeTimeout;


extern const uint8_t broadcast_addr[6];
uint8_t App_ProcessPacket(ISMARTPACKAGE *iSmartPacket);
void app_mesh_task(void);
void mesh_app_init(void);
void cmd_process_internal_command(ISMARTPACKAGE *p_packet);
uint32_t app_get_mode(void);
void app_tick_task(void);
void app_set_mode(uint32_t mode);

void enable_mesh_task(void);
void disable_mesh_task(void);
bool is_mesh_task_enable(void);
	
extern bool bSendingToMesh;
extern bool bEnableSendingToMesh;
#endif
