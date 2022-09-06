#include "stdint.h"
#include "stdio.h"
#include "stdarg.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "system_config.h"


#include "IR_DeviceConstructor.h"
#include "IR_Interface.h"
#include "IR_DataConverter-Toshiba.h"
#include "IR_DataConverter-Hitachi.h"

#include "IR_DataConverter_Funiki.h"
#include "IR_DataConverter_OG104AC.h"
#include "IR_DataConverter_Olimpia136AC.h"

#include "IR_Common.h"

#include "IRcommon.h"
#include "ir_Sharp.h"
#include "ir_Casper.h"
#include "ir_Casper104.h"
#include "ir_Sanyo.h"
#include "ir_Mitsubishi144.h"
#include "ir_Mitsubishi112.h"
#include "ir_Mitsubishi136.h"
#include "ir_MitsubishiHeavy.h"
#include "ir_Panasonic.h"
#include "ir_Panasonic32.h"
#include "ir_LG.h"
#include "ir_Daikin.h"
#include "ir_Casper343.h"
#include "ir_Toshiba.h"
#include "ir_Reetech.h"
#include "ir_Coolix.h"
#include "ir_Vestel.h"
#include "ir_Gree.h"
#include "ir_Samsung.h"

extern ComClass_t gIRInterface;

const ComClass_t IR_Constrcutors[AC_MAX] = {
  {NULL, encode_ToshibaAc,                          decode_ToshibaAc,                        NULL, isIrTxEnable,                 setIrTxState,                 0,   TOSHIBA_BITS - 1,  TOSHIBA_BITS - 1},
  {NULL, encode_Daikin152,                          decode_Daikin152,                        NULL, isIrTxEnable,                 setIrTxState,                 1,   DAIKIN152_BITS - 1,   DAIKIN152_BITS - 1},
  {NULL, IR_EncodeUserCmdToIRProtocol_Hitachi,      IR_DecodeRawFrameToUserCmd_Hitachi,      NULL, IsEnableIrTx_Hitachi,         SetIrTxState_Hitachi,         2,   530, 530},
  {NULL, encode_DaikinESP,                          decode_DaikinESP,                        NULL, isIrTxEnable,                 setIrTxState,                 3,   DAIKIN_BITS - 1,   DAIKIN_BITS - 1},
  {NULL, encode_PanasonicAc,                        decode_PanasonicAc,                      NULL, isIrTxEnable,                 setIrTxState,                 4,   PANASONICAC_BITS - 1,  PANASONICAC_BITS - 1},
  {NULL, IR_EncodeUserCmdToIRProtocol_Funiki,       IR_DecodeRawFrameToUserCmd_Funiki,       NULL, IsEnableIrTx_Funiki,          SetIrTxState_Funiki,          5,   DATASET_MAX_INDEX_FUNIKI,                  DATASET_MAX_INDEX_FUNIKI},
  {NULL, encode_Mitsubishi144,                      decode_Mitsubishi144,                    NULL, isIrTxEnable,                 setIrTxState,                 6,   MITSUBISHI_144_MAX_INDEX - 1,              MITSUBISHI_144_MAX_INDEX - 1},
  {NULL, encode_Mitsubishi112,                      decode_Mitsubishi112,                    NULL, isIrTxEnable,                 setIrTxState,                 7,   MITSUBISHI_112_MAX_INDEX,                  MITSUBISHI_112_MAX_INDEX},
  {NULL, IR_EncodeUserCmdToIRProtocol_OG104AC,      IR_DecodeRawFrameToUserCmd_OG104AC,      NULL, IsEnableIrTx_OG104AC,         SetIrTxState_OG104AC,         8,   DATASET_MAX_INDEX_OG104AC,                 DATASET_MAX_INDEX_OG104AC},
  {NULL, IR_EncodeUserCmdToIRProtocol_Olimpia136AC, IR_DecodeRawFrameToUserCmd_Olimpia136AC, NULL, IsEnableIrTx_Olimpia136AC,    SetIrTxState_Olimpia136AC,    9,   DATASET_MAX_INDEX_OLIMPIA136AC,            DATASET_MAX_INDEX_OLIMPIA136AC},
  {NULL, encode_Sharp,                              decode_Sharp,                            NULL, isIrTxEnable,                 setIrTxState,                 10,  SHARP_BITS - 1,                            SHARP_BITS - 1},
  {NULL, encode_Sanyo,                              decode_Sanyo,                            NULL, isIrTxEnable,                 setIrTxState,                 11,  DATASET_MAX_INDEX_SANYO - 1,               DATASET_MAX_INDEX_SANYO - 1},
  {NULL, encode_Coolix,                             decode_Coolix,                           NULL, isIrTxEnable,                 setIrTxState,                 12,  COOLIX_BITS - 1,              COOLIX_BITS - 1},
  {NULL, encode_Mitsubishi136,                      decode_Mitsubishi136,                    NULL, isIrTxEnable,                 setIrTxState,                 13,  MITSUBISHI_136_MAX_INDEX,                  MITSUBISHI_136_MAX_INDEX},
  {NULL, encode_PanasonicAc32,                      decode_PanasonicAc32,                    NULL, isIrTxEnable,                 setIrTxState,                 14,  DATASET_MAX_INDEX_PANASONICAC32_LONG - 1,  DATASET_MAX_INDEX_PANASONICAC32_LONG - 1},
  {NULL, encode_PanasonicAc32,                      decode_PanasonicAc32,                    NULL, isIrTxEnable,                 setIrTxState,                 15,  DATASET_MAX_INDEX_PANASONICAC32_SHORT - 1, DATASET_MAX_INDEX_PANASONICAC32_SHORT - 1},
  {NULL, encode_LG,                                 decode_LG,                               NULL, isIrTxEnable,                 setIrTxState,                 16,  LG_BITS - 1,                               LG_BITS - 1},
  {NULL, encode_Casper104,                          decode_Casper104,                        NULL, isIrTxEnable,                 setIrTxState,                 17,  CASPER104_BITS - 1,                        CASPER104_BITS - 1},
  {NULL, encode_MitsubishiHeavy,                    decode_MitsubishiHeavy,                  NULL, isIrTxEnable,                 setIrTxState,                 18,  MITSUBISHI88_BITS - 1,                     MITSUBISHI88_BITS - 1},
  {NULL, encode_MitsubishiHeavy,                    decode_MitsubishiHeavy,                  NULL, isIrTxEnable,                 setIrTxState,                 19,  MITSUBISHI152_BITS - 1,                    MITSUBISHI152_BITS - 1},
  {NULL, encode_Daikin2,                            decode_Daikin2,                          NULL, isIrTxEnable,                 setIrTxState,                 AC_DAIKIN2,   DAIKIN2_BITS - 1,     DAIKIN2_BITS - 1, },
  {NULL, encode_Daikin216,                          decode_Daikin216,                        NULL, isIrTxEnable,                 setIrTxState,                 AC_DAIKIN216, DAIKIN216_BITS - 1,   DAIKIN216_BITS - 1},
  {NULL, encode_Daikin160,                          decode_Daikin160,                        NULL, isIrTxEnable,                 setIrTxState,                 AC_DAIKIN160, DAIKIN160_BITS - 1,   DAIKIN160_BITS - 1},
  {NULL, encode_Daikin176,                          decode_Daikin176,                        NULL, isIrTxEnable,                 setIrTxState,                 AC_DAIKIN176, DAIKIN176_BITS - 1,   DAIKIN176_BITS - 1},
  {NULL, encode_Daikin128,                          decode_Daikin128,                        NULL, isIrTxEnable,                 setIrTxState,                 AC_DAIKIN128, DAIKIN128_BITS - 1,   DAIKIN128_BITS - 1},
  {NULL, encode_Daikin64,                           decode_Daikin64,                         NULL, isIrTxEnable,                 setIrTxState,                 AC_DAIKIN64,  DAIKIN64_BITS - 1,    DAIKIN64_BITS - 1},
  {NULL, encode_Casper343,                          decode_Casper343,                        NULL, isIrTxEnable,                 setIrTxState,                 AC_CASPER343, CASPER343_BITS - 1,    CASPER343_BITS - 1},
  {NULL, encode_ReetechAc,                          decode_ReetechAc,                        NULL, isIrTxEnable,                 setIrTxState,                 AC_CHIGO96AC, REETECH_BITS - 1, REETECH_BITS - 1},
  {NULL, encode_VestelAc,                           decode_VestelAc,                         NULL, isIrTxEnable,                 setIrTxState,                 AC_VESTEL,    VESTELAC_BITS - 1, VESTELAC_BITS - 1},
  {NULL, encode_Gree,                               decode_Gree,                             NULL, isIrTxEnable,                 setIrTxState,                 AC_GREE,      GREE_BITS - 1,     GREE_BITS - 1},
  {NULL, encode_Samsung,                            decode_Samsung,                          NULL, isIrTxEnable,                 setIrTxState,                 AC_SAMSUNG,   SAMSUNG_BITS - 1,  SAMSUNG_BITS - 1},
};


void IR_ConstructDeviceType()
{
  CFG_Load(false);
  uint32_t deviceType = sysCfg.IR_devicetype;
  NRF_LOG_INFO("deviceType = %u", deviceType);
  IR_ConstructSpecificDeviceType(deviceType);
}

void IR_ConstructSpecificDeviceType(uint32_t deviceType){
  if( deviceType >= AC_MAX ||
      IR_Constrcutors[deviceType].IR_EncodeBLEToIR == NULL ||
      IR_Constrcutors[deviceType].IR_Decode == NULL ){
    gIRInterface = IR_Constrcutors[0];
  }
  else{
    gIRInterface = IR_Constrcutors[deviceType];
  }
}

ac_type_t IRInterface_DetectDeviceType(uint32_t u32MaxIdx, int16_t* input) 
{
  int16_t* data = input;
  ac_type_t u32Type = (ac_type_t)sysCfg.IR_devicetype;

  if( u32MaxIdx >= TOSHIBA_BITS-1 && isHeader_Toshiba(data))
  {
    u32Type = AC_TOSHIBA;
  }
  else if( (u32MaxIdx == (DAIKIN_BITS - 1)) &&  DaikinESP_isHeaderMatch(data))
  {
    u32Type = AC_DAIKIN;
  }
  else if( (u32MaxIdx == (DAIKIN2_BITS - 1)) &&  matchMark(data[0], kDaikin2LeaderMark, kTolerance + kDaikin2Tolerance))
  {
    u32Type = AC_DAIKIN2;
  }   
  else if( (u32MaxIdx == (DAIKIN216_BITS - 1)) &&  Daikin216_isHeaderMatch(data)) // 50% 
  {
    u32Type = AC_DAIKIN216;
  }
  else if( (u32MaxIdx == (DAIKIN160_BITS - 1)) &&  matchMark(data[0], kDaikin160HdrMark, kTolerance)) // 50% 
  {
    u32Type = AC_DAIKIN160;
  }
  else if( (u32MaxIdx == (DAIKIN176_BITS - 1)) &&  matchMark(data[0], kDaikin176HdrMark, kTolerance)) // 50% 
  {
    u32Type = AC_DAIKIN176;
  }
  else if( (u32MaxIdx == (DAIKIN128_BITS - 1)) &&  matchMark(data[0], kDaikin128LeaderMark, kTolerance)) // 50% 
  {
    u32Type = AC_DAIKIN128;
  }
  else if( (u32MaxIdx == (DAIKIN152_BITS - 1)) &&  Daikin152_isHeaderMatch(data) ) // 50% 
  {
    u32Type = AC_DAIKIN152;
  }
  else if( (u32MaxIdx == (DAIKIN64_BITS - 1)) &&  matchMark(data[0], kDaikin64LdrMark, kTolerance) ) // 50% 
  {
    u32Type = AC_DAIKIN64;
  } 
  else if(u32MaxIdx == DATASET_MAX_INDEX_HITACHI) 
  {
    u32Type = AC_HITACHI;
  }
  else if(u32MaxIdx == (PANASONICAC_BITS - 1) && isPanasonicAcHeader(data))
  {
    u32Type = AC_PANASONIC;
  }
  else if(u32MaxIdx == DATASET_MAX_INDEX_FUNIKI && (data[0] > 8000) && (data[0] < 10000)) 
  {
    u32Type = AC_FUNIKI;
  }
  else if(u32MaxIdx == MITSUBISHI_144_MAX_INDEX - 1 && (data[0] > 2380) && (data[0] < 4420)) // 30%  Tolerance
  {
    u32Type = AC_MITSUBISHI144;
  }
  else if(u32MaxIdx == MITSUBISHI_112_MAX_INDEX && (data[0] > 2410) && (data[0] < 4485))  // 30%  Tolerance
  {
    u32Type = AC_MITSUBISHI112;
  }
  else if( (u32MaxIdx == (CASPER104_BITS - 1)) && Casper104_isHeaderMatch(data) )
  {
    u32Type = AC_CASPER104;
  }
  else if(u32MaxIdx == DATASET_MAX_INDEX_OG104AC && (data[0] > 7000) && (data[0] < 10000)) 
  {
    u32Type = AC_OG104AC;
  }
  else if(u32MaxIdx == (SHARP_BITS - 1) && (matchMark(data[0], kSharpAcHdrMark, kTolerance)) )   // 50%  Tolerance
  {
    u32Type = AC_SHARP104;
  }
  else if( (u32MaxIdx == (DATASET_MAX_INDEX_SANYO - 1)) && (data[0] > 7500) && (data[0] < 10000) )
  {
    u32Type = AC_SANYO;
  }
  else if( (u32MaxIdx == (COOLIX_BITS - 1)) && isHeader_Coolix(data) )
  {
    u32Type = AC_COOLIX;
  }
  else if(u32MaxIdx == MITSUBISHI_136_MAX_INDEX && (data[0] > 2326) && (data[0] < 4321) )  // 30%  Tolerance
  {
    u32Type = AC_MITSUBISHI136;
  }
  else if( (u32MaxIdx == (DATASET_MAX_INDEX_PANASONICAC32_LONG - 1)) && (data[0] > 2480) && (data[0] < 4605) )  // 30%  Tolerance
  {
    u32Type = AC_PANASONICAC32_LONG;
  }
  else if( (u32MaxIdx == (DATASET_MAX_INDEX_PANASONICAC32_SHORT - 1)) && (data[0] > 2480) && (data[0] < 4605) )  // 30%  Tolerance
  {
    u32Type = AC_PANASONICAC32_SHORT;
  }
  else if(   (u32MaxIdx == (LG_BITS - 1)) &&
             (((data[0] > 5950) && (data[0] < 11050)) || ((data[0] > 2240) && (data[0] < 4160))) )  // 30%  Tolerance ac28bit
  {
    u32Type = AC_LG;
  }
  else if( (u32MaxIdx == MITSUBISHI88_BITS - 1)  && matchMark(data[0], kMitsubishiHeavyHdrMark, kTolerance) )
  {
    u32Type = AC_MITSUBISHI_HEAVY88;
    MitsubishiHeavy_set152AcTypes(false);
  }
  else if( (u32MaxIdx == (MITSUBISHI152_BITS - 1)) && matchMark(data[0], kMitsubishiHeavyHdrMark, kTolerance) )
  {
    u32Type = AC_MITSUBISHI_HEAVY152;
    MitsubishiHeavy_set152AcTypes(true);
  }
  else if( (u32MaxIdx == (CASPER343_BITS - 1)) && isCapser343(data) )
  {
    u32Type = AC_CASPER343;
  }
  else if( (u32MaxIdx == (REETECH_BITS - 1)) && isHeader_Reetech(data) )
  {
    u32Type = AC_CHIGO96AC;
  }
  else if( (u32MaxIdx == (VESTELAC_BITS - 1)) && isVestelAc(data) )
  {
    u32Type = AC_VESTEL;
  }
  else if( (u32MaxIdx == (GREE_BITS - 1)) && isGreeAc(data) )
  {
    u32Type = AC_GREE;
  }
  else if( ( (u32MaxIdx == (SAMSUNG_BITS - 1)) || (u32MaxIdx == (SAMSUNG_EXT_BITS - 1)) )
            && 
            isSamsung(data) )
  {
    u32Type = AC_SAMSUNG;
  }
  else
  { 
    u32Type = 0xFF;
  }
  NRF_LOG_INFO("irRecvLen: %u", u32MaxIdx);
  //NRF_LOG_INFO("Dectec_Ac_4FirstData: %04X %04X %04X %04X", (uint16_t)data[0], (uint16_t)data[1], (uint16_t)data[2], (uint16_t)data[3]);
  NRF_LOG_INFO("IRInterface_DetectDeviceType: %u", u32Type);
  return u32Type;
}

