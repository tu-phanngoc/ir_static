/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#ifndef __AT_COMMAND_PARSER_H__
#define __AT_COMMAND_PARSER_H__
#include "stdio.h"
#include "string.h"
#include "lib/sparser.h"
#include "at_command_parser.h"
#include "ampm_gsm_sms.h"
#include "ampm_gsm_call.h"
#include "ampm_gsm_ring.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define GPRS_DATA_MAX_LENGTH	1024

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
}GSM_DATE_TIME;
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
}GSM_DATE_TIME;
#endif
	 
extern GSM_DATE_TIME sysGsmTime;
extern uint8_t ampm_AtCommandParserEnable;
extern char *commandSeparator;
extern STR_PARSER_Type AT_CmdParser;
extern const uint8_t terminateStr[7];
void AT_CmdProcessInit(void);
void AT_ComnandParser(char c);

void AT_CommandCtl(void);

#ifdef __cplusplus
}
#endif
#endif
