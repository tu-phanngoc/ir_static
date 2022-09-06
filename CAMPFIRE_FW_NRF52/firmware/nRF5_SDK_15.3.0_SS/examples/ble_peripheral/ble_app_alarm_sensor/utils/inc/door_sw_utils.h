#ifndef __DOOR_SW_UTILS_H__
#define __DOOR_SW_UTILS_H__
#include <stdbool.h>
#include <stdint.h>
/* Module APIs */
void door_sw_init(void);

void door_sw_send_presentation(void);

void door_sw_send_status(uint8_t sequence);

void _send_door_sw_info(uint8_t sequence);

void _set_door_status(bool status);

void _clear_door_status(void);

#endif
