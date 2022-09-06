
#include <stdio.h>
#include <string.h>
#include "tracker.h"
#include <math.h>
#include "system_config.h"
#include "gps/ampm_gps.h"
#include "db.h"
#include "lib/sys_time.h"
#include "adc_task.h"
#include "database_app.h"
#include "lib/sys_tick.h"
#include "diskio.h"
#include "ff.h"
#include "io_control.h"
#include "ampm_gsm_main_task.h"
#include "app_tracking.h"


#define TRACKER(...)	//DbgCfgPrintf(__VA_ARGS__)
#define SPEED_UP_LIMIT 5 //km/h
#define TICK_RATE			CONFIG_SYS_CLOCK
#define REMIND_INTERVAL		60*60				// VEHICLE_INFO report interval (sec)
#define OVERSPEED_INTERVAL	30					// sec
#define PARKING_INTERVAL	15

//#define USER_TEST_CODE	1			  /*user for test speed, GT402 will genarate internal speed to use*/
#define USER_SPEED_FOR_CAR_STATUS	 /*if not zezo then will use speed for detect car status is runing or stoping*/
																			/*else will use ACC for detect car status*/
#define USER_STOP_TIME_CONDITION	(15*60) /*sec*/
#define USER_START_TIME_CONDITION	(15) /*sec*/
//#define ENABLE_OVER_SPEED_WARNING

uint8_t flagNewMinTracker = 0;


uint8_t trackerTaskRun = 0;

uint8_t trackerSaveLog;
uint32_t trackerTaskCnt = 0;
extern uint8_t flagSDcardErr;
//GPS
uint32_t gpsGetCnt = 0;
uint8_t gpsFixedFlag = 0;
uint8_t  gpsGotWhenACC_OFF = 0;
uint8_t gpsTryCntWhenACC_OFF = 10;
uint16_t getGpsTimeout = GPS_TRY_TIMEOUT;
uint8_t timeFixed = 0;
//
uint32_t buzzStatus = 0;
uint32_t timeOverSpeedCnt = 0;
uint32_t startingTime = 0;
uint32_t speedRecordSaveTime = 0;
//Server message
uint8_t sendStatus = 0;
uint8_t statusSentFlag = 0;
MSG_STATUS_RECORD	logSend;
MSG_STATUS_RECORD	*logSendPt = NULL;
MSG_SPEED_RECORD logSpeed;
MSG_SPEED_RECORD	*logSpeedSendPt = NULL;
//Record
uint8_t newDayFlag = 0;

extern SYSTEM_RECORD systemRecord ;
PARKING_RECORD parkingRecord;
DOOR_RECORD doorRecord;
TRACKING_RECORD trackingRecord;
WORKING_TIME_RECORD driverRecord;
SPEED_RECORD speedRecord;

uint8_t flagStopWarning = 1;
uint8_t flagSaveDrivingTime = 0;
uint8_t rfidLoginFlag = 0;
uint8_t speedBufLen = 0;
uint8_t speedBuf[60];
uint8_t firstSampleFlag = 1;
uint8_t flagChangeDriver = 0;
uint16_t gpsLostCnt = 0;
uint8_t lowPowerCnt = 0;
uint8_t powerGoodCnt = 0;
extern float pulseSpeed_km_h;
float currentSpeed = 0,gpsCurrentSpeed;
float speedSamples[5];
float lastSpeed = 0;
float dv_div_dt = 0;
//System warning

typedef enum _DriverWarningType{
	RESET_WARNING,
	ON_WARNING,
	OFF_WARNING,
}DriverWarningType;

DriverWarningType flagDrivingTimeWarning = RESET_WARNING;
DriverWarningType flagTotalDrivingTimeWarning = RESET_WARNING;
Timeout_Type tSpeedAlarmTimeout;
Timeout_Type tGPS_PowerOffTimeout;
DATE_TIME printfReportTime;
extern uint8_t FirmwareTask_IsBusy(void);
extern FATFS sdfs;

uint8_t gotLastRecord = 0;

typedef enum{
	REGULARLY_REPORT = 0xA0,
	POWER_ON_REPORT,
	POWER_OFF_REPORT,
	RFID_REPORT,
	SD_REPORT,
	OVER_4HOUR_REPORT,
	OVER_SPEED_REPORT,
	ACC_REPORT,
	WORKING_TIME_REPORT,
}MSG_TYPE;

void TrackerInit(void)
{
	
}

void TRACKER_Pause(void)
{
	trackerTaskRun = 0;
}
void TRACKER_Resume(void)
{
	trackerTaskRun = 1;
}

uint8_t TRACKER_Task_IsIdle(void)
{
	if((!trackerSaveLog) 
	&& (logSendPt == NULL)
	&& !accChagneReportFlag
	){
		return 1;
	}
	return 0;
}

uint8_t GetDataFromLog(void)
{
	if((ringLog.head != ringLog.tail))
	{
		if(logSendPt == NULL)
		{
			if(logSend.serverSent)
			{
				logSend.serverSent = 0;
				DB_RingLogNext();
			}
			if(DB_LoadNextLog(&logSend) == 0)
			{
				logSendPt = &logSend; 
				return 0;
			}
		}
	}
	return 1;
}

uint8_t GetDataFromLogSpeed(void)
{
	if((speedRingLog.head != speedRingLog.tail))
	{
		if(logSpeedSendPt == NULL)
		{
			if(logSpeed.sent)
			{
				logSpeed.sent = 0;
				DB_RingLogNextSpeed();
			}
			if(DB_LoadNextLogSpeed(&logSpeed) == 0)
			{
				logSpeedSendPt = &logSpeed; 
				return 0;
			}
		}
	}
	return 1;
}

void EventReport(MSG_TYPE msg_type)
{
	logRecord.msgType = msg_type;
	logRecord.time = TIME_GetSec(&sysTime);
	logRecord.fuel1 = systemRecord.fuel1;
	logRecord.fuel2 = systemRecord.fuel2;
	logRecord.fuelTemper = systemRecord.fuelTemper;
	
	logRecord.ns = lastNmeaInfo.ns;
	logRecord.ew = lastNmeaInfo.ew;
	
	logRecord.gpsLat = (uint32_t)(lastNmeaInfo.mlat*100000);
	logRecord.gpsLon = (uint32_t)(lastNmeaInfo.mlon*100000);
	logRecord.gpsHdop = lastNmeaInfo.HDOP;
	logRecord.gpsSpeed = currentSpeed*10;
	logRecord.gpsDir = lastNmeaInfo.direction;
	logRecord.serverSent = 0;
	logRecord.IOStatus = 0;
	logRecord.mileage = systemRecord.mileage;
	logRecord.driverIndex = systemRecord.driverIndex;
	if(GET_AVLIO1_PIN)
		logRecord.IOStatus |= 1<<0; //ACC
	if(GET_AVLIO2_PIN)
		logRecord.IOStatus |= 1<<1;
	if(GET_AVLIO3_PIN)
		logRecord.IOStatus |= 1<<2;
	if(GET_AVLIO4_PIN)
		logRecord.IOStatus |= 1<<3;
	if(nmeaInfo.fix >= 3)
		logRecord.IOStatus |= 1<<4;
	
	if(rfidLoginFlag)
		logRecord.IOStatus |= 1<<6;
	
	DB_SaveLog(&logRecord);
}

void TrackerTask(void)
{
	uint32_t i;
	DATE_TIME timeTemp;
	uint8_t *pt;
	uint32_t gpsTimeSec;
//	if(trackerTaskRun)
	{
		trackerTaskCnt++;
		//SD Card Init
		#ifdef _USE_MMC
		if(((sdfs.fs_type == 0
					|| (disk_memory_size == 0)
					|| (disk_sector_number_max == 0) 
					|| (disk_sector_size == 0)
				)
				&& ((trackerTaskCnt % 30) == 0))
		    && (!FirmwareTask_IsBusy())
		)
		#else
		if(((sdfs.fs_type == 0)
				&& ((trackerTaskCnt % 30) == 0))
		    && (!FirmwareTask_IsBusy())
		)
		#endif
		{
			mscInit(0);
			if(flagSDcardErr)
				mscInit(0);
			if(flagSDcardErr)
			{
				flagSystemStatus |= SYS_SD_CARD_LOSE;
				flagSDcardErr = 0;
				EventReport(SD_REPORT);
			}
			else
			{
				flagSystemStatus &= ~SYS_SD_CARD_LOSE;
			}
		}
		
		if(CheckTimeout(&tGPS_PowerOffTimeout) == SYSTICK_TIMEOUT)
		{
			GPS_Enable();
		}
		//GPS get info
		//GPS_GetInfo();
		TIME_FromGps(&timeTemp,&nmeaInfo);
		gpsTimeSec = TIME_GetSec(&timeTemp);
		if(ACC_IS_ON)
		{
			gpsGotWhenACC_OFF = 0;
			getGpsTimeout = GPS_TRY_TIMEOUT;
			gpsTryCntWhenACC_OFF = 10;
		}
		else
		{
			if(getGpsTimeout)
			{
				if(getGpsTimeout == 1 && (flagSystemStatus == SYS_GPS_LOSE))
				{
					getGpsTimeout = 1;
					GPS_Disable();
					InitTimeout(&tGPS_PowerOffTimeout,SYSTICK_TIME_SEC(10));
				}
				getGpsTimeout--;
			}
		}
		if((nmeaInfo.fix == 1 || nmeaInfo.fix == 2) 
		&& (nmeaInfo.utc.year >= 115) 
		&& (nmeaInfo.HDOP != 99.99)
		&& (nmeaInfo.VDOP != 99.99)
		&& (nmeaInfo.PDOP != 99.99)
		)
		{
			if(nmeaInfo.satinfo.inview)
			{
				if(UpdateRtcTime(gpsTimeSec))
					TIME_FromSec(&sysTime,gpsTimeSec);
			}
		}
		
		if((nmeaInfo.fix == 3) && 
				(nmeaInfo.utc.year >= 113) && 
				(nmeaInfo.sig > 0) && 
				(nmeaInfo.lat != 0) && 
				(nmeaInfo.lon != 0) && 
				(nmeaInfo.lat <= 90 || nmeaInfo.lat >= -90) &&
				(nmeaInfo.lon <= 180  || nmeaInfo.lon >= -180)
			)
		{
			IO_ToggleSetStatus(&io_sys_led,100,3000,IO_TOGGLE_ENABLE,IO_MAX_VALUE);
			gpsLostCnt = 0;
			if(gpsGetCnt++ >= 10)
			{
				gpsGetCnt = 20;
				if(!ACC_IS_ON)
				{
					gpsGotWhenACC_OFF = 1;
					//getGpsTimeout = 0;
				}
				if(gpsFixedFlag == 0)//send a message to server when GPS is fixed
				{
					gpsFixedFlag = 1;
					trackerSaveLog = 1;
				}
				flagSystemStatus &= ~SYS_GPS_LOSE;
				lastNmeaInfo = nmeaInfo;
				timeFixed = 1;
				if(UpdateRtcTime(gpsTimeSec)){
					TIME_FromSec(&sysTime,gpsTimeSec);
					TIME_FromSec(&localTime,gpsTimeSec + sysTimeZone);
				}
			}
		}
		else
		{
			IO_ToggleSetStatus(&io_sys_led,100,1000,IO_TOGGLE_ENABLE,0xffffffff);
			flagSystemStatus |= SYS_GPS_LOSE;
			gpsGetCnt = 0;
			gpsLostCnt++;
			if(gpsLostCnt >= 600)
			{
				gpsLostCnt = 0;
				GPS_Disable();
				InitTimeout(&tGPS_PowerOffTimeout,SYSTICK_TIME_SEC(10));
			}
			if(gpsFixedFlag) // send last gps to server
			{

			}
		}
		
		
		if(flagNewMinTracker)
		{
			DB_SaveAll();
			flagNewMinTracker = 0;
		}
		
	//calc speed
		dv_div_dt = nmeaInfo.speed - lastSpeed;
		lastSpeed = nmeaInfo.speed;
		if(dv_div_dt < 0) dv_div_dt = -dv_div_dt;
		if(nmeaInfo.speed >= 150 || nmeaInfo.speed < CONFIG_SPEED_STOP || nmeaInfo.fix < 3 || nmeaInfo.utc.year < 111 || nmeaInfo.sig < 1 || nmeaInfo.lat == 0 || nmeaInfo.lon == 0 || nmeaInfo.HDOP == 0)
		{
			gpsCurrentSpeed = 0;
		}
		else if(nmeaInfo.speed >= 0 && gpsCurrentSpeed >= 0)
		{
			/* dv/dt*/
			if(dv_div_dt <= 10 )
			{
				if(nmeaInfo.speed >= (gpsCurrentSpeed + SPEED_UP_LIMIT))
				{
					gpsCurrentSpeed += SPEED_UP_LIMIT;
				}
				else if(gpsCurrentSpeed >= (nmeaInfo.speed + SPEED_UP_LIMIT))
				{
					gpsCurrentSpeed -= SPEED_UP_LIMIT;
				}
				else gpsCurrentSpeed =  nmeaInfo.speed;
			}
		}
		
		if(gpsCurrentSpeed < 0) gpsCurrentSpeed= 0;

		
		if(sysCfg.speedSensorRatio == 0)
			currentSpeed = gpsCurrentSpeed;
		else
			currentSpeed = pulseSpeed_km_h;
		
		if(!ACC_IS_ON)
			currentSpeed = 0;
		if(sysCfg.speedSensorRatio == 0)
			systemRecord.mileage += currentSpeed / 3600;

		// over speed warning
		//if(sysCfg.enableWarning)
		{
		#ifdef USER_TEST_CODE
			if(timeOverSpeedCnt == 120)
				testSpeedTemp = 50;//km/h
		#endif
			if(currentSpeed > (float)sysCfg.speedLimit[2]/10)
			{
				buzzStatus = 1;
				timeOverSpeedCnt++;
				if((timeOverSpeedCnt % 30)  == 0) //30s
				{
					trackerSaveLog = 1;
					if(sysCfg.featureSet & FEATURE_BUZZ)
						BuzzerSetStatus(&buzzer1Ctrl,500, 500, BUZZER_TURN_ON,600);		// 10min
				}	
			}
			else
			{
				timeOverSpeedCnt = 0;
				if(buzzStatus) 
				{
					buzzStatus = 0;
					if(sysCfg.featureSet & FEATURE_BUZZ)
						BuzzerSetStatus(&buzzer1Ctrl,50, 50, BUZZER_TURN_ON,3);
				} 
			}
		}
	
		////////////////////////////////////////////////////////////////////////
		// LOGGER
		if(localTime.year < 2016)
		{
			timeFixed = 0;
		}
		else 
		{
			
			//Speed Record
			if(speedBufLen >= 30)
			{
				
				memcpy(speedRecord.speed,speedBuf,30);
				memcpy(speedLogRecord.speed,speedBuf,30);
				
				if((firstSampleFlag == 0) && (speedLogRecord.time > 1400401742))
				{
					DB_SpeedSaveLog(&speedLogRecord);
					DB_SaveSpeedRecord(localTime,&speedRecord);
				} 
				
				speedRecord.hour = localTime.hour;
				speedRecord.min = localTime.min;
				speedRecord.sec = localTime.sec;
				speedRecord.lat = gps.lastGpsInfo.lat;
				speedRecord.lon = gps.lastGpsInfo.lon;
				
				speedLogRecord.time = rtcTimeSec;
				speedLogRecord.sent = 0;
				speedBufLen = 0;
				speedRecordSaveTime = 0;
				firstSampleFlag = 0;
				
			}
			
			if(speedBufLen < sizeof(speedBuf))
			{
				speedBuf[speedBufLen++] = currentSpeed;
			}
			
			TIME_FromSec(&timeTemp,systemRecord.timeSave);
			if(timeTemp.mday != localTime.mday 
				|| timeTemp.month != localTime.month 
				|| timeTemp.year != localTime.year
			)
			{
				systemRecord.timeSave = TIME_GetSec(&localTime);
				DB_SaveAll();
				newDayFlag = 1;
			}
			
			// tính toán nếu như bắt đầu ngày mới
			if(newDayFlag)
			{
				newDayFlag = 0;
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
					if(rfidLoginFlag)
					{
						strcpy((char *)driverRecord.driverName,(char *)sysCfg.driverList[systemRecord.driverIndex].driverName);
						strcpy((char *)driverRecord.licenseNo,(char *)sysCfg.driverList[systemRecord.driverIndex].licenseNo);
					}
					else
					{
						strcpy((char *)driverRecord.driverName,"------");
						strcpy((char *)driverRecord.licenseNo,"------");
					}
					DB_SaveWorkingTime(localTime,&driverRecord);		
					DriverWorkingSendToServer();
					
					systemRecord.beginTime.hour = 0;
					systemRecord.beginTime.min = 0;
					systemRecord.beginTime.sec = 0;
					systemRecord.beginLat = lastNmeaInfo.lat;
					systemRecord.beginLon = lastNmeaInfo.lon;
					systemRecord.drivingTime = 0;
					systemRecord.totalDrivingTime = 0;
					
				}
			}
			//tính toán thời gian lái xe liên tục khi fix được thời gian
			#ifdef USER_START_TIME_CONDITION
			if((startingTime == 0) && systemRecord.drivingTime)
			{
				startingTime = systemRecord.drivingTime + USER_START_TIME_CONDITION;
			}
			#else
			if((startingTime == 0) && systemRecord.drivingTime)
			{
				startingTime = systemRecord.drivingTime;
			}
			#endif
			//lưu trữ giá trị khi fix được thời gian
			//systemRecord.mileage = mileage;
			systemRecord.speed = currentSpeed;
			systemRecord.currentTime.hour = localTime.hour;
			systemRecord.currentTime.min = localTime.min;
			systemRecord.currentTime.sec = localTime.sec;
			systemRecord.lat = lastNmeaInfo.lat;
			systemRecord.lon = lastNmeaInfo.lon;
		}
		if(ioStatus.din[0].bitNew)
			systemRecord.status |= 1<<0;
		else
			systemRecord.status &= ~(1<<0);
		
		if(ioStatus.din[1].bitNew)
			systemRecord.status |= 1<<1;
		else
			systemRecord.status &= ~(1<<1);
		
		if(ioStatus.din[2].bitNew)
			systemRecord.status |= 1<<2;
		else
			systemRecord.status &= ~(1<<2);
		
		if(ioStatus.din[3].bitNew)
			systemRecord.status |= (1<<3);
		else
			systemRecord.status &= ~(1<<3);
		
		if(rfidLoginFlag)
			systemRecord.status |= 1<<7;
		else
			systemRecord.status &= ~(1<<7);
		
		//if(ADC1_9_Value < 7)
		if(0)//thienhaiblue need edit
		{
			if((systemRecord.status & 1<<6) == 0)
				if(lowPowerCnt++ >= 5)
				{
					powerGoodCnt = 0;
					lowPowerCnt = 0;
					systemRecord.status |= 1<<6;//POWER DOWN FLAG
					EventReport(POWER_OFF_REPORT);
				}
		}
		//else if(ADC1_9_Value >= 9)
		if(1)
		{
			if((systemRecord.status & 1<<6))
				if(powerGoodCnt++ >= 5)
				{
					lowPowerCnt = 0;
					powerGoodCnt = 0;
					systemRecord.status &= ~(1<<6);//POWER DOWN FLAG
					EventReport(POWER_ON_REPORT);
				}
		}
		
		//update thời gian lái xe liên tục để lưu trữ vào bộ nhớ flash
		drivingTimeSave.value =  systemRecord.drivingTime;
		parkingTimeSave.value = systemRecord.parkingTime;
		// Tính toán thời gian lái xe liên tục
#ifdef USER_SPEED_FOR_CAR_STATUS
//#if 0
		if((currentSpeed >= CONFIG_SPEED_STOP) && (ACC_IS_ON))
#else
		if(ACC_IS_ON) //ACC_ON
#endif
		{
#ifdef USER_START_TIME_CONDITION
			if(startingTime >= USER_START_TIME_CONDITION)
#endif
			{
				flagStopWarning = 0;
				if(systemRecord.drivingTime == 0)
				{
					systemRecord.beginLat = lastNmeaInfo.lat;
					systemRecord.beginLon = lastNmeaInfo.lon;
				}
				systemRecord.drivingTime++;
				systemRecord.totalDrivingTime++;

				if(flagTotalDrivingTimeWarning == OFF_WARNING)
					flagTotalDrivingTimeWarning = RESET_WARNING;
				if(flagTotalDrivingTimeWarning == OFF_WARNING)
					flagDrivingTimeWarning = RESET_WARNING;
			}
#ifdef USER_START_TIME_CONDITION
			else
				startingTime++;
#endif
			//
			if(((systemRecord.drivingTime + 600) >= sysCfg.drivingTimeLimit)
				&& systemRecord.drivingTime
				&& flagSaveDrivingTime == 0
			)
			{
				flagSaveDrivingTime = 1;
			}
			//Cảnh báo lái xe liên tục quá thời gian cho phép trước 5 phút
			if(((systemRecord.drivingTime + 300) >= (DEFAULT_DRIVING_TIME_LIMIT)) 
				&& systemRecord.drivingTime
				&& flagDrivingTimeWarning != ON_WARNING
			)
			{
				if(sysCfg.featureSet & FEATURE_RFID)
					if(sysCfg.featureSet & FEATURE_BUZZ)
						BuzzerSetStatus(&buzzer1Ctrl,100, 400,BUZZER_TURN_ON, 600); 	// 120 beeps/min, 300 ms interval
				flagDrivingTimeWarning = ON_WARNING;
			}
			//Cảnh báo tổng thời gian lái xe trong ngày trước 5 phút
//			if(((systemRecord.totalDrivingTime + 300) >= (DEFAULT_TOTAL_DRIVING_TIME_LIMIT))
//				&& systemRecord.totalDrivingTime
//				&& flagTotalDrivingTimeWarning != ON_WARNING
//			)
//			{
//				if(sysCfg.featureSet & FEATURE_RFID)
//					if(sysCfg.featureSet & FEATURE_BUZZ)
//						BuzzerSetStatus(&buzzer1Ctrl,100, 400,BUZZER_TURN_ON, 600); 	// 60 beeps, 300 ms interval
//				flagTotalDrivingTimeWarning = ON_WARNING;
//			}
		}
		else //ACC_OFF
		{
			systemRecord.parkingTime++;
			
			//Nếu dừng quá  5 phút thì tính là 1 lần dừng đỗ
			if(systemRecord.parkingTime >= 300 && (flagStopWarning == 0))//5min
			{
				if(systemRecord.parkingTime)
				{
					parkingRecord.currentTime = systemRecord.currentTime;
					parkingRecord.parkingTime = systemRecord.parkingTime;
					parkingRecord.lat = systemRecord.lat;
					parkingRecord.lon = systemRecord.lon;
					DB_SaveParkingCnt(localTime,&parkingRecord);
					systemRecord.parkingTime = 0;
				}
				flagStopWarning = 1;
			}
			//Nếu dừng lại quá 15 phút thì coi như một lần đăng nhập mới
#ifdef USER_STOP_TIME_CONDITION
			if(systemRecord.parkingTime >= USER_STOP_TIME_CONDITION)
#endif
			{
				//Nếu thời gian lái xe liên tục khác 0 thì lưu lại thời gian lái xe liên tục của tài xế hiện tại
				if(systemRecord.drivingTime)
				{
							
					//lưu lại thời gian làm việc của tài xế
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
					
					if(rfidLoginFlag)
					{
						strcpy((char *)driverRecord.driverName,(char *)sysCfg.driverList[systemRecord.driverIndex].driverName);
						strcpy((char *)driverRecord.licenseNo,(char *)sysCfg.driverList[systemRecord.driverIndex].licenseNo);
					}
					else
					{
						strcpy((char *)driverRecord.driverName,"------");
						strcpy((char *)driverRecord.licenseNo,"------");
					}
					DB_SaveWorkingTime(localTime,&driverRecord);
					DriverWorkingSendToServer();
					systemRecord.drivingTime = 0;
				}
				flagSaveDrivingTime = 0;
				startingTime = 0;
			}
			
			//stop warning
			if(flagTotalDrivingTimeWarning != OFF_WARNING)
			{
				flagTotalDrivingTimeWarning = OFF_WARNING;
				if(sysCfg.featureSet & FEATURE_RFID)
					if(sysCfg.featureSet & FEATURE_BUZZ) 
						BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_OFF, 10); 
			}
			if(flagDrivingTimeWarning != OFF_WARNING)
			{
				flagDrivingTimeWarning = OFF_WARNING;
				if(sysCfg.featureSet & FEATURE_RFID)
					if(sysCfg.featureSet & FEATURE_BUZZ) 
						BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_OFF, 10); 
			}
		}
	
		if(flagChangeDriver)
		{
			flagChangeDriver = 0;
			EventReport(RFID_REPORT);
		}
		
	//Log send to server
		if(trackerSaveLog)
		{
			trackerSaveLog = 0;
			EventReport(REGULARLY_REPORT);
			trackingRecord.currentTime.hour = localTime.hour;
			trackingRecord.currentTime.min = localTime.min;
			trackingRecord.currentTime.sec =  localTime.sec;
			trackingRecord.lat = lastNmeaInfo.lat;
			trackingRecord.lon = lastNmeaInfo.lon;
			trackingRecord.pulseSpeed = currentSpeed;
			trackingRecord.gpsSpeed = currentSpeed;
			trackingRecord.driver = systemRecord.driverIndex;
			DB_SaveTrackingRecord(localTime,&trackingRecord);
			
			
		}
	}
}

void DriverWorkingSendToServer(void)
{
	WORKING_TIME_RECORD_LOG log;
	log.time = TIME_GetSec(&sysTime);
	log.beginLat = driverRecord.beginLat;
	log.beginLon = driverRecord.beginLon;
	log.endLat = driverRecord.endLat;
	log.endLon = driverRecord.endLon;
	log.beginTime = driverRecord.beginTime;
	log.endTime = driverRecord.endTime;
	log.driverIndex = sysCfg.driverIndex;
	memcpy((uint8_t *)&logRecord,(uint8_t *)&log,sizeof(WORKING_TIME_RECORD_LOG));
	logRecord.msgType = WORKING_TIME_REPORT;	
	DB_SaveLog(&logRecord);
}

void TrackerGetNewData(void)
{
	trackerSaveLog = 1;
}
	

uint8_t TrackerCrc(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint8_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}


extern uint8_t tcpIpAnswerCmd;
uint16_t AddTrackerPacket(MSG_STATUS_RECORD *logRecordSend,char *data,uint16_t *len)
{
//	uint32_t u32Temp0,u32Temp1;
//	int32_t i32Temp0,i32Temp1;
	char *buff = data;
	char tmpBuf[32],ns,ew;
//	double fTemp;
	DATE_TIME time;
	WORKING_TIME_RECORD_LOG *log;
	buff[0] = 0;
	data[0] = 0;
	TIME_FromSec(&time,logRecordSend->time); 
	
	
	switch(logRecordSend->msgType)
	{
		case REGULARLY_REPORT:
		{
			//device ID
			sprintf(tmpBuf, "$,%s,",sysCfg.id);
			strcat(buff, tmpBuf);
			//YYMMDDHHMMSS
			sprintf(tmpBuf, "%02d%02d%02d,",time.mday,time.month,time.year%100);
			strcat(buff, tmpBuf);
			sprintf(tmpBuf, "%02d%02d%02d,",time.hour, time.min, time.sec);
			strcat(buff, tmpBuf);
			//Operating time
			if(logRecordSend->IOStatus & 1<<4)
				strcat(buff,"A,");
			else
				strcat(buff,"V,");
			
			//GPS lon
			if(logRecordSend->gpsLon <= 0)
				sprintf(tmpBuf, "E00000.0000,");
			else
				sprintf(tmpBuf, "%c%0.4f,",logRecordSend->ew,(double)logRecordSend->gpsLon/100000.0);
			strcat(buff, tmpBuf);
			//GPS lat
			if(logRecordSend->gpsLat <= 0)
				sprintf(tmpBuf, "N0000.0000,");
			else
				sprintf(tmpBuf, "%c%0.4f,",logRecordSend->ns,(double)logRecordSend->gpsLat/100000.0);
			strcat(buff, tmpBuf);
				//GPS Dir
			sprintf(tmpBuf, "%0.3d,",logRecordSend->gpsDir);
			strcat(buff, tmpBuf);
			//GPS speed
			sprintf(tmpBuf, "%03d,",logRecordSend->gpsSpeed/10);
			strcat(buff, tmpBuf);
			//mileage
			sprintf(tmpBuf,"%08d,",(uint32_t)((logRecordSend->mileage + sysCfg.baseMileage/100)*100));
			strcat(buff,tmpBuf);
			//Engine RPM
			sprintf(tmpBuf,"%04d,",logRecordSend->engineRPM);
			strcat(buff,tmpBuf);
			//fuel1
			sprintf(tmpBuf,"%04d,",logRecordSend->fuel1);
			strcat(buff,tmpBuf);
			//fuel2
			sprintf(tmpBuf,"%04d,",logRecordSend->fuel2);
			strcat(buff,tmpBuf);
			//fuelTemper
			sprintf(tmpBuf,"%04d,",logRecordSend->fuelTemper);
			strcat(buff,tmpBuf);
			//gsmSignal

			//ACC is ON
			if(logRecordSend->IOStatus & (1 << 0)) {
				sprintf(tmpBuf, "0,");
				strcat(buff, tmpBuf);
			}else{
				sprintf(tmpBuf, "1,");
				strcat(buff, tmpBuf);
			}
			//EXT1
			if(logRecordSend->IOStatus & (1 << 1)) {
				sprintf(tmpBuf, "1,");
				strcat(buff, tmpBuf);
			}else{
				sprintf(tmpBuf, "0,");
				strcat(buff, tmpBuf);
			}
			//EXT2
			if(logRecordSend->IOStatus & (1 << 2)) {
				sprintf(tmpBuf, "1,");
				strcat(buff, tmpBuf);
			}else{
				sprintf(tmpBuf, "0,");
				strcat(buff, tmpBuf);
			}
			strcat(buff, "0#\r\n");
		}
		break;
		case RFID_REPORT:
		{
			//device ID
			sprintf(tmpBuf, "^,%s,",sysCfg.id);
			strcat(buff, tmpBuf);
			if(logRecordSend->IOStatus & 1<<6)
				strcat(buff, "Drv_ID1,");
			else
				strcat(buff, "Drv_ID0,");
			//YYMMDDHHMMSS
			sprintf(tmpBuf, "%02d%02d%02d,",time.mday,time.month,time.year%100);
			strcat(buff, tmpBuf);
			sprintf(tmpBuf, "%02d%02d%02d,",time.hour, time.min, time.sec);
			strcat(buff, tmpBuf);
			sprintf(tmpBuf, "001;%s;%s,",sysCfg.driverList[logRecordSend->driverIndex].driverName,
																		sysCfg.driverList[logRecordSend->driverIndex].licenseNo);
			strcat(buff, tmpBuf);
			//Operating time
			if(logRecordSend->IOStatus & 1<<4)
				strcat(buff,"A,");
			else
				strcat(buff,"V,");
			
			//GPS lon
			if(logRecordSend->gpsLon <= 0)
				sprintf(tmpBuf, "E00000.0000,");
			else
				sprintf(tmpBuf, "%c%0.4f,",logRecordSend->ew,(double)logRecordSend->gpsLon/100000.0);
			strcat(buff, tmpBuf);
			//GPS lat
			if(logRecordSend->gpsLat <= 0)
				sprintf(tmpBuf, "N0000.0000,");
			else
				sprintf(tmpBuf, "%c%0.4f,",logRecordSend->ns,(double)logRecordSend->gpsLat/100000.0);
			strcat(buff, tmpBuf);
			strcat(buff, "0,");
			//GPS Dir
			sprintf(tmpBuf, "%0.3d,",logRecordSend->gpsDir);
			strcat(buff, tmpBuf);
			//GPS speed
			sprintf(tmpBuf, "%03d,",logRecordSend->gpsSpeed/10);
			strcat(buff, tmpBuf);
			strcat(buff, "0,#\r\n");
		}
		break;
		case SD_REPORT:
		case POWER_ON_REPORT:
		case POWER_OFF_REPORT:
		{
			//device ID
			sprintf(tmpBuf, "^,%s,",sysCfg.id);
			strcat(buff, tmpBuf);
			
			if(logRecordSend->msgType == SD_REPORT)
				strcat(buff, "SD_ERROR,");
			if(logRecordSend->msgType == POWER_ON_REPORT)
				strcat(buff, "PWR_ON,");
			if(logRecordSend->msgType == POWER_OFF_REPORT)
				strcat(buff, "PWR_OFF,");
			
			//YYMMDDHHMMSS
			sprintf(tmpBuf, "%02d%02d%02d,",time.mday,time.month,time.year%100);
			strcat(buff, tmpBuf);
			sprintf(tmpBuf, "%02d%02d%02d,,",time.hour, time.min, time.sec);
			strcat(buff, tmpBuf);
			//Operating time
			if(logRecordSend->IOStatus & 1<<4)
				strcat(buff,"A,");
			else
				strcat(buff,"V,");
			
			//GPS lon
			if(logRecordSend->gpsLon <= 0)
				sprintf(tmpBuf, "E00000.0000,");
			else
				sprintf(tmpBuf, "%c%0.4f,",logRecordSend->ew,(double)logRecordSend->gpsLon/100000.0);
			strcat(buff, tmpBuf);
			//GPS lat
			if(logRecordSend->gpsLat <= 0)
				sprintf(tmpBuf, "N0000.0000,");
			else
				sprintf(tmpBuf, "%c%0.4f,",logRecordSend->ns,(double)logRecordSend->gpsLat/100000.0);
			strcat(buff, tmpBuf);
			strcat(buff, "0,");
			//GPS Dir
			sprintf(tmpBuf, "%0.3d,",logRecordSend->gpsDir);
			strcat(buff, tmpBuf);
			//GPS speed
			sprintf(tmpBuf, "%03d,",logRecordSend->gpsSpeed/10);
			strcat(buff, tmpBuf);
			strcat(buff, "0,#\r\n");
			//*,SN,TIME,DATE,IMEI,SIM SERIES,#
			//*,B123343435,020123,010317,8645287851235,256645665444,#
			//device ID
			if(logRecordSend->msgType == POWER_ON_REPORT)
			{
				sprintf(tmpBuf, "*,%s,",sysCfg.id);
				strcat(buff, tmpBuf);
				//YYMMDDHHMMSS
				
				sprintf(tmpBuf, "%02d%02d%02d,",time.hour, time.min, time.sec);
				strcat(buff, tmpBuf);
				sprintf(tmpBuf, "%02d%02d%02d,",time.mday,time.month,time.year%100);
				strcat(buff, tmpBuf);
				//IMEI
				sprintf(tmpBuf, "%s,",sysCfg.imei);
				strcat(buff, tmpBuf);
				//CCID
				sprintf(tmpBuf, "%s,",sysCfg.ccid);
				strcat(buff, tmpBuf);
				//end
				strcat(buff, "#\r\n");
			}
		}
		break;
		case OVER_4HOUR_REPORT:
		case OVER_SPEED_REPORT:
		case ACC_REPORT:
		break;
		case WORKING_TIME_REPORT:
		{
			log = (WORKING_TIME_RECORD_LOG *)logRecordSend;

			//device ID
			sprintf(tmpBuf, "@,%s,",sysCfg.id);
			strcat(buff, tmpBuf);
			//YYMMDDHHMMSS
			sprintf(tmpBuf, "%02d%02d%02d,",time.mday,time.month,time.year%100);
			strcat(buff, tmpBuf);
			sprintf(tmpBuf, "%s,%s,",sysCfg.driverList[log->driverIndex].driverName,
																		sysCfg.driverList[log->driverIndex].licenseNo);
			strcat(buff, tmpBuf);
			sprintf(tmpBuf, "%02d%02d%02d,",log->beginTime.hour,log->beginTime.min,log->beginTime.sec);
			strcat(buff, tmpBuf);
			//GPS Lon
			if(log->beginLon == 0)
			{
				sprintf(tmpBuf, "E00000.0000,");
			}
			else if(log->beginLon > 0)
			{
				sprintf(tmpBuf, "E%0.4f,",neamFormatLatLng(log->beginLon));
			}
			else
			{
				sprintf(tmpBuf, "W%0.4f,",neamFormatLatLng(-log->beginLon));
			}
			strcat(buff, tmpBuf);
			//GPS Lat
			if(log->beginLat == 0)
			{
				sprintf(tmpBuf, "N00000.0000,");
			}
			else if(log->beginLat > 0)
			{
				sprintf(tmpBuf, "N%0.4f,",neamFormatLatLng(log->beginLat));
			}
			else
			{
				sprintf(tmpBuf, "S%0.4f,",neamFormatLatLng(-log->beginLat));
			}
			strcat(buff, tmpBuf);
			//EndTime
			sprintf(tmpBuf, "%02d%02d%02d,",log->endTime.hour,log->endTime.min,log->endTime.sec);
			strcat(buff, tmpBuf);
			//GPS Lon
			if(log->endLon == 0)
			{
				sprintf(tmpBuf, "E00000.0000,");
			}
			else if(log->endLon > 0)
			{
				sprintf(tmpBuf, "E%0.4f,",neamFormatLatLng(log->endLon));
			}
			else
			{
				sprintf(tmpBuf, "W%0.4f,",neamFormatLatLng(-log->endLon));
			}
			strcat(buff, tmpBuf);
			//GPS Lat
			if(log->endLat == 0)
			{
				sprintf(tmpBuf, "N00000.0000,");
			}
			else if(log->endLat > 0)
			{
				sprintf(tmpBuf, "N%0.4f,",neamFormatLatLng(log->endLat));
			}
			else
			{
				sprintf(tmpBuf, "S%0.4f,",neamFormatLatLng(-log->endLat));
			}
			strcat(buff, tmpBuf);
			strcat(buff, "#\r\n");
		}
		break;
		default:
			break;
	}
	*len = strlen(buff);

	return 0;
}

uint16_t AddTrackerPacket_Speed(MSG_SPEED_RECORD *logRecordSend,char *data,uint16_t *len)
{
//	uint32_t u32Temp0,u32Temp1;
//	int32_t i32Temp0,i32Temp1;
	char *buff = data;
	char tmpBuf[32],ns,ew;
	uint8_t i;
//	double fTemp;
	DATE_TIME time;
	buff[0] = 0;
	data[0] = 0;
	TIME_FromSec(&time,logRecordSend->time); 
	//device ID
	sprintf(tmpBuf, "*,%s,",sysCfg.id);
	strcat(buff, tmpBuf);
	//YYMMDDHHMMSS
	sprintf(tmpBuf, "%02d%02d%02d,",time.mday,time.month,time.year%100);
	strcat(buff, tmpBuf);
	sprintf(tmpBuf, "%02d%02d%02d,",time.hour, time.min, time.sec);
	strcat(buff, tmpBuf);
	for(i = 0;i < 30;i++)
	{
		sprintf(tmpBuf, "%03d,",logRecordSend->speed[i]);
		strcat(buff, tmpBuf);
	}
	strcat(buff, "#\r\n");
	*len = strlen(buff);

	return 0;
}

void PrintTrackerInfo(MSG_STATUS_RECORD *log)
{

}



