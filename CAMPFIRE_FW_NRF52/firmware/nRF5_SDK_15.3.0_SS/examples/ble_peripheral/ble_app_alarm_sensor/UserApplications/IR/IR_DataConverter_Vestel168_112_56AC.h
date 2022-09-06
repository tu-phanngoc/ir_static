#ifndef __IR_DATACONVERTER_VESTEL168_112_56AC_H
#define __IR_DATACONVERTER_VESTEL168_112_56AC_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_SHARP104AC			210

#define FRAME_BYTE_LEN_SHARP104AC			    13

#define MARK_SHARP104AC             500
#define ONE_SPACE_SHARP104AC        1350
#define ZERO_SPACE_SHARP104AC       500
#define LONG_MARK_SHARP104AC        3800   // ^^^^^^|___|^^|__|^^
#define LONG_SPACE_SHARP104AC       1900  

static void IR_EncodeByteToBitForm_SHARP104AC(uint8_t* input, uint8_t* i16Output);
void IR_EncodeUserCmdToIRProtocol_SHARP104AC(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_SHARP104AC(int16_t* input, uint8_t* output);
int IsEnableIrTx_SHARP104AC(void);
void SetIrTxState_SHARP104AC(int state);

#endif
#endif