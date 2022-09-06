#include <stdlib.h>
#include "nrf_assert.h"
#include "pingpong.h"
#define APP_ASSERT ASSERT
void pingpong_init(pingpong_t *p, size_t s)
{
	p->pong = (long *)malloc(s * sizeof(long));
	APP_ASSERT(p->pong != NULL);
	p->ping = (long *)malloc(s * sizeof(long));
	APP_ASSERT(p->ping != NULL);
	p->prev = p->pong;
	p->current = p->ping;
}

void pingpong_swap(pingpong_t *p)
{
	register long *tmp = p->prev;
	p->prev = p->current;
	p->current = tmp;
	p->ready = 1;
}

void pingpong_deinit(pingpong_t *p)
{
	free(p->pong);
	free(p->ping);
	p->prev = NULL;
	p->current = NULL;
}
