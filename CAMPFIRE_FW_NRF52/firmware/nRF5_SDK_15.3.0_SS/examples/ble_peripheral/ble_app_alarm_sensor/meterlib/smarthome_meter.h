#ifndef _SMARTHOME_METER_H_
#define _SMARTHOME_METER_H_
#include "fraclib.h"
#include "meterlib.h"

#define NUM_CHAN 4
#define NUM_DUAL_CHAN 2

typedef struct
{
  double            umax;         /*!< maximal voltage measurement range       */
  double            imax[NUM_CHAN]; /*!< maximal current 0 measurement range   */
  Frac32            thrd[NUM_CHAN]; /*!< sensitivity threshold                 */
  int               sens;         /*!< sensor: 1-proportional, 2-derivative    */
  tBIASFILTER_COEF  bias;         /*!< dc bias supressor filter coefficients   */
  tBIASFILTER_COEF  intg;         /*!< integrator filter coefficients          */
  tBIASFILTER_DATA  uDcb[2];      /*!< ph1-voltage dc bias supressor data      */
  tBIASFILTER_DATA  intd;         /*!< ph1-current integrator filter data      */
  tBIASFILTER_DATA  iDcb[NUM_CHAN][2]; /*!< ph1-current dc bias supressor data */
  tPHSHFILTER_COEF  hilb;         /*!< Hilbert & delay filter coefficients     */
  tPHSHFILTER_DATA  phSh;         /*!< ph1-Hilbert & delay filter data         */
  tAVERFILTER_COEF  aver;         /*!< averager filter coefficients            */
  tAVERFILTER_DATA  uRms;         /*!< ph1-voltage RMS averager data           */
  tAVERFILTER_DATA  iRms[NUM_CHAN];  /*!< current 0 RMS averager data          */
  tAVERFILTER_DATA  PAve[NUM_CHAN];  /*!< active power 0 averager data         */
  tAVERFILTER_DATA  QAve[NUM_CHAN];  /*!< ph1-reactive power averager data     */
  tENERGY_DATA      Wh[NUM_CHAN];    /*!< active energy 0                      */
  tENERGY_DATA      VARh[NUM_CHAN];  /*!< reactive energy                      */
  Frac32 rawi[NUM_CHAN];
} tSMARTHOME_DATA;

typedef struct
{
  Frac32 u_gain;
  Frac32 i_gain[NUM_CHAN];
  Frac32 p_gain[NUM_CHAN];
}tCORRECT_DATA;

typedef struct
{
	double urms;
	double irms[NUM_CHAN];
	double p[NUM_CHAN];
	double q[NUM_CHAN];
	double pf[NUM_CHAN];
}tSMARTHOME_RESULT;

/** @brief remove DC bias in signal
 *  @param p address to smarthome struct
 *  @param uQ voltage sample
 *  @param iQ address to current channels
 */
void SMARTHOME_RemoveDcBias (tSMARTHOME_DATA *p, Frac24 uQ, Frac24* iQ);

/** @brief calculate energy
 *  @param p address to smarthome struct
 *  @param pCnt address which energy counter written
 *  @param outRes counter resolution
 *  @param Correct correct factor
 */
int SMARTHOME_CalcWattHours (tSMARTHOME_DATA *p, Frac32 *pCnt, Frac64 *outRes,Frac32 *attenuation);
int SMARTHOME_CalcVarHours (tSMARTHOME_DATA *p, Frac32 *pCnt, Frac64 *outRes,Frac32 *attenuation);
/** @brief calculate auxiliary variable
 *  @param p address of smarthome struct
 */
void SMARTHOME_CalcAuxiliary (tSMARTHOME_DATA *p);

/** @brief read result
 *  @param p address of smarthome struct
 *  @param r address which result written
 *  @param umax address which umax written
 *  @param imax address which imax written
 */
void SMARTHOME_ReadResults (tSMARTHOME_DATA *p, tSMARTHOME_RESULT *r, double *umax,double *imax);

 #endif
