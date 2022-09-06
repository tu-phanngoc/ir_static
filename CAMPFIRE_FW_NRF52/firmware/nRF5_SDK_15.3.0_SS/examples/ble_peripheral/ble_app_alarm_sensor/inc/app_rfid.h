

#ifndef __APP_RFID_H__
#define __APP_RFID_H__
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "m_nrf_spi.h"
#include "lib_CR95HF.h"
#include "lib_iso15693.h"
#include "lib_M24LRXX.h"
#include "lib_iso14443A.h"
#include "lib_iso14443B.h"
#include "lib_SRIX4k.h"
#include "miscellaneous.h"
#include "system_config.h"


extern uint8_t rfidCardInfo[71];
extern  uint8_t rfidGotCardInfoFlag;
extern  uint8_t rfidDataCnt;

void rfid_app_init(void);
void rfid_process(void);
void rfid_1sec_process(void);
void RFID_Login(uint8_t login);

#endif
