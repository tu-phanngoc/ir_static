#ifndef __IR_INTERFACE_H
#define __IR_INTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"
#define IRINTERFACE_DATASET_MAX_IDX				640


// DO NOT CHANGE POSTITION
typedef enum ac_type_t {
  AC_TOSHIBA,
  AC_DAIKIN152,
  AC_HITACHI,
  AC_DAIKIN,
  AC_PANASONIC,
  AC_FUNIKI, // =5
  AC_MITSUBISHI144,
  AC_MITSUBISHI112,
  AC_OG104AC,
  AC_OLIMPIA136AC,
  AC_SHARP104, // 10
  AC_SANYO,
  AC_COOLIX,
  AC_MITSUBISHI136,
  AC_PANASONICAC32_LONG,
  AC_PANASONICAC32_SHORT, // 15
  AC_LG,
  AC_CASPER104,
  AC_MITSUBISHI_HEAVY88,
  AC_MITSUBISHI_HEAVY152,
  AC_DAIKIN2,
  AC_DAIKIN216,
  AC_DAIKIN160,
  AC_DAIKIN176,
  AC_DAIKIN128,
  AC_DAIKIN64, 
  AC_CASPER343,
  AC_CHIGO96AC,
  AC_VESTEL,
  AC_GREE,
  AC_SAMSUNG,
  AC_MAX,
} ac_type_t;

typedef enum {
	TYPE_TOSHIBA = 0,
	TYPE_DAIKIN_2,
	TYPE_HITACHI,
	TYPE_LG
}DEVICE_TYPE_E;

/* Base struct */
typedef struct ComClass_t{  
	int 			(*open)(void);
	void			(*IR_EncodeBLEToIR)(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
	void			(*IR_Decode)(int16_t* input, uint8_t* output);
	void			(*IR_Decode_Secondary)(int16_t* input, uint8_t* output);
	int				(*IsEnableIrTx)(void);
	void			(*SetIrTxState)(int state);
    // And data goes here.
	uint32_t	u32Type;
	uint32_t	u32DataSetRecvMaxIdx;
	uint32_t	u32DataSetTransmitMaxIdx;
} ComClass_t;

void IRInterface_EncodeBLEToIR(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IRInterface_Decode();
uint32_t IRInterface_GetDataSetRecvMaxIdx();
uint32_t IRInterface_GetDataSetTransmitMaxIdx();
int IRInterface_IsEnableIrTx(void);
void IRInterface_SetIrTxState(int state);
void IRInterface_PrepareDataToSend(int16_t* input);
void IRInterface_TransmitIR(int16_t* data, uint32_t len);
bool IRInterface_IsTransmitBusy();
void IRInterface_TransmitCompleted(void);

void IR_TX_WDT_Tick(void);
void IR_TX_WDT_Trigger(void);
void IR_TX_WDT_Task(void);

extern ComClass_t gIRInterface;
extern int16_t g_i16RawIRBits[IRINTERFACE_DATASET_MAX_IDX];
extern uint16_t gu16IRRawData[IRINTERFACE_DATASET_MAX_IDX];
extern bool gbIR_IsRxbusy;
extern bool g_bIRCompletedFlag;
extern bool g_bIRCompletedTrigger;
extern uint32_t gu32IRTxTrigger;
#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_HAL_H */