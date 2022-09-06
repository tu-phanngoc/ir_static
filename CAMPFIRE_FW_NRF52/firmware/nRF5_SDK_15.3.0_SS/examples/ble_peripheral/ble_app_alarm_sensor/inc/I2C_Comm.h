/*****************************************************************************
* File Name: I2C_Comm.h
*
* Version 1.00
*
* Description:
*  This file contains the low level i2c function prototypes specific to 
*  CY8CMBR3116 device.
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
#if !defined(LOW_LEVEL_I2C_ROUTINES_H) 
#define LOW_LEVEL_I2C_ROUTINES_H 

/*****************************************************************************
* MACRO Definition
*****************************************************************************/
#define BYTE_WRITE_LEN 2
#define BURST_READ_LEN 1
#define PASS 1
#define FAIL 0
#define RESEND 2
#define FAULT_DELAY	200
#define WAIT 1
#define DONE 0


#include <stdbool.h>
#include <stdint.h>
/*****************************************************************************
* Data Type Definition
*****************************************************************************/

/* Slave Address (Default) */
#define SLAVE_ADDR				0x37
/*****************************************************************************
* Enumerated Data Definition
*****************************************************************************/
#define READ_BIT  (0x01)
#define WRITE_BIT (0x00)

/*****************************************************************************
* Data Struct Definition
*****************************************************************************/


/*****************************************************************************
* Global Variable Declaration
*****************************************************************************/



/*****************************************************************************
* Function Prototypes
*****************************************************************************/
bool   I2CReadRegister(uint8_t AddSlv, uint8_t AddReg, uint8_t *pData, uint8_t nBytes);
bool   I2CWriteRegister(uint8_t AddSlv, uint8_t AddReg, uint8_t *pData, uint8_t nBytes);
bool   I2CReadBytes(uint8_t AddSlv, uint8_t *pData, uint8_t nBytes);
bool   I2CWriteBytes(uint8_t AddSlv, uint8_t cmd);

/*****************************************************************************
* External Function Prototypes
*****************************************************************************/
/* Write commands */
extern unsigned char BurstWrite(unsigned char *, unsigned char);
extern unsigned char ByteWrite(unsigned char, unsigned char);

/* Read commands */
extern unsigned char ByteRead(unsigned char addrOffset,unsigned char *value);
extern unsigned char BurstRead(unsigned char addrOffset, unsigned char *readBuffer,unsigned char dataSize);

/* Writing control commands */
extern void WriteControlCommand(unsigned char);

#endif /* LOW_LEVEL_I2C_ROUTINES_H */

/* [] END OF FILE */
