#ifndef __SOS_SW_UTILS_H__
#define __SOS_SW_UTILS_H__
#include <stdint.h>
#include <stdbool.h>
/* Module APIs */
void sos_sw_init(void);

void sos_sw_send_presentation(void);

void sos_sw_send_status(uint8_t sequence);

void _send_sos_sw_info(uint8_t sequence);

void _set_sos_status(bool status);

uint32_t sos_get_cnt(void);

bool _get_sos_status(void);

void _clear_sos_status(void);
#endif
