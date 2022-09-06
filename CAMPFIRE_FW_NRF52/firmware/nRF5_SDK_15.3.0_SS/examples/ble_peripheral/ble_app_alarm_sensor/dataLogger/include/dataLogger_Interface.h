/**************************************************************************************************
 *
 * Copyright (C) 2019
 *
 *************************************************************************************************/
#ifndef __DATALOGGER_INTERFACE_H__
#define __DATALOGGER_INTERFACE_H__
#include <stdint.h>
#include <stdbool.h>
#include "dataLogger_Config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NUMBER_RETRY        3

typedef enum
{
    DATA_LOGGER_STT_NONE,        /**< return when memory is empty*/
    DATA_LOGGER_STT_SUCCESS,     /**< return when process init, read, write, get, pop is successful*/
    DATA_LOGGER_STT_FAIL,        /**< return when process init, read, write, get, pop is failure*/
    DATA_LOGGER_STT_INIT_FAIL,   /**< return when datatype has not initialized yet*/
    DATA_LOGGER_STT_MAX = 0xFF   /**< Max size of status enum*/
} dataLoggerStatus_t;

typedef struct
{
    dbDataType_t DataType;          /**< The type of Data*/
    uint32_t memoryIndexSector;     /**< The start sector of ringLog in Memory */
    uint32_t memoryDataSector;      /**< The start sector of dataLog in Memory */
    uint16_t memoryLength;          /**< The Length of user-defined structure */
    uint16_t memoryNumberSector;    /**< The Number sector to save data*/
} dataLoggerConfig_t;

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief This function must be called from the main state machine once time
 * right at initialize stage for initializing default value for variables
 */
void dataLogger_Init(void);

/*!
 * @brief This function to save data record to memory
 *
 * @param dbDataType : type of data to save data.
 * @param pdbDataItem: pointer to buffer or struct data to save.
 *
 * @return - DATA_LOGGER_STT_SUCCESS: if successful
 *         - DATA_LOGGER_STT_FAIL: if failed
 *         - DATA_LOGGER_STT_NONE: if the memory is empty
 *         - DATA_LOGGER_STT_NON_INIT: if the data type has not initialized yet.
 */
dataLoggerStatus_t dataLogger_Save(dbDataType_t dbDataType, void* pdbDataItem);

/*!
 * @brief This function to read latest data from memory
 *
 * @param dbDataType : type of data to save data.
 * @param pdbDataItem: pointer to buffer or struct data to save.
 *
 * @return - DATA_LOGGER_STT_SUCCESS: if successful
 *         - DATA_LOGGER_STT_FAIL: if failed
 *         - DATA_LOGGER_STT_NONE: if the memory is empty
 *         - DATA_LOGGER_STT_NON_INIT: if the data type has not initialized yet.
 */
dataLoggerStatus_t dataLogger_Get(dbDataType_t dbDataType, void* pdbDataItem);

/*!
 * @brief This function to pop oldest data valid data record.
 *
 * @param dbDataType : type of data to save data.
 * @param pdbDataItem: pointer to buffer or struct data to save.
 *
 * @return - DATA_LOGGER_STT_SUCCESS: if successful
 *         - DATA_LOGGER_STT_FAIL: if failed
 *         - DATA_LOGGER_STT_NONE: if the memory is empty
 *         - DATA_LOGGER_STT_NON_INIT: if the data type has not initialized yet.
 */
dataLoggerStatus_t dataLogger_Pop(dbDataType_t dbDataType, void* pdbDataItem);
/*!
 * @brief This function to get data valid data record, if fail, get previous value.
 *
 * @param dbDataType : type of data to save data.
 * @param pdbDataItem: pointer to buffer or struct data to save.
 *
 * @return - DATA_LOGGER_STT_SUCCESS: if successful
 *         - DATA_LOGGER_STT_FAIL: if failed
 *         - DATA_LOGGER_STT_NONE: if the memory is empty
 *         - DATA_LOGGER_STT_NON_INIT: if the data type has not initialized yet.
 */
dataLoggerStatus_t dataLogger_GetLatestValid(dbDataType_t dbDataType, void* pdbDataItem);

/*!
 * @brief get number of data saved in memory
 *
 * @param dbDataType : type of data to save data.
 *
 * @return uint32_t : number of data saved
 */
uint32_t dataLogger_GetNumberSaved(dbDataType_t dbDataType);

#endif /* __DATALOGGER_INTERFACE_H__ */
