#if(1)
#include "IR_Interface.h"
#include "IR_Common.h"
#include "IR_HAL.h"
#include "ampm_gt999.h"
#include "app_mesh.h"
#include "nrf_log.h"
#include "app_util_platform.h"

#include "IR_DataConverter_Funiki.h"

/*******************************/
/* 1. Array to store IR interval */
/* */
static int s_IrEnableTx;

/* 2. Array to store IR Byte protocol */
static uint8_t s_SampleIRByteFrame[11] = {0x28, 0x0A,0x00, 0x60, 0x00 , 0x02, 0x00, 0x00, 0x48, 0xDC , 0xFF};
/*******************************/

static uint8_t IsNumberInRange(uint8_t number,uint8_t min, uint8_t max);
static uint8_t GetValueFromBitPos(uint8_t u8Num, uint8_t LsbPos, uint8_t MsbPos);

/* Function 0.1 */
static uint8_t IsNumberInRange(uint8_t number,uint8_t min, uint8_t max)
{
	if(number >= min && number <= max)
		return 1;
	else
		return 0;
}

/* Function 0.2 */
static uint8_t GetValueFromBitPos(uint8_t u8Num, uint8_t LsbPos, uint8_t MsbPos)
{	
	uint8_t ret = u8Num >> LsbPos;
	
	MsbPos = MsbPos - LsbPos;
	
	switch(MsbPos)
	{
		case 0:
			ret = ret & 0x01;
			break;
		case 1:
			ret = ret & 0x03;
			break;
		case 2:
			ret = ret & 0x07;
			break;
		case 3:
			ret = ret & 0x0F;
			break;
		case 4:
			ret = ret & 0x1F;
			break;
		case 5:
			ret = ret & 0x3F;
			break;
		case 6:
			ret = ret & 0x7F;
			break;
		case 7:
			ret = ret & 0xFF;
			break;
		default:
			return 0;
			break;
	}
	return ret;
}

/* Function 1.1 */
static void IR_EncodeBitToRawFrame(uint8_t* input, int16_t* i16Output) {
	uint32_t TxBitIndex = 0;
	i16Output[0] = LONG_MARK_FUNIKI;
	i16Output[1] = -LONG_SPACE_FUNIKI;
	i16Output[2] = MARK_FUNIKI;
	TxBitIndex = 3;
	for(int16_t i = 0; i < DATASET_MAX_INDEX_FUNIKI; i++)
	{
		if(input[i] == 1)
		{
			i16Output[TxBitIndex] = -ONE_SPACE_FUNIKI;
			i16Output[TxBitIndex + 1] = MARK_FUNIKI;
		}
		else if(input[i] == 0)
		{
			i16Output[TxBitIndex] = -ZERO_SPACE_FUNIKI;
			i16Output[TxBitIndex + 1] = MARK_FUNIKI;
		}
		TxBitIndex = TxBitIndex + 2;
	}	
	SetIrTxState_Funiki(1);
}

/* Function 1.2 */
static void IR_EncodeByteToBitForm_Funiki(uint8_t* input, uint8_t* i16Output){
	uint32_t TxBitIndex = 0;
	uint32_t TxRaw8Bit[8];
	uint8_t EncodedByte = 0;
	for(uint32_t i = 0 ; i < 33; i++) {
		EncodedByte = input[i];
		TxRaw8Bit[0] = (EncodedByte >> 0) & 0x01;
		TxRaw8Bit[1] = (EncodedByte >> 1) & 0x01;
		TxRaw8Bit[2] = (EncodedByte >> 2) & 0x01;
		TxRaw8Bit[3] = (EncodedByte >> 3) & 0x01;
		TxRaw8Bit[4] = (EncodedByte >> 4) & 0x01;
		TxRaw8Bit[5] = (EncodedByte >> 5) & 0x01;
		TxRaw8Bit[6] = (EncodedByte >> 6) & 0x01;
		TxRaw8Bit[7] = (EncodedByte >> 7) & 0x01;
		
		for(uint8_t i = 0; i<8; i++)
		{
			i16Output[TxBitIndex + i] = TxRaw8Bit[i];
		}	
		TxBitIndex = TxBitIndex + 8;
	}
}

/* Function 1.3 */
static void IR_EncodeUserCmdToByteFrame(uint8_t* input, uint8_t* output) {
	/* 1. Set ON/OFF config */
	if(input[0] == 0x00)
	{
		NRF_LOG_INFO("=======>>> OFF");
		s_SampleIRByteFrame[0] &= ~(1<<3);
	}
	else if(input[0] == 0x01)
	{
		NRF_LOG_INFO("===>>> ON");

		s_SampleIRByteFrame[0] |= (1<<3);
	}
	
	/* 2. Set Temperature values */
	if(input[1] >= 32)
	{
		input[1] = 32;
	}
	NRF_LOG_INFO("TEMP");
	s_SampleIRByteFrame[1] = s_SampleIRByteFrame[1] & 0xF0;
	s_SampleIRByteFrame[1] |= (input[1] - 16);
	
	/* 3. Set Speed values */
	NRF_LOG_INFO("Fan speed");
	s_SampleIRByteFrame[0] &= 0x0F;
	s_SampleIRByteFrame[0] |= input[2] << 4;
	
	
	/* 4. Set SWING values */
	
	/* 5. Set MODE values */
	
	for(uint8_t i = 0; i < FRAME_BYTE_LEN; i++){
		output[i] = s_SampleIRByteFrame[i];
	}
}

/* Function 1.4 */
/* Get values from BLE command then fill to IR protocol */
void IR_EncodeUserCmdToIRProtocol_Funiki(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
	uint8_t txIRByte[48];
	uint8_t txBit[512];
	IR_EncodeUserCmdToByteFrame(InputBleCommands, txIRByte);
	IR_EncodeByteToBitForm_Funiki(txIRByte, txBit);
	IR_EncodeBitToRawFrame(txBit, OutputIRProtocol);
}

/* */
/* */
/* DECODE PART */
/* */
/* */

/* Function 2.1 */
static uint32_t IR_DecodeRawFrameToBit_Funiki(int16_t* input, uint8_t* output) {
	uint32_t index = 0;
	for(uint32_t i = 3; i < DATASET_MAX_INDEX_FUNIKI ; i++) {
		int32_t u32Interval = input[i];
		if(u32Interval < 0) {
			if(u32Interval > -800) {
				/* u32Interval ~ -400 -> -500us   --->>> set bit = 0 */
				output[index] = 0;
			}
			else {
				/* u32Interval ~ -1100 -> -1500us --->>> set bit = 1 */
				output[index] = 1;
			}
			index++;
		}
	}
	
	for(uint32_t i = index; i < DATASET_MAX_INDEX_FUNIKI ; i++) {
		output[i] = 0;
	}
	return index;
}

/* Function 2.2 */
static void IR_DecodeBitToByteFrame(uint8_t* input, uint8_t* output, uint32_t idx) {
	uint32_t index = idx;
	uint8_t byteIdx = 0;
	uint8_t decodeByte = 0;
	uint8_t temp[8];
	for(uint32_t i = 0; i < index; i = i+8) {
		temp[0] = input[i];
		temp[1] = input[i+1];
		temp[2] = input[i+2];
		temp[3] = input[i+3];
		temp[4] = input[i+4];
		temp[5] = input[i+5];
		temp[6] = input[i+6];
		temp[7] = input[i+7];
		for(uint32_t j = 0; j < 8; j++) {
			decodeByte = decodeByte | (temp[j] << j);
		}
		output[byteIdx++] = decodeByte & 0xff;
		decodeByte = 0;
	}
	
	for(uint32_t i = 0; i < 11 ; i++) {
		s_SampleIRByteFrame[i] = output[i];
	}	
}

/* Function 2.3 */
/* Get values from IR protocol then fill to BLE command */
static void IR_DecodeByteToUserCmd(uint8_t* IR_Bytes, uint8_t* BLEData)
{
	uint8_t val = 0;
	/* 0. Set byte 0 */
	/* ON/OFF */
	/* If byte[11] == 19, control ON/OFF */
	val = GetValueFromBitPos(IR_Bytes[0], 3, 3);
	BLEData[0] = val;
	
	/* 1. Set byte 1 */
	/* CONTROL TEMPERATURE */
	/* Value = setting temperature * 4 */
	val = GetValueFromBitPos(IR_Bytes[1], 0, 3);
	BLEData[1] = val + 16;
	
	/* 2. Set byte 2 */
	/* WIND LEVEL */
	val = GetValueFromBitPos(IR_Bytes[0], 4, 7);
	BLEData[2] = val;
	
	
	/* 3. Set byte 3 */
	/* AUTO/MANUAL SWING */
	
	/* 4. Set byte 4 */
	/* SET AIRCONDITIONER MODE */
}

/* Function 2.4 */
void IR_DecodeRawFrameToUserCmd_Funiki(int16_t* input, uint8_t* output) {
	DEBUG_PRINT("IR_DecodeRawFrameToUserCmd_Funiki");
	uint8_t irBitFrame[640];
	uint8_t rxByteFrame[48];
	
	uint32_t idx = IR_DecodeRawFrameToBit_Funiki(input, irBitFrame);
	IR_DecodeBitToByteFrame(irBitFrame, rxByteFrame, idx);
	IR_DecodeByteToUserCmd(rxByteFrame, output);	
}


/* Function 3.1 */
int IsEnableIrTx_Funiki(void)
{
	return s_IrEnableTx;
}

/* Function 3.2 */
void SetIrTxState_Funiki(int state)
{
	s_IrEnableTx = state;
}

uint8_t* GetByteFromFrame_Funiki() 
{
	return s_SampleIRByteFrame;
}

#endif