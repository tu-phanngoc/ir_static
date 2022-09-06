#ifndef __CFSM_H__
#define __CFSM_H__

#include <stdint.h>


enum ret_codes {
	ok, 
	fail, 
	repeat
};

int fsm_main_loop();
#endif


