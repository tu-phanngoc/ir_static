#if(1)
#include "IR_Interface.h"
#include "IR_Common.h"
#include "IR_HAL.h"
#include "ampm_gt999.h"
#include "app_mesh.h"
#include "nrf_log.h"
#include "app_util_platform.h"

#include "IR_DataConverter-Hitachi.h"
#include "app_ac_status.h"
/*******************************/
/* 1. Array to store IR interval */
/* */
static int s_IrEnableTx;

/* 2. Array to store IR Byte protocol */
static uint8_t s_inputBleCmd[24];
static uint8_t s_SampleIRByteFrame[48] = {1,16,0,64,191,255,0,204,51,146,   109,19,236,120,135,0,255,0,255,0,		255,0,255,0,255,21,234,209,46,0,   255,0,255};
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
	i16Output[0] = 3300;
	i16Output[1] = -1750;
	i16Output[2] = 400;
	TxBitIndex = 3;
	for(int16_t i = 0; i < 264; i++)
	{
		if(input[i] == 1)
		{
			i16Output[TxBitIndex] = -1200;
			i16Output[TxBitIndex + 1] = 450;
		}
		else if(input[i] == 0)
		{
			i16Output[TxBitIndex] = -400;
			i16Output[TxBitIndex + 1] = 450;
		}
		TxBitIndex = TxBitIndex + 2;
	}	
	SetIrTxState_Hitachi(1);
}

/* Function 1.2 */
static void IR_EncodeByteToBitForm_Hitachi(uint8_t* input, uint8_t* i16Output){
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


	/* 2. Set ON/OFF config */
	if(input[0] == 0x00)
	{
		NRF_LOG_INFO("=======>>> OFF");
		
		s_SampleIRByteFrame[11] = 19;
		s_SampleIRByteFrame[12] = 255 - s_SampleIRByteFrame[11];
		s_SampleIRByteFrame[13] = input[1] << 2;
		s_SampleIRByteFrame[14] = 255 - s_SampleIRByteFrame[13];
		s_SampleIRByteFrame[27] = 193;
		s_SampleIRByteFrame[28] = 255 - s_SampleIRByteFrame[27];
	}
	else if(input[0] == 0x01)
	{
		NRF_LOG_INFO("===>>> ON");

		if(input[1] >= 32)
		{
			input[1] = 32;
		}
		
		NRF_LOG_INFO("TEMP");
		if(s_SampleIRByteFrame[13] < (input[1] << 2)) 
		{
			s_SampleIRByteFrame[11] = 68;
			s_SampleIRByteFrame[12] = 0xFF - s_SampleIRByteFrame[11];
			s_SampleIRByteFrame[13] = input[1] << 2;
			s_SampleIRByteFrame[14] = 255 - s_SampleIRByteFrame[13];
		}
		else if(s_SampleIRByteFrame[13] > (input[1] << 2)) 
		{
			s_SampleIRByteFrame[11] = 67;
			s_SampleIRByteFrame[12] = 0xFF - s_SampleIRByteFrame[11];
			s_SampleIRByteFrame[13] = input[1] << 2;
			s_SampleIRByteFrame[14] = 255 - s_SampleIRByteFrame[13];
		}
		else
		{
			s_SampleIRByteFrame[11] = 19;
			s_SampleIRByteFrame[12] = 0xFF - s_SampleIRByteFrame[11];
		}

		s_SampleIRByteFrame[27] = 209;
		s_SampleIRByteFrame[28] = 255 - s_SampleIRByteFrame[27];
	}
	
	/* 1. Set Temperature values */
	if(input[1] > 0)
	{
		
	}
	/* 3. Set Speed values */
	if(input[2] > 0)
	{
		NRF_LOG_INFO("SPEED");
		s_SampleIRByteFrame[25] |= input[2] << 4; 
		s_SampleIRByteFrame[26] = 0xFF - s_SampleIRByteFrame[25];  
	}
	
	/* 4. Set SWING values */
//	if(input[3] > 0)
//	{
//		NRF_LOG_INFO("SWING");
//		s_SampleIRByteFrame[11] = 129;
//		s_SampleIRByteFrame[12] = 0xFF - s_SampleIRByteFrame[11];
//	}
	
	/* 5. Set MODE values */
	if(input[4] > 0)
	{
		NRF_LOG_INFO("MODE");
		s_SampleIRByteFrame[25] |= input[4]; 
		s_SampleIRByteFrame[26] = 0xFF - s_SampleIRByteFrame[25]; 
	}
	
	for(uint8_t i = 0; i < 48; i++){
		output[i] = s_SampleIRByteFrame[i];
	}
}

/* Function 1.4 */
static void IR_DetectUserCmd(uint8_t* inputBleCmd)
{
//	uint8_t cmd = 0xFF;
//	//is ON/OFF
//	if(inputBleCmd[0] == 0)
//	{
//		//Cmd OFF
//		DEBUG_PRINT("Cmd OFF");
//		cmd = 1;
//	}

//	if(inputBleCmd[0] == 1 && s_inputBleCmd[1] == inputBleCmd[1])
//	{
//		//Cmd ON
//		DEBUG_PRINT("Cmd ON");
//		cmd = 2;
//	}
//	else if(inputBleCmd[0] == 1 && inputBleCmd[1] > s_inputBleCmd[1])
//	{
//		//Cmd temperature up
//		DEBUG_PRINT("Cmd Temp Up");
//		cmd = 3;
//	}
//	else if(inputBleCmd[0] == 1 && inputBleCmd[1] < s_inputBleCmd[1])
//	{
//		//Cmd temperature down
//		DEBUG_PRINT("Cmd Temp Down");
//		cmd = 4;
//	}
//	
//	if(inputBleCmd[0] == 1 && inputBleCmd[2] != s_inputBleCmd[2])
//	{
//		//Cmd change fan speed
//		DEBUG_PRINT("Cmd Change Fan Speed");
//		cmd = 5;
//	}
//	
//	if(inputBleCmd[0] == 1 && inputBleCmd[3] != s_inputBleCmd[3])
//	{
//		//Cmd change swing
//		DEBUG_PRINT("Cmd Change Swing");
//		cmd = 6;
//	}
//	
//	if(inputBleCmd[0] == 1 && inputBleCmd[4] != s_inputBleCmd[4])
//	{
//		//Cmd change mode
//		DEBUG_PRINT("Cmd Change Mode");
//		cmd = 7;
//	}
//	
//	DEBUG_PRINT("Cmd ID: %d", cmd);
//	//
//	if(cmd == 0xFF)
//	{
//		DEBUG_PRINT("Can not detect Cmd");
//	}
//	
	//
	for(uint8_t i = 0; i < 8; i++)
	{
		s_inputBleCmd[i] = inputBleCmd[i];
	}
}

/* Function 1.5 */
/* Get values from BLE command then fill to IR protocol */
void IR_EncodeUserCmdToIRProtocol_Hitachi(uint8_t* InputBleCommands, int16_t* OutputIRProtocol) 
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
		InputBleCommands[4] = 7;
	}
	else if(ac_status.mode == AC_MODE_DRY)
	{
		InputBleCommands[4] = 5;
	}
	else if(ac_status.mode == AC_MODE_FAN_ONLY)
	{
		InputBleCommands[4] = 1;
	}
	else if(ac_status.mode == AC_MODE_COOL)
	{
		InputBleCommands[4] = 3;
	}
	else if(ac_status.mode == AC_MODE_HEAT)
	{
		InputBleCommands[4] = 6;
	}
	

	if(ac_status.fan == AC_FAN_LVL_AUTO)
	{
		InputBleCommands[2] = 5;
	}
	else if(ac_status.fan == AC_FAN_LVL_1)
	{
		InputBleCommands[2] = 1;
	}
	else if(ac_status.fan == AC_FAN_LVL_2)
	{
		InputBleCommands[2] = 1;
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
		InputBleCommands[2] = 4;
	}
	
	
	IR_DetectUserCmd(InputBleCommands);
	
	IR_EncodeUserCmdToByteFrame(InputBleCommands, txIRByte);
//	for(uint32_t i = 0; i < 33 ; i++) {
//		DEBUG_PRINT("%d ", txIRByte[i]);
//	}
	IR_EncodeByteToBitForm_Hitachi(txIRByte, txBit);
	IR_EncodeBitToRawFrame(txBit, OutputIRProtocol);
}

/* */
/* */
/* DECODE PART */
/* */
/* */

/* Function 2.1 */
static uint32_t IR_DecodeRawFrameToBit_Hitachi(int16_t* input, uint8_t* output) {
	uint32_t index = 0;
	for(uint32_t i = 3; i < DATASET_MAX_INDEX_HITACHI ; i++) {
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
	for(uint32_t i = index; i < DATASET_MAX_INDEX_HITACHI ; i++) {
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
	
	for(uint32_t i = 0; i < 37 ; i++) {
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
	
	val = GetValueFromBitPos(IR_Bytes[27], 4, 4);

	BLEData[0] = val;
	
	/* 1. Set byte 1 */
	/* CONTROL TEMPERATURE */
	/* Value = setting temperature * 4 */
	BLEData[1] = IR_Bytes[13] >> 2;
	
	if(BLEData[1] > 32)
	{
		BLEData[1] = 32;
	}
	
	if(BLEData[1] < 16)
	{
		BLEData[1] = 16;
	}
	
	/* 2. Set byte 2 */
	/* WIND LEVEL */
	val = GetValueFromBitPos(IR_Bytes[25], 4, 6);

	BLEData[2] = val;
	
	
	/* 3. Set byte 3 */
	/* AUTO/MANUAL SWING */
	if(IR_Bytes[11] == 129)
	{
		BLEData[3] = 1;
	}
	else
	{
		BLEData[3] = 0;
	}
	
	/* 4. Set byte 4 */
	/* SET AIRCONDITIONER MODE */
	val = GetValueFromBitPos(IR_Bytes[25], 0, 3);
	BLEData[4] = val;
}

/* Function 2.4 */
void IR_DecodeRawFrameToUserCmd_Hitachi(int16_t* input, uint8_t* output) {
	DEBUG_PRINT("IR_DecodeRawFrameToUserCmd_Hitachi");
	uint8_t irBitFrame[640];
	uint8_t rxByteFrame[48];
	
	uint32_t idx = IR_DecodeRawFrameToBit_Hitachi(input, irBitFrame);
	IR_DecodeBitToByteFrame(irBitFrame, rxByteFrame, idx);
	IR_DecodeByteToUserCmd(rxByteFrame, output);
	
	
	/* Set AC params */
	//1. Power
	ac_control_set_power_status(output[0]);
	//2. Temperature
	ac_control_set_temperature(output[1] - 16);
	
	//3. Mode
	if(output[4] == 1)
	{
		ac_control_set_mode(AC_MODE_FAN_ONLY);
	}
	else if(output[4] == 3)
	{
		ac_control_set_mode(AC_MODE_COOL);
	}
	else if(output[4] == 5)
	{
		ac_control_set_mode(AC_MODE_DRY);
	}
	else if(output[4] == 6)
	{
		ac_control_set_mode(AC_MODE_HEAT);
	}
	else if(output[4] == 7)
	{
		ac_control_set_mode(AC_MODE_AUTO);
	}
	
	//4. Fan
	if(output[2] == 5)
	{
		ac_control_set_fan(AC_FAN_LVL_AUTO);
	}
	else if(output[2] == 4)
	{
		ac_control_set_fan(AC_FAN_LVL_5);
	}
	else if(output[2] == 3)
	{
		ac_control_set_fan(AC_FAN_LVL_3);
	}
	else if(output[2] == 1)
	{
		ac_control_set_fan(AC_FAN_LVL_1);
	}
	
	ac_control_update_status_to_payload();
}


/* Function 3.1 */
int IsEnableIrTx_Hitachi(void)
{
	return s_IrEnableTx;
}

/* Function 3.2 */
void SetIrTxState_Hitachi(int state)
{
	s_IrEnableTx = state;
}

uint8_t* GetByteFromFrame_Hitachi() 
{
	return s_SampleIRByteFrame;
}

#endif