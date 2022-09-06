
#ifndef __AMPM_LL_H__
#define __AMPM_LL_H__
#include <stdint.h>
#include <stdbool.h>
#include "ts_controller.h"


#define PACKET_TYPE_LEN             (1)
#define PACKET_LENGTH_LEN           (1)
#define PACKET_ADDR_LEN             (BLE_GAP_ADDR_LEN)

#define PACKET_TYPE_POS             (0)
#define PACKET_LENGTH_POS           (1)
#define PACKET_PADDING_POS          (2)
#define PACKET_ADDR_POS             (3)
#define PACKET_DATA_POS             (PACKET_ADDR_POS + PACKET_ADDR_LEN)


#define PACKET_TYPE_ADV_NONCONN     (0x02)

#define PACKET_TYPE_MASK            (0x0F)
#define PACKET_LENGTH_MASK          (0x3F)
#define PACKET_ADDR_TYPE_MASK       (0x40)

#define PACKET_DATA_MAX_LEN         (28)
#define PACKET_MAX_CHAIN_LEN        (1)

//typedef struct
//{
//    uint8_t header;
//		uint8_t pkg_len;
//		uint8_t padding;
//    uint8_t pkg[PACKET_DATA_MAX_LEN + 6];
//    uint32_t rx_crc;
//		uint8_t rssi;
//} packet_t;


extern uint32_t m_packets_invalid;
extern uint32_t m_packets_valid;

void ampm_ll_rx_cb (bool crc_valid);
void ampm_ll_timeout_cb (void);
void ampm_ll_tx_cb (void);

uint32_t ampm_ll_start (void);
void ampm_ll_callback_set(uint8_t (*callback)(packet_t *data_in_out));

#endif
