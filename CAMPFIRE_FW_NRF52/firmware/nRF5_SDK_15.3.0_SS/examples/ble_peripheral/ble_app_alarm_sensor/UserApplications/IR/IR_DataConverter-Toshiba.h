#ifndef __IR_DATACONVERTER_TOSHIBA_H
#define __IR_DATACONVERTER_TOSHIBA_H

#ifdef __cplusplus
 extern "C" {
#endif
#if(1)
#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#define DATASET_MAX_INDEX_TOSHIBA			294
#define SHORT_FRAME_BYTE_LEN			9


#define TOSHIBA_MARK   530
#define TOSHIBA_ONE_SPACE  1600
#define TOSHIBA_ZERO_SPACE 460

#define TOSHIBA_LONG_MARK   4500   // ^^^^^^|___|^^|__|^^
#define TOSHIBA_LONG_SPACE  4350  
#define TOSHIBA_VERY_LONG_SPACE 6700


void IR_DecodeRawFrameToUserCmd_Toshiba(int16_t* input, uint8_t* output);
void IR_EncodeUserCmdToIRProtocol_Toshiba(uint8_t* InputBleCommands, int16_t* OutputIRProtocol);

/* Function 3.1 */
int IsEnableIrTx_Toshiba(void);

/* Function 3.2 */
void SetIrTxState_Toshiba(int state);

#endif

#endif