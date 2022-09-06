/**************************************************************************************************
 *
 * Copyright (C) 2019
 *
 *************************************************************************************************/
#include <stdbool.h>
#include <unity.h>
#ifndef __DATALOGGER_UT_H__
#define __DATALOGGER_UT_H__
#define LOGRING_INDEX_SIZE 12
/*Extern all function to test Unit*/
/**
 * @brief Extern all function to test Unit
 */
extern dataLoggerStatus_t testRingLogSave(dbDataType_t dbDataType);
extern uint16_t testSearchLatestSectorValid(dbDataType_t dbDataType);
extern void testLogRingIndexClearStruct(dbDataType_t dbDataType);
extern uint16_t testLogRingIndexGet_Head(dbDataType_t dbDataType);
extern uint16_t testLogRingIndexGet_Tail(dbDataType_t dbDataType);
extern uint16_t testLogRingIndexGet_IndexCount(dbDataType_t dbDataType);
extern uint16_t testLogRingIndexGet_WriteFailCRC(dbDataType_t dbDataType, uint16_t ringAddressIndex);
extern uint32_t testHandleRingAddressIndexGet(dbDataType_t dbDataType);
extern void testHandleRingAddressIndexSet(dbDataType_t dbDataType, uint32_t Value);
extern uint16_t testHandleGetIndexSector(dbDataType_t dbDataType);
extern uint16_t testHandleGetLengDataDummyCRC(dbDataType_t dbDataType);
extern uint16_t testHandleGetMaxDataRecordNumber(dbDataType_t dbDataType);
extern uint32_t testHandleGetstartAddressData(dbDataType_t dbDataType);
extern uint32_t testGetCurrentAddress(uint32_t baseAddress, uint16_t recordIndex, uint16_t lenghDataWithCRC);
extern void test_dataLoggerDriver_Write(uint8_t* DataBuffer, uint32_t Address, uint32_t Lengh);
extern const dataLoggerConfig_t dataLoggerConfigTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG];
extern dataLoggerStatus_t dataLogger_GetLatestValid(dbDataType_t dbDataType,void* pdbDataItem);
extern dataLoggerStatus_t test_getDataTypeInfo(dbDataType_t dbDataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when has no data in memory
 * Result test: successfully, indexcount = 0, latest sector = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_001_Init_Empty(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when sector 0 has data, sector 1 empty
 * Result test: successfully, indexcount = N, latest sector = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_002_Init_Sector0HasData_Sector1Empty(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when sector 0 full, sector 1 empty
 * Result test: successfully, indexcount = N, latest sector = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_003_Init_Sector0Full_Sector1Empty(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when sector 0 full, sector 1 has data
 * Result test: successfully, indexcount = N, latest sector = 1
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_004_Init_Sector0Full_Sector1HasData(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when sector 0 empty, sector 1 full
 * Result test: successfully, indexcount = N, latest sector = 1
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_005_Init_Sector1Full_Sector0Empty(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when sector 0 has data, sector 1 full
 * Result test: successfully, indexcount = N, latest sector = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_006_Init_Sector1Full_Sector0HasData(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when has CRC fail in head of logring
 * Result test: successfully, indexcount = N-1, latest sector = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_007_Init_GetLogRing_CRC_fail_Head(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when has CRC fail in midlle of logring
 * Result test: successfully, indexcount = N, latest sector = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_008_Init_GetLogRing_CRC_fail_Mid(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case get Infotype when has CRC fail in end of logring
 * Result test: successfully, indexcount = N, latest sector = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_009_Init_GetLogRing_CRC_fail_Tail(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Save 1 record sucess.
 * Result test: successfully, indexcount = N, tail = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_010_Save_1Record_Success(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Save 1 record fail
 * Result test: successfully, indexcount = 1, head = N, tail = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_011_Save_1Record_Fail(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Save full one sector.
 * Result test: successfully, indexcount = N, head = N, tail = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_012_Save_Full_1Sector(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Save full 2 sector
 * Result test: successfully, indexcount = 2N, head = 2N, tail = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_013_Save_Full_2Sector(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Save full 3 sector
 * Result test: successfully, indexcount = 3N, head = 0, tail = 1N
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_014_Save_Full_3Sector(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Save full 4 sector
 * Result test: successfully, indexcount = 4N, head = N, tail = 2N
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_015_Save_Full_4Sector(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Get data when has no data in memory
 * Result test: status none
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_016_Get_Empty(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Get data when has 1 record in memory
 * Result test: successfully, compare data successfully
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_017_Get_Has1Record(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Get data when has N record in memory (full 1 sector)
 * Result test: successfully, compare data successfully
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_018_Get_HasNRecord(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Get data when has N record in memory but CRC fail
 * Result test: get data fail
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_019_Get_HarNRecord_failCRC(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Pop data when has no data in memory
 * Result test: get data fail, return status None
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_020_Pop_Empty(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Pop data when has 1 record in memory
 * Result test: Pop1: successfully, head = 1, tail = 1,pop2: status None
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_021_Pop_Has1Record(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Pop data when has N record in memory (full 1 sector)
 * Result test: Pop data successfully
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_022_Pop_HasNRecord(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Pop data when has N record but CRC fail
 * Result test: Popdata successfully at tail = 1 and increase tail = 2
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_023_Pop_HarNRecord_failCRC(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Get latest valid data when has no data in memory
 * Result test: Get latest unsuccessfully, return status NONE
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_024_GetLatest_NoData(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Get latest valid data when has 1 record.
 * Result test: Get latest data and compare data successfully, head = 1, tail = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_025_GetLatest_1Record(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Get latest valid data successfully.
 * Result test: Get latest data and compare data successfully, head = N, tail = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_026_GetLatest_Success(dbDataType_t DataType);
/**
 * @brief This function use for  Unit Test , test case Get latest valid data when fail CRC
 * Result test:  Get latest data and compare data successfully, head = N-2, tail = 0
 * @param input dbDataType_t : type of data in put (struct 8, 9, 10, 11 bytes)
 * @param output None
 */
void DL_UT_027_GetLatest_Fail2CRC(dbDataType_t DataType);
#endif /* __DATALOGGER_UT_H__ */
