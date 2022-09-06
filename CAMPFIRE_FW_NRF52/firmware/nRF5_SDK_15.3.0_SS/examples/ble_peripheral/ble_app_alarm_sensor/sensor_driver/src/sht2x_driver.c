/**************************************************************************************************
/@ Project   :  Smart Home      
/@ File      :  humid_driver.c
/@ Author    :  
/@ Brief     :  Driver for the Sensirion SHT21 Humidity sensor
**************************************************************************************************/


/* ------------------------------------------------------------------------------------------------
*                                           Includes
* -------------------------------------------------------------------------------------------------
*/

#include "sht2x_driver.h"

#include "I2C_Comm.h"
#include "nrf_delay.h"

#include <math.h>

#if (defined(BOARD_STH2x))

/* ------------------------------------------------------------------------------------------------
*                                           Defines
* -------------------------------------------------------------------------------------------------
*/
#define Temperature SHT_Temp_Read()
#define Humidity    SHT_Humid_Read()

// Sensor I2C address
#define HAL_SHT21_I2C_ADDRESS      0x40

#define S_REG_LEN                  2
#define DATA_LEN                   3

// Sensor Commands
#define SHT21_CMD_TEMP_T_H         0xE3 // Command trig. temp meas. hold master
#define SHT21_CMD_HUMID_T_H         0xE5 // Command trig. humidity meas. hold master
#define SHT21_CMD_TEMP_T_NH        0xF3 // Command trig. temp meas. no hold master
#define SHT21_CMD_HUMID_T_NH        0xF5 // Command trig. humidity meas. no hold master
#define SHT21_CMD_WRITE_U_R        0xE6 // Command write user register
#define SHT21_CMD_READ_U_R         0xE7 // Command read user register
#define SHT21_CMD_SOFT_RST         0xFE // Command soft reset

// User Register Modes
#define USR_REG_DEFAULT            0x02 // Disable OTP reload
#define USR_HEATER_ON              0x04 // Heater on
#define USR_HEATER_OFF             0x00 // Heater off
#define USR_HEATER_MASK            0x04 // Mask for Heater bit(2) in user reg.
#define USR_REG_MASK               0x38 // Mask off reserved bits (3,4,5)
#define USR_EOB_ON                 0x40 // End of battery
#define USR_EOB_MASK               0x40 // Mask for EOB bit(6) in user reg.
#define USR_REG_RES_MASK           0x7E // Only change bits 0 and 7 (meas. res.)
// Measurement Resolution 
#define USR_RES_12_14BIT           0x00 // RH=12bit, T=14bit
#define USR_RES_8_12BIT            0x01 // RH= 8bit, T=12bit
#define USR_RES_10_13BIT           0x80 // RH=10bit, T=13bit
#define USR_RES_11_11BIT           0x81 // RH=11bit, T=11bit
#define USR_RES_MASK               0x81 // Mask for res. bits (7,0) in user reg.

const uint16_t POLYNOMIAL = 0x131;

/* ------------------------------------------------------------------------------------------------
*                                           Local Variables
* -------------------------------------------------------------------------------------------------
*/
static uint8_t usr;                       // Keeps user register value
static bool  success;                     // Return Flag


/* ------------------------------------------------------------------------------------------------
*                                           Private Functions
* -------------------------------------------------------------------------------------------------
*/

static uint8_t SHT_CheckCRC(uint8_t data[], uint8_t nBytes, uint8_t checksum);

static bool SHT_Read_Cmd(uint8_t *pBuf,uint8_t nBytes);
static bool SHT_Write_Cmd(uint8_t cmd);

static double SHT_Humidity_Convert(uint16_t rawH);
static double SHT_Temperature_Convert(uint16_t rawT);

static int16_t SHT_Temp_Read(void); 
static int16_t SHT_Humid_Read(void);

/**************************************************************************************************
* @fn          SHT_CheckCRC
*
* @brief       CRC Verification
*
* @param       data[] - read from sensor
*			   nBytes - number of Bytes
*			   checksum
*
* @return      TRUE if the command has been transmitted successfully
**************************************************************************************************/
static uint8_t SHT_CheckCRC(uint8_t data[], uint8_t nBytes, uint8_t checksum)
//==============================================================================
{
  uint8_t crc = 0;	
  uint8_t byteCtr;
  uint8_t checksum_error;
  //calculates 8-Bit checksum with given polynomial
  for (byteCtr = 0; byteCtr < nBytes; ++byteCtr)
  { crc ^= (data[byteCtr]);
    for (uint8_t bit = 8; bit > 0; --bit)
    { if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      else crc = (crc << 1);
    }
  }
  if (crc != checksum) return checksum_error;
  else return 0;
}

/**************************************************************************************************
* @fn          halHumiWriteCmd
*
* @brief       Write a command to the humidity sensor
*
* @param       cmd - command to write
*
* @return      TRUE if the command has been transmitted successfully
**************************************************************************************************/
static bool SHT_Write_Cmd(uint8_t cmd)
{
  /* Send command */
  return I2CWriteBytes(HAL_SHT21_I2C_ADDRESS, cmd);
}

/**************************************************************************************************
* @fn          SHT_Read_Cmd
*
* @brief       This function implements the I2C protocol to read from the SHT21.
*
* @param       pBuf - pointer to buffer to place data
*
* @param       nBytes - number of bytes to read
*
* @return      TRUE if the required number of bytes are received
**************************************************************************************************/
static bool SHT_Read_Cmd(uint8_t *pBuf, uint8_t nBytes)
{
  /* Read data */
  return I2CReadBytes(HAL_SHT21_I2C_ADDRESS, pBuf, nBytes);
}



/**************************************************************************************************
* @fn          SHT_Temperature/Humidity_Read()
*
* @brief       This function implements procedure to measure temperature/humidity
*              1 - Write command to start measuring. Delay. 2 - Read raw data. 3 - Convert to physical value
*			   Delay for 11_11BITS = 15ms minimum, 12_14BITS = 22ms minimum (specs for more details)
*
* @param       pBuf - pointer to buffer to place data
*
* @param       nBytes - number of bytes to read
*
* @return      TRUE if the required number of bytes are received
**************************************************************************************************/

static double SHT_Temperature_Convert(uint16_t rawT)
{
    double v;
	rawT &= ~0x0003;
    //-- calculate temperature [�C] --
    v = -46.85 + 175.72/65536 *(double)rawT;

    return v;
}
static double SHT_Humidity_Convert(uint16_t rawH)
{
    double v;

    rawH &= ~0x0003; // clear bits [1..0] (status bits)
    //-- calculate relative humidity [%RH] --
    v = -6.0 + 125.0/65536 * (double)rawH; // RH= -6 + 125 * SRH/2^16

    return v;
}


 static int16_t SHT_Temp_Read(void)												//Temperature measuring step
{
	
	uint8_t buf[3];
	uint8_t checksum_error = 0;
	
	success = SHT_Write_Cmd(SHT21_CMD_TEMP_T_NH);
	nrf_delay_ms(20);
	
	if (success)
	{
		success = SHT_Read_Cmd(buf, DATA_LEN);
		checksum_error   = SHT_CheckCRC(buf, 2, buf[2]);
		if (!checksum_error)
		{
			uint16_t raw_T    =  (buf[0] << 8) | buf[1] ;	
			double cvt_value = SHT_Temperature_Convert(raw_T);
			return (int16_t) (cvt_value*10);
		}
	}
	return 0;
}

static int16_t SHT_Humid_Read(void)												//Humididty measuring step
{
	
	uint8_t buf[3];
	uint8_t checksum_error = 0;
	
	success = SHT_Write_Cmd(SHT21_CMD_HUMID_T_NH);
	nrf_delay_ms(20);
	
	if (success)
	{
		success = SHT_Read_Cmd(buf, DATA_LEN);
		checksum_error   = SHT_CheckCRC(buf, 2, buf[2]);
		if (!checksum_error)
		{
		uint16_t raw_H    =  (buf[0] << 8) | buf[1] ;	
		double cvt_value = SHT_Humidity_Convert(raw_H);
		return (int16_t) (cvt_value*10);
		}
	}
	return 0;
}
/* ------------------------------------------------------------------------------------------------
*                                           Public functions
* -------------------------------------------------------------------------------------------------
*/

/**************************************************************************************************
* @fn          vt_sensor_humidity_init
*
* @brief       Initialise the humidity sensor driver
*
* @return      TRUE if process is completed
*/
bool vt_sensor_humidity_init()
{
  // Set measurement resolution
  
  I2CReadRegister(HAL_SHT21_I2C_ADDRESS, SHT21_CMD_READ_U_R,&usr,1);        //Read User Register to verify connection
  usr &= USR_REG_RES_MASK; 													
  usr |= USR_RES_11_11BIT; 													//Resolution selection
  //user = BB for 11 bit mode
  I2CWriteRegister(HAL_SHT21_I2C_ADDRESS, SHT21_CMD_WRITE_U_R,&usr,1);		//Write mode to User Register
  
  success = false;
  return (!success);
}


/**************************************************************************************************
* @fn          vt_sensor_humidity_read
*
* @brief       Read humidity and ambience temperature value from sensor SHT21
*
* @param       16bits pointer - pointer to buffer place data multiplied 10 
*
* @return      TRUE if process is completed
*/

bool vt_sensor_humidity_read(int16_t *p_temp, int16_t *p_humid)
{
  *p_temp = SHT_Temp_Read();
  *p_humid= SHT_Humid_Read();
  if (*p_temp & *p_humid)
  {
	  return true;
  }
  else return false;
}

#endif
