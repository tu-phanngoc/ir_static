#ifndef __IR_DATACONVERTER_OLIMPIA136AC_H
#define __IR_DATACONVERTER_OLIMPIA136AC_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_OLIMPIA136AC			274

#define FRAME_BYTE_LEN_OLIMPIA136AC			    17

#define MARK_OLIMPIA136AC   			            400

#define ONE_SPACE_OLIMPIA136AC  	                1200
#define ZERO_SPACE_OLIMPIA136AC                400


#define LONG_MARK_OLIMPIA136AC   	            3200   // ^^^^^^|___|^^|__|^^
#define LONG_SPACE_OLIMPIA136AC  	            1600  

static void IR_EncodeByteToBitForm_Olimpia136AC(uint8_t* input, uint8_t* i16Output);

void IR_EncodeUserCmdToIRProtocol_Olimpia136AC(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_Olimpia136AC(int16_t* input, uint8_t* output);
int IsEnableIrTx_Olimpia136AC(void);
void SetIrTxState_Olimpia136AC(int state);



#endif
#endif

