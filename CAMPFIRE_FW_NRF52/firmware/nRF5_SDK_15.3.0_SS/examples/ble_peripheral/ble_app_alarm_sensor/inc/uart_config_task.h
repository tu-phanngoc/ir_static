
#ifndef __UART_CONFIG_TASK_H__
#define __UART_CONFIG_TASK_H__

#include "app_config_task.h"

#define  UART_PACKET_SIZE		516
extern Timeout_Type tUartInConfigTimeout;
void UartConfigTaskInit(void);
void UartConfigTask(void);
void UartConfigInput(uint8_t c);

#endif

