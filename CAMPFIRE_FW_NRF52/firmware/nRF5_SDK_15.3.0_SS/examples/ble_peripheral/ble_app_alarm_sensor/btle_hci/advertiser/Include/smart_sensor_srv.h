

#ifndef __VTSMART_MESH_SRV_H__
#define __VTSMART_MESH_SRV_H__

#include "ble_gap.h"
#include "ts_controller.h"
#include "nrf_advertiser.h"
#include "ts_peripheral.h"
/* Packet related constants */
#define MESH_PACKET_HANDLE_LEN          (1)
#define MESH_PACKET_VERSION_LEN         (2)

#define MESH_PACKET_HANDLE_OFFSET       (0)
#define MESH_PACKET_VERSION_OFFSET      (MESH_PACKET_HANDLE_OFFSET + MESH_PACKET_HANDLE_LEN)
#define MESH_PACKET_DATA_OFFSET         (MESH_PACKET_VERSION_OFFSET + MESH_PACKET_VERSION_LEN)

#define CONN_HANDLE_INVALID             (0xFFFF)

#define MESH_VALUE_LOLLIPOP_LIMIT       (200)


#define MAX_VALUE_COUNT                 (155) /* The highest possible number of values stored in the mesh. */
#define MAX_VALUE_LENGTH                (28) /* The maximum number of bytes available in one mesh value. */

#define MESH_SRV_UUID                   (0x0001) /* Mesh service UUID */
#define MESH_MD_CHAR_UUID               (0x0002) /* Mesh metadata characteristic UUID */
#define MESH_VALUE_CHAR_UUID            (0x0003) /* Mesh value characteristic UUID */

#define MESH_MD_CHAR_LEN                (10) /* Total length of Mesh metadata characteristic data */
#define MESH_MD_CHAR_AA_OFFSET          (0) /* Metadata characteristic Access Address offset */
#define MESH_MD_CHAR_ADV_INT_OFFSET     (4) /* Metadata characteristic Advertisement interval offset */
#define MESH_MD_CHAR_COUNT_OFFSET       (8) /* Metadata characteristic value count offset */
#define MESH_MD_CHAR_CH_OFFSET          (9) /* Metadata characteristic channel offset */

#define MESH_MD_FLAGS_USED_POS          (0) /* Metadata flag: Used */
#define MESH_MD_FLAGS_INITIALIZED_POS   (1) /* Metadata flag: Initialized */
#define MESH_MD_FLAGS_IS_ORIGIN_POS     (2) /* Metadata flag: Is origin */

//#define MESH_HANDLE_CNT 5



typedef enum {
	SENSOR_FIND_NEIGHBOR_MODE = 0,
	SENSOR_FOUND_NEIGHBOR_MODE,
}sensor_neighbor_t;

typedef struct
{
    ble_gap_addr_t sender;
    uint8_t length;
    uint8_t data[PACKET_DATA_MAX_LEN];
    uint32_t rx_crc;
} mesh_packet_t;

extern sensor_neighbor_t sensorMode;
extern ble_gap_addr_t _device_addr;
extern ble_gap_addr_t neighbor_addr;
extern mesh_packet_t		mesh_rx;
extern uint8_t rx_miss_times;
uint32_t vtsmart_sensor_mesh_srv_init(uint32_t access_address,uint32_t txPower,uint8_t channel);
extern uint8_t vtsmart_sensor_got_data_flag;
uint32_t vtsmart_sensor_prepare_data(uint8_t *data,uint8_t len);
//void packet_create_from_data(uint8_t* data, mesh_packet_t* packet);
void hci_app_data_tx_callback(ts_packet_t *packet);
uint32_t hci_app_data_rx_callback(ts_packet_t *radio_data,uint8_t rssi);
//uint32_t mesh_srv_packet_process(mesh_packet_t* packet);
uint32_t vtsmart_sensor_send_data(uint32_t access_address,uint32_t txPower,uint8_t channel);
bool vtsmart_sensor_is_busy(void);
#endif
