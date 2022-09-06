#ifndef _RS232_SVR_H_
#define _RS232_SVR_H_

#include "typedef.h"
#include "uip.h"
#include "resolver.h"
#include "lib/sys_tick.h"
#include "tcp_ip_task.h"
#include "lib/sys_time.h"
typedef struct {
	DATE_TIME time;
	float csqValue;
	float vin;
}ALARM_TYPE;

extern ALARM_TYPE fireAlarmRecord;

extern uint32_t gprsDataSending;
extern Timeout_Type timeTransferData;
extern uint8_t rpCloseSocketFlag;
extern struct uip_conn *rpConn;
extern TCP_STATE_TYPE RS232_SVR_Manage(void);
extern void RS232_SVR_Reset(void);
extern void RS232_SVR_Callback(void);
extern void RS232_SVR_Init(uint32_t priority);
extern void vReporterTask(void *pvParameters);

#endif
