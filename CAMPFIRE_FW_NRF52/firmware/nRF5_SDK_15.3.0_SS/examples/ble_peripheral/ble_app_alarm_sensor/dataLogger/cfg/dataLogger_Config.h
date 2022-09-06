/**************************************************************************************************
 *
 * Copyright (C)
 *************************************************************************************************/
#ifndef __DATALOGGER_CONFIG_H__
#define __DATALOGGER_CONFIG_H__
#include <stdint.h>
#include "system_config.h"
#include "app_datalog.h"

//#include "app_fw_ota.h"
/*******************************************************************************/
/*Definitions: This file to declare enum, data and config memory layout        */
/*******************************************************************************/
typedef enum
{
//    DATA_LOG_TYPE_8BYTE,
//    DATA_LOG_TYPE_9BYTE,
//    DATA_LOG_TYPE_10BYTE,
//    DATA_LOG_TYPE_11BYTE,
	SENSOR_RECORD_LOG,
	METER_ENERGY_RECORD_LOG,
	RTC_RECORD_LOG
/*User add more type here*/
} dbDataType_t;

/*******************************************************************************
 * Data Struct
 ******************************************************************************/


typedef struct __attribute__((packed)){
	uint32_t time;
	uint8_t speed[30];
	uint8_t sent;
	uint8_t dummy[3];
	uint16_t crc;
}msg_speed_record_t;

typedef struct __attribute__((packed)){
	uint16_t head;
	uint16_t tail;
	uint16_t cnt;
	uint16_t 	crc;
}log_ring_record_t;

typedef struct __attribute__((packed)){
	uint32_t value;
	uint32_t oldValue;
	uint16_t cnt;
	uint16_t crc;
}DB_U32;
typedef struct
{/*Demo data structure*/
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    uint16_t data4;
} dataAign8b_t;
typedef struct
{/*Demo data structure*/
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    uint16_t data4;
    uint8_t data5;
}__attribute__((packed, aligned(1)))dataAign9b_t;
/*To optimize 9 byte*/
typedef struct
{/*Demo data structure*/
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    uint16_t data4;
    uint16_t data5;
} dataAign10b_t;
typedef struct
{/*Demo data structure*/
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    uint16_t data4;
    uint16_t data5;
    uint8_t data6;
}__attribute__((packed, aligned(1))) dataAign11b_t;
/*To optimize 9 byte*/
/*******************************************************************************
 * Memory Layout
 ******************************************************************************/

#define MAX_DB_MEMORY_DATA_TYPE_CONFIG      3

#define MEMORY_SECTOR_SIZE                  4096
#define NUMBER_SECTOR_INDEX                 2
#define LOGRING_INDEX_MAX_SIZE              (NUMBER_SECTOR_INDEX * MEMORY_SECTOR_SIZE) /*Default 2 sector 4K*/


#define MAX_OF_STRUCT_DATA_LOG              600//6000 /*Max of struct data record + Dummy + 2byteCRC : (Must %4 == 0)*/

#define MEMORY_BASE_ADDR                    DATALOG_STORAGE_ADDR_START /*Depend on FLASH LOG_MEMORY_BASE_ADDR*/  

#define SENSOR_VAL_INDEX_SECTOR           0x0 /*Default: Use 2 sector*/
#define SENSOR_VAL_DATA_SECTOR            0x0A
#define SENSOR_VAL_DATA_NUMBER_SECTOR     0x0A /*Must be use more than 2 sector*/

#define RTC_VAL_INDEX_SECTOR           		0x15 /*Default: Use 2 sector*/
#define RTC_VAL_DATA_SECTOR            		0x17
#define RTC_VAL_DATA_NUMBER_SECTOR     		2 /*Must be use more than 2 sector*/

#define ENERGY_VAL_INDEX_SECTOR           0x1B /*Default: Use 2 sector*/
#define ENERGY_VAL_DATA_SECTOR            0x1D
#define ENERGY_VAL_DATA_NUMBER_SECTOR     2 /*Must be use more than 2 sector*/



#define DL_TC_9B_INDEX_SECTOR           0 /*Default: Use 2 sector*/
#define DL_TC_9B_DATA_SECTOR            2
#define DL_TC_9B_DATA_NUMBER_SECTOR     3 /*Must be use more than 2 sector*/

#define DL_TC_10B_INDEX_SECTOR           0 /*Default: Use 2 sector*/
#define DL_TC_10B_DATA_SECTOR            2
#define DL_TC_10B_DATA_NUMBER_SECTOR     3 /*Must be use more than 2 sector*/

#define DL_TC_11B_INDEX_SECTOR           4 /*Default: Use 2 sector*/
#define DL_TC_11B_DATA_SECTOR            6
#define DL_TC_11B_DATA_NUMBER_SECTOR     3 /*Must be use more than 2 sector*/


#endif /* __DATALOGGER_CONFIG_H__ */
