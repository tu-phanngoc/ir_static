#ifndef __IR_DATACONVERTER_OG104AC_H
#define __IR_DATACONVERTER_OG104AC_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_OG104AC			210

#define FRAME_BYTE_LEN_OG104AC			13

#define MARK_OG104AC   			480

#define ONE_SPACE_OG104AC  	1650
#define ZERO_SPACE_OG104AC 	570


#define LONG_MARK_OG104AC   	9200   // ^^^^^^|___|^^|__|^^
#define LONG_SPACE_OG104AC  	4500  

static void IR_EncodeByteToBitForm_OG104AC(uint8_t* input, uint8_t* i16Output);

void IR_EncodeUserCmdToIRProtocol_OG104AC(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_OG104AC(int16_t* input, uint8_t* output);
int IsEnableIrTx_OG104AC(void);
void SetIrTxState_OG104AC(int state);



#endif
#endif

