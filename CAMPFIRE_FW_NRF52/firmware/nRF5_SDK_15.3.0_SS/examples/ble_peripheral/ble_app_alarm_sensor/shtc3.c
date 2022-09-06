#include "shtc3.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_twi.h"
#define SHTC3_ADDRESS								0x70
#define SHTC3_ADDRESS_READ          SHTC3_ADDRESS //(0x70 << 1) | 0x01
#define SHTC3_ADDRESS_WRITE         SHTC3_ADDRESS //(0x70 << 1)

#define SHTC3_PRODUCT_CODE_MASK     0x083F
#define SHTC3_SENSOR_ID_MASK        0xF7C0

#define SHTC3_DEBUG									NRF_LOG_INFO
//#define SHTC3_DEBUG(...)

// NOTE: all commands are "byte swapped" (ntohs), meaning:
// 0x3517 -> 0x1735

#define SHTC3_CMD_WAKEUP                                    0x1735
#define SHTC3_CMD_SLEEP                                     0x98B0
#define SHTC3_CMD_SOFT_RESET                                0x5D80
#define SHTC3_CMD_READ_ID                                   0xC8EF //0xC8EF

// Clock stretching based commands
#define SHTC3_CMD_CLK_STRETCH_READ_HUM_FIRST                0x245C
#define SHTC3_CMD_CLK_STRETCH_READ_HUM_FIRST_LOW_POWER      0xDE44

#define SHTC3_CMD_CLK_STRETCH_READ_T_FIRST                	0xA27C

// Polling commands
#define SHTC3_CMD_POLL_HUM_FIRST                            0xE058
#define SHTC3_CMD_POLL_HUM_FIRST_LOW_POWER                  0x1A40

uint32_t sht_temperature = 2665;
uint8_t sht_humidity = 69;

int32_t SHTC3_raw2DegC(uint16_t T)
{
	return -45 + 175 * ((float)T / 65535);
}

int32_t SHTC3_raw2DegF(uint16_t T)
{
	return SHTC3_raw2DegC(T) * (9.0 / 5) + 32.0;
}

int32_t SHTC3_raw2Percent(uint16_t RH)
{
	return 100 * ((float)RH / 65535);
}

uint16_t shtc3_read_id(nrf_drv_twi_t const * p_instance)
{
  uint8_t data[2];
  uint16_t command = SHTC3_CMD_READ_ID;
	nrf_drv_twi_tx(p_instance, SHTC3_ADDRESS, (uint8_t*)&command, 2, true);
	nrf_drv_twi_rx(p_instance, SHTC3_ADDRESS, (uint8_t*)data, 2);
	
	NRF_LOG_INFO("id: %d %d", data[0], data[1]);
  // SHTC3 16 bit ID encoded as:
  // xxxx 1xxx xx00 0111
  // where "x" are actual, sensor ID, while the rest
  // sensor product code (unchangeable)
  uint16_t id = data[0] << 8 | data[1];
  uint16_t code = id & SHTC3_PRODUCT_CODE_MASK;
  if (code == 0x807) {
    // Sensor preset, return actual ID
    return id & SHTC3_SENSOR_ID_MASK;
  }
  return 0;
}

uint32_t shtc3_sleep(nrf_drv_twi_t const *p_instance)
{
  uint16_t command = SHTC3_CMD_SLEEP;
	nrf_drv_twi_tx(p_instance, SHTC3_ADDRESS_WRITE, (uint8_t*)&command, 2, false);
}

uint32_t shtc3_wakeup(nrf_drv_twi_t const *p_instance)
{
  uint16_t command = SHTC3_CMD_WAKEUP;
	nrf_drv_twi_tx(p_instance, SHTC3_ADDRESS_WRITE, (uint8_t*)&command, 2, false);
}

uint32_t shtc3_softreset(nrf_drv_twi_t const *p_instance)
{
  uint16_t command = SHTC3_CMD_SOFT_RESET;
	nrf_drv_twi_tx(p_instance, SHTC3_ADDRESS_WRITE, (uint8_t*)&command, 2, false);
}

static uint32_t checkCRC(uint16_t value, uint8_t expected)
{
	uint8_t data[2] = {value >> 8, value & 0xFF};
	uint8_t crc = 0xFF;
	uint8_t poly = 0x31;

	for (uint8_t indi = 0; indi < 2; indi++) {
		crc ^= data[indi];
		for (uint8_t indj = 0; indj < 8; indj++) {
			if (crc & 0x80) {
				crc = (uint8_t)((crc << 1) ^ poly);
			} else {
				crc <<= 1;
			}
		}
	}

	if (expected ^ crc)	{
    return 0;
	}
  return 1;
}

static uint32_t _read_values(uint8_t* data, int32_t* out_temp, int32_t* out_hum)
{
  // Check CRC
  uint32_t raw_hum = data[0] << 8 | data[1];
  uint32_t raw_temp = data[3] << 8 | data[4];

  if (!checkCRC(raw_hum, data[2])) {
    return 0;
  }
  if (!checkCRC(raw_temp, data[5])) {
    return 0;
  }

//  // Convert values
  if (out_hum) {
//    *out_hum = raw_hum * 100 / 65535;
		*out_hum = SHTC3_raw2Percent(raw_hum);
  }
  if (out_temp) {
    *out_temp = raw_temp * 17500 / 65535 - 4500;
//		*out_temp = SHTC3_raw2DegC(raw_temp);
  }
//	*out_hum = raw_hum;
//	*out_temp = raw_temp;
  return 1;
}

static uint32_t _perform_measurements(nrf_drv_twi_t const *p_instance, uint16_t command, int32_t* out_temp, int32_t* out_hum)
{
  uint8_t result[6];

  uint32_t res = nrf_drv_twi_tx(p_instance, SHTC3_ADDRESS_WRITE, (uint8_t*)&command, 2, false);

  res = nrf_drv_twi_rx(p_instance, SHTC3_ADDRESS_READ, (uint8_t*)result, 6);

  return _read_values(result, out_temp, out_hum);
}

uint32_t shtc3_perform_measurements(nrf_drv_twi_t const *p_instance, int32_t* out_temp, int32_t* out_hum)
{
  return _perform_measurements(p_instance, SHTC3_CMD_CLK_STRETCH_READ_HUM_FIRST, out_temp, out_hum);
}

uint32_t shtc3_perform_measurements_low_power(nrf_drv_twi_t const *p_instance, int32_t* out_temp, int32_t* out_hum)
{
  return _perform_measurements(p_instance, SHTC3_CMD_CLK_STRETCH_READ_HUM_FIRST_LOW_POWER, out_temp, out_hum);
}


 /* Number of possible TWI addresses. */
#define TWI_INSTANCE_ID     0
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
#define TWI_ADDRESSES      127


/**
 * @brief TWI initialization.
 */
void twi_init (void)
{
		ret_code_t err_code;
    const nrf_drv_twi_config_t twi_config = {
       .scl                = 20,
       .sda                = 19,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };
    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    nrf_drv_twi_enable(&m_twi);
}

/**
 * @brief Function for main application entry.
 */
uint8_t shtc3_init()
{
	NRF_LOG_INFO("shtc3_init");
	ret_code_t err_code;
	uint8_t address;
	uint8_t sample_data;
	bool detected_device = false;
	twi_init();
	nrf_delay_ms(100);
	for (address = 1; address <= TWI_ADDRESSES; address++)
	{
		nrf_delay_ms(1);
			err_code = nrf_drv_twi_rx(&m_twi, address, &sample_data, sizeof(sample_data));
			if (err_code == NRF_SUCCESS)
			{
					detected_device = true;
				NRF_LOG_INFO("TWI device detected at address 0x%x. sample_data: %d", address, sample_data);
			}
			NRF_LOG_FLUSH();
	}
	uint16_t id = 0;
	nrf_delay_ms(10);
	id = shtc3_read_id(&m_twi);
	NRF_LOG_INFO("\r\n id: %d",id);
	if(detected_device == false)
		return false;
	else
	{
		shtc3_wakeup(&m_twi);
		nrf_delay_ms(10);
		shtc3_softreset(&m_twi);
		nrf_delay_ms(10);
		return true;
	}
}
#ifdef BENKON_HW_V2
#elif BENKON_HW_V3
#define SENSOR_RAW_DATA
#endif
//#define SENSOR_RAW_DATA
void shtc3_loop()
{
	int32_t temp;
	int32_t hum;
	if (shtc3_perform_measurements(&m_twi, &temp, &hum)) {
		#ifdef SENSOR_RAW_DATA
		sht_temperature = temp;
		sht_humidity = hum;	
//		if(sht_temperature < 2300)
//		{
//			sht_temperature = sht_temperature + 150;
//		}
//		else if(sht_temperature > 2900)
//		{
//			sht_temperature = sht_temperature - 120;
//		}
		sht_humidity = sht_humidity + 6;
		#else
		/**/
		//
		//T'=T*0.93-2.123
		double dTemp = temp * 0.93 - 212.3;
		sht_temperature = (uint16_t)dTemp;
		//RH'=RH*0.9817 + T*0.2590 + 9.5786
		double dHumi = 0.2590*(temp/100)+ 0.9817*hum + 9.5786;
		sht_humidity = (uint8_t)dHumi;
		//
		#endif
		/**/
		// Do something with values
		SHTC3_DEBUG("Temperature: %d.%d, Humidity: %d",sht_temperature/100, sht_temperature % 100, sht_humidity);
//		app_shtc3_sleep();
	}
}

void app_shtc3_sleep()
{
	shtc3_sleep(&m_twi);
}