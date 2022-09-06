

#ifndef __APP_TRACKING_H__
#define __APP_TRACKING_H__
#include <stdint.h>
#include <string.h>
#include "fatfs.h"
#include "ff_gen_drv.h"
#include "sst25.h"
#include "system_config.h"
#include "lib/sys_time.h"
#include "lib/sys_tick.h"
#include "uart1.h"
#include "ampm_gsm_main_task.h"
#include "app_config_task.h"
#include "tcp_ip_task.h"
#include "sms_task.h"
#include "firmware_task.h"
#include "io_control.h"
#include "io_process_action.h"
#include "adc_task.h"

#define SMS_TIME_CHECK 600

extern AMPM_GPS gps;

#define nmeaInfo	gps.gpsInfo
#define lastNmeaInfo	gps.lastGpsInfo


extern FATFS sdfs;

void tracking_app_init(void);
void AppRun(void);
#endif

