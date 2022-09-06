/**************************************************************************************************
 *
 * Copyright (C) 2019
 *
 *************************************************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "dataLogger_Config.h"
#include "dataLogger_Interface.h"
#include "dataLogger_IT.h"
/*******************************************************************************
 * This file to test functions
 ******************************************************************************/
 /*******************************************************************************
 * Definitions
 ******************************************************************************/
#define  CRC_LENGTH         2
#define  ALIGNMENT_MEMORY   4
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern const dataLoggerConfig_t dataLoggerConfigTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG];

/*******************************************************************************
 * Utilities functions
 ******************************************************************************/
uint16_t lengthWithCRC(uint16_t length);
uint16_t lengthWithCRC(uint16_t length)
{
    uint16_t numberByteDummy = ALIGNMENT_MEMORY - ((length + CRC_LENGTH) % ALIGNMENT_MEMORY);
    return (length + numberByteDummy + CRC_LENGTH);
}

void fillBuffer(uint8_t *buffer, uint16_t length, uint8_t value);
void fillBuffer(uint8_t *buffer, uint16_t length, uint8_t value)
{
    uint16_t index = 0;

    for (index = 0; index < length; index++)
    {
        buffer[index] = value;
    }
}

/*******************************************************************************
 * Test case
 ******************************************************************************/
void DL_IT_001_SaveGetVerify(dbDataType_t dataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_NONE;
    uint16_t testLoopNumber = 0;
    uint16_t index = 0;
    uint16_t maxNumberDataLog = 0;
    uint8_t bufferForWrite[dataLoggerConfigTable[dataType].memoryLength];
    uint8_t bufferForRead[dataLoggerConfigTable[dataType].memoryLength];
    uint8_t value = 0;

    maxNumberDataLog = MEMORY_SECTOR_SIZE / lengthWithCRC(dataLoggerConfigTable[dataType].memoryLength);
    maxNumberDataLog *= dataLoggerConfigTable[dataType].memoryNumberSector;

    for (testLoopNumber = 0; testLoopNumber < 2; testLoopNumber++)
    {
        for (index = 0; index < maxNumberDataLog; index++)
        {
            fillBuffer(bufferForWrite, dataLoggerConfigTable[dataType].memoryLength, value);
            value++;

            status = dataLogger_Save(dataType, &bufferForWrite);
            TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);

            status = dataLogger_Get(dataType, &bufferForRead);
            TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);

            TEST_ASSERT_EQUAL_UINT8_ARRAY(&bufferForWrite, &bufferForRead, dataLoggerConfigTable[dataType].memoryLength);
        }
    }
}
void DL_IT_002_SavePopVerify(dbDataType_t dataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_NONE;
    uint16_t testLoopNumber = 0;
    uint16_t index = 0;
    uint16_t maxNumberDataLog = 0;
    uint8_t bufferForWrite[dataLoggerConfigTable[dataType].memoryLength];
    uint8_t bufferForPop[dataLoggerConfigTable[dataType].memoryLength];
    uint8_t value = 0;

    maxNumberDataLog = MEMORY_SECTOR_SIZE / lengthWithCRC(dataLoggerConfigTable[dataType].memoryLength);
    maxNumberDataLog *= dataLoggerConfigTable[dataType].memoryNumberSector;

    for (testLoopNumber = 0; testLoopNumber < 2; testLoopNumber++)
    {
        for (index = 0; index < maxNumberDataLog; index++)
        {
            fillBuffer(bufferForWrite, dataLoggerConfigTable[dataType].memoryLength, value);
            value++;

            status = dataLogger_Save(dataType, &bufferForWrite);
            TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);

            status = dataLogger_Pop(dataType, &bufferForPop);
            TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);

            TEST_ASSERT_EQUAL_UINT8_ARRAY(&bufferForWrite, &bufferForPop, dataLoggerConfigTable[dataType].memoryLength);
        }
    }
}
void DL_IT_003_SaveOneRoundPopOneRound(dbDataType_t dataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_NONE;
    uint16_t index = 0;
    uint16_t maxNumberDataLog = 0;
    uint8_t bufferForWrite[dataLoggerConfigTable[dataType].memoryLength];
    uint8_t bufferForPop[dataLoggerConfigTable[dataType].memoryLength];
    uint8_t value = 0;
    uint16_t numberOfSavedRecord = 0;

    maxNumberDataLog = MEMORY_SECTOR_SIZE / lengthWithCRC(dataLoggerConfigTable[dataType].memoryLength);
    maxNumberDataLog *= dataLoggerConfigTable[dataType].memoryNumberSector;

    for (index = 0; index < maxNumberDataLog; index++)
    {
        fillBuffer(bufferForWrite, dataLoggerConfigTable[dataType].memoryLength, value);
        value++;

        status = dataLogger_Save(dataType, &bufferForWrite);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }

    numberOfSavedRecord = dataLogger_GetNumberSaved(dataType);
    value = value - numberOfSavedRecord;
    for (index = 0; index < numberOfSavedRecord; index++)
    {
        status = dataLogger_Pop(dataType, &bufferForPop);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);

        fillBuffer(bufferForWrite, dataLoggerConfigTable[dataType].memoryLength, value);
        value++;

        TEST_ASSERT_EQUAL_UINT8_ARRAY(&bufferForWrite, &bufferForPop, dataLoggerConfigTable[dataType].memoryLength);
    }
}
