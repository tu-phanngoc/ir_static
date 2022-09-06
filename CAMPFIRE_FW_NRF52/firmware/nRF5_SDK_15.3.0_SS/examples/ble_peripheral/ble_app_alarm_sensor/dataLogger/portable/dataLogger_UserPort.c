/**************************************************************************************************
 *
 * Copyright (C) 2019
 *
 *************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "dataLogger_UserPort.h"
#include "app_datalog.h"
#include "nrf_nvmc.h"
/*******************************************************************************
 * @brief Contains User port Driver function
 ******************************************************************************/
#define POLY_CRC            0x8408
/*******************************************************************************
 * Variables
 ******************************************************************************/
 extern volatile uint32_t ms_counter;
/*******************************************************************************
 * Code
 ******************************************************************************/
uint16_t dataLoggerCheckSumCRC16(const uint8_t*pdata, uint16_t size)
{
    unsigned char i;
    unsigned int data;
    unsigned int crc = 0xffff;

    if (size == 0)
         return (~crc);
    do
    {
        for (i=0, data=(unsigned int)0xff & *pdata++;
            i < 8;
            i++, data >>= 1)
        {
            if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ POLY_CRC;
            else  crc >>= 1;
        }
    } while (--size);

    crc = ~crc;
    data = crc;
    crc = (crc << 8) | (data >> 8 & 0xff);

    return (crc);
}

dataLoggerStatus_t dataLoggerDriver_Read(uint32_t Address, uint32_t Lengh,uint8_t* DataBuffer)
{
    /*Write Driver function here*/
		ret_code_t rc = NRF_ERROR_NOT_FOUND;
    if (DataBuffer != NULL)
    {
        /*Use this function to test*/
			bool opt;
			rc = datalog_nrf_use_flash_read(Address, (uint32_t*)DataBuffer, Lengh, &opt);
    }
		if(rc == NRF_SUCCESS)
		{
			return DATA_LOGGER_STT_SUCCESS;
		}
		else
		{
			return DATA_LOGGER_STT_FAIL;
		}
		
}

dataLoggerStatus_t dataLoggerDriver_Write(uint8_t* DataBuffer, uint32_t Address,uint32_t Lengh)
{
    /*Write Driver function here*/
    /*Use this function to test*/
		ret_code_t rc = NRF_ERROR_NOT_FOUND;
		
		bool opt;
		uint32_t before = ms_counter;
	rc = datalog_nrf_use_flash_write(Address, (uint32_t*)DataBuffer , Lengh, &opt);
//		uint32_t *p_dest;
//		p_dest = (uint32_t*)Address;
//		nrf_nvmc_write_bytes(Address, DataBuffer, Lengh);
//		rc = sd_flash_write(p_dest,(uint32_t*) DataBuffer, Lengh);

		NRF_LOG_INFO("ms = %d", ms_counter - before);
    if(rc == NRF_SUCCESS)
		{
			return DATA_LOGGER_STT_SUCCESS;
		}
		else
		{
			return DATA_LOGGER_STT_FAIL;
		}
}

dataLoggerStatus_t dataLoggerDriver_Erase(uint32_t sectorIndex, uint32_t numberSector)
{
	bool flash_operation_pending;
	ret_code_t rc;
    /*Write Driver function here*/
	NRF_LOG_INFO("dataLoggerDriver_Erase %d, %d", sectorIndex, numberSector);
	uint32_t dest_addr = 0x50000 + sectorIndex * 0x1000;
//		rc = datalog_nrf_use_flash_erase(dest_addr, 1, &flash_operation_pending);

//    if (rc != NRF_SUCCESS)
//    {
//        NRF_LOG_INFO("Erasing from flash memory failed.\r\n");
//        flash_operation_pending = false;
//        return DATA_LOGGER_STT_FAIL;
//    }
    /*Use this function to test*/
//	bool flash_operation_pending;
//    externalFlash_EraseMultiSector(sectorIndex,numberSector);
//		datalog_nrf_use_flash_erase(p_dest, 1, &flash_operation_pending);
    return DATA_LOGGER_STT_SUCCESS;
}
