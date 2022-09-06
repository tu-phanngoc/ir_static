#if(1)
#include "IR_Interface.h"
#include "IR_HAL.h"
#include "ampm_gt999.h"
#include "app_mesh.h"
#include "nrf_log.h"
#include "app_util_platform.h"

#include "IR_DataConverter-Toshiba.h"
#include "app_ac_status.h"
/*******************************/
/* 1. Array to store IR interval */
/*******************************/
static int s_IrEnableTx;
/*******************************/

/* 2. Array to store IR Byte protocol */
/* Daikin IR protocol contains 3 frames */
static uint8_t IRFirstFrame[5] = {0xF2, 0x0D, 0x03, 0xFC, 0x01};
static uint8_t IRSecondFrame[4] = {0x00, 0x40, 0x00, 0x41};
static uint8_t s_SampleIRByteFrame[48] = 	{0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x01, 0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x01};

static uint8_t s_SwingCmdByteFrame[14] = 	{0xF2, 0x0D, 0x01, 0xFE, 0x21, 0x04, 0x25, 0xF2, 0x0D, 0x01, 0xFE, 0x21, 0x04, 0x25};
	
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
	i16Output[0] = TOSHIBA_LONG_MARK;
	i16Output[1] = -TOSHIBA_LONG_SPACE;
	i16Output[2] = TOSHIBA_MARK;
	TxBitIndex = 3;
	for(int16_t i = 0; i < 72; i++)
	{
		if(input[i] == 1)
		{
			i16Output[TxBitIndex] = -TOSHIBA_ONE_SPACE;
			i16Output[TxBitIndex + 1] = TOSHIBA_MARK;
		}
		else if(input[i] == 0)
		{
			i16Output[TxBitIndex] = -TOSHIBA_ZERO_SPACE;
			i16Output[TxBitIndex + 1] = TOSHIBA_MARK;
		}
		TxBitIndex = TxBitIndex + 2;
	}

//	NRF_LOG_INFO("[start frame 2 at idx: %d] ", TxBitIndex);

	i16Output[147] = -TOSHIBA_VERY_LONG_SPACE;
	i16Output[148] = TOSHIBA_LONG_MARK;
	i16Output[149] = -TOSHIBA_LONG_SPACE;
	i16Output[150] = TOSHIBA_MARK;
	TxBitIndex = TxBitIndex + 4;
	for(int16_t i = 0; i < 72; i++)
	{
		if(input[i] == 1)
		{
			i16Output[TxBitIndex] = -TOSHIBA_ONE_SPACE;
			i16Output[TxBitIndex + 1] = TOSHIBA_MARK;
		}
		else if(input[i] == 0)
		{
			i16Output[TxBitIndex] = -TOSHIBA_ZERO_SPACE;
			i16Output[TxBitIndex + 1] = TOSHIBA_MARK;
		}
		TxBitIndex = TxBitIndex + 2;
	}

//	NRF_LOG_INFO("[End frame 2 at idx: %d] ", TxBitIndex);
	SetIrTxState_Toshiba(1);
}


/* Function 1.2 */
static void IR_EncodeByteToBitForm_Toshiba(uint8_t* input, uint8_t* i16Output){
	uint32_t TxBitIndex = 0;
	uint32_t TxRaw8Bit[8];
	uint8_t EncodedByte = 0;
	for(uint32_t i = 0 ; i < 18; i++) {
		EncodedByte = input[i];
		/* Bit order in Toshiba is reversed with Hitachi */
		TxRaw8Bit[7] = (EncodedByte >> 0) & 0x01;
		TxRaw8Bit[6] = (EncodedByte >> 1) & 0x01;
		TxRaw8Bit[5] = (EncodedByte >> 2) & 0x01;
		TxRaw8Bit[4] = (EncodedByte >> 3) & 0x01;
		TxRaw8Bit[3] = (EncodedByte >> 4) & 0x01;
		TxRaw8Bit[2] = (EncodedByte >> 5) & 0x01;
		TxRaw8Bit[1] = (EncodedByte >> 6) & 0x01;
		TxRaw8Bit[0] = (EncodedByte >> 7) & 0x01;
		
		for(uint8_t i = 0; i<8; i++)
		{
			i16Output[TxBitIndex + i] = TxRaw8Bit[i];
		}	
		TxBitIndex = TxBitIndex + 8;
	}
}


/* Function 1.3 */
static void IR_EncodeUserCmdToByteFrame(uint8_t* input, uint8_t* output) {
	/* 1. Set Temperature values */
	/* Byte[6]: 0TTTTTT0*/
	if(input[1] > 0)
	{
//		NRF_LOG_INFO("TEMP");
		/* TODO: is input[1] > 17?, Thanh */
		if(s_SampleIRByteFrame[5] != ((input[1] - 17) << 4)) 
		{
			/* Set ON */
			s_SampleIRByteFrame[6] &= ~(0x07);
			
			/* Set Temperature */
			s_SampleIRByteFrame[5] = (input[1] - 17) << 4;
		}
	}

	/* 2. Set ON/OFF config */
	if(input[0] == 0x00)
	{
		NRF_LOG_INFO("=======>>> OFF");
//		
		s_SampleIRByteFrame[6] = 0x07;
	}
	else if(input[0] == 0x01)
	{
		NRF_LOG_INFO("===>>> ON");
		s_SampleIRByteFrame[6] &= ~(0x07);			//Clear bit 3
	}
	
	
	/* 3. Set Speed values */
	if(input[2] > 0)
	{
//		NRF_LOG_INFO("SPEED");
		
		s_SampleIRByteFrame[6] = s_SampleIRByteFrame[6]  & 0x1F;
		s_SampleIRByteFrame[6] |= input[2] << 5;
	}
	
	/* 4. Set SWING values */
	if(input[3] > 0)
	{
//		NRF_LOG_INFO("SWING");
	}
	
	/* 5. Set MODE values */
//	if(InputBleCommands[4] > 0)
//	if(input[4] >= 0x01)
	if(input[0] == 0x01)
	{
//		NRF_LOG_INFO("MODE");
		s_SampleIRByteFrame[6] = s_SampleIRByteFrame[6] & 0xF0;
		s_SampleIRByteFrame[6] |= input[4];
	}
	
	s_SampleIRByteFrame[8] = 0;
	for(uint8_t i = 0; i<8; i++)
	{
		s_SampleIRByteFrame[8] = s_SampleIRByteFrame[8] ^ s_SampleIRByteFrame[i];
	}
	
	s_SampleIRByteFrame[8] = s_SampleIRByteFrame[8] & 0xFF;
	
	for(uint8_t i = 0; i<SHORT_FRAME_BYTE_LEN; i++)
	{
		s_SampleIRByteFrame[SHORT_FRAME_BYTE_LEN+i] = s_SampleIRByteFrame[i];
	}
		
	
	for(uint8_t i = 0; i<SHORT_FRAME_BYTE_LEN*2; i++)
	{
		output[i] = s_SampleIRByteFrame[i];
	}

}



/* Function 1.4 */
/* Get values from BLE command then fill to IR protocol */
void IR_EncodeUserCmdToIRProtocol_Toshiba(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
{
	uint8_t txIRByte[48];
	uint8_t txBit[512];

	/* Set AC params */
	//1. Power
	InputBleCommands[0] = ac_status.power_status;
	//2. Temperature
	InputBleCommands[1] = ac_status.temperature + 16;
	//3. Mode
	if(ac_status.mode == AC_MODE_AUTO)
	{
		InputBleCommands[4] = 0;
	}
	else if(ac_status.mode == AC_MODE_DRY)
	{
		InputBleCommands[4] = 2;
	}
	else if(ac_status.mode == AC_MODE_FAN_ONLY)
	{
		InputBleCommands[4] = 4;
	}
	else if(ac_status.mode == AC_MODE_COOL)
	{
		InputBleCommands[4] = 1;
	}
	
	if(ac_status.fan == AC_FAN_LVL_AUTO)
	{
		InputBleCommands[2] = 0;
	}
	else if(ac_status.fan == AC_FAN_LVL_1)
	{
		InputBleCommands[2] = 1;
	}
	else if(ac_status.fan == AC_FAN_LVL_2)
	{
		InputBleCommands[2] = 2;
	}
	else if(ac_status.fan == AC_FAN_LVL_3)
	{
		InputBleCommands[2] = 3;
	}
	else if(ac_status.fan == AC_FAN_LVL_4)
	{
		InputBleCommands[2] = 4;
	}
	else if(ac_status.fan == AC_FAN_LVL_5)
	{
		InputBleCommands[2] = 5;
	}
	
	
//	for(uint32_t i = 0; i < 8 ; i++) {
//		DEBUG_PRINT("%d ", InputBleCommands[i]);
//	}

	IR_EncodeUserCmdToByteFrame(InputBleCommands, txIRByte);
	
//	DEBUG_PRINT("Byte frame: ");
//	
//	for(uint32_t i = 0; i < 18 ; i++) {
//		DEBUG_PRINT("%d ", txIRByte[i]);
//	}

	IR_EncodeByteToBitForm_Toshiba(txIRByte, txBit);
	IR_EncodeBitToRawFrame(txBit, OutputIRProtocol);

//	for(uint32_t i = 0; i < 294 ; i++) {
//		DEBUG_PRINT("%d ", OutputIRProtocol[i]);
//	}
//	
}


/* Function 2.1 */
static uint32_t IR_DecodeRawFrameToBit_Toshiba(int16_t* input, uint8_t* output) {
	uint32_t index = 0;
	for(uint32_t i = 2; i < 146 ; i++) {
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
	for(uint32_t i = index; i < DATASET_MAX_INDEX_TOSHIBA ; i++) {
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
		/* Bit order in Toshiba is reversed with Hitachi */
		temp[7] = input[i];
		temp[6] = input[i+1];
		temp[5] = input[i+2];
		temp[4] = input[i+3];
		temp[3] = input[i+4];
		temp[2] = input[i+5];
		temp[1] = input[i+6];
		temp[0] = input[i+7];

		for(uint32_t j = 0; j < 8; j++) {
			decodeByte = decodeByte | (temp[j] << j);
		}
		output[byteIdx++] = decodeByte & 0xff;
		decodeByte = 0;
	}
	
	for(uint32_t i = 0; i < 9 ; i++) {
		s_SampleIRByteFrame[i] = output[i];
	}
	for(uint32_t i = 0; i < 9 ; i++) {
		s_SampleIRByteFrame[i + 9] = s_SampleIRByteFrame[i];
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
	val = GetValueFromBitPos(IR_Bytes[6], 2, 2);
	if(val == 1)
	{
		BLEData[0] = 0;
	}
	else
	{
		BLEData[0] = 1;
	}
	
	/* 1. Set byte 1 */
	/* CONTROL TEMPERATURE */
	/* Value = setting temperature * 4 */
	BLEData[1] = GetValueFromBitPos(IR_Bytes[5], 4, 7) + 17;
	/* 2. Set byte 2 */
	/* WIND LEVEL */
	val = GetValueFromBitPos(IR_Bytes[6], 5, 7);
	BLEData[2] = val;
	/* 3. Set byte 3 */
	/* AUTO/MANUAL SWING */
//	if(IR_Bytes[11] == 129)
//	{
//		BLEData[3] = 1;
//	}
//	else
//	{
//		BLEData[3] = 0;
//	}
	
	/* 4. Set byte 4 */
	/* SET AIRCONDITIONER MODE */
	val = GetValueFromBitPos(IR_Bytes[6], 0, 3);
	BLEData[4] = val;
}


/* Function 2.4 */
void IR_DecodeRawFrameToUserCmd_Toshiba(int16_t* input, uint8_t* output) {
//	DEBUG_PRINT("IR_DecodeRawFrameToUserCmd_Toshiba!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	uint8_t irBitFrame[640];
	uint8_t rxByteFrame[48];
	uint32_t idx = IR_DecodeRawFrameToBit_Toshiba(input, irBitFrame);
	IR_DecodeBitToByteFrame(irBitFrame, rxByteFrame, idx);
	IR_DecodeByteToUserCmd(rxByteFrame, output);	
	
	/* Set AC params */
	//1. Power
	ac_control_set_power_status(output[0]);
	//2. Temperature
	ac_control_set_temperature(output[1] - 16);
	//3. Mode
	if(output[4] == 0)
	{
		ac_control_set_mode(AC_MODE_AUTO);
	}
	else if(output[4] == 1)
	{
		ac_control_set_mode(AC_MODE_COOL);
	}
	else if(output[4] == 2)
	{
		ac_control_set_mode(AC_MODE_DRY);
	}	
	else if(output[4] == 4)
	{
		ac_control_set_mode(AC_MODE_FAN_ONLY);
	}
	
	//4. Fan
	if(output[2] == 0)
	{
		ac_control_set_fan(AC_FAN_LVL_AUTO);
	}
	else if(output[2] == 1)
	{
		ac_control_set_fan(AC_FAN_LVL_1);
	}
	else if(output[2] == 2)
	{
		ac_control_set_fan(AC_FAN_LVL_2);
	}
	else if(output[2] == 3)
	{
		ac_control_set_fan(AC_FAN_LVL_3);
	}
	else if(output[2] == 4)
	{
		ac_control_set_fan(AC_FAN_LVL_4);
	}
	else if(output[2] >=5)
	{
		ac_control_set_fan(AC_FAN_LVL_5);
	}
	
//	ac_control_set_swing(BLEData[3]);
	ac_control_update_status_to_payload();
}

/* Function 3.1 */
int IsEnableIrTx_Toshiba(void)
{
	return s_IrEnableTx;
}

/* Function 3.2 */
void SetIrTxState_Toshiba(int state)
{
	s_IrEnableTx = state;
}

uint8_t* GetByteFromFrame_Toshiba() 
{
	return s_SampleIRByteFrame;
}

#endif