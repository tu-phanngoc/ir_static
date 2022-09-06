#include "app_datalog.h"

#include "system_config.h"
#include "nrf_nvmc.h"
#include "nrf_log.h"

#include "nrf_fstorage.h"
#include "dataLogger_Interface.h"
#include "data_transmit_fsm.h"
#include "nrf_delay.h"
#include "app_scheduler.h"

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_fstorage_sd.h"
#else
#include "nrf_drv_clock.h"
#include "nrf_fstorage_nvmc.h"
#endif

#include "app_task_control.h"

#define DATALOG_DEBUG_ENABLE
#ifdef DATALOG_DEBUG_ENABLE		
#define DATALOG_DEBUG			NRF_LOG_INFO
#else
#define DATALOG_DEBUG
#endif

bool is_app_alive = false;
DB_t dbData;
static bool flash_operation_pending;
// Function prototypes
static void datalog_fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);

DB_t saveDB;

uint32_t taskCount = 0;
uint32_t gu32DatalogCount = 0;
extern ble_gap_addr_t _device_addr;

bool is_datalog_save_enabled = false;
extern bool gb_is_gateway_alive;
extern uint32_t msg_from_fw_tick;

bool app_meter_trigger_saving_energy;
bool trigger_saving_rtc;
extern uint32_t wh_cnt;
uint32_t gu32_datalog_tick_ms = 0;
uint32_t gu32_datalog_tick_s = 0;
extern const dataLoggerConfig_t dataLoggerConfigTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG];

uint8_t datalog_enabled = 0;
uint16_t datalog_number = 0;
NRF_FSTORAGE_DEF(nrf_fstorage_t datalog_storage_config) =
{
    /* Set a handler for fstorage events. */
    .evt_handler = datalog_fstorage_evt_handler,

    /* These below are the boundaries of the flash space assigned to this instance of fstorage.
     * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
     * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
     * last page of flash available to write data. */
    .start_addr = DATALOG_STORAGE_ADDR_START,
    .end_addr   = DATALOG_STORAGE_ADDR_START + CODE_PAGE_SIZE*31 - 1, //CODE_PAGE_SIZE
};


static void datalog_fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        DATALOG_DEBUG("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            DATALOG_DEBUG("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            DATALOG_DEBUG("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
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
        DATALOG_DEBUG("Waiting for other flash operation to finish.\r\n");
        delay_operation();
    }
}

static void datalog_print_flash_info(nrf_fstorage_t * p_fstorage)
{
    DATALOG_DEBUG("========| datalog_flash info |========");
    DATALOG_DEBUG("erase unit: \t%d bytes",      p_fstorage->p_flash_info->erase_unit);
    DATALOG_DEBUG("program unit: \t%d bytes",    p_fstorage->p_flash_info->program_unit);
    DATALOG_DEBUG("==============================");
}

void datalog_wait_for_flash_ready(nrf_fstorage_t const * p_fstorage)
{
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(p_fstorage))
    {
        
    }
}

/**@brief   Helper function to obtain the last address on the last page of the on-chip flash that
 *          can be used to write user data.
 */
static uint32_t datalog_nrf5_flash_end_addr_get()
{
    uint32_t const bootloader_addr = NRF_UICR->NRFFW[0];
    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
    uint32_t const code_sz         = NRF_FICR->CODESIZE;

    return (bootloader_addr != 0xFFFFFFFF ?
            bootloader_addr : (code_sz * page_sz));
}

uint32_t datalog_nrf_use_flash_init(void)
{
  nrf_fstorage_api_t * p_fs_api;
	ret_code_t rc;
#ifdef SOFTDEVICE_PRESENT
    DATALOG_DEBUG("SoftDevice is present.");
    DATALOG_DEBUG("Initializing nrf_fstorage_sd implementation...");
    /* Initialize an fstorage instance using the nrf_fstorage_sd backend.
     * nrf_fstorage_sd uses the SoftDevice to write to flash. This implementation can safely be
     * used whenever there is a SoftDevice, regardless of its status (enabled/disabled). */
    p_fs_api = &nrf_fstorage_sd;
#else
    NRF_LOG_INFO("SoftDevice not present.");
    NRF_LOG_INFO("Initializing nrf_fstorage_nvmc implementation...");
    /* Initialize an fstorage instance using the nrf_fstorage_nvmc backend.
     * nrf_fstorage_nvmc uses the NVMC peripheral. This implementation can be used when the
     * SoftDevice is disabled or not present.
     *
     * Using this implementation when the SoftDevice is enabled results in a hardfault. */
    p_fs_api = &nrf_fstorage_nvmc;
#endif
    rc = nrf_fstorage_init(&datalog_storage_config, p_fs_api, NULL);
    APP_ERROR_CHECK(rc);

    datalog_print_flash_info(&datalog_storage_config);

    /* It is possible to set the start and end addresses of an fstorage instance at runtime.
     * They can be set multiple times, should it be needed. The helper function below can
     * be used to determine the last address on the last page of flash memory available to
     * store data. */	
}


/*	p_dest   : destination address
 *	p_src    : pointer to data src
 *  len_byte : length of data in byte
 */

uint32_t datalog_nrf_use_flash_write(uint32_t p_dest, uint32_t const *p_src, uint32_t len_bytes, bool *flash_operation_pending)
{
	ret_code_t rc;
//	NRF_LOG_INFO("write p_dest: %08X, len: %d, datalog_storage_config->start_addr: %08X, datalog_storage_config->start_end: %08X",p_dest, len_bytes, datalog_storage_config.start_addr, datalog_storage_config.end_addr);
	
	// erase flash
	if(p_dest % (1024 * 4) == 0)
	{
		rc = datalog_nrf_use_flash_erase(p_dest, 1, flash_operation_pending);

    if (rc != NRF_SUCCESS)
    {
        DATALOG_DEBUG("Erasing from flash memory failed.\r\n");
        flash_operation_pending = false;
        return NRF_ERROR_INTERNAL;
    }
	}
	
	*flash_operation_pending = true;
    rc = nrf_fstorage_write(&datalog_storage_config, p_dest, p_src, len_bytes, NULL);
    if(rc == NRF_SUCCESS)
	{
		datalog_wait_for_flash_ready(&datalog_storage_config);
	}
	else
	{
		DATALOG_DEBUG("error %d",rc);
	}
	*flash_operation_pending = false;
	return rc;
}

uint32_t datalog_nrf_use_flash_read(uint32_t src, uint32_t const *p_dest, uint32_t len_bytes, bool *flash_operation_pending)
{		 						
//	NRF_LOG_INFO("read: src: %08X, len: %d",src, len_bytes);
	*flash_operation_pending = true;
    ret_code_t rc = nrf_fstorage_read(&datalog_storage_config, src, (void*)p_dest, len_bytes);
    if(rc == NRF_SUCCESS)
	{
		datalog_wait_for_flash_ready(&datalog_storage_config);
	}
	else
	{
		DATALOG_DEBUG("error %d",rc);
	}
	*flash_operation_pending = false;
	return rc;
}

uint32_t datalog_nrf_use_flash_erase(uint32_t page_addr, uint32_t number_pages, bool *flash_operation_pending)
{
	*flash_operation_pending = true;
	ret_code_t rc = nrf_fstorage_erase(&datalog_storage_config, page_addr, number_pages, NULL);
    if(rc == NRF_SUCCESS)
	{
		datalog_wait_for_flash_ready(&datalog_storage_config);
	}
	*flash_operation_pending = false;
	return rc;
}
void datalog_init(void)
{
	DATALOG_DEBUG("datalog_init");
	datalog_nrf_use_flash_init();
	dataLogger_Init();
}



void app_datalog_task()
{
	uint32_t res;
	if(datalog_tick_s_get() >= 5)
	{
		datalog_tick_s_reset();
		
		if(datalog_enabled == 0)
		{
			return;
		}
		
		uint32_t number_of_db_stored = 0;
		number_of_db_stored = dataLogger_GetNumberSaved(SENSOR_RECORD_LOG);
		datalog_number = number_of_db_stored;
//		NRF_LOG_INFO("number_of_db_stored: %d", number_of_db_stored);
		if(number_of_db_stored > 0 && gb_is_gateway_alive)
		{
			DATALOG_DEBUG("number_of_db_stored: %d", number_of_db_stored);
			DB_t db2;
			res = dataLogger_Pop(SENSOR_RECORD_LOG, &db2);
			
			DATALOG_DEBUG("Pop db2.u8cmd: %02X", db2.u8cmd);
			DATALOG_DEBUG("Pop db2.u8cmd: %02X", db2.u8type);
			uint32_t u32_cnt = db2.payload[8] | db2.payload[9] << 8;
//			DATALOG_DEBUG("datalog, u32_cnt: %d", u32_cnt);
			
		if(db2.payload[3] == 0x01 && db2.payload[1] == 0x01 && db2.payload[2] == 0x01)
			{
			}
		else if(db2.payload[3] == 0x02 && db2.payload[1] == 0x02 && db2.payload[2] == 0x02)
			{
			}
				else if(db2.payload[3] == 0x03 && db2.payload[1] == 0x03 && db2.payload[2] == 0x03)
			{
			}
			else
			{
				if(db2.payload[0] == 0x17 && db2.payload[3] == 0x00)
				{
					db2.payload[1] = 0x04;
					db2.payload[2] = 0x04;
					db2.payload[3] = 0x04;
				}
			}
			
			DATALOG_DEBUG("Data:Data[4..6] %02X %02X %02X..........................", db2.payload[4], db2.payload[5], db2.payload[6]);
			if(res == DATA_LOGGER_STT_SUCCESS) 
			{
				DATALOG_DEBUG("Pop History data SUCCESS..........");
				if(db2.u8cmd == 0 || db2.u8type == 0)
				{
					return;
				}
				transmit_fsm_prepare_payload(db2.payload, db2.u8cmd, db2.u8type, DATA_TRANSMIT_FSM_PAYLOAD_LEN);
			}
			else
			{
				DATALOG_DEBUG("Pop History data FAILED..........");
			}
		}
	}
}


void app_save_datalog(void)
{
	if(TASK_IDX_DEFAULT != task_control_get_current_task())
		return;
	if(is_datalog_save_enabled == true)
	{
		is_datalog_save_enabled = false;
		
		if(saveDB.payload[3] == 0x01 && saveDB.payload[1] == 0x01 && saveDB.payload[2] == 0x01)
			{
			}
		else if(saveDB.payload[3] == 0x02 && saveDB.payload[1] == 0x02 && saveDB.payload[2] == 0x02)
			{
			}
			else
			{
				if(saveDB.payload[0] == 0x17 && saveDB.payload[3] == 0x00)
				{
					saveDB.payload[1] = 0x03;
					saveDB.payload[2] = 0x03;
					saveDB.payload[3] = 0x03;
				}
			}
		
		uint32_t res = dataLogger_Save(SENSOR_RECORD_LOG, &saveDB);
		uint32_t u32_cnt = saveDB.payload[8] | saveDB.payload[9] << 8;
//		DATALOG_DEBUG("app_save_datalog, u32_cnt: %d", u32_cnt);
//		DATALOG_DEBUG("Save history cmd: %02X, type: %02X.........", saveDB.u8cmd, saveDB.u8type);
//		DATALOG_DEBUG("Data:Data[4..6] %02X %02X %02X..........................", saveDB.payload[4], saveDB.payload[5], saveDB.payload[6]);
		if(res == DATA_LOGGER_STT_SUCCESS) 
		{
//			DATALOG_DEBUG("SENSOR SAVE SUCCESS .........");
		}
		else
		{
//			DATALOG_DEBUG("SENSOR SAVE FAILED ..........");
		}
//		datalog_number = dataLogger_GetNumberSaved(SENSOR_RECORD_LOG);
//		DATALOG_DEBUG("SENSOR dataLogger_GetNumberSaved: %d..........", datalog_number);
	}
}

void save_energy_val(void)
{
	if(app_meter_trigger_saving_energy == true)
	{
		app_meter_trigger_saving_energy = false;
		uint32_t res = dataLogger_Save(METER_ENERGY_RECORD_LOG, &wh_cnt);
		if(res == DATA_LOGGER_STT_SUCCESS) 
		{
			DATALOG_DEBUG("SAVE ENERGY SUCCESS %d .........", wh_cnt);
		}
		else
		{
			DATALOG_DEBUG("SAVE ENERGY FAILED ..........");
		}
		DATALOG_DEBUG("ENERGY dataLogger_GetNumberSaved: %d..........", dataLogger_GetNumberSaved(METER_ENERGY_RECORD_LOG));
		//
		//
	}
}

void datalog_rtc_init(void)
{
}

void save_rtc_val(void)
{
	if(trigger_saving_rtc == true)
	{
		trigger_saving_rtc = false;
		uint32_t res = dataLogger_Save(RTC_RECORD_LOG, &rtcTimeSec);
		if(res == DATA_LOGGER_STT_SUCCESS) 
		{
			DATALOG_DEBUG("SAVE RTC SUCCESS %d.........", rtcTimeSec);
		}
		else
		{
			DATALOG_DEBUG("SAVE RTC FAILED ..........");
		}
		DATALOG_DEBUG("RTC dataLogger_GetNumberSaved: %d..........", dataLogger_GetNumberSaved(RTC_RECORD_LOG));
		//
	}
}

void datalog_tick_s_run()
{
	gu32_datalog_tick_s++;
}

void datalog_tick_s_reset()
{
	gu32_datalog_tick_s = 0;
}

uint32_t datalog_tick_s_get()
{
	return gu32_datalog_tick_s;
}


void datalog_tick_ms_run()
{
	gu32_datalog_tick_ms++;
}

void datalog_tick_ms_reset()
{
	gu32_datalog_tick_ms = 0;
}

uint32_t datalog_tick_ms_get()
{
	return gu32_datalog_tick_ms;
}

void datalog_clear()
{
	DATALOG_DEBUG("Clear all data");
	nrf_delay_ms(1000);
//	for (int i = 0; i < MAX_DB_MEMORY_DATA_TYPE_CONFIG; i++) {
//		uint32_t offset = dataLoggerConfigTable[i].memoryIndexSector;
//		uint32_t total_sectors = 
//			dataLoggerConfigTable[i].memoryDataSector
//			+ dataLoggerConfigTable[i].memoryLength
//			- offset;
//		datalog_nrf_use_flash_erase(
//			MEMORY_BASE_ADDR + offset*MEMORY_SECTOR_SIZE,
//			total_sectors, NULL);
//	}
	bool flag;
	datalog_nrf_use_flash_erase(DATALOG_STORAGE_ADDR_START, 26, &flag);
}

void datalog_clear_history_sensor()
{
	DATALOG_DEBUG("datalog_clear_history_sensor");
	bool flag;
	datalog_nrf_use_flash_erase(DATALOG_STORAGE_ADDR_START, 26, &flag);
}

