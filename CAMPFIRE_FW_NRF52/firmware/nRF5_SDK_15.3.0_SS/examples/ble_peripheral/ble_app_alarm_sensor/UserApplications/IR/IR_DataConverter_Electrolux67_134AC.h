#ifndef __IR_DATACONVERTER_ELECTROLUX67_134AC_H
#define __IR_DATACONVERTER_ELECTROLUX67_134AC_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_ELECTROLUX67_134AC			138

#define FRAME_BYTE_LEN_ELECTROLUX67_134AC			    9

#define MARK_ELECTROLUX67_134AC                 500
#define ONE_SPACE_ELECTROLUX67_134AC            1350
#define ZERO_SPACE_ELECTROLUX67_134AC           550

#define LONG_MARK_ELECTROLUX67_134AC            9000   // ^^^^^^|___|^^|__|^^
#define LONG_SPACE_ELECTROLUX67_134AC           4500  
#define VERY_LONG_SPACE_ELECTROLUX67_134AC      19500

static void IR_EncodeByteToBitForm_SHARP104AC(uint8_t* input, uint8_t* i16Output);
void IR_EncodeUserCmdToIRProtocol_SHARP104AC(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_SHARP104AC(int16_t* input, uint8_t* output);
int IsEnableIrTx_SHARP104AC(void);
void SetIrTxState_SHARP104AC(int state);

#endif
#endif