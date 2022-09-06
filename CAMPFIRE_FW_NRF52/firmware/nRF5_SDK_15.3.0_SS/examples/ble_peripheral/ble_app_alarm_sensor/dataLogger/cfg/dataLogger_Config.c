/**************************************************************************************************
 *
 * Copyright (C)
 *************************************************************************************************/
#include <stdio.h>
#include "dataLogger_Interface.h"
#include "dataLogger_Config.h"
#include "system_config.h"

//#include "app_fw_ota.h"
//#include "dbLogRecord.h"
/*******************************************************************************
 * This file To declare dataLoggerConfigTable
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* user add more here */
const dataLoggerConfig_t dataLoggerConfigTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG] =
{
    {SENSOR_RECORD_LOG       	,SENSOR_VAL_INDEX_SECTOR		,SENSOR_VAL_DATA_SECTOR		,sizeof(DB_t)		,SENSOR_VAL_DATA_NUMBER_SECTOR},
		{ENERGY_VAL_INDEX_SECTOR  ,ENERGY_VAL_INDEX_SECTOR   ,ENERGY_VAL_DATA_SECTOR  	,sizeof(uint32_t)    ,ENERGY_VAL_DATA_NUMBER_SECTOR},
		{RTC_RECORD_LOG       		,RTC_VAL_INDEX_SECTOR      ,RTC_VAL_DATA_SECTOR  			,sizeof(uint32_t)    ,RTC_VAL_DATA_NUMBER_SECTOR},
};
