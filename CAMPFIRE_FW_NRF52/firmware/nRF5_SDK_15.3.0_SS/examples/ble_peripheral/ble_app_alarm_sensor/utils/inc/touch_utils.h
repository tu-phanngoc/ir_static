#ifndef __TOUCH_UTILS_H__
#define __TOUCH_UTILS_H__

#include "stdbool.h"
#include "boards.h"

typedef enum _touch_btn_no_e {
	eTOUCH_BUTTON_NO_0 = 0,
	eTOUCH_BUTTON_NO_1,
	eTOUCH_BUTTON_NO_2,
	eTOUCH_BUTTON_NO_3
} touch_btn_no_e;

/* Function prototype */
void touch_btn_init(void);
void touch_btn_update_look(uint8_t status);
void touch_btn_update_single_look(touch_btn_no_e btn, bool status);
void touch_btn_set_default_look(void);
#endif
