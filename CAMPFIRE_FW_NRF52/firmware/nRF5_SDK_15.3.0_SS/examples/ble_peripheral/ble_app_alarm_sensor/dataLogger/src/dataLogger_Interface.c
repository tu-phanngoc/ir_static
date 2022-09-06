/**************************************************************************************************
 *
 * Copyright (C) 2019
 *
 *************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "dataLogger_UserPort.h"
#include "nrf_log.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define  NUMBER_CRC_2BYTE                   2
#define  LOG_WRITE_BYTES                    4
#define  INVALID_VALUE                      0xFFFF
#define  LOGRING_INDEX_SIZE                 12 /* define 12 because sizeof(logRingIndex_t) = 12 */
#define  LOGRING_INDEX_PER_SECTOR           (MEMORY_SECTOR_SIZE / LOGRING_INDEX_SIZE)
#define  LOGRING_INDEX_SIZE_WITHOUT_CRC     10 /* LOGRING_INDEX_SIZE - NUMBER_CRC_2BYTE */
typedef struct
{
    uint16_t ringHead;          /**< ringHead: index of latest data to write*/
    uint16_t ringTail;          /**< ringTail: index of oldest data stored in memory*/
    uint32_t ringIndexCount;    /**< This count The number Data record in memory*/
    uint16_t ringReserve;       /**< Reserve data*/
    uint16_t ringCRC;           /**< The CRC checksum to validate logRing struct*/
} logRingIndex_t;
typedef struct
{
    const dataLoggerConfig_t* pDataLoggerConfig; /**< address of user data logger configuration*/
    uint32_t lengthDataDummyCrc;                 /**< length of record after adding dummy and CRC */
    uint32_t startAddressIndex;                  /**< The start address index of record in memory */
    uint32_t startAddressData;                   /**< The start address data of record in memory */
    uint32_t maxDataRecordNumber;                /**< the number DataRecord to record data*/
    uint32_t ringAddressIndex;                   /**< The index address to read Index data */
    logRingIndex_t logRingIndex;                 /**< The Number sector to save data* */
    bool needToGetDataTypeInfo;                  /**< This variable is in order to chect if need to get info or not */
} dataLoggerHandle_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern const dataLoggerConfig_t dataLoggerConfigTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG];
dataLoggerHandle_t dataLoggerHandleTable[MAX_DB_MEMORY_DATA_TYPE_CONFIG];

static uint16_t getNewDataStructLength(uint16_t length);
static dataLoggerHandle_t* getHandleInfo(dbDataType_t dbDataType);
static const dataLoggerConfig_t* getConfigInfo(dbDataType_t dbDataType);
static uint16_t calculateDataChecksum(uint8_t* pbuff, uint16_t length);
static uint32_t dbGetCurrentAddress(uint32_t baseAddress, uint16_t recordIndex, uint16_t lenghDataWithCRC);
static dataLoggerStatus_t dbCRCCheckValidData(dbDataType_t dbDataType, void* pdbDataItem);
static dataLoggerStatus_t dbRingLogSave(dbDataType_t dbDataType);
static dataLoggerStatus_t dbGetDataLogTail(dbDataType_t dbDataType, void* pdbDataItem);
static bool isDataWritten(logRingIndex_t* pDBLogRingIdx);
static uint16_t searchLatestIndexSector(dbDataType_t dbDataType);
static dataLoggerStatus_t dbShiftRingLogTail(dataLoggerHandle_t* pHandleInfo, logRingIndex_t* pDBLogRingIdx);
static dataLoggerStatus_t dbShiftRingLogHead(dataLoggerHandle_t* pHandleInfo, logRingIndex_t* pDBLogRingIdx);
static dataLoggerStatus_t getDataTypeInfo(dbDataType_t dbDataType);

/*!
 * @brief To get lengh of new data structure after added dummy and CRC
 *
 */
static uint16_t getNewDataStructLength(uint16_t length)
{
    /* LOG_WRITE_BYTES = 4
    * numberByteDummy: number byte add into buffer to NewLengh%4 == 0
    * Number byte Dummy = LOG_WRITE_BYTES -  remainder of ((lengh & CRC)/4) */
    uint16_t numberByteDummy = LOG_WRITE_BYTES - ((length + NUMBER_CRC_2BYTE) % LOG_WRITE_BYTES);
    return (length + numberByteDummy + NUMBER_CRC_2BYTE);
}

/*!
 * @brief This function to get Memory Handle Infomation follow DataType
 *
 */
static dataLoggerHandle_t* getHandleInfo(dbDataType_t dbDataType)
{
    if (dbDataType < MAX_DB_MEMORY_DATA_TYPE_CONFIG)
    {
        return &dataLoggerHandleTable[dbDataType];
    }
    return NULL;
}

/*!
 * @brief This function to get Memory Config Infomation follow DataType
 *
 */
static const dataLoggerConfig_t* getConfigInfo(dbDataType_t dbDataType)
{
    if (dbDataType < MAX_DB_MEMORY_DATA_TYPE_CONFIG)
    {
        return &dataLoggerConfigTable[dbDataType];
    }
    return NULL;
}

/*!
 * @brief  Calculate lengh of data with Number byte dummy and 2 byte CRC
 *
 */
static uint16_t calculateDataChecksum(uint8_t* pbuff, uint16_t length)
{
    return dataLoggerCheckSumCRC16(pbuff, length);
}

/*!
 * @brief Get current address depend on sector
 *
 */
static uint32_t dbGetCurrentAddress(uint32_t baseAddress, uint16_t recordIndex, uint16_t lenghDataWithCRC)
{
    uint16_t numberDataRecordPerSector = MEMORY_SECTOR_SIZE / lenghDataWithCRC;
    uint32_t addressOfSector = baseAddress + ((recordIndex / numberDataRecordPerSector) * MEMORY_SECTOR_SIZE);
    uint32_t currentAddress = addressOfSector + ((recordIndex % numberDataRecordPerSector) * lenghDataWithCRC);
    return currentAddress;
}

/*!
 * @brief Check the CRC validation of a data record
 *
 */
static dataLoggerStatus_t dbCRCCheckValidData(dbDataType_t dbDataType, void* pdbDataItem)
{
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
    uint16_t u16Crc = 0;
		if(pHandleInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    uint16_t lenthExcludeCRC = pHandleInfo->lengthDataDummyCrc - NUMBER_CRC_2BYTE;

    /* Read out CRC of [record] (the last 2-byte) */
    u16Crc =*(uint16_t*) ((uint8_t*) pdbDataItem + lenthExcludeCRC);
    /* Compare with calculated result checksum of data block */
    if (u16Crc == calculateDataChecksum((uint8_t*) pdbDataItem,lenthExcludeCRC))
    {
        return DATA_LOGGER_STT_SUCCESS;
    }
    return DATA_LOGGER_STT_FAIL;
}

/*!
 * @brief Save ring log structure
 *
 */
static dataLoggerStatus_t dbRingLogSave(dbDataType_t dbDataType)
{
    dataLoggerStatus_t dbWriteStatus;
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
		if(pHandleInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    const dataLoggerConfig_t* pConfigInfo = pHandleInfo->pDataLoggerConfig;
		if(pConfigInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    logRingIndex_t* pDBLogRingIdx = &pHandleInfo->logRingIndex;
    logRingIndex_t tempRingLog;
    uint16_t tryCnt = NUMBER_RETRY;
    uint16_t ringlogCrc = 0;
    uint16_t oldSector = 0;
    uint16_t currentSector = 0;
    uint32_t currentAddress = 0;
    uint16_t sectorErase = 0;

    /* Increase IndexCount before save LogRing */
    pDBLogRingIdx->ringIndexCount++;
    /* Calculate CRC of current ringLog */
    pDBLogRingIdx->ringCRC = calculateDataChecksum((uint8_t*) pDBLogRingIdx, LOGRING_INDEX_SIZE_WITHOUT_CRC);

    while (tryCnt != 0)
    {
        oldSector = pHandleInfo->ringAddressIndex / LOGRING_INDEX_PER_SECTOR;
        pHandleInfo->ringAddressIndex++;
        currentSector = pHandleInfo->ringAddressIndex / LOGRING_INDEX_PER_SECTOR;
        if (currentSector != oldSector)
        {
            if (currentSector == NUMBER_SECTOR_INDEX)
            {
                pHandleInfo->ringAddressIndex = 0;
                currentSector = 0;
            }
            sectorErase = pConfigInfo->memoryIndexSector + currentSector;
            /* Erase 1 sector */
            if (dataLoggerDriver_Erase(sectorErase, 1) != DATA_LOGGER_STT_SUCCESS)
            {
                /* TODO: Need to find solution to replace sector when erase fail */
                return DATA_LOGGER_STT_FAIL;
            }
        }
        /* Write ringLog and readout, checking write operation. If successful, increase counter
        * to next ringLog, else re-write
        */
        currentAddress = dbGetCurrentAddress(pHandleInfo->startAddressIndex, pHandleInfo->ringAddressIndex, LOGRING_INDEX_SIZE);
        dbWriteStatus = dataLoggerDriver_Write((uint8_t*) pDBLogRingIdx, currentAddress, LOGRING_INDEX_SIZE);
        if (dbWriteStatus == DATA_LOGGER_STT_SUCCESS)
        {
            if (dataLoggerDriver_Read(currentAddress, LOGRING_INDEX_SIZE, (uint8_t*) &tempRingLog) == DATA_LOGGER_STT_SUCCESS)
            {
                ringlogCrc = calculateDataChecksum((uint8_t*) &tempRingLog, LOGRING_INDEX_SIZE_WITHOUT_CRC);
                if (ringlogCrc == tempRingLog.ringCRC)
                {
                    /* Read back and compare data*/
                    if (memcmp(&tempRingLog, pDBLogRingIdx, LOGRING_INDEX_SIZE) == 0)
                    {
                        return DATA_LOGGER_STT_SUCCESS;
                    }
                }
            }
        }
        tryCnt--;
    }
    return DATA_LOGGER_STT_FAIL;
}

/*!
 * @brief This function to get data record at current tailRing in memory.
 *
 */
static dataLoggerStatus_t dbGetDataLogTail(dbDataType_t dbDataType, void* pdbDataItem)
{
    uint8_t dataLoggerRAMBuffer[MAX_OF_STRUCT_DATA_LOG];
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
		if(pHandleInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    const dataLoggerConfig_t* pConfigInfo = pHandleInfo->pDataLoggerConfig;
		if(pConfigInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    uint32_t currentAddress = 0;
    logRingIndex_t* pDBLogRingIdx = &pHandleInfo->logRingIndex;
    /* Allocate a temp memory pool with pdbDataItemWithCRC struct size */
    uint8_t* pdbDataItemWithCRC = dataLoggerRAMBuffer;

    if (pdbDataItem == NULL)
    {
        return DATA_LOGGER_STT_FAIL;
    }
    currentAddress = dbGetCurrentAddress(pHandleInfo->startAddressData, pDBLogRingIdx->ringTail, pHandleInfo->lengthDataDummyCrc);
    /* Read out [Record] at current tailRing */
    if (dataLoggerDriver_Read(currentAddress, pHandleInfo->lengthDataDummyCrc,
            (uint8_t*) pdbDataItemWithCRC) == DATA_LOGGER_STT_SUCCESS)
    {
         /* Check valid [Record], if successful, load to pdbDataItem */
        if (dbCRCCheckValidData(dbDataType, pdbDataItemWithCRC) == DATA_LOGGER_STT_SUCCESS)
        {
            memcpy(pdbDataItem, pdbDataItemWithCRC, pConfigInfo->memoryLength);
            return DATA_LOGGER_STT_SUCCESS;
        }
    }
    return DATA_LOGGER_STT_FAIL;
}

/*!
 * @brief this function to check has nodata in sector
 *
 */
static bool isDataWritten(logRingIndex_t* pDBLogRingIdx)
{
    uint8_t index = 0;
    uint8_t* pData = (uint8_t*)pDBLogRingIdx;

    for(index = 0; index < LOGRING_INDEX_SIZE; index++)
    {
        if(pData[index]!=0xFF)
        {
            return true;
        }
    }
    return false;
}

/*!
 * @brief this function to get lastest Index Sector Valid
 *
 */
static uint16_t searchLatestIndexSector(dbDataType_t dbDataType)
{
    uint16_t tempCrc = 0;
    uint16_t indexSector = 0;
    uint16_t offset = 0;
    logRingIndex_t logRingTemp;
    uint16_t latestIndexSector = 0;
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx;

    /* Init all value to handle save memory */
    pHandleInfo->pDataLoggerConfig = getConfigInfo(dbDataType);
    pDBLogRingIdx = &pHandleInfo->logRingIndex;
    for(indexSector = 0; indexSector < NUMBER_SECTOR_INDEX; indexSector++)
    {
        /* Scan all sector storage area to find last valid sector */
        for (offset = 0; offset < MEMORY_SECTOR_SIZE; offset += LOGRING_INDEX_SIZE)
        {
            if (dataLoggerDriver_Read(pHandleInfo->startAddressIndex + (MEMORY_SECTOR_SIZE * indexSector) + offset,
                    LOGRING_INDEX_SIZE, (uint8_t*) &logRingTemp) != DATA_LOGGER_STT_SUCCESS)
            {
                return INVALID_VALUE;
            }
            tempCrc = calculateDataChecksum((uint8_t*) &logRingTemp, LOGRING_INDEX_SIZE_WITHOUT_CRC);
            if ((tempCrc == logRingTemp.ringCRC)
                    && (logRingTemp.ringHead < pHandleInfo->maxDataRecordNumber)
                    && (logRingTemp.ringTail < pHandleInfo->maxDataRecordNumber))
            {
                /* Find ringIndexCount max value : it mean this sector is latest index sector*/
                if (logRingTemp.ringIndexCount < pDBLogRingIdx->ringIndexCount)
                {
                    return latestIndexSector;
                }
                else
                {
                    *pDBLogRingIdx = logRingTemp;
                    latestIndexSector = indexSector;
                }
                break;
            }
            else
            {
                /* If DataWriten == false: mean next area don't have data , completed searching */
                if (isDataWritten(&logRingTemp) == false)
                {
                    break;
                }
            }
        }
    }
    return latestIndexSector;
}

/*!
 * @brief this function to increase RingLog tail
 *
 */
static dataLoggerStatus_t dbShiftRingLogTail(dataLoggerHandle_t* pHandleInfo, logRingIndex_t* pDBLogRingIdx)
{
    if (pDBLogRingIdx->ringTail == pDBLogRingIdx->ringHead)
    {
        return DATA_LOGGER_STT_FAIL;
    }

    pDBLogRingIdx->ringTail++;

    if (pDBLogRingIdx->ringTail >= pHandleInfo->maxDataRecordNumber)
    {
        pDBLogRingIdx->ringTail = 0;
    }

    return DATA_LOGGER_STT_SUCCESS;
}
/*!
 * @brief this function to increase RingLog head
 *
 */
static dataLoggerStatus_t dbShiftRingLogHead(dataLoggerHandle_t* pHandleInfo, logRingIndex_t* pDBLogRingIdx)
{
    if (pDBLogRingIdx->ringTail == pDBLogRingIdx->ringHead)
    {
        return DATA_LOGGER_STT_FAIL;
    }
    if (pDBLogRingIdx->ringHead == 0)
    {
        pDBLogRingIdx->ringHead = pHandleInfo->maxDataRecordNumber - 1;
    }
    else
    {
        if (pDBLogRingIdx->ringTail != pDBLogRingIdx->ringHead)
        {
            pDBLogRingIdx->ringHead--;
        }
        else
        {
            return DATA_LOGGER_STT_FAIL;
        }
    }
    return DATA_LOGGER_STT_SUCCESS;
}

/*!
 * @brief This function is to get the information of a data type.
 *
 */
static dataLoggerStatus_t getDataTypeInfo(dbDataType_t dbDataType)
{
//	NRF_LOG_INFO("getDataTypeInfo..........");
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
		if(pHandleInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    uint16_t tempCrc = 0;
    uint16_t indexRingRecord = 0;
    logRingIndex_t logRingTemp;
    const dataLoggerConfig_t* pConfigInfo;
    logRingIndex_t*pDBLogRingIdx;
    uint16_t dataRecordPerSector = 0;
    uint16_t latestIndexSector = 0;
    uint32_t currentAddress = 0;
    uint16_t currentRingAddressIndex = 0;

    /* Init all value to handle save memory */
    pHandleInfo->pDataLoggerConfig = getConfigInfo(dbDataType);
    pConfigInfo = pHandleInfo->pDataLoggerConfig;
    pDBLogRingIdx = &pHandleInfo->logRingIndex;
    /* Calc lengh of Data by add 2 byte CRC & x Dummy byte
    * (condition: pHandleInfo->lengthDataDummyCrc%4=0) */
    pHandleInfo->lengthDataDummyCrc = getNewDataStructLength(pConfigInfo->memoryLength);
    /* number raw data for each sector */
    dataRecordPerSector = MEMORY_SECTOR_SIZE / pHandleInfo->lengthDataDummyCrc;
    /* maxDataRecordNumber:
    * It must be calc number ring in each sector and sumarize number ring of all sector */
    pHandleInfo->maxDataRecordNumber = dataRecordPerSector*pConfigInfo->memoryNumberSector;
    /* startAddressIndex:
    * to save Index data depend on number sector & FLASH base address */
    pHandleInfo->startAddressIndex = pConfigInfo->memoryIndexSector
                                            * MEMORY_SECTOR_SIZE + MEMORY_BASE_ADDR;
    /* startAddressData: address to save data depend on number sector & FLASH base address */
    pHandleInfo->startAddressData =  pConfigInfo->memoryDataSector*MEMORY_SECTOR_SIZE
                                            + MEMORY_BASE_ADDR;
//		NRF_LOG_INFO("getDataTypeInfo: pConfigInfo %d, pDBLogRingIdx: %d, pHandleInfo: %d, pHandleInfo->lengthDataDummyCrc: %d..........", 
//		pConfigInfo, pDBLogRingIdx, pHandleInfo, pHandleInfo->lengthDataDummyCrc);
    /* Check pointer condition & Check if memoryLength of pdbDataItem is N* LOG_WRITE_BYTES */
    if ((pConfigInfo == NULL) || (pDBLogRingIdx == NULL)
        || (pHandleInfo->lengthDataDummyCrc % LOG_WRITE_BYTES)||(pHandleInfo==NULL)
        || (pHandleInfo->lengthDataDummyCrc > MAX_OF_STRUCT_DATA_LOG))
    {
//			NRF_LOG_INFO("...DATA_LOGGER_STT_FAIL..........");
        return DATA_LOGGER_STT_FAIL;
    }
		
    /* Initialize ringLog and pdbDataItem data */
    pDBLogRingIdx->ringHead = 0;
    pDBLogRingIdx->ringTail = 0;
    pDBLogRingIdx->ringReserve = 0;
    pDBLogRingIdx->ringCRC = 0;
    pDBLogRingIdx->ringIndexCount = 0;
    pHandleInfo->ringAddressIndex = 0 ;

    latestIndexSector = searchLatestIndexSector(dbDataType);
    if (latestIndexSector != INVALID_VALUE)
    {
        indexRingRecord = LOGRING_INDEX_PER_SECTOR;
        do
        {
            indexRingRecord--;
            currentRingAddressIndex = (LOGRING_INDEX_PER_SECTOR * latestIndexSector) + indexRingRecord;

            /* Scan all data in sector to find last valid ringLog */
            currentAddress = dbGetCurrentAddress(pHandleInfo->startAddressIndex, currentRingAddressIndex, LOGRING_INDEX_SIZE);
            if (dataLoggerDriver_Read(currentAddress, LOGRING_INDEX_SIZE, (uint8_t*) &logRingTemp)!= DATA_LOGGER_STT_SUCCESS)
            {
                return DATA_LOGGER_STT_FAIL;
            }
            if (isDataWritten(&logRingTemp) == true)
            {
                tempCrc = calculateDataChecksum((uint8_t*) &logRingTemp, LOGRING_INDEX_SIZE_WITHOUT_CRC);
                if ((tempCrc == logRingTemp.ringCRC)
                        && (logRingTemp.ringHead < pHandleInfo->maxDataRecordNumber)
                        && (logRingTemp.ringTail < pHandleInfo->maxDataRecordNumber))
                {
                    *pDBLogRingIdx = logRingTemp;
                    pHandleInfo->ringAddressIndex = currentRingAddressIndex;
                    break;
                }
            }
        } while(indexRingRecord > 0);
    }
    pHandleInfo->needToGetDataTypeInfo = false;
    return DATA_LOGGER_STT_SUCCESS;
}
/*******************************************************************************
 * Code
 ******************************************************************************/
void dataLogger_Init()
{
    uint16_t dataType = 0;
    dataLoggerHandle_t* pHandleInfo = NULL;

    for (dataType = 0; dataType < MAX_DB_MEMORY_DATA_TYPE_CONFIG; dataType++)
    {
        pHandleInfo = getHandleInfo((dbDataType_t)dataType);
        pHandleInfo->needToGetDataTypeInfo = true;
    }
}

dataLoggerStatus_t dataLogger_Save(dbDataType_t dbDataType, void* pdbDataItem)
{
//	NRF_LOG_INFO("dataLogger_Save..........");
    uint16_t tryCnt = NUMBER_RETRY;
    uint32_t currentAddress = 0;
    uint32_t tempHead = 0;
    uint32_t tailSector = 0;
    uint32_t headSector = 0;
    uint32_t headSectorOld = 0;
    uint16_t* pCRCPosition = 0;
    uint8_t dataLoggerRAMBuffer[MAX_OF_STRUCT_DATA_LOG];
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
		if(pHandleInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    const dataLoggerConfig_t* pConfigInfo = NULL;
    logRingIndex_t* pDBLogRingIdx = NULL;
    /* number data record for each sector */
    uint16_t dataRecordPerSector = 0;
    /* Allocate a temp memory pool with pdbDataItemWithCRC struct size */
    uint8_t* pdbDataItemWithCRC = dataLoggerRAMBuffer;

    if(pHandleInfo->needToGetDataTypeInfo == true)
    {
        if (getDataTypeInfo(dbDataType) != DATA_LOGGER_STT_SUCCESS)
        {
//					NRF_LOG_INFO("dataLogger_Save: DATA_LOGGER_STT_INIT_FAIL..........");
            return DATA_LOGGER_STT_INIT_FAIL;
        }
    }

    pConfigInfo = pHandleInfo->pDataLoggerConfig;
    pDBLogRingIdx = &pHandleInfo->logRingIndex;

    dataRecordPerSector = MEMORY_SECTOR_SIZE / pHandleInfo->lengthDataDummyCrc;
    while (tryCnt != 0)
    {
        /* Get current tailSector, headSector*/
        tailSector = pDBLogRingIdx->ringTail / dataRecordPerSector;
        headSectorOld = pDBLogRingIdx->ringHead / dataRecordPerSector;
        tempHead = pDBLogRingIdx->ringHead;
        /* Increase current head to save new [Record] */
        tempHead++;
        /* If reach end log sector, reset to first log sector */
        if (tempHead >= pHandleInfo->maxDataRecordNumber)
        {
            tempHead = 0;
        }
        /* Recalculate next headSector*/
        headSector = tempHead / dataRecordPerSector;
        /* If next headSector different from current headSector, erase next sector for preparing next write*/
        if (headSector != headSectorOld)
        {		
            /* Erase 1 sector */
            if (dataLoggerDriver_Erase(pConfigInfo->memoryDataSector + headSector, 1) != DATA_LOGGER_STT_SUCCESS)
            {
                /* TODO: Need to find solution to replace sector when erase fail */
                return DATA_LOGGER_STT_FAIL;
            }
        }
        /* If next headSector reach tailSector and next headRing run behind current tailRing,
        * increase tailSector to next tailSector (accept to ignore all [record]s in current
        * tailSector).
        * Recalculate tailRing:
        * If [SECTOR_SIZE (mod) memoryLength != 0], ignore [record] at the middle of two Sector,
        * set tailRing to next [record]
        **/
        if ((headSector == tailSector) && (tempHead <= pDBLogRingIdx->ringTail))
        {
            tailSector++;
            pDBLogRingIdx->ringTail = tailSector * dataRecordPerSector;
            if (pDBLogRingIdx->ringTail >= pHandleInfo->maxDataRecordNumber)
            {
                pDBLogRingIdx->ringTail = 0;
            }
        }
        /* Calculate CRC of new [record] */
        memcpy(pdbDataItemWithCRC, pdbDataItem, pConfigInfo->memoryLength);
        pCRCPosition = (uint16_t*) ((uint8_t*) pdbDataItemWithCRC + pHandleInfo->lengthDataDummyCrc - NUMBER_CRC_2BYTE);
        *pCRCPosition = calculateDataChecksum((uint8_t*) pdbDataItemWithCRC, pHandleInfo->lengthDataDummyCrc - NUMBER_CRC_2BYTE);
        currentAddress = dbGetCurrentAddress(pHandleInfo->startAddressData, pDBLogRingIdx->ringHead, pHandleInfo->lengthDataDummyCrc);
        pDBLogRingIdx->ringHead = tempHead;
        if (dataLoggerDriver_Write(((uint8_t*) pdbDataItemWithCRC), currentAddress, pHandleInfo->lengthDataDummyCrc) == DATA_LOGGER_STT_SUCCESS)
        {
            /* Read out for checking write operation */
            if (dataLoggerDriver_Read(currentAddress, pHandleInfo->lengthDataDummyCrc,(uint8_t*) pdbDataItemWithCRC) == DATA_LOGGER_STT_SUCCESS)
            {
                /* Check valid [Record], if successful, save current ringLog */
                if (dbCRCCheckValidData(dbDataType, pdbDataItemWithCRC) == DATA_LOGGER_STT_SUCCESS)
                {
                    /* Check Data and Compare data with out Dummy and CRC */
                    if(memcmp(pdbDataItemWithCRC, pdbDataItem, pConfigInfo->memoryLength) == 0)
                    {
                        if (dbRingLogSave(dbDataType) == DATA_LOGGER_STT_SUCCESS)
                        {
                            return DATA_LOGGER_STT_SUCCESS;
                        }
                        else
                        {
                            return DATA_LOGGER_STT_FAIL;
                        }
                    }
                }
            }
        }
        tryCnt--;
    }
    return DATA_LOGGER_STT_FAIL;
}

dataLoggerStatus_t dataLogger_Get(dbDataType_t dbDataType, void* pdbDataItem)
{
    uint32_t currentAddress = 0;
    uint32_t currentHead = 0;
    uint8_t dataLoggerRAMBuffer[MAX_OF_STRUCT_DATA_LOG];
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
		if(pHandleInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    const dataLoggerConfig_t* pConfigInfo = NULL;
    logRingIndex_t* pDBLogRingIdx = NULL;
    /* Allocate a temp memory pool with pdbDataItemWithCRC struct size */
    uint8_t* pdbDataItemWithCRC = dataLoggerRAMBuffer;

    if(pHandleInfo->needToGetDataTypeInfo == true)
    {
        if (getDataTypeInfo(dbDataType) != DATA_LOGGER_STT_SUCCESS)
        {
            return DATA_LOGGER_STT_INIT_FAIL;
        }
    }

    pConfigInfo = pHandleInfo->pDataLoggerConfig;
    pDBLogRingIdx = &pHandleInfo->logRingIndex;

    /* Check pointer condition # NULL */
    if (pdbDataItem == NULL)
    {
        return DATA_LOGGER_STT_FAIL;
    }
    /* Get current headRing */
    currentHead = pDBLogRingIdx->ringHead;
    if (currentHead == pDBLogRingIdx->ringTail)
    {
        /* EmptyData in memory */
        return DATA_LOGGER_STT_NONE;
    }
    /* Scan valid headRing from current head */
    if (currentHead == 0)
    {
        currentHead = pHandleInfo->maxDataRecordNumber - 1;
    }
    else
    {
        currentHead--;
    }
    currentAddress = dbGetCurrentAddress(pHandleInfo->startAddressData, currentHead,pHandleInfo->lengthDataDummyCrc);
    /* Read out [record] at current headRing */
    if (dataLoggerDriver_Read(currentAddress, pHandleInfo->lengthDataDummyCrc,
            (uint8_t*) pdbDataItemWithCRC) == DATA_LOGGER_STT_SUCCESS)
    {
        /* Check valid [Record], if successful, load to pdbDataItem */
        if (dbCRCCheckValidData(dbDataType, pdbDataItemWithCRC) == DATA_LOGGER_STT_SUCCESS)
        {
            memcpy(pdbDataItem, pdbDataItemWithCRC, pConfigInfo->memoryLength);
            return DATA_LOGGER_STT_SUCCESS;
        }
    }
    return DATA_LOGGER_STT_FAIL;
}

dataLoggerStatus_t dataLogger_Pop(dbDataType_t dbDataType,void* pdbDataItem)
{
//	NRF_LOG_INFO("dataLogger_Pop..........");
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx = NULL;
    /* Get current tailRing */
		if(pHandleInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    if(pHandleInfo->needToGetDataTypeInfo == true)
    {
        if (getDataTypeInfo(dbDataType) != DATA_LOGGER_STT_SUCCESS)
        {
            return DATA_LOGGER_STT_INIT_FAIL;
        }
    }

    pDBLogRingIdx = &pHandleInfo->logRingIndex;

    /* It mean has no data in memory */
    if (pDBLogRingIdx->ringTail == pDBLogRingIdx->ringHead)
    {
        return DATA_LOGGER_STT_NONE;
    }

    /* Get the first valid tail [Record] till the current one,
    * [dbGetDataLogTail] check if valid else next
    */
    while (dbGetDataLogTail(dbDataType, pdbDataItem) != DATA_LOGGER_STT_SUCCESS)
    {
        if(dbShiftRingLogTail(pHandleInfo, pDBLogRingIdx) == DATA_LOGGER_STT_FAIL)
        {
            return DATA_LOGGER_STT_FAIL;
        }
    }
    /*POP data: Increase tail and Save Ring Log*/
    if(dbShiftRingLogTail(pHandleInfo, pDBLogRingIdx) == DATA_LOGGER_STT_FAIL)
    {
        return DATA_LOGGER_STT_FAIL;
    }
    if (dbRingLogSave(dbDataType) != DATA_LOGGER_STT_SUCCESS)
    {
        return DATA_LOGGER_STT_FAIL;
    }
    return DATA_LOGGER_STT_SUCCESS;
}

dataLoggerStatus_t dataLogger_GetLatestValid(dbDataType_t dbDataType,void* pdbDataItem)
{
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx = NULL;
    uint16_t tempHead = 0;
		if(pHandleInfo == NULL)
		{
			return DATA_LOGGER_STT_FAIL;
		}
    if(pHandleInfo->needToGetDataTypeInfo == true)
    {
        if (getDataTypeInfo(dbDataType) != DATA_LOGGER_STT_SUCCESS)
        {
            return DATA_LOGGER_STT_INIT_FAIL;
        }
    }

    pDBLogRingIdx = &pHandleInfo->logRingIndex;
    tempHead = pDBLogRingIdx->ringHead;

    /* It mean has no data in memory */
    if (pDBLogRingIdx->ringTail == pDBLogRingIdx->ringHead)
    {
        return DATA_LOGGER_STT_NONE;
    }
    /* Get the first valid head [Record] till the tail,
    * [dataLogger_Get] check if valid else next
    */
    while (dataLogger_Get(dbDataType, pdbDataItem) != DATA_LOGGER_STT_SUCCESS)
    {
        if(dbShiftRingLogHead(pHandleInfo, pDBLogRingIdx) == DATA_LOGGER_STT_FAIL)
        {
            return DATA_LOGGER_STT_FAIL;
        }
    }
    if(tempHead != pDBLogRingIdx->ringHead)
    {
        if (dbRingLogSave(dbDataType) != DATA_LOGGER_STT_SUCCESS)
        {
            return DATA_LOGGER_STT_FAIL;
        }
    }
    return DATA_LOGGER_STT_SUCCESS;
}

uint32_t dataLogger_GetNumberSaved(dbDataType_t dbDataType)
{
    uint32_t numberSaved = 0;
    dataLoggerHandle_t* pHandleInfo = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx = NULL;

    if(pHandleInfo->needToGetDataTypeInfo == true)
    {
        if (getDataTypeInfo(dbDataType) != DATA_LOGGER_STT_SUCCESS)
        {
            return DATA_LOGGER_STT_INIT_FAIL;
        }
    }

    pDBLogRingIdx = &pHandleInfo->logRingIndex;

    if (pDBLogRingIdx->ringHead >= pDBLogRingIdx->ringTail)
    {
        numberSaved = (pDBLogRingIdx->ringHead - pDBLogRingIdx->ringTail);
    }
    else
    {
        numberSaved = ((pHandleInfo->maxDataRecordNumber) - pDBLogRingIdx->ringTail + pDBLogRingIdx->ringHead);
    }
    return numberSaved;
}
#define DATA_LOGGER_TEST    1
/*TODO: move define to pre-process*/
#if DATA_LOGGER_TEST
/*******************************************************************************
 * Code Only for Testing
 ******************************************************************************/
/**
 * @brief This function to save ringlog, using for Unit testing
 * @param input dbDataType_t : type of data in put
 * @param output dataLoggerStatus_t: status
 */
dataLoggerStatus_t testRingLogSave(dbDataType_t dbDataType);
dataLoggerStatus_t testRingLogSave(dbDataType_t dbDataType)
{
     return dbRingLogSave(dbDataType);
}
/**
 * @brief This function to get latest sector index, using for Unit testing
 * @param input dbDataType_t : type of data in put
 * @param output uint16_t: latest index sector valid
 */
uint16_t testSearchLatestSectorValid(dbDataType_t dbDataType);
uint16_t testSearchLatestSectorValid(dbDataType_t dbDataType)
{
     return searchLatestIndexSector(dbDataType);
}
/**
 * @brief This function tor clear RingLog structure, using for Unit test
 * @param input dbDataType_t : type of data in put
 * @param output None
 */
void testLogRingIndexClearStruct(dbDataType_t dbDataType);
void testLogRingIndexClearStruct(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx = &pDataLogHandle->logRingIndex;
    memset(pDBLogRingIdx,0,LOGRING_INDEX_SIZE);
}
/**
 * @brief This function to get head value in logring, using for Unit test.
 * @param input dbDataType_t : type of data in put
 * @param uint16_t head
 */
uint16_t testLogRingIndexGet_Head(dbDataType_t dbDataType);
uint16_t testLogRingIndexGet_Head(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx = &pDataLogHandle->logRingIndex;
    return pDBLogRingIdx->ringHead;
}
/**
 * @brief This function to get tail value in logring, using for Unit test.
 * @param input dbDataType_t : type of data in put
 * @param uint16_t tail
 */
uint16_t testLogRingIndexGet_Tail(dbDataType_t dbDataType);
uint16_t testLogRingIndexGet_Tail(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx = &pDataLogHandle->logRingIndex;
    return pDBLogRingIdx->ringTail;
}
/**
 * @brief This function to get index count, using for Unit test
 * @param input dbDataType_t : type of data in put
 * @param uint16_t Index count
 */
uint16_t testLogRingIndexGet_IndexCount(dbDataType_t dbDataType);
uint16_t testLogRingIndexGet_IndexCount(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx = &pDataLogHandle->logRingIndex;
    return pDBLogRingIdx->ringIndexCount;
}
/**
 * @brief This function to write fail crc for 1 record, using for Unit test
 * @param input dbDataType_t : type of data in put, ringAddressIndex: address to write fail
 * @param output uint16_t: ring Index Count
 */
uint16_t testLogRingIndexGet_WriteFailCRC(dbDataType_t dbDataType, uint16_t ringAddressIndex);
uint16_t testLogRingIndexGet_WriteFailCRC(dbDataType_t dbDataType, uint16_t ringAddressIndex)
{
    uint8_t buffer[8] = {0xF1, 0x1F, 0x00, 0xFF, 0xAA, 0xBB, 0xCC, 0xDD};
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    logRingIndex_t* pDBLogRingIdx = &pDataLogHandle->logRingIndex;
    uint16_t currentAddressIndex = dbGetCurrentAddress(pDataLogHandle->startAddressIndex, ringAddressIndex, LOGRING_INDEX_SIZE);
    dataLoggerDriver_Write((uint8_t*) buffer, currentAddressIndex, LOGRING_INDEX_SIZE-4);
    return pDBLogRingIdx->ringIndexCount;
}
/**
 * @brief This function to get ring address index, using for Unit test.
 * @param input dbDataType_t : type of data in put
 * @param output uint32_t: ring address index
 */
uint32_t testHandleRingAddressIndexGet(dbDataType_t dbDataType);
uint32_t testHandleRingAddressIndexGet(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    return pDataLogHandle->ringAddressIndex;
}
/**
 * @brief This function to set Address Index, using for Unit test.
 * @param input dbDataType_t : type of data in put, Value: Address Index
 * @param output None
 */
void testHandleRingAddressIndexSet(dbDataType_t dbDataType, uint32_t Value);
void testHandleRingAddressIndexSet(dbDataType_t dbDataType, uint32_t Value)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    pDataLogHandle->ringAddressIndex = Value;
}
/**
 * @brief This function to get first sector to save Index data
 * @param input dbDataType_t : type of data in put
 * @param output uint16_t: index sector
 */
uint16_t testHandleGetIndexSector(dbDataType_t dbDataType);
uint16_t testHandleGetIndexSector(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    const dataLoggerConfig_t* pDataLogConfig = pDataLogHandle->pDataLoggerConfig;
    return pDataLogConfig->memoryIndexSector;
}
/**
 * @brief This function to get Lengh of data record
 * @param input dbDataType_t : type of data in put
 * @param output uint16_t: lengh of data after add dummy and CRC
 */
uint16_t testHandleGetLengDataDummyCRC(dbDataType_t dbDataType);
uint16_t testHandleGetLengDataDummyCRC(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    return pDataLogHandle->lengthDataDummyCrc;
}
/**
 * @brief This function to get max of data record, use for unit test.
 * @param input dbDataType_t : type of data in put
 * @param output uint16_t: max of data record
 */
uint16_t testHandleGetMaxDataRecordNumber(dbDataType_t dbDataType);
uint16_t testHandleGetMaxDataRecordNumber(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    return pDataLogHandle->maxDataRecordNumber;
}
/**
 * @brief This function to get start address of data sector.
 * Using for testing in Unit test
 * @param input dbDataType_t : type of data in put
 * @param output uint32_t: start address of data
 */
uint32_t testHandleGetstartAddressData(dbDataType_t dbDataType);
uint32_t testHandleGetstartAddressData(dbDataType_t dbDataType)
{
    dataLoggerHandle_t* pDataLogHandle = getHandleInfo(dbDataType);
    return pDataLogHandle->startAddressData;
}
/**
 * @brief This function to get Current address base on BASE ADDR , index and lengh
 * @param baseAddress: base address, recordIndex, lenghDataWithCRC
 * @param output uint32_t: current address
 */
uint32_t testGetCurrentAddress(uint32_t baseAddress, uint16_t recordIndex, uint16_t lenghDataWithCRC);
uint32_t testGetCurrentAddress(uint32_t baseAddress, uint16_t recordIndex, uint16_t lenghDataWithCRC)
{
    return dbGetCurrentAddress(baseAddress, recordIndex, lenghDataWithCRC);
}
/**
 * @brief This function to write data to memory (called driver function).
 * @param input DataBuffer : data buffer, Address: address memory, Lengh: of data.
 * @param output None
 */
void test_dataLoggerDriver_Write(uint8_t* DataBuffer, uint32_t Address, uint32_t Lengh);
void test_dataLoggerDriver_Write(uint8_t* DataBuffer, uint32_t Address, uint32_t Lengh)
{
    dataLoggerDriver_Write(DataBuffer, Address, Lengh);
}
/**
 * @brief This function to get DataTypeInfo that using for testing
 * @param input dbDataType_t : type of data in put
 * @param output dataLoggerStatus_t : status of init
 */
dataLoggerStatus_t test_getDataTypeInfo(dbDataType_t dbDataType);
dataLoggerStatus_t test_getDataTypeInfo(dbDataType_t dbDataType)
{
    return getDataTypeInfo(dbDataType);
}
#endif
