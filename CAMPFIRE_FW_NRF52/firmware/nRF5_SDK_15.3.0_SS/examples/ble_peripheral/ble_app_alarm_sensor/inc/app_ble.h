
#ifndef __APP_BLE_H__
#define __APP_BLE_H__
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "boards.h"
//#include "softdevice_handler.h"
#include "nrf_ble_gatt.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "fds.h"
//#include "fstorage.h"
#include "nrf_ble_gatt.h"
#include "ble_conn_state.h"
#include "app_gpiote.h"
#include "app_timer.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "m_nrf_spi.h"
#include "lib/ringbuf.h"

#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "ble_gap.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_mesh.h"
extern RINGBUF BLE_RxRingBuff;
extern RINGBUF BLE_TxRingBuff;
extern ISMARTPACKAGE giSmartPacket;

uint32_t ble_get_data(uint8_t *data);
uint32_t ble_is_connected(void);
uint32_t ble_send_data(uint8_t *data,uint8_t len);
uint32_t ble_check_status(void);
void ble_app_init(void);

void advertising_start(bool erase_bonds);
void sleep_mode_enter(void);
uint8_t BLE_PutChar (uint8_t ch);
void BLE_PutString (char *s);
uint32_t ble_task(void);
uint32_t force_device_disconnect(void);
void set_ble_transmit_status(uint8_t status);
uint8_t get_ble_transmit_status(void);

void BLE_SendFromNUSToMesh();
#endif

