#ifndef __DB_H__
#define __DB_H__

#include <stdint.h>
#include "lib/sys_time.h"
#include "system_config.h"
#include "lib/sys_time.h"
#include "sst25.h"
/*
* log db layout:
* year_month/mday/hour.log		speed log every second
*/

#define LOG_WRITE_BYTES 4
#define RINGLOG_MAX 	(LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD))
#define DB_MCU_FLASH_ADDR_BASE 0x08030000


typedef struct __attribute__((packed)){
	uint32_t time; //
	uint32_t gpsLat;// kinh do
	uint32_t gpsLon;// vi do
	float gpsHdop;//  sai so trong don vi met
	float mileage; // so km hien thi tren dong ho do cua xe
	
	uint16_t gpsSpeed;// toc do
	uint16_t pulseSpeed;// toc do
	
	uint16_t gpsDir;// huong di chuyen
	uint16_t engineRPM; //
	
	uint16_t fuel1; //
	uint16_t fuel2;//
	
	char ns;
	char ew;
	uint8_t fuelTemper;
	uint8_t IOStatus;// BIT0=ACC | BIT1=DOOR | BIT2=AIRCON | BIT3=PWR | BIT4=CHUYEN DONG BAT THUONG | BIT5=MAT GPS | BIT6 = QUA TOC DO | BIT7 = SOS;
	
	uint8_t serverSent;
	uint8_t driverIndex;
	uint8_t msgType;
	uint8_t crc;
}MSG_STATUS_RECORD;

typedef struct __attribute__((packed)){
	uint32_t time;
	uint8_t speed[30];
	uint8_t sent;
	uint8_t crc;
}MSG_SPEED_RECORD;

extern MSG_STATUS_RECORD	logRecord,newestLog;
extern MSG_SPEED_RECORD	speedLogRecord;
typedef struct __attribute__((packed)){
	uint16_t head;
	uint16_t tail;
	uint16_t cnt;
	uint16_t 	crc;
}LOG_RING_TYPE;

//__packed union
//{
//		uint8_t  u8[4];
//		uint16_t u16[2];
//		uint32_t u32;
//} u32_u16_u8;  

typedef struct __attribute__((packed)){
	uint32_t value;
	uint32_t oldValue;
	uint16_t cnt;
	uint16_t crc;
}DB_U32;

extern uint32_t powerRemoveKey;
extern LOG_RING_TYPE ringLog;
extern LOG_RING_TYPE speedRingLog;
extern uint32_t flashInUse;


//flash log
uint8_t DB_ParaCheckErr(MSG_STATUS_RECORD *log);
int8_t DB_LoadEndLog(MSG_STATUS_RECORD *log);

int8_t DB_SaveLog(MSG_STATUS_RECORD *log);
int8_t DB_LoadLog(MSG_STATUS_RECORD *log,LOG_RING_TYPE *r);
int8_t DB_LoadNextLog(MSG_STATUS_RECORD *log);
void DB_RingLogReset(void);
void DB_RingLogSave(void);
void DB_RingLogNext(void);

uint8_t DB_ParaCheckErrSpeed(MSG_SPEED_RECORD *log);
int8_t DB_LoadEndLogSpeed(MSG_SPEED_RECORD *log);
void DB_InitLog(MSG_STATUS_RECORD *log,MSG_SPEED_RECORD *logSpeed);
int8_t DB_SaveLogSpeed(MSG_SPEED_RECORD *log);
int8_t DB_LoadLogSpeed(MSG_SPEED_RECORD *log,LOG_RING_TYPE *r);
int8_t DB_LoadNextLogSpeed(MSG_SPEED_RECORD *log);
void DB_RingLogResetSpeed(void);
void DB_RingLogSaveSpeed(void);
void DB_RingLogNextSpeed(void);

uint8_t DbCalcCheckSum(uint8_t *buff, uint32_t length);
uint16_t DbCalcCheckSum16(uint8_t *buff, uint16_t length);
void DB_U32Save(DB_U32 *dbu32,uint32_t addr);
void DB_U32Load(DB_U32 *dbu32,uint32_t addr);
uint32_t DB_FloatToU32(double lf);
float DB_U32ToFloat(uint32_t *u32);
int8_t DB_SpeedSaveLog(MSG_SPEED_RECORD *log);
int8_t DB_LoadNextSpeed(MSG_SPEED_RECORD *log);

#endif

