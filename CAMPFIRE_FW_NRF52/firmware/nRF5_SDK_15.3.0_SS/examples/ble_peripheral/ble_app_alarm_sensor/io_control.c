#include "io_control.h"
#include "system_config.h"
#include "boards.h"


IO_TOGGLE_TYPE io_signal;
IO_TOGGLE_TYPE io_bell;
IO_TOGGLE_TYPE io_sys_led;
IO_TOGGLE_TYPE buzzer1Ctrl;
IOstatusType  ioStatus;
uint8_t configModeExit = 1;

void IO_ToggleSetStatus(IO_TOGGLE_TYPE *ioCtrl,uint32_t onTime,uint32_t offTime,uint32_t enable,uint32_t times)		
{
	if(ioCtrl->timesSave != times 
	|| ioCtrl->times == 0
	|| ioCtrl->enable != enable
	|| ioCtrl->onTime != onTime
	|| ioCtrl->offTime != offTime
	)
	{
		ioCtrl->onTime = onTime;
		ioCtrl->offTime = offTime;
		ioCtrl->counter = 0;
		ioCtrl->enable = enable;
		ioCtrl->times = times;
		ioCtrl->timesSave = times;
	}
}													
											
uint8_t IO_ToggleProcess(IO_TOGGLE_TYPE *ioCtrl, uint32_t preodic)	
{
	if(ioCtrl->enable == IO_TOGGLE_ENABLE) 
	{
			if(ioCtrl->counter > preodic)
				ioCtrl->counter -= preodic;
			else ioCtrl->counter = 0;
				
			if(ioCtrl->counter == 0) 
			{
				if(ioCtrl->times) 
				{
					ioCtrl->times--;
					ioCtrl->counter = ioCtrl->offTime + ioCtrl->onTime;
					ioCtrl->status = IO_STATUS_ON;
				}
				else
				{
					ioCtrl->enable = IO_TOGGLE_DISABLE;
					ioCtrl->status = IO_STATUS_OFF;
				}
			}
			
			if(ioCtrl->counter <= ioCtrl->offTime) 
				ioCtrl->status = IO_STATUS_OFF;
	}
	else if(ioCtrl->enable == IO_TOGGLE_DISABLE) 
	{
		ioCtrl->enable = IO_TOGGLE_NOCONTROL;
		ioCtrl->status = IO_STATUS_OFF;
	}
	else
	{
		ioCtrl->status = IO_STATUS_NOCONTROL;
	}
	return ioCtrl->status;
}

