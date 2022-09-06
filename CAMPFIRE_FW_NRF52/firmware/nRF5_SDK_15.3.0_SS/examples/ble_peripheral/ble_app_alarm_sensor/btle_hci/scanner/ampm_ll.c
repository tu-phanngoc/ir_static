

#include "ampm_ll.h"


#include "radio.h"
#include "boards.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "system_config.h"
#include "ble_gap.h"
/*****************************************************************************
* Local definitions
*****************************************************************************/
 
                              


/*****************************************************************************
* Static Globals
*****************************************************************************/

uint8_t radio_channel = 37;
uint32_t m_packets_invalid;
uint32_t m_packets_valid;


extern ts_packet_t radio_rx_packet;
extern ts_packet_t radio_tx_packet;

uint8_t m_rssi;

/*****************************************************************************
* Static Function prototypes
*****************************************************************************/

uint8_t (*process_rx_packet)(ts_packet_t *data_in_out);

/**@brief Entry function for SCANNER_STATE_RECEIVE_ADV */
static void m_state_receive_entry (void);

/**@brief Exit function for SCANNER_STATE_RECEIVE_ADV */
static void m_state_receive_exit (void);

/**@brief Entry function for SCANNER_STATE_SEND_REQ */
static void m_state_send_entry (void);


/*****************************************************************************
* Static Function definitions
*****************************************************************************/

void ampm_ll_callback_set(uint8_t (*callback)(ts_packet_t *data_in_out))//added by thienhaiblue@gmail.com#date:26/10/2015
{
	process_rx_packet = callback;
}

/**
* @brief check whether a packet is a valid data packet, eligible for processing
*/
static inline bool packet_is_data_packet(uint8_t* data)
{
	//return 1;
    return ((data[PACKET_TYPE_POS] & PACKET_TYPE_MASK) 
        == PACKET_TYPE_ADV_NONCONN);
}

static void m_state_receive_entry (void)
{
  radio_buffer_configure ((uint8_t *)&radio_rx_packet);
  radio_rx_prepare (true);
  radio_rssi_enable ();
 
  radio_tx_mode_on_receipt ();
}

static void m_state_receive_exit (void)
{
  m_rssi = radio_rssi_get ();
}

static void m_state_send_entry (void)
{
	if(radio_tx_packet.pkg_len > PACKET_DATA_MAX_LEN)
		radio_tx_packet.pkg_len = PACKET_DATA_MAX_LEN;
  radio_buffer_configure ((uint8_t *)&radio_tx_packet);
  radio_tx_prepare ();
}



static void m_state_receive_tx_wait_rx_entry (void)
{
  radio_buffer_configure ((uint8_t *)&radio_tx_packet);
  radio_rx_prepare (false);
  radio_rssi_enable ();
  radio_rx_timeout_enable ();
}


/*****************************************************************************
* Interface Functions
*****************************************************************************/

void ampm_ll_rx_cb (bool crc_valid)
{
  /* Received invalid packet */
	uint32_t checksum = (NRF_RADIO->RXCRC & 0x00FFFFFF);
  if (crc_valid && packet_is_data_packet((uint8_t *)&radio_rx_packet))
  {
		m_state_receive_exit ();
		m_packets_valid++;
		radio_rx_packet.rx_crc = checksum;
		if (process_rx_packet(&radio_rx_packet))
		{
			radio_tx_packet = radio_rx_packet;
			m_state_send_entry ();
		}
		else
		{
			m_state_receive_entry ();
		}
  }
  else
	{
		m_packets_invalid++;
		m_state_receive_exit ();
		radio_disable ();
		m_state_receive_entry ();
	}
}

void ampm_ll_tx_cb (void)
{
		m_state_receive_tx_wait_rx_entry ();
}

void ampm_ll_timeout_cb (void)
{
		m_state_receive_exit ();
		radio_disable ();
		m_state_receive_entry ();
}


uint32_t ampm_ll_start (void)
{ 
	radio_channel++;
	if(radio_channel >= 40) radio_channel = 37;

  NVIC_EnableIRQ(TIMER0_IRQn);
  
  radio_init (radio_channel);
  radio_rx_timeout_init ();
  
  m_state_receive_entry ();

  return NRF_SUCCESS;
}


