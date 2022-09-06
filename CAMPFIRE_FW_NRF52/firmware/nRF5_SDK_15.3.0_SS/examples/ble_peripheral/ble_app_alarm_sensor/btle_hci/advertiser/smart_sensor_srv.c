#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_advdata.h"
#include "ts_controller.h"
#include "ts_peripheral.h"
#include "nrf_advertiser.h"
#include "smart_sensor_srv.h"

#define DBG_MESH_SRV(...) NRF_LOG_INFO(__VA_ARGS__)
#define DBG_MESH_SRV_PUTS //dbg_puts

#define MESH_NODE_ADDR0_POS (PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 0)
#define MESH_NODE_ADDR1_POS (PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 1)
#define MESH_NODE_ADDR2_POS (PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 2)
#define MESH_NODE_ADDR3_POS (PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 3)
#define MESH_NODE_ADDR4_POS (PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 4)
#define MESH_NODE_ADDR5_POS (PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 5)
#define MESH_NODE_CMD_POS 	(PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 6)
#define MESH_NODE_TYPE_POS 	(PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 7)
#define MESH_NODE_PAYLOAD_POS 	(PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 8)
#define MESH_NODE_PAYLOAD_CRC 	(PACKET_DATA_POS + MESH_PACKET_DATA_OFFSET + 8 + 8)
extern uint32_t default_crc;
extern ble_gap_addr_t _device_addr;
uint8_t vtsmart_sensor_got_data_flag;
mesh_packet_t		mesh_rx;
mesh_packet_t		mesh_tx;
uint8_t rx_cnt = 0;
uint16_t mesh_version = 0;
uint8_t my_handle;                     
bool has_anything_to_send;
ble_gap_addr_t neighbor_addr;
uint8_t rx_miss_times = 0;
sensor_neighbor_t sensorMode;
btle_cmd_param_le_write_advertising_parameters_t adv_params;
static bool is_initialized = false;
uint64_t GetMyAddr(ble_gap_addr_t addr)
{
	uint64_t ll_addr = 0;
	uint8_t *pt = (uint8_t *)&ll_addr;
	pt[0] = addr.addr[0];
	pt[1] = addr.addr[1];
	pt[2] = addr.addr[2];
	pt[3] = addr.addr[3];
	pt[4] = addr.addr[4];
	pt[5] = addr.addr[5];
	return ll_addr;
}

static void hci_setup(void)
{
	/* we use software interrupt 0 */
	btle_hci_adv_init(SWI0_IRQn);
	
	/* want to maximize potential scan requests */
	adv_params.channel_map = BTLE_CHANNEL_MAP_ALL;
	adv_params.direct_address_type = BTLE_ADDR_TYPE_RANDOM;
	adv_params.filter_policy = BTLE_ADV_FILTER_ALLOW_ANY;
	adv_params.interval_min = 3200;
	adv_params.interval_max = 4000;
	
	adv_params.own_address_type = BTLE_ADDR_TYPE_RANDOM;
	
	/* Only want scan requests */
	adv_params.type = BTLE_ADV_TYPE_SCAN_IND;

	btle_hci_adv_params_set(&adv_params);

	btle_hci_tx_data_callback_set(hci_app_data_tx_callback);

	btle_hci_rx_data_callback_set(hci_app_data_rx_callback);
	/* all parameters are set up, enable advertisement */
	btle_hci_adv_enable(BTLE_ADV_ENABLE);
}


uint32_t vtsmart_sensor_mesh_srv_init(uint32_t access_address,uint32_t txPower,uint8_t channel)
{
//		radio_set_param(access_address,txPower,channel);
//		hci_setup();
//		/* Get device address for packaging packet*/
//		#if (NRF_SD_BLE_API_VERSION >= 3)
//		sd_ble_gap_addr_get(&_device_addr);
//		#else
//		sd_ble_gap_address_get(&_device_addr);
//		#endif
//		my_handle = (GetMyAddr(_device_addr) % MESH_HANDLE_CNT) + 1;
//		if(my_handle == 0) my_handle = 1;
//		else if(my_handle > MESH_HANDLE_CNT) my_handle = MESH_HANDLE_CNT;
//		is_initialized = true;
    return NRF_SUCCESS;
}


static void packet_create_from_data(uint8_t* data, mesh_packet_t* packet)
{
    /* advertisement package */
    
    packet->length = (data[PACKET_LENGTH_POS] & PACKET_LENGTH_MASK) - PACKET_ADDR_LEN;
    memcpy(packet->data, &data[PACKET_DATA_POS], packet->length);
    memcpy(packet->sender.addr, &data[PACKET_ADDR_POS], PACKET_ADDR_LEN);
    
    /* addr type */
    bool addr_is_random = (data[PACKET_TYPE_POS] & PACKET_ADDR_TYPE_MASK);
    
    if (addr_is_random)
    {
        bool is_static = ((packet->sender.addr[5] & 0xC0) == 0xC0);
        if (is_static)
        {
            packet->sender.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
        }
        else
        {
            bool is_resolvable = ((packet->sender.addr[5] & 0xC0) == 0x40);
            packet->sender.addr_type = (is_resolvable? 
                BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE :
                BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE);
        }
    }
    else
    {
        packet->sender.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
    }
}

enum{
	SEND_OUT_A_REQUEST_TO_GET_OLD_DATA_FROM_NEIGHBOR = 0,
	SEND_OUT_A_NEW_DATA_TO_MESH
}tx_phase = SEND_OUT_A_REQUEST_TO_GET_OLD_DATA_FROM_NEIGHBOR;

void hci_app_data_tx_callback(ts_packet_t *packet) // to ble_adv_set_adv_data_calback
{
	switch(tx_phase)
	{
		case SEND_OUT_A_REQUEST_TO_GET_OLD_DATA_FROM_NEIGHBOR:
			rx_miss_times++;
			packet->header = 0x40 | PACKET_TYPE_VTSMART;
			memcpy(packet->addr,_device_addr.addr, PACKET_ADDR_LEN);
			packet->pkg[MESH_PACKET_HANDLE_OFFSET] = my_handle;
			packet->pkg[MESH_PACKET_VERSION_OFFSET] = mesh_version;
			memcpy(&packet->pkg[MESH_PACKET_DATA_OFFSET],neighbor_addr.addr,BLE_GAP_ADDR_LEN);
			memcpy(&packet->pkg[MESH_PACKET_DATA_OFFSET + BLE_GAP_ADDR_LEN + 2/*CMD and CMD_TYPE*/],&default_crc,sizeof(default_crc));
			packet->pkg_len = BLE_GAP_ADDR_LEN + 0x17;		
		break;
		case SEND_OUT_A_NEW_DATA_TO_MESH:
			packet->header = 0x40 | PACKET_TYPE_ADV_NONCONN;
			mesh_version++;
			memcpy(packet->addr,_device_addr.addr, PACKET_ADDR_LEN);
			memcpy(packet->pkg,mesh_tx.data,mesh_tx.length);
			//packet->pkg[MESH_PACKET_HANDLE_OFFSET] = my_handle;
			packet->pkg[MESH_PACKET_VERSION_OFFSET] = mesh_version;
			packet->pkg[MESH_PACKET_VERSION_OFFSET + 1] = mesh_version>>8;
			//memcpy(&packet->pkg[MESH_PACKET_DATA_OFFSET],mesh_tx.sender.addr, PACKET_ADDR_LEN);
			packet->pkg_len = BLE_GAP_ADDR_LEN + mesh_tx.length;	
			tx_phase = SEND_OUT_A_REQUEST_TO_GET_OLD_DATA_FROM_NEIGHBOR;
			has_anything_to_send = false;
		break;
	}
		
}

uint32_t vtsmart_sensor_prepare_data(uint8_t *data,uint8_t len)
{
	if(is_initialized == false)
	{
		return NRF_ERROR_BUSY;
	}
	if(has_anything_to_send == false)
	{
		mesh_tx.data[MESH_PACKET_HANDLE_OFFSET] = my_handle;
		memcpy(&mesh_tx.data[MESH_PACKET_DATA_OFFSET],data,len);
		mesh_tx.length = len + MESH_PACKET_VERSION_LEN + MESH_PACKET_HANDLE_LEN;
		has_anything_to_send = true;
		return NRF_SUCCESS;  
	}
	return NRF_ERROR_BUSY;
}

uint32_t hci_app_data_rx_callback(ts_packet_t *radio_data,uint8_t rssi)
{
	if(((radio_data->header & PACKET_TYPE_MASK) == PACKET_TYPE_VTSMART)
		&& vtsmart_sensor_got_data_flag == 0
		&& ((radio_data->pkg[MESH_PACKET_DATA_OFFSET] == _device_addr.addr[0]
			&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 1] == _device_addr.addr[1]
			&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 2] == _device_addr.addr[2]
			&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 3] == _device_addr.addr[3]
			&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 4] == _device_addr.addr[4]
			&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 5] == _device_addr.addr[5]
		)
		|| (radio_data->pkg[MESH_PACKET_DATA_OFFSET] == 0x99
				&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 1] == 0x99
				&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 2] == 0x99
				&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 3] == 0x99
				&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 4] == 0x99
				&& radio_data->pkg[MESH_PACKET_DATA_OFFSET + 5] == 0x99
			)
		)
	)
	{
		rx_miss_times = 0;
		packet_create_from_data((uint8_t *)radio_data,&mesh_rx);
		uint16_t version;
		version = (mesh_rx.data[MESH_PACKET_VERSION_OFFSET] | (((uint16_t) mesh_rx.data[MESH_PACKET_VERSION_OFFSET + 1]) << 8));
		if(version != mesh_version)
		{
			mesh_version = version;
			vtsmart_sensor_got_data_flag  = 1;
		}
		#if (!defined(BOARD_SOS))
		if(sensorMode == SENSOR_FIND_NEIGHBOR_MODE)
		{
			if(radio_data->addr[0] != _device_addr.addr[0]
				&& radio_data->addr[1] != _device_addr.addr[1]
				&& radio_data->addr[2] != _device_addr.addr[2]
				&& radio_data->addr[3] != _device_addr.addr[3]
				&& radio_data->addr[4] != _device_addr.addr[4]
				&& radio_data->addr[5] != _device_addr.addr[5]
			)
			{
				memcpy(neighbor_addr.addr,radio_data->addr,BLE_GAP_ADDR_LEN);
				sensorMode = SENSOR_FOUND_NEIGHBOR_MODE;
			}
		}
		#endif
		if(has_anything_to_send)
		{
			tx_phase = SEND_OUT_A_NEW_DATA_TO_MESH;
		}
	}
	rx_cnt++;
	return 0;
}

bool vtsmart_sensor_is_busy(void)
{
	return ts_is_in_timeslot();
}

uint32_t vtsmart_sensor_send_data(uint32_t access_address,uint32_t txPower,uint8_t channel)
{
	//setup a timeslot to send out message ASAP
	if(!ts_is_in_timeslot())
	{
		radio_set_param(access_address,txPower,channel);
		ctrl_timeslot_order();
		return NRF_SUCCESS;
	}
	return NRF_ERROR_BUSY;
}



