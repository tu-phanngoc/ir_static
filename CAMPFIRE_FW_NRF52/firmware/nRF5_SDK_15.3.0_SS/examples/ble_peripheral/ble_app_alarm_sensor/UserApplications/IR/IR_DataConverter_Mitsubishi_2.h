#ifndef __IR_DATACONVERTER_MITSHIBISHI_2_H
#define __IR_DATACONVERTER_MITSHIBISHI_2_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_MITSUBISHI_2			582

#define SHORT_FRAME_BYTE_LEN_MITSUBISHI_2			18
#define TOTAL_FRAME_BYTE_LEN_MITSUBISHI_2			36

#define MARK_MITSUBISHI_2   500
#define ONE_SPACE_MITSUBISHI_2  1250
#define ZERO_SPACE_MITSUBISHI_2 400
#define LONG_MARK_MITSUBISHI_2   3500   // ^^^^^^|___|^^|__|^^
#define LONG_SPACE_MITSUBISHI_2  1700  

#define VERY_LONG_SPACE_MITSUBISHI_2 17200

static void IR_EncodeByteToBitForm_Mitsubishi_2(uint8_t* input, uint8_t* i16Output);
void IR_EncodeUserCmdToIRProtocol_Mitsubishi_2(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_Mitsubishi_2(int16_t* input, uint8_t* output);
int IsEnableIrTx_Mitsubishi_2(void);
void SetIrTxState_Mitsubishi_2(int state);

#endif
#endif

