/**************************************************************************************************
 *
 * Copyright (C)
 *
 *************************************************************************************************/
#ifndef __DATALOGGER_USER_PORT_H__
#define __DATALOGGER_USER_PORT_H__
#include <stdint.h>
#include <stdbool.h>
//#include "external_flash.h"
#include "dataLogger_Config.h"
#include "dataLogger_Interface.h"

uint16_t dataLoggerCheckSumCRC16(const uint8_t*pdata, uint16_t size);
/*!
 *  @brief This is driver function to read data in memory.
 *
 * @param Address: address in memory
 * @param DataBuffer: pointer to buffer data to read.
 * @param Lengh: lengh of data
 *
 * @return - DB_MEM_STT_SUCCESS: if successful
 *         - DB_MEM_STT_FAIL: if failed
 */
dataLoggerStatus_t dataLoggerDriver_Read(uint32_t Address, uint32_t Lengh,uint8_t* DataBuffer);
/*!
 * @brief This is driver function to write data in memory.
 *
 * @param Address: address in memory
 * @param DataBuffer: pointer to buffer data to write.
 * @param Lengh: lengh of data
 *
 * @return - DB_MEM_STT_SUCCESS: if successful
 *         - DB_MEM_STT_FAIL: if failed
 */
dataLoggerStatus_t dataLoggerDriver_Write(uint8_t* DataBuffer, uint32_t Address, uint32_t Lengh);
/*!
 * @brief This is driver function to erase memory.
 *
 * @param sectorIndex: Index of sector in memory
 * @param numberSector: number of sector erase (it must erase follow block sector size).
 *
 * @return - DB_MEM_STT_SUCCESS: if successful
 *         - DB_MEM_STT_FAIL: if failed
 */
dataLoggerStatus_t dataLoggerDriver_Erase(uint32_t sectorIndex, uint32_t numberSector);
#endif /* __DATALOGGER_USER_PORT_H__ */
