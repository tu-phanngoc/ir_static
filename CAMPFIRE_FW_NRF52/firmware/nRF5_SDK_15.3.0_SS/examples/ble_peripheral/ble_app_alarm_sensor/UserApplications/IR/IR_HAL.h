#ifndef __IR_HAL_H
#define __IR_HAL_H


#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

//#include "stm32f0xx_hal.h"
#define DEBUG_PRINT NRF_LOG_INFO

#define IR_INPUT_GPIO		GPIOC
#define IR_INPUT_PIN		GPIO_PIN_3

#define IR_OUTPUT_GPIO GPIOB
#define IR_OUTPUT_PIN GPIO_PIN_0

void IR_PinClear();
void IR_PinSet();

void IR_GPIO_PinSet();
void IR_GPIO_PinClear();

void HAL_Delay_us(uint32_t Delay);

void Timer_Inc_Tick();
#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_HAL_H */