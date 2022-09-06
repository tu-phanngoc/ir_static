#ifndef __IR_COMMON_H
#define __IR_COMMON_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#include "IR_DataConverter.h"

#define IR_CMD_TIMEOUT	1

typedef struct {
	uint8_t state;
	uint16_t interval;
} IR_DATA_t;

void UserTickInc();
void IRReceiveRawData(void);


void HAL_Delay_us(uint32_t Delay);
void TransmitIRBits_reverse(uint32_t* Data);

void IR_SendPause(uint32_t Interval_us);
void IR_SendPulse(uint32_t Interval_us);


int  isIrTxEnable(void);
void setIrTxState(int state);

//extern IR_DATA_t DataSet[DATASET_MAX_INDEX];
extern uint32_t dataSetIndex;

extern uint32_t IR_State;

extern volatile uint32_t gStateTimer;
extern volatile uint32_t gInputState;
extern uint32_t gOldState;
extern volatile uint32_t gu32IRCmdTimeout;
void IR_Task();
void IR_Task_Learning();
#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_HAL_H */