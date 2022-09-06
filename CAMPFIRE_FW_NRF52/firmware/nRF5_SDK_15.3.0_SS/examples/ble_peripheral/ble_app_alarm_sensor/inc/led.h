#ifndef __LED__H__
#define __LED__H__


#include <stdint.h>
#include "boards.h"
#include "io_control.h"

extern IO_TOGGLE_TYPE	io_led1;




void LedInit(void);
void CtrLed(uint32_t time);

#endif

