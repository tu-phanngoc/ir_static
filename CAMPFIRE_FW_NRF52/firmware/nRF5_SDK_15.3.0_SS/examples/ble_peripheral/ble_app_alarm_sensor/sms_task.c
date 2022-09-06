
#include <string.h>
#include <ctype.h>
#include "system_config.h"
#include "lib/sparser.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "lib/ampm_list.h"
#include "lib/encoding.h"
#include "ampm_gsm_sms.h"
#include "ampm_gsm_call.h"
#include "ampm_gsm_main_task.h"
#include "ppp.h"
#include "sms_task.h"
#include "db.h"
#include "fire_alarm_message_create.h"
#include "app_config_task.h"
#define INFO(...)	DbgCfgPrintf(ALL_LOG,__VA_ARGS__)

#define smsScanf	sscanf

struct SMS_LIST_TYPE forwardSms;
struct SMS_LIST_TYPE replySms;
extern void MeterSetComCfg(void);
uint8_t smsSendBuff[256];
extern uint32_t bootloader_start(void);

uint8_t forwardToNumber[16];
uint8_t lastRecvSmsPhone[16];
uint8_t flag_set = 0;

void all_sms_init(void)
{
	memset(&replySms,sizeof(struct SMS_LIST_TYPE),0);

}
void io_process_action_task(uint32_t rtc)
{
	if((replySms.flag != SMS_MSG_SENT)
		&& (replySms.flag != SMS_MSG_FAIL)
	)
	{	
		if((replySms.msg == NULL) 
			|| (replySms.phone == NULL )
			|| (Ampm_SmsCheckMessage_IsEmpty())
			|| (!Ampm_SMS_IsInProcess(&replySms))
		)
		{
			replySms.flag = SMS_MSG_SENT;
		}
	}
  if(incomingCall.state == CALL_INCOMING)
	{
			if(ampm_GotIncomingNumberFlag)
			{
				Ampm_VoiceCallSetAction(&incomingCall,CALL_PICKUP_WHEN_INCOMING);
			}
	}
}

void Ampm_MainSmsRecvCallback(uint8_t *buf)
{
	uint16_t len;
	CMD_CfgParse((char *)buf,smsSendBuff,sizeof(smsSendBuff),&len,1);
	if(len)
	{
		mainBuf[0] = 0;
		if(flag_set)
			strcat((char *)mainBuf,"SET:");
		else
			strcat((char *)mainBuf,"GET:");
		strcat((char *)mainBuf,(char *)smsSendBuff);
		mainBuf[strlen((char *)mainBuf) - 1] = '.';
		Ampm_Sms_SendMsg(&replySms,smsSender,(uint8_t *)mainBuf,len,SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
	}
}


uint16_t CMD_CfgParse(char *buff,uint8_t *smsSendBuff,uint32_t smsLenBuf,uint16_t *dataOutLen,uint8_t pwdCheck)
{
	uint8_t smsLen = 0,c,u8Flag;
	char *pt,tempBuff0[128],tempBuff1[16],tempBuff2[16],i,flagCfgSave = 0;
	uint32_t t1,t2,t3,t4,t5,t6;
	uint16_t u16Temp;
	uint8_t replySmsBuf[160];
	float f0;
	DATE_TIME time;
	uint8_t pass = 0;
	uint32_t len;
	*dataOutLen = 0;
	
	len = strlen((char *)forwardToNumber);
	if(len < 6)
		len = 3;
	else
		len = 6;
	
	if(forwardToNumber[0] 
	&& Ampm_ComparePhoneNumber_1((char *)forwardToNumber,(char *)smsSender,len))
	{
		forwardToNumber[0] = 0;
		strcpy((char *)replySmsBuf,buff);
		replySmsBuf[sizeof(replySmsBuf) - 1] = 0;
		smsLen = strlen((char *)replySmsBuf);
		replySmsBuf[smsLen] = 0;
		Ampm_Sms_SendMsg(&replySms,lastRecvSmsPhone,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
		return 0xff;
	}
//	FireAlarmCreateSmsPassword("+8401695039433",(char *)sysCfg.,tempBuff1);
	if(pwdCheck)
	{
		FireAlarmCreateSmsPassword((char *)smsSender,(char *)sysCfg.id,(char *)sysCfg.deviceKey,(uint8_t *)tempBuff1);
		INFO("\n\rSMS->PASSWORD:%s\n\r",tempBuff1);
	}
	pt = strstr(buff,"SET,");
	if(pt != NULL)
	{
			flag_set = 1;
			// compare with saved password here
			smsScanf(pt,"SET,%s",tempBuff0);
			if(pwdCheck && (memcmp(tempBuff0, (char *)tempBuff1,strlen((char *)tempBuff1)) != 0))
			{
					INFO("\n\rSMS->PASSWORD FAILS\n\r");
						return 1;
			}
			INFO("\n\rSMS->PASSWORD OK\n\r");
			pass = 1;
	}
	
	//GET
	pt = strstr(buff,"GET,");
	if(pt != NULL)
	{
			flag_set = 0;
			// compare with saved password here
			smsScanf(pt,"GET,%s",tempBuff0);
			if(pwdCheck && (memcmp(tempBuff0, (char *)tempBuff1,strlen((char *)tempBuff1)) != 0))
			{
					INFO("\n\rSMS->PASSWORD FAILS\n\r");
						return 1;
			}
			INFO("\n\rSMS->PASSWORD OK\n\r");
			pass = 1;
	}

	if(pass == 0) return 1;
	len = 0;
	
	
	
	//set default config
	if(flag_set)
	{
		//USER PHONE NUMBER
		pt = strstr(buff,"USER=");
		if(pt != NULL)
		{
			smsScanf(pt,"USER=%[^ ,:\t\n\r#]",tempBuff0);
			strcpy((char *)sysCfg.bossPhoneNum,tempBuff0);
			flagCfgSave = 1;
			INFO("\n\rCONFIG->USER=%s\n\r", sysCfg.bossPhoneNum);
			len += sprintf((char *)&smsSendBuff[len], "USER=%s,", sysCfg.bossPhoneNum);
		}
		
		pt = strstr(buff,"PASS=");
		if(pt != NULL)
		{
			sscanf(pt,"PASS=%s",tempBuff0);
			strcpy((char *)sysCfg.customerId,tempBuff0);
			INFO("\n\rCONFIG->PASS=%s\r\n",sysCfg.customerId);
			len += sprintf((char *)&smsSendBuff[len],"PASS=%s,",sysCfg.customerId);
			flagCfgSave = 1;
		}
		
		pt = strstr(buff,"ADDR=");
		if(pt != NULL)
		{
			sscanf(pt,"ADDR=%d.%d.%d.%d",&t1,&t2,&t3,&t4);
			pt = (char *)&sysCfg.radio_access_addr;
			pt[3] = t1;
			pt[2] = t2;
			pt[1] = t3;
			pt[0] = t4;
			flagCfgSave = 1;
			INFO("\n\rCONFIG->ADDR=%d.%d.%d.%d\n\r",pt[3],pt[2],pt[1],pt[0]);
			len += sprintf((char *)&smsSendBuff[len],"ADDR=%d.%d.%d.%d,",pt[3],pt[2],pt[1],pt[0]);
		}
		pt = strstr(buff,"BOOTMODE");
		if(pt != NULL)
		{
			bootloader_start();
		}
		//change Server Name
		pt = strstr(buff,"DOMAIN=");
		if(pt != NULL)
		{
			sscanf(pt,"DOMAIN=%[^ ,:]:%d",tempBuff0,&t1);
			if(strlen(tempBuff0) < sizeof(sysCfg.priDserverName))
			{
				memcpy((char *)sysCfg.priDserverName,tempBuff0,CONFIG_SIZE_SERVER_NAME);
				sysCfg.priDserverPort = t1;
				INFO("\n\rCONFIG->DOMAIN=%s:%d\n\r",(char *)sysCfg.priDserverName,sysCfg.priDserverPort);
				len += sprintf((char *)&smsSendBuff[len],"DOMAIN=%s:%d,",(char *)sysCfg.priDserverName,sysCfg.priDserverPort);
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"ERROR_MAX_SIZE\r\n");
			}
		}
		
		//change Server Name
		pt = strstr(buff,"DOMAIN2=");
		if(pt != NULL)
		{
			sscanf(pt,"DOMAIN2=%[^ ,:]:%d",tempBuff0,&t1);
			if(strlen(tempBuff0) < sizeof(sysCfg.secDserverName))
			{
				memcpy((char *)sysCfg.secDserverName,tempBuff0,CONFIG_SIZE_SERVER_NAME);
				sysCfg.secDserverPort = t1;
				INFO("\n\rCONFIG->DOMAIN2=%s:%d\n\r",(char *)sysCfg.secDserverName,sysCfg.secDserverPort);
				len += sprintf((char *)&smsSendBuff[len],"DOMAIN2=%s:%d,",(char *)sysCfg.secDserverName,sysCfg.secDserverPort);
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"ERROR_MAX_SIZE\r\n");
			}
		}
		
		//change Server Name
		pt = strstr(buff,"FIRMWARE=");
		if(pt != NULL)
		{
			sscanf(pt,"FIRMWARE=%[^ ,:]:%d",tempBuff0,&t1);
			if(strlen(tempBuff0) < sizeof(sysCfg.priFserverName))
			{
				memcpy((char *)sysCfg.priFserverName,tempBuff0,CONFIG_SIZE_SERVER_NAME);
				sysCfg.priFserverPort = t1;
				INFO("\n\rCONFIG->FIRMWARE=%s:%d\n\r",(char *)sysCfg.priFserverName,sysCfg.priFserverPort);
				len += sprintf((char *)&smsSendBuff[len],"FIRMWARE=%s:%d,",(char *)sysCfg.priFserverName,sysCfg.priFserverPort);
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"ERROR_MAX_SIZE\r\n");
			}
		}
		
		//change Server Name
		pt = strstr(buff,"FIRMWARE2=");
		if(pt != NULL)
		{
			sscanf(pt,"FIRMWARE2=%[^ ,:]:%d",tempBuff0,&t1);
			if(strlen(tempBuff0) < sizeof(sysCfg.secFserverName))
			{
				memcpy((char *)sysCfg.secFserverName,tempBuff0,CONFIG_SIZE_SERVER_NAME);
				sysCfg.secFserverPort = t1;
				INFO("\n\rCONFIG->FIRMWARE2=%s:%d\n\r",(char *)sysCfg.secFserverName,sysCfg.secFserverPort);
				len += sprintf((char *)&smsSendBuff[len],"FIRMWARE2=%s:%d,",(char *)sysCfg.secFserverName,sysCfg.secFserverPort);
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"ERROR_MAX_SIZE\r\n");
			}
		}
		
		//FREQ
		pt = strstr(buff,"FREQ=");
		if(pt)
		{
			sscanf(pt,"FREQ=%[^ ,\t\n\r]",tempBuff0);
			if(strlen(tempBuff0) < 10)
			{
				sscanf(tempBuff0,"%d",&t1);
				sysCfg.freq = t1;
				INFO("\n\rCONFIG->FREQ=%s:%d\n\r",sysCfg.freq);
				len += sprintf((char *)&smsSendBuff[len],"FREQ=%d,",sysCfg.freq);
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"ERROR_MAX_SIZE\r\n");
			}
		}
		
		//FREQ2
		pt = strstr(buff,"FREQ2=");
		if(pt)
		{
			sscanf(pt,"FREQ2=%[^ ,\t\n\r]",tempBuff0);
			if(strlen(tempBuff0) < 10)
			{
				sscanf(tempBuff0,"%d",&t1);
				sysCfg.freq2 = t1;
				INFO("\n\rCONFIG->FREQ1=%s:%d\n\r",sysCfg.freq2);
				len += sprintf((char *)&smsSendBuff[len],"FREQ2=%d,",sysCfg.freq2);
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"ERROR_MAX_SIZE\r\n");
			}
		}

		pt = strstr(buff,"TRACE=0");
		if(pt)
		{
			INFO("\n\rCONFIG->TRACE=0\n\r");
			len += sprintf((char *)&smsSendBuff[len],"TRACE=0,");
			sysCfg.feature &= ~FEATURE_TRACE_ENABLE;
			flagCfgSave = 1;
		}
		pt = strstr(buff,"TRACE=1");
		if(pt)
		{
			INFO("\n\rCONFIG->TRACE=1\n\r");
			len += sprintf((char *)&smsSendBuff[len],"TRACE=1,");
			sysCfg.feature |= FEATURE_TRACE_ENABLE;
			flagCfgSave = 1;
		}
		//VINLBV
		pt = strstr(buff,"VINLBV");
		if(pt)
		{
			sscanf(pt,"VINLBV=%[^ ,\t\n\r]",tempBuff0);
			if(strlen(tempBuff0) < 7)
			{
				sscanf(tempBuff0,"%f",&f0);
				sysCfg.vinlbv = f0;
				INFO("\n\rCONFIG->VINLBV=%f\n\r",sysCfg.vinlbv);
				len += sprintf((char *)&smsSendBuff[len],"VINLBV=%f,",sysCfg.vinlbv);
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"ERROR_MAX_SIZE\r\n");
			}
		}

		//change gprs apn
		pt = strstr(buff,"APN");
		if(pt != NULL)
		{
			memset(tempBuff0,0,sizeof(tempBuff0));
			smsScanf(pt,"APN=%[^ ,\r\n]%[^ ,\r\n]%[^ ,\r\n]",tempBuff0,tempBuff1,tempBuff2);
			memcpy((char *)sysCfg.gprsApn,tempBuff0,CONFIG_SIZE_GPRS_APN);
			memcpy((char *)sysCfg.gprsUsr,tempBuff1,CONFIG_SIZE_GPRS_USR);
			memcpy((char *)sysCfg.gprsPwd,tempBuff2,CONFIG_SIZE_GPRS_PWD);
			sysCfg.gprsApn[CONFIG_SIZE_GPRS_APN - 1] = 0;
			sysCfg.gprsUsr[CONFIG_SIZE_GPRS_USR - 1] = 0;
			sysCfg.gprsPwd[CONFIG_SIZE_GPRS_PWD - 1] = 0;
			INFO("\n\rAPN=%s %s %s\n\r", sysCfg.gprsApn,sysCfg.gprsUsr,sysCfg.gprsPwd);
			len += sprintf((char *)&smsSendBuff[len], "APN=%s %s %s,", sysCfg.gprsApn,sysCfg.gprsUsr,sysCfg.gprsPwd);
			Ampm_GsmSetApn(sysCfg.gprsApn);
			PPP_SetAuthentication((int8_t *)sysCfg.gprsUsr, (int8_t *)sysCfg.gprsPwd);
			flagCfgSave = 1;
		}
		
		//set default config
		pt = strstr(buff,"FORMAT=1");
		if(pt != NULL)
		{
			//strcpy((char *)tempBuff0, (char *)sysCfg.id);
			memset((void*)&sysCfg, 0xFF, sizeof sysCfg);
			CFG_Save();
			CFG_Load();
			//strcpy((char *)sysCfg.id ,(char *)tempBuff0);
			//CFG_Save();
			INFO("\n\rSMS:Set default config\n\r");
			len += sprintf((char *)&smsSendBuff[len], "FORMAT=1,");
			ResetMcuSet(MCU_RESET_AFTER_30_SEC);
		}
		
		//reset device
		pt = strstr(buff,"REBOOT=1");
		if(pt != NULL)
		{
			ResetMcuSet(MCU_RESET_AFTER_30_SEC);
			len += sprintf((char *)&smsSendBuff[len], "REBOOT=1,");
		}
		
//		//check monney
//		pt = strstr(buff,"NAPTIEN=");
//		if(pt != NULL)
//		{
//			sscanf(pt,"NAPTIEN=%[^ ,\t\n\r]",tempBuff0);
//			Ampm_SendCommand("+CUSD: ",modemError,5000,3,"AT+CUSD=1,\"%s\",15\r",tempBuff0);
//			SysTick_DelayMs(5000);
//			t1 = 0;
//			while(RINGBUF_Get(commRxRingBuf, &replySmsBuf[t1])==0)
//			{
//				if(t1 < 16 && (replySmsBuf[t1] == '"'))
//					t1 = 0;
//				else
//					t1++;
//				if(t1 >= sizeof(replySmsBuf)) break;
//			}
//			while(t1)
//			{
//				if(replySmsBuf[t1] == '"')
//				{
//					break;
//				}
//				t1--;
//			}
//			replySmsBuf[t1] = 0;
//			Ampm_Sms_SendMsg(&replySms,smsSender,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
//		}
		
		//check monney
		pt = strstr(buff,"NAPTIEN=");
		if(pt != NULL)
		{
			if(Ampm_SendCommand("OK",modemError,1000,3,"AT\r")== AMPM_GSM_RES_OK)
			{
				if(Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"UCS2\"\r")== AMPM_GSM_RES_OK)
				{
					Ampm_SendCommand("OK",modemError,1000,3,"AT+CUSD=1\r");
					sscanf(pt,"NAPTIEN=%[^ ,\t\n\r]",tempBuff0);
					if(Ampm_SendCommand("+CUSD:",modemError,10000,3,"ATD%s;\r",tempBuff0) == AMPM_GSM_RES_OK)
					{
						SysTick_DelayMs(1000);
						t1 = 0;
						u16Temp = 0;
						replySmsBuf[0] = 0;
						u8Flag = 0;
						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(c == ',')
							{
								break;
							}
						}

						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(u8Flag == 0)
							{
								u8Flag = 1;
								if(c == '"')
									RINGBUF_Get(commRxRingBuf, &c);
							}
							if(c == ',' || c == '"')
							{
								break;
							}
							else
							{
								if(c >= '0' && c <= '9')
									c = c - '0';
								else if(c >= 'A' && c <= 'F')
									c = 0x0A + (c - 'A');
								else
									break;
								switch(t1)
								{
									case 0:
										u16Temp = c;
										t1 = 1;
									break;
									case 1:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 2;
									break;
									case 2:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 3;
									break;
									case 3:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 0;	
									
										ucs2_to_utf8(u16Temp,tempBuff2);
										strcat(replySmsBuf,tempBuff2);
									break;
									
								}
							}				
						}
						Ampm_RingingReset();
						Ampm_Sms_SendMsg(&replySms,smsSender,(uint8_t *)replySmsBuf,strlen(replySmsBuf),SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
					}
				}
				Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"GSM\"\r");
			}
		}
		
		//send sms
		pt = strstr(buff,"SMSSEND=");
		if(pt != NULL)
		{
			strcpy((char *)lastRecvSmsPhone,(char *)smsSender);
			smsScanf(pt,"SMSSEND=%[^, :\t\n\r#]",forwardToNumber);

			strcpy((char *)mainBuf,(pt + strlen((char *)forwardToNumber) + 9));
			smsLen = strlen((char *)mainBuf);
			Ampm_Sms_SendMsg(&forwardSms,(uint8_t *)forwardToNumber,(uint8_t *)mainBuf,smsLen,SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
		}
		
		pt = strstr(buff,"SENSOR=NO");
		if(pt)
		{
			INFO("\n\rCONFIG->SENSOR NO\n\r");
			len += sprintf((char *)&smsSendBuff[len],"SENSOR=NO,");
			sysCfg.feature &= ~FEATURE_ALARM_IF_INPUT_1;
			flagCfgSave = 1;
		}
		pt = strstr(buff,"SENSOR=NC");
		if(pt)
		{
			INFO("\n\rCONFIG->SENSOR=NC\n\r");
			len += sprintf((char *)&smsSendBuff[len],"SENSOR=NC,");
			sysCfg.feature |= FEATURE_ALARM_IF_INPUT_1;
			flagCfgSave = 1;
		}
		
		pt = strstr(buff,"PULLUP=1");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->PULLUP=1\n\r");
			len += sprintf((char *)&smsSendBuff[len],"PULLUP=1,");
			sysCfg.feature |= FEATURE_U6_ENABLE;
			sysCfg.feature |= FEATURE_ALARM_IF_INPUT_1;
			flagCfgSave = 1;
		}
		
		pt = strstr(buff,"PULLUP=0");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->PULLUP=0\n\r");
			len += sprintf((char *)&smsSendBuff[len],"PULLUP=0,");
			sysCfg.feature &= ~FEATURE_U6_ENABLE;
			sysCfg.feature &= ~FEATURE_ALARM_IF_INPUT_1;
			flagCfgSave = 1;
		}
	}
	else
	{
		
		//USER PHONE NUMBER
		pt = strstr(buff,"USER");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->USER=%s\n\r", sysCfg.bossPhoneNum);
			len += sprintf((char *)&smsSendBuff[len], "USER=%s,", sysCfg.bossPhoneNum);
		}
		pt = strstr(buff,"PASS");
		if(pt != NULL)
		{
			sscanf(pt,"PASS=%s",tempBuff0);
			INFO("\n\rCONFIG->PASS=%s\r\n",sysCfg.customerId);
			len += sprintf((char *)&smsSendBuff[len],"PASS=%s,",sysCfg.customerId);
		}
		
		pt = strstr(buff,"ADDR");
		if(pt != NULL)
		{
			pt = (char *)&sysCfg.radio_access_addr;
			INFO("\n\rCONFIG->ADDR=%d.%d.%d.%d\n\r",pt[3],pt[2],pt[1],pt[0]);
			len += sprintf((char *)&smsSendBuff[len],"ADDR=%d.%d.%d.%d,",pt[3],pt[2],pt[1],pt[0]);
		}
		//IMEI
		pt = strstr(buff,"IMEI");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->IMEI=%s\n\r", sysCfg.imei);
			len += sprintf((char *)&smsSendBuff[len], "IMEI=%s,", sysCfg.imei);
		}
		//NAME
		pt = strstr(buff,"NAME");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->NAME=%s\n\r", sysCfg.id);
			len += sprintf((char *)&smsSendBuff[len], "NAME=%s,", sysCfg.id);
		}
		//change Server Name
		pt = strstr(buff,"DOMAIN");
		if(pt != NULL && pt[strlen("DOMAIN")] != 1)
		{
			INFO("\n\rCONFIG->DOMAIN=%s:%d\n\r",(char *)sysCfg.priDserverName,sysCfg.priDserverPort);
			len += sprintf((char *)&smsSendBuff[len],"DOMAIN=%s:%d,",(char *)sysCfg.priDserverName,sysCfg.priDserverPort);
		}
		
		//change Server Name
		pt = strstr(buff,"DOMAIN2");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->DOMAIN2=%s:%d\n\r",(char *)sysCfg.secDserverName,sysCfg.secDserverPort);
			len += sprintf((char *)&smsSendBuff[len],"DOMAIN2=%s:%d,",(char *)sysCfg.secDserverName,sysCfg.secDserverPort);
		}
		
		//change Server Name
		pt = strstr(buff,"FIRMWARE");
		if(pt != NULL && pt[strlen("FIRMWARE")] != 1)
		{
			INFO("\n\rCONFIG->FIRMWARE=%s:%d\n\r",(char *)sysCfg.priFserverName,sysCfg.priFserverPort);
			len += sprintf((char *)&smsSendBuff[len],"FIRMWARE=%s:%d,",(char *)sysCfg.priFserverName,sysCfg.priFserverPort);
		}
		
		//change Server Name
		pt = strstr(buff,"FIRMWARE2");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->FIRMWARE2=%s:%d\n\r",(char *)sysCfg.secFserverName,sysCfg.secFserverPort);
			len += sprintf((char *)&smsSendBuff[len],"FIRMWARE2=%s:%d,",(char *)sysCfg.secFserverName,sysCfg.secFserverPort);
		}
		
		//FREQ
		pt = strstr(buff,"FREQ");
		if(pt != NULL && pt[strlen("FREQ")] != 2)
		{
			INFO("\n\rCONFIG->FREQ=%s:%d\n\r",sysCfg.freq);
			len += sprintf((char *)&smsSendBuff[len],"FREQ=%d,",sysCfg.freq);
		}
		
		//FREQ2
		pt = strstr(buff,"FREQ2");
		if(pt)
		{
			INFO("\n\rCONFIG->FREQ2=%s:%d\n\r",sysCfg.freq2);
			len += sprintf((char *)&smsSendBuff[len],"FREQ2=%d,",sysCfg.freq2);
		}
		
		pt = strstr(buff,"TRACE");
		if(pt)
		{
			if(sysCfg.feature & FEATURE_TRACE_ENABLE)
			{
				INFO("\n\rCONFIG->TRACE=1\n\r");
				len += sprintf((char *)&smsSendBuff[len],"TRACE=1,");
			}
			else
			{
				INFO("\n\rCONFIG->TRACE=0\n\r");
				len += sprintf((char *)&smsSendBuff[len],"TRACE=0,");
			}
		}
		
		//VINLBV
		pt = strstr(buff,"VINLBV");
		if(pt)
		{
			INFO("\n\rCONFIG->VINLBV=%f\n\r",sysCfg.vinlbv);
			len += sprintf((char *)&smsSendBuff[len],"VINLBV=%f,",sysCfg.vinlbv);
		}

		//change gprs apn
		pt = strstr(buff,"APN");
		if(pt != NULL)
		{
			INFO("\n\rAPN=%s %s %s\n\r", sysCfg.gprsApn,sysCfg.gprsUsr,sysCfg.gprsPwd);
			len += sprintf((char *)&smsSendBuff[len], "APN=%s %s %s,", sysCfg.gprsApn,sysCfg.gprsUsr,sysCfg.gprsPwd);
		}
		
		//check monney
		pt = strstr(buff,"TKC");
		if(pt != NULL)
		{
			if(Ampm_SendCommand("OK",modemError,1000,3,"AT\r")== AMPM_GSM_RES_OK)
			{
				if(Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"UCS2\"\r")== AMPM_GSM_RES_OK)
				{
					Ampm_SendCommand("OK",modemError,1000,3,"AT+CUSD=1\r");
					if(Ampm_SendCommand("+CUSD:",modemError,10000,3,"ATD*101#;\r") == AMPM_GSM_RES_OK)
					{
						SysTick_DelayMs(1000);
						t1 = 0;
						u16Temp = 0;
						replySmsBuf[0] = 0;
						u8Flag = 0;
						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(c == ',')
							{
								break;
							}
						}

						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(u8Flag == 0)
							{
								u8Flag = 1;
								if(c == '"')
									RINGBUF_Get(commRxRingBuf, &c);
							}
							if(c == ',' || c == '"')
							{
								break;
							}
							else
							{
								if(c >= '0' && c <= '9')
									c = c - '0';
								else if(c >= 'A' && c <= 'F')
									c = 0x0A + (c - 'A');
								else
									break;
								switch(t1)
								{
									case 0:
										u16Temp = c;
										t1 = 1;
									break;
									case 1:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 2;
									break;
									case 2:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 3;
									break;
									case 3:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 0;	
									
										ucs2_to_utf8(u16Temp,tempBuff2);
										strcat(replySmsBuf,tempBuff2);
									break;
									
								}
							}				
						}
						Ampm_RingingReset();
						Ampm_Sms_SendMsg(&replySms,smsSender,(uint8_t *)replySmsBuf,strlen(replySmsBuf),SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
					}
				}
				Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"GSM\"\r");
			}
		}
		//check monney
		pt = strstr(buff,"TKKM");
		if(pt != NULL)
		{
			if(Ampm_SendCommand("OK",modemError,1000,3,"AT\r")== AMPM_GSM_RES_OK)
			{
				if(Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"UCS2\"\r")== AMPM_GSM_RES_OK)
				{
					Ampm_SendCommand("OK",modemError,1000,3,"AT+CUSD=1\r");
					if(Ampm_SendCommand("+CUSD:",modemError,10000,3,"ATD*102#;\r") == AMPM_GSM_RES_OK)
					{
						SysTick_DelayMs(1000);
						t1 = 0;
						u16Temp = 0;
						replySmsBuf[0] = 0;
						u8Flag = 0;
						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(c == ',')
							{
								break;
							}
						}

						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(u8Flag == 0)
							{
								u8Flag = 1;
								if(c == '"')
									RINGBUF_Get(commRxRingBuf, &c);
							}
							if(c == ',' || c == '"')
							{
								break;
							}
							else
							{
								if(c >= '0' && c <= '9')
									c = c - '0';
								else if(c >= 'A' && c <= 'F')
									c = 0x0A + (c - 'A');
								else
									break;
								switch(t1)
								{
									case 0:
										u16Temp = c;
										t1 = 1;
									break;
									case 1:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 2;
									break;
									case 2:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 3;
									break;
									case 3:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 0;	
									
										ucs2_to_utf8(u16Temp,tempBuff2);
										strcat(replySmsBuf,tempBuff2);
									break;
									
								}
							}				
						}
						Ampm_RingingReset();
						Ampm_Sms_SendMsg(&replySms,smsSender,(uint8_t *)replySmsBuf,strlen(replySmsBuf),SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
					}
				}
				Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"GSM\"\r");
			}
		}
		//SENSOR
		pt = strstr(buff,"SENSOR");
		if(pt)
		{
			if(sysCfg.feature & FEATURE_ALARM_IF_INPUT_1)
			{
				INFO("\n\rCONFIG->SENSOR=NC\n\r");
				len += sprintf((char *)&smsSendBuff[len],"SENSOR=NC,");
			}
			else
			{
				INFO("\n\rCONFIG->SENSOR=NO\n\r");
				len += sprintf((char *)&smsSendBuff[len],"SENSOR=NO,");
			}
		}
		
		//PULLUP
		pt = strstr(buff,"PULLUP");
		if(pt)
		{
			if(sysCfg.feature & FEATURE_U6_ENABLE)
			{
				INFO("\n\rCONFIG->PULLUP=1\n\r");
				len += sprintf((char *)&smsSendBuff[len],"PULLUP=1,");
			}
			else
			{
				INFO("\n\rCONFIG->PULLUP=0\n\r");
				len += sprintf((char *)&smsSendBuff[len],"PULLUP=0,");
			}
		}
		//SOFTVERSION
		pt = strstr(buff,"SOFTVERSION");
		if(pt)
		{
			INFO("\n\rCONFIG->SOFTVERSION=%s\n\r",sysCfg.fwVersion);
			len += sprintf((char *)&smsSendBuff[len],"SOFTVERSION=%s,",sysCfg.fwVersion);
		}
		//STA
		pt = strstr(buff,"STA");
		if(pt)
		{
			INFO("\n\rCONFIG->STA=%s\n\r",sysCfg.fwVersion);
			len += sprintf((char *)&smsSendBuff[len],"STA=%s,",sysCfg.fwVersion);
		}
	}
	
	
	if(len >= smsLenBuf)	len = smsLenBuf;
	
	smsSendBuff[len] = 0;
	*dataOutLen = len;
	
	if(flagCfgSave)
	{
		CFG_Save();
	}

	return 0;
}


uint16_t CMD_CfgParseServer(char *buff,uint8_t *smsSendBuff,uint32_t smsLenBuf,uint16_t *dataOutLen,uint8_t pwdCheck)
{
	uint8_t smsLen = 0,c,u8Flag;
	char *pt,tempBuff0[128],tempBuff1[16],tempBuff2[16],i,flagCfgSave = 0;
	uint8_t replySmsBuf[160];
	uint32_t t1,t2,t3,t4,t5,t6;
	float f0;
	uint16_t u16Temp;
	DATE_TIME time;
	uint8_t pass = 0;
	uint32_t len;
	*dataOutLen = 0;
	
	len = strlen((char *)forwardToNumber);
	if(len < 6)
		len = 3;
	else
		len = 6;
	
	if(forwardToNumber[0] 
	&& Ampm_ComparePhoneNumber_1((char *)forwardToNumber,(char *)smsSender,len))
	{
		forwardToNumber[0] = 0;
		strcpy((char *)replySmsBuf,buff);
		replySmsBuf[sizeof(replySmsBuf) - 1] = 0;
		smsLen = strlen((char *)replySmsBuf);
		replySmsBuf[smsLen] = 0;
		Ampm_Sms_SendMsg(&replySms,lastRecvSmsPhone,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
		return 0xff;
	}
//	FireAlarmCreateSmsPassword("+8401695039433",(char *)sysCfg.,tempBuff1);
	if(pwdCheck)
	{
		FireAlarmCreateSmsPassword((char *)smsSender,(char *)sysCfg.id,(char *)sysCfg.deviceKey,(char *)tempBuff1);
		INFO("\n\rSMS->PASSWORD:%s\n\r",tempBuff1);
	}
	pt = strstr(buff,"SET,");
	if(pt != NULL)
	{
			flag_set = 1;
			// compare with saved password here
			smsScanf(pt,"SET,%s",tempBuff0);
			if(pwdCheck && (memcmp(tempBuff0, (char *)tempBuff1,strlen((char *)tempBuff1)) != 0))
			{
					INFO("\n\rSMS->PASSWORD FAILS\n\r");
						return 1;
			}
			INFO("\n\rSMS->PASSWORD OK\n\r");
			pass = 1;
	}
	
	//GET
	pt = strstr(buff,"GET,");
	if(pt != NULL)
	{
			// compare with saved password here
			smsScanf(pt,"GET,%s",tempBuff0);
			if(pwdCheck && (memcmp(tempBuff0, (char *)tempBuff1,strlen((char *)tempBuff1)) != 0))
			{
					INFO("\n\rSMS->PASSWORD FAILS\n\r");
						return 1;
			}
			INFO("\n\rSMS->PASSWORD OK\n\r");
			pass = 1;
	}

	if(pass == 0) return 1;
	len = 0;
	
	
	
	//set default config
	if(flag_set)
	{
		//USER PHONE NUMBER
		pt = strstr(buff,"USER=");
		if(pt != NULL)
		{
			smsScanf(pt,"USER=%[^ ,:\t\n\r#]",tempBuff0);
			strcpy((char *)sysCfg.bossPhoneNum,tempBuff0);
			flagCfgSave = 1;
			INFO("\n\rCONFIG->USER=%s\n\r", sysCfg.bossPhoneNum);
			len += sprintf((char *)&smsSendBuff[len], "USER=1,");
		}
		
		//change Server Name
		pt = strstr(buff,"DOMAIN=");
		if(pt != NULL)
		{
			sscanf(pt,"DOMAIN=%[^ ,:]:%d",tempBuff0,&t1);
			if(strlen(tempBuff0) < sizeof(sysCfg.priDserverName))
			{
				memcpy((char *)sysCfg.priDserverName,tempBuff0,CONFIG_SIZE_SERVER_NAME);
				sysCfg.priDserverPort = t1;
				INFO("\n\rCONFIG->DOMAIN=%s:%d\n\r",(char *)sysCfg.priDserverName,sysCfg.priDserverPort);
				len += sprintf((char *)&smsSendBuff[len],"DOMAIN=1,");
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"DOMAIN=0,");
			}
		}
		
		//change Server Name
		pt = strstr(buff,"DOMAIN2=");
		if(pt != NULL)
		{
			sscanf(pt,"DOMAIN2=%[^ ,:]:%d",tempBuff0,&t1);
			if(strlen(tempBuff0) < sizeof(sysCfg.secDserverName))
			{
				memcpy((char *)sysCfg.secDserverName,tempBuff0,CONFIG_SIZE_SERVER_NAME);
				sysCfg.secDserverPort = t1;
				INFO("\n\rCONFIG->DOMAIN2=%s:%d\n\r",(char *)sysCfg.secDserverName,sysCfg.secDserverPort);
				len += sprintf((char *)&smsSendBuff[len],"DOMAIN2=1,");
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"DOMAIN2=0,");
			}
		}
		
		//change Server Name
		pt = strstr(buff,"FIRMWARE=");
		if(pt != NULL)
		{
			sscanf(pt,"FIRMWARE=%[^ ,:]:%d",tempBuff0,&t1);
			if(strlen(tempBuff0) < sizeof(sysCfg.priFserverName))
			{
				memcpy((char *)sysCfg.priFserverName,tempBuff0,CONFIG_SIZE_SERVER_NAME);
				sysCfg.priFserverPort = t1;
				INFO("\n\rCONFIG->FIRMWARE=%s:%d\n\r",(char *)sysCfg.priFserverName,sysCfg.priFserverPort);
				len += sprintf((char *)&smsSendBuff[len],"FIRMWARE=1,");
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"FIRMWARE=0,");
			}
		}
		
		//change Server Name
		pt = strstr(buff,"FIRMWARE2=");
		if(pt != NULL)
		{
			sscanf(pt,"FIRMWARE2=%[^ ,:]:%d",tempBuff0,&t1);
			if(strlen(tempBuff0) < sizeof(sysCfg.secFserverName))
			{
				memcpy((char *)sysCfg.secFserverName,tempBuff0,CONFIG_SIZE_SERVER_NAME);
				sysCfg.secFserverPort = t1;
				INFO("\n\rCONFIG->FIRMWARE2=%s:%d\n\r",(char *)sysCfg.secFserverName,sysCfg.secFserverPort);
				len += sprintf((char *)&smsSendBuff[len],"FIRMWARE2=1,");
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"FIRMWARE2=0,");
			}
		}
		
		//FREQ
		pt = strstr(buff,"FREQ=");
		if(pt)
		{
			sscanf(pt,"FREQ=%[^ ,\t\n\r]",tempBuff0);
			if(strlen(tempBuff0) < 10)
			{
				sscanf(tempBuff0,"%d",&t1);
				sysCfg.freq = t1;
				INFO("\n\rCONFIG->FREQ=%s:%d\n\r",sysCfg.freq);
				len += sprintf((char *)&smsSendBuff[len],"FREQ=1,");
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"FREQ=0,");
			}
		}
		
		//FREQ2
		pt = strstr(buff,"FREQ2=");
		if(pt)
		{
			sscanf(pt,"FREQ2=%[^ ,\t\n\r]",tempBuff0);
			if(strlen(tempBuff0) < 10)
			{
				sscanf(tempBuff0,"%d",&t1);
				sysCfg.freq2 = t1;
				INFO("\n\rCONFIG->FREQ1=%s:%d\n\r",sysCfg.freq2);
				len += sprintf((char *)&smsSendBuff[len],"FREQ2=1,");
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"FREQ2=0,");
			}
		}

		pt = strstr(buff,"TRACE=0");
		if(pt)
		{
			INFO("\n\rCONFIG->TRACE=0\n\r");
			len += sprintf((char *)&smsSendBuff[len],"TRACE=1,");
			sysCfg.feature &= ~FEATURE_TRACE_ENABLE;
			flagCfgSave = 1;
		}
		pt = strstr(buff,"TRACE=1");
		if(pt)
		{
			INFO("\n\rCONFIG->TRACE=1\n\r");
			len += sprintf((char *)&smsSendBuff[len],"TRACE=1,");
			sysCfg.feature |= FEATURE_TRACE_ENABLE;
			flagCfgSave = 1;
		}
		//VINLBV
		pt = strstr(buff,"VINLBV");
		if(pt)
		{
			sscanf(pt,"VINLBV=%[^ ,\t\n\r]",tempBuff0);
			if(strlen(tempBuff0) < 7)
			{
				sscanf(tempBuff0,"%f",&f0);
				sysCfg.vinlbv = f0;
				INFO("\n\rCONFIG->VINLBV=%f\n\r",sysCfg.vinlbv);
				len += sprintf((char *)&smsSendBuff[len],"VINLBV=1,");
				flagCfgSave = 1;
			}
			else
			{
				len += sprintf((char *)&smsSendBuff[len],"VINLBV=0,");
			}
		}

		//change gprs apn
		pt = strstr(buff,"APN");
		if(pt != NULL)
		{
			memset(tempBuff0,0,sizeof(tempBuff0));
			smsScanf(pt,"APN=%[^ ,\r\n]%[^ ,\r\n]%[^ ,\r\n]",tempBuff0,tempBuff1,tempBuff2);
			memcpy((char *)sysCfg.gprsApn,tempBuff0,CONFIG_SIZE_GPRS_APN);
			memcpy((char *)sysCfg.gprsUsr,tempBuff1,CONFIG_SIZE_GPRS_USR);
			memcpy((char *)sysCfg.gprsPwd,tempBuff2,CONFIG_SIZE_GPRS_PWD);
			sysCfg.gprsApn[CONFIG_SIZE_GPRS_APN - 1] = 0;
			sysCfg.gprsUsr[CONFIG_SIZE_GPRS_USR - 1] = 0;
			sysCfg.gprsPwd[CONFIG_SIZE_GPRS_PWD - 1] = 0;
			INFO("\n\rAPN=%s %s %s\n\r", sysCfg.gprsApn,sysCfg.gprsUsr,sysCfg.gprsPwd);
			len += sprintf((char *)&smsSendBuff[len], "APN=%s %s %s,", sysCfg.gprsApn,sysCfg.gprsUsr,sysCfg.gprsPwd);
			Ampm_GsmSetApn(sysCfg.gprsApn);
			PPP_SetAuthentication((int8_t *)sysCfg.gprsUsr, (int8_t *)sysCfg.gprsPwd);
			flagCfgSave = 1;
		}
		
		//set default config
		pt = strstr(buff,"FORMAT=1");
		if(pt != NULL)
		{
			//strcpy((char *)tempBuff0, (char *)sysCfg.id);
			memset((void*)&sysCfg, 0xFF, sizeof sysCfg);
			CFG_Save();
			CFG_Load();
			//strcpy((char *)sysCfg.id ,(char *)tempBuff0);
			//CFG_Save();
			INFO("\n\rSMS:Set default config\n\r");
			len += sprintf((char *)&smsSendBuff[len], "FORMAT=1,");
			ResetMcuSet(MCU_RESET_AFTER_30_SEC);
		}
		
		//reset device
		pt = strstr(buff,"REBOOT=1");
		if(pt != NULL)
		{
			ResetMcuSet(MCU_RESET_AFTER_30_SEC);
			len += sprintf((char *)&smsSendBuff[len], "REBOOT=1,");
		}
		
//		//check monney
//		pt = strstr(buff,"NAPTIEN=");
//		if(pt != NULL)
//		{
//			sscanf(pt,"NAPTIEN=%[^ ,\t\n\r]",tempBuff0);
//			Ampm_SendCommand("+CUSD: ",modemError,10000,1,"AT+CUSD=1,\"%s\",15\r",tempBuff0);
//			SysTick_DelayMs(5000);
//			t1 = 0;
//			while(RINGBUF_Get(commRxRingBuf, &replySmsBuf[t1])==0)
//			{
//				if(t1 < 16 && (replySmsBuf[t1] == '"'))
//					t1 = 0;
//				else
//					t1++;
//				if(t1 >= sizeof(replySmsBuf)) break;
//			}
//			while(t1)
//			{
//				if(replySmsBuf[t1] == '"')
//				{
//					break;
//				}
//				t1--;
//			}
//			replySmsBuf[t1] = 0;
//			Ampm_Sms_SendMsg(&replySms,smsSender,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
//		}

		pt = strstr(buff,"NAPTIEN=");
		if(pt != NULL)
		{
			if(Ampm_SendCommand("OK",modemError,1000,3,"AT\r")== AMPM_GSM_RES_OK)
			{
				if(Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"UCS2\"\r")== AMPM_GSM_RES_OK)
				{
					Ampm_SendCommand("OK",modemError,1000,3,"AT+CUSD=1\r");
					sscanf(pt,"NAPTIEN=%[^ ,\t\n\r]",tempBuff0);
					if(Ampm_SendCommand("+CUSD:",modemError,10000,3,"ATD%s;\r",tempBuff0) == AMPM_GSM_RES_OK)
					{
						SysTick_DelayMs(1000);
						t1 = 0;
						u16Temp = 0;
						replySmsBuf[0] = 0;
						u8Flag = 0;
						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(c == ',')
							{
								break;
							}
						}

						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(u8Flag == 0)
							{
								u8Flag = 1;
								if(c == '"')
									RINGBUF_Get(commRxRingBuf, &c);
							}
							if(c == ',' || c == '"')
							{
								break;
							}
							else
							{
								if(c >= '0' && c <= '9')
									c = c - '0';
								else if(c >= 'A' && c <= 'F')
									c = 0x0A + (c - 'A');
								else
									break;
								switch(t1)
								{
									case 0:
										u16Temp = c;
										t1 = 1;
									break;
									case 1:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 2;
									break;
									case 2:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 3;
									break;
									case 3:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 0;	
									
										ucs2_to_utf8(u16Temp,tempBuff2);
										strcat(replySmsBuf,tempBuff2);
									break;
									
								}
							}				
						}
						Ampm_RingingReset();
						Ampm_Sms_SendMsg(&replySms,smsSender,(uint8_t *)replySmsBuf,strlen(replySmsBuf),SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
					}
				}
				Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"GSM\"\r");
			}
		}
		//send sms
		pt = strstr(buff,"SMSSEND=");
		if(pt != NULL)
		{
			strcpy((char *)lastRecvSmsPhone,(char *)smsSender);
			smsScanf(pt,"SMSSEND=%[^, :\t\n\r#]",forwardToNumber);

			strcpy((char *)mainBuf,(pt + strlen((char *)forwardToNumber) + 9));
			smsLen = strlen((char *)mainBuf);
			Ampm_Sms_SendMsg(&forwardSms,(uint8_t *)forwardToNumber,(uint8_t *)mainBuf,smsLen,SMS_TEXT_MODE,30000,SMS_MAX_RETRY);
		}
		
		pt = strstr(buff,"SENSOR=NO");
		if(pt)
		{
			INFO("\n\rCONFIG->SENSOR NO\n\r");
			len += sprintf((char *)&smsSendBuff[len],"SENSOR=1,");
			sysCfg.feature &= ~FEATURE_ALARM_IF_INPUT_1;
			flagCfgSave = 1;
		}
		pt = strstr(buff,"SENSOR=NC");
		if(pt)
		{
			INFO("\n\rCONFIG->SENSOR=NC\n\r");
			len += sprintf((char *)&smsSendBuff[len],"SENSOR=1,");
			sysCfg.feature |= FEATURE_ALARM_IF_INPUT_1;
			flagCfgSave = 1;
		}
		
		pt = strstr(buff,"PULLUP=1");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->PULLUP=1\n\r");
			len += sprintf((char *)&smsSendBuff[len],"PULLUP=1,");
			sysCfg.feature |= FEATURE_U6_ENABLE;
			sysCfg.feature |= FEATURE_ALARM_IF_INPUT_1;
			flagCfgSave = 1;
		}
		
		pt = strstr(buff,"PULLUP=0");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->PULLUP=0\n\r");
			len += sprintf((char *)&smsSendBuff[len],"PULLUP=0,");
			sysCfg.feature &= ~FEATURE_U6_ENABLE;
			sysCfg.feature &= ~FEATURE_ALARM_IF_INPUT_1;
			flagCfgSave = 1;
		}
	}
	else
	{
		
		//USER PHONE NUMBER
		pt = strstr(buff,"USER");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->USER=%s\n\r", sysCfg.bossPhoneNum);
			len += sprintf((char *)&smsSendBuff[len], "USER=%s,", sysCfg.bossPhoneNum);
		}
		//IMEI
		pt = strstr(buff,"IMEI");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->IMEI=%s\n\r", sysCfg.imei);
			len += sprintf((char *)&smsSendBuff[len], "IMEI=%s,", sysCfg.imei);
		}
		//NAME
		pt = strstr(buff,"NAME");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->NAME=%s\n\r", sysCfg.id);
			len += sprintf((char *)&smsSendBuff[len], "NAME=%s,", sysCfg.id);
		}
		//change Server Name
		pt = strstr(buff,"DOMAIN");
		if(pt != NULL && pt[strlen("DOMAIN")] != 1)
		{
			INFO("\n\rCONFIG->DOMAIN=%s:%d\n\r",(char *)sysCfg.priDserverName,sysCfg.priDserverPort);
			len += sprintf((char *)&smsSendBuff[len],"DOMAIN=%s:%d,",(char *)sysCfg.priDserverName,sysCfg.priDserverPort);
		}
		
		//change Server Name
		pt = strstr(buff,"DOMAIN2");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->DOMAIN2=%s:%d\n\r",(char *)sysCfg.secDserverName,sysCfg.secDserverPort);
			len += sprintf((char *)&smsSendBuff[len],"DOMAIN2=%s:%d,",(char *)sysCfg.secDserverName,sysCfg.secDserverPort);
		}
		
		//change Server Name
		pt = strstr(buff,"FIRMWARE");
		if(pt != NULL && pt[strlen("FIRMWARE")] != 1)
		{
			INFO("\n\rCONFIG->FIRMWARE=%s:%d\n\r",(char *)sysCfg.priFserverName,sysCfg.priFserverPort);
			len += sprintf((char *)&smsSendBuff[len],"FIRMWARE=%s:%d,",(char *)sysCfg.priFserverName,sysCfg.priFserverPort);
		}
		
		//change Server Name
		pt = strstr(buff,"FIRMWARE2");
		if(pt != NULL)
		{
			INFO("\n\rCONFIG->FIRMWARE2=%s:%d\n\r",(char *)sysCfg.secFserverName,sysCfg.secFserverPort);
			len += sprintf((char *)&smsSendBuff[len],"FIRMWARE2=%s:%d,",(char *)sysCfg.secFserverName,sysCfg.secFserverPort);
		}
		
		//FREQ
		pt = strstr(buff,"FREQ");
		if(pt != NULL && pt[strlen("FREQ")] != 2)
		{
			INFO("\n\rCONFIG->FREQ=%s:%d\n\r",sysCfg.freq);
			len += sprintf((char *)&smsSendBuff[len],"FREQ=%d,",sysCfg.freq);
		}
		
		//FREQ2
		pt = strstr(buff,"FREQ2");
		if(pt)
		{
			INFO("\n\rCONFIG->FREQ2=%s:%d\n\r",sysCfg.freq2);
			len += sprintf((char *)&smsSendBuff[len],"FREQ2=%d,",sysCfg.freq2);
		}
		
		pt = strstr(buff,"TRACE");
		if(pt)
		{
			if(sysCfg.feature & FEATURE_TRACE_ENABLE)
			{
				INFO("\n\rCONFIG->TRACE=1\n\r");
				len += sprintf((char *)&smsSendBuff[len],"TRACE=1,");
			}
			else
			{
				INFO("\n\rCONFIG->TRACE=0\n\r");
				len += sprintf((char *)&smsSendBuff[len],"TRACE=0,");
			}
		}
		
		//VINLBV
		pt = strstr(buff,"VINLBV");
		if(pt)
		{
			INFO("\n\rCONFIG->VINLBV=%f\n\r",sysCfg.vinlbv);
			len += sprintf((char *)&smsSendBuff[len],"VINLBV=%f,",sysCfg.vinlbv);
		}

		//change gprs apn
		pt = strstr(buff,"APN");
		if(pt != NULL)
		{
			INFO("\n\rAPN=%s %s %s\n\r", sysCfg.gprsApn,sysCfg.gprsUsr,sysCfg.gprsPwd);
			len += sprintf((char *)&smsSendBuff[len], "APN=%s %s %s,", sysCfg.gprsApn,sysCfg.gprsUsr,sysCfg.gprsPwd);
		}
		
		//check monney
		pt = strstr(buff,"TKC");
		if(pt != NULL)
		{
			if(Ampm_SendCommand("OK",modemError,1000,3,"AT\r")== AMPM_GSM_RES_OK)
			{
				if(Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"UCS2\"\r")== AMPM_GSM_RES_OK)
				{
					Ampm_SendCommand("OK",modemError,1000,3,"AT+CUSD=1\r");
					if(Ampm_SendCommand("+CUSD:",modemError,10000,3,"ATD*101#;\r") == AMPM_GSM_RES_OK)
					{
						SysTick_DelayMs(1000);
						t1 = 0;
						u16Temp = 0;
						replySmsBuf[0] = 0;
						u8Flag = 0;
						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(c == ',')
							{
								break;
							}
						}

						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(u8Flag == 0)
							{
								u8Flag = 1;
								if(c == '"')
									RINGBUF_Get(commRxRingBuf, &c);
							}
							if(c == ',' || c == '"')
							{
								break;
							}
							else
							{
								if(c >= '0' && c <= '9')
									c = c - '0';
								else if(c >= 'A' && c <= 'F')
									c = 0x0A + (c - 'A');
								else
									break;
								switch(t1)
								{
									case 0:
										u16Temp = c;
										t1 = 1;
									break;
									case 1:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 2;
									break;
									case 2:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 3;
									break;
									case 3:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 0;	
									
										ucs2_to_utf8(u16Temp,tempBuff2);
										strcat(replySmsBuf,tempBuff2);
									break;
									
								}
							}				
						}
						Ampm_RingingReset();
						len += sprintf((char *)&smsSendBuff[len], "TKC=%s,", replySmsBuf);
					}
				}
				Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"GSM\"\r");
			}
		}
		//check monney
		pt = strstr(buff,"TKKM");
		if(pt != NULL)
		{
			if(Ampm_SendCommand("OK",modemError,1000,3,"AT\r")== AMPM_GSM_RES_OK)
			{
				if(Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"UCS2\"\r")== AMPM_GSM_RES_OK)
				{
					Ampm_SendCommand("OK",modemError,1000,3,"AT+CUSD=1\r");
					if(Ampm_SendCommand("+CUSD:",modemError,10000,3,"ATD*102#;\r") == AMPM_GSM_RES_OK)
					{
						SysTick_DelayMs(1000);
						t1 = 0;
						u16Temp = 0;
						replySmsBuf[0] = 0;
						u8Flag = 0;
						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(c == ',')
							{
								break;
							}
						}

						while(RINGBUF_Get(commRxRingBuf, &c)==0)
						{
							if(u8Flag == 0)
							{
								u8Flag = 1;
								if(c == '"')
									RINGBUF_Get(commRxRingBuf, &c);
							}
							if(c == ',' || c == '"')
							{
								break;
							}
							else
							{
								if(c >= '0' && c <= '9')
									c = c - '0';
								else if(c >= 'A' && c <= 'F')
									c = 0x0A + (c - 'A');
								else
									break;
								switch(t1)
								{
									case 0:
										u16Temp = c;
										t1 = 1;
									break;
									case 1:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 2;
									break;
									case 2:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 3;
									break;
									case 3:
										u16Temp <<= 4;
										u16Temp += c;
										t1 = 0;	
									
										ucs2_to_utf8(u16Temp,tempBuff2);
										strcat(replySmsBuf,tempBuff2);
									break;
									
								}
							}				
						}
						Ampm_RingingReset();
						len += sprintf((char *)&smsSendBuff[len], "TKKM=%s,", replySmsBuf);
					}
				}
				Ampm_SendCommand("OK",modemError,1000,3,"AT+CSCS=\"GSM\"\r");
			}
		}
		//SENSOR
		pt = strstr(buff,"SENSOR");
		if(pt)
		{
			if(sysCfg.feature & FEATURE_ALARM_IF_INPUT_1)
			{
				INFO("\n\rCONFIG->SENSOR=NC\n\r");
				len += sprintf((char *)&smsSendBuff[len],"SENSOR=NC,");
			}
			else
			{
				INFO("\n\rCONFIG->SENSOR=NO\n\r");
				len += sprintf((char *)&smsSendBuff[len],"SENSOR=NO,");
			}
		}
		
		//PULLUP
		pt = strstr(buff,"PULLUP");
		if(pt)
		{
			if(sysCfg.feature & FEATURE_U6_ENABLE)
			{
				INFO("\n\rCONFIG->PULLUP=1\n\r");
				len += sprintf((char *)&smsSendBuff[len],"PULLUP=1,");
			}
			else
			{
				INFO("\n\rCONFIG->PULLUP=0\n\r");
				len += sprintf((char *)&smsSendBuff[len],"PULLUP=0,");
			}
		}
		//SOFTVERSION
		pt = strstr(buff,"SOFTVERSION");
		if(pt)
		{
			INFO("\n\rCONFIG->SOFTVERSION=%s\n\r",sysCfg.fwVersion);
			len += sprintf((char *)&smsSendBuff[len],"SOFTVERSION=%s,",sysCfg.fwVersion);
		}
		//STA
		pt = strstr(buff,"STA");
		if(pt)
		{
			INFO("\n\rCONFIG->STA=%s\n\r",sysCfg.fwVersion);
			len += sprintf((char *)&smsSendBuff[len],"STA=%s,",sysCfg.fwVersion);
		}
	}
	
	
	if(len >= smsLenBuf)	len = smsLenBuf;
	
	smsSendBuff[len] = 0;
	*dataOutLen = len;
	
	if(flagCfgSave)
	{
		CFG_Save();
	}

	return 0;
}

