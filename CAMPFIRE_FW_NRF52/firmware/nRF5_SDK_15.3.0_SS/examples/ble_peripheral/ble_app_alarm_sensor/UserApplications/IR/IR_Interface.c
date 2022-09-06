#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"
#include "nrf_delay.h"
#include "ir_lib.h"
#include "IR_Interface.h"
#include "app_indicator_led.h"
#include "app_led_animation.h"
#include "IRcommon.h"
//
// IR interface
//
extern bool g_samsungIsPowerTriggerred;

extern volatile uint32_t gStateTimer;
bool gbIR_IsRxbusy = false;
ComClass_t gIRInterface;
int16_t g_i16RawIRBits[IRINTERFACE_DATASET_MAX_IDX];
bool g_bIRCompletedFlag = false;
bool g_bIRCompletedTrigger = false;

uint32_t gu32IRTxTrigger = 0;

uint32_t IR_tx_wdt_timeout = 0;
bool IR_tx_wdt_trigger = false;

uint32_t ir_lock = 0;


void start_timer(void);
void stop_timer(void);

void adc_init(void);
void init_timer(void);

/* User Interface here */
void IRInterface_EncodeBLEToIR(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) {
	if(gIRInterface.IR_EncodeBLEToIR != NULL) {
		gIRInterface.IR_EncodeBLEToIR(InputBleCommands, OutputIRProtocol);
	}
}

void IRInterface_Decode(int16_t* input, uint8_t* output) {
//	for(uint32_t i = 0; i < IRINTERFACE_DATASET_MAX_IDX ; i++) {
//		NRF_LOG_INFO("%d: %d",i , input[i]);
//	}
	
	if(gIRInterface.IR_Decode != NULL) {
		gIRInterface.IR_Decode(input, output);
	}
	
	if(gIRInterface.IR_Decode_Secondary != NULL) {
		gIRInterface.IR_Decode_Secondary(input, NULL);
	}
	
}

uint32_t IRInterface_GetDataSetRecvMaxIdx() {
	return gIRInterface.u32DataSetRecvMaxIdx;
}

uint32_t IRInterface_GetDataSetTransmitMaxIdx() {

  // Temporary Workaround for Samsung, should be enhanced.
  if(gIRInterface.u32Type == AC_SAMSUNG){
    if(g_samsungIsPowerTriggerred)
      return SAMSUNG_EXT_BITS;
    else
      return SAMSUNG_BITS;
  }

	return gIRInterface.u32DataSetTransmitMaxIdx;
}

int IRInterface_IsEnableIrTx(void)
{
	return gIRInterface.IsEnableIrTx();
}

void IRInterface_SetIrTxState(int state)
{
	NRF_LOG_INFO("SetIrTxState %d", state);
	gIRInterface.SetIrTxState(state);
}

/* Convert raw IR data to fit with nrf52 driver */
void IRInterface_PrepareDataToSend(int16_t* input)
{
	for(uint16_t i = 0; i < IRINTERFACE_DATASET_MAX_IDX; i++)
	{
		if (g_i16RawIRBits[i] >= 0)
		{
			g_i16RawIRBits[i] = g_i16RawIRBits[i];
		}
		else
		{
			g_i16RawIRBits[i] = -g_i16RawIRBits[i];
		}
	}
}
void saadc_sampling_event_enable(void);
void IRInterface_TransmitCompleted(void)
{
	IRInterface_SetIrTxState(0);
//	ir_lib_deinit();
	g_bIRCompletedFlag = true;
	g_bIRCompletedTrigger = true;
//	start_timer();
	app_led_animation_trigger(100, LED_ANIMATION_TRIGGER_GREEN);
	ir_lock = 0;
}


void IRInterface_TransmitIR(int16_t* data, uint32_t len)
{
	/* Port driver code here */
	NRF_LOG_INFO("IRInterface_TransmitIR............");
	if((data[0] == 0) && (data[1] == 0))
	{
		NRF_LOG_INFO("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHECK BUFFER FAIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return;
	}
	
	IR_TX_WDT_Trigger();
	ir_lib_init(IR_LED_PIN, IRInterface_TransmitCompleted);
	ir_lib_send((uint16_t*)data, len);
}

bool IRInterface_IsTransmitBusy()
{
	return ir_lock;
}


void IR_TX_WDT_Tick(void)
{
	if(IR_tx_wdt_timeout > 0)
	{
		IR_tx_wdt_timeout--;
	}
}

void IR_TX_WDT_Trigger(void)
{
	NRF_LOG_INFO("IR_TX_WDT_Trigger........................");
	IR_tx_wdt_trigger = true;
	IR_tx_wdt_timeout = IR_TX_WDT_TIMEOUT;
}

void IR_TX_WDT_Task(void)
{
	if(IR_tx_wdt_trigger && IR_tx_wdt_timeout == 0)
	{
		
		NRF_LOG_INFO("IR_TX_WDT_Task");
		IR_tx_wdt_trigger = false;
		
		//
		IRInterface_SetIrTxState(0);
		g_bIRCompletedFlag = true;
		g_bIRCompletedTrigger = true;
		ir_lib_deinit();
		ir_lock = 0;
	}
}


