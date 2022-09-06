/**************************************************************************************************
 *
 * Copyright (C) 2019
 *
 *************************************************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "dataLogger_Config.h"
#include "dataLogger_UserPort.h"
#include "dataLogger_UT.h"
/*******************************************************************************
 * This file to test functions
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DL_UT_001_Init_Empty(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t ringHead = 0;
    uint16_t ringTail = 0;
    uint16_t ringIndexCount = 0;
    /*check structure data*/
    TEST_ASSERT_EQUAL(8, sizeof(dataAign8b_t));
    TEST_ASSERT_EQUAL(9, sizeof(dataAign9b_t));
    TEST_ASSERT_EQUAL(10, sizeof(dataAign10b_t));
    TEST_ASSERT_EQUAL(11, sizeof(dataAign11b_t));
    /*end check*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);

    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringIndexCount);
}
void DL_UT_002_Init_Sector0HasData_Sector1Empty(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t numberRingLogPerSector = (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE);
    uint32_t ringAddressIndex = 0;
    uint16_t latestSector = 0;
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Write to 1/2 sector to have condition*/
    for(index = 0; index < (numberRingLogPerSector/2); index++)
    {
        status = testRingLogSave(DataType);
    }
    /*Init to test for this Case*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    /*ringIndexCount will +1 after Init*/
    TEST_ASSERT_EQUAL_UINT16(((numberRingLogPerSector/2)), ringIndexCount);
    latestSector = testSearchLatestSectorValid(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, latestSector);
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector/2, ringAddressIndex);
}
void DL_UT_003_Init_Sector0Full_Sector1Empty(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t numberRingLogPerSector = (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE);
    uint32_t ringAddressIndex = 0;
    uint16_t latestSector = 0;
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Write to Full 1 sector to have condition
    * because init skip first index data record*/
    for(index = 0; index < (numberRingLogPerSector-1); index++)
    {
        status = testRingLogSave(DataType);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Init to test for this Case*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector-1), ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector-1, ringAddressIndex);
    testLogRingIndexClearStruct(DataType);
    latestSector = testSearchLatestSectorValid(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, latestSector);
}
void DL_UT_004_Init_Sector0Full_Sector1HasData(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t numberRingLogPerSector = (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE);
    uint32_t ringAddressIndex = 0;
    uint16_t latestSector = 0;
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    for(index = 0; index < (numberRingLogPerSector+(numberRingLogPerSector/2)); index++)
    {
        status = testRingLogSave(DataType);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Init to test for this Case*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector+(numberRingLogPerSector/2)), ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector+(numberRingLogPerSector/2), ringAddressIndex);
    testLogRingIndexClearStruct(DataType);
    latestSector = testSearchLatestSectorValid(DataType);
    TEST_ASSERT_EQUAL_UINT16(1, latestSector);
}
void DL_UT_005_Init_Sector1Full_Sector0Empty(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t numberRingLogPerSector = (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE);
    uint16_t latestSector = 0;
    uint32_t ringAddressIndex = 0;
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    for(index = 0; index < ((numberRingLogPerSector*NUMBER_SECTOR_INDEX)-1); index++)
    {
        status = testRingLogSave(DataType);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Init to test for this Case*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(((numberRingLogPerSector*NUMBER_SECTOR_INDEX)-1), ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector*NUMBER_SECTOR_INDEX)-1, ringAddressIndex);
    testLogRingIndexClearStruct(DataType);
    latestSector = testSearchLatestSectorValid(DataType);
    TEST_ASSERT_EQUAL_UINT16(1, latestSector);
}
void DL_UT_006_Init_Sector1Full_Sector0HasData(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t numberRingLogPerSector = (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE);
    uint16_t latestSector = 0;
    uint32_t ringAddressIndex = 0;
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    for(index = 0; index < ((numberRingLogPerSector*NUMBER_SECTOR_INDEX)+(numberRingLogPerSector/2)); index++)
    {
        status = testRingLogSave(DataType);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Init to test for this Case*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(((numberRingLogPerSector*NUMBER_SECTOR_INDEX)+(numberRingLogPerSector/2)), ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector/2), ringAddressIndex);
    testLogRingIndexClearStruct(DataType);
    latestSector = testSearchLatestSectorValid(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, latestSector);
}
void DL_UT_007_Init_GetLogRing_CRC_fail_Head(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t numberRingLogPerSector = (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE);
    uint16_t latestSector = 0;
    uint32_t ringAddressIndex = 0;
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    for(index = 0; index < (numberRingLogPerSector/2); index++)
    {
        status = testRingLogSave(DataType);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Write head to fail CRC 3 log*/
    testLogRingIndexGet_WriteFailCRC(DataType, 0);
    testLogRingIndexGet_WriteFailCRC(DataType, 1);
    testLogRingIndexGet_WriteFailCRC(DataType, 3);
    /*Init to test for this Case*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector/2), ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector/2), ringAddressIndex);
    testLogRingIndexClearStruct(DataType);
    latestSector = testSearchLatestSectorValid(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, latestSector);
}
void DL_UT_008_Init_GetLogRing_CRC_fail_Mid(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t numberRingLogPerSector = (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE);
    uint16_t latestSector = 0;
    uint32_t ringAddressIndex = 0;
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    for(index = 0; index < (numberRingLogPerSector/2); index++)
    {
        status = testRingLogSave(DataType);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Write head to fail CRC 2 log*/
    testLogRingIndexGet_WriteFailCRC(DataType, numberRingLogPerSector/4);
    testLogRingIndexGet_WriteFailCRC(DataType, numberRingLogPerSector/3);
    /*Init to test for this Case*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector/2), ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector/2), ringAddressIndex);
    testLogRingIndexClearStruct(DataType);
    latestSector = testSearchLatestSectorValid(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, latestSector);
}
void DL_UT_009_Init_GetLogRing_CRC_fail_Tail(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t numberRingLogPerSector = (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE);
    uint16_t latestSector = 0;
    uint32_t ringAddressIndex = 0;
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    for(index = 0; index < (numberRingLogPerSector/2); index++)
    {
        status = testRingLogSave(DataType);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Write head to fail CRC 2 log*/
    testLogRingIndexGet_WriteFailCRC(DataType, numberRingLogPerSector/2);
    testLogRingIndexGet_WriteFailCRC(DataType, numberRingLogPerSector/2-1);
    /*Init to test for this Case*/
    status = test_getDataTypeInfo(DataType);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector/2)-2, ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16((numberRingLogPerSector/2)-2, ringAddressIndex);
    testLogRingIndexClearStruct(DataType);
    latestSector = testSearchLatestSectorValid(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, latestSector);
}

void DL_UT_010_Save_1Record_Success(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint32_t ringAddressIndex = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];

    status = dataLogger_Save(DataType, sampleData);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(1, ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16(1, ringAddressIndex);
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(1, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    status = dataLogger_Get(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
}

extern bool enableWriteFail;
void DL_UT_011_Save_1Record_Fail(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint32_t ringAddressIndex = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE, 0xFF, 0xFF, 0xFF};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];

    enableWriteFail = true;
    status = dataLogger_Save(DataType, sampleData);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(1, ringIndexCount);
    /*Get latest sector*/
    ringAddressIndex = testHandleRingAddressIndexGet(DataType);
    TEST_ASSERT_EQUAL_UINT16(1, ringAddressIndex);
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(2, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    status = dataLogger_Get(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
}
void DL_UT_012_Save_Full_1Sector(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE, 0xFF, 0xFF, 0xFF};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    uint16_t numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;

    for(index = 0; index < numberRingLogPerSector; index++)
    {
        sampleData[2] = sampleData[2] + index;
        sampleData[3] = sampleData[3] + index;
        status = dataLogger_Save(DataType, sampleData);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    status = dataLogger_Get(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
}
void DL_UT_013_Save_Full_2Sector(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE, 0xFF, 0xFF, 0xFF};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    uint16_t numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;

    for(index = 0; index < (numberRingLogPerSector*2); index++)
    {
        sampleData[2] = sampleData[2] + index;
        sampleData[3] = sampleData[3] + index;
        status = dataLogger_Save(DataType, sampleData);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector*2, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector*2, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    status = dataLogger_Get(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
}
void DL_UT_014_Save_Full_3Sector(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE, 0xFF, 0xFF, 0xFF};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    uint16_t numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;

    for(index = 0; index < (numberRingLogPerSector*3); index++)
    {
        sampleData[2] = sampleData[2] + index;
        sampleData[3] = sampleData[3] + index;
        status = dataLogger_Save(DataType, sampleData);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector*3, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringTail);
    status = dataLogger_Get(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
}
void DL_UT_015_Save_Full_4Sector(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE, 0xFF, 0xFF, 0xFF};
    uint8_t  sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    uint16_t numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;

    for(index = 0; index < (numberRingLogPerSector*4); index++)
    {
        sampleData[2] = sampleData[2] + index;
        sampleData[3] = sampleData[3] + index;
        status = dataLogger_Save(DataType, sampleData);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector*4, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector*2, ringTail);
    status = dataLogger_Get(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
}
void DL_UT_016_Get_Empty(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t  sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    if(status == DATA_LOGGER_STT_SUCCESS)
    {
        ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
        TEST_ASSERT_EQUAL_UINT16(0, ringIndexCount);
        /*Get latest sector*/
        ringHead = testLogRingIndexGet_Head(DataType);
        TEST_ASSERT_EQUAL_UINT16(0, ringHead);
        ringTail = testLogRingIndexGet_Tail(DataType);
        TEST_ASSERT_EQUAL_UINT16(0, ringTail);
        status = dataLogger_Get(DataType,sampleDataGet);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_NONE, status);
}
void DL_UT_017_Get_Has1Record(dbDataType_t DataType)
{
    /*Same test with this function*/
    DL_UT_010_Save_1Record_Success(DataType);
    /*This function include Get function*/
}
void DL_UT_018_Get_HasNRecord(dbDataType_t DataType)
{
    /*Same test with this function*/
    DL_UT_012_Save_Full_1Sector(DataType);
    /*This function include Get function*/
}
void DL_UT_019_Get_HarNRecord_failCRC(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint16_t currentAddress = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE};
    uint8_t buffer[11] = {0xF1, 0x1F, 0x00, 0xFF, 0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    uint16_t numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;

    for(index = 0; index < numberRingLogPerSector; index++)
    {
        sampleData[2] = sampleData[2] + index;
        sampleData[3] = sampleData[3] + index;
        status = dataLogger_Save(DataType, sampleData);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    /*write to create fail CRC*/
    currentAddress = testGetCurrentAddress(testHandleGetstartAddressData(DataType), ringHead-1, lenghDataDummyCRC);
    test_dataLoggerDriver_Write((uint8_t*) buffer, currentAddress, LOGRING_INDEX_SIZE-4);

    status = dataLogger_Get(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_FAIL, status);
}
void DL_UT_020_Pop_Empty(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t  sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    /*First init when Memory is Empty */
    status = test_getDataTypeInfo(DataType);
    if(status == DATA_LOGGER_STT_SUCCESS)
    {
        ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
        TEST_ASSERT_EQUAL_UINT16(0, ringIndexCount);
        /*Get latest sector*/
        ringHead = testLogRingIndexGet_Head(DataType);
        TEST_ASSERT_EQUAL_UINT16(0, ringHead);
        ringTail = testLogRingIndexGet_Tail(DataType);
        TEST_ASSERT_EQUAL_UINT16(0, ringTail);
        status = dataLogger_Pop(DataType,sampleDataGet);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_NONE, status);
}
void DL_UT_021_Pop_Has1Record(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];

    status = dataLogger_Save(DataType, sampleData);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(1, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    status = dataLogger_Pop(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Pop more 1 times => status = None*/
    status = dataLogger_Pop(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_NONE, status);
}
void DL_UT_022_Pop_HasNRecord(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE, 0xFF, 0xFF, 0xFF};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    uint16_t numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;

    for(index = 0; index < numberRingLogPerSector; index++)
    {
        sampleData[2] = index;
        status = dataLogger_Save(DataType, sampleData);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    for(index = 0; index < numberRingLogPerSector; index++)
    {
        sampleData[2] = index;
        status = dataLogger_Pop(DataType,sampleDataGet);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Pop more 1 times => status = None*/
    status = dataLogger_Pop(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_NONE, status);
}
void DL_UT_023_Pop_HarNRecord_failCRC(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE, 0xFF, 0xFF, 0xFF};
    uint8_t buffer[11] = {0xF1, 0x1F, 0x00, 0xFF, 0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = 0;
    uint16_t numberRingLogPerSector = 0;
    uint32_t startAddressData = 0;
    uint32_t currentAddress = 0;

    lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;
    for(index = 0; index < numberRingLogPerSector; index++)
    {
        sampleData[2] = index;
        status = dataLogger_Save(DataType, sampleData);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    /*write to create fail CRC*/
    startAddressData = testHandleGetstartAddressData(DataType);
    currentAddress = testGetCurrentAddress(startAddressData, ringHead-2, lenghDataDummyCRC);
    test_dataLoggerDriver_Write((uint8_t*) buffer, currentAddress, 8);
    for(index = 0; index < (numberRingLogPerSector-2); index++)
    {
        sampleData[2] = index;
        status = dataLogger_Pop(DataType,sampleDataGet);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData, sampleDataGet, dataLoggerConfigTable[DataType].memoryLength);
    }
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Pop more 1 times => status = None*/
    status = dataLogger_Pop(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    /*Pop more 1 times => status = None*/
    status = dataLogger_Pop(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_NONE, status);
}
void DL_UT_024_GetLatest_NoData(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];

    status = dataLogger_GetLatestValid(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_NONE, status);
}
void DL_UT_025_GetLatest_1Record(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];

    status = dataLogger_Save(DataType, sampleData);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    status = dataLogger_GetLatestValid(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    status = dataLogger_GetLatestValid(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
}
void DL_UT_026_GetLatest_Success(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    uint16_t numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;

    for(index = 0; index < numberRingLogPerSector; index++)
    {
        sampleData[2] = sampleData[2] + index;
        sampleData[3] = sampleData[3] + index;
        status = dataLogger_Save(DataType, sampleData);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);

    status = dataLogger_GetLatestValid(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
}
void DL_UT_027_GetLatest_Fail2CRC(dbDataType_t DataType)
{
    dataLoggerStatus_t status = DATA_LOGGER_STT_SUCCESS;
    uint16_t index = 0;
    uint16_t ringIndexCount = 0;
    uint16_t ringTail = 0;
    uint16_t ringHead = 0;
    uint16_t currentAddress = 0;
    uint8_t sampleData[11] = {0xAA, 0xBB, 0x03, 0x04, 0x05, 0x06, 0xDD, 0xEE};
    uint8_t buffer[11] = {0xF1, 0x1F, 0x00, 0xFF, 0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t sampleDataGet[dataLoggerConfigTable[DataType].memoryLength];
    uint16_t lenghDataDummyCRC = testHandleGetLengDataDummyCRC(DataType);
    uint16_t numberRingLogPerSector = MEMORY_SECTOR_SIZE / lenghDataDummyCRC;

    for(index = 0; index < numberRingLogPerSector; index++)
    {
        sampleData[2] = index;
        status = dataLogger_Save(DataType, sampleData);
        TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    }
    ringIndexCount = testLogRingIndexGet_IndexCount(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringIndexCount);
    /*Get latest sector*/
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    /*write to create fail CRC*/
    currentAddress = testGetCurrentAddress(testHandleGetstartAddressData(DataType), ringHead-1, lenghDataDummyCRC);
    test_dataLoggerDriver_Write((uint8_t*) buffer, currentAddress, LOGRING_INDEX_SIZE-4);
    currentAddress = testGetCurrentAddress(testHandleGetstartAddressData(DataType), ringHead-2, lenghDataDummyCRC);
    test_dataLoggerDriver_Write((uint8_t*) buffer, currentAddress, LOGRING_INDEX_SIZE-4);

    status = dataLogger_GetLatestValid(DataType,sampleDataGet);
    TEST_ASSERT_EQUAL(DATA_LOGGER_STT_SUCCESS, status);
    ringHead = testLogRingIndexGet_Head(DataType);
    TEST_ASSERT_EQUAL_UINT16(numberRingLogPerSector-2, ringHead);
    ringTail = testLogRingIndexGet_Tail(DataType);
    TEST_ASSERT_EQUAL_UINT16(0, ringTail);
    sampleData[2] = sampleData[2] - 2;
    TEST_ASSERT_EQUAL_UINT8_ARRAY(sampleData,sampleDataGet,dataLoggerConfigTable[DataType].memoryLength);
}
