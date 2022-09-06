
#include "m_nrf_spi.h"

//***********************************************************************************

void spi1_Init(void)
{
    //Configure GPIO
		nrf_gpio_pin_set(SPI_FLASH_CS);
		nrf_gpio_cfg_output(SPI_FLASH_CS);
		nrf_gpio_pin_set(SPI_FLASH_CS);
	
    nrf_gpio_cfg_output(SPIM1_SCK_PIN);
    nrf_gpio_cfg_output(SPIM1_MOSI_PIN);
    nrf_gpio_cfg_input(SPIM1_MISO_PIN, NRF_GPIO_PIN_NOPULL);
		//mapping
		NRF_SPI1->PSELSCK = SPIM1_SCK_PIN;
		NRF_SPI1->PSELMOSI = SPIM1_MOSI_PIN;
		NRF_SPI1->PSELMISO = SPIM1_MISO_PIN;
		NRF_SPI1->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M1;
		NRF_SPI1->CONFIG = (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos)//MSB first
											| (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos)
											| (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
		/* Clear waiting interrupts and events */
		NRF_SPI1->EVENTS_READY = 0;
		NRF_SPI1->INTENSET = 0; //disable interrupt
	/* Enable SPI hardware */
		NRF_SPI1->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);
}



uint8_t halSpi1_WriteByte(uint8_t data)
{
    uint8_t rc = 0;
		uint32_t timeout = 1000;
    while (NRF_SPI1->EVENTS_READY && timeout--);
		timeout = 1000;
		NRF_SPI1->EVENTS_READY = 0;
		NRF_SPI1->TXD = data;
    while (!NRF_SPI1->EVENTS_READY && timeout--);
		NRF_SPI1->EVENTS_READY = 0;
    rc = NRF_SPI1->RXD;
    return rc;
}


