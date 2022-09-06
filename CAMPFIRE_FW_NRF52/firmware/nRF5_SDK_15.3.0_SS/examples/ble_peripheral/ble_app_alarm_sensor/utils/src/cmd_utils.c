#include "lib\sys_time.h"
#include "cmd_utils.h"
#include "cmd_utils.h"
#include "sensor_utils.h"

void cmd_process_internal_command(ISMARTPACKAGE *p_packet) {
	switch (p_packet->packet.type) {
		case ePROTOCOL_INTERAL_TYPE_TIME:
			time_update(p_packet->packet.data);
			
			break;
		case ePROTOCOL_INTERAL_TYPE_SET:
			set_sensor_status(p_packet->packet.data[1]);
			
			break;
		default:
			break;
	}
}
