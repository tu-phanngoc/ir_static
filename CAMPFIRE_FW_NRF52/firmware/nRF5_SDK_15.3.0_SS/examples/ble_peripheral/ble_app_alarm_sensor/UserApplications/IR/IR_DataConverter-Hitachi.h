#ifndef __IR_DATACONVERTER_HITACHI_H
#define __IR_DATACONVERTER_HITACHI_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_HITACHI			530

//#define SHORT_FRAME_BYTE_LEN			9


//#define HITACHI_MARK   600
//#define HITACHI_ONE_SPACE  1600
//#define HITACHI_ZERO_SPACE 500

//#define HITACHI_LONG_MARK   4600   // ^^^^^^|___|^^|__|^^
//#define HITACHI_LONG_SPACE  4450  

//#define HITACHI_VERY_LONG_SPACE 7500

extern uint8_t RxRawIRBytes[48];
extern uint8_t TxRawIRBytes[48];

static void IR_EncodeByteToBitForm_Hitachi(uint8_t* input, uint8_t* i16Output);

void IR_EncodeUserCmdToIRProtocol_Hitachi(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_Hitachi(int16_t* input, uint8_t* output);
int IsEnableIrTx_Hitachi(void);

void SetIrTxState_Hitachi(int state);

#endif
#endif