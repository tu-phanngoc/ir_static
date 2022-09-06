/******************************************************************************
 *
 * (c) Copyright 2010-2013, Freescale Semiconductor Inc.
 *
 * ALL RIGHTS RESERVED.
 *
 ***************************************************************************//*!
 *
 * @file      FRACLIB.h
 *
 * @author    R55013
 *
 * @version   1.0.3.0
 *
 * @date      Mar-08-2014
 *
 * @brief     Header file containing common data types, macros and list of 
 *            exported functions supporting 32-bit fractional arithmetic.
 *
 ******************************************************************************/
#ifndef __FRACLIB_H
#define __FRACLIB_H

/******************************************************************************
 * include header files                                                       *
 ******************************************************************************/
#include "FRACLIB_Types.h"
#include "FRACLIB_Mul32.h"
#include "FRACLIB_Inlines.h" 

/******************************************************************************
 * exported function prototypes                                               *
 ******************************************************************************/

/****************************************************************************//*!
 *
 * @brief   Division of a 32-bit fractional value by a 32-bit fractional value
 *          returning 16-bit fractional result.
 *
 * @param   num    - Input 32-bit fractional value.
 * @param   den    - Input 32-bit fractional value. 
 *
 * @return  Function returns 16-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac16 S_div_ll  (register Frac32 num, register Frac32 den);

/****************************************************************************//*!
 *
 * @brief   Division of a 32-bit fractional value by a 32-bit fractional value
 *          returning 32-bit fractional result.
 *
 * @param   num    - Input 32-bit fractional value.
 * @param   den    - Input 32-bit fractional value. 
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_div     (register Frac32 num, register Frac32 den);

/****************************************************************************//*!
 *
 * @brief   Division of a 64-bit fractional value by a 32-bit fractional value
 *          returning 32-bit fractional result.
 *
 * @param   num    - Input 64-bit fractional value.
 * @param   den    - Input 32-bit fractional value. 
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_div_lll (register Frac64 num, register Frac32 den);

/****************************************************************************//*!
 *
 * @brief   Division of a 64-bit fractional value by a 64-bit fractional value
 *          returning 16-bit fractional result.
 *
 * @param   num    - Input 64-bit fractional value.
 * @param   den    - Input 64-bit fractional value. 
 *
 * @return  Function returns 16-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac16 S_div_llll(register Frac64 num, register Frac64 den);

/****************************************************************************//*!
 *
 * @brief   Division of a 64-bit fractional value by a 64-bit fractional value
 *          returning 64-bit fractional result.
 *
 * @param   num    - Input 64-bit fractional value.
 * @param   den    - Input 64-bit fractional value. 
 *
 * @return  Function returns 64-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac64 LL_div    (register Frac64 num, register Frac64 den);

/****************************************************************************//*!
 *
 * @brief   Square root value of a 32-bit fractional value returning a 16-bit 
 *          result.  
 *
 * @param   x - Input 32-bit fractional value.
 *
 * @return  Function returns 16-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac16 S_sqr_l   (register Frac32 x);

/****************************************************************************//*!
 *
 * @brief   Square root value of a 32-bit fractional value returning a 32-bit 
 *          result.  
 *
 * @param   x - Input 32-bit fractional value.
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_sqr     (register Frac32 x);

/****************************************************************************//*!
 *
 * @brief   Square root value of a 64-bit fractional value returning a 32-bit 
 *          result.  
 *
 * @param   x - Input 64-bit fractional value.
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/ 
extern Frac32 L_sqr_ll  (register Frac64 x);

/****************************************************************************//*!
 *
 * @brief   Square root value of a 64-bit fractional value returning a 64-bit 
 *          result.  
 *
 * @param   x - Input 64-bit fractional value.
 *
 * @return  Function returns 64-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/ 
extern Frac64 LL_sqr    (register Frac64 x);

/****************************************************************************//*!
 *
 * @brief   Execute finite impulse response filter (FIR) iteration using the 
 *          following equation:
 *
 *          y = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *
 *          Internal accumulations don't saturate. The FIR filter output is within 
 *          32-bit fractional range from 0x80000000 to 0x7fffffff.
 *
 * @param   x        - Input fractional value represented in 16-bit fractional 
 *                     format "x(n)".
 * @param   *px      - Pointer to previous input values represented in 16-bit 
 *                     fractional format "x(n-1) ... x(n-nb)".
 * @param   *pcoef   - Pointer to filter constants represented in 16-bit fractional 
 *                     format "b(1) ... b(nb+1)".
 * @param   len      - Filter length "nb". 
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_fir_ss  (register Frac16 x, register Frac16 *px, const Frac16 *pcoef, 
                         register Word16 len);

/****************************************************************************//*!
 *
 * @brief   Execute finite impulse response filter (FIR) iteration using the 
 *          following equation:
 *
 *          y = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *
 *          Internal accumulations don't saturate. The FIR filter output is within 
 *          32-bit fractional range from 0x80000000 to 0x7fffffff.
 *
 * @param   x        - Input fractional value represented in 16-bit fractional 
 *                     format "x(n)".
 * @param   *px      - Pointer to previous input values represented in 16-bit 
 *                     fractional format "x(n-1) ... x(n-nb)".
 * @param   *pcoef   - Pointer to filter constants represented in 32-bit fractional 
 *                     format "b(1) ... b(nb+1)".
 * @param   len      - Filter length "nb". 
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_fir_sl  (register Frac16 x, register Frac16 *px, const Frac32 *pcoef, 
                         register Word16 len);

/****************************************************************************//*!
 *
 * @brief   Execute finite impulse response filter (FIR) iteration using the 
 *          following equation:
 *
 *          y = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *
 *          Internal accumulations don't saturate. The FIR filter output is within 
 *          32-bit fractional range from 0x80000000 to 0x7fffffff.
 *
 * @param   x        - Input fractional value represented in 32-bit fractional 
 *                     format "x(n)".
 * @param   *px      - Pointer to previous input values represented in 32-bit 
 *                     fractional format "x(n-1) ... x(n-nb)".
 * @param   *pcoef   - Pointer to filter constants represented in 16-bit fractional 
 *                     format "b(1) ... b(nb+1)".
 * @param   len      - Filter length "nb". 
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_fir_ls  (register Frac32 x, register Frac32 *px, const Frac16 *pcoef, 
                         register Word16 len);

/****************************************************************************//*!
 *
 * @brief   Execute finite impulse response filter (FIR) iteration using the 
 *          following equation:
 *
 *          y = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *
 *          Internal accumulations don't saturate. The FIR filter output is within 
 *          32-bit fractional range from 0x80000000 to 0x7fffffff.
 *
 * @param   x        - Input fractional value represented in 32-bit fractional 
 *                     format "x(n)".
 * @param   *px      - Pointer to previous input values represented in 32-bit 
 *                     fractional format "x(n-1) ... x(n-nb)".
 * @param   *pcoef   - Pointer to filter constants represented in 32-bit fractional 
 *                     format "b(1) ... b(nb+1)".
 * @param   len      - Filter length "nb". 
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_fir     (register Frac32 x, register Frac32 *px, const Frac32 *pcoef, 
                         register Word16 len);

/****************************************************************************//*!
 *
 * @brief   Execute finite impulse response filter (FIR) iteration using the 
 *          following equation:
 *
 *          y = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *
 *          Internal accumulations don't saturate. The FIR filter output is within 
 *          64-bit fractional range from 0x8000000000000000 to 0x7ffffffffffffff.
 *
 * @param   x        - Input fractional value represented in 32-bit fractional 
 *                     format "x(n)".
 * @param   *px      - Pointer to previous input values represented in 32-bit 
 *                     fractional format "x(n-1) ... x(n-nb)".
 * @param   *pcoef   - Pointer to filter constants represented in 32-bit fractional 
 *                     format "b(1) ... b(nb+1)".
 * @param   len      - Filter length "nb". 
 *
 * @return  Function returns 64-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac64 LL_fir_ll (register Frac32 x, register Frac32 *px, const Frac32 *pcoef, 
                         register Word16 len);

/****************************************************************************//*!
 *
 * @brief   Execute finite impulse response filter (FIR) iteration using the 
 *          following equation:
 *
 *          y = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
 *
 *          Internal accumulations don't saturate. The FIR filter output is within 
 *          64-bit fractional range from 0x8000000000000000 to 0x7ffffffffffffff.
 *
 * @param   x        - Input fractional value represented in 64-bit fractional 
 *                     format "x(n)".
 * @param   *px      - Pointer to previous input values represented in 64-bit 
 *                     fractional format "x(n-1) ... x(n-nb)".
 * @param   *pcoef   - Pointer to filter constants represented in 32-bit fractional 
 *                     format "b(1) ... b(nb+1)".
 * @param   len      - Filter length "nb". 
 *
 * @return  Function returns 64-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac64 LL_fir_lll(register Frac64 x, register Frac64 *px, const Frac32 *pcoef, 
                         register Word16 len);

/****************************************************************************//*!
 *
 * @brief   Execute nth order infinite impulse response filter (IIR) iteration 
 *          using the following equation:
 *
 *          y(n) = b(1)*x(n) + b(2)*x(n-1) - a(na+1)*y(n-na)
 *
 *          Internal accumulations don't saturate. The IIR filter output is within 
 *          32-bit fractional range from 0x80000000 to 0x7fffffff..
 *
 * @param   x       - Input fractional value represented in 16-bit fractional 
 *                    format "x(n)".
 * @param   *px     - Pointer to previous input values represented in 16-bit 
 *                    fractional format "x(n-1) ... x(n-nb)".
 * @param   len_x   - Filter length "nb".
 * @param   *py     - Pointer to previous output values represented in 16-bit 
 *                    fractional format "y(n-1) ... y(n-na)".
 * @param   len_y   - Filter length "na".
 * @param   *pcoef  - Pointer to filter constants represented in 16-bit fractional 
 *                    format "b(1) ... b(nb+1), a(2) ... a(na+1)".
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_iir_ss  (register Frac16 x, register Frac16 *px, register Word16 len_x, 
                         register Frac32 *py, register Word16 len_y, const Frac16 *pcoef);

/****************************************************************************//*!
 *
 * @brief   Execute nth order infinite impulse response filter (IIR) iteration 
 *          using the following equation:
 *
 *          y(n) = b(1)*x(n) + b(2)*x(n-1) - a(na+1)*y(n-na)
 *
 *          Internal accumulations don't saturate. The IIR filter output is within 
 *          32-bit fractional range from 0x80000000 to 0x7fffffff..
 *
 * @param   x       - Input fractional value represented in 32-bit fractional 
 *                    format "x(n)".
 * @param   *px     - Pointer to previous input values represented in 32-bit 
 *                    fractional format "x(n-1) ... x(n-nb)".
 * @param   len_x   - Filter length "nb".
 * @param   *py     - Pointer to previous output values represented in 32-bit 
 *                    fractional format "y(n-1) ... y(n-na)".
 * @param   len_y   - Filter length "na".
 * @param   *pcoef  - Pointer to filter constants represented in 32-bit fractional 
 *                    format "b(1) ... b(nb+1), a(2) ... a(na+1)".
 *
 * @return  Function returns 32-bit value in fractional format.
 *
 * @remarks Implementation in C-language.
 *
 *******************************************************************************/
extern Frac32 L_iir     (register Frac32 x, register Frac32 *px, register Word16 len_x, 
                         register Frac32 *py, register Word16 len_y, const Frac32 *pcoef);
                
#endif /* __FRACLIB_H */
