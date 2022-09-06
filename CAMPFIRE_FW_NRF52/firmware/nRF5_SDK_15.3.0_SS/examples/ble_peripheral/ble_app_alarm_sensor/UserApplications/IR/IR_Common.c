#include "IR_Common.h"
#include "IR_HAL.h"
#include "ampm_gt999.h"
#include "app_mesh.h"
#include "nrf_log.h"
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "IR_Interface.h"
#include "IR_DataConverter.h"
#include "IR_DeviceConstructor.h"
#include "ir_lib.h"
#include "ir_learning_fsm.h"
#include "ac_type_detection_task.h"
#include "app_indicator_led.h"
#include "app_led_animation.h"


static int IrEnableTx_s;



volatile uint32_t u32StartTime = 0;
uint32_t dataSetIndex = 0;
uint32_t IR_State = 0;
volatile uint32_t gStateTimer = 0;
volatile uint32_t gInputState = 0;
uint32_t gOldState = 1;
uint32_t gu32Counter = 0;
uint32_t g_u32IRTimer = 0;
uint32_t g_u32IRIdx = 0;
uint8_t gBleData[24];

volatile uint32_t gu32IRCmdTimeout = 3;
extern volatile bool gbBleCommandRecv;
extern uint8_t u8FwType;

void HAL_Delay_us(uint32_t Delay);
void protocol_send_IR_data(uint8_t* data);

void start_timer(void);
void stop_timer(void);
void IR_ConstructDeviceType(void);
void init_timer(void);
void IR_Init(void)
{
//  IR_GPIO_INIT;
  IR_ConstructDeviceType();
}

void FlushData(void)
{
  for(uint32_t i = 0; i < IRINTERFACE_DATASET_MAX_IDX; i++)
  {
    g_i16RawIRBits[i] = 0;
  }
}

/* @brief: The function runs in main loop
*/

void IR_Task()
{
  
  if(g_bIRCompletedFlag == true) {
    NRF_LOG_INFO("ir deinit");
		
    HAL_Delay_us(2000);
    ir_lib_deinit();
    g_bIRCompletedFlag = false;
    bEnableSendingToMesh = true;
    IR_LED_PIN_INIT;
    IR_PinClear();
//		init_timer();
  }

  if(g_bIRCompletedTrigger == true) {
    g_bIRCompletedTrigger = false;
    g_bIRCompletedFlag = false;
  }

		for(uint32_t i = 0; i < 639; i++)
		{
			g_i16RawIRBits[i] = g_i16RawIRBits[i + 1];
		}
		
		dataSetIndex = dataSetIndex - 1;
		
		NRF_LOG_INFO("dataSetIndex: %d", dataSetIndex);
		
		uint8_t bleOutput[12];
		
		if(is_ac_type_detection_enabled())
		{
			uint32_t deviceType = IRInterface_DetectDeviceType(dataSetIndex - 1, g_i16RawIRBits);
			sysCfg.IR_devicetype = deviceType;
			IR_ConstructSpecificDeviceType(deviceType);
			IRInterface_Decode(g_i16RawIRBits, bleOutput);
			CFG_Save();
			ac_type_detection_set_signal(deviceType);
		}
		else if(sysCfg.IR_devicetype != 0xFF && (dataSetIndex == (IRInterface_GetDataSetRecvMaxIdx() + 1)))
		{
			IRInterface_Decode(g_i16RawIRBits, bleOutput);
		}

		FlushData();
		if(dataSetIndex == (IRInterface_GetDataSetRecvMaxIdx() + 1))
		{				
			app_led_animation_trigger(50, LED_ANIMATION_TRIGGER_BLUE);
			gu32Counter = 0;
			IR_State = 0;
			dataSetIndex = 0;
			gStateTimer = 0;
			gbIR_IsRxbusy = false;
			NRF_LOG_INFO("Decode IR correctly......................"); 
			protocol_send_IR_data(bleOutput);
		}
		else
		{
			app_led_animation_trigger(50, LED_ANIMATION_TRIGGER_RED);
			bleOutput[0] = 0xFF;
			bleOutput[1] = dataSetIndex & 0xFF;
			bleOutput[2] = (dataSetIndex >> 8) & 0xFF;
			IR_State = 0;
			dataSetIndex = 0;
			gStateTimer = 0;
			NRF_LOG_INFO("reset params");  
		}
}


void IR_Task_Learning()
{

  #if(1)
  if(IR_State == 2 && dataSetIndex > 50 && gStateTimer>50000) {
      NRF_LOG_INFO("dataSetIndex: %u",dataSetIndex);
        IR_State = 3;
    }

    if(IR_State == 3) {
      NRF_LOG_INFO("gu32Counter: %u",gu32Counter);

//      NRF_LOG_HEXDUMP_DEBUG(g_i16RawIRBits, gu32Counter);
      ir_learning_fsm_init(g_i16RawIRBits, gu32Counter);
      ir_learning_set_ret_code(ok);

      FlushData();
      gu32Counter = 0;
      IR_State = 0;
      dataSetIndex = 0;
      gStateTimer = 0;
      gbIR_IsRxbusy = false;
    }
    #endif
}


/*
*/
uint32_t dbgCnt = 0;
void IR_Loop_Recv(void)
{
//  NRF_LOG_INFO(".");
  if( ((gStateTimer > 300000 && IR_State > 0)  || (gStateTimer > 1000000)) ) {
//    DEBUG_PRINT("%u",dbgCnt);
    gu32Counter = 0;
    gStateTimer = 0;
    IR_State = 0;
    dataSetIndex = 0;
    dbgCnt = 0;
    gbBleCommandRecv = false;
  }
  if(gbBleCommandRecv == true || IRInterface_IsEnableIrTx())
      return;
    
  if(nrf_gpio_pin_read(IR_RECV_PIN) == 1) {
    gInputState = 1;
//    DEBUG_PRINT("%u",gInputState);
  }
  else {
    gInputState = 0;
//    DEBUG_PRINT("%u",gInputState);
  }
    
  //
  {      
    if(gOldState != gInputState) 
    {
      dbgCnt++;
//      NRF_LOG_INFO("%u:%u",gOldState,gStateTimer);  
      gu32Counter++;
      if(IR_State == 0 && gInputState == 0 && dataSetIndex == 0)
      {
//        DEBUG_PRINT("%u:%u ..........................",gOldState,gStateTimer);  
        gStateTimer = 0;
        IR_State = 1;
//          DEBUG_PRINT("IR_State = 1");
      }
      if((IR_State == 1) && (gInputState == 1) && (dataSetIndex == 0))
      {
        if(gStateTimer < 300) 
        {
//          IR_State = 0;
//          gu32Counter = 0;
        }
        else
        {
          gbIR_IsRxbusy = true;
          DEBUG_PRINT("REC----");
          IR_State = 2;
        }
      }
      
      {
//          if((dataSetIndex <= IRInterface_GetDataSetRecvMaxIdx()) && (IR_State == 2)) 
        if((dataSetIndex < 640) && (IR_State == 2)) 
        {                    
          /*
          Pulse: Input logic = 0, set interval to positive
          Pause: Input logic = 1, set interval to negative
          */
          uint32_t stateTimer = gStateTimer;
          if(gOldState == 0) {
            g_i16RawIRBits[dataSetIndex++] = stateTimer;
          }
          else
          {
            g_i16RawIRBits[dataSetIndex++] = -stateTimer;
          }
          gStateTimer = 1;
        }
      }
      gOldState = gInputState;
    }
  }
}



int isIrTxEnable(void){
  return IrEnableTx_s;
}

void setIrTxState(int state){
  IrEnableTx_s = state;
}



