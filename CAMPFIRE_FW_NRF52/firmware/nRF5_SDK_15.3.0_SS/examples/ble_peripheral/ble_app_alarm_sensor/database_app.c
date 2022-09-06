
#include "database_app.h"
#include "db.h"
#include "diskio.h"
#include "ff.h"
#include "lib/sys_tick.h"
#include "app_config_task.h"
#include "mmc_ssp.h"
#include "msc_init.h"
//#define DATABASE_DBG(...)		//{printf("\r\nUTC:%04d/%02d/%02d %02d:%02d:%02d #",sysTime.year,sysTime.month,sysTime.mday,sysTime.hour,sysTime.min,sysTime.sec); \
printf(__VA_ARGS__);}

extern uint32_t  DbgCfgPrintf(uint8_t type_log,const uint8_t *format, ...);
#define DATABASE_DBG(...) DbgCfgPrintf(ALL_LOG,__VA_ARGS__)

extern uint8_t flagSDcardErr;

char DB_Init(void)
{
	mscInit(0);
	if(flagSDcardErr)
		mscInit(0);
	return 0;
}


FRESULT scan_files_and_remove (
		DATE_TIME time,
    char* path        /* Start node to be scanned (also used as work area) */
)
{
		DATE_TIME t,t_file;
    FRESULT res;
    FILINFO fno;
		uint8_t unlink = 0;
    DIR dir;
    uint32_t i,t1,t2,t3;
    char *fn;   /* This function is assuming non-Unicode cfg. */
		char partTemp[64],tempBuf[16];
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif

		t = time;
		TIME_AddSec(&t, -10*60*60);//10 hour
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
//                sprintf(&path[i], "/%s", fn);
//                res = scan_files(path);
//                if (res != FR_OK) break;
//                path[i] = 0;
							//format lai bo nho neu xuat hien bat cu thu muc rac nao
							DATABASE_DBG("DATABASE->MMC->FORMAT\r\n");
							#ifdef _USE_MMC
							f_mkfs("0:",0,512);
							#else
							f_mkfs("0:",0,512);
							#endif
            } else {                                    /* It is a file. */
							sprintf(partTemp,"%s\\%s\r\n",path,fn);
							DATABASE_DBG("DATABASE->Found a file-> %s\\%s\r\n",path,fn);
							unlink = 0;
							if((strstr(partTemp,".LOG") != NULL) || (strstr(partTemp,".TXT") != NULL))
							{
								sscanf(partTemp,"0:\\%02d%02d%02d%s",&t1,&t2,&t3,tempBuf);
								DATABASE_DBG("DATABASE->SSCANF-------> 0:\\%02d%02d%02d%s\r\n",t1,t2,t3,tempBuf);
								if((t1 > 12)
									||  (t2 > 31)
									||  (t3 > 23)
								)
								{
									unlink = 1;
								}
								else
								{
									t_file = time;
									t_file.month = t1;
									t_file.mday = t2;
									t_file.hour = t3;
									
									if(TIME_GetSec(&t_file) < TIME_GetSec(&t))
										unlink = 1;
									//DATABASE_DBG("DATABASE->FILE_TIME->%d#%d\r\n",TIME_GetSec(&t_file),TIME_GetSec(&t));
								}
							}
							else
								unlink = 1;
							if(unlink && (strstr(partTemp,".INF") == NULL))
							{
								res = f_unlink(partTemp);
								DATABASE_DBG("DATABASE-> f_unlink:%s->%d\r\n",partTemp,res);
							}
            }
        }
        f_closedir(&dir);
    }
    return res;
}



char DB_CreateHierarchy(DATE_TIME time)
{
	DATE_TIME t;
	DIR dir;
	FRESULT res;
	char path[48],i;
	FILINFO fno;
	if(sdfs.fs_type == 0 || time.year < 2016)
	{
		return 0xff;
	}
	// remove old records
	t.year = 2017;
	t.month = 1;
	t.mday = 1;
	t.hour = 0;
	t.min = 0;
	t.sec = 0;
	if(TIME_GetSec(&time) > TIME_GetSec(&t))
	{
		t = time;
		
		strcpy((char *)path, "0:");
		scan_files_and_remove(time,path);
	}
	return 0;
}


/**
* save speed log to db
*/



char DB_DataLogSave(DATE_TIME time,uint8_t *data,uint8_t len)
{
	FIL fil;
	FRESULT res;
	FATFS *fs = &sdfs;
	char path[48];
	uint32_t i,offset,free;
	if(sdfs.fs_type == 0 || time.year < 2016)
	{	
		return 0xff;
	}
	
	if(f_getfree("0:",&free,&fs) == FR_OK)
	{
		if(free * _MAX_SS < 4*1024)
		{
			return 0xff;
		}
	}
	else
	{
		return 0xff;
	}
	
	DATABASE_DBG("DATABASE-> DB_DataLogSave\r\n");
	//sprintf(path, "0:/%02d%02d%02d.LOG", time.year%100, time.month, time.mday);
	sprintf(path, "0:/%02d%02d%02d.LOG", time.month, time.mday, time.hour);
	DATABASE_DBG("DATABASE->f_open: %s->",path);
	if((res = f_open(&fil, path, FA_READ | FA_WRITE | FA_OPEN_EXISTING)) != FR_OK)
	{
		DB_CreateHierarchy(time);
		if((res = f_open(&fil, path, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK)
		{
			if(res == FR_LOCKED)
			{
				DATABASE_DBG("DATABASE->**************FR_LOCKED**************\r\n");
				sdfs.fs_type = 0;
			}
			else
			{
				DATABASE_DBG("%d\r\n",res);
			}
			f_close(&fil);
			return 0xff;
		}
	}

	offset = f_size(&fil);
	if((res = f_lseek(&fil,offset)) == FR_OK)
	{
		DATABASE_DBG("DATABASE-> f_write:%dBytes==>%s\r\n",len,data);
		if(f_write(&fil, data, len, &i) == FR_OK && i == len)
		{
			f_close(&fil);
			return 0;
		}
		else
		{
			f_close(&fil);
		}
	}
	else
	{
		f_close(&fil);
	}
	return 0xff;
}

char DB_DataMsgSave(DATE_TIME time,uint8_t *data,uint8_t len)
{
	FIL fil;
	FRESULT res;
	FATFS *fs = &sdfs;
	char path[48];
	uint32_t i,offset,free;
	//return 0;
	if(sdfs.fs_type == 0 || time.year < 2016)
	{	
		return 0xff;
	}
	
	if(f_getfree("0:",&free,&fs) == FR_OK)
	{
		if(free * _MAX_SS < 4*1024)
		{
			return 0xff;
		}
	}
	else
	{
		return 0xff;
	}
	DATABASE_DBG("DATABASE-> DB_DataMsgSave\r\n");
	sprintf(path, "0:/%02d%02d%02d.TXT", time.month, time.mday, time.hour);
	DATABASE_DBG("DATABASE->f_open: %s->",path);
	if((res = f_open(&fil, path, FA_READ | FA_WRITE | FA_OPEN_EXISTING)) != FR_OK)
	{
		DB_CreateHierarchy(time);
		if((res = f_open(&fil, path, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK)
		{
			if(res == FR_LOCKED)
			{
				DATABASE_DBG("DATABASE->**************FR_LOCKED**************\r\n");
				sdfs.fs_type = 0;
			}
			else
			{
				DATABASE_DBG("%d\r\n",res);
			}
			f_close(&fil);
			return 0xff;
		}
	}

	offset = f_size(&fil);
	if((res = f_lseek(&fil,offset)) == FR_OK)
	{
		DATABASE_DBG("DATABASE-> f_write:%dBytes==>%s\r\n",len,data);
		if(f_write(&fil, data, len, &i) == FR_OK && i == len)
		{
			f_close(&fil);
			return 0;
		}
		else
		{
			f_close(&fil);
		}
	}
	else
	{
		f_close(&fil);
	}
	return 0xff;
}

