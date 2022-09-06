
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "lib/sys_tick.h"
#include "lib/ringbuf.h"
#include "app_config_task.h"
#include "uart_config_task.h"
#include "system_config.h"
#include "uip.h"
#include "resolver.h"
#include "tcp_ip_task.h"
#include "tcpip.h"

extern uint32_t  DbgCfgPrintf(uint8_t type_log,const uint8_t *format, ...);
#define FW_DBG(...)		DbgCfgPrintf(TCP_IP_LOG,__VA_ARGS__)

#define GPRS_PACKET_SIZE 			516

#define FIRMWARE_SIZE_MAX (128*1024) //128Kbytes

#define TIME_KEEP_CONNECT 60

#define TCPIP_ECHO_ON



#define FW_RECONNECT_TIMEOUT		30 * SYSTICK_SECOND

extern uint8_t CfgCalcCheckSum(uint8_t *buff, uint32_t length);
void FIRMWARE_Callback(void);

uint16_t fwServerPort = 0;
uint32_t fwCheckDns = 0;

TCP_STATE_TYPE fwState = INITIAL;
uip_ipaddr_t fwServerIpAddr;
uip_ipaddr_t *fwServerIp;
struct uip_conn *fwConn;
uint32_t fwTick;
static int16_t fwRexmitCnt = 0;
int16_t fwTimedOutCnt = 0;
int16_t fwLen = 0;
volatile int8_t fwLoggedIn = 0;
extern U32 rpTick;
uint8_t fwBuf[64];
extern uint32_t smsCheckTime;

#if defined ( __ICCARM__ )
#pragma pack(1) 
typedef struct
{
	uint8_t imei[18];
	uint8_t id[18];
	uint8_t ver[16];
}INFO_FIRMWARE_TYPE;
#pragma pack()
#elif defined (__CC_ARM)
typedef struct __attribute__((packed))
{
	uint8_t imei[18];
	uint8_t id[18];
	uint8_t ver[16];
}INFO_FIRMWARE_TYPE;
#endif


INFO_FIRMWARE_TYPE fInfo;

uint8_t loginSendBuff[sizeof(INFO_FIRMWARE_TYPE) + 5];

uint32_t fwCallBackCnt = 0;


Timeout_Type fdummySendPacket;


Timeout_Type tFrimwareUpdateTimeout;
Timeout_Type fwCallbackTimeout;
Timeout_Type tFwTcpDataOut;
Timeout_Type tFwStateTimeout;
uint8_t fSendBuff[64];

CFG_PROTOCOL_TYPE GPRS_ProtoRecv;
CFG_PROTOCOL_TYPE GPRS_ProtoSend;
PARSER_PACKET_TYPE GPRS_parserPacket;
uint8_t fFlagSendData = 0;
uint16_t fDataLength = 0;
uint32_t fCnt;
uint32_t fwSwitchServer = 0;



uint8_t FirmwareTask_IsBusy(void)
{
		if(firmwareStatus != 0xFFFFFFFF && (CheckTimeout(&tFirmwareTryToUpdate) != SYSTICK_TIMEOUT))
			return 1;
		return 0;
}

void FIRMWARE_Task(void)
{
	if(firmwareStatus != 0xFFFFFFFF
		&& (CheckTimeout(&tFrimwareUpdateTimeout) == SYSTICK_TIMEOUT)
	)
	{
		FW_DBG("FW_SVR: Firmware update timeout\r\n");
		firmwareStatus = 0xFFFFFFFF;
		firmwareFileOffSet = 0;
		InitTimeout(&tFrimwareUpdateTimeout,SYSTICK_TIME_SEC(10));
		fSendBuff[0] = 0;
		CFG_Save();
		InitTimeout(&fdummySendPacket,SYSTICK_TIME_SEC(300));
	}
}


void FIRMWARE_Init(uint32_t priority)
{
	fwConn = NULL;
	fwState = INITIAL;
	InitTimeout(&tFrimwareUpdateTimeout,SYSTICK_TIME_SEC(300));
	InitTimeout(&fwCallbackTimeout,SYSTICK_TIME_SEC(10));
	InitTimeout(&tFwTcpDataOut,SYSTICK_TIME_SEC(10));
	InitTimeout(&tFwStateTimeout,SYSTICK_TIME_SEC(10));
}


void FIRMWARE_Reset(void)
{
	fwConn = NULL;
	fwState = INITIAL;
	InitTimeout(&fwCallbackTimeout,SYSTICK_TIME_SEC(10));
	InitTimeout(&tFwTcpDataOut,SYSTICK_TIME_SEC(10));
}

TCP_STATE_TYPE FIRMWARE_Manage(void)
{ 
	uint8_t i;
	uint32_t addr0,addr1,addr2,addr3;
	switch(fwState)
	{
		case INITIAL:
			if(1)	
			{
					if(Domain_IpCheck((uint8_t *)sysCfg.priFserverName) == IS_IP)
					{
						sscanf((char *)sysCfg.priFserverName,"%d.%d.%d.%d",&addr0,&addr1,&addr2,&addr3);
						fwServerIpAddr.u8[0] = addr0;
						fwServerIpAddr.u8[1] = addr1;
						fwServerIpAddr.u8[2] = addr2;
						fwServerIpAddr.u8[3] = addr3;
						fwServerIp = &fwServerIpAddr;
					}
					else
					{
							fwServerIp = RESOLVER_Lookup((char *)sysCfg.priFserverName);
							if((fwServerIp == NULL && (CheckTimeout(&tFwStateTimeout) == SYSTICK_TIMEOUT)) || (SysTick_Get() - fwCheckDns >= 180000))
							{
								InitTimeout(&tFwStateTimeout,SYSTICK_TIME_SEC(10));
								fwCheckDns = SysTick_Get();
								resolv_query((const char *)sysCfg.priFserverName);
							}else if(fwServerIp
											&& fwServerIp->u8[0] == 0
											&& fwServerIp->u8[1] == 0
											&& fwServerIp->u8[2] == 0
											&& fwServerIp->u8[3] == 0
							){
									if((SysTick_Get() - fwCheckDns >= 10000)){
										fwCheckDns = SysTick_Get();
									 resolv_query((const char *)sysCfg.priFserverName);
									}
									break;
							 }
					}
					fwServerPort = sysCfg.priFserverPort;
			}
			else
			{
				if(Domain_IpCheck((uint8_t *)sysCfg.secFserverName) == IS_IP)
					{
						sscanf((char *)sysCfg.priFserverName,"%d.%d.%d.%d",&addr0,&addr1,&addr2,&addr3);
						fwServerIpAddr.u8[0] = addr0;
						fwServerIpAddr.u8[1] = addr1;
						fwServerIpAddr.u8[2] = addr2;
						fwServerIpAddr.u8[3] = addr3;
						fwServerIp = &fwServerIpAddr;
					}
					else
					{
							fwServerIp = RESOLVER_Lookup((char *)sysCfg.secFserverName);
							if((fwServerIp == NULL && (CheckTimeout(&tFwStateTimeout) == SYSTICK_TIMEOUT)) || (SysTick_Get() - fwCheckDns >= 180000))
							{
								InitTimeout(&tFwStateTimeout,SYSTICK_TIME_SEC(10));
								fwCheckDns = SysTick_Get();
								resolv_query((const char *)sysCfg.secFserverName);
							}else if(fwServerIp
											&& fwServerIp->u8[0] == 0
											&& fwServerIp->u8[1] == 0
											&& fwServerIp->u8[2] == 0
											&& fwServerIp->u8[3] == 0
							){
									if((SysTick_Get() - fwCheckDns >= 10000)){
										fwCheckDns = SysTick_Get();
									 resolv_query((const char *)sysCfg.secFserverName);
									}
									break;
							 }
					}
					fwServerPort = sysCfg.secFserverPort;
			}
			
//			fwServerIp = &fwServerIpAddr;
//			fwServerIp->u8[0] = 117;
//			fwServerIp->u8[1] = 2;
//			fwServerIp->u8[2] = 3;
//			fwServerIp->u8[3] = 40;
//			fwServerIp->u8[0] = DEFAULT_FSERVER_IP0;
//			fwServerIp->u8[1] = DEFAULT_FSERVER_IP1;
//			fwServerIp->u8[2] = DEFAULT_FSERVER_IP2;
//			fwServerIp->u8[3] = DEFAULT_FSERVER_IP3;

			//fwServerPort = 50000;
			
			if(fwServerIp == NULL) 
				break;
			else if(fwServerIp->u8[0] == 0 
			&& fwServerIp->u8[1]== 0 
			&& fwServerIp->u8[2] == 0
			&& fwServerIp->u8[3] == 0
			)
			{
				//CFG_Load();
				break;
			}
			fwLoggedIn = 0;
			fwConn = NULL;
			FW_DBG("\r\nFW_SVR: Started, Server %d.%d.%d.%d:%d\r\n", ((uint8_t*)(fwServerIp))[0], ((uint8_t*)(fwServerIp))[1], 
					((uint8_t*)(fwServerIp))[2], ((uint8_t*)(fwServerIp))[3], fwServerPort);
			fwConn = tcp_connect(fwServerIp, uip_htons(fwServerPort),FIRMWARE_Callback,NULL);
			fwTick = SysTick_Get();
			if(fwConn == NULL)
			{
				FW_DBG("\r\nFW_SVR: uip_connect returns NULL\r\n");
				fwState = WAIT_TIMEOUT_RECONNECT;
				break;
			}
			
			memcpy((char  *)fInfo.imei,(char  *)sysCfg.imei,18);
			memcpy((char  *)fInfo.id,(char  *)sysCfg.id,18);
			memcpy((char  *)fInfo.ver,FIRMWARE_VERSION,sizeof(FIRMWARE_VERSION));
			loginSendBuff[1] = sizeof(INFO_FIRMWARE_TYPE);
			loginSendBuff[0] = 0xCA;
			loginSendBuff[2] = 0;
			loginSendBuff[3] = 0x77;
			memcpy((char *)&loginSendBuff[4],(char  *)&fInfo,sizeof(INFO_FIRMWARE_TYPE));
			loginSendBuff[loginSendBuff[1] + 4] = CfgCalcCheckSum((uint8_t *)&fInfo,sizeof(INFO_FIRMWARE_TYPE));
			
			InitTimeout(&fdummySendPacket,SYSTICK_TIME_SEC(10));
			//InitTimeout(&tFrimwareUpdateTimeout,SYSTICK_TIME_SEC(30));
			fwState = CONNECT;
			break;
		case CONNECT:
		case WAIT_TIMEOUT_RECONNECT:
			if(SysTick_Get() - fwTick >= FW_RECONNECT_TIMEOUT)
			{
				fwTick = SysTick_Get();
				FIRMWARE_Reset();
			}
			break;
		case CONNECTED:
			if(CheckTimeout(&fwCallbackTimeout) == SYSTICK_TIMEOUT)
			{
				FIRMWARE_Init(1);
				fwState = INITIAL;
				fwTimedOutCnt = 0;
				fwRexmitCnt = 0;
			}
		break;
		default:
			
			break;
	}
	
	return fwState;
}

void FIRMWARE_Callback(void)
{
	uint32_t i;
	uint8_t *u8pt;
	if(uip_conn != fwConn)
		return;
	fwCallBackCnt++;
	InitTimeout(&fwCallbackTimeout,SYSTICK_TIME_SEC(10));
	if(uip_connected())
	{
		FW_DBG("FW_SVR: connected\r\n");
		fwTimedOutCnt = 0;
		fwRexmitCnt = 0;
		fwLen = 0;
		fwState = CONNECTED;
	}
	
	if(uip_poll())
	{
		if(CheckTimeout(&tFwTcpDataOut) == SYSTICK_TIMEOUT)
			fwLen = 0;
		
		if(fwLen == 0)
		{
			InitTimeout(&tFwTcpDataOut,SYSTICK_TIME_SEC(10));
			if(fwLoggedIn)
			{
				if(fSendBuff[0] == 0xCA)
				{
					if(fFlagSendData)
					{
							InitTimeout(&tFrimwareUpdateTimeout,SYSTICK_TIME_SEC(300));
					}
					
					if((CheckTimeout(&fdummySendPacket) == SYSTICK_TIMEOUT) || fFlagSendData)
					{
						fFlagSendData = 0;
						fwLen = fSendBuff[1] + 5;
						memcpy(fwBuf,fSendBuff,fwLen);
						InitTimeout(&fdummySendPacket,SYSTICK_TIME_SEC(10));
					}
				}
				else if(CheckTimeout(&fdummySendPacket) == SYSTICK_TIMEOUT)
				{
					InitTimeout(&fdummySendPacket,SYSTICK_TIME_SEC(300));
					fwLen = loginSendBuff[1] + 5;
					memcpy(fwBuf,loginSendBuff,fwLen);
				}
			}
			else
			{
				if(firmwareStatus == 0xA5A5A5A5)
				{
					if((firmwareFileOffSet < firmwareFileSize) && (firmwareFileSize <= FIRMWARE_SIZE_MAX))
					{
						fSendBuff[0] = 0xCA;
						fSendBuff[1] = 4;
						fSendBuff[2] = 0;
						fSendBuff[3] = 0x12;
						u8pt = (uint8_t *)&firmwareFileOffSet;
						fSendBuff[4] = u8pt[0];
						fSendBuff[5] = u8pt[1];
						fSendBuff[6] = u8pt[2];
						fSendBuff[7] = u8pt[3];
						fSendBuff[8] = CfgCalcCheckSum(&fSendBuff[4],4);
					}
				}
				
				fwLen = loginSendBuff[1] + 5;
				memcpy(fwBuf,loginSendBuff,fwLen);
				InitTimeout(&tFrimwareUpdateTimeout,SYSTICK_TIME_SEC(300));
				fwLoggedIn = 1;
			}
			if(fwLen)
			{
				uip_send(fwBuf,fwLen);
				FW_DBG("FW_SVR: sending %d bytes",fwLen);
			}
		}
	}
	
	if(uip_newdata())
	{
		FW_DBG("FW_SVR: Got New Data:%d bytes",uip_len);
			if(fFlagSendData == 0 && uip_len)
			{
				GPRS_ProtoRecv.dataPt = ((uint8_t *)uip_appdata);
				GPRS_parserPacket.lenMax = GPRS_PACKET_SIZE;
				GPRS_parserPacket.state = CFG_CMD_WAITING_SATRT_CODE;
				for(i = 0;i < uip_len;i++)
				{
					if(CfgParserPacket(&GPRS_parserPacket,&GPRS_ProtoRecv,((uint8_t *)uip_appdata)[i]) == 0)
					{
						FW_DBG("FW_SVR: Parser Packet->OK\r\n");
						smsCheckTime = SysTick_Get();
						InitTimeout(&tFrimwareUpdateTimeout,SYSTICK_TIME_SEC(300));
						break;
					}

				}
				for(;i < uip_len;i++) 
				{
						((uint8_t *)uip_appdata)[i] = 0;
				}
				CfgProcessData(&GPRS_ProtoRecv,&GPRS_ProtoSend,((uint8_t *)uip_appdata),GPRS_parserPacket.lenMax - 4,0);
				if(GPRS_ProtoSend.length)
				{
					FW_DBG("FW_SVR: Send %d bytes to server\r\n",GPRS_ProtoSend.length);
					fSendBuff[0] = GPRS_ProtoSend.start;
					fSendBuff[1] = ((uint8_t *)&GPRS_ProtoSend.length)[0];
					fSendBuff[2] = ((uint8_t *)&GPRS_ProtoSend.length)[1];
					fSendBuff[3] = GPRS_ProtoSend.opcode;
					for(i = 0;i < GPRS_ProtoSend.length;i++)
						fSendBuff[i+4] = GPRS_ProtoSend.dataPt[i];
					fSendBuff[i+4] = GPRS_ProtoSend.crc;
					GPRS_ProtoSend.length = 0;
					fFlagSendData = 1;
				}
			}
			InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(TCP_BUSY_TIMEOUT));
			InitTimeout(&tFirmwareTryToUpdate,SYSTICK_TIME_SEC(180));
			InitTimeout(&fdummySendPacket,SYSTICK_TIME_SEC(10));
	}	
	if(uip_acked())
	{
		FW_DBG("FW_SVR: sent:%d byte(s)\r\n",fwLen);
		fwLen = 0;
		fwTimedOutCnt = 0;
		fwRexmitCnt = 0;
		tcpIpTryCnt = 0;
		InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(PPP_RESET_TIMEOUT));
	}
	
	if(uip_rexmit())
	{
		uip_send(fwBuf, fwLen);
		FW_DBG("FW_SVR: rexmit %d bytes\r\n",fwLen);
		fwRexmitCnt ++;
		if(fwRexmitCnt >= 3) 
		{
			fwRexmitCnt = 0;
			fwLen = 0;
			fwState = WAIT_TIMEOUT_RECONNECT;
			InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(10));
		}
	}
	
	if(uip_closed() || uip_aborted())
	{
		fwConn = NULL;
		fwTick = SysTick_Get();
		FW_DBG("FW_SVR: timedout|closed|aborted %d\r\n", fwTimedOutCnt);
		fwState = WAIT_TIMEOUT_RECONNECT;		
	}
}


