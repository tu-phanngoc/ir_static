#ifndef __IR_DATACONVERTER_FUNIKI_H
#define __IR_DATACONVERTER_FUNIKI_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_FUNIKI			178

#define FRAME_BYTE_LEN			11

#define MARK_FUNIKI   			600

#define ONE_SPACE_FUNIKI  	1600
#define ZERO_SPACE_FUNIKI 	430


#define LONG_MARK_FUNIKI   	9200   // ^^^^^^|___|^^|__|^^
#define LONG_SPACE_FUNIKI  	4500  

static void IR_EncodeByteToBitForm_Funiki(uint8_t* input, uint8_t* i16Output);

void IR_EncodeUserCmdToIRProtocol_Funiki(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_Funiki(int16_t* input, uint8_t* output);
int IsEnableIrTx_Funiki(void);
void SetIrTxState_Funiki(int state);



#endif
#endif

