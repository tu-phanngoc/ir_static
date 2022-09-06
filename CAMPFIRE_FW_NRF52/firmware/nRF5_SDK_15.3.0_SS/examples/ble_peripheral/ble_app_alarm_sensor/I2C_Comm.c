/*****************************************************************************
* File Name: I2C_Comm.c
*
* Version 1.00
*
* Description:
*   This file contains the low level i2c functions specific to CY8CMBR3116
*   device.
*
* Related Document:
* 	CY3280-MBR3 User Guide
*   MBR3 (Street Fighter) Device Datasheet
*
* Hardware Dependency:
* 	CY8CKIT-042 Pioneer Kit and CY3280-MBR3 Evaluation Kit
*
* Code Tested With:
* 	Creator 3.0
*	CY8CKIT-042
*
******************************************************************************
* Copyright (2013), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/
#if (defined(BOARD_SHT2x))
#include "twi_master.h"
#include "I2C_Comm.h"

/*****************************************************************************
* Local Function Prototypes
*****************************************************************************/
static void SetOffset(unsigned char);
unsigned char CheckWriteStatus(void);
unsigned char CheckReadStatus(void);
/*****************************************************************************
* Global Function Prototypes
*****************************************************************************/
unsigned char BurstWrite(unsigned char *, unsigned char);
unsigned char ByteWrite(unsigned char, unsigned char);
void WriteControlCommand(unsigned char);
unsigned char ByteRead(unsigned char addrOffset,unsigned char *value);
unsigned char BurstRead(unsigned char addrOffset, unsigned char *readBuffer,unsigned char dataSize);

/*****************************************************************************
* Global Variable Declarations
*****************************************************************************/

/*******************************************************************************
* Function Name: SetOffset()
********************************************************************************
* Summary:
*  sets the address offset to read or write from the CY8CMBR3116 device
*
* Parameters:
*  addrOffset: Offset Address from which data has to be read or written.
*
* Return:
*  void
*
*******************************************************************************/
static void SetOffset(unsigned char addrOffset)
{
	unsigned char bufferArray[BYTE_WRITE_LEN];
	bufferArray[0] = addrOffset;
	BurstWrite(&bufferArray[0], 1);
	
}

/*******************************************************************************
* Function Name: BurstWrite()
********************************************************************************
* Summary:
*  Writing a burst of i2c data from a data array
*
* Parameters:
*  dataArray : Pointer to the data array from which data has to be written.
*  dataSize  : Number of bytes of array to be written to the register map.
*
* Return:
*  void
*
*******************************************************************************/
unsigned char BurstWrite(unsigned char * dataArray, unsigned char dataSize)
{
	return twi_master_transfer(SLAVE_ADDR << 1, &dataArray[0], dataSize, TWI_ISSUE_STOP);
}

/*******************************************************************************
* Function Name: ByteWrite()
********************************************************************************
* Summary:
*  Writing a byte of i2c data from a data array to a specific register address
*
* Parameters:
*  addrOffset: Offset Address from which data has to be read or written.
*  dataValue : Value to be written to the specified offset address
*
* Return:
*  void
*
*******************************************************************************/
unsigned char ByteWrite(unsigned char addrOffset, unsigned char dataValue)
{
	unsigned char bufferArray[BYTE_WRITE_LEN];
	
	bufferArray[0] = addrOffset;
	bufferArray[1] = dataValue;
	
	return (BurstWrite(&bufferArray[0], BYTE_WRITE_LEN));
}

/*******************************************************************************
* Function Name: WriteControlCommand()
********************************************************************************
* Summary:
*  Writing a command to CTRL_CMD register.
*
* Parameters:
*  cmdValue: Command to be written to CTRL_CMD register. Control commands MACROS
			 defined in configurations.h can be used here directly.
*
* Return:
*  void
*
*******************************************************************************/
//void WriteControlCommand(unsigned char cmdValue)
//{
//	unsigned char bufferArray[BYTE_WRITE_LEN];
//	
//	bufferArray[0] = CTRL_CMD;
//	bufferArray[1] = cmdValue;
//	
//	BurstWrite (&bufferArray[0], BYTE_WRITE_LEN);
//}

/*******************************************************************************
* Function Name: BurstRead()
********************************************************************************
* Summary:
*  Reading a burst of i2c data and storing it into a global data array
*
* Parameters:
*  addrOffset: Offset Address from which data has to be read or written.
*  dataSize  : Number of bytes of array to be read from the register map.
*
* Return:
*  void
*
*******************************************************************************/
unsigned char BurstRead(unsigned char addrOffset, unsigned char *readBuffer,unsigned char dataSize)
{

	if (twi_master_transfer(SLAVE_ADDR << 1, &addrOffset, 1, TWI_DONT_ISSUE_STOP) == true) 
		return twi_master_transfer(SLAVE_ADDR << 1 | TWI_READ_BIT, &readBuffer[0], dataSize, TWI_ISSUE_STOP);
	return false;
}


/*******************************************************************************
* Function Name: ByteRead()
********************************************************************************
* Summary:
*  Reading a byte of i2c data and storing it into a global data array
*
* Parameters:
*  addrOffset: Offset Address from which data has to be read or written.
*
* Return:
*  void
*
*******************************************************************************/
unsigned char ByteRead(unsigned char addrOffset,unsigned char *value)
{
	return BurstRead(addrOffset,value,BURST_READ_LEN);
}


//==============================================================================
// Project   :  Viettel Smart Home      
// File      :  i2c_sensor.c
// Author    :  VTSmart
// Brief     :  Wrapper of low level I2C for communicating with Sensor Registers
//==============================================================================
/* ------------------------------------------------------------------------------------------------
*                                          Includes
* ------------------------------------------------------------------------------------------------
*/

/* ------------------------------------------------------------------------------------------------
*                                           Local Variables
* ------------------------------------------------------------------------------------------------
*/
static uint8_t send_buf[24];

/**************************************************************************************************
 * @fn          I2CSensorReadReg
 *
 * @brief       This function implements the I2C protocol to read from a sensor. The sensor must
 *              be selected before this routine is called.
 *
 * @param 		AddSlv - which slave address
 * @param       AddReg - which register to read
 * @param       pData  - pointer to send_buf to place data
 * @param       nBytes - numbver of bytes to read
 *
 * @return      TRUE if the required number of bytes are received
 **************************************************************************************************/
bool I2CReadRegister(uint8_t AddSlv, uint8_t AddReg, uint8_t *pData, uint8_t nBytes)
{
  uint8_t i = 0;
  // Initialize data to zero so we don't return random values.
  for (i = 0; i < nBytes; i++)
  {
    pData[i] = 0; 
  }

  /* Send address we're reading from */
  if (twi_master_transfer( AddSlv << 1, &AddReg, 1, TWI_DONT_ISSUE_STOP))
  {
    /* Now read data */
    return twi_master_transfer( AddSlv << 1 | READ_BIT, &pData[0], nBytes, TWI_ISSUE_STOP);
  }
  return false;
}

/**************************************************************************************************
* @fn          I2CSensorWriteReg
* @brief       This function implements the I2C protocol to write to a sensor. 
*
* @param	   AddSlv - which slave address
* @param       AddReg - which register to write
* @param       pData  - pointer to send_buf containing data to be written
* @param       nBytes - number of bytes to write
*
* @return      TRUE if successful write
*/
bool I2CWriteRegister(uint8_t AddSlv, uint8_t AddReg, uint8_t *pData, uint8_t nBytes)
{
  uint8_t i= 0;
  uint8_t *p = send_buf;
  /* Copy address and data to local send_buf for burst write */
  *p++ = AddReg;
  for (i = 0; i < nBytes; i++)
  {
    *p++ = *pData++;
  }
  nBytes++;

  /* Send address and data */
  if (twi_master_transfer( AddSlv << 1, &send_buf[0], nBytes, TWI_ISSUE_STOP))
  {
	  return true;
  }
  return false;
}

/**************************************************************************************************
* @fn          I2CSensorReadCmd
* @brief       This function implements the I2C protocol to read data directly from sensor no register
*
* @param	   AddSlv - which slave address
* @param       pData  - pointer to send_buf containing data to be written
* @param       nBytes - number of bytes to write
*
* @return      TRUE if successful write
*/
bool   I2CReadBytes(uint8_t AddSlv, uint8_t *pData, uint8_t nBytes)
{
  /* Now read data */
  return twi_master_transfer( AddSlv << 1 | READ_BIT, &pData[0], nBytes, TWI_ISSUE_STOP);
}
/**************************************************************************************************
* @fn          I2CSensorWriteCmd
* @brief       This function implements the I2C protocol to write Command to a sensor no register
*
* @param	   AddSlv - which slave address
* @param       AddReg - which register to write
*
* @return      TRUE if successful write
*/
bool   I2CWriteBytes(uint8_t AddSlv, uint8_t cmd)
{
  return twi_master_transfer( AddSlv << 1, &cmd, 1, TWI_ISSUE_STOP);
}

/*********************************************************************
*********************************************************************/
#endif
/* [] END OF FILE */
