#include <stdint.h>
#include <string.h>
#include "ampm_gsm_main_task.h"
#include "app_config_task.h"
#include "tcp_ip_task.h"
#include "app_ble.h"
#include "lib/sys_time.h"
#include "app_tracking.h"
#include "led.h"
#include "rs232_server.h"
#include "fire_alarm_message_create.h"
#include "app_mesh.h"

#define Ampm_PrintLog  DbgCfgPrintf

void AppRun(void);
uint8_t GSM_GPRS_Task(uint8_t *arg);
void SysSleep(void);
void SysInit(void);
void PowerWarningTask(void);

extern uint32_t sequence;
uint8_t smsTriger = 0;

uint32_t mainCnt = 0;
uint32_t timer2Cnt;
uint16_t sleepCnt = 0;
uint32_t reportInterVal = 0;
uint32_t smsTimeCheck = 0;
uint8_t sendStatus = 0;
uint8_t statusSentFlag = 0;
uint64_t imeiValue = 0;

Timeout_Type tFwDownload;
Timeout_Type tResetApp;
Timeout_Type tAppTimeout;
Timeout_Type tGPRS_EnTimeout;

Timeout_Type smsTimeoutToSend;
const uint8_t CoChay[] = "Có cháy!";
uint8_t alarmMsg[256];
uint8_t alarmMsg1[256];
uint8_t alarmMsgLen;
struct SMS_LIST_TYPE sms0,sms1;

uint32_t getPhaseFlag = 0;



uint32_t startup_delay = 3000;
uint32_t alarm_timeout = 3600000;
extern Timeout_Type timeTransferData;
uint8_t intervalReportWakeFlag = 0;

float ADC1_8_Value = 12;

uint8_t mainBuf[340];


void SysDeInit(void)
{
	nrf_delay_ms(100);
	app_preodic_timer_stop();
	UART1_DeInit();
}


void SysWakeUp(void)
{
	spi1_Init();
	nrf_delay_ms(100);
	UART1_Init();
	app_preodic_timer_start();
	
}

uint8_t GSM_GPRS_Task(uint8_t *arg)
{

		Timeout_Type t1SecTask;
		
		AMPM_GSM_Init("internet","mms","mms",vTcpIpTask,vTcpIpTaskInit);
		//Ampm_GsmSetMode(AMPM_GSM_NONE);
		Ampm_GsmSetMode(AMPM_GSM_GPRS_ENABLE);
		InitTimeout(&tGPRS_EnTimeout,SYSTICK_TIME_SEC(60));
		InitTimeout(&tAppTimeout,SYSTICK_TIME_SEC(120));
		InitTimeout(&t1SecTask,SYSTICK_TIME_SEC(1));
		startup_delay = 3;
		while(1)
		{
				power_manage();
				app_mesh_task();
				ble_task();
				boot_mode_task();
				mainCnt++;
				sysCfg.deviceKey[16] = 0;
				sysCfg.mainAesKey[16] = 0;
				watchdogFeed[WTD_MAIN_LOOP] = 0; 
				if(CheckTimeout(&tGPRS_EnTimeout) == SYSTICK_TIMEOUT)
				{
					InitTimeout(&tGPRS_EnTimeout,SYSTICK_TIME_SEC(3600));
					Ampm_GsmSetMode(AMPM_GSM_GPRS_ENABLE);
				}
				//
				if(flagSystemStatus >= SYS_SERVER_OK)
				{
					IO_ToggleSetStatus(&io_led1,100,3000,IO_TOGGLE_ENABLE,IO_MAX_VALUE);
				}
				else if(flagSystemStatus >= SYS_GPRS_OK)
				{
					IO_ToggleSetStatus(&io_led1,IO_STATUS_ON_TIME_DFG,IO_STATUS_OFF_TIME_DFG,IO_TOGGLE_ENABLE,IO_MAX_VALUE);
				}
				else if(flagSystemStatus >= SYS_GSM_OK)
				{
					IO_ToggleSetStatus(&io_led1,IO_STATUS_ON_TIME_DFG,0,IO_TOGGLE_ENABLE,IO_MAX_VALUE);
				}
				else
				{
					//InitTimeout(&tGPRS_EnTimeout,SYSTICK_TIME_SEC(90));
					IO_ToggleSetStatus(&io_led1,0,0,IO_TOGGLE_DISABLE,IO_MAX_VALUE);
				}
			  io_process_action_task(rtcTimeSec);
				AMPM_GSM_MainTask(NULL);
				//PowerWarningTask();
				//Config and debug task
			//Read ADC Value
				ADC_Task(rtcTimeSec);
				FIRMWARE_Task();
				// reset device task
				ResetMcuTask();
				//get IMEI
				if(flagGotIMEI 
					&& (strlen((char *)gsmIMEIBuf) == 15) 
					&& (strstr((char *)sysCfg.imei,(char *)gsmIMEIBuf) == NULL
						|| memcmp((char *)&gsmIMEIBuf[7],(char *)&sysCfg.id[4],8)
					)
				)
				{
					flagGotIMEI = 0;
					strcpy((char *)sysCfg.imei,(char *)gsmIMEIBuf);
					imeiValue = atoll((char *)sysCfg.imei);
					sysCfg.id[0] = sysCfg.imei[0];
					sysCfg.id[1] = sysCfg.imei[1];
					sysCfg.id[2] = '1';
					sysCfg.id[3] = '7';
					Ampm_PrintLog(ALL_LOG,"**********NEW IMEI:%s***********",(char *)sysCfg.imei);
					strcpy((char *)&sysCfg.id[4],(char *)&sysCfg.imei[strlen((char *)sysCfg.imei) - 8]);
					Ampm_PrintLog(ALL_LOG,"**********NEW DEVICE ID:%s******",(char *)sysCfg.id);
					CFG_Save();
				}
				//Get CCID
				if(flagGotSimCID)
				{
					if(strcmp((char *)sysCfg.ccid,(char *)gsmSimCIDBuf))
					{
						strcpy((char *)sysCfg.ccid, (char *)gsmSimCIDBuf);
						CFG_Save();
					}
				}
				//Get IMSI
				if(flagGotSimCIMI)
				{
					if(strcmp((char *)sysCfg.cimi,(char *)gsmSimCIMIBuf))
					{
						strcpy((char *)sysCfg.cimi, (char *)gsmSimCIMIBuf);
						CFG_Save();
					}
				}
				if(CheckTimeout(&t1SecTask) == SYSTICK_TIMEOUT)
				{
					InitTimeout(&t1SecTask,SYSTICK_TIME_SEC(1));
					if(startup_delay) startup_delay--;
				}
				
				if(smsTriger && CheckTimeout(&smsTimeoutToSend) == SYSTICK_TIMEOUT
				&& sms0.flag != SMS_NEW_MSG && sms1.flag != SMS_NEW_MSG
				)
				{
					smsTriger = 0;
					fireAlarmRecord.time = sysTime;
					fireAlarmRecord.csqValue = csqValue;
					fireAlarmRecord.vin = ADC1_8_Value;
					if(fired_flag)
					{
						alarmMsgLen = sprintf((char *)alarmMsg,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T25,0,%s,%s,0,0,14,0.0.0.0,00%02d%02d,0,%03d",
															fireAlarmRecord.time.year,
															fireAlarmRecord.time.month,
															fireAlarmRecord.time.mday,
															fireAlarmRecord.time.hour,
															fireAlarmRecord.time.min,
															fireAlarmRecord.time.sec,
															sysCfg.id,
															"E,0",
															"N,0",
															(uint32_t)fireAlarmRecord.csqValue,
															(uint32_t)fireAlarmRecord.vin,
															sequence
										);
					}
					else
					{
						alarmMsgLen = sprintf((char *)alarmMsg,"%04d-%02d-%02d %02d:%02d:%02d,2,W4,%s,T25,0,%s,%s,0,0,2,0.0.0.0,00%02d%02d,0,%03d",
															fireAlarmRecord.time.year,
															fireAlarmRecord.time.month,
															fireAlarmRecord.time.mday,
															fireAlarmRecord.time.hour,
															fireAlarmRecord.time.min,
															fireAlarmRecord.time.sec,
															sysCfg.id,
															"E,0",
															"N,0",
															(uint32_t)fireAlarmRecord.csqValue,
															(uint32_t)fireAlarmRecord.vin,
															sequence
										);
					}
					alarmMsgLen = FireAlarmCreateMsg(alarmMsg,alarmMsgLen,mainBuf,sizeof(mainBuf),sysCfg.deviceKey);
					alarmMsg[0] = '[';
					alarmMsg[1] = 0;
					strcat((char *)alarmMsg,(char *)mainBuf);
					strcat((char *)alarmMsg,"]");
					alarmMsgLen = strlen((char *)alarmMsg);
					if(strlen((char *)sysCfg.bossPhoneNum) >= 10)
					{	
						if(alarmMsgLen > SMS_TEXT_LENGTH_MAX)
						{
							strcpy((char *)alarmMsg1,(char *)&alarmMsg[SMS_TEXT_LENGTH_MAX]);
							alarmMsg[SMS_TEXT_LENGTH_MAX] = 0;
							Ampm_Sms_SendMsg(&sms0,sysCfg.bossPhoneNum,(uint8_t *)alarmMsg,SMS_TEXT_LENGTH_MAX,SMS_TEXT_MODE,30000,10);
							Ampm_Sms_SendMsg(&sms1,sysCfg.bossPhoneNum,(uint8_t *)alarmMsg1,alarmMsgLen - SMS_TEXT_LENGTH_MAX,SMS_TEXT_MODE,30000,10);
						}
						else
						{
							Ampm_Sms_SendMsg(&sms0,sysCfg.bossPhoneNum,(uint8_t *)alarmMsg,alarmMsgLen,SMS_TEXT_MODE,30000,10);
							sms1.flag = SMS_MSG_SENT;
						}
					}
				}

				if(ADC1_8_Value < sysCfg.vinlbv)
				{
					if(lowpower_flag == 0)
					{
						lowpower_trigger = 1;
						smsTriger = 1;
						InitTimeout(&smsTimeoutToSend,SYSTICK_TIME_SEC(5));
					}
					lowpower_flag = 1;
				}
				else if(ADC1_8_Value > (sysCfg.vinlbv + 1.0))
				{
					if(lowpower_flag == 1)
					{
						lowpower_trigger = 1;
					}
					lowpower_flag = 0;
				}
				//alarm process
				if(startup_delay == 0)
				{
					//if(sysCfg.feature & FEATURE_ALARM_IF_INPUT_1)
					{
						if(alarm_in1_is_high)
						{
							if(fired_flag == 0)
							{
								fire_trigger = 1;
								smsTriger = 1;
								InitTimeout(&smsTimeoutToSend,SYSTICK_TIME_SEC(5));
							}
							fired_flag = 1;
						}
						else
						{
							if(fired_flag == 1)
							{
								fire_trigger = 1;
								smsTriger = 1;
								InitTimeout(&smsTimeoutToSend,SYSTICK_TIME_SEC(5));
							}
							alarm_timeout = 0;
							fired_flag = 0;
						}
					}
				}
			
				if(fired_flag == 1)
				{
					nrf_gpio_pin_set(BUZZER_PIN);
					LED_SYS_PIN_SET;
				}
				else
				{
					LED_SYS_PIN_CLR;
					nrf_gpio_pin_clear(BUZZER_PIN);
				}
		}
		return 0;
}


void SendSmsToServer(void)
{
	uint8_t smsLen,i;
	uint8_t pdu2uniBuf[256]; 
	char latBuf[16];
	char lonBuf[16];
	if(sms0.flag != SMS_NEW_MSG && sms1.flag != SMS_NEW_MSG && (alarm_timeout == 0))
	{
		
		InitTimeout(&smsTimeoutToSend,SYSTICK_TIME_SEC(5));
		alarm_timeout = 30000;
	}
	if(alarm_timeout && sms0.flag == SMS_MSG_SENT && sms1.flag == SMS_MSG_SENT)
	{
		alarm_timeout = 3600000; //resend each 1 hour
	}
}



void AppRun(void)
{
	uint8_t arg = 0,cnt = 1;
	uint32_t rtcTimeSecOld;
	//LoadFirmwareFile();
	watchdogEnable[WTD_MAIN_LOOP] = 1;
	adc_configure();
	nrf_drv_saadc_sample();
	Board_IO_Init();
	while(1)
	{
		GSM_GPRS_Task(&arg);
	}
}




uint32_t cnt_temp = 0;
void tracking_app_init(void)
{	
	//advertising_start(false);
	//while(1) power_manage();
	
	if((sysState != SYS_RUNNING_APP) && (sysState != SYS_RUNNING_BOOT))
	{
		modemJustPowerOff = 1;
		sysState = SYS_RUNNING_APP;
		rtcTimeSec = 0;
	}
	SysTimeSetup();
	NRF_LOG_INFO("System Start!\r\n");
	application_timers_start();
	advertising_start(false);
}
