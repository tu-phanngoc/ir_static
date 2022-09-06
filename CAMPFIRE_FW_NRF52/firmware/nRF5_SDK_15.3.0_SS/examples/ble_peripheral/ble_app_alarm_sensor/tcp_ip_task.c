
#include "ampm_gsm_common.h"
#include "ppp.h"
#include "resolv.h"
#include "resolver.h"
#include "rs232_server.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "firmware_task.h"
#include "system_config.h"
uint32_t gprsDataSending = 0;
uint32_t tcpCnt = 0;
Timeout_Type tcpIpReset;
Timeout_Type tTcpDataIsBusy;
uint8_t tcpIpTryCnt;
Timeout_Type timeTransferData;


uint8_t vTcpIpTaskInit(void)
{
	InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(PPP_RESET_TIMEOUT));
	InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(TCP_BUSY_TIMEOUT));
	RS232_SVR_Init(1);
	FIRMWARE_Init(1);
	return 0;
}

uint8_t vTcpIpTask(void)
{	
      if(CheckTimeout(&tcpIpReset) == SYSTICK_TIMEOUT || tcpIpTryCnt >= 20)
      {
          tcpIpTryCnt  = 0;
          RS232_SVR_Reset();
          FIRMWARE_Reset();	
          InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(PPP_RESET_TIMEOUT));
          return 0xff;
      }
			
			if(CheckTimeout(&timeTransferData) == SYSTICK_TIMEOUT)
			{
				gprsDataSending = 0;
			}
			else 
			{
				gprsDataSending = 1;
			}
			
      RS232_SVR_Manage();
      FIRMWARE_Manage();
      return 0;
}
