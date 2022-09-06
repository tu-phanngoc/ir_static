#include "uart_config_task.h"





void CfgUARTSendData(CFG_PROTOCOL_TYPE *data);


CFG_PROTOCOL_TYPE UART_ProtoRecv;
CFG_PROTOCOL_TYPE UART_ProtoSend;
uint8_t UART_DataBuff[UART_PACKET_SIZE];
PARSER_PACKET_TYPE UART_parserPacket;
Timeout_Type checkUart;
Timeout_Type tUartInConfigTimeout;
uint32_t uart1DataHead;
void UartConfigTaskInit(void)
{
	AppConfigTaskInit();
	InitTimeout(&checkUart,SYSTICK_TIME_MS(10));
	UART_ProtoRecv.dataPt = UART_DataBuff;
	UART_parserPacket.state = CFG_CMD_WAITING_SATRT_CODE;
	UART_parserPacket.lenMax = UART_PACKET_SIZE;
	
}

void UartConfigInput(uint8_t c)
{
	if(CfgParserPacket(&UART_parserPacket,&UART_ProtoRecv,c) == 0)
	{
		InitTimeout(&tUartInConfigTimeout,SYSTICK_TIME_SEC(10));
	}
}


//void UartConfigTask(void)
//{
//	if(CheckTimeout(&checkUart) == SYSTICK_TIMEOUT)
//	{
//		InitTimeout(&checkUart,SYSTICK_TIME_MS(1));	
//		CfgProcessData(&UART_ProtoRecv,&UART_ProtoSend,UART_DataBuff,UART_parserPacket.lenMax - 4,1);
//		CfgUARTSendData(&UART_ProtoSend);
//	}
//}

void UartConfigTask(void)
{
	uint8_t c;
	uint32_t u32Temp;
	if(CheckTimeout(&checkUart) == SYSTICK_TIMEOUT)
	{
		InitTimeout(&checkUart,SYSTICK_TIME_MS(10));	
		u32Temp = RINGBUF_GetFill(&configTaskRingBuf);
		if(uart1DataHead != u32Temp && RINGBUF_GetFill(&USART3_RxRingBuff) == 0)
			uart1DataHead = u32Temp;
		else
		{
			//while(RINGBUF_Get(&USART3_RxRingBuff,&c) == 0)
			{
					if(CfgParserPacket(&UART_parserPacket,&UART_ProtoRecv,c) == 0)
					{
						InitTimeout(&tUartInConfigTimeout,SYSTICK_TIME_SEC(10));
					}
			}
			CfgProcessData(&UART_ProtoRecv,&UART_ProtoSend,UART_DataBuff,UART_parserPacket.lenMax - 4,1);
			CfgUARTSendData(&UART_ProtoSend);
		}
	}
}


void CfgUARTSendData(CFG_PROTOCOL_TYPE *data)
{
	uint16_t i;
	if(data->length)
	{
		data->length = 0;
	}
}

