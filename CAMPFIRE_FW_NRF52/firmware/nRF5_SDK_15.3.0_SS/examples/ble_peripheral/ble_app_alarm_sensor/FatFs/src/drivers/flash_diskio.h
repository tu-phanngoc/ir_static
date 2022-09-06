

#ifndef __FLASH_DISKIO_H__
#define __FLASH_DISKIO_H__
#include "diskio.h"
#include <stdint.h>
#include "sst25.h"
#include "ff_gen_drv.h"
extern uint8_t flashMemBuf[4096];
extern Diskio_drvTypeDef  FLASH_Driver;
DSTATUS SST25_flash_disk_initialize(BYTE lun);
DSTATUS MCU_flash_disk_initialize(void);
DSTATUS SST25_disk_status(BYTE lun);
DRESULT SST25_disk_read(BYTE lun,BYTE *buff, DWORD sector, UINT count);
DRESULT SST25_disk_write(BYTE lun,const BYTE *buff, DWORD sector, UINT count);

DRESULT SST25_disk_ioctl(BYTE lun,BYTE ctrl, void* buff);


#endif
