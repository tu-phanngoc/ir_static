#ifndef __IR_DEVICECONSTRUCTOR_H
#define __IR_DEVICECONSTRUCTOR_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "IR_Interface.h"



void IR_ConstructDeviceType();
void IR_ConstructSpecificDeviceType(uint32_t deviceType);
ac_type_t IRInterface_DetectDeviceType(uint32_t u32MaxIdx, int16_t* input);



#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_HAL_H */
