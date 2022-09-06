/**
* \file
*         uart2 driver
* \author
*         Nguyen Van Hai <blue@ambo.com.vn>
*/

#include "uart1.h"
#include "app_uart.h"
#include "boards.h"

extern uint8_t modemCmdMode;
extern void AT_ComnandParser(char c);
extern RINGBUF configTaskRingBuf;

 uint8_t UART1_RxBuff[1024] = {0};
 RINGBUF UART1_RxRingBuff;

 uint8_t UART1_TxBuff[256] = {0};
 RINGBUF UART1_TxRingBuff;



void UART1_RingBufInit(void)
{
	RINGBUF_Init(&UART1_RxRingBuff,UART1_RxBuff,sizeof(UART1_RxBuff));
	RINGBUF_Init(&UART1_TxRingBuff,UART1_TxBuff,sizeof(UART1_TxBuff));
}



/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' '\n' (hex 0x0A) or if the string has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
    uint8_t c;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
							UNUSED_VARIABLE(app_uart_get(&c));
							RINGBUF_Put(&UART1_RxRingBuff,c);
							//AT_ComnandParser(c);
            break;

        case APP_UART_COMMUNICATION_ERROR:
            //APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
/**@snippet [Handling the data received over UART] */




/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
void UART1_Init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_ENABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
    };
		UART1_RingBufInit();
    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}

void UART1_DeInit(void)
{
	app_uart_close();
}
/**@snippet [UART Initialization] */


uint8_t UART1_PutChar (uint8_t ch) 
{
	while (app_uart_put(ch) == NRF_ERROR_BUSY);
	return ch;
}

void UART1_PutString (char *s) 
{
   while(*s)
	{
		app_uart_put(*s++);
	}
}




/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/




