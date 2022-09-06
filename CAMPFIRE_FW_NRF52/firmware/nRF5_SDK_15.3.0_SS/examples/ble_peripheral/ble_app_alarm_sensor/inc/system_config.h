
#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "boards.h"
#include <stdint.h>
#include "app_util_platform.h"

#include "fraclib.h"
#include "smarthome_meter.h"
#include "ac_timer_task.h"
//#include "app_alarm.h"

//#include "fds_internal_defs.h"

/** @brief  Size of a flash codepage. This value is used for calculating the size of the reserved
 *          flash space in the bootloader region. It is checked against NRF_UICR->CODEPAGESIZE
 *          at run time to ensure that the region is correct.
 */
 
 // The size of a physical page, in 4-byte words.
#if defined(NRF51)
    #define FDS_PHY_PAGE_SIZE   (256)
#else
    #define FDS_PHY_PAGE_SIZE   (1024)
#endif

#define CODE_PAGE_SIZE            (FDS_PHY_PAGE_SIZE * sizeof(uint32_t))


/** @brief  Maximum size of a data object.*/
#if defined( NRF51 )
    #define DATA_OBJECT_MAX_SIZE           (CODE_PAGE_SIZE * 4)
#elif defined( NRF52_SERIES ) || defined ( __SDK_DOXYGEN__ )
    #define DATA_OBJECT_MAX_SIZE           (CODE_PAGE_SIZE)
#else
    #error "Architecture not set."
#endif

/** @brief  Page location of the bootloader settings address.
 */

#if defined ( NRF51 )
    #define APP_SYSTEM_CONFIG_ADDRESS     (0x0003FC00UL)
#elif defined( NRF52832_XXAA )
    #define APP_SYSTEM_CONFIG_ADDRESS     (0x0006F000UL)//(0x0007E000UL)
#elif defined( NRF52811_XXAA )
    #define APP_SYSTEM_CONFIG_ADDRESS     (0x00027000UL) 	// (0x00027000UL)/(0x0002E000UL)
#elif defined( NRF52840_XXAA )
    #define APP_SYSTEM_CONFIG_ADDRESS     (0x000FF000UL)
#else
    #error No valid target set for APP_SYSTEM_CONFIG_ADDRESS.
#endif

#define DEFAULT_GPSR_APN		"internet"
#define DEFAULT_GPRS_USR		"mms"
#define DEFAULT_GPRS_PWD		"mms"

#define MCU_RESET_NONE				0
#define MCU_RESET_IMMEDIATELY 1
#define MCU_RESET_AFTER_10_SEC 2
#define MCU_RESET_AFTER_30_SEC 3
#define MCU_RESET_IS_WAITING 4

#define FEATURE_RF_ENABLE        	0x00000008
#ifdef BENKON_HW_V1
#define VERSION										"V1.1.90"
#endif
#ifdef BENKON_HW_V2
#define VERSION										"V2.4.24"
//#define VERSION										"V2.3.13.6"
#endif
#ifdef BENKON_HW_V3
#define VERSION										"V3.0.24"
#endif

#define DB_VERSION (5)

#define DEFAULT_RADIO_ACCESS_ADDR 0xA542A688
//#define DEFAULT_RADIO_ACCESS_ADDR 0xA541A681
#define DEFAULT_RADIO_CHANNEL 38
#define DEFAULT_CUSTOMER_ID	"12345678"
#define DEFAULT_MANUFACTORY_ID	"12345678"
#define FEATURE_MUTE_ENABLE	0x10

#define FW_APP_TYPE_IR_LEARNING	1
#define FW_APP_TYPE_CAMPFIRE		2

#define NUMBER_OF_SCHEDULE					10 

#define CFG_SAVING_TRIGGER_TIMEOUT	5

#define IR_TX_WDT_TIMEOUT						2
typedef struct alarm_config
{
    uint8_t isEnabled;
    uint8_t hour;
    uint8_t min;
		uint8_t isRepeatAvailable;
    uint8_t isSundayRepeated;
    uint8_t isMondayRepeated;
    uint8_t isTuesdayRepeated;
    uint8_t isWednesdayRepeated;
    uint8_t isThursdayRepeated;
    uint8_t isFridayRepeated;
    uint8_t isSaturdayRepeated;
}alarm_config_t;

typedef struct
{
    alarm_config_t alarm_config;
    uint8_t ac_status[10]; //PROTOCOL_PAYLOAD_LEN = 10
}schedule_control_t;


typedef struct /* __attribute__((packed)) */ {
	uint32_t size;
	uint32_t radio_access_addr;
	uint8_t fwVersion[16];	/**< firmware version>**/
	uint8_t deviceKey[18];
	uint8_t radio_channel;
	uint8_t feature;
	uint8_t gatewayHandle;
	uint8_t gatewayAddr[6];
	uint8_t dummy[249];
	uint32_t IR_devicetype;
	uint32_t crc;
	
	schedule_control_t schedule_control[NUMBER_OF_SCHEDULE];
	uint8_t ac_payload[6];
	
	//
	uint8_t enable_sensor_streaming;
	uint8_t enable_power_streaming;
	uint8_t enable_remote_data_streaming;
	uint8_t enable_schedule_event_streaming;
	uint8_t enable_timer_event_streaming;
	
	uint8_t enable_sensor_saving;
	uint8_t enable_power_saving;
	uint8_t enable_remote_data_saving;
	uint8_t enable_schedule_event_saving;
	uint8_t enable_timer_event_saving;
	
	uint8_t enable_sensor_datalog;
	uint8_t enable_power_datalog;
	uint8_t enable_remote_data_datalog;
	uint8_t enable_schedule_event_datalog;
	uint8_t enable_timer_event_datalog;
	uint16_t database_version; 
	uint8_t ota_flag;
}CONFIG_POOL;

typedef struct
{
	double urms_cal;  /* preset calibration voltage [Vrms]                */
	double irms_cal[NUM_CHAN]; /* preset calibration current [Arms]                */
	double u_msr;
	double i_msr[NUM_CHAN];
	double p_msr[NUM_CHAN];
	/* basic power meter configuration data                                     */
	Frac32      u_msrmax;   /* measured maximum voltage                         */
	Frac32      u_msrmin;   /* measured minimum voltage                         */
	Frac32      i_msrmax[NUM_CHAN];   /* measured maximum current                        */
	Frac32      i_msrmin[NUM_CHAN];   /* measured minimum current                        */

	/* post-calibration data - calculated phase delay, offsets and gains        */
	int16_t     delay[NUM_CHAN];      /* ch1 delay in modulator clocks                   */
	int16_t     delayu;      /* u delay                                         */
	Frac32      i_offset[NUM_CHAN];   /* current measurement offset (AFE ch0)            */
  Frac32			u_offset;

  /* configuration flag                                                       */
  uint16_t      flag;
  tCORRECT_DATA correct;
} tCONFIG_FLASH_DATA;


typedef struct __attribute__((packed))
{
	uint8_t payload[10];
	uint8_t u8cmd;
	uint8_t u8type;
	uint32_t u32reserved;
}DB_t;


extern volatile tCONFIG_FLASH_DATA  ramcfg;
extern CONFIG_POOL sysCfg;
extern uint8_t mainBuf[340];
extern uint8_t sysResetMcuFlag;
extern uint8_t u8FwType;
extern uint32_t gu32_save_cfg_timeout;
uint32_t CFG_Init(void);
ret_code_t CFG_Save(void);
void CFG_Load(bool reset);
void CFG_ReLoad(void);
void CFG_Write(uint8_t *buff, uint32_t address, int offset, int length);
uint8_t CFG_CheckSum(CONFIG_POOL *sysCfg);
uint8_t CFG_Check(CONFIG_POOL *sysCfg);
void ResetMcuSet(uint8_t resetType);
void ResetMcuTask(void);

void CFG_Saving_Trigger(void);
void CFG_Saving_Task(void);
void CFG_Saving_Tick(void);

#endif
