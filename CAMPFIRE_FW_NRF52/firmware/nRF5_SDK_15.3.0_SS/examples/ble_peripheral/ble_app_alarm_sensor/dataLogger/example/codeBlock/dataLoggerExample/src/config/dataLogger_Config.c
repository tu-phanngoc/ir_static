/**************************************************************************************************
 *
 * Copyright (C) 2019
 *************************************************************************************************/
#include <stdio.h>
#include "dataLogger_Interface.h"
#include "dataLogger_Config.h"
/*******************************************************************************
 * This file To declare dataLoggerConfigTable
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* user add more here */
const dataLoggerConfig_t dataLoggerConfigTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG] =
{
    {EXAMPLE1_TYPE       ,EXAMPLE_DB_MEM_INDEX_SECTOR       ,EXAMPLE_DB_MEM_DATA_SECTOR  ,sizeof(dataExample_t)    , EXAMPLE_DB_MEM_DATA_NUMBER_SECTOR},
    /* user add more here */
};
