/** 
 * 	ambo_pedometer.h
 *  blue@ambo.com.vn
 *  AMBO TECH Inc.
 */

#ifndef __AMBO_PEDOMETER_H__
#define __AMBO_PEDOMETER_H__

#include <stdint.h>
#include <stdio.h>

#ifndef NULL
#define NULL							(0)
#endif

void vibrate_init(void);
uint8_t vibrate_detect(void);
uint16_t vibrate_sample_update(int16_t x,uint16_t y,uint16_t z);
#endif
