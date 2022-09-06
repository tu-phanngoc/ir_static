/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#include "system_config.h"
#include "nrf_use_flash.h"
#include "nrf_log.h"
#include "crc32.h"
#include <string.h>
#include "app_scheduler.h"
#include "nrf_delay.h"
#include "lib/sys_tick.h"

#define SYSTEM_CFG_DEBUG_ENABLE

#ifdef SYSTEM_CFG_DEBUG_ENABLE
#define SYSTEM_CFG_DEBUG				NRF_LOG_INFO
#else
#define SYSTEM_CFG_DEBUG(...)
#endif

extern void datalog_clear();

uint8_t  app_settings_buffer[CODE_PAGE_SIZE] __attribute__((at(APP_SYSTEM_CONFIG_ADDRESS)))
                                                   __attribute__((used));																				 
static bool flash_operation_pending; // barrier for reading flash
CONFIG_POOL sysCfg; 	
uint8_t sysResetMcuFlag = MCU_RESET_NONE;

uint8_t flagSystemStatusUpdate = 0;
extern uint32_t DB_SaveAll(void);
uint8_t tMcuResetTimeout;

volatile tCONFIG_FLASH_DATA  ramcfg;

bool gb_save_config_enabled = false;
uint32_t gu32_save_cfg_timeout = CFG_SAVING_TRIGGER_TIMEOUT;

void ResetMcuSet(uint8_t resetType)
{
	sysResetMcuFlag = resetType;
}

void ResetMcuTask(void)
{
		switch(sysResetMcuFlag)
		{
			case MCU_RESET_NONE:

			break;
			case MCU_RESET_IMMEDIATELY:
				NVIC_SystemReset();
			break;
			case MCU_RESET_AFTER_10_SEC:
				tMcuResetTimeout = 10;
				sysResetMcuFlag = MCU_RESET_IS_WAITING;
			break; 
			case MCU_RESET_AFTER_30_SEC:
				tMcuResetTimeout = 30;
				sysResetMcuFlag = MCU_RESET_IS_WAITING;
			break;
			case MCU_RESET_IS_WAITING:
				if(tMcuResetTimeout-- == 0)
				{
					NVIC_SystemReset();
				}
			break;
			default:
				NVIC_SystemReset();
				break;
		}
}

static void delay_operation(void)
{
   nrf_delay_ms(1);
   app_sched_execute();
}


static void wait_for_pending(void)
{
    while (flash_operation_pending == true)
    {
        SYSTEM_CFG_DEBUG("Waiting for other flash operation to finish.\r\n");
        delay_operation();
    }
}

uint32_t app_flash_program_calculate_crc(void)
{
    // the crc is calculated from the sysCfg struct, except the crc itself and the init command
    return crc32_compute((uint8_t*)&sysCfg, sizeof(CONFIG_POOL) - 4, NULL);
}

uint32_t CFG_Init(void)
{
	return nrf_use_flash_init();
}	

ret_code_t CFG_Save(void)
{
	CONFIG_POOL tempSysCfg;
	memcpy((void*)&tempSysCfg, &app_settings_buffer[0], sizeof(CONFIG_POOL));
	
	if(memcmp(app_settings_buffer, &sysCfg, sizeof(CONFIG_POOL)) == 0)
	{
		NRF_LOG_INFO("SYS CONFIG NOT CHANGE ....................");
		return NRF_SUCCESS;
	}
	
	ret_code_t err_code = NRF_SUCCESS;
    SYSTEM_CFG_DEBUG("Erasing old settings at: 0x%08x\r\n", (uint32_t)&app_settings_buffer[0]);

    // Wait for any ongoing operation (because of multiple calls to app_flash_program_write)
    wait_for_pending();

    flash_operation_pending = true;

	// erase flash
	err_code = nrf_use_flash_erase((uint32_t)&app_settings_buffer[0], 1, &flash_operation_pending);

    if (err_code != NRF_SUCCESS)
    {
        SYSTEM_CFG_DEBUG("Erasing from flash memory failed.\r\n");
        flash_operation_pending = false;
        return NRF_ERROR_INTERNAL;
    }

    sysCfg.crc = app_flash_program_calculate_crc();

    SYSTEM_CFG_DEBUG("Writing 0x%08x words\r\n", sizeof(CONFIG_POOL)/4);

	err_code = nrf_use_flash_store((uint32_t)&app_settings_buffer[0],
								   (uint32_t*)&sysCfg,
								   sizeof(CONFIG_POOL) + (4 -  sizeof(CONFIG_POOL)%4),
								   &flash_operation_pending);

    if (err_code != NRF_SUCCESS)
    {
        SYSTEM_CFG_DEBUG("Storing to flash memory failed.\r\n");
        flash_operation_pending = false;
        return NRF_ERROR_INTERNAL;
    }

    SYSTEM_CFG_DEBUG("Writing settings...\r\n");
    return NRF_SUCCESS;
}


void CFG_Load(bool reset)
{
	uint32_t saveFlag = 0,i;
	
	SYSTEM_CFG_DEBUG("Start Load Old Info");
	flash_operation_pending = false;
	// Copy the DFU settings out of flash and into a buffer in RAM.
	memcpy((void*)&sysCfg, &app_settings_buffer[0], sizeof(CONFIG_POOL));

//	if((sysCfg.crc == 0xFFFFFFFF) || (sysCfg.crc != app_flash_program_calculate_crc()) || reset)
//	{
//		// Reached if nothing is configured or if CRC was wrong
//		NRF_LOG_DEBUG("!!!!!!!!!!!!!!! Resetting bootloader settings !!!!!!!!!!!\r\n");
//		memset((void*)&sysCfg, 0xFF, sizeof sysCfg);
//		saveFlag = 1;
//	}

	
	if(sysCfg.size != sizeof(CONFIG_POOL))
	{
		sysCfg.size = sizeof(CONFIG_POOL);
		saveFlag = 1;
	}

	
	if((sysCfg.fwVersion[0] == 0xFF) || (memcmp((char *)sysCfg.fwVersion,VERSION,sizeof(VERSION))))
	{
		memcpy((char *)sysCfg.fwVersion,VERSION,sizeof(VERSION));
		saveFlag = 1;
	}
	
	if(sysCfg.radio_access_addr == 0xffffffff)
	{
		sysCfg.radio_access_addr = DEFAULT_RADIO_ACCESS_ADDR;
		saveFlag = 1;
	}
	
	if(sysCfg.radio_channel < 37 || sysCfg.radio_channel > 39)
	{
		sysCfg.radio_channel = DEFAULT_RADIO_CHANNEL;
		saveFlag = 1;
	}
	if(sysCfg.feature == 0xff)
	{
		sysCfg.feature = 0;
		saveFlag = 1;
	}
	if(sysCfg.deviceKey[0] == 0xff)
	{
		memset(sysCfg.deviceKey,0,sizeof(sysCfg.deviceKey));
		memcpy((char *)sysCfg.deviceKey,DEFAULT_CUSTOMER_ID,sizeof(DEFAULT_CUSTOMER_ID));
		saveFlag = 1;
	}
	
	if(sysCfg.ac_payload[0] == 0xFF)
	{
		sysCfg.ac_payload[0] = 0;
		sysCfg.ac_payload[1] = 0;
		sysCfg.ac_payload[2] = 0;
		sysCfg.ac_payload[3] = 0;
		sysCfg.ac_payload[4] = 0;
		sysCfg.ac_payload[5] = 0;
	}
	
	
//	if(sysCfg.gatewayAddr[0] == 0xff)
//	{
//		
//	}
//	
//	if(saveFlag)
//	{
//		NRF_LOG_INFO("Start Save New Info");
//		APP_ERROR_CHECK(CFG_Save());
//	}
}

void CFG_Saving_Tick(void)
{
	if(gu32_save_cfg_timeout > 0)
	{
		gu32_save_cfg_timeout--;
	}
}

void CFG_Saving_Trigger(void)
{
	gb_save_config_enabled = true;
	gu32_save_cfg_timeout = CFG_SAVING_TRIGGER_TIMEOUT;
}

void CFG_Saving_Task(void)
{
	if(gb_save_config_enabled && gu32_save_cfg_timeout == 0)
	{
		NRF_LOG_INFO("CFG_Saving_Task");
		gb_save_config_enabled = false;
		CFG_Save();
	}
}


