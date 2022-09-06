#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
//#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "boards.h"
#include "app_util_platform.h"
#include "app_gpiote.h"
//#ifdef USE_MESH
#include "rbc_mesh.h"
#include "timeslot_handler.h"
//#endif

#include "system_config.h"

#include "lib/ampm_sprintf.h"
#include "lib/data_cmp.h"
#include "lib/ringbuf.h"
#include "lib/sys_tick.h"

#include "app_message_queue.h"
#include "aes128.h"

#include "nrf_delay.h"
#include "sensor_process.h"
#include "transport_control.h"
#include "app_mesh.h"
extern uint32_t default_crc;
extern ble_gap_addr_t _device_addr;

//packet_t		mesh_rx;
static uint16_t mesh_version;
uint8_t *default_crc_pt = (uint8_t *)&default_crc;


void RF_SensorInit(void)
{

}




