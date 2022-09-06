#include "led.h"
#include "io_control.h"

IO_TOGGLE_TYPE	io_led1;	

void LedInit(void)			
{
	LED_SYS_PIN_SET;
	IO_ToggleSetStatus(&io_led1,IO_STATUS_ON_TIME_DFG,0,IO_STATUS_OFF_TIME_DFG,0xffffffff);
}


void CtrLed(uint32_t time)
{
	if(IO_ToggleProcess(&io_led1,50)){
		LED_ALARM_PIN_SET;
	}else{
		LED_ALARM_PIN_CLR;
	}
}	
