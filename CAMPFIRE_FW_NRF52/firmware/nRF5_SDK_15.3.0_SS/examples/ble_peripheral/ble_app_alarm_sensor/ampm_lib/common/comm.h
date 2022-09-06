
#ifndef __COMM_H__
#define __COMM_H__
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lib/ringbuf.h"

#include "app_config_task.h"
extern uint32_t  DbgCfgPrintf(uint8_t type_log,const uint8_t *format, ...);

#define PPP_Info(...)		//DbgCfgPrintf(TCP_IP_LOG,__VA_ARGS__)

#define PPP_Debug(...)	//DbgCfgPrintf(TCP_IP_LOG,__VA_ARGS__)
#define AT_CMD_Debug(...)	DbgCfgPrintf(GSM_AT_CMD_LOG,__VA_ARGS__)
#define UIP_PPP_Info(...) DbgCfgPrintf(TCP_IP_LOG,__VA_ARGS__)
#define AMPM_GSM_LIB_DBG(...) DbgCfgPrintf(TCP_IP_LOG,__VA_ARGS__)
#define MODEM_Info(...)		DbgCfgPrintf(TCP_IP_LOG,__VA_ARGS__)

extern RINGBUF *commTxRingBuf;
extern RINGBUF *commRxRingBuf;

extern  void COMM_Putc(uint8_t c);
extern  int32_t COMM_Getc(uint8_t *c);
extern  void COMM_ClearTx(void);
extern  void COMM_Puts(uint8_t *s);
extern  uint8_t COMM_CarrierDetected(void);
extern  void MODEM_RTS_Set(void);
extern  void MODEM_RTS_Clr(void);
extern  void MODEM_DTR_Set(void);
extern  void MODEM_DTR_Clr(void);
extern  void MODEM_MOSFET_On(void);
extern  void MODEM_MOSFET_Off(void);
extern  void MODEM_POWER_Set(void);
extern  void MODEM_POWER_Clr(void);
extern  void MODEM_RESET_Set(void);
extern  void MODEM_RESET_Clr(void);
extern  void MODEM_Wakeup(void);
extern  void MODEM_UartInit(uint32_t baudrate);
#endif
