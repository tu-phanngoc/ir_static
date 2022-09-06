#ifndef __PIR_DRIVER_H__
#define __PIR_DRIVER_H__

#include "boards.h"
#include "vt_common.h"


#if defined(USE_PCA10040)
#define PIR_SENSOR_PIN       13
#define PIR_SENSOR_PIN_PULL    NRF_GPIO_PIN_PULLUP
#else
#define PIR_SENSOR_PIN       23
#define PIR_SENSOR_PIN_PULL    NRF_GPIO_PIN_NOPULL
#endif

#endif
