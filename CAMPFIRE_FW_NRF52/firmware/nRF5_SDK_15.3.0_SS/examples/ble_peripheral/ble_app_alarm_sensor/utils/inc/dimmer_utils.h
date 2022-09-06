#ifndef __DIMMER_UTILS_H__
#define __DIMMER_UTILS_H__

#include <stdint.h>

#define DIMMER_ID	0x04

/* Module APIs */
void dimmer_init(void);

void dimmer_update_value(uint8_t percent);

void dimmer_send_status(void);

void dimmer_trigger_to_send_status(void);

#endif
