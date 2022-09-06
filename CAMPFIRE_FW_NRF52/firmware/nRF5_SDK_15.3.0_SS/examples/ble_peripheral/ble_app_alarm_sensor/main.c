
#define NRF_LOG_MODULE_NAME app
#include "app_ble.h"
#include "app_mesh.h"
#include "app_sensor.h"
#include "sensor_utils.h"
#include "btn.h"
#include "smart_sensor_srv.h"
#include "nrf_delay.h"
#include "app_settings.h"
#include "lib/sys_time.h"

#include "nrf_sdm.h"
#include "adc_task.h"

#include "ir_lib.h"
#include "IR_Interface.h"
#include "IR_DataConverter-Hitachi.h"
#include "IR_Common.h"

#include "user_ringbuf.h"
#include "cFSM.h"
#include "ir_learning_fsm.h"
#include "app_ac_status.h"
#include "data_transmit_fsm.h"
#include "app_alarm.h"
#include "app_common.h"
#include "ac_timer_task.h"
#include "ac_type_detection_task.h"
#include "app_indicator_led.h"
#include "nrf_temp.h"
#include "shtc3.h"
#include "smarthome_cfg.h"
#include "smarthome_meter.h"

#include "app_datalog.h"
#include "meterlib.h"
#include "app_error_code.h"
#include "nrf_power.h"

#include "app_led_animation.h"
#include "dataLogger_Config.h"
#include "dataLogger_Interface.h"

/**************************************************************************************************
DEFINE MACRO
**************************************************************************************************/

#define IRQ_ENABLED                     0x01                        /**< Field that identifies if an interrupt is enabled. */
#define MAX_NUMBER_INTERRUPTS           32                          /**< Maximum number of interrupts available. */
#define BOOTLOADER_DFU_START            0xB1

#define IR_RECV_TIMER						NRF_TIMER2
#define IR_RECV_IRQHandler			TIMER2_IRQHandler
#define IR_Timer_IRQn						TIMER2_IRQn

#define IR_RECV_PIN_NUMBER		 	15
#define GPIOTE_CHANNEL					1
#define PPI_CHANNEL							6

/**@brief 
 */
/* If g_bTestMode is true, dont need to use button to configure the device */
bool g_bTestMode = true;
ble_gap_addr_t _broadcase_addr;
bool gb_is_gateway_alive = false;
uint32_t msg_from_fw_tick = 61;
ble_gap_addr_t	addr;
uint32_t fastRtcTimeCount = 0;
extern uint32_t sht_temperature;
extern uint8_t sht_humidity;
uint32_t cpu_temp_task_cnt = 0;
int32_t cpu_temp;

uint32_t irq_cnt = 0;
volatile uint32_t ir_timeout = 0;
uint8_t flagReadSHTC3Event = 0;



/**@brief 
 */

extern uint8_t g_u8MeshPackRecv;
extern uint32_t gu32MeshTimeout;

extern volatile uint32_t gStateTimer;
extern uint8_t g_u8BleConnected;

extern uint8_t waitingForNewCustomerKey;
extern ble_gap_addr_t _device_addr;
extern uint32_t u32Tick;

extern SER_BUF_T adc_ring_buf;

extern tSMARTHOME_DATA mlib;
extern tSMARTHOME_RESULT result;
extern tSMARTHOME_RESULT meter_result;
extern double            umax;         /*!< maximal voltage measurement range      */
extern double            imax; 
extern uint32_t wh_cnt;

extern bool app_meter_trigger_saving_energy;
extern bool trigger_saving_rtc;

extern uint8_t datalog_enabled;
extern uint16_t datalog_number;

extern uint32_t ir_lock;
/**@brief 
 */

void IR_Task();
void IR_Init(void);

void Timer_Inc_Tick();
void IR_Loop_Recv(void);
void HAL_Delay(uint32_t Delay);

void Transmit_IR(void);
void IR_PinSet();
void HAL_Delay_us(uint32_t Delay);
void main_loop(void);
void adc_init(void);
void save_rtc_val(void);
void HardFault_Handler(void);
void datalog_clear_history_sensor();
void Task_1s();
uint32_t sysTick = 0;

void SysTick_Handler(void)  {                               /* SysTick interrupt Handler. */
  sysTick++;                                                /* See startup file startup_LPC17xx.s for SysTick vector */ 
	
	
}

#if(1)
void start_timer_adc(void)
{		  
	NVIC_EnableIRQ(TIMER1_IRQn);
	NRF_TIMER1->TASKS_START = 1;               // Start TIMER2
}

void stop_timer_adc(void)
{
	NVIC_DisableIRQ(TIMER1_IRQn);
	NRF_TIMER1->TASKS_START = 0;  
}
void init_timer_adc(void)
{
	NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;  // Set the timer in Counter Mode
  NRF_TIMER1->TASKS_CLEAR = 1;               // clear the task first to be usable for later
	NRF_TIMER1->PRESCALER = 4;                             //Set prescaler. Higher number gives slower timer. Prescaler = 0 gives 16MHz timer
	NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_16Bit;		 //Set counter to 16 bit resolution
	NRF_TIMER1->CC[0] = 50; //10 / 50;                             //Set value for TIMER2 compare register 0
	NRF_TIMER1->CC[1] = 1050; //20 / 100;                                 //Set value for TIMER2 compare register 1
  // Enable interrupt on Timer 2, both for CC[0] and CC[1] compare match events
	NRF_TIMER1->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos) | (TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos);
	NVIC_SetPriority(TIMER1_IRQn,6);
	start_timer_adc();
}
volatile uint32_t ms_counter = 0;
volatile uint32_t mainloop_start = 0;
volatile uint32_t mainloop_exit = 0;

volatile uint32_t mainloop_min = 1000000;
volatile uint32_t mainloop_max = 0;
/** TIMTER1 peripheral interrupt handler. This interrupt handler is called whenever there it a TIMER2 interrupt
 */
volatile uint32_t g_timer_counter_1ms = 0;
void TIMER1_IRQHandler(void)
{
	if ((NRF_TIMER1->EVENTS_COMPARE[0] != 0) && ((NRF_TIMER1->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0))
  {
		if(g_timer_counter_1ms >= 1000)
		{
			g_timer_counter_1ms = 0;
			Task_1s();
		}
		g_timer_counter_1ms++;
		ms_counter++;
		NRF_TIMER1->EVENTS_COMPARE[0] = 0;           //Clear compare register 0 event	
		adc_task();
		ir_timeout += 1000;
		app_led_animation_tick();
		datalog_tick_ms_run();
		app_blink_tick();
		app_led_task();
		app_led_animation_task();
  }
	if ((NRF_TIMER1->EVENTS_COMPARE[1] != 0) && ((NRF_TIMER1->INTENSET & TIMER_INTENSET_COMPARE1_Msk) != 0))
  {
		NRF_TIMER1->EVENTS_COMPARE[1] = 0;           //Clear compare register 1 event
		NRF_TIMER1->TASKS_CLEAR = 1;
  }
}
#endif 


#if(1)
void start_timer(void)
{		  
	NVIC_EnableIRQ(IR_Timer_IRQn);
	IR_RECV_TIMER->TASKS_START = 1;               // Start TIMER2
}

void stop_timer(void)
{
	NVIC_DisableIRQ(IR_Timer_IRQn);
	IR_RECV_TIMER->TASKS_START = 0;  
}


void GPIOTE_IRQHandler(void)
{
  if (NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL] == 1)
  {
    NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL] = 0;
		
		if(!IRInterface_IsTransmitBusy())
		{
			if(dataSetIndex < 640)
			{
				if(nrf_gpio_pin_read(IR_RECV_PIN) == 1)
				{
					g_i16RawIRBits[dataSetIndex++] = IR_RECV_TIMER->CC[0];
				}
				else
				{
					g_i16RawIRBits[dataSetIndex++] = -(IR_RECV_TIMER->CC[0]);
				}
			}
		}
		ir_timeout = 0;
		IR_RECV_TIMER->TASKS_STOP = 1;   
		IR_RECV_TIMER->TASKS_CLEAR = 1;
		IR_RECV_TIMER->TASKS_START = 1;
  }
}

void init_timer(void)
{
	NRF_P0->PIN_CNF[IR_RECV_PIN_NUMBER] = GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos |
                                      GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos |
                                      GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos |
                                      GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos;

  NRF_PPI->CHEN |= 1 << PPI_CHANNEL;
  NRF_PPI->CH[PPI_CHANNEL].EEP = (uint32_t)&NRF_GPIOTE->EVENTS_IN[GPIOTE_CHANNEL];
  NRF_PPI->CH[PPI_CHANNEL].TEP = (uint32_t)&IR_RECV_TIMER->TASKS_CAPTURE[0];

  NRF_GPIOTE->CONFIG[GPIOTE_CHANNEL] = GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos |
                                    IR_RECV_PIN_NUMBER << GPIOTE_CONFIG_PSEL_Pos |
                                    GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos; 
  NRF_GPIOTE->INTENSET = (1 << GPIOTE_CHANNEL);
  NVIC_EnableIRQ(GPIOTE_IRQn);
	NVIC_SetPriority(GPIOTE_IRQn,0);
	IR_RECV_TIMER->TASKS_STOP = 1;   
  IR_RECV_TIMER->TASKS_CLEAR = 1;
  IR_RECV_TIMER->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
  IR_RECV_TIMER->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
  IR_RECV_TIMER->PRESCALER = 4;
  IR_RECV_TIMER->TASKS_START = 1;
}
#endif 


void wdt_init(void)
{
	NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
	NRF_WDT->CRV = 10*32768; // 20 sec. timout
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk; //Enable reload register 0
	NRF_WDT->TASKS_START = 1;
}

void wdt_feed(void)
{
//	NRF_LOG_INFO("wdt_feed");
	NRF_WDT->RR[0] = WDT_RR_RR_Reload; //Reload watchdog register 0
}

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}
/**@brief Function for disabling all interrupts before jumping from bootloader to application.
 */
static void interrupts_disable(void)
{
    uint32_t interrupt_setting_mask;
    uint32_t irq;

    // Fetch the current interrupt settings.
    interrupt_setting_mask = NVIC->ISER[0];
    // Loop from interrupt 0 for disabling of all interrupts.
    for (irq = 0; irq < MAX_NUMBER_INTERRUPTS; irq++)
    {
        if (interrupt_setting_mask & (IRQ_ENABLED << irq))
        {
            // The interrupt was enabled, hence disable it.
            NVIC_DisableIRQ((IRQn_Type)irq);
        }
    }
}


volatile bool gb_send_first_msg = false;
uint32_t app_send_first_msg_cnt = 0;
uint8_t flagTimestampRequestEvent = 1;
uint8_t flagAppSensorTaskEvent = 0;

uint8_t flagAppMeterTaskEvent = 0;

uint8_t flagErrorCodeTaskEvent = 0;

void app_emeter_task(void);
void app_rtc_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
}

void Task_1s()
{
		
	fastRtcTimeCount++;
	rtcTimeSec++;

//		if(rtcTimeSec < 1631356449)
//			rtcTimeSec = 1631356449;
		TIME_FromSec(&sysTime,rtcTimeSec);
		NRF_LOG_INFO("sysTime: %d:%d:%d - %d", sysTime.hour, sysTime.min, sysTime.sec, sysTime.wday);
		if((rtcTimeSec % 8) == 0)
		{
			createOtpFlag = 1;
		}
		if((rtcTimeSec % 5) == 0)
		{
			flagReadSHTC3Event = 1;
		}
		#if(1)	
			/* */
		
		if((rtcTimeSec % 300) == 7 /* && (u8FwType != FW_APP_TYPE_IR_LEARNING) */)
		{
			/* */
			flagAppSensorTaskEvent = 1;
			/* */
		}
		if((rtcTimeSec % 60) == 13 /* && (u8FwType != FW_APP_TYPE_IR_LEARNING)*/)
		{
			/* */
			flagAppMeterTaskEvent = 1;
			/* */
		}		
		if(((rtcTimeSec % 60) == 23) /* && (u8FwType != FW_APP_TYPE_IR_LEARNING) */ && (is_app_alive == true))
		{
			flagErrorCodeTaskEvent = 1;
		}
		if((rtcTimeSec % 60) == 17 /*&& (u8FwType != FW_APP_TYPE_IR_LEARNING) */ && rtcUpdateFlag == 0 && is_app_alive == true)
		{
			/* */
			flagTimestampRequestEvent = 1;
			/* */
		}
		#endif
		/* */
		app_tick_task();
		if(g_u8MeshPackRecv == 1)
		{
			gu32MeshTimeout++;
		}
		if(gu32MeshTimeout > 1)
		{
			gu32MeshTimeout = 0;
			g_u8MeshPackRecv = 0;
		}
		if(gu32IRCmdTimeout > 0) {
			gu32IRCmdTimeout--;
		}

		#if(1)
		ir_learning_fsm_main_loop();
		data_transmit_tick_loop();
		app_alarm_task();
		ac_timer_task();
		ac_type_detection_task();
		#endif
		
		msg_from_fw_tick++;
		
		if(rtcTimeSec % 30 == 7)
		{
			app_meter_trigger_saving_energy = true;
		}
		if(rtcTimeSec % 30 == 3)
		{
			trigger_saving_rtc = true;
			mainloop_max = 0;
			mainloop_min = 1000000;
		}
		CFG_Saving_Tick();
		IR_TX_WDT_Tick();
		datalog_tick_s_run();
		NRF_LOG_INFO("mainloop_max: %d, mainloop_min: %d", mainloop_max, mainloop_min);
		
}
/**@brief Function for preparing the reset, disabling SoftDevice, and jumping to the bootloader.
 *
 */

void nrf_temperature_task(void)
{
//	cpu_temp_task_cnt++;
//	if(cpu_temp_task_cnt < 50000)
//		return;
//	cpu_temp_task_cnt = 0;
//	int32_t temp;
//	sd_temp_get(&temp);
//	temp = temp / 4;
//	cpu_temp = temp;
//	NRF_LOG_INFO("CPU temperature: %d", (int)cpu_temp);
}

uint32_t bootloader_start(void)
{
	uint32_t err_code;
	err_code = sd_power_gpregret_clr(0, 0xffffffff);
	VERIFY_SUCCESS(err_code);
	err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
	VERIFY_SUCCESS(err_code);
	err_code = sd_softdevice_disable();
	VERIFY_SUCCESS(err_code);
	err_code = sd_softdevice_vector_table_base_set(NRF_UICR->NRFFW[0]);
	VERIFY_SUCCESS(err_code);
	NVIC_ClearPendingIRQ(SWI2_IRQn);
	interrupts_disable();
	NVIC_SystemReset();
	return NRF_SUCCESS;
}

void datalog_clear();

/**@brief Application main function.
 */
int main(void)
{
		uint32_t u32Reset_reason = NRF_POWER->RESETREAS;
		nrf_power_resetreas_clear(0xFFFFFFFF);
		/* Initialize IR_LED as soon as possible */
		nrf_gpio_cfg_output(IR_LED_PIN);;
		nrf_gpio_pin_clear(IR_LED_PIN);

	//Init peripheral
		wdt_init();
		// Initialize.
		boards_init();
		log_init();
		datalog_init();
		
		nrf_delay_ms(10);
		app_meter_init();
		nrf_delay_ms(100);
	
		nrf_temp_init();
		adc_init();
		init_timer_adc();
		nrf_gpio_cfg_input(10, NRF_GPIO_PIN_PULLUP);
		if(u32Reset_reason > 0)
		{
			gu32_reset_reason = u32Reset_reason;
		}
		else
		{
			gu32_reset_reason = 0xFF;
		}
		error_code_init();
		
		nrf_gpio_cfg_output(LED_R);
		nrf_gpio_cfg_output(LED_B);
		nrf_gpio_cfg_output(LED_G);
	
		NRF_LOG_INFO("CampFire App %s", VERSION);
		
    uint32_t err_code;
    bool erase_bonds;
		if(findNeighborFlag != FOUND_NEIGHBOR)
		{
			memset(&neighbor_addr,0,sizeof(ble_gap_addr_t));
			sensorMode = SENSOR_FIND_NEIGHBOR_MODE;
		}
		else
		{
			sensorMode = SENSOR_FOUND_NEIGHBOR_MODE;
		}
		nrf_delay_ms(100);
		#ifdef USE_BLE
		ble_app_init();
		#endif
		nrf_delay_ms(100);
		NRF_LOG_INFO("ble_app_init done");
		sensor_init();
		app_sensor_init();
		btn_init();
		
		CFG_Init();
		CFG_Load(false);
		
		datalog_number = dataLogger_GetNumberSaved(SENSOR_RECORD_LOG);
		datalog_enabled = sysCfg.enable_sensor_datalog;
		nrf_delay_ms(500);
		NRF_LOG_INFO("OLD DB version: %d", sysCfg.database_version);
		NRF_LOG_INFO("NEW DB version: %d", DB_VERSION);
		if (sysCfg.database_version != DB_VERSION)
		{
			nrf_delay_ms(500);
			datalog_clear();
			nrf_delay_ms(500);
			sysCfg.database_version = DB_VERSION;
			CFG_Save();
			CFG_Saving_Trigger();
		}
		else
		{
			NRF_LOG_INFO("Same DB version");
		}
		if(sysCfg.ota_flag == 1)
		{
			sysCfg.ota_flag = 0;
			CFG_Saving_Trigger();
			nrf_delay_ms(500);
			datalog_clear_history_sensor();
			nrf_delay_ms(500);
			app_led_animation_trigger(500, LED_ANIMATION_TRIGGER_GREEN);
		}
		
		app_ac_init(sysCfg.ac_payload);
		
		ac_timer_task_init();		
		app_alarm_init();
		
		IR_Init();
		init_timer();
		
		#if(1)
		application_timers_start();
		app_set_mode(MESH_MODE);
		#if(1)
		mesh_app_init();
		#endif
		#ifdef USE_BLE
		advertising_start(false);
		#endif
		
		memset(_broadcase_addr.addr,0x99,BLE_GAP_ADDR_LEN);
		default_crc = AppMakeOtp(sysCfg.deviceKey,_broadcase_addr.addr,0x12345678);
		#endif
		
		if(shtc3_init() == false)
		{
			NRF_LOG_INFO("Init sensor FAILED.........");
		}
		else 
		{
			NRF_LOG_INFO("Init sensor SUCCESS.........");
		}
		
		shtc3_loop();
		uint8_t mac[6] = {0};
		
		sd_ble_gap_addr_get(&addr);
		memcpy(mac,addr.addr,6);
 		NRF_LOG_INFO("MAC: %02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		
//		app_led_animation_trigger(50, LED_ANIMATION_TRIGGER_GREEN);
//		app_led_blink_trigger(LED_ANIMATION_TRIGGER_BLUE);
		
		NRF_LOG_INFO("Device Type %d", sysCfg.IR_devicetype);
		// Enter main loop.
		
		for (;;)
		{
			mainloop_start = ms_counter;
#if(1)			
			//Main Loop
			main_loop();
			mainloop_exit = ms_counter - mainloop_start;
			if(mainloop_min > mainloop_exit) {
				mainloop_min = mainloop_exit;
			}
			if(mainloop_max < mainloop_exit) {
				mainloop_max = mainloop_exit;
			}
			//
#endif
		}
}

void main_loop(void)
{
	wdt_feed();
	if(ir_timeout >= 100000 && dataSetIndex > 0)
	{
		if(dataSetIndex > 50)
		{
			if(ir_lock == 0)
			{
				IR_Task();
			}
		}

		dataSetIndex = 0;
		ir_timeout = 0;
	}
		NRF_LOG_PROCESS();

		if(createOtpFlag)
		{
			createOtpFlag = 0;
			my_otp_crc = AppMakeOtp(sysCfg.deviceKey, _device_addr.addr ,rtcTimeSec>>4);
			broadcast_crc = AppMakeOtp(sysCfg.deviceKey,_broadcase_addr.addr,rtcTimeSec>>4);
		}
	#if(1)
		if((button_pressed_flag) /*&& (nrf_gpio_pin_read(BUTTON_1) == 0) */)
		{
			if(button_click_times == 1 && button_press_time <= 1)
			{
				NRF_LOG_INFO("****Reset button mode*");
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(200);
				app_indicator_led_off(RGB_TYPE_R);
				#if(1)
				if(waitingForNewCustomerKey)
				{
					waitingForNewCustomerKey = 0;
				}
				#endif				
			}
			else 
				if(button_click_times == 3)
			{
				NRF_LOG_INFO("****WAITING FOR NEW CUSTOMER KEY****");
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_off(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_off(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_off(RGB_TYPE_R);

				waitingForNewCustomerKey = 1;			
			}
			else if(button_click_times == 5)
			{
				NRF_LOG_INFO("*********** BOOTLOADER **********");
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_off(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_off(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_off(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_off(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_on(RGB_TYPE_R);
				nrf_delay_ms(100);
				app_indicator_led_off(RGB_TYPE_R);
				bootloader_start();
			}
			else if(button_click_times == 10 || (button_press_time >= 10 && button_click_times == 1))
			{
				NRF_LOG_INFO("******* FORMAT FLASH *******");					
				//LED_SYS_OFF;			
				CFG_Load(true);
			}
			button_pressed_flag = 0;
		}
		
	app_mesh_task();

	#ifdef USE_BLE
	ble_task(); 
	AppSettingsTask();
	#endif
	
	/* IR */
	if( /* gu32IRCmdTimeout == 0 && */ gu32IRTxTrigger == 1 /*&& !IRInterface_IsTransmitBusy() */ && ir_lock == 0)
	{
		ir_lock = 1;
			IRInterface_TransmitIR(g_i16RawIRBits, IRInterface_GetDataSetTransmitMaxIdx() + 2);
			gu32IRTxTrigger = 0;
	}
	
	if(flagTimestampRequestEvent)
	{
		flagTimestampRequestEvent = 0;
		uint8_t data[10];
		memset(data, 0xAA, 10);
		transmit_fsm_prepare_payload(data, 0x12, 0x04, 10);
		
	}
	if(flagAppSensorTaskEvent)
	{
		flagAppSensorTaskEvent = 0;
		app_sensor_task();
	}
	if(flagAppMeterTaskEvent)
	{
			flagAppMeterTaskEvent = 0;
			app_emeter_task();
	}
	if(flagErrorCodeTaskEvent)
	{
			flagErrorCodeTaskEvent = 0;
			error_code_task();
	}
	if(flagReadSHTC3Event == 1)
	{
			flagReadSHTC3Event = 0;
			shtc3_loop();
	}
	data_transmit_fsm_main_loop();
	/* IR */
	
	app_led_blink_task();
	
	
	app_datalog_task();
	app_save_datalog();
	save_energy_val();
	save_rtc_val();
	CFG_Saving_Task();
	IR_TX_WDT_Task();
#endif
}

void HardFault_Handler(void)
{
	__disable_irq(); //added by haidv#date:12/03/2016
	NVIC_SystemReset();
	while(1);
}

void HAL_Delay_us(uint32_t Delay)
{
	Delay = Delay*8;
	for(uint32_t i = 0; i < Delay;i++);
}

bool getTestMode()
{
	return g_bTestMode;
}


/**
 * @}
 */
