#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <time.h>
#include <stdint.h>
#include "m_time.h"
#include "gps/ampm_gps.h"

#define TIME_ARRAY_SIZE	(7)

#define TIME_ZONE			(+7)
#if defined ( __ICCARM__ )
#pragma pack(1) 
typedef struct {
	int16_t year;			// year with all 4-digit (2011)
	int8_t month;			// month 1 - 12 (1 = Jan)
	int8_t mday;			// day of month 1 - 31
	int8_t wday;			// day of week 1 - 7 (1 = Sunday)
	int8_t hour;			// hour 0 - 23
	int8_t min;				// min 0 - 59
	int8_t sec;				// sec 0 - 59
}DATE_TIME;
#pragma pack()
#elif defined (__CC_ARM)
typedef struct __attribute__((packed)){
	int16_t year;			// year with all 4-digit (2011)
	int8_t month;			// month 1 - 12 (1 = Jan)
	int8_t mday;			// day of month 1 - 31
	int8_t wday;			// day of week 1 - 7 (1 = Sunday)
	int8_t hour;			// hour 0 - 23
	int8_t min;				// min 0 - 59
	int8_t sec;				// sec 0 - 59
}DATE_TIME;
#endif
extern DATE_TIME sysTime;
extern DATE_TIME localTime;
extern uint32_t rtcTimeSec;
extern uint32_t timeZone;
extern uint32_t sysTimeZone;
extern uint32_t rtcUpdateFlag;
extern uint32_t rtcUpdateCnt;

void SysTimeSetup(void);
void UpdateRtcTime(uint32_t updateTimeSec);
int8_t TIME_AddSec(DATE_TIME *t, int32_t sec);
int32_t TIME_GetSec(DATE_TIME *t);
int8_t TIME_FromSec(DATE_TIME *t, int32_t sec);
int8_t TIME_FromGsm(DATE_TIME *t, DATE_TIME *time);
void time_update(uint8_t *p_time_data);

void time_get(uint8_t *p_time);
void set_sys_timezone(uint8_t timezone);
#endif

