
/**
 * @file external_flash.h
 * @brief External flash abstraction file header
 */

#ifndef _EXTERNAL_FLASH_H_
#define _EXTERNAL_FLASH_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

/***************************************************************/
/*              Definitions                                    */
/***************************************************************/
#define EXT_FLASH_SECTOR_SIZE           1024U   /**< Sector size is 4096 bytes */
#define EXT_FLASH_NO_SEC_PER_BLOCK      4U       /**< 4 sector per block */

typedef enum
{
    EXT_FLASH_SUCCESS = 0U,                        /**< External flash success*/
    EXT_FLASH_ERROR,                               /**< External flash error */
} externalFlashErrorCode_t;


/**
 * @brief External flash init
 *
 * @return external flash error code.
 */
externalFlashErrorCode_t externalFlash_Init(void);

/**
 * @brief External flash erase sector
 *
 * @param[in] sectorAddress Sector address
 *
 * @return external flash error code.
 */
externalFlashErrorCode_t externalFlash_EraseSector(uint32_t sectorAddress);

/**
 * @brief External flash erase multi sectors
 *
 * @param[in] firstSectorAddress    First sector address
 * @param[in] noSector              Number of sector to be erased
 *
 * @return external flash error code.
 */
externalFlashErrorCode_t externalFlash_EraseMultiSector(uint32_t firstSectorAddress, uint8_t noSector);

/**
 * @brief External flash erase block
 *
 * @param[in] blockAddress Block address
 *
 * @return external flash error code.
 */
externalFlashErrorCode_t externalFlash_EraseBlock(uint32_t blockAddress);

/**
 * @brief External flash erase multi block
 *
 * @param[in] firstBlockAddress Block address
 * @param[in] noBlock     Number of block to be erased
 *
 * @return external flash error code.
 */
externalFlashErrorCode_t externalFlash_EraseMultiBlock(uint32_t firstBlockAddress, uint8_t noBlock);

/**
 * @brief External flash write data
 *
 * @param[in] data Write data
 * @param[in] address Address to be written
 * @param[in] address Length of data to be writen
 *
 * @return external flash error code.
 */
externalFlashErrorCode_t externalFlash_Write(uint8_t *data, uint32_t address, uint32_t length);

/**
 * @brief External flash read data
 *
 * @param[out] data read data
 * @param[in] address Address to be read
 * @param[in] address Length of data to be read
 *
 * @return external flash error code.
 */
externalFlashErrorCode_t externalFlash_Read(uint8_t *data, uint32_t address, uint32_t length);

#endif /* _EXTERNAL_FLASH_H_ */
