#ifndef __DBG_H__
#define __DBG_H__

#include "stdint.h"


typedef enum{
	NO_LOG = 0,
	TCP_IP_LOG,
	GSM_AT_CMD_LOG,
	GPS_LOG,
	ALL_LOG,
}LOG_TYPE;



uint32_t  DbgCfgPrintf(uint8_t type_log,const uint8_t *format, ...);

#endif
