#include "ampm_gsm_main_task.h"
#include "tracking_server.h"
#include "lib/sys_tick.h"
#include "system_config.h"
#include "tcp_ip_task.h"
#include "lib/sys_time.h"
#include "tracker.h"
#include "sms_task.h"
#include "gt121_setting.h"
#include "io_control.h"
#define TIME_KEEP_CONNECT 60

#define TCPIP_ECHO_ON

#include "app_config_task.h"
extern uint32_t  DbgCfgPrintf(uint8_t type_log,const uint8_t *format, ...);
#define TRACKING_SVR_DBG(...) DbgCfgPrintf(TCP_IP_LOG,__VA_ARGS__)


#define TRACKING_SVR_DISCONNECT_TIMEOUT		3600
#define TRACKING_RECONNECT_TIMEOUT		10 * SYSTICK_SECOND

TCP_STATE_TYPE trackingState = INITIAL;

uint8_t trackingSwitchServer = 0;
uint16_t trackingServerPort = 0;
uint32_t trackingCheckDns = 0;
uip_ipaddr_t trackingServerIpAddr;
uip_ipaddr_t *trackingServerIp;
struct uip_conn *trackingConn;
extern uint8_t gsmStatus;

U32 trackingTick;
I16 trackingRexmitCnt = 0;
I16 trackingLen = 0;
uint8_t trackingCloseSocketFlag = 0;

Timeout_Type tTrackingTimeReconnect;
Timeout_Type tTrackingCloseSocket;
Timeout_Type tTrackingTimeKeepConnect;
Timeout_Type tTrackingTcpDataOut;
Timeout_Type tTrackingCallbackTimeout;

TCP_IP_CONNECTION_TYPE trackingServerConnectType = KEEP_CONNECTION_WHEN_HAVE_NO_DATA;

extern uint32_t pgoodStatus;

uint8_t tcpIpAnswerCmd = 0;


uint32_t trackingTaskLen = 0;	


uint8_t trackingBuf[512];			// must big enough to hold a packet of all types

uint32_t trackingCallBackCnt = 0;
uint32_t trackingReportCnt = 0;

void TRACKING_SVR_Init(uint32_t priority)
{
	trackingConn = NULL;
	trackingState = INITIAL;
	/*Ring Buff*/
	InitTimeout(&tTrackingTcpDataOut,SYSTICK_TIME_SEC(10));
	InitTimeout(&tTrackingCallbackTimeout,SYSTICK_TIME_SEC(10));
	InitTimeout(&tTrackingTimeReconnect,SYSTICK_TIME_SEC(TRACKING_SVR_DISCONNECT_TIMEOUT));
	InitTimeout(&tTrackingCloseSocket,SYSTICK_TIME_SEC((TRACKING_SVR_DISCONNECT_TIMEOUT - 10)));//close socket before 10 sec
	TRACKING_SVR_Reset();
}


void TRACKING_SVR_Reset(void)
{
	trackingCloseSocketFlag = 0;
	trackingConn = NULL;
	trackingState = INITIAL;
}
uint32_t sentDataCnt = 0;
uint32_t recvDataCnt = 0;
TCP_STATE_TYPE TRACKING_SVR_Manage(void)
{ 
	uint32_t addr0,addr1,addr2,addr3;
	trackingReportCnt++;
	if(CheckTimeout(&tTrackingTimeReconnect) == SYSTICK_TIMEOUT)
	{
		InitTimeout(&tTrackingTimeReconnect,SYSTICK_TIME_SEC(TRACKING_SVR_DISCONNECT_TIMEOUT));
		InitTimeout(&tTrackingCloseSocket,SYSTICK_TIME_SEC((TRACKING_SVR_DISCONNECT_TIMEOUT - 10)));//close socket before 10 sec
		PPP_ReInit();		// prevent deadlock after too much timedout efforts
	}
	else if(CheckTimeout(&tTrackingCloseSocket) == SYSTICK_TIMEOUT && trackingState == CONNECTED)
	{
		InitTimeout(&tTrackingCloseSocket,SYSTICK_TIME_SEC((TRACKING_SVR_DISCONNECT_TIMEOUT - 10)));//close socket before 10 sec
		trackingCloseSocketFlag = 1;
	}
	switch(trackingState)
	{
		case INITIAL:
				trackingCloseSocketFlag = 0;
				if(trackingServerConnectType == DISCONNECT_WHEN_NO_DATA && logSendPt == NULL)
				{
					break;
				}
				gsmStatus = 2;
				trackingSwitchServer = 0;
				if(trackingSwitchServer == 0)
				{
					if(Domain_IpCheck((uint8_t *)sysCfg.priDserverName) == IS_IP)
					{
						sscanf((char *)sysCfg.priDserverName,"%d.%d.%d.%d",&addr0,&addr1,&addr2,&addr3);
						trackingServerIpAddr.u8[0] = addr0;
						trackingServerIpAddr.u8[1] = addr1;
						trackingServerIpAddr.u8[2] = addr2;
						trackingServerIpAddr.u8[3] = addr3;
						trackingServerIp = &trackingServerIpAddr;
					}
					else
					{
							trackingServerIp = RESOLVER_Lookup((char *)sysCfg.priDserverName);
							if(trackingServerIp == NULL && (SysTick_Get() - trackingCheckDns >= 10000))
							{
											trackingCheckDns = SysTick_Get();
											resolv_query((const char *)sysCfg.priDserverName);
							}else if(trackingServerIp
											&& trackingServerIp->u8[0] == 0
											&& trackingServerIp->u8[1] == 0
											&& trackingServerIp->u8[2] == 0
											&& trackingServerIp->u8[3] == 0
							){
									if((SysTick_Get() - trackingCheckDns >= 10000)){
										trackingCheckDns = SysTick_Get();
									 resolv_query((const char *)sysCfg.priDserverName);
									}
									break;
							 }
					}
					trackingServerPort = sysCfg.priDserverPort;
					//trackingServerIp = &trackingServerIpAddr;
				}
				else
				{
					if(Domain_IpCheck((uint8_t *)sysCfg.secDserverName) == IS_IP)
					{
						sscanf((char *)sysCfg.priDserverName,"%d.%d.%d.%d",&addr0,&addr1,&addr2,&addr3);
						trackingServerIpAddr.u8[0] = addr0;
						trackingServerIpAddr.u8[1] = addr1;
						trackingServerIpAddr.u8[2] = addr2;
						trackingServerIpAddr.u8[3] = addr3;
						trackingServerIp = &trackingServerIpAddr;
					}
					else
					{
							trackingServerIp = RESOLVER_Lookup((char *)sysCfg.secDserverName);
							if(trackingServerIp == NULL && (SysTick_Get() - trackingCheckDns >= 10000))
							{
											trackingCheckDns = SysTick_Get();
											resolv_query((const char *)sysCfg.secDserverName);
							}else if(trackingServerIp
											&& trackingServerIp->u8[0] == 0
											&& trackingServerIp->u8[1] == 0
											&& trackingServerIp->u8[2] == 0
											&& trackingServerIp->u8[3] == 0
							){
									if((SysTick_Get() - trackingCheckDns >= 10000)){
										trackingCheckDns = SysTick_Get();
									 resolv_query((const char *)sysCfg.secDserverName);
									}
									break;
							 }
					}
					trackingServerPort = sysCfg.secDserverPort;
					//trackingServerIp = &trackingServerIpAddr;
				}
//				trackingServerIpAddr.u8[0] = 183;
//				trackingServerIpAddr.u8[1] = 91;
//				trackingServerIpAddr.u8[2] = 4;
//				trackingServerIpAddr.u8[3] = 233;
//				trackingServerPort = 5555;
				
				if(trackingServerIp == NULL)
				{
						trackingSwitchServer ^= 1;
						break;	
				}
				else if((trackingServerIp->u8[0] == 0)
				&& (trackingServerIp->u8[1]== 0) 
				&& (trackingServerIp->u8[2] == 0)
				&& (trackingServerIp->u8[3] == 0))
				{
						break;
				}
				trackingConn = NULL;
				//TRACKING_SVR_DBG("TRACKING_SVR: Started, Server %d.%d.%d.%d:%d\r\n");
				TRACKING_SVR_DBG("TRACKING_SVR: Started, Server %d.%d.%d.%d:%d\r\n", ((uint8_t*)(trackingServerIp))[0], ((uint8_t*)(trackingServerIp))[1], 
												((uint8_t*)(trackingServerIp))[2], ((uint8_t*)(trackingServerIp))[3], trackingServerPort);
				
				trackingConn = tcp_connect(trackingServerIp, uip_htons(trackingServerPort),TRACKING_SVR_Callback,NULL);
				trackingTick = SysTick_Get();
				if(trackingConn == NULL)
				{
								TRACKING_SVR_DBG("TRACKING_SVR: uip_connect returns NULL\r\n");
								trackingState = WAIT_TIMEOUT_RECONNECT;
								break;
				}
				trackingState = CONNECT;
				
				InitTimeout(&tTrackingCallbackTimeout,SYSTICK_TIME_SEC(10));
			break;
		case CONNECT:
		case WAIT_TIMEOUT_RECONNECT:
			if(SysTick_Get() - trackingTick >= TRACKING_RECONNECT_TIMEOUT)
				TRACKING_SVR_Reset();
			break;
		case CONNECTED:
			gsmStatus = 3;
			IO_ToggleSetStatus(&io_sys_led,100,3000,IO_TOGGLE_ENABLE,IO_MAX_TIMES);
			flagSystemStatus &= ~SYS_GPRS_LOSE;
			if(CheckTimeout(&tTrackingCallbackTimeout) == SYSTICK_TIMEOUT)
			{
				tcpIpTryCnt++;
				TRACKING_SVR_Init(1);
				trackingState = INITIAL;
				trackingRexmitCnt = 0;
			}
		break;
		default:
			break;
	}
	return trackingState;
}

void TRACKING_SVR_Callback()
{
	uint32_t i;
	uint16_t len;
//	static MSG_SPEED_RECORD *logSpeedPt = NULL;
	MSG_SPEED_RECORD logSpeed;
	if(uip_conn != trackingConn)
		return;
	trackingCallBackCnt++;
	InitTimeout(&tTrackingCallbackTimeout,SYSTICK_TIME_SEC(10));
	if(uip_connected())
	{

		TRACKING_SVR_DBG("TRACKING_SVR: connected\r\n");
		trackingCloseSocketFlag = 0;
		trackingRexmitCnt = 0;
		trackingLen = 0;
		trackingState = CONNECTED;
		InitTimeout(&tTrackingTimeReconnect,SYSTICK_TIME_SEC(TRACKING_SVR_DISCONNECT_TIMEOUT));
		InitTimeout(&tTrackingCloseSocket,SYSTICK_TIME_SEC((TRACKING_SVR_DISCONNECT_TIMEOUT - 10)));//close socket before 10 sec
		InitTimeout(&tTrackingTimeKeepConnect,SYSTICK_TIME_MS(1));
		
	}
	
	if(uip_poll())
	{
		if(CheckTimeout(&tTrackingTcpDataOut) == SYSTICK_TIMEOUT)
			trackingLen = 0;
			
		if(trackingLen == 0)
		{
			if(logSendPt == NULL)
			{
				if(DB_LoadNextLog(&logSend) == 0)
				{
					logSendPt = &logSend;	
				}
			}
			
			GetDataFromLog();
			GetDataFromLogSpeed();
			if(logSendPt && logSendPt->serverSent == 0)
				AddTrackerPacket(logSendPt,(char *)trackingBuf,(uint16_t *)&trackingLen);
			else if(logSpeedSendPt && logSpeedSendPt->sent == 0)
			{
				AddTrackerPacket_Speed(logSpeedSendPt,(char *)trackingBuf,(uint16_t *)&trackingLen);
			}
			else if(trackingServerConnectType == DISCONNECT_WHEN_NO_DATA)
			{
				uip_close();
			}
			InitTimeout(&tTrackingTcpDataOut,SYSTICK_TIME_SEC(10));
			if(trackingLen)
			{			
				sentDataCnt += trackingLen;
				trackingBuf[trackingLen] = 0;
				//trackingLen = sprintf(trackingBuf,"hello/r/n");
				
				InitTimeout(&tTrackingTimeKeepConnect,SYSTICK_TIME_SEC(TIME_KEEP_CONNECT));
				uip_send(trackingBuf,trackingLen);
				TRACKING_SVR_DBG(" TRACKING_SVR: sending %d byte(s)\r\n", trackingLen);
				TRACKING_SVR_DBG(" TRACKING_SVR:send_data = %s \r\n",trackingBuf);

			}
		}
	}
	
	if(uip_newdata())
	{
		if(uip_len)
		{
			
			TRACKING_SVR_DBG("TRACKING_SVR:uip_newdata = %dBytes\r\n",uip_len);
			TRACKING_SVR_DBG("TRACKING_SVR:recv_data = %s\r\n",((uint8_t *)uip_appdata));
			
			((uint8_t *)uip_appdata)[uip_len] = 0;
			((uint8_t *)uip_appdata)[uip_len + 1] = 0;
			((uint8_t *)uip_appdata)[uip_len + 2] = 0;
			if(((uint8_t *)uip_appdata)[0] == '{' 
				&& ((uint8_t *)uip_appdata)[1] == ','
				&& (memcmp(&((uint8_t *)uip_appdata)[2],sysCfg.id,strlen((char *)sysCfg.id)) == 0)
			)
			{
				len = sprintf((char *)trackingBuf,"{,%s,",(char *)sysCfg.id);
				GT121_ProcessCmdSms((char *)uip_appdata,(char *)&trackingBuf[len],NULL);
				strcat((char *)trackingBuf,",#\r\n");
				if(trackingLen >= sizeof(trackingBuf)) trackingLen = sizeof(trackingBuf);
				trackingLen = strlen((char *)trackingBuf);
				uip_send(trackingBuf,trackingLen);
			}
			
			InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(TCP_BUSY_TIMEOUT));
			InitTimeout(&tTrackingTimeReconnect,SYSTICK_TIME_SEC(TRACKING_SVR_DISCONNECT_TIMEOUT));
			InitTimeout(&tTrackingCloseSocket,SYSTICK_TIME_SEC((TRACKING_SVR_DISCONNECT_TIMEOUT - 10)));//close socket before 10 sec
			recvDataCnt += uip_len;

		 }
	}	
	if(uip_acked())
	{
		if(logSendPt)
		{	
			if(trackerSaveLog == 0 && (sendStatus || (!ACC_IS_ON)))
			{
				sendStatus = 0;
				statusSentFlag = 1;
			}
			logSendPt->serverSent = 1;
			logSendPt = NULL;
		}
		
		if(logSpeedSendPt)
		{
			logSpeedSendPt->sent = 1;
			logSpeedSendPt = NULL;
		}
		if(askAddrCnt >= 6 && getAddrFlag)
		{
			askAddrCnt = 0;
			getAddrFlag = 0;	
		}
		TRACKING_SVR_DBG("TRACKING_SVR:uip_acked\r\n");
		TRACKING_SVR_DBG("TRACKING_SVR:sent:%dBytes\r\n",trackingLen);
		trackingLen = 0;
		trackingRexmitCnt = 0;	
		tcpIpTryCnt = 0;
		InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(PPP_RESET_TIMEOUT));
	}
	
	if((uip_rexmit()))
	{
		uip_send(trackingBuf, trackingLen);
		TRACKING_SVR_DBG("TRACKING_SVR: rexmit\r\n");
		trackingRexmitCnt ++;
		if(trackingRexmitCnt >= 5) 
		{
			trackingRexmitCnt = 0;
			trackingLen = 0;
			trackingState = WAIT_TIMEOUT_RECONNECT;
		}
	}
	
	
	if(uip_closed() || uip_aborted())
	{
		TRACKING_SVR_DBG("TRACKING_SVR:uip_closed\r\n");
//		LedSetStatus(&led1Ctr,500,500,LED_TURN_ON,0xffffffff);
		trackingConn = NULL;
		trackingTick = SysTick_Get();
		trackingState = WAIT_TIMEOUT_RECONNECT;		
	}
	
	if(trackingCloseSocketFlag)
	{
		trackingCloseSocketFlag = 0;
		uip_close();
	}
	
}


