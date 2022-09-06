#ifndef __DATA_TRANSMIT_FSM_H
#define __DATA_TRANSMIT_FSM_H

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

#include "app_message_queue.h"

#include "cFSM.h"

#define DATA_TRANSMIT_FSM_PAYLOAD_LEN			PROTOCOL_PAYLOAD_LEN


typedef enum data_transmit_state_codes { 
	data_transmit_fsm_standby, 
	data_transmit_fsm_start, 
	data_transmit_fsm_transmit, 
	data_transmit_fsm_wait, 
	data_transmit_fsm_exit
}data_transmit_state_codes_e;


static int standby_state(void);
static int start_state(void);
static int transmit_state(void);
static int wait_state(void);
static int exit_state(void);

typedef struct data_transmit_transition {
    data_transmit_state_codes_e src_state;
    enum ret_codes   ret_code;
    data_transmit_state_codes_e dst_state;
}data_transmit_transition_t;



extern uint8_t data_transmit_fsm_payload[DATA_TRANSMIT_FSM_PAYLOAD_LEN];

int data_transmit_lookup_transitions(data_transmit_state_codes_e cur_state,enum ret_codes rc, data_transmit_transition_t* state_trans, uint8_t transition_len);
int data_transmit_fsm_main_loop();
void data_transmit_fsm_init();
void data_transmit_tick_loop(void);
void notify_data_transmit_fsm(uint8_t value);
void transmit_fsm_prepare_payload(uint8_t* data, uint8_t cmd, uint8_t type, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F0xx_HAL_H */