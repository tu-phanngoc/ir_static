#ifndef _SMART_HOME_CFG_H_
#define _SMART_HOME_CFG_H_
#include "smarthome_meter.h"
/**************************************************************************************
 * General parameters and scaling coefficients
 **************************************************************************************/
//#define U_MAX 396
#define U_MAX (649.8)	//(3.6*361/2) 

#define I0_MAX (36.0)	//(72/2)

//#define I0_MAX 54
//#define I0_MAX 656
//#define I0_MAX 520
//#define I1_MAX 33
//#define I1_MAX 656
#define I1_MAX 18
//#define I1_MAX 520
#define I2_MAX 18
#define I3_MAX 18
#define RAW_SAMPLE_FREQ 1000 /*!< Sample frequency in Hz of raw samples   */
#define CALCFREQ 1000.000 /*!< Sample frequency in Hz */
#define IMP_PER_KWH 5000 /*!< Impulses per kWh */
#define IMP_PER_KVARH 5000 /*!< Impulses per kVARh */
#define DECIM_FACTOR 1 /*!< Auxiliary calculations decimation factor  */
#define REF_IMP_PER_KWH 5000
#define MAINS_FREQ 50 /*!< AC frequency in Hz */
#define MAINS_FREQ_ERR 2 /*!< AC frequency error in Hz */
#define MINIMAL_CURRENT 0.01


/**********************************************************************************
 * Filter-based metering algorithm configuration structure
 **********************************************************************************/
#define SMARTHOME_CFG                                                                 \
{                                                                                     \
  U_MAX,                                                                              \
  {I0_MAX,I1_MAX,I2_MAX,I3_MAX},                                                      \
  {PWR_SENS_THR0(0.110),PWR_SENS_THR1(0.110),PWR_SENS_THR2(0.110),PWR_SENS_THR3(0.110)},                                                             \
  1,                                                                                  \
  {FRAC32(+0.99921521804155),FRAC32(-0.99921521804155),FRAC32(-0.99843043608309)},    \
  {FRAC32(+0.13165249758740),FRAC32(+0.13165249758740),FRAC32(-1.0)},                 \
  {{0l,0ll},{0l,0ll}},                                                                \
  {0l,0ll},                                                                           \
  {{{0l,0ll},{0l,0ll}},{{0l,0ll},{0l,0ll}},                                           \
   {{0l,0ll},{0l,0ll}},{{0l,0ll},{0l,0ll}}},                                           \
  {  49,                                                                              \
    {                                                                                 \
      FRAC32(0.0),FRAC32(-0.00073728465714),FRAC32(0.0),FRAC32(-0.00196750272687),    \
      FRAC32(0.0),FRAC32(-0.00411945802255),FRAC32(0.0),FRAC32(-0.00756839142185),    \
      FRAC32(0.0),FRAC32(-0.01278720365088),FRAC32(0.0),FRAC32(-0.02040684105768),    \
      FRAC32(0.0),FRAC32(-0.03136483560542),FRAC32(0.0),FRAC32(-0.04728105184137),    \
      FRAC32(0.0),FRAC32(-0.07151114503989),FRAC32(0.0),FRAC32(-0.11276139617420),    \
      FRAC32(0.0),FRAC32(-0.20318408017719),FRAC32(0.0),FRAC32(-0.63356345988777),    \
      FRAC32(0.0),FRAC32(+0.63356345988777),FRAC32(0.0),FRAC32(+0.20318408017719),    \
      FRAC32(0.0),FRAC32(+0.11276139617420),FRAC32(0.0),FRAC32(+0.07151114503989),    \
      FRAC32(0.0),FRAC32(+0.04728105184137),FRAC32(0.0),FRAC32(+0.03136483560542),    \
      FRAC32(0.0),FRAC32(+0.02040684105768),FRAC32(0.0),FRAC32(+0.01278720365088),    \
      FRAC32(0.0),FRAC32(+0.00756839142185),FRAC32(0.0),FRAC32(+0.00411945802255),    \
      FRAC32(0.0),FRAC32(+0.00196750272687),FRAC32(0.0),FRAC32(+0.00073728465714),    \
      FRAC32(0.0),                                                                    \
    },                                                                                \
     25,                                                                              \
    {                                                                                 \
      FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),        \
      FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),        \
      FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),        \
      FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),FRAC16(0.0),        \
      FRAC16(-1.0),                                                                   \
    }                                                                                 \
  },                                                                                  \
  {                                                                                   \
    {                                                                                 \
      0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,  \
      0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,           \
    },                                                                                \
    0ll,                                                                              \
    {                                                                                 \
      0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,     \
    },                                                                                \
    0l,                                                                               \
  },                                                                                  \
  {                                                                                   \
    {FRAC32(+0.00130728645170),FRAC32(+0.00130728645170),FRAC32(-0.99738542709660)},  \
    {FRAC32(+0.00130728645170),FRAC32(+0.00130728645170),FRAC32(-0.99738542709660)},  \
  },                                                                                  \
  {0ll,0ll,0l,0ll},                                                                   \
  {{0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll}},                                                                  \
  {{0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll}},                                                                  \
  {{0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll},                                                                   \
   {0ll,0ll,0l,0ll}},                                                                  \
  {{                                                                                  \
    FRAC48((+0.0000/(U_MAX*I0_MAX))),KWH_DISP_RES0(   1000),0ll,0ll,0l,FRAC16(-1.0),  \
    {FRAC32(+0.00779293629195),FRAC32(+0.00779293629195),FRAC32(-0.98441412741610)},  \
    0ll,0ll,                                                                          \
   },                                                                                  \
   {                                                                                   \
    FRAC48((+0.0000/(U_MAX*I1_MAX))),KWH_DISP_RES1(   1000),0ll,0ll,0l,FRAC16(-1.0),  \
    {FRAC32(+0.00779293629195),FRAC32(+0.00779293629195),FRAC32(-0.98441412741610)},  \
    0ll,0ll,                                                                          \
   },                                                                                  \
   {                                                                                   \
    FRAC48((+0.0000/(U_MAX*I2_MAX))),KWH_DISP_RES2(   1000),0ll,0ll,0l,FRAC16(-1.0),  \
    {FRAC32(+0.00779293629195),FRAC32(+0.00779293629195),FRAC32(-0.98441412741610)},  \
    0ll,0ll,                                                                          \
   },                                                                                  \
   {                                                                                   \
    FRAC48((+0.0000/(U_MAX*I3_MAX))),KWH_DISP_RES3(   1000),0ll,0ll,0l,FRAC16(-1.0),  \
    {FRAC32(+0.00779293629195),FRAC32(+0.00779293629195),FRAC32(-0.98441412741610)},  \
    0ll,0ll,                                                                          \
   }},                                                                                \
	{{                                                                                  \
    FRAC48((+0.0000/(U_MAX*I0_MAX))),KWH_DISP_RES0(   1000),0ll,0ll,0l,FRAC16(-1.0),  \
    {FRAC32(+0.00779293629195),FRAC32(+0.00779293629195),FRAC32(-0.98441412741610)},  \
    0ll,0ll,                                                                          \
   },                                                                                  \
   {                                                                                   \
    FRAC48((+0.0000/(U_MAX*I1_MAX))),KWH_DISP_RES1(   1000),0ll,0ll,0l,FRAC16(-1.0),  \
    {FRAC32(+0.00779293629195),FRAC32(+0.00779293629195),FRAC32(-0.98441412741610)},  \
    0ll,0ll,                                                                          \
   },                                                                                  \
   {                                                                                   \
    FRAC48((+0.0000/(U_MAX*I2_MAX))),KWH_DISP_RES2(   1000),0ll,0ll,0l,FRAC16(-1.0),  \
    {FRAC32(+0.00779293629195),FRAC32(+0.00779293629195),FRAC32(-0.98441412741610)},  \
    0ll,0ll,                                                                          \
   },                                                                                  \
   {                                                                                   \
    FRAC48((+0.0000/(U_MAX*I3_MAX))),KWH_DISP_RES3(   1000),0ll,0ll,0l,FRAC16(-1.0),  \
    {FRAC32(+0.00779293629195),FRAC32(+0.00779293629195),FRAC32(-0.98441412741610)},  \
    0ll,0ll,                                                                          \
   }},                                                                                \
}
#endif
