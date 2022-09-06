
#include <string.h>
#include <ctype.h>
#include "system_config.h"
#include "lib/sparser.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "lib/ampm_list.h"
#include "app_settings.h"
#include "app_ble.h"
#include "nrf_delay.h"

#include "IR_DeviceConstructor.h"
#include "ac_type_detection_task.h"
#include "app_indicator_led.h"
#define INFO(...)	NRF_LOG_INFO(__VA_ARGS__)

typedef enum {
	IDLE,
	LOGGED_IN
}CMD_STATE_TYPE;

uint8_t u8FwType = FW_APP_TYPE_CAMPFIRE;

CMD_STATE_TYPE appSettingPhase;
extern uint32_t wh_cnt;
extern uint32_t bootloader_start(void);
const char *appSetttingOk = "OK";
const char *appSetttingError = "ERROR";
extern ble_gap_addr_t _device_addr;
void AppSettingsTask(void)
{
	
	char *pt;
	uint32_t u32T0,u32T1,u32T2,u32T3;
	char buff[256],tempBuff0[32];
	if(!ble_is_connected())
	{
		appSettingPhase = IDLE;
	}
	appSettingPhase = LOGGED_IN;
	switch(appSettingPhase)
	{
		case IDLE:
			if(ble_get_data(buff))
			{
			}
		break;
		case LOGGED_IN:
			if(ble_get_data(buff))
			{

				pt = strstr(buff,"FW_VERSION?");
				if(pt != NULL)
				{
					sprintf(tempBuff0,"FW_VERSION=%s",VERSION);
					ble_send_data((uint8_t *)tempBuff0,strlen((char *)tempBuff0));
				}
				
				pt = strstr(buff,"TYPE=");
				if(pt != NULL)
				{
					sscanf(pt,"TYPE=%d",tempBuff0);
					sysCfg.IR_devicetype = tempBuff0[0];
					CFG_Save();
					ble_send_data((uint8_t *)appSetttingOk,strlen(appSetttingOk));
					IR_ConstructSpecificDeviceType(sysCfg.IR_devicetype);
				}
				pt = strstr(buff,"TYPE?");
				if(pt != NULL)
				{
					sprintf(tempBuff0,"TYPE=%u",sysCfg.IR_devicetype);
					ble_send_data((uint8_t *)tempBuff0,strlen((char *)tempBuff0));
				}
				pt = strstr(buff,"MAC?");
				if(pt != NULL)
				{
					sprintf(tempBuff0,"MAC=%02X%02X%02X%02X%02X%02X",_device_addr.addr[0], _device_addr.addr[1], _device_addr.addr[2], _device_addr.addr[3], _device_addr.addr[4], _device_addr.addr[5]);
					ble_send_data((uint8_t *)tempBuff0,strlen((char *)tempBuff0));
				}
				pt = strstr(buff,"IRLEARNINGMODE");
				if(pt != NULL)
				{
					u8FwType = FW_APP_TYPE_IR_LEARNING;
					sprintf(tempBuff0,"OK");
					ble_send_data((uint8_t *)tempBuff0,strlen((char *)tempBuff0));
				}
				pt = strstr(buff,"PASS=");
				if(pt != NULL)
				{
					sscanf(pt,"PASS=%s",tempBuff0);
					strncpy((char *)sysCfg.deviceKey,tempBuff0, 16);
					CFG_Save();
					ble_send_data((uint8_t *)appSetttingOk,strlen(appSetttingOk));
				}
				
				pt = strstr(buff,"ENABLE_AC_DETECTION=");
				if(pt != NULL)
				{
					NRF_LOG_INFO("*********** ENABLE_AC_DETECTION **********");
					sscanf(pt,"ENABLE_AC_DETECTION=%d",tempBuff0);
//					strcpy((char *)sysCfg.deviceKey,tempBuff0);
					ac_type_detection_task_trigger(tempBuff0[0]);
					CFG_Save();
					ble_send_data((uint8_t *)appSetttingOk,strlen(appSetttingOk));
				}
				
				pt = strstr(buff,"SET_ENERGY=");
				if(pt != NULL)
				{
					NRF_LOG_INFO("*********** SET ENERGY **********");
					uint32_t wh;
					sscanf(pt,"SET_ENERGY=%d",&wh);
					wh_cnt = wh;
					ble_send_data((uint8_t *)appSetttingOk,strlen(appSetttingOk));
				}
				pt = strstr(buff,"PASS?");
				if(pt != NULL)
				{
					sprintf(tempBuff0,"PASS=%s",sysCfg.deviceKey);
					ble_send_data((uint8_t *)tempBuff0,strlen((char *)tempBuff0));
				}
				
				pt = strstr(buff,"ADDR=");
				if(pt != NULL)
				{
					sscanf(pt,"ADDR=%d.%d.%d.%d",&u32T0,&u32T1,&u32T2,&u32T3);
					pt = (char *)&sysCfg.radio_access_addr;
					pt[3] = u32T0;
					pt[2] = u32T1;
					pt[1] = u32T2;
					pt[0] = u32T3;
					CFG_Save();
					ble_send_data((uint8_t *)appSetttingOk,strlen(appSetttingOk));
				}
				pt = strstr(buff,"ADDR?");
				if(pt != NULL)
				{
					pt = (char *)&sysCfg.radio_access_addr;
					sprintf(tempBuff0,"ADDR=%d.%d.%d.%d",pt[3],pt[2],pt[1],pt[0]);
					ble_send_data((uint8_t *)tempBuff0,strlen((char *)tempBuff0));
				}
				pt = strstr(buff,"BOOTMODE");
				if(pt != NULL)
				{
					sysCfg.ota_flag = 1;
					CFG_Save();
					nrf_delay_ms(200);
					force_device_disconnect();
					NRF_LOG_INFO("*********** BOOTLOADER **********");
					app_indicator_led_on(RGB_TYPE_R);
					nrf_delay_ms(100);
					app_indicator_led_off(RGB_TYPE_B);
					nrf_delay_ms(100);
					app_indicator_led_on(RGB_TYPE_R);
					nrf_delay_ms(100);
					app_indicator_led_off(RGB_TYPE_B);
					nrf_delay_ms(100);
					app_indicator_led_on(RGB_TYPE_R);
					nrf_delay_ms(100);
					app_indicator_led_off(RGB_TYPE_B);
					nrf_delay_ms(100);
					app_indicator_led_on(RGB_TYPE_R);
					nrf_delay_ms(100);
					app_indicator_led_off(RGB_TYPE_B);
					nrf_delay_ms(100);
					app_indicator_led_on(RGB_TYPE_R);
					nrf_delay_ms(100);
					app_indicator_led_off(RGB_TYPE_B);
					
					bootloader_start();
				}
				
				pt = strstr(buff,"FORMAT");
				if(pt != NULL)
				{
					CFG_Load(true);
					ble_send_data((uint8_t *)appSetttingOk,strlen(appSetttingOk));
				}
				
				pt = strstr(buff,"REBOOT");
				if(pt != NULL)
				{
					NVIC_SystemReset();
				}
			}
		break;
	}
}

