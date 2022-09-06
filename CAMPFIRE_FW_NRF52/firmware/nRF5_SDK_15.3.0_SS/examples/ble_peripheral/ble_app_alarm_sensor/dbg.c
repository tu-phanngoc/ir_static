#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "app_config_task.h"
#include "lib/sys_time.h"
#include "nrf_log.h"

extern RINGBUF configTaskRingBuf;
LOG_TYPE logSource = ALL_LOG;

uint32_t  DbgCfgPrintf(uint8_t type_log,const uint8_t *format, ...)
{
	static  uint8_t  buffer[512];
	uint32_t len = 0,i;
	__va_list     vArgs;		    
	va_start(vArgs, format);
	if(type_log != GSM_AT_CMD_LOG)
		len = sprintf((char *)buffer,"\r\nUTC:%04d/%02d/%02d %02d:%02d:%02d #",sysTime.year,sysTime.month,sysTime.mday,sysTime.hour,sysTime.min,sysTime.sec);
	len += vsprintf((char *)&buffer[len], (char const *)format, vArgs);
	va_end(vArgs);
	if(type_log == logSource || logSource == ALL_LOG)
	{
		NRF_LOG_INFO ("%s",(uint32_t)buffer);
	}
	return 0;
}

