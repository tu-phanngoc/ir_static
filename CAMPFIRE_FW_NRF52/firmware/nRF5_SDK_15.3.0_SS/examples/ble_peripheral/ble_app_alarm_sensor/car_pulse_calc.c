
#include "car_pulse_calc.h"
#include "app_tracking.h"

uint32_t newCntTMR = 0,lastCntTMR = 0;
int32_t pulseSpeedTimeIn_Us = 0;
uint32_t pulseSpeedCnt = 0;
#define PULSE_SPEED_SAMPLE 5
uint32_t pulseSpeedTimeIn_Us_Avg = 0;
uint64_t pulseSpeedPerHour;
uint32_t resetPulseSpeedCnt = 0;

float pulseSpeed_km_h = 0;
float pulseSpeed_km_h_new = 0;



void CarPulseCalc_Handle(void)
{
		//if(sysCfg.speedSensorRatio >= 1000)
		//	resetPulseSpeedCnt = 2000;//1s
		//else
		resetPulseSpeedCnt = 2000;
		if(sysCfg.speedSensorRatio)
			systemRecord.mileage += 1000.0/(float)sysCfg.speedSensorRatio/*xung/km*/;
		pulseSpeedCnt++; //thienhaiblue need edit
//		newCntTMR = timer2Cnt & 0x7FF;
//		newCntTMR *= 1000;//us
//		newCntTMR += TIM2->CNT;
		if(newCntTMR >= lastCntTMR)
		{
			pulseSpeedTimeIn_Us = newCntTMR - lastCntTMR;
		}
		else
		{
			pulseSpeedTimeIn_Us = (0x7FF + 1)*1000 + newCntTMR;
			pulseSpeedTimeIn_Us -= lastCntTMR;
		}
		lastCntTMR = newCntTMR;
		
		pulseSpeedTimeIn_Us_Avg += pulseSpeedTimeIn_Us;
		pulseSpeedTimeIn_Us_Avg >>= 1;
		if(pulseSpeedTimeIn_Us_Avg)
		{
			pulseSpeedPerHour = (3600000000/pulseSpeedTimeIn_Us_Avg);
			pulseSpeed_km_h_new = ((float)(pulseSpeedPerHour*10/sysCfg.speedSensorRatio))/10;
			
		}
		else
			pulseSpeed_km_h_new = 0;
		if(pulseSpeed_km_h_new < 150)
		{
			if((pulseSpeed_km_h >= pulseSpeed_km_h_new))
			{
				if(((pulseSpeed_km_h - pulseSpeed_km_h_new) < 5/*km/h*/))
					pulseSpeed_km_h = pulseSpeed_km_h_new;
				else
					pulseSpeed_km_h -= 2;
			}
			else if((pulseSpeed_km_h < pulseSpeed_km_h_new))
			{
				if(((pulseSpeed_km_h_new - pulseSpeed_km_h) < 5/*km/h*/))
					pulseSpeed_km_h = pulseSpeed_km_h_new;
				else
					pulseSpeed_km_h += 2;
			}
		}
		else
			pulseSpeed_km_h = 0;
}