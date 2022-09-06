#ifndef __UART1_H__
#define __UART1_H__

#include <stdint.h>
#include "lib/ringbuf.h"

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

void UART1_Init(void);
uint8_t UART1_PutChar (uint8_t ch);
void UART1_PutString (char *s);
void UART1_RingBufInit(void);
void UART1_DeInit(void);
extern   RINGBUF UART1_RxRingBuff;
extern RINGBUF UART1_TxRingBuff;

#endif

