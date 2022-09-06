
#include "comm.h"
#include "uart1.h"
#include "lib/sys_tick.h"
RINGBUF *commTxRingBuf = &UART1_TxRingBuff;
RINGBUF *commRxRingBuf = &UART1_RxRingBuff;

void COMM_Putc(uint8_t c)
{
	UART1_PutChar(c);
}
void COMM_Puts(uint8_t *s)
{
	UART1_PutString((char *)s);
}


int32_t COMM_Getc(uint8_t *c)
{
	return RINGBUF_Get(commRxRingBuf,c);
}


uint8_t COMM_CarrierDetected(void)
{
	return 0;
}

void MODEM_RTS_Set(void)
{
	RTS_PIN_SET;
}
void MODEM_RTS_Clr(void)
{
	RTS_PIN_CLR;
}

void MODEM_DTR_Set(void)
{

}
void MODEM_DTR_Clr(void)
{
	
}

void COMM_ClearTx(void)
{
	
}

void MODEM_MOSFET_On(void)
{
	GSM_MOSFET_PIN_SET;
}

void MODEM_MOSFET_Off(void)
{
	GSM_MOSFET_PIN_CLR;
}

void MODEM_POWER_Set(void)
{
	POWER_PIN_SET;
}

void MODEM_POWER_Clr(void)
{
	POWER_PIN_CLR;
}

void MODEM_RESET_Set(void)
{
	
}

void MODEM_RESET_Clr(void)
{
	
}

void MODEM_Wakeup(void)
{
	MODEM_DTR_Clr();
	SysTick_DelayMs(100);
}

void MODEM_Sleep(void)
{
	MODEM_DTR_Set();
	SysTick_DelayMs(100);
}
	

void MODEM_UartInit(uint32_t baudrate)
{
	
}



