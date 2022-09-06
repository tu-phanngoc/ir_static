#include "ampm_gsm_main_task.h"
#include "rs232_server.h"
#include "xtea.h"
#include "lib/sys_tick.h"
#include "system_config.h"
#include "led.h"
#include "tcp_ip_task.h"
#include "lib/sys_time.h"
#include "fire_alarm_message_create.h"
#include "app_config_task.h"
#include "adc_task.h"
#include "sms_task.h"
#include "firmware_task.h"

#define TIME_KEEP_CONNECT 60

#define TCPIP_ECHO_ON

#define TEST_DEVICE 0

extern uint32_t  DbgCfgPrintf(uint8_t type_log,const uint8_t *format, ...);
#define RS232_SVR_DBG(...) DbgCfgPrintf(TCP_IP_LOG,__VA_ARGS__)



#define RS232_RECONNECT_TIMEOUT		10 * SYSTICK_SECOND
TCP_STATE_TYPE rs232State = INITIAL;


DATE_TIME t3_time;
uint8_t t3_send_flag = 0;
#define FireNoti 1
uint8_t rs232SwitchServer = 0;
uint16_t rs232ServerPort = 0;
uint32_t rs232CheckDns = 0;
uip_ipaddr_t rs232ServerIpAddr;
uip_ipaddr_t *rs232ServerIp;
struct uip_conn *rs232Conn;
uint8_t rs232LoggedIn = 0;
U32 rs232Tick;
I16 rs232RexmitCnt = 0;
 I16 rs232Len = 0;
uint8_t rs232CloseSocketFlag = 0;
extern Timeout_Type timeTransferData;
Timeout_Type tRs232TimeReconnect;
Timeout_Type tRs232CloseSocket;
Timeout_Type tRs232TimeKeepConnect;
Timeout_Type tRs232TcpDataOut;
Timeout_Type tRs232CallbackTimeout;
Timeout_Type tDataRetryTimeout;
Timeout_Type tLoginTimeout;
Timeout_Type tRs232StateTimeout;
extern uint32_t pgoodStatus;
uint32_t reconnectCnt = 0;
char latBuf[16];
char lonBuf[16];
extern float csqValue;

extern float ADC1_8_Value;

ALARM_TYPE fireAlarmRecord;


enum {
	FIREALARM_INITIAL = 0,
	FIREALARM_IDLE
}FireAlarm_Phase = FIREALARM_INITIAL;

//----------------------------------end function---------------------------------

uint32_t rs232TaskLen = 0;	


uint8_t rs232Buf[300];			// must big enough to hold a packet of all types
//uint8_t rs232BufTemp[300];			// must big enough to hold a packet of all types
uint32_t rs232CallBackCnt = 0;
uint32_t rs232ReportCnt = 0;
uint8_t deviceKey[32];

void RS232_SVR_Init(uint32_t priority)
{
	uint8_t i, u8Temp0;
	rs232Conn = NULL;
	rs232State = INITIAL;
	flagSystemStatus &= ~SYS_SERVER_OK;
//	LedSetStatus(&led1Ctr,500,0,LED_TURN_ON,0xffffffff);
	/*Ring Buff*/
	InitTimeout(&tRs232TcpDataOut,SYSTICK_TIME_SEC(10));
	InitTimeout(&tRs232CallbackTimeout,SYSTICK_TIME_SEC(10));
	InitTimeout(&tRs232TimeReconnect,SYSTICK_TIME_SEC(sysCfg.reconnectTimeout));
	InitTimeout(&tRs232CloseSocket,SYSTICK_TIME_SEC((sysCfg.reconnectTimeout - 10)));//close socket before 10 sec
	InitTimeout(&tRs232StateTimeout,SYSTICK_TIME_SEC(10));
	RS232_SVR_Reset();
	
//	for(i = 0; i < strlen((char *)sysCfg.deviceKey);i++)
//	{
//		if((sysCfg.deviceKey[i] >= '0') && (sysCfg.deviceKey[i] <= '9'))
//			u8Temp0 = sysCfg.deviceKey[i] - '0';
//		else if((sysCfg.deviceKey[i] >= 'a') && (sysCfg.deviceKey[i] <= 'f'))
//			u8Temp0 = sysCfg.deviceKey[i] - 'a' + 10;
//		else if((sysCfg.deviceKey[i] >= 'A') && (sysCfg.deviceKey[i] <= 'F'))
//			u8Temp0 = sysCfg.deviceKey[i] - 'A' + 10;
//		if(i & 1)
//			deviceKey[(i-1)/2] += u8Temp0;
//		else
//		{
//			deviceKey[i/2] = u8Temp0;
//			deviceKey[i/2] <<=4;
//		}
//	}
	
}


void RS232_SVR_Reset(void)
{
	flagSystemStatus &= ~SYS_SERVER_OK;
	rs232CloseSocketFlag = 0;
	rs232Conn = NULL;
	rs232State = INITIAL;
}
uint32_t sentDataCnt = 0;
uint32_t recvDataCnt = 0;
uint32_t sequence = 0;


TCP_STATE_TYPE RS232_SVR_Manage(void)
{ 
	uint32_t addr0,addr1,addr2,addr3;
	rs232ReportCnt++;
	if(CheckTimeout(&tRs232TimeReconnect) == SYSTICK_TIMEOUT)
	{
		InitTimeout(&tRs232TimeReconnect,SYSTICK_TIME_SEC(sysCfg.reconnectTimeout));
		InitTimeout(&tRs232CloseSocket,SYSTICK_TIME_SEC((sysCfg.reconnectTimeout - 10)));//close socket before 10 sec
		PPP_ReInit();		// prevent deadlock after too much timedout efforts
	}
	else if(CheckTimeout(&tRs232CloseSocket) == SYSTICK_TIMEOUT && rs232State == CONNECTED)
	{
		InitTimeout(&tRs232CloseSocket,SYSTICK_TIME_SEC((sysCfg.reconnectTimeout - 10)));//close socket before 10 sec
		rs232CloseSocketFlag = 1;
	}
	
	
/// dua vao trang thai gui va nhan	
	
	switch(rs232State)
	{
		case INITIAL:
				flagSystemStatus &= ~SYS_SERVER_OK;
				rs232CloseSocketFlag = 0;
				rs232LoggedIn = 0;
				if(strstr((char *)sysCfg.secDserverName,"0.0.0.0"))
				{
					rs232SwitchServer = 0;
				}
				#if TEST_DEVICE
				rs232SwitchServer = 0;
				strcpy((char *)sysCfg.priDserverName,"42.117.105.14");
				sysCfg.priDserverPort = 11111;
				#endif
				if(rs232SwitchServer == 0)
				{
					if(Domain_IpCheck((uint8_t *)sysCfg.priDserverName) == IS_IP)
					{
						sscanf((char *)sysCfg.priDserverName,"%d.%d.%d.%d",&addr0,&addr1,&addr2,&addr3);
						rs232ServerIpAddr.u8[0] = addr0;
						rs232ServerIpAddr.u8[1] = addr1;
						rs232ServerIpAddr.u8[2] = addr2;
						rs232ServerIpAddr.u8[3] = addr3;
						rs232ServerIp = &rs232ServerIpAddr;
					}
					else
					{
							rs232ServerIp = RESOLVER_Lookup((char *)sysCfg.priDserverName);
							if((rs232ServerIp == NULL && (CheckTimeout(&tRs232StateTimeout) == SYSTICK_TIMEOUT)) || (SysTick_Get() - rs232CheckDns >= 180000))
							{
								InitTimeout(&tRs232StateTimeout,SYSTICK_TIME_SEC(10));
								rs232CheckDns = SysTick_Get();
								resolv_query((const char *)sysCfg.priDserverName);
							}else if(rs232ServerIp
											&& rs232ServerIp->u8[0] == 0
											&& rs232ServerIp->u8[1] == 0
											&& rs232ServerIp->u8[2] == 0
											&& rs232ServerIp->u8[3] == 0
							){
									if((SysTick_Get() - rs232CheckDns >= 10000)){
										rs232CheckDns = SysTick_Get();
									 resolv_query((const char *)sysCfg.priDserverName);
									}
									break;
							 }
					}
					rs232ServerPort = sysCfg.priDserverPort;
				}
				else
				{
					if(Domain_IpCheck((uint8_t *)sysCfg.secDserverName) == IS_IP)
					{
						sscanf((char *)sysCfg.secDserverName,"%d.%d.%d.%d",&addr0,&addr1,&addr2,&addr3);
						rs232ServerIpAddr.u8[0] = addr0;
						rs232ServerIpAddr.u8[1] = addr1;
						rs232ServerIpAddr.u8[2] = addr2;
						rs232ServerIpAddr.u8[3] = addr3;
						rs232ServerIp = &rs232ServerIpAddr;
					}
					else
					{
							rs232ServerIp = RESOLVER_Lookup((char *)sysCfg.secDserverName);
							if((rs232ServerIp == NULL && (CheckTimeout(&tRs232StateTimeout) == SYSTICK_TIMEOUT)) || (SysTick_Get() - rs232CheckDns >= 180000))
							{
								InitTimeout(&tRs232StateTimeout,SYSTICK_TIME_SEC(10));
								resolv_query((const char *)sysCfg.secDserverName);
							}else if(rs232ServerIp
											&& rs232ServerIp->u8[0] == 0
											&& rs232ServerIp->u8[1] == 0
											&& rs232ServerIp->u8[2] == 0
											&& rs232ServerIp->u8[3] == 0
							){
									if((SysTick_Get() - rs232CheckDns >= 10000)){
										rs232CheckDns = SysTick_Get();
									 resolv_query((const char *)sysCfg.secDserverName);
									}
									break;
							 }
					}
					rs232ServerPort = sysCfg.secDserverPort;
				}
				
				if(rs232ServerIp == NULL)
				{
						rs232SwitchServer ^= 1;
						break;	
				}
				else if(rs232ServerIp->u8[0] == 0 
				&& rs232ServerIp->u8[1]== 0 
				&& rs232ServerIp->u8[2] == 0
				&& rs232ServerIp->u8[3] == 0
				)
				{
						break;
				}
				rs232Conn = NULL;
				RS232_SVR_DBG("\r\nRS232_SVR: Started, Server %d.%d.%d.%d:%d\r\n", ((uint8_t*)(rs232ServerIp))[0], ((uint8_t*)(rs232ServerIp))[1], 
												((uint8_t*)(rs232ServerIp))[2], ((uint8_t*)(rs232ServerIp))[3], rs232ServerPort);
				
				rs232Conn = tcp_connect(rs232ServerIp, uip_htons(rs232ServerPort),RS232_SVR_Callback,NULL);
				rs232Tick = SysTick_Get();
				if(rs232Conn == NULL)
				{
								RS232_SVR_DBG("\r\nRS232_SVR: uip_connect returns NULL\r\n");
								rs232State = WAIT_TIMEOUT_RECONNECT;
								break;
				}
				reconnectCnt++;
				rs232State = CONNECT;
				FireAlarm_Phase = FIREALARM_INITIAL;
				InitTimeout(&tRs232CallbackTimeout,SYSTICK_TIME_SEC(10));
			break;
		case CONNECT:
			if(rs232Conn == NULL)
			{
				RS232_SVR_Reset();
				break;
			}
			if((SysTick_Get() - rs232Tick >= 60000)
			|| 	rs232Conn->tcpstateflags == UIP_CLOSED
			)
			{
				RS232_SVR_Reset();
			}
		break;
		case WAIT_TIMEOUT_RECONNECT:
			if(SysTick_Get() - rs232Tick >= RS232_RECONNECT_TIMEOUT)
				RS232_SVR_Reset();
			break;
		case CONNECTED:
			
			if(CheckTimeout(&tRs232CallbackTimeout) == SYSTICK_TIMEOUT)
			{
				tcpIpTryCnt++;
				RS232_SVR_Init(1);
				rs232State = INITIAL;
				rs232RexmitCnt = 0;
			}
		break;
		default:
			break;
	}
	return rs232State;
}



void RS232_SVR_Callback()
{
	uint32_t i;
	DATE_TIME time;
	uint32_t u32Temp0,u32Temp1,u32Temp2,u32Temp3,u32Temp4,u32Temp5,u32Temp6,u32Temp7;
	char *pt,*pt0;
	uint8_t buf[64],u8Temp0;
	uint16_t len = 0;
	if(uip_conn != rs232Conn)
		return;
	rs232CallBackCnt++;
	InitTimeout(&tRs232CallbackTimeout,SYSTICK_TIME_SEC(10));
	if(uip_connected())
	{
//		LedSetStatus(&led1Ctr,100,3000,LED_TURN_ON,0xffffffff);
		RS232_SVR_DBG("\r\nRS232_SVR: connected\r\n");
		rs232CloseSocketFlag = 0;
		rs232RexmitCnt = 0;
		rs232Len = 0;
		rs232LoggedIn = 0;
		rs232State = CONNECTED;
		flagSystemStatus |= SYS_SERVER_OK;
		InitTimeout(&tRs232TimeReconnect,SYSTICK_TIME_SEC(sysCfg.reconnectTimeout));
		InitTimeout(&tRs232CloseSocket,SYSTICK_TIME_SEC((sysCfg.reconnectTimeout - 10)));//close socket before 10 sec
		InitTimeout(&tRs232TimeKeepConnect,SYSTICK_TIME_MS(1));
		InitTimeout(&tDataRetryTimeout,SYSTICK_TIME_SEC(3));
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	
	if(uip_poll())//ranh co the gui data len server 
	{
		if(CheckTimeout(&tRs232TcpDataOut) == SYSTICK_TIMEOUT)
			rs232Len = 0;
		
		if(FirmwareTask_IsBusy())
		{
			t3_send_flag = 0;
		}
		
		if(rs232Len == 0)
		{
			InitTimeout(&tRs232TcpDataOut,SYSTICK_TIME_SEC(10));
			if(CheckTimeout(&tLoginTimeout) == SYSTICK_TIMEOUT)
			{
				InitTimeout(&tLoginTimeout,SYSTICK_TIME_SEC(3600));
				FireAlarm_Phase = FIREALARM_INITIAL;
			}
			switch(FireAlarm_Phase)
			{
				case FIREALARM_INITIAL:					
					if(CheckTimeout(&tDataRetryTimeout) == SYSTICK_TIMEOUT)
					{
						InitTimeout(&tLoginTimeout,SYSTICK_TIME_SEC(3600));
						InitTimeout(&tDataRetryTimeout,SYSTICK_TIME_SEC(10));
						rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T1,%s,%s,%s,0,%03d",
											sysTime.year,
											sysTime.month,
											sysTime.mday,
											sysTime.hour,
											sysTime.min,
											sysTime.sec,
											sysCfg.id,
											sysCfg.cimi,
											sysCfg.bossPhoneNum,
											sysCfg.deviceKey,
											sequence
						);
						RS232_SVR_DBG("\r\nRS232_SVR: sending:%s\r\n", rs232Buf);
						sprintf((char *)mainBuf,"D->S:{%s}\r\n",rs232Buf);
						//DB_DataMsgSave(sysTime,mainBuf,strlen((char *)mainBuf));
						rs232Len = FireAlarmCreateMsg(rs232Buf,rs232Len,mainBuf,sizeof(mainBuf),sysCfg.mainAesKey);
						rs232Buf[0] = '{';
						rs232Buf[1] = 0;
						strcat((char *)rs232Buf,(char *)mainBuf);
						strcat((char *)rs232Buf,"}");
						rs232Len = strlen((char *)rs232Buf);
					}
				break;
				
				// case 2				
				case FIREALARM_IDLE:			
					rs232Len = 0;
//					rs232Len = sprintf((char *)&rs232Buf[rs232Len],"%04d-%02d-%02d %02d:%02d:%02d,\1,\
//					W4,\
//					%s,\
//					T2,\
//					%d,\
//					%d,\
//					%d\
//					",
//										sysTime.year,
//										sysTime.month,
//										sysTime.mday,
//										sysTime.hour,
//										sysTime.min,
//										sysTime.sec,
//										sysCfg.id,
//										sysCfg.freq,
//										sysCfg.freq2,
//										sequence
//					);
					
					//ban tin t3 ====duy tri ket noi
					///ban tin T25---bao chay
					if(sysCfg.lon == 0)
					{
						sprintf(lonBuf, "0");
					}
					else if(sysCfg.lon > 0)
					{
						sprintf(lonBuf, "E,%0.6f",(sysCfg.lon));
					}
					else
					{
						sprintf(lonBuf, "W%0.6f",(-sysCfg.lon));
					}
					//GPS Lat
					if(sysCfg.lat == 0)
					{
						sprintf(latBuf, "0");
					}
					else if(sysCfg.lat > 0)
					{
						sprintf(latBuf, "N,%0.6f",(sysCfg.lat));
					}
					else
					{
						sprintf(latBuf, "S,%0.6f",(-sysCfg.lat));//neamFormatLatLng
					}
					
					if (fire_trigger && (CheckTimeout(&tDataRetryTimeout) == SYSTICK_TIMEOUT))
					{
						InitTimeout(&tDataRetryTimeout,SYSTICK_TIME_SEC(3));
						if (fired_flag)
						{	
							rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T25,0,%s,%s,0,0,14,0.0.0.0,00%02d%02d,0,%03d",
												sysTime.year,
												sysTime.month,
												sysTime.mday,
												sysTime.hour,
												sysTime.min,
												sysTime.sec,
												sysCfg.id,
												"E,0",
												"N,0",
												(uint32_t)csqValue,
												(uint32_t)ADC1_8_Value,
												sequence
							);
						}
						else
						{
							rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T25,0,%s,%s,0,0,2,0.0.0.0,00%02d%02d,0,%03d",
												sysTime.year,
												sysTime.month,
												sysTime.mday,
												sysTime.hour,
												sysTime.min,
												sysTime.sec,
												sysCfg.id,
												"E,0",
												"N,0",
												(uint32_t)csqValue,
												(uint32_t)ADC1_8_Value,
												sequence
							);
						}
					}
					else if(lowpower_flag && lowpower_trigger && (CheckTimeout(&tDataRetryTimeout) == SYSTICK_TIMEOUT))
					{
						//2008-12-16 10:00:00,1,W4,661401000001,T4,11.05,11.50,2,123]
						lowpower_trigger = 0;
						InitTimeout(&tDataRetryTimeout,SYSTICK_TIME_SEC(3));
						if (fired_flag)
						{
							rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T4,%0.2f,%0.2f,14,%03d",
												sysTime.year,
												sysTime.month,
												sysTime.mday,
												sysTime.hour,
												sysTime.min,
												sysTime.sec,
												sysCfg.id,
												ADC1_8_Value,
												sysCfg.vinlbv,
												sequence										
							);
						}
						else
						{
							rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T4,%0.2f,%0.2f,2,%03d",
												sysTime.year,
												sysTime.month,
												sysTime.mday,
												sysTime.hour,
												sysTime.min,
												sysTime.sec,
												sysCfg.id,
												ADC1_8_Value,
												sysCfg.vinlbv,
												sequence										
							);
						}
					}
					else if(t3_send_flag)//if(CheckTimeout(&tRs232TimeKeepConnect) == SYSTICK_TIMEOUT)
					{
						
						t3_send_flag = 0;
						//rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T3,1,E,113.252432,N,22.564152,0,0,1,0.0.0.0,00%02d%02d,0,%03d",
						if (fired_flag)
						{
							rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T3,0,%s,%s,0,0,14,0.0.0.0,00%02d%02d,0,%03d",
												t3_time.year,
												t3_time.month,
												t3_time.mday,
												t3_time.hour,
												t3_time.min,
												t3_time.sec,
												sysCfg.id,
												"E,0",
												"N,0",
												(uint32_t)csqValue,
												(uint32_t)ADC1_8_Value,
												sequence										
							);
					}
					else
					{
						rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T3,0,%s,%s,0,0,2,0.0.0.0,00%02d%02d,0,%03d",
											t3_time.year,
											t3_time.month,
											t3_time.mday,
											t3_time.hour,
											t3_time.min,
											t3_time.sec,
											sysCfg.id,
											"E,0",
											"N,0",
											(uint32_t)csqValue,
											(uint32_t)ADC1_8_Value,
											sequence										
						);
					}
//						rs232Len = sprintf((char *)rs232Buf,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T3,0,E,0,N,0,0,0,2,0.0.0.0,001813,0,%03d",
//											sysTime.year,
//											sysTime.month,
//											sysTime.mday,
//											sysTime.hour,
//											sysTime.min,
//											sysTime.sec,
//											sysCfg.id,
//											sequence		
//						);
					}
					if(rs232Len)
					{
						RS232_SVR_DBG("\r\nRS232_SVR: sending:%s\r\n", rs232Buf);
						sprintf((char *)mainBuf,"D->S:[%s]\r\n",rs232Buf);
						//DB_DataMsgSave(sysTime,mainBuf,strlen((char *)mainBuf));
						rs232Len = FireAlarmCreateMsg(rs232Buf,rs232Len,mainBuf,sizeof(mainBuf),sysCfg.deviceKey);
						rs232Buf[0] = '[';
						rs232Buf[1] = 0;
						strcat((char *)rs232Buf,(char *)mainBuf);
						strcat((char *)rs232Buf,"]");
						rs232Len = strlen((char *)rs232Buf);
					}
				break;
					
			}
		
			//ngay thang nam gio phut giay la sysTime
			
			if(rs232Len)
			{	
				sequence++;
				sequence %= 999;
//				LedSetStatus(&led2Ctr,rs232Len,10,LED_TURN_ON,1);
				sentDataCnt += rs232Len;
				rs232Buf[rs232Len] = 0;
				if (fired_flag)
				{
					InitTimeout(&tRs232TimeKeepConnect,SYSTICK_TIME_SEC(sysCfg.freq2));
				}
				else
				{
					InitTimeout(&tRs232TimeKeepConnect,SYSTICK_TIME_SEC(sysCfg.freq));
				}
				uip_send(rs232Buf,rs232Len);
				RS232_SVR_DBG("\r\nRS232_SVR: sending:%s\r\n", rs232Buf);
				RS232_SVR_DBG("\r\nRS232_SVR: sending %d byte(s)\r\n", rs232Len);

			}
		}
	}
	
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	if(uip_newdata())// nhan data tu server 
	{
		if(uip_len)
		{
			RS232_SVR_DBG("\r\nRS232_SVR:uip_newdata = %dBytes\r\n",uip_len);
			
			InitTimeout(&timeTransferData,SYSTICK_TIME_SEC(30));
			InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(TCP_BUSY_TIMEOUT));
			InitTimeout(&tRs232TimeReconnect,SYSTICK_TIME_SEC(sysCfg.reconnectTimeout));
			InitTimeout(&tRs232CloseSocket,SYSTICK_TIME_SEC((sysCfg.reconnectTimeout - 10)));//close socket before 10 sec
//			LedSetStatus(&led2Ctr,uip_len,10,LED_TURN_ON,1);

			
			
			recvDataCnt += uip_len;
			
			switch(FireAlarm_Phase)
			{
				case FIREALARM_INITIAL:
					if(((char *)uip_appdata)[0] == '{')
					{
						
						rs232Len = FireAlarmParsingMsg((uint8_t *)&((char *)uip_appdata)[1],uip_len - 2,mainBuf,sizeof(mainBuf),sysCfg.mainAesKey);
						if(rs232Len)
						{

							memcpy((uint8_t *)&((char *)uip_appdata)[1],mainBuf,rs232Len);
							((char *)uip_appdata)[rs232Len + 1] = 0;
							RS232_SVR_DBG("\r\nRS232_SVR:FireAlarmParsingMsg = %s\r\n",(char *)uip_appdata);
							
							sprintf((char *)mainBuf,"S->D:%s\r\n",(char *)uip_appdata);
							//DB_DataMsgSave(sysTime,mainBuf,strlen((char *)mainBuf));
							
							sscanf((char *)uip_appdata,"{%04d-%02d-%02d %02d:%02d:%02d,S1,%d,%d",
									&u32Temp0,
									&u32Temp1,
									&u32Temp2,
									&u32Temp3,
									&u32Temp4,
									&u32Temp5,
									&u32Temp6,
									&u32Temp7
									);
									time.year = u32Temp0;
									time.month = u32Temp1;
									time.mday = u32Temp2;
									time.hour = u32Temp3;
									time.min =  u32Temp4;
									time.sec = u32Temp5;
									UpdateRtcTime(TIME_GetSec(&time));
									RS232_SVR_DBG("\r\nRS232_SVR:UpdateRtcTime = %04d-%02d-%02d %02d:%02d:%02d\r\n",time.year,time.month,time.mday,time.hour,time.min,time.sec);
									if(u32Temp6 && u32Temp7 >= (sequence - 1))
									{
											FireAlarm_Phase = FIREALARM_IDLE;
											InitTimeout(&tRs232TimeKeepConnect,SYSTICK_TIME_SEC(3));
									}
								}
							}
				break;
							
				case FIREALARM_IDLE:
					if(((char *)uip_appdata)[0] == '[')
					{
						rs232Len = FireAlarmParsingMsg((uint8_t *)&((char *)uip_appdata)[1],uip_len - 2,mainBuf,sizeof(mainBuf),sysCfg.deviceKey);
					}
					else if(((char *)uip_appdata)[0] == '{')
					{
						rs232Len = FireAlarmParsingMsg((uint8_t *)&((char *)uip_appdata)[1],uip_len - 2,mainBuf,sizeof(mainBuf),sysCfg.mainAesKey);
					}
					if(rs232Len)
					{
						memcpy((uint8_t *)&((char *)uip_appdata)[1],mainBuf,rs232Len);
						
						sprintf((char *)mainBuf,"S->D:%s\r\n",(char *)uip_appdata);
						//DB_DataMsgSave(sysTime,mainBuf,strlen((char *)mainBuf));
						
						rs232Len = 0;
						pt0 = strstr((char *)uip_appdata,"[,S2,");
						if(pt0 != NULL)
						{
							memcpy(pt0," SET,",5);
							CMD_CfgParseServer((char *)pt0,mainBuf,sizeof(mainBuf),&len,0);
							//response s2
							if(len)
							{
								rs232Len = sprintf((char *)&rs232Buf[rs232Len],"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T2,%s%03d",
														sysTime.year,
														sysTime.month,
														sysTime.mday,
														sysTime.hour,
														sysTime.min,
														sysTime.sec,
														sysCfg.id,
														(char *)mainBuf,
														sequence
									);
//								rs232Len = sprintf((char *)&rs232Buf[rs232Len],"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T2,1,%03d",
//														sysTime.year,
//														sysTime.month,
//														sysTime.mday,
//														sysTime.hour,
//														sysTime.min,
//														sysTime.sec,
//														sysCfg.id,
//														sequence
//									);
							}
//							else
//							{
//								rs232Len = sprintf((char *)&rs232Buf[rs232Len],"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T2,0,%03d",
//														sysTime.year,
//														sysTime.month,
//														sysTime.mday,
//														sysTime.hour,
//														sysTime.min,
//														sysTime.sec,
//														sysCfg.id,
//														sequence
//								);
//							}
						}
	//////////////////////////////////////nhan va reply ban tin T14				
						pt0 = strstr((char *)uip_appdata,"[,S14,");
						if(pt0 != NULL)
						{
							memcpy(pt0,"  GET,",6);
							CMD_CfgParseServer((char *)pt0,mainBuf,sizeof(mainBuf),&len,0);
							//response s2
							rs232Len = sprintf((char *)&rs232Buf[rs232Len],"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T14,%s%03d",
													sysTime.year,
													sysTime.month,
													sysTime.mday,
													sysTime.hour,
													sysTime.min,
													sysTime.sec,
													sysCfg.id,
													(char *)mainBuf,
													sequence
								);
						}
	////////////////////nhan va kiem tra S25
						//[,S25,123]
						pt0 = strstr((char *)uip_appdata,"[,S25,");
						if(pt0 != NULL)
						{
							sscanf(pt0,"[,S25,%d",&u32Temp0);

							if(u32Temp0 >= (sequence - 1))
								fire_trigger = 0;
						}
						
						//[,S4,123]
						pt0 = strstr((char *)uip_appdata,"[,S4,");
						if(pt0 != NULL)
						{
							sscanf(pt0,"[,S4,%d",&u32Temp0);

							if(u32Temp0 >= (sequence - 1))
								lowpower_trigger = 0;
						}
						
				}
				if(rs232Len)
				{	
					RS232_SVR_DBG("\r\nRS232_SVR: sending:%s\r\n", rs232Buf);
					sprintf((char *)mainBuf,"D->S:[%s]\r\n",rs232Buf);
					//DB_DataMsgSave(sysTime,mainBuf,strlen((char *)mainBuf));
					rs232Len = FireAlarmCreateMsg(rs232Buf,rs232Len,mainBuf,sizeof(mainBuf),sysCfg.deviceKey);
					rs232Buf[0] = '[';
					rs232Buf[1] = 0;
					strcat((char *)rs232Buf,(char *)mainBuf);
					strcat((char *)rs232Buf,"]");
					rs232Len = strlen((char *)rs232Buf);
					sequence++;
//					LedSetStatus(&led2Ctr,rs232Len,10,LED_TURN_ON,1);
					sentDataCnt += rs232Len;
					rs232Buf[rs232Len] = 0;
					//InitTimeout(&tRs232TimeKeepConnect,SYSTICK_TIME_SEC(sysCfg.freq));
					uip_send(rs232Buf,rs232Len);
					RS232_SVR_DBG("\r\nRS232_SVR: sending %d byte(s)\r\n", rs232Len);

				}
				break;
				
			}
		}
	}
	if(uip_acked()) // xac nhan goi tin da duoc truyen len server
	{
		switch(FireAlarm_Phase)
		{
			case FIREALARM_INITIAL:
				#if TEST_DEVICE
				FireAlarm_Phase = FIREALARM_IDLE;
				#endif
			break;
			case FIREALARM_IDLE:
			break;
		}
		rs232LoggedIn = 1;
		rs232Len = 0;
		rs232RexmitCnt = 0;	
		tcpIpTryCnt = 0;
		RS232_SVR_DBG("\r\nRS232_SVR:uip_acked\r\n");
		InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(PPP_RESET_TIMEOUT));
	}
	
	if((uip_rexmit()))// gui lai goi tin vua truyen len server neu ko nhan dc ack
	{
		uip_send(rs232Buf, rs232Len);
		RS232_SVR_DBG("\r\nRS232_SVR: rexmit\r\n");
		rs232RexmitCnt ++;
		if(rs232RexmitCnt >= 3) 
		{
			rs232RexmitCnt = 0;
			rs232Len = 0;
			rs232State = WAIT_TIMEOUT_RECONNECT;
			InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(10));
		}
	}
	
	
	if(uip_closed() || uip_aborted())//ngat ket noi voi server
	{
		RS232_SVR_DBG("\r\nRS232_SVR:uip_closed\r\n");
		flagSystemStatus &= ~SYS_SERVER_OK;
		rs232Conn = NULL;
		rs232Tick = SysTick_Get();
		rs232State = WAIT_TIMEOUT_RECONNECT;		
	}
	
	if(rs232CloseSocketFlag)
	{
		rs232CloseSocketFlag = 0;
		flagSystemStatus &= ~SYS_SERVER_OK;
		uip_close();
	}
	
}


