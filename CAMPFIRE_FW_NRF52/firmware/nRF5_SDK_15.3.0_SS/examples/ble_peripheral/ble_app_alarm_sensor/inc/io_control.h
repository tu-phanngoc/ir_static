#ifndef __IO_CONTROL__H__
#define __IO_CONTROL__H__

#include <string.h>
#include <stdint.h>
#include "boards.h"

#define IO_INPUT_NUM				5
#define IO_FILLTER_CNT			1
#define KEY_IS_ON()					ACC_IS_ON
#define BELL_FOLLOW_SIGNAL_CFG_TIMEOUT			5//s

typedef struct {
	uint8_t bitOld;
	uint8_t bitNew;
	uint8_t highCnt;
	uint8_t lowCnt;
	uint8_t newUpdate;
	uint8_t updateCnt;
}IOFilterType;

typedef struct {
	uint32_t updateCnt;
	IOFilterType din[IO_INPUT_NUM]; // {KEY - SIGNAL A - B}
}IOstatusType;

typedef struct
{
    uint32_t onTime;
		uint32_t offTime;
		int32_t counter;
		uint8_t status;
		uint8_t enable;
		uint32_t times;
		uint32_t timesSave;
} IO_TOGGLE_TYPE;


#define TIMER_PERIOD					10	//ms

#define IO_STATUS_ON					1
#define IO_STATUS_OFF 				0
#define IO_STATUS_NOCONTROL 	2

#define IO_STATUS_ON_TIME_DFG	(500 / TIMER_PERIOD) /*1s */
#define IO_STATUS_OFF_TIME_DFG	(500 / TIMER_PERIOD) /*1s */

#define IO_TOGGLE_ENABLE		1
#define IO_TOGGLE_DISABLE		0
#define IO_TOGGLE_NOCONTROL	2

#define IO_MAX_TIMES 0xffffffff
#define IO_MAX_VALUE 0xffffffff

#define BuzzerSetStatus IO_ToggleSetStatus

#define 	BUZZER_ON			IO_STATUS_ON
#define 	BUZZER_OFF 		IO_STATUS_OFF
#define  	BUZZER_TURN_ON 		IO_TOGGLE_ENABLE
#define  	BUZZER_TURN_OFF  	IO_TOGGLE_DISABLE

extern IO_TOGGLE_TYPE io_signal;
extern IO_TOGGLE_TYPE io_relay_2;
extern IO_TOGGLE_TYPE io_bell;
extern IO_TOGGLE_TYPE buzzer1Ctrl;
extern IO_TOGGLE_TYPE io_sys_led;
extern IOstatusType  ioStatus;
extern uint8_t configModeExit;
void IO_InitACC_ON(void);
uint8_t IO_ToggleProcess(IO_TOGGLE_TYPE *ioCtrl, uint32_t preodic);
void IO_ToggleSetStatus(IO_TOGGLE_TYPE *ledCtr,uint32_t onTime,uint32_t offTime,uint32_t enable,uint32_t times);
void IO_Init(void);
void IO_Control(void);
void signalTask(void);
#endif

