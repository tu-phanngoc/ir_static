#ifndef __PING_PONG_H__
#define __PING_PONG_H__
#include <stdint.h>

typedef struct {
	long* ping;
	long* pong;
	uint8_t ready;
	long *current;
	long *prev;
} pingpong_t;

void pingpong_init(pingpong_t *p, size_t s);
void pingpong_swap(pingpong_t *p);
#endif
