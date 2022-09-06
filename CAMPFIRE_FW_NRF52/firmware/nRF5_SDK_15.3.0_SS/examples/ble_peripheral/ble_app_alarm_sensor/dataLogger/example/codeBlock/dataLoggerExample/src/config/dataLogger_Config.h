/**************************************************************************************************
 *
 * Copyright (C) 2019
 *
 *************************************************************************************************/
#ifndef __DATALOGGER_CONFIG_H__
#define __DATALOGGER_CONFIG_H__
#include <stdint.h>
/*******************************************************************************/
/*Definitions: This file to declare enum, data and config memory layout        */
/*******************************************************************************/
typedef enum
{
    EXAMPLE1_TYPE
/*User add more type here*/
} dbDataType_t;

/*******************************************************************************
 * Data Struct
 ******************************************************************************/
typedef struct
{
    uint32_t head;
    uint32_t count;
    uint32_t data;
    uint32_t tail;
} dataExample_t;


/*******************************************************************************
 * Memory Layout
 ******************************************************************************/

#define MAX_DB_MEMORY_DATA_TYPE_CONFIG      1

#define MEMORY_SECTOR_SIZE                  128
#define NUMBER_SECTOR_INDEX                 2
#define LOGRING_INDEX_MAX_SIZE              (NUMBER_SECTOR_INDEX * MEMORY_SECTOR_SIZE) /*Default 2 sector 4K*/

#define MAX_OF_STRUCT_DATA_LOG              32 /*Max of struct data record + Dummy + 2byteCRC : (Must %4 == 0)*/

#define MEMORY_BASE_ADDR                    0x00000000 /*Depend on FLASH*/

#define EXAMPLE_DB_MEM_INDEX_SECTOR           0 /*Default: Use 2 sector*/
#define EXAMPLE_DB_MEM_DATA_SECTOR            2
#define EXAMPLE_DB_MEM_DATA_NUMBER_SECTOR     2 /*Must be use more than 2 sector*/

#endif /* __DATALOGGER_CONFIG_H__ */
