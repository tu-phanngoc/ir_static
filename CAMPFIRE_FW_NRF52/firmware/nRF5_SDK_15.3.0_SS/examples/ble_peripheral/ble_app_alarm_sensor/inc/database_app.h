
#ifndef __DATABASE_APP_H__
#define __DATABASE_APP_H__
#include <stdint.h>
#include "db.h"

char DB_Init(void);
char DB_CreateHierarchy(DATE_TIME time);
char DB_DataLogSave(DATE_TIME time,uint8_t *data,uint8_t len);
char DB_DataMsgSave(DATE_TIME time,uint8_t *data,uint8_t len);
#endif

