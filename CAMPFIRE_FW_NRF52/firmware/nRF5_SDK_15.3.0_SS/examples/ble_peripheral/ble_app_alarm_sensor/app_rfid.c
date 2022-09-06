
#include "app_rfid.h"
#include "app_tracking.h"

Timeout_Type tRfidCmdTimeout;
Timeout_Type tRfidTagTimeout;

uint8_t rfidCmdLen = 0;
uint8_t rfidBuf[64];
uint8_t rfidCardInfo[71];
uint8_t rfidGotCardInfoFlag = 0;
uint8_t rfidDataCnt = 0;
uint8_t cardStayOnDock = 0;
uint8_t flagLongTimeCardInsert = 0;
uint32_t rfidTimeSec = 0;
//RFID
ReaderConfigStruct			ReaderConfig;
uint8_t			InventoryFlagByte,CR95HF_Handle;
extern CR95HF_INTERFACE		CR95HF_Interface;
uint8_t 	ReaderRecBuf[MAX_BUFFER_SIZE+3]; 	// buffer for SPI ou UART reception
uint8_t				ResponseFlags;
char 			ASCIIString [MAX_BUFFER_SIZE];
uint8_t cardData[128],len;


enum{
	RFID_CARD_IN,
	RFID_CARD_OUT
}rfidPhase = RFID_CARD_OUT;

typedef struct{
	uint8_t ledCode;
	uint8_t flashCycle;
	uint8_t flashCount;
	uint8_t cycle;
	uint8_t checksum;
}RFID_LED_CONTROL;

typedef struct{
	uint8_t buzzCode;
	uint8_t buzzCycle;
	uint8_t buzzCount;
	uint8_t cycle;
	uint8_t checksum;
}RFID_BUZZER_CONTROL;

RFID_BUZZER_CONTROL rfid_buzz;
RFID_LED_CONTROL rfid_led;
#define RFID_GREEN_LED	2
#define RFID_RED_LED		1

void RFID_SetBuzz(uint8_t buzzCode,
	uint8_t buzzCycle,
	uint8_t buzzCount,
	uint8_t cycle)
{
	
}


void RFID_Login(uint8_t login)
{
	uint16_t i,len;
	uint8_t c;
	char *pt;
	flagChangeDriver = 1;
	//TrackerGetNewData();
	pt = (char *)rfidCardInfo;
	for(i = 0; i < CONFIG_MAX_DRIVERS;i++)
	{
		len = i;
		if(memcmp((char *)sysCfg.driverList[i].licenseNo,&pt[8],16) == NULL)
		{
			break;
		}
	}
	
	if(i >= CONFIG_MAX_DRIVERS)
	{
		sysCfg.driverListCnt++;
		if(sysCfg.driverListCnt >= CONFIG_MAX_DRIVERS)
			sysCfg.driverListCnt = 0;
		memcpy((char *)sysCfg.driverList[sysCfg.driverListCnt].licenseNo,&pt[8],16);
		memcpy((char *)sysCfg.driverList[sysCfg.driverListCnt].driverName,&pt[24],44);
		len = sysCfg.driverListCnt;
		CFG_Save();
	}
	
	if(sysCfg.driverIndex != len)
	{
		sysCfg.driverIndex = len;
	}
	
	if(login)
	{
		rfidLoginFlag = 1;
		
		systemRecord.driverIndex = sysCfg.driverIndex;
		systemRecord.beginLat = lastNmeaInfo.lat;
		systemRecord.beginLon = lastNmeaInfo.lon;
		//systemRecord.drivingTime = 0;

		rfid_led.ledCode = RFID_GREEN_LED;
		rfid_led.flashCycle = 0xff; //led alway on
		rfid_led.flashCount = 1;
		rfid_led.cycle = 10;
		rfid_led.checksum = DbCalcCheckSum((uint8_t *)&rfid_led,sizeof(RFID_LED_CONTROL) - 1);
		mUSART3_PutString(">JAS511:C1");
		mUSART3_PutChar(0x04);
		mUSART3_PutChar(rfid_led.ledCode);
		mUSART3_PutChar(rfid_led.flashCycle);
		mUSART3_PutChar(rfid_led.flashCount);
		mUSART3_PutChar(rfid_led.cycle);
		mUSART3_PutChar(rfid_led.checksum);
		
		rfid_buzz.buzzCode = 0x01;
		rfid_buzz.buzzCycle = 0x01;
		rfid_buzz.buzzCount = 0x01;
		rfid_buzz.cycle = 0;
		rfid_buzz.checksum = DbCalcCheckSum((uint8_t *)&rfid_buzz,sizeof(RFID_BUZZER_CONTROL) - 1);
		rfidCmdLen = sprintf((char *)rfidBuf,">JAS511:C2%c%c%c%c%c%c",0x04,
			rfid_buzz.buzzCode,
			rfid_buzz.buzzCycle,
			rfid_buzz.buzzCount,
			rfid_buzz.cycle,
			rfid_buzz.checksum
		);
		InitTimeout(&tRfidCmdTimeout,SYSTICK_TIME_MS(500));
	}
	else
	{
		rfidLoginFlag = 0;
		//n?u dang xu?t thì luu l?i log v? th?i gian b?t d?u và k?t thúc c?a tài x? hi?n t?i
		if(systemRecord.drivingTime)
		{
			
			uint32_t u32Temp = (uint32_t)systemRecord.currentTime.hour*3600 + 
							(uint32_t)systemRecord.currentTime.min*60 + 
							systemRecord.currentTime.sec;
			if(u32Temp > systemRecord.drivingTime)
			{
				u32Temp -= systemRecord.drivingTime;
				driverRecord.beginTime.hour = u32Temp/3600;
				driverRecord.beginTime.min = (u32Temp - (uint32_t)driverRecord.beginTime.hour*3600)/60;
				driverRecord.beginTime.sec = ((u32Temp - (uint32_t)driverRecord.beginTime.hour*3600) - (uint32_t)driverRecord.beginTime.min*60) % 60;
			}
			else
			{
				driverRecord.beginTime.hour = 0;
				driverRecord.beginTime.min = 0;
				driverRecord.beginTime.sec = 0;
			}
			driverRecord.endTime = systemRecord.currentTime;
			driverRecord.beginLat = systemRecord.beginLat;
			driverRecord.beginLon = systemRecord.beginLon;
			driverRecord.endLat = systemRecord.lat;
			driverRecord.endLon = systemRecord.lon; 
			driverRecord.totalDrivingTime = systemRecord.totalDrivingTime;
			strcpy((char *)driverRecord.driverName,(char *)sysCfg.driverList[systemRecord.driverIndex].driverName);
			strcpy((char *)driverRecord.licenseNo,(char *)sysCfg.driverList[systemRecord.driverIndex].licenseNo);
			DB_SaveWorkingTime(localTime,&driverRecord);
			DriverWorkingSendToServer();
			systemRecord.drivingTime = 0;
		}
		//N?u th? qu?t vào là tài x? khác thì dang nh?p cho tài x? m?i luôn
		if(systemRecord.driverIndex != sysCfg.driverIndex)
		{
			rfidLoginFlag = 1;
			systemRecord.driverIndex = sysCfg.driverIndex;
			systemRecord.beginLat = lastNmeaInfo.lat;
			systemRecord.beginLon = lastNmeaInfo.lon;
		}
		systemRecord.timeSave = TIME_GetSec(&localTime);
		DB_SaveAll();
	}
	
	if(rfidLoginFlag)
	{
		rfid_led.ledCode = RFID_GREEN_LED;
		rfid_led.flashCycle = 0xff; //led alway on
		rfid_led.flashCount = 1;
		rfid_led.cycle = 10;
		rfid_led.checksum = DbCalcCheckSum((uint8_t *)&rfid_led,sizeof(RFID_LED_CONTROL) - 1);
		
		rfid_buzz.buzzCode = 0x01;
		rfid_buzz.buzzCycle = 0x01;
		rfid_buzz.buzzCount = 0x01;
		rfid_buzz.cycle = 0;
		rfid_buzz.checksum = DbCalcCheckSum((uint8_t *)&rfid_buzz,sizeof(RFID_BUZZER_CONTROL) - 1);
	}
	else
	{
		rfid_led.ledCode = RFID_GREEN_LED;
		rfid_led.flashCycle = 0x01; //led off
		rfid_led.flashCount = 1;
		rfid_led.cycle = 50;
		rfid_led.checksum = DbCalcCheckSum((uint8_t *)&rfid_led,sizeof(RFID_LED_CONTROL) - 1);
		rfid_buzz.buzzCode = 0x01;
		rfid_buzz.buzzCycle = 0x01;
		rfid_buzz.buzzCount = 0x03;
		rfid_buzz.cycle = 0;
		rfid_buzz.checksum = DbCalcCheckSum((uint8_t *)&rfid_buzz,sizeof(RFID_BUZZER_CONTROL) - 1);
		
	}
	
	mUSART3_PutString(">JAS511:C1");
		mUSART3_PutChar(0x04);
		mUSART3_PutChar(rfid_led.ledCode);
		mUSART3_PutChar(rfid_led.flashCycle);
		mUSART3_PutChar(rfid_led.flashCount);
		mUSART3_PutChar(rfid_led.cycle);
		mUSART3_PutChar(rfid_led.checksum);
	rfidCmdLen = sprintf((char *)rfidBuf,">JAS511:C2%c%c%c%c%c%c",0x04,
			rfid_buzz.buzzCode,
			rfid_buzz.buzzCycle,
			rfid_buzz.buzzCount,
			rfid_buzz.cycle,
			rfid_buzz.checksum
		);
	InitTimeout(&tRfidCmdTimeout,SYSTICK_TIME_MS(500));
}

void rfid_1sec_process(void)
{
	rfidTimeSec++;
	if(rfidTimeSec % 60 == 0)
	{
		if(systemRecord.drivingTime && rfidLoginFlag == 0)
		{
			if(sysCfg.featureSet & FEATURE_RFID)
			{
			rfid_buzz.buzzCode = 0x01;
			rfid_buzz.buzzCycle = 0x01;
			rfid_buzz.buzzCount = 0x05;
			rfid_buzz.cycle = 0;
			rfid_buzz.checksum = DbCalcCheckSum((uint8_t *)&rfid_buzz,sizeof(RFID_BUZZER_CONTROL) - 1);
			mUSART3_PutString(">JAS511:C2");
			mUSART3_PutChar(0x04);
			mUSART3_PutChar(rfid_buzz.buzzCode);
			mUSART3_PutChar(rfid_buzz.buzzCycle);
			mUSART3_PutChar(rfid_buzz.buzzCount);
			mUSART3_PutChar(rfid_buzz.cycle);
			mUSART3_PutChar(rfid_buzz.checksum);
			}
		}	
	}
	if(((rfidTimeSec % 35 == 0) || (flagSystemStatus != flagSystemStatusUpdate))
		&& (rfidLoginFlag == 0)
	)
	{
		flagSystemStatusUpdate = flagSystemStatus;
		rfid_led.ledCode = RFID_GREEN_LED;
		rfid_led.flashCycle = 1; //0.2ms
		rfid_led.flashCount = 1;
		rfid_led.cycle = 50;
		if(flagSystemStatus)
		{
			rfid_led.flashCount += flagSystemStatus; // 1 time
		}

		rfid_led.checksum = DbCalcCheckSum((uint8_t *)&rfid_led,sizeof(RFID_LED_CONTROL) - 1);
		mUSART3_PutString(">JAS511:C1");
		mUSART3_PutChar(0x04);
		mUSART3_PutChar(rfid_led.ledCode);
		mUSART3_PutChar(rfid_led.flashCycle);
		mUSART3_PutChar(rfid_led.flashCount);
		mUSART3_PutChar(rfid_led.cycle);
		mUSART3_PutChar(rfid_led.checksum);
	}
}

void rfid_process(void)
{
	char *pt;
	if(CheckTimeout(&tRfidCmdTimeout) == SYSTICK_TIMEOUT)
	{
		if(rfidCmdLen)
			for(uint8_t i = 0;i < rfidCmdLen;i++)
			{
				mUSART3_PutChar(rfidBuf[i]);
			}
		rfidCmdLen = 0;
		InitTimeout(&tRfidCmdTimeout,SYSTICK_TIME_MS(500));
	}
	
	switch(rfidPhase)
	{
		case RFID_CARD_OUT:
			if(rfidGotCardInfoFlag)
			{
				rfidGotCardInfoFlag = 0;
				flagLongTimeCardInsert = 0;
				rfidDataCnt = 0;
				rfidPhase = RFID_CARD_IN;
				InitTimeout(&tRfidTagTimeout,SYSTICK_TIME_SEC(2));
				RFID_Login(!rfidLoginFlag);
			}
		break;
		case RFID_CARD_IN:
			if(CheckTimeout(&tRfidTagTimeout) == SYSTICK_TIMEOUT)
			{
				InitTimeout(&tRfidTagTimeout,SYSTICK_TIME_SEC(3));
				if(rfidDataCnt == 0)
				{
					rfidGotCardInfoFlag = 0;
					rfidPhase = RFID_CARD_OUT;
					if(rfidLoginFlag && flagLongTimeCardInsert)
					{
						RFID_Login(!rfidLoginFlag);
					}
				}
				else
				{
					flagLongTimeCardInsert = 1;
					if(rfidLoginFlag == 0)
					{
						RFID_Login(!rfidLoginFlag);
					}
					pt = rfidCardInfo;
					if(memcmp((char *)sysCfg.driverList[systemRecord.driverIndex].licenseNo,&pt[8],16))
					{
						RFID_Login(!rfidLoginFlag);
					}
				}
				rfidDataCnt = 0;
			}
		break;
	}
	
	
	
	
}

void rfid_app_init(void)
{
	
//		ReaderConfig.Interface = CR95HF_INTERFACE_SPI;
//		CR95HF_Interface = CR95HF_INTERFACE_SPI;
//		CR95HF_Handle = CR95HF_PORsequence( );
	
		rfid_led.ledCode = RFID_GREEN_LED;
		if(rfidLoginFlag)
		{
			rfidPhase = RFID_CARD_IN;
			//flagLongTimeCardInsert = 1;
			InitTimeout(&tRfidTagTimeout,SYSTICK_TIME_SEC(3));
			rfid_led.flashCycle = 0xff; //led alway on
		}
		else
		{
			rfid_led.flashCycle = 0x00; //led alway off
		}
		rfid_led.flashCount = 0;
		rfid_led.cycle = 0;
		rfid_led.checksum = DbCalcCheckSum((uint8_t *)&rfid_led,sizeof(RFID_LED_CONTROL) - 1);
		mUSART3_PutString(">JAS511:C1");
		mUSART3_PutChar(0x04);
		mUSART3_PutChar(rfid_led.ledCode);
		mUSART3_PutChar(rfid_led.flashCycle);
		mUSART3_PutChar(rfid_led.flashCount);
		mUSART3_PutChar(rfid_led.cycle);
		mUSART3_PutChar(rfid_led.checksum);
	
}



