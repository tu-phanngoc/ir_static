#ifndef __IR_LEARNING_FSM_H__
#define __IR_LEARNING_FSM_H__

#include <stdint.h>
#include "cFSM.h"
enum ir_state_codes { 
	ir_standby, 
	ir_start, 
	ir_transmit, 
	ir_exit
};

void ir_learning_fsm_init(int16_t* data, uint16_t len);
#define IR_FRAME_LEN					240
#define IR_DATA_TOTAL_LEN			1024

typedef struct ir_learning_transition {
    enum ir_state_codes src_state;
    enum ret_codes   ret_code;
    enum ir_state_codes dst_state;
}ir_learning_transition_t;

int ir_learning_fsm_main_loop();
void ir_learning_tick_loop(void);
void ir_learning_set_ret_code(enum ret_codes ret);
uint32_t get_ir_learning_tick(void);
void ir_learning_tick_reset();
#endif


