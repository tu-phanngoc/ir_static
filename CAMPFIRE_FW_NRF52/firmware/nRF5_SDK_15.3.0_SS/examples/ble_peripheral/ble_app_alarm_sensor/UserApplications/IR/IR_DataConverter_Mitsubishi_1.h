#ifndef __IR_DATACONVERTER_MITSHIBISHI_1_H
#define __IR_DATACONVERTER_MITSHIBISHI_1_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_MITSUBISHI_1			178

//#define SHORT_FRAME_BYTE_LEN			9


//#define HITACHI_MARK   600
//#define HITACHI_ONE_SPACE  1600
//#define HITACHI_ZERO_SPACE 500

//#define HITACHI_LONG_MARK   4600   // ^^^^^^|___|^^|__|^^
//#define HITACHI_LONG_SPACE  4450  

//#define HITACHI_VERY_LONG_SPACE 7500

static void IR_EncodeByteToBitForm_Mitsubishi_1(uint8_t* input, uint8_t* i16Output);
void IR_EncodeUserCmdToIRProtocol_Mitsubishi_1(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);
void IR_DecodeRawFrameToUserCmd_Mitsubishi_1(int16_t* input, uint8_t* output);
int IsEnableIrTx_Mitsubishi_1(void);
void SetIrTxState_Mitsubishi_1(int state);

#endif
#endif

