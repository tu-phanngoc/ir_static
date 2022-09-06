#if(1)
#include "IR_Interface.h"
#include "IR_Common.h"
#include "IR_HAL.h"
#include "ampm_gt999.h"
#include "app_mesh.h"
#include "nrf_log.h"
#include "app_util_platform.h"

#include "IR_DataConverter_Mitsubishi_2.h"
#include "app_ac_status.h"
/*******************************/
/* 1. Array to store IR interval */
/* */
static int s_IrEnableTx;

/* 2. Array to store IR Byte protocol */
static uint8_t s_SampleIRByteFrame[36] = {0x23, 0xCB, 0x26 , 0x01, 0x00, 0x20, 0x18, 0x09, 0x36, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCD};
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
	i16Output[0] = LONG_MARK_MITSUBISHI_2;
	i16Output[1] = -LONG_SPACE_MITSUBISHI_2;
	i16Output[2] = MARK_MITSUBISHI_2;
	TxBitIndex = 3;
	for(int16_t i = 0; i < 144; i++)
	{
		if(input[i] == 1)
		{
			i16Output[TxBitIndex] = -ONE_SPACE_MITSUBISHI_2;
			i16Output[TxBitIndex + 1] = MARK_MITSUBISHI_2;
		}
		else if(input[i] == 0)
		{
			i16Output[TxBitIndex] = -ZERO_SPACE_MITSUBISHI_2;
			i16Output[TxBitIndex + 1] = MARK_MITSUBISHI_2;
		}
		TxBitIndex = TxBitIndex + 2;
	}	
	
	/* Frame 2 */
	i16Output[291] = -LONG_SPACE_MITSUBISHI_2;
	i16Output[292] = LONG_MARK_MITSUBISHI_2;
	i16Output[293] = -LONG_SPACE_MITSUBISHI_2;
	i16Output[294] = MARK_MITSUBISHI_2;
	TxBitIndex = 295;
	
	for(int16_t i = 0; i < 144; i++)
	{
		if(input[i] == 1)
		{
			i16Output[TxBitIndex] = -ONE_SPACE_MITSUBISHI_2;
			i16Output[TxBitIndex + 1] = MARK_MITSUBISHI_2;
		}
		else if(input[i] == 0)
		{
			i16Output[TxBitIndex] = -ZERO_SPACE_MITSUBISHI_2;
			i16Output[TxBitIndex + 1] = MARK_MITSUBISHI_2;
		}
		TxBitIndex = TxBitIndex + 2;
	}	
	SetIrTxState_Mitsubishi_2(1);
}

/* Function 1.2 */
static void IR_EncodeByteToBitForm_Mitsubishi_2(uint8_t* input, uint8_t* i16Output) {
	NRF_LOG_INFO("IR_EncodeByteToBitForm_Mitsubishi_2");
	uint32_t TxBitIndex = 0;
	uint32_t TxRaw8Bit[8];
	uint8_t EncodedByte = 0;
	for(uint32_t i = 0 ; i < TOTAL_FRAME_BYTE_LEN_MITSUBISHI_2; i++) {
		EncodedByte = input[i];
		TxRaw8Bit[0] = (EncodedByte >> 0) & 0x01;
		TxRaw8Bit[1] = (EncodedByte >> 1) & 0x01;
		TxRaw8Bit[2] = (EncodedByte >> 2) & 0x01;
		TxRaw8Bit[3] = (EncodedByte >> 3) & 0x01;
		TxRaw8Bit[4] = (EncodedByte >> 4) & 0x01;
		TxRaw8Bit[5] = (EncodedByte >> 5) & 0x01;
		TxRaw8Bit[6] = (EncodedByte >> 6) & 0x01;
		TxRaw8Bit[7] = (EncodedByte >> 7) & 0x01;
		
		for(uint8_t j = 0; j<8; j++)
		{
			i16Output[TxBitIndex + j] = TxRaw8Bit[j];
		}	
		TxBitIndex = TxBitIndex + 8;
		
//		NRF_LOG_INFO("TxBitIndex: %d", TxBitIndex);
	}
}

/* Function 1.3 */
static void IR_EncodeUserCmdToByteFrame(uint8_t* input, uint8_t* output) {
	/* 2. Set ON/OFF config */
	if(input[0] == 0x00)
	{
		NRF_LOG_INFO("=======>>> OFF");
		
		s_SampleIRByteFrame[5] &= ~(1<<5);
	}
	else if(input[0] == 0x01)
	{
		NRF_LOG_INFO("===>>> ON");
		s_SampleIRByteFrame[5] |= (1<<5);
	}
	
	/* 1. Set Temperature values */
	if(input[1] > 0)
	{
		if(input[1] >= 32)
		{
			input[1] = 32;
		}
		
		NRF_LOG_INFO("TEMP");
		s_SampleIRByteFrame[7] &= 0xF0; 
		s_SampleIRByteFrame[7] |= (input[1] - 16);
	}
	
	s_SampleIRByteFrame[SHORT_FRAME_BYTE_LEN_MITSUBISHI_2 - 1] = 0; /* Prepare to calculate CRC */
	
	/* Calculate Checksum */
	for(uint8_t i = 0; i < SHORT_FRAME_BYTE_LEN_MITSUBISHI_2 - 1; i++)
	{
		s_SampleIRByteFrame[SHORT_FRAME_BYTE_LEN_MITSUBISHI_2 - 1] = s_SampleIRByteFrame[SHORT_FRAME_BYTE_LEN_MITSUBISHI_2 - 1] + s_SampleIRByteFrame[i];
	}
	/* Calculate Checksum end */
	
	/* Calculate Assign value from frame 1 to frame 2 */
	for(uint8_t i = 0; i < SHORT_FRAME_BYTE_LEN_MITSUBISHI_2; i++)
	{
		s_SampleIRByteFrame[SHORT_FRAME_BYTE_LEN_MITSUBISHI_2 + i] = s_SampleIRByteFrame[i];
	}
	
	

	for(uint8_t i = 0; i < TOTAL_FRAME_BYTE_LEN_MITSUBISHI_2; i++){
		output[i] = s_SampleIRByteFrame[i];
	}
}

/* Function 1.4 */
/* Get values from BLE command then fill to IR protocol */
void IR_EncodeUserCmdToIRProtocol_Mitsubishi_2(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
	uint8_t txIRByte[48];
	uint8_t txBit[512];
			/* Set AC params */
	//1. Power
	InputBleCommands[0] = ac_status.power_status;
	//2. Temperature
	InputBleCommands[1] = ac_status.temperature + 16;
	
	IR_EncodeUserCmdToByteFrame(InputBleCommands, txIRByte);
	uint8_t str[128];
	
	for(uint8_t i = 0; i < TOTAL_FRAME_BYTE_LEN_MITSUBISHI_2; i++){
		NRF_LOG_INFO("%02X", txIRByte[i]);
	}
//	NRF_LOG_INFO("%s", str);
	IR_EncodeByteToBitForm_Mitsubishi_2(txIRByte, txBit);
	IR_EncodeBitToRawFrame(txBit, OutputIRProtocol);
}

/* */
/* */
/* DECODE PART */
/* */
/* */

/* Function 2.1 */
static uint32_t IR_DecodeRawFrameToBit_Mitsubishi_2(int16_t* input, uint8_t* output) {
	uint32_t index = 0;
	for(uint32_t i = 3; i < 290 ; i++) {
		int32_t u32Interval = input[i];
		if(u32Interval < 0) {
			if(u32Interval > -700) {
				/* u32Interval ~ -400 -> -700us   --->>> set bit = 0 */
				output[index] = 0;
			}
			else if(u32Interval < -1000) {
				/* u32Interval ~ -1100 -> -1500us --->>> set bit = 1 */
				output[index] = 1;
			}
			index++;
		}
	}
	for(uint32_t i = index; i < 290 ; i++) {
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
	
	for(uint32_t i = 0; i < SHORT_FRAME_BYTE_LEN_MITSUBISHI_2 ; i++) {
		s_SampleIRByteFrame[i] = output[i];
	}	
	for(uint32_t i = 0; i < SHORT_FRAME_BYTE_LEN_MITSUBISHI_2 ; i++) {
		s_SampleIRByteFrame[i + SHORT_FRAME_BYTE_LEN_MITSUBISHI_2] = output[i];
	}	
}

/* Function 2.3 */
/* Get values from IR protocol then fill to BLE command */
static void IR_DecodeByteToUserCmd(uint8_t* IR_Bytes, uint8_t* BLEData)
{
	uint8_t val = 0;
	/* 0. ON/OFF -> BLEData[0]*/
	/* If byte[5], bit5 == 1 -> ON */
	/* If byte[5], bit5 == 0 -> OFF */
	
	val = GetValueFromBitPos(IR_Bytes[5], 5, 5);
	BLEData[0] = val;
	
	/* 1. TEMPERATURE -> BLEData[1] */
	/* Byte[7], bit0..3 */
	val = GetValueFromBitPos(IR_Bytes[7], 0, 3);
	BLEData[1] = val + 16;
	if(BLEData[1] > 32)
	{
		BLEData[1] = 32;
	}
	if(BLEData[1] < 16)
	{
		BLEData[1] = 16;
	}
	
	/* 4. MODE -> BLEData[4] */
	/* Byte[6], bit 3..5       */
	/* val == 0, mode AUTO     */
	val = GetValueFromBitPos(IR_Bytes[6], 3, 5);
	BLEData[4] = val;
	
	/* 2. FAN SPEED -> BLEData[2]*/
	val = GetValueFromBitPos(IR_Bytes[9], 0, 2);
	BLEData[2] = val;
	
	/* 3. SWING -> BLEData[3]*/
	val = GetValueFromBitPos(IR_Bytes[9], 3, 6);
	BLEData[3] = val;
}

/* Function 2.4 */
void IR_DecodeRawFrameToUserCmd_Mitsubishi_2(int16_t* input, uint8_t* output) {
	DEBUG_PRINT("IR_DecodeRawFrameToUserCmd_Mitsubishi_2");
	uint8_t irBitFrame[640];
	uint8_t rxByteFrame[48];
	
	uint32_t idx = IR_DecodeRawFrameToBit_Mitsubishi_2(input, irBitFrame);
	IR_DecodeBitToByteFrame(irBitFrame, rxByteFrame, idx);
	IR_DecodeByteToUserCmd(rxByteFrame, output);	
	
	//1. Power
	ac_control_set_power_status(output[0]);
	//2. Temperature
	ac_control_set_temperature(output[1] - 16);
	
	ac_control_update_status_to_payload();
}



/* Function 3.1 */
int IsEnableIrTx_Mitsubishi_2(void)
{
	return s_IrEnableTx;
}

/* Function 3.2 */
void SetIrTxState_Mitsubishi_2(int state)
{
	s_IrEnableTx = state;
}

uint8_t* GetByteFromFrame_Mitsubishi_2() 
{
	return s_SampleIRByteFrame;
}

#endif