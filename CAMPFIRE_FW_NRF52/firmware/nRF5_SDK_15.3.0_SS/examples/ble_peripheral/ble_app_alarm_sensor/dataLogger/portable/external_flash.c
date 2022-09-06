/*
 * external_flash.c
 *
 *  Created on:
 *      Author:
 */
#include "external_flash.h"

/***************************************************************/
/*              Definitions                                    */
/***************************************************************/

/***************************************************************/
/*              Variables                                      */
/***************************************************************/
FILE* pExternalFlash;
static uint8_t sectorBuffer[EXT_FLASH_SECTOR_SIZE];

/***************************************************************/
/*              Functions                                      */
/***************************************************************/
bool enableWriteFail = false;
externalFlashErrorCode_t externalFlash_Init(void)
{
    externalFlashErrorCode_t retVal = EXT_FLASH_SUCCESS;
    enableWriteFail = false;
    pExternalFlash = fopen ("ExternalFlash.bin", "r+b");
    if(NULL == pExternalFlash)
    {
        /* In case cannot open file or file does not existed, create new file */
        pExternalFlash = fopen ("ExternalFlash.bin", "w+b");
        if(NULL == pExternalFlash)
        {
            retVal = EXT_FLASH_ERROR;
        }
    }

    return retVal;
}

externalFlashErrorCode_t externalFlash_EraseSector(uint32_t sectorAddress)
{
    externalFlashErrorCode_t retVal = EXT_FLASH_SUCCESS;
    uint16_t index;

    /* Init 1 sector buffer */
    for(index = 0; index < EXT_FLASH_SECTOR_SIZE; index++)
    {
        sectorBuffer[index] = 0xFFU;
    }

    /* set file position */
    fseek(pExternalFlash, sectorAddress * EXT_FLASH_SECTOR_SIZE, SEEK_SET);

    fwrite(sectorBuffer, EXT_FLASH_SECTOR_SIZE, 1U, pExternalFlash);

    return retVal;
}

externalFlashErrorCode_t externalFlash_EraseMultiSector(uint32_t firstSectorAddress, uint8_t noSector)
{
    externalFlashErrorCode_t retVal = EXT_FLASH_SUCCESS;
    uint16_t index;

    /* Init 1 sector buffer */
    for(index = 0; index < EXT_FLASH_SECTOR_SIZE; index++)
    {
        sectorBuffer[index] = 0xFFU;
    }

    /* set file position */
    fseek(pExternalFlash, firstSectorAddress * EXT_FLASH_SECTOR_SIZE, SEEK_SET);

    /* write to file */
    for(index = 0; index < noSector; index++)
    {
        fwrite(sectorBuffer, EXT_FLASH_SECTOR_SIZE, 1U, pExternalFlash);
    }

    return retVal;
}

externalFlashErrorCode_t externalFlash_EraseBlock(uint32_t blockAddress)
{
    externalFlashErrorCode_t retVal = EXT_FLASH_SUCCESS;
    uint16_t index;

    for(index = 0; index < EXT_FLASH_SECTOR_SIZE; index++)
    {
        sectorBuffer[index] = 0xFFU;
    }

    /* set file position */
    fseek(pExternalFlash, blockAddress * EXT_FLASH_SECTOR_SIZE * EXT_FLASH_NO_SEC_PER_BLOCK, SEEK_SET);

    /* write to file */
    for(index = 0; index < EXT_FLASH_NO_SEC_PER_BLOCK; index++)
    {
        fwrite(sectorBuffer, EXT_FLASH_SECTOR_SIZE, 1U, pExternalFlash);
    }

    return retVal;
}

externalFlashErrorCode_t externalFlash_EraseMultiBlock(uint32_t firstBlockAddress, uint8_t noBlock)
{
    externalFlashErrorCode_t retVal = EXT_FLASH_SUCCESS;
    uint16_t index;

    for(index = 0; index < EXT_FLASH_SECTOR_SIZE; index++)
    {
        sectorBuffer[index] = 0xFFU;
    }

    /* set file position */
    fseek(pExternalFlash, firstBlockAddress * EXT_FLASH_SECTOR_SIZE * EXT_FLASH_NO_SEC_PER_BLOCK, SEEK_SET);

    /* write to file */
    for(index = 0; index < EXT_FLASH_NO_SEC_PER_BLOCK*noBlock; index++)
    {
        fwrite(sectorBuffer, EXT_FLASH_SECTOR_SIZE, 1U, pExternalFlash);
    }

    return retVal;
}

externalFlashErrorCode_t externalFlash_Write(uint8_t *data, uint32_t address, uint32_t length)
{
    externalFlashErrorCode_t retVal = EXT_FLASH_SUCCESS;
    if(enableWriteFail==false)
    {
        /* set file position */
        fseek(pExternalFlash, address, SEEK_SET);

        fwrite(data, length, 1U, pExternalFlash);
    }
    else
    {
       retVal = EXT_FLASH_ERROR;
       enableWriteFail = false;
    }
    return retVal;
}

externalFlashErrorCode_t externalFlash_Read(uint8_t *data, uint32_t address, uint32_t length)
{
    externalFlashErrorCode_t retVal = EXT_FLASH_SUCCESS;

    /* set file position */
    fseek(pExternalFlash, address, SEEK_SET);

    fread(data, length, 1U, pExternalFlash);

    return retVal;
}
