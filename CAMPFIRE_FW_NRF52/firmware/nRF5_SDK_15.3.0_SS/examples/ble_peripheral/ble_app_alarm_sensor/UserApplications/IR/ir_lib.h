#ifndef __IR_LIB_H
#define __IR_LIB_H

#include <stdint.h>
#include <stdbool.h>

#define IR_TIMER_CARRIER                NRF_TIMER3

#define IR_CARRIER_COUNTER              NRF_TIMER4
#define IR_CARRIER_COUNTER_IRQn         TIMER4_IRQn
#define IR_CARRIER_COUNTER_IRQHandler   TIMER4_IRQHandler
#define IR_CARRIER_COUNTER_IRQ_Priority 2

#define IR_PPI_CH_A         0
#define IR_PPI_CH_B         1
#define IR_PPI_CH_C         2
#define IR_PPI_CH_D         3
#define IR_PPI_CH_E         4

#define IR_PPI_GROUP        1

//#define IR_CARRIER_LOW_US   13
//#define IR_CARRIER_HIGH_US  13
#define IR_CARRIER_LOW_US   210
#define IR_CARRIER_HIGH_US  210
uint32_t ir_lib_init(uint32_t ir_pin, void* cb);

void ir_lib_send(uint16_t *time_us, uint32_t length);
bool ir_lib_is_transmit_busy();
void ir_lib_clear_busy_flag(void);
uint32_t ir_lib_deinit(void);
#endif
