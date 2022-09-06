#include "db.h"
#include "sst25.h"
#include "lib/sys_tick.h"
#include "lib/ampm_list.h"
#include "lib/ringbuf.h"
#include "database_app.h"
#include "crc16.h"

extern uint32_t  DbgCfgPrintf(uint8_t type_log,const uint8_t *format, ...);
#define DATABASE_DBG(...)		DbgCfgPrintf(__VA_ARGS__)

#define MAX_DISK_TRY		5


#define LOG_WRITE_BYTES 4
#define RINGLOG_MAX 	(LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD))
#define DB_U32_MAX_SIZE 	(4096 / sizeof(DB_U32))
#define MSG_STATUS_RECORD_NUM	200
#define MSG_SPEED_RECORD_NUM 200

extern DATE_TIME sysTime;
MSG_STATUS_RECORD	logRecord;
MSG_SPEED_RECORD	speedLogRecord;
LOG_RING_TYPE ringLog;
LOG_RING_TYPE speedRingLog;
uint32_t powerRemoveKey 																						__attribute__((at(0x2000EFF8)));
SYSTEM_RECORD systemRecord 																					__attribute__((at(0x2000EFF8 + sizeof(powerRemoveKey))));

#ifdef DB_USE_RAM
RINGBUF logRingBuff 																								__attribute__((at(0x20008004)));
RINGBUF speedRingBuff 																							__attribute__((at(0x20008004 + sizeof(RINGBUF))));
uint8_t logBuff[sizeof(MSG_STATUS_RECORD) * MSG_STATUS_RECORD_NUM]  __attribute__((at(0x20008004 + sizeof(RINGBUF)*2)));
uint8_t speedBuff[sizeof(MSG_SPEED_RECORD) * MSG_SPEED_RECORD_NUM] 	__attribute__((at(0x20008004 + sizeof(RINGBUF)*2 + sizeof(MSG_STATUS_RECORD) * MSG_STATUS_RECORD_NUM)));
//DB_U32 dataSaved[20] __attribute__((at(0x20008000 + sizeof(MSG_STATUS_RECORD) * 500 + sizeof(RINGBUF))));//total 200byte
#else
uint32_t flagNewLog = 1;
uint32_t flashInUse = 0;
#endif

uint8_t DbCalcCheckSum(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint8_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}

uint16_t DbCalcCheckSum16(uint8_t *buff, uint16_t length)
{
	return crc16_compute(buff,length,NULL);
}

void DB_InitLog(MSG_STATUS_RECORD *log,MSG_SPEED_RECORD *logSpeed)
{
	uint8_t c;
	#ifdef DB_USE_RAM
	if(logRingBuff.pt != logBuff
		|| logRingBuff.size != sizeof(logBuff)
		|| logRingBuff.size % sizeof(MSG_STATUS_RECORD)
	)
	{
	  RINGBUF_Init(&logRingBuff,logBuff,sizeof(logBuff));
	}
	else
	{
		while(RINGBUF_GetFill(&logRingBuff) % sizeof(MSG_STATUS_RECORD))
		{
				RINGBUF_Get(&logRingBuff,&c);
		}
	}
	
	if(speedRingBuff.pt != speedBuff 
		|| speedRingBuff.size != sizeof(speedBuff)
		|| speedRingBuff.size % sizeof(MSG_SPEED_RECORD)
	)
	{
	  RINGBUF_Init(&speedRingBuff,speedBuff,sizeof(speedBuff));
	}
	else
	{
		while(RINGBUF_GetFill(&speedRingBuff) % sizeof(MSG_SPEED_RECORD))
		{
				RINGBUF_Get(&speedRingBuff,&c);
		}
	}
	#else //use FLASH
	uint16_t u16temp;
	int32_t i;
	LOG_RING_TYPE logTemp;
	ringLog.head = 0;
	ringLog.tail = 0;
	ringLog.cnt = 0;
	ringLog.crc = 0;
	memset((uint8_t *)log,0,sizeof(MSG_STATUS_RECORD));
	if(sizeof(MSG_STATUS_RECORD) % LOG_WRITE_BYTES)
		while(1); //khai bao lai kieu bien
	for(i = 0;i < 4096;i += sizeof(LOG_RING_TYPE))
	{
		SST25_Read(LOG_POSITION_ADDR + i,(uint8_t *)&logTemp, sizeof(LOG_RING_TYPE));
		u16temp = DbCalcCheckSum16((uint8_t *)&logTemp,sizeof(LOG_RING_TYPE) - 2);
		if(u16temp == logTemp.crc
			&& logTemp.head < RINGLOG_MAX
			&& logTemp.tail < RINGLOG_MAX
			&& (logTemp.cnt == i)
		)
		{
			ringLog = logTemp;
			ringLog.cnt += sizeof(LOG_RING_TYPE);
		}
		else{
		 break;
		}
	}
	DB_LoadEndLog(log);
	if(DB_ParaCheckErr(log))
		memset((uint8_t *)log,0,sizeof(MSG_STATUS_RECORD));
	//LOAD LOG SPEED
	speedRingLog.head = 0;
	speedRingLog.tail = 0;
	speedRingLog.cnt = 0;
	speedRingLog.crc = 0;
	memset((uint8_t *)logSpeed,0,sizeof(MSG_SPEED_RECORD));
	if(sizeof(MSG_SPEED_RECORD) % LOG_WRITE_BYTES)
		while(1); //khai bao lai kieu bien
	for(i = 0;i < 4096;i += sizeof(LOG_RING_TYPE))
	{
		SST25_Read(LOG_SPEED_POSITION_ADDR + i,(uint8_t *)&logTemp, sizeof(LOG_RING_TYPE));
		u16temp = DbCalcCheckSum16((uint8_t *)&logTemp,sizeof(LOG_RING_TYPE) - 2);
		if(u16temp == logTemp.crc
			&& logTemp.head < RINGLOG_MAX
			&& logTemp.tail < RINGLOG_MAX
			&& (logTemp.cnt == i)
		)
		{
			speedRingLog = logTemp;
			speedRingLog.cnt += sizeof(LOG_RING_TYPE);
		}
		else{
		 break;
		}
	}
	DB_LoadEndLogSpeed(logSpeed);
	if(DB_ParaCheckErrSpeed(logSpeed))
		memset((uint8_t *)logSpeed,0,sizeof(MSG_SPEED_RECORD));
	flashInUse = 0;
	#endif
}


uint32_t DB_FloatToU32(double lf)
{
	uint32_t *u32pt;
	float f = lf,*tPt;
	tPt = &f;
	u32pt = (uint32_t *)tPt;
	return *u32pt;
}

float DB_U32ToFloat(uint32_t *u32)
{
	float *fpt;
	fpt = (float *)u32;
	return *fpt;
}


void DB_U32Load(DB_U32 *dbu32,uint32_t addr)
{
	int32_t i;
	DB_U32 db32_temp;
	uint16_t u16temp;
	uint32_t *u32temp,addrTemp;
	#ifdef DB_USE_RAM__
	db32_temp = dataSaved[addr];
	u16temp = DbCalcCheckSum16((uint8_t *)&db32_temp, sizeof(DB_U32) - 2);
	if(u16temp == db32_temp.crc)
	{
		*dbu32 = db32_temp;
		dbu32->cnt += sizeof(DB_U32);
	}
	else
	{
		memset(dbu32,0,sizeof(DB_U32));
	}
	#elif DB_USE_SST25
	
	flashInUse = 1;
	
	dbu32->oldValue = 0;
	dbu32->value = 0;
	dbu32->cnt = 0;
	dbu32->crc = 0;
	for(i = 0;i < DB_U32_MAX_SIZE*sizeof(DB_U32);i += sizeof(DB_U32))
	{
		SST25_Read(addr + i,(uint8_t *)&db32_temp, sizeof(DB_U32));
		u16temp = DbCalcCheckSum16((uint8_t *)&db32_temp, sizeof(DB_U32) - 2);
		if(u16temp == db32_temp.crc
			&& (db32_temp.cnt == i)
		)
		{
			*dbu32 = db32_temp;
			dbu32->cnt += sizeof(DB_U32);
		}
		else{
		 break;
		}
	}
	flashInUse = 0;
	#else
	dbu32->oldValue = 0;
	dbu32->value = 0;
	dbu32->cnt = 0;
	dbu32->crc = 0;
	addrTemp = DB_MCU_FLASH_ADDR_BASE + addr*FLASH_PAGE_SIZE;
	for(i = 0;i < FLASH_PAGE_SIZE;i += 4)
	{
		u32temp = (uint32_t *)(addrTemp + dbu32->cnt);
		if(*u32temp != 0xFFFFFFFF)
		{
			dbu32->value = *u32temp;
			dbu32->cnt += 4;
		}
		else{
		 break;
		}
	}
	
	for(;i < FLASH_PAGE_SIZE;i += 4)
	{
		u32temp = (uint32_t *)(addrTemp + dbu32->cnt);
		if(*u32temp != 0xFFFFFFFF) //database error
		{
			if(addrTemp < (CONFIG_AREA_START - PAGE_SIZE))
				FLASH_ErasePage(addrTemp);
			dbu32->value = 0;
			dbu32->cnt = 0;
			break;
		}
	}
	
	#endif
}

extern uint8_t FirmwareTask_IsBusy(void);

void DB_U32Save(DB_U32 *dbu32,uint32_t addr)
{
	uint32_t timeout = 10000,*temp,i;
	DB_U32 dbu32T;
	uint32_t *u32temp,addrTemp;
	#ifdef DB_USE_RAM__
	if(dbu32->oldValue != dbu32->value)
	{
		dbu32->oldValue = dbu32->value;
		dbu32->crc = DbCalcCheckSum16((uint8_t *)dbu32,sizeof(DB_U32) - 2);
		dataSaved[addr] = *dbu32;
	}
	#elif DB_USE_SST25
	
	flashInUse = 1; 
	if(dbu32->oldValue != dbu32->value)
	{
		dbu32->oldValue = dbu32->value;
		dbu32->crc = DbCalcCheckSum16((uint8_t *)dbu32,sizeof(DB_U32) - 2);
		while(1)
		{
			if((dbu32->cnt +  sizeof(DB_U32)) >= (DB_U32_MAX_SIZE*sizeof(DB_U32)))
			{
					SST25_Erase(addr,block4k);
					dbu32->cnt = 0;
					dbu32->crc = DbCalcCheckSum16((uint8_t *)dbu32,sizeof(DB_U32) - 2);
			}
			SST25_Write(addr + dbu32->cnt,(uint8_t *)dbu32,sizeof(DB_U32));
			SST25_Read(addr + dbu32->cnt,(uint8_t *)&dbu32T,sizeof(DB_U32));
			if(memcmp(&dbu32T, dbu32,sizeof(DB_U32)) != NULL)
			{
				SST25_Erase(addr,block4k);
				dbu32->cnt = 0;
				dbu32->crc = DbCalcCheckSum16((uint8_t *)dbu32,sizeof(DB_U32) - 2);
			}
			else
			{
				dbu32->cnt += sizeof(DB_U32);
				break;
			}
		}
	}
	flashInUse = 0;
	#else

	if(FirmwareTask_IsBusy())
		return;
//	__disable_irq();
	addrTemp = DB_MCU_FLASH_ADDR_BASE + addr*FLASH_PAGE_SIZE;
	dbu32->cnt = 0;
	for(i = 0;i < FLASH_PAGE_SIZE;i += 4)
	{
		u32temp = (uint32_t *)(addrTemp + dbu32->cnt);
		if(*u32temp != 0xFFFFFFFF)
		{
			dbu32->cnt += 4;
		}
		else{
		 break;
		}
	}
	
	for(;i < FLASH_PAGE_SIZE;i += 4)
	{
		u32temp = (uint32_t *)(addrTemp + dbu32->cnt);
		if(*u32temp != 0xFFFFFFFF) //database error
		{
			if(addrTemp < (CONFIG_AREA_START - PAGE_SIZE))
				FLASH_ErasePage(addrTemp);
			dbu32->cnt = 0;
			break;
		}
	}

	if((dbu32->cnt +  4) >= FLASH_PAGE_SIZE || dbu32->cnt == 0)
	{
			dbu32->cnt = 0;
		if(addrTemp < (CONFIG_AREA_START - PAGE_SIZE))
			FLASH_ErasePage(addrTemp);
	}
	temp = (uint32_t *)(addrTemp + dbu32->cnt);
	if((addrTemp + dbu32->cnt) < (CONFIG_AREA_START - PAGE_SIZE))
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,addrTemp + dbu32->cnt, dbu32->value);
	if(dbu32->value != *temp)
	{
		if(addrTemp < (CONFIG_AREA_START - PAGE_SIZE))
			FLASH_ErasePage(addrTemp);
	}
	else
	{
		dbu32->cnt += 4;
	}

//	__enable_irq();
	#endif
}


void DB_RingLogReset(void)
{
	#ifdef DB_USE_RAM
	RINGBUF_Init(&logRingBuff,logBuff,sizeof(logBuff));
	#else //use FLASH
	flashInUse = 1;
	SST25_Erase(LOG_POSITION_ADDR,block4k);
	ringLog.head = 0;
	ringLog.tail = 0;
	DB_RingLogSave();
	flashInUse = 0; 
	#endif
}


void DB_RingLogResetSpeed(void)
{
	#ifdef DB_USE_RAM
	RINGBUF_Init(&speedRingBuff,speedBuff,sizeof(speedBuff));
	#else //use FLASH
	flashInUse = 1;
	SST25_Erase(LOG_SPEED_POSITION_ADDR,block4k);
	speedRingLog.head = 0;
	speedRingLog.tail = 0;
	DB_RingLogSaveSpeed();
	flashInUse = 0; 
	#endif
}



void DB_RingLogNext(void)
{
	#ifndef DB_USE_RAM
	flashInUse = 1;
	if(ringLog.tail != ringLog.head)
	{
		ringLog.tail++;
		if(ringLog.tail >= RINGLOG_MAX)
			ringLog.tail = 0;
		DB_RingLogSave();
	}
	flashInUse = 0;
	#endif
}

void DB_RingLogNextSpeed(void)
{
	#ifndef DB_USE_RAM
	flashInUse = 1;
	if(speedRingLog.tail != speedRingLog.head)
	{
		speedRingLog.tail++;
		if(speedRingLog.tail >= RINGLOG_MAX)
			speedRingLog.tail = 0;
		DB_RingLogSaveSpeed();
	}
	flashInUse = 0;
	#endif
}

#ifndef DB_USE_RAM
void DB_RingLogSave(void)
{
	LOG_RING_TYPE ringLogT;
	flashInUse = 1; 
	ringLog.crc = DbCalcCheckSum16((uint8_t *)&ringLog,sizeof(LOG_RING_TYPE) - 2);
	while(1)
	{
		if((ringLog.cnt +  sizeof(LOG_RING_TYPE) >=  4096) )
		{
			SST25_Erase(LOG_POSITION_ADDR,block4k);
			ringLog.cnt = 0;
			ringLog.crc = DbCalcCheckSum16((uint8_t *)&ringLog,sizeof(LOG_RING_TYPE) - 2);
		}
		SST25_Write(LOG_POSITION_ADDR + ringLog.cnt,(uint8_t *)&ringLog,sizeof(LOG_RING_TYPE));
		SST25_Read(LOG_POSITION_ADDR + ringLog.cnt,(uint8_t *)&ringLogT,sizeof(LOG_RING_TYPE));
		if(memcmp(&ringLogT, &ringLog,sizeof(LOG_RING_TYPE)) != NULL)
		{
			SST25_Erase(LOG_POSITION_ADDR,block4k);
			ringLog.cnt = 0;
			ringLog.crc = DbCalcCheckSum16((uint8_t *)&ringLog,sizeof(LOG_RING_TYPE) - 2);
		}
		else
		{
			ringLog.cnt += sizeof(LOG_RING_TYPE);
			break;
		}
	}
	flashInUse = 0;
}

void DB_RingLogSaveSpeed(void)
{
	LOG_RING_TYPE ringLogT;
	flashInUse = 1; 
	speedRingLog.crc = DbCalcCheckSum16((uint8_t *)&speedRingLog,sizeof(LOG_RING_TYPE) - 2);
	while(1)
	{
		if((speedRingLog.cnt +  sizeof(LOG_RING_TYPE) >=  4096) )
		{
			SST25_Erase(LOG_SPEED_POSITION_ADDR,block4k);
			speedRingLog.cnt = 0;
			speedRingLog.crc = DbCalcCheckSum16((uint8_t *)&speedRingLog,sizeof(LOG_RING_TYPE) - 2);
		}
		SST25_Write(LOG_SPEED_POSITION_ADDR + speedRingLog.cnt,(uint8_t *)&speedRingLog,sizeof(LOG_RING_TYPE));
		SST25_Read(LOG_SPEED_POSITION_ADDR + speedRingLog.cnt,(uint8_t *)&ringLogT,sizeof(LOG_RING_TYPE));
		if(memcmp(&ringLogT, &speedRingLog,sizeof(LOG_RING_TYPE)) != NULL)
		{
			SST25_Erase(LOG_SPEED_POSITION_ADDR,block4k);
			speedRingLog.cnt = 0;
			speedRingLog.crc = DbCalcCheckSum16((uint8_t *)&speedRingLog,sizeof(LOG_RING_TYPE) - 2);
		}
		else
		{
			speedRingLog.cnt += sizeof(LOG_RING_TYPE);
			break;
		}
	}
	flashInUse = 0;
}
#endif

uint32_t DB_LogFill(void)
{
	#ifndef DB_USE_RAM
	if(ringLog.head >= ringLog.tail)
	{
		return (ringLog.head - ringLog.tail);
	}
	else
	{
	   return(LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD) - ringLog.tail + ringLog.head);
	}
	#else
	return RINGBUF_GetFill(&logRingBuff);
	#endif
}

uint32_t DB_LogFillSpeed(void)
{
	#ifndef DB_USE_RAM
	if(speedRingLog.head >= speedRingLog.tail)
	{
		return (speedRingLog.head - speedRingLog.tail);
	}
	else
	{
	   return(LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD) - speedRingLog.tail + speedRingLog.head);
	}
	#else
	return RINGBUF_GetFill(&speedRingBuff);
	#endif
}

#ifndef DB_USE_RAM
int8_t DB_LoadEndLog(MSG_STATUS_RECORD *log)
{

	uint32_t head = ringLog.head;
	MSG_STATUS_RECORD tLog;
	flashInUse = 1;
	while(1)
	{
		if(head == 0)
		{
			head = (LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD)) - 1;
		}
		else
		{
			head--;
		}
		if(head == ringLog.tail) break;
		SST25_Read(LOG_DATABASE_ADDR + head*sizeof(MSG_STATUS_RECORD),(uint8_t *)&tLog,sizeof(MSG_STATUS_RECORD));
		if(!DB_ParaCheckErr(&tLog))
		{
			*log = tLog;
			flashInUse = 0;
			return 0;
		}
	}
	flashInUse = 0;
	return 1;
}

int8_t DB_LoadEndLogSpeed(MSG_SPEED_RECORD *log)
{

	uint32_t head = speedRingLog.head;
	MSG_SPEED_RECORD tLog;
	flashInUse = 1;
	while(1)
	{
		if(head == 0)
		{
			head = (LOG_DATA_SIZE_MAX / sizeof(MSG_SPEED_RECORD)) - 1;
		}
		else
		{
			head--;
		}
		if(head == speedRingLog.tail) break;
		SST25_Read(LOG_SPEED_DATABASE_ADDR + head*sizeof(MSG_SPEED_RECORD),(uint8_t *)&tLog,sizeof(MSG_SPEED_RECORD));
		if(!DB_ParaCheckErrSpeed(&tLog))
		{
			*log = tLog;
			flashInUse = 0;
			return 0;
		}
	}
	flashInUse = 0;
	return 1;
}
#endif



uint8_t DB_ParaCheckErr(MSG_STATUS_RECORD *log)
{
	if(log->crc == DbCalcCheckSum((uint8_t *)log,sizeof(MSG_STATUS_RECORD) - 1))
		return 0;
	return 1;
}

uint8_t DB_ParaCheckErrSpeed(MSG_SPEED_RECORD *log)
{
	if(log->crc == DbCalcCheckSum((uint8_t *)log,sizeof(MSG_SPEED_RECORD) - 1))
		return 0;
	return 1;
}

#ifndef DB_USE_RAM
int8_t DB_LoadLog(MSG_STATUS_RECORD *log,LOG_RING_TYPE *r)
{
	
	MSG_STATUS_RECORD tLog;
	if(r->tail == r->head) return 0xff;
	SST25_Read(LOG_DATABASE_ADDR + r->tail*sizeof(MSG_STATUS_RECORD),(uint8_t *)&tLog,sizeof(MSG_STATUS_RECORD));
	if(!DB_ParaCheckErr(&tLog))
	{
		*log = tLog;
		return 0;
	}
	else 
		return 0xff;
}

int8_t DB_LoadLogSpeed(MSG_SPEED_RECORD *log,LOG_RING_TYPE *r)
{
	
	MSG_SPEED_RECORD tLog;
	if(r->tail == r->head) return 0xff;
	SST25_Read(LOG_SPEED_DATABASE_ADDR + r->tail*sizeof(MSG_SPEED_RECORD),(uint8_t *)&tLog,sizeof(MSG_SPEED_RECORD));
	if(!DB_ParaCheckErrSpeed(&tLog))
	{
		*log = tLog;
		return 0;
	}
	else 
		return 0xff;
}
#endif

int8_t DB_LoadNextLogSpeed(MSG_SPEED_RECORD *log)
{
	#ifdef DB_USE_RAM
	uint8_t i,*pt = (uint8_t *)log,c;
	uint32_t fill;
	fill = RINGBUF_GetFill(&speedRingBuff);
	if(fill < sizeof(MSG_SPEED_RECORD) || fill % sizeof(MSG_SPEED_RECORD))
	{
		if(fill)
		{
			while(RINGBUF_Get(&speedRingBuff,&c) == 0);//clear error data
		}
		return 0xff;
	}
	for(i = 0;i < sizeof(MSG_SPEED_RECORD);i++)
	{
		RINGBUF_Get(&speedRingBuff,&pt[i]);
	}
	#else
	uint32_t tail = speedRingLog.tail;
	flashInUse = 1;
	if(speedRingLog.tail == speedRingLog.head){
		flashInUse = 0;
		return 0xff;
	}
	while(DB_LoadLogSpeed(log,&speedRingLog) != 0)
	{
		speedRingLog.tail++;
		if(speedRingLog.tail >= RINGLOG_MAX)
			speedRingLog.tail = 0;
		if(speedRingLog.tail == speedRingLog.head){
			flashInUse = 0;
			return 0xff;
		}
	}
	if(tail != speedRingLog.tail)
		DB_RingLogSaveSpeed();
	flashInUse = 0;
	#endif
	return 0;
}

int8_t DB_LoadNextLog(MSG_STATUS_RECORD *log)
{
	#ifdef DB_USE_RAM
	uint8_t i,*pt = (uint8_t *)log,c;
	uint32_t fill;
	fill = RINGBUF_GetFill(&logRingBuff);
	if(fill < sizeof(MSG_STATUS_RECORD) || fill % sizeof(MSG_STATUS_RECORD))
	{
		if(fill)
		{
			while(RINGBUF_Get(&logRingBuff,&c) == 0);//clear error data
		}
		return 0xff;
	}
	for(i = 0;i < sizeof(MSG_STATUS_RECORD);i++)
	{
		RINGBUF_Get(&logRingBuff,&pt[i]);
	}
	if(log->crc != DbCalcCheckSum((uint8_t *)log,sizeof(MSG_STATUS_RECORD) - 1))
	{
		return 0xff;
	}
	#else
	uint32_t tail = ringLog.tail;
	flashInUse = 1;
	if(ringLog.tail == ringLog.head){
		flashInUse = 0;
		return 0xff;
	}
	while(DB_LoadLog(log,&ringLog) != 0)
	{
		ringLog.tail++;
		if(ringLog.tail >= RINGLOG_MAX)
			ringLog.tail = 0;
		if(ringLog.tail == ringLog.head){
			flashInUse = 0;
			return 0xff;
		}
	}
	if(tail != ringLog.tail)
		DB_RingLogSave();
	flashInUse = 0;
	#endif
	return 0;
}


int8_t DB_SpeedSaveLog(MSG_SPEED_RECORD *log)
{
	#ifdef DB_USE_RAM
	uint8_t i,*pt = (uint8_t *)log;
	if(RINGBUF_GetFill(&speedRingBuff) + sizeof(MSG_SPEED_RECORD) >= (speedRingBuff.size))
	{	
		return 0xff;
	}
	for(i = 0;i < sizeof(MSG_SPEED_RECORD);i++)
	{
		RINGBUF_Put(&speedRingBuff,pt[i]);
	}
	return 0;
	#else
	MSG_SPEED_RECORD tLog;
	uint8_t tryCnt = 0,err = 0;
	uint32_t u32temp,tailSector = 0,headSector = 0,headSectorOld = 0,i;
	flashInUse = 1;
	while(1)
	{
		tryCnt++;
		if(tryCnt >= 10){
			flashInUse = 0;
			return 1;
		}
		tailSector = speedRingLog.tail * sizeof(MSG_SPEED_RECORD) / SST25_SECTOR_SIZE;
		headSectorOld = speedRingLog.head * sizeof(MSG_SPEED_RECORD) / SST25_SECTOR_SIZE;
		u32temp = speedRingLog.head;
		u32temp++;
		if(u32temp >= LOG_SPEED_DATA_SIZE_MAX / sizeof(MSG_SPEED_RECORD))	
		{
			u32temp = 0;
		}
		headSector = u32temp * sizeof(MSG_SPEED_RECORD) / SST25_SECTOR_SIZE;
		if(headSector != headSectorOld)
		{
			SST25_Erase(LOG_SPEED_DATABASE_ADDR + headSector * SST25_SECTOR_SIZE,block4k);
		}
		if((headSector == tailSector) && (u32temp <= speedRingLog.tail))
		{
			tailSector++;
			speedRingLog.tail = tailSector * SST25_SECTOR_SIZE / sizeof(MSG_SPEED_RECORD);
			if((tailSector * SST25_SECTOR_SIZE) % sizeof(MSG_SPEED_RECORD))
				 speedRingLog.tail++;
			if(speedRingLog.tail >= LOG_SPEED_DATA_SIZE_MAX / sizeof(MSG_SPEED_RECORD))
				speedRingLog.tail = 0;
		}

		log->crc = DbCalcCheckSum((uint8_t *)log,sizeof(MSG_SPEED_RECORD) - 1);
		
		for(i = 0;i < sizeof(MSG_SPEED_RECORD);i += LOG_WRITE_BYTES)
		{
			if(SST25_Write(LOG_SPEED_DATABASE_ADDR + speedRingLog.head*sizeof(MSG_SPEED_RECORD) + i,(uint8_t *)log + i,LOG_WRITE_BYTES) == SST25_FAIL){
				err = 1;
				break;
			}
		}
		if(!err){
			SST25_Read(LOG_SPEED_DATABASE_ADDR + speedRingLog.head*sizeof(MSG_SPEED_RECORD),(uint8_t *)&tLog,sizeof(MSG_SPEED_RECORD));
			speedRingLog.head = u32temp;
			if(!DB_ParaCheckErrSpeed(&tLog))
			{
				DB_RingLogSaveSpeed();
				flashInUse = 0;
				return 0;
			}
		}
		else{
			err = 0;
			speedRingLog.head = u32temp;
		}
	}
	flashInUse = 0;
	return 1;
	#endif
}

int8_t DB_SaveLog(MSG_STATUS_RECORD *log)
{
	#ifdef DB_USE_RAM
	uint8_t i,*pt = (uint8_t *)log;
	log->crc = DbCalcCheckSum((uint8_t *)log,sizeof(MSG_STATUS_RECORD) - 1);
	if(RINGBUF_GetFill(&logRingBuff) + sizeof(MSG_STATUS_RECORD) >= (logRingBuff.size))
		return 0xff;
	for(i = 0;i < sizeof(MSG_STATUS_RECORD);i++)
	{
		RINGBUF_Put(&logRingBuff,pt[i]);
	}
	return 0;
	#else
	MSG_STATUS_RECORD tLog;
	uint8_t tryCnt = 0,err = 0;
	uint32_t u32temp,tailSector = 0,headSector = 0,headSectorOld = 0,i;
	flashInUse = 1;
	while(1)
	{
		tryCnt++;
		if(tryCnt >= 10){
			flashInUse = 0;
			return 1;
		}
		tailSector = ringLog.tail * sizeof(MSG_STATUS_RECORD) / SST25_SECTOR_SIZE;
		headSectorOld = ringLog.head * sizeof(MSG_STATUS_RECORD) / SST25_SECTOR_SIZE;
		u32temp = ringLog.head;
		u32temp++;
		if(u32temp >= LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD))	
		{
			u32temp = 0;
		}
		headSector = u32temp * sizeof(MSG_STATUS_RECORD) / SST25_SECTOR_SIZE;
		if(headSector != headSectorOld)
		{
			SST25_Erase(LOG_DATABASE_ADDR + headSector * SST25_SECTOR_SIZE,block4k);
		}
		if((headSector == tailSector) && (u32temp <= ringLog.tail))
		{
			tailSector++;
			ringLog.tail = tailSector * SST25_SECTOR_SIZE / sizeof(MSG_STATUS_RECORD);
			if((tailSector * SST25_SECTOR_SIZE) % sizeof(MSG_STATUS_RECORD))
				 ringLog.tail++;
			if(ringLog.tail >= LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD))
				ringLog.tail = 0;
		}

		log->crc = DbCalcCheckSum((uint8_t *)log,sizeof(MSG_STATUS_RECORD) - 1);
		
		for(i = 0;i < sizeof(MSG_STATUS_RECORD);i += LOG_WRITE_BYTES)
		{
			if(SST25_Write(LOG_DATABASE_ADDR + ringLog.head*sizeof(MSG_STATUS_RECORD) + i,(uint8_t *)log + i,LOG_WRITE_BYTES) == SST25_FAIL){
				err = 1;
				break;
			}
		}
		if(!err){
			SST25_Read(LOG_DATABASE_ADDR + ringLog.head*sizeof(MSG_STATUS_RECORD),(uint8_t *)&tLog,sizeof(MSG_STATUS_RECORD));
			ringLog.head = u32temp;
			if(!DB_ParaCheckErr(&tLog))
			{
				DB_RingLogSave();
				flashInUse = 0;
				return 0;
			}
		}
		else{
			err = 0;
			ringLog.head = u32temp;
		}
	}
	flashInUse = 0;
	return 1;
	#endif
}



