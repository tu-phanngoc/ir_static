#include <stdint.h>
#include "system_config.h"
#include "app_config_task.h"
#include "dimmer.h"
#include "dimmer_utils.h"

#define DIMMER_DEFAULT_VALUE	1

/* Static variables */
bool _is_need_to_send_status = true;

/* Static function prototypes */
static void _send_presentation(void);

/* Public functions */
void dimmer_init() {
	dimmer_dev_init();
	
	/* Control dimmer*/
	dimmer_set(DIMMER_DEFAULT_VALUE);
	
	/* Send out presentation message */
	_send_presentation();
}

void dimmer_update_value(uint8_t percent) {
	static bool _is_using_default_value = true;
	uint8_t old_value;
	
	if (percent > DIMMER_MAX_PERCENT) {
		percent = DIMMER_MAX_PERCENT;
	}
	if (percent < DIMMER_MIN_PERCENT) {
		percent = DIMMER_MIN_PERCENT;
	}
	
	if (_is_using_default_value) {
		_is_using_default_value = false;
		old_value = DIMMER_DEFAULT_VALUE;
		
	} else {
		old_value = sys_cfg_get_dim_percent();
	}
	
	if (old_value != percent) {
		dimmer_set(percent);
		
		/* Save dim percent */
		sys_cfg_set_dim_percent(percent);
	}
	
	/* Trigger to send out dimmer status */
	_is_need_to_send_status = true;
}

void dimmer_send_status() {
	if (_is_need_to_send_status) {
		uint8_t buff[PROTOCOL_PAYLOAD_LEN];
		memset(buff, 0x00, PROTOCOL_PAYLOAD_LEN);
		buff[0] = DIMMER_ID;
		buff[1] = sys_cfg_get_dim_percent();
		
		protocol_send_data_to_gw(buff, 2, ePROTOCOL_CMD_TYPE_SET, ePROTOCOL_VALUE_TYPE_PERCENTAGE);
		_is_need_to_send_status = false;
	}
}

void dimmer_trigger_to_send_status() {
	_is_need_to_send_status = true;
}

/* Static functions */
static void _send_presentation() {
	protocol_send_presentation(DIMMER_ID, ePROTOCOL_DEVICE_TYPE_DIMMER);
}
