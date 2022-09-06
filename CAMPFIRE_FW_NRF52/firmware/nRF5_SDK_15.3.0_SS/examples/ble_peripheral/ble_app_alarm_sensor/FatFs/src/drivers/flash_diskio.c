

#include "flash_diskio.h"
#include "diskio.h"
#include "string.h"
#include "ffconf.h"
#include "ff_gen_drv.h"
#include "nrf_log.h"

#define FLASH_DBG(...)  NRF_LOG_INFO(__VA_ARGS__)
#define MSC_FLASH_SECTOR_SIZE	_MAX_SS
#define FLASH_SECTOR_SIZE 0x8000


uint32_t flashFormatCnt = 0;
uint8_t flashMemBuf[4096];
DSTATUS FLASH_disk_status (
	BYTE drv		/* Physical drive number (0) */
);

Diskio_drvTypeDef  FLASH_Driver =
{
  SST25_flash_disk_initialize,
  SST25_disk_status,
  SST25_disk_read, 
  SST25_disk_write,
  SST25_disk_ioctl,
};


/*-----------------------------------------------------------------------*/
/* Get disk status                                                       */
/*-----------------------------------------------------------------------*/


DSTATUS SST25_flash_disk_initialize(BYTE lun)
{
	SST25_Init();
	return RES_OK;
}


DSTATUS SST25_disk_status(BYTE lun)
{
	return RES_OK;
}

DRESULT SST25_disk_read(BYTE lun,BYTE *buff, DWORD sector, UINT count)
{
	uint8_t trycnt = 3;
	while(trycnt--)
	{
		if(SST25_Read(sector*MSC_FLASH_SECTOR_SIZE + SST25_MSC_FLASH_START,buff,MSC_FLASH_SECTOR_SIZE) == SST25_OK)
			return RES_OK;
	}
	return RES_ERROR;
}



DRESULT SST25_disk_write(BYTE lun,const BYTE *buff, DWORD sector, UINT count)
{
	uint32_t n,flashSectorNum,flashSectorAddr,offset;
	uint8_t trycnt = 3;
	FLASH_DBG("\r\nFLASH:Write sector:%d\r\n",sector);
	while(trycnt--)
	{
		n = sector * MSC_FLASH_SECTOR_SIZE; //flash addr
		flashSectorNum = (n / SST25_SECTOR_SIZE);
		flashSectorAddr = flashSectorNum*SST25_SECTOR_SIZE;
		SST25_Read(flashSectorAddr + SST25_MSC_FLASH_START,flashMemBuf,SST25_SECTOR_SIZE);
		offset = sector * MSC_FLASH_SECTOR_SIZE - flashSectorAddr;
		memcpy(&flashMemBuf[offset],buff,MSC_FLASH_SECTOR_SIZE);
		if(SST25_Write(flashSectorAddr + SST25_MSC_FLASH_START,flashMemBuf,SST25_SECTOR_SIZE) == SST25_OK)
			return RES_OK;
	}
	return RES_ERROR;
}

DRESULT SST25_disk_ioctl(BYTE lun,BYTE ctrl, void* buff)
{
	DRESULT res;
	DWORD csize;
	
	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC:
				res = RES_OK;
			break;
			
		case GET_SECTOR_COUNT:			// get number of sectors on disk
				csize = (FLASH_SIZE_MAX - SST25_MSC_FLASH_START) / _MAX_SS;
				*(DWORD*)buff = (DWORD)csize;
				res = RES_OK;		
			break;
			
		case GET_SECTOR_SIZE : // get size of sectors on disk 
			*(WORD*) buff = _MAX_SS;
			res = RES_OK;
			break;
			
		case GET_BLOCK_SIZE : 		// get erase block size in units of sectors
				res = RES_OK;		
			break;
		default:
			res = RES_PARERR;	
	}	
	return res;
}

