

#include "lib/sys_tick.h"
#include "spi2uart1.h"
#include "lib/sys_time.h"
#include "system_config.h"
#include "db.h"
#include "tracker.h"
#include "qcvn31_2014.h"
#include "database_app.h"
#include "ff.h"
#include "app_config_task.h"
#include "gps/ampm_gps.h"
#include "app_tracking.h"

extern FATFS sdfs;
extern uint8_t gsmStatus;
extern SYSTEM_RECORD systemRecord;
extern LOG_TYPE logSource;
static uint8_t checksum(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint8_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}

void QCVN_ProcessCmd(char *cmd_in,char *cmd_out,void ( *printOutFunc) (char *s))
{
	char *pt;
	char tempBuf[256], buff[128],buff1[32],buff2[32],buff3[32];
	uint16_t resLen = 0;
	uint32_t t1,t2,t3,t4,t5,t6,offset,len;
	int32_t filSize,i,driverIndex;
	DATE_TIME tm;
	uint8_t dataType,typeCmd,mday,month,crc;
	uint16_t year;
	FIL fil;
	FRESULT res;
	PARKING_RECORD parkingRecord;
	TRACKING_RECORD trackingRecord;
	WORKING_TIME_RECORD driverRecord;
	SPEED_RECORD speedRecord;
	if(cmd_out)
		cmd_out[0] = '\0';
	pt = strstr(cmd_in,"READ");
	if(pt && cmd_in[12] == '#')
	{
		logSource = NO_LOG;
		sscanf(pt,"READ%02d%02d%02d%02d#",&t1,&t2,&t3,&t4);
		if(t1 <=  6 && t2 <= 31 && t3 <= 12 && t4 < 100)
		{
			dataType = t1;
			mday = t2;
			month = t3;
			year = t4 + 2000;
			typeCmd = dataType;
			do{
				switch(typeCmd)
				{
					case 0://Truyền toàn bộ số liệu
						
					break;
					case 1://Dữ liệu nhập liệu ban đầu
						//Đơn vị cung cấp thiết bị
						resLen = sprintf(tempBuf,"1,%s",(char *)sysCfg.companyName);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Kiểu loại TBGSHT
						resLen = sprintf(tempBuf,"2,%s",(char *)sysCfg.companyName);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Số seri của TBGSHT
						resLen = sprintf(tempBuf,"3,%s",(char *)sysCfg.id);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Thông tin biển số xe
						resLen = sprintf(tempBuf,"4,%s",(char *)sysCfg.plateNo);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Phương pháp đo tốc độ
						if(sysCfg.speedSensorRatio)
							resLen = sprintf(tempBuf,"5,CAR");
						else
							resLen = sprintf(tempBuf,"5,GPS");
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Cấu hình xung / km
						resLen = sprintf(tempBuf,"6,%d",sysCfg.speedSensorRatio);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Cấu hình vận tốc tối đa
						resLen = sprintf(tempBuf,"7,%d",sysCfg.speedLimit[2]/10);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Ngày lắp đặt / sửa đổi thiết bị
						TIME_FromSec(&tm,sysCfg.deviceInstallTime);
						resLen = sprintf(tempBuf,"8,%04d/%02d/%02d",tm.year,tm.month,tm.mday);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Ngày cập nhật phần mềm thiết bị
						TIME_FromSec(&tm,sysCfg.deviceFirmwareUpdateTime);
						resLen = sprintf(tempBuf,"9,%04d/%02d/%02d",tm.year,tm.month,tm.mday);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Tinh trang GSM
						resLen = sprintf(tempBuf,"10,%d",gsmStatus);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Tinh trang GPS
						if(gps.gpsInfo.fix >= 3)
							resLen = sprintf(tempBuf,"11,1");
						else
							resLen = sprintf(tempBuf,"11,0");
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Tình trạng bộ nhớ
						if(sdfs.fs_type == 0)
							resLen = sprintf(tempBuf,"12,0");
						else
							resLen = sprintf(tempBuf,"12,1");
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Tình trạng bộ nhớ
						resLen = sprintf(tempBuf,"13,%ld",30*1024*1024);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Thông tin lái xe hiện tại
						resLen = sprintf(tempBuf,"14,%s,%s",sysCfg.driverList[sysCfg.driverIndex].driverName,sysCfg.driverList[sysCfg.driverIndex].licenseNo);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Thoi gian lai xe lien tuc
						resLen = sprintf(tempBuf,"15,%d",(uint32_t)systemRecord.drivingTime/60);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Thong tin ve gps
						resLen = sprintf(tempBuf,"16,%0.5f,%0.5f",gps.gpsInfo.lat,gps.gpsInfo.lon);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Thong tin ve gps
						resLen = sprintf(tempBuf,"17,%0.1f",gps.gpsInfo.speed);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						//Thời gian của thiết bị
						resLen = sprintf(tempBuf,"18,%04d/%02d/%02d %02d:%02d:%02d",localTime.year,localTime.month,localTime.mday,localTime.hour, localTime.min, localTime.sec);
						resLen = sprintf(buff,"$GSHT,1,%02d,<%s>,",resLen,tempBuf);
						sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
						if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
						
					break;
					case 2://Thời gian làm việc của lái xe
						
						//for(t1 = 0; t1  < CONFIG_MAX_DRIVERS;t1++)
						{
							sprintf(tempBuf, "MMC:/%02d%02d%02d.drv", year%100, month, mday);
							if((res = f_open(&fil, tempBuf, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK)
							{
								resLen = sprintf(tempBuf,"00:00:00,0.00000,0.00000,0,0");
								resLen = sprintf(buff,"$GSHT,2,%02d,<%s>,",resLen,tempBuf);
								sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
								if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
							}
							else
							{
								filSize = f_size(&fil);
								filSize = (f_size(&fil) /sizeof(WORKING_TIME_RECORD))  * sizeof(WORKING_TIME_RECORD);
								while(filSize > 0)
								{
									filSize -= sizeof(WORKING_TIME_RECORD);
									if(filSize >= 0)
									{
										f_lseek(&fil, filSize);
										res = f_read(&fil, buff, sizeof(WORKING_TIME_RECORD), (UINT *)&len);
										if((res == FR_OK) && len == sizeof(WORKING_TIME_RECORD))
										{
											driverRecord = *(WORKING_TIME_RECORD *)&buff;
											crc = checksum((uint8_t *)buff,sizeof(WORKING_TIME_RECORD) - 1);
											if(driverRecord.crc == crc)
											{
												resLen = sprintf(tempBuf,"%s,%s,%02d:%02d:%02d,%0.5f,%0.5f,%02d:%02d:%02d,%0.5f,%0.5f,%d",
												driverRecord.driverName,
												driverRecord.licenseNo,
												driverRecord.beginTime.hour,
												driverRecord.beginTime.min,
												driverRecord.beginTime.sec,
												driverRecord.beginLat,
												driverRecord.beginLon,
												driverRecord.endTime.hour,
												driverRecord.endTime.min,
												driverRecord.endTime.sec,
												driverRecord.endLat,
												driverRecord.endLon,
												driverRecord.totalDrivingTime/60
											);
												resLen = sprintf(buff,"$GSHT,2,%02d,<%s>,",resLen,tempBuf);
												sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
												if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
											}
										}
									}	
								}
							}
						}
					break;
					case 3://Số lần và thời gian dừng, đỗ xe
							
							sprintf(tempBuf, "MMC:/%02d%02d%02d.prk", year%100, month, mday);
							if((res = f_open(&fil, tempBuf, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK)
							{
								resLen = sprintf(tempBuf,"00:00:00,0.00000,0.00000,0");
								resLen = sprintf(buff,"$GSHT,3,%02d,<%s>,",resLen,tempBuf);
								sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
								if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
							}
							else
							{
								filSize = f_size(&fil);
								filSize = (f_size(&fil) /sizeof(PARKING_RECORD))  * sizeof(PARKING_RECORD);
								while(filSize > 0)
								{
									filSize -= sizeof(PARKING_RECORD);
									if(filSize >= 0)
									{
										f_lseek(&fil, filSize);
										res = f_read(&fil, buff, sizeof(PARKING_RECORD), (UINT *)&len);
										if((res == FR_OK) && len == sizeof(PARKING_RECORD))
										{
											parkingRecord = *(PARKING_RECORD *)&buff;
											crc = checksum((uint8_t *)buff,sizeof(PARKING_RECORD) - 1);
											if(parkingRecord.crc == crc)
											{
												tm.year = year;
												tm.month = month;
												tm.mday = mday;
												tm.hour = parkingRecord.currentTime.hour;
												tm.min = parkingRecord.currentTime.min;
												tm.sec = parkingRecord.currentTime.sec;
												t1 = TIME_GetSec(&tm);
												t1 -= parkingRecord.parkingTime;
												TIME_FromSec(&tm,t1);
												resLen = sprintf(tempBuf,"%02d:%02d:%02d,%0.5f,%0.5f,%d",
													tm.hour,
													tm.min,
													tm.sec,
													parkingRecord.lat,
													parkingRecord.lon,
													(parkingRecord.parkingTime/60)
												);
												resLen = sprintf(buff,"$GSHT,3,%02d,<%s>,",resLen,tempBuf);
												sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
												if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
											}
										}
									}	
								}
							}
					break;
					case 4://Hành trình của xe
						for(i = 23;i >= 0;i--)
						{
							sprintf(tempBuf, "MMC:/%02d%02d%02d%d.trk", year%100, month, mday,i);
							if((res = f_open(&fil, tempBuf, FA_READ|FA_OPEN_EXISTING)) == FR_OK)
							{
								filSize = f_size(&fil);
								filSize = (f_size(&fil) /sizeof(TRACKING_RECORD))  * sizeof(TRACKING_RECORD);
								while(filSize > 0)
								{
									filSize -= sizeof(TRACKING_RECORD);
									if(filSize >= 0)
									{
										f_lseek(&fil, filSize);
										res = f_read(&fil, buff, sizeof(TRACKING_RECORD), (UINT *)&len);
										if((res == FR_OK) && len == sizeof(TRACKING_RECORD))
										{
											trackingRecord = *(TRACKING_RECORD *)&buff;
											crc = checksum((uint8_t *)buff,sizeof(TRACKING_RECORD) - 1);
											if(trackingRecord.crc == crc)
											{
												resLen = sprintf(tempBuf,"%02d:%02d:%02d,%0.5f,%0.5f,%d,%d",
													trackingRecord.currentTime.hour,
													trackingRecord.currentTime.min,
													trackingRecord.currentTime.sec,
													trackingRecord.lat,
													trackingRecord.lon,
													(uint32_t)trackingRecord.pulseSpeed,
													(uint32_t)trackingRecord.gpsSpeed
												);
												resLen = sprintf(buff,"$GSHT,4,%02d,<%s>,",resLen,tempBuf);
												sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
												if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
											}
										}
									}
								}
								f_close(&fil);
							}
						}
					break;
					case 5://Vận tốc từng giây của xe
						for(i = 23;i >= 0;i--)
						{
							sprintf(tempBuf, "MMC:/%02d%02d%02d%d.spd", year%100, month, mday,i);
							if((res = f_open(&fil, tempBuf, FA_READ|FA_OPEN_EXISTING)) == FR_OK)
							{
								filSize = f_size(&fil);
								filSize = (f_size(&fil) /sizeof(SPEED_RECORD))  * sizeof(SPEED_RECORD);
								while(filSize > 0)
								{
									filSize -= sizeof(SPEED_RECORD);
									if(filSize >= 0)
									{
										f_lseek(&fil, filSize);
										res = f_read(&fil, buff, sizeof(SPEED_RECORD), (UINT *)&len);
										if((res == FR_OK) && len == sizeof(SPEED_RECORD))
										{
											speedRecord = *(SPEED_RECORD *)&buff;
											crc = checksum((uint8_t *)buff,sizeof(SPEED_RECORD) - 1);
											if(speedRecord.crc == crc)
											{
												resLen = sprintf(tempBuf,"%02d:%02d:%02d",
													speedRecord.hour,
													speedRecord.min,
													speedRecord.sec
												);
												for(t1 = 0;t1 < 30;t1++)
												{
													 resLen += sprintf(&tempBuf[resLen],",%d",speedRecord.speed[t1]);
												}
												
												resLen = sprintf(buff,"$GSHT,5,%02d,<%s>,",resLen,tempBuf);
												sprintf(&buff[resLen],"%03d#",checksum((uint8_t *)buff,resLen));
												if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
											}
										}
									}
								}
								f_close(&fil);
							}
						}
					break;
				}
				typeCmd++;
				watchdogFeed[WTD_MAIN_LOOP] = 0; 
			}while((typeCmd <= 5) && (dataType == 0));
		}
		if(cmd_out){strcat(cmd_out,buff);}else if(printOutFunc){printOutFunc(buff);}
	}
}


