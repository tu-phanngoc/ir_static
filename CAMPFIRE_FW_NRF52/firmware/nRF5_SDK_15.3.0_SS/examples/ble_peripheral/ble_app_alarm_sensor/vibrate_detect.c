/**
* \file
*         vibrate detect
* \author
*         IKY Company
*/
#include <stdint.h>
#include "vibrate_detect.h"
#include "hal_acc.h"


#define SAMPLE_NUM				30

int16_t sample_x[SAMPLE_NUM];
int16_t sample_y[SAMPLE_NUM];
int16_t sample_z[SAMPLE_NUM];
int16_t sample_t[SAMPLE_NUM];
uint8_t sample_index = 0;

int16_t b_filter(int16_t *sample,int16_t *sample_t,int16_t* min,int16_t* max);


void vibrate_init(void)
{
	uint16_t index;
	sample_index = 0;
	for(index = 0;index < SAMPLE_NUM;index++)
	{
		sample_x[index] = 0;
		sample_y[index] = 0;
		sample_z[index] = 0;
	}
}


uint16_t vibrate_sample_update(int16_t x,uint16_t y,uint16_t z)
{
	sample_x[sample_index] = x;
	sample_y[sample_index] = y;
	sample_z[sample_index] = z;
	sample_index++;
	if(sample_index >= SAMPLE_NUM)
	{
		sample_index = 0;
		return 1;
	}
	return 0;
}


uint8_t vibrate_detect(void)
{
	int16_t deltax,deltay,deltaz;
	uint8_t have_vibrate_flag = 0;
	int16_t maxValueX,maxValueY,maxValueZ,minValueX,minValueY,minValueZ;
	//Testing
	b_filter(sample_x,sample_t,&minValueX,&maxValueX);
	b_filter(sample_y,sample_t,&minValueY,&maxValueY);
	b_filter(sample_z,sample_t,&minValueZ,&maxValueZ);	
	//
	deltax = maxValueX - minValueX;
	deltay = maxValueY - minValueY;
	deltaz = maxValueZ - minValueZ;
	//
	if(deltax < 0) deltax = -deltax;
	if(deltay < 0) deltay = -deltay;
	if(deltaz < 0) deltaz = -deltaz;	
	
	if(deltax > ACCEL_MOVING_THS || deltay > ACCEL_MOVING_THS || deltaz > ACCEL_MOVING_THS)
	{
		have_vibrate_flag =  1;
	}
	return have_vibrate_flag;
}
 


int16_t b_filter(int16_t *sample,int16_t *sample_t,int16_t* min,int16_t* max)
{
	uint16_t i,j;
	int16_t i16Temp = 0;
	int32_t i32Temp;
	//
	for(i=0;i < SAMPLE_NUM;i++){
		sample_t[i] = sample[i];
	}
	//ascending
	for(i=0;i<SAMPLE_NUM-1;i++){
		for (j = i + 1; j < SAMPLE_NUM; j++){
			if (sample_t[i] > sample_t[j]){
				i16Temp = sample_t[i];
        sample_t[i] = sample_t[j];
        sample_t[j] = i16Temp;
			}
		}
	}
	i32Temp = sample_t[0] + sample_t[1];
	*min =  (int16_t)(i32Temp>>1);
	i32Temp = sample_t[SAMPLE_NUM-1] + sample_t[SAMPLE_NUM-2];
	*max =  (int16_t)(i32Temp>>1);
	return 0;
}
 
 
