#include <stdio.h>
#include "cFSM.h"
#include "nrf_log.h"
#include "app_ble.h"
#include "ir_learning_fsm.h"
#include "crc32.h"
static int standby_state(void);
static int start_state(void);
static int transmit_data_state(void);
static int ir_exit_state(void);

uint32_t ir_data_byte_len = IR_DATA_TOTAL_LEN*4;
int16_t i16_ir_raw_data[IR_DATA_TOTAL_LEN];
uint8_t ir_transport_data[IR_DATA_TOTAL_LEN*4];

uint8_t ir_start_frame[14];
uint8_t ir_frame_cnt = 0;
uint8_t ir_frame_idx = 0;
uint32_t ir_byte_transmited = 0;
uint32_t ir_learning_tick = 0;

uint32_t remain_len = 0;

/* transitions from end state aren't needed */
ir_learning_transition_t ir_learning_state_transitions[] = {
    {ir_standby, ok,     ir_start},
    {ir_standby, repeat,   ir_standby},
    {ir_start,   ok,     ir_transmit},
    {ir_start,   fail,   ir_exit},
    {ir_start,   repeat, ir_start},
    {ir_transmit,   ok,     ir_exit},
    {ir_transmit,   fail,   ir_exit},
    {ir_transmit,   repeat, ir_transmit},
		{ir_exit,   ok, ir_standby},
		{ir_exit,   fail, ir_standby}
};

/* array and enum below must be in sync! */
static int (* state[])(void) = { standby_state, start_state, transmit_data_state, ir_exit_state};

static enum ir_state_codes ir_cur_state = 0;
static enum ret_codes rc = 2;
static int (* state_fun)(void);


static int standby_state(void) {
//    NRF_LOG_INFO("standby_state\r\n");
		if(rc == ok)
			return ok;
    return repeat;
}

static int start_state() {
	NRF_LOG_INFO("start_state");

	uint8_t ble_transmit_status = get_ble_transmit_status();
	ir_learning_tick_loop();
	return ok;
//	if(ble_transmit_status == 0) //standby
//	{
//		set_ble_transmit_status(3);
//		if(ble_check_status() != NRF_ERROR_BUSY)
//		{
//			ble_send_data(ir_start_frame, 14);
//		}
//		return repeat;
//	}
//	else if (ble_transmit_status == 1) //ok
//	{
//		set_ble_transmit_status(0);
//		return ok;
//	}
//	else if (ble_transmit_status == 2)//fail
//	{
//		set_ble_transmit_status(0);
//		return fail;
//	} 
//	else if (ble_transmit_status == 3)//busy
//	{
//		if(get_ir_learning_tick() > 10)
//		{
//			set_ble_transmit_status(0);
//			ir_learning_tick_reset();
//			return ok;
//		}
//		else 
//		{
//			return repeat;
//		}
//	} 
}
static int transmit_data_state(void) {
	remain_len = ir_data_byte_len - ir_byte_transmited;
	uint8_t ble_transmit_status = get_ble_transmit_status();
	NRF_LOG_INFO("transmit_data_state = %d, remain_len = %d", ble_transmit_status, remain_len);

	ir_learning_tick_loop();

	if(ble_transmit_status == 3)
	{
		if(get_ir_learning_tick() > 10)
		{
			set_ble_transmit_status(0);
			ir_learning_tick_reset();
			return fail;
		}
		else 
		{
			return repeat;
		}
	}
	else if(ble_transmit_status == 2) //fail
	{
		set_ble_transmit_status(0);
		return fail;
	}
	else if(ble_transmit_status == 1)
	{
		ir_learning_tick_reset();
		set_ble_transmit_status(0);
		return repeat;
	}
	else if(ble_transmit_status == 0)
	{
		if(remain_len > 0)
		{
			if(remain_len < IR_FRAME_LEN)
			{
				NRF_LOG_INFO("Send last frame from idx: %d with len: %d",ir_byte_transmited, remain_len);
				uint8_t data[remain_len];
				for(uint32_t i = 0; i < remain_len; i++)
				{
					data[i] = ir_transport_data[i + ir_byte_transmited];
				}
				set_ble_transmit_status(3);
				if(ble_check_status() != NRF_ERROR_BUSY)
				{		
					ble_send_data(data, remain_len);
				}
				ir_byte_transmited += remain_len;
			}
			else
			{
				NRF_LOG_INFO("Send normal frame, from idx: %d with len: %d",ir_byte_transmited, IR_FRAME_LEN);
				uint8_t data[IR_FRAME_LEN];
				for(uint32_t i = 0; i < IR_FRAME_LEN; i++)
				{
					data[i] = ir_transport_data[i + ir_byte_transmited];
				}
				set_ble_transmit_status(3);
				if(ble_check_status() != NRF_ERROR_BUSY)
				{		
					ble_send_data(data, (uint8_t)IR_FRAME_LEN);
				}
				ir_byte_transmited += IR_FRAME_LEN;
			}
			
			return repeat;
		}
		else
		{
			return ok;
		}
	}

}
static int ir_exit_state(void) {
		ir_learning_tick_reset();
		ir_byte_transmited = 0;
    NRF_LOG_INFO("exit_state...");
    return ok;
}


void ir_learning_fsm_init(int16_t* data, uint16_t len)
{
	ir_byte_transmited = 0;
	remain_len = len*4;
	ir_data_byte_len = len*4;
	
	for(uint16_t i = 0; i < len; i++)
	{
		i16_ir_raw_data[i] = data[i];
	}
	for(uint16_t i = 0; i < len; i++)
	{
		ir_transport_data[4*i] = i16_ir_raw_data[i] & 0xFF;
		ir_transport_data[4*i + 1] = (i16_ir_raw_data[i] >> 8) & 0xFF;
		ir_transport_data[4*i + 2] = 0;
		ir_transport_data[4*i + 3] = 0;
	}
	
	//
	uint32_t crc32 = crc32_compute(ir_transport_data, ir_data_byte_len, NULL);
	NRF_LOG_INFO("ir_byte_length: %d", ir_data_byte_len);
	NRF_LOG_INFO("CRC32: %d", crc32);
	ir_start_frame[0] = 0;
	ir_start_frame[1] = 0;
	ir_start_frame[2] = 0;
	ir_start_frame[3] = 0;
	ir_start_frame[4] = (ir_data_byte_len) & 0xFF;
	ir_start_frame[5] = (ir_data_byte_len >> 8) & 0xFF;
	ir_start_frame[6] = (crc32) & 0xFF; //CRC cua ca day data
	ir_start_frame[7] = (crc32 >> 8) & 0xFF;
	ir_start_frame[8] = (crc32 >> 16) & 0xFF;
	ir_start_frame[9] = (crc32 >> 24) & 0xFF;
	
	crc32 = crc32_compute(ir_start_frame, 10, NULL);
	ir_start_frame[10] = (crc32) & 0xFF; //CRC cua start frame
	ir_start_frame[11] = (crc32 >> 8) & 0xFF;
	ir_start_frame[12] = (crc32 >> 16) & 0xFF;
	ir_start_frame[13] = (crc32 >> 24) & 0xFF;
	
	
	if(ir_data_byte_len % IR_FRAME_LEN == 0)
	{
		ir_frame_cnt = ir_data_byte_len/IR_FRAME_LEN;
	}
	else
	{
		ir_frame_cnt = ir_data_byte_len/IR_FRAME_LEN + 1;
	}
	NRF_LOG_INFO("ir_frame_cnt %d\r\n", ir_frame_cnt);
	
}

int ir_learning_lookup_transitions(enum ir_state_codes cur_state,enum ret_codes rc, ir_learning_transition_t* state_trans, uint8_t transition_len) {
    enum ir_state_codes ret = 0;
    for(int i = 0; i < transition_len; i++) {
        if(ir_cur_state == state_trans[i].src_state && rc == state_trans[i].ret_code) {
            ret = state_trans[i].dst_state;
        }
    }
    return ret;
}

int ir_learning_fsm_main_loop()
{
	
	state_fun = state[ir_cur_state];
	rc = state_fun();
	if (ir_exit == ir_cur_state)
	{
		ir_learning_set_ret_code(repeat);
		ir_cur_state = ir_standby;
	}
	ir_cur_state = ir_learning_lookup_transitions(ir_cur_state, rc, ir_learning_state_transitions, sizeof(ir_learning_state_transitions)/sizeof(ir_learning_state_transitions[0]));
}

void ir_learning_tick_loop(void)
{
	ir_learning_tick++;
	NRF_LOG_INFO("ir_learning_tick %d", ir_learning_tick);
}

void ir_learning_tick_reset()
{
	ir_learning_tick = 0;
}

uint32_t get_ir_learning_tick(void)
{
	return ir_learning_tick;
}

void ir_learning_set_ret_code(enum ret_codes ret)
{
	NRF_LOG_INFO("ir_learning_set_ret_code %d", ret);
	rc = ret;
}

