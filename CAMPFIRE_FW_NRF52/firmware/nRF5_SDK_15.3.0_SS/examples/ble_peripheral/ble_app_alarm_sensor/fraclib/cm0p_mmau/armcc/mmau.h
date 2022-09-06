/******************************************************************************
 * (c) Copyright 2014, Freescale Semiconductor Inc.
 * ALL RIGHTS RESERVED.
 ***************************************************************************//*!
 * @file      mmau.h
 * @version   1.0.0.0
 * @date      Jul-23-2014
 * @brief     Memory Mapped Arithmetic Unit (MMAU) software driver header file
 ******************************************************************************/
#ifndef __MMAU_H
#define __MMAU_H

/******************************************************************************
 * MMAU memory map for use with operations                                    *
 ******************************************************************************/
/* Registers decorated load/store addresses                                   */
#define MMAU__X0        0xF0004000l /* Accumulator register X0                */
#define MMAU__X1        0xF0004004l /* Accumulator register X1                */
#define MMAU__X2        0xF0004008l /* Accumulator register X2                */
#define MMAU__X3        0xF000400Cl /* Accumulator register X3                */
#define MMAU__A0        0xF0004010l /* Accumulator register A0                */
#define MMAU__A1        0xF0004014l /* Accumulator register A1                */
#define MMAU__A10       0xF0004010l /* Accumulator register pair A10          */
/* Unsigned integer instructions decorated load/store addresses               */
#define MMAU__REGRW     0xF0004000l /* Registers RW                           */
#define MMAU__UMUL      0xF0004020l /* A10=X2*X3                              */
#define MMAU__UMULD     0xF0004040l /* A10=X21*X3                             */
#define MMAU__UMULDA    0xF0004060l /* A10=A10*X3                             */
#define MMAU__UMAC      0xF00040A0l /* A10=X2*X3+A10                          */
#define MMAU__UMACD     0xF00040C0l /* A10=X21*X3+A10                         */
#define MMAU__UMACDA    0xF00040E0l /* A10=A10*X3+X21                         */
#define MMAU__UDIV      0xF0004120l /* X21/X3=A10                             */
#define MMAU__UDIVD     0xF0004140l /* A10=X2/X3                              */
#define MMAU__UDIVDA    0xF0004160l /* A10=X21/X3                             */
#define MMAU__UDIVDD    0xF0004180l /* A10=A10/X3                             */
#define MMAU__UDIVDDA   0xF00041A0l /* A10=A10/X32                            */
#define MMAU__USQR      0xF0004220l /* A10=SQR(X3)                            */
#define MMAU__USQRD     0xF0004240l /* A10=SQR(X32)                           */
#define MMAU__USQRDA    0xF0004260l /* A10=SQR(A10)                           */
/* Signed fractional instructions decorated load/store addresses              */
#define MMAU__QSQR      0xF00042A0l /* A10=SQR(X3)                            */
#define MMAU__QSQRD     0xF00042C0l /* A10=SQR(X32)                           */
#define MMAU__QSQRDA    0xF00042E0l /* A10=SQR(A10)                           */
#define MMAU__QDIV      0xF0004320l /* A10=X2/X3                              */
#define MMAU__QDIVD     0xF0004340l /* A10=X21/X3                             */
#define MMAU__QDIVDA    0xF0004360l /* A10=A10/X3                             */
#define MMAU__QMUL      0xF0004420l /* A10=X2*X3                              */
#define MMAU__QMULD     0xF0004440l /* A10=X21*X3                             */
#define MMAU__QMULDA    0xF0004460l /* A10=A10*X3                             */
#define MMAU__QMAC      0xF00044A0l /* A10=X2*X3+A10                          */
#define MMAU__QMACD     0xF00044C0l /* A10=X21*X3+A10                         */
#define MMAU__QMACDA    0xF00044E0l /* A10=A10*X3+X21                         */
/* Signed integer instructions decorated load/store addresses                 */
#define MMAU__SMUL      0xF0004620l /* A10=X2*X3                              */
#define MMAU__SMULD     0xF0004640l /* A10=X21*X3                             */
#define MMAU__SMULDA    0xF0004660l /* A10=A10*X3                             */
#define MMAU__SMAC      0xF00046A0l /* A10=X2*X3+A10                          */
#define MMAU__SMACD     0xF00046C0l /* A10=X21*X3+A10                         */
#define MMAU__SMACDA    0xF00046E0l /* A10=A10*X3+X21                         */
#define MMAU__SDIV      0xF0004720l /* A10=X2/X3                              */
#define MMAU__SDIVD     0xF0004740l /* A10=X21/X3                             */
#define MMAU__SDIVDA    0xF0004760l /* A10=A10/X3                             */
#define MMAU__SDIVDD    0xF0004780l /* A10=X10/X32                            */
#define MMAU__SDIVDDA   0xF00047A0l /* A10=A10/X32                            */
/* Auxiliary decorated load/store addresses                                   */
#define MMAU__SAT       0xF0004800l /* Saturation                             */

/******************************************************************************
 * MMAU internal state definition                                             *
 ******************************************************************************/
typedef struct { uint32 x0, x1, x2, x3, a0, a1, csr; } tMMAU_STATE;

/******************************************************************************
 * MMAU instruction status flags definitions used by MMAU_GetInstrFlags() and
 * MMAU_SetInstrFlags() macros
 *
 *//*! @addtogroup mmau_instruction_flags
 * @{
 * List of instruction flags. These flags are updated on each calculation and
 * can be set/cleared by the @ref MMAU_WriteInstrFlags macro.
 ******************************************************************************/
#define MMAU_Q  (1l<<0)   ///< Accumulation Overflow
#define MMAU_V  (1l<<1)   ///< Multiply or Divide Overflow
#define MMAU_DZ (1l<<2)   ///< Divide by Zero
#define MMAU_N  (1l<<3)   ///< Signed calculation result is negative
/*! @} End of mmau_instruction_flags                                          */

/******************************************************************************
 * MMAU interrupt flags definitions used by MMAU_GetIntFlags() and
 * MMAU_ClrIntFlags() macros
 *
 *//*! @addtogroup mmau_interrupt_flags
 * @{
 * List of interrupt flags. These flags are sticky version of the @ref
 * mmau_instruction_flags. They can only be set by MMAU execution of
 * instructions and cleared by the @ref MMAU_ClrIntFlags macro.
 ******************************************************************************/
#define MMAU_QIF  (1l<<4)   ///< Accumulation Overflow Interrupt Flag
#define MMAU_VIF  (1l<<5)   ///< Multiply or Divide Overflow Interrupt Flag
#define MMAU_DZIF (1l<<6)   ///< Divide by Zero Interrupt Flag
/*! @} End of mmau_interrupt_flags                                            */

/******************************************************************************
 * MMAU configuration structure definitions                                   *
 ******************************************************************************/
typedef struct { uint32 CR; } tMMAU;

/******************************************************************************
 * MMAU default configurations used by MMAU_InitCallback() function
 *
 *//*! @addtogroup mmau_callback_config
 * @{
 ******************************************************************************/
/***************************************************************************//*!
 * @brief   Disable all interrupts of the MMAU module.
 * @details This configuration structure configures interrupts of the MMAU as
 *          follows:
 *          - Divide-by-Zero Interrupt disabled.
 *          - Divide/Multiply Overflow (V flag) Interrupt disabled.
 *          - Accumulation Overflow (Q flag) Interrupt disabled.
 * @showinitializer
 ******************************************************************************/
#define MMAU_DZIE_DI_VIE_DI_QIE_DI_CONFIG                                      \
(tMMAU){                                                                       \
/* CR */ CLR(MMAU_CSR_DZIE_MASK)|CLR(MMAU_CSR_VIE_MASK)|CLR(MMAU_CSR_QIE_MASK) \
}

/***************************************************************************//*!
 * @brief   Disable all interrupts of the MMAU module.
 * @details This configuration structure configures interrupts of the MMAU as
 *          follows:
 *          - Divide-by-Zero Interrupt enabled.
 *          - Divide/Multiply Overflow (V flag) Interrupt disabled.
 *          - Accumulation Overflow (Q flag) Interrupt disabled.
 * @showinitializer
 ******************************************************************************/
#define MMAU_DZIE_EN_VIE_DI_QIE_DI_CONFIG                                      \
(tMMAU){                                                                       \
/* CR */ SET(MMAU_CSR_DZIE_MASK)|CLR(MMAU_CSR_VIE_MASK)|CLR(MMAU_CSR_QIE_MASK) \
}

/***************************************************************************//*!
 * @brief   Disable all interrupts of the MMAU module.
 * @details This configuration structure configures interrupts of the MMAU as
 *          follows:
 *          - Divide-by-Zero Interrupt disabled.
 *          - Divide/Multiply Overflow (V flag) Interrupt enabled.
 *          - Accumulation Overflow (Q flag) Interrupt disabled.
 * @showinitializer
 ******************************************************************************/
#define MMAU_DZIE_DI_VIE_EN_QIE_DI_CONFIG                                      \
(tMMAU){                                                                       \
/* CR */ CLR(MMAU_CSR_DZIE_MASK)|SET(MMAU_CSR_VIE_MASK)|CLR(MMAU_CSR_QIE_MASK) \
}

/***************************************************************************//*!
 * @brief   Disable all interrupts of the MMAU module.
 * @details This configuration structure configures interrupts of the MMAU as
 *          follows:
 *          - Divide-by-Zero Interrupt enabled.
 *          - Divide/Multiply Overflow (V flag) Interrupt enabled.
 *          - Accumulation Overflow (Q flag) Interrupt disabled.
 * @showinitializer
 ******************************************************************************/
#define MMAU_DZIE_EN_VIE_EN_QIE_DI_CONFIG                                      \
(tMMAU){                                                                       \
/* CR */ SET(MMAU_CSR_DZIE_MASK)|SET(MMAU_CSR_VIE_MASK)|CLR(MMAU_CSR_QIE_MASK) \
}

/***************************************************************************//*!
 * @brief   Disable all interrupts of the MMAU module.
 * @details This configuration structure configures interrupts of the MMAU as
 *          follows:
 *          - Divide-by-Zero Interrupt disabled.
 *          - Divide/Multiply Overflow (V flag) Interrupt disabled.
 *          - Accumulation Overflow (Q flag) Interrupt enabled.
 * @showinitializer
 ******************************************************************************/
#define MMAU_DZIE_DI_VIE_DI_QIE_EN_CONFIG                                      \
(tMMAU){                                                                       \
/* CR */ CLR(MMAU_CSR_DZIE_MASK)|CLR(MMAU_CSR_VIE_MASK)|SET(MMAU_CSR_QIE_MASK) \
}

/***************************************************************************//*!
 * @brief   Disable all interrupts of the MMAU module.
 * @details This configuration structure configures interrupts of the MMAU as
 *          follows:
 *          - Divide-by-Zero Interrupt enabled.
 *          - Divide/Multiply Overflow (V flag) Interrupt disabled.
 *          - Accumulation Overflow (Q flag) Interrupt enabled.
 * @showinitializer
 ******************************************************************************/
#define MMAU_DZIE_EN_VIE_DI_QIE_EN_CONFIG                                      \
(tMMAU){                                                                       \
/* CR */ SET(MMAU_CSR_DZIE_MASK)|CLR(MMAU_CSR_VIE_MASK)|SET(MMAU_CSR_QIE_MASK) \
}

/***************************************************************************//*!
 * @brief   Disable all interrupts of the MMAU module.
 * @details This configuration structure configures interrupts of the MMAU as
 *          follows:
 *          - Divide-by-Zero Interrupt disabled.
 *          - Divide/Multiply Overflow (V flag) Interrupt enabled.
 *          - Accumulation Overflow (Q flag) Interrupt enabled.
 * @showinitializer
 ******************************************************************************/
#define MMAU_DZIE_DI_VIE_EN_QIE_EN_CONFIG                                      \
(tMMAU){                                                                       \
/* CR */ CLR(MMAU_CSR_DZIE_MASK)|SET(MMAU_CSR_VIE_MASK)|SET(MMAU_CSR_QIE_MASK) \
}

/***************************************************************************//*!
 * @brief   Enable all interrupts of the MMAU module.
 * @details This configuration structure configures interrupts of the MMAU as
 *          follows:
 *          - Divide-by-Zero Interrupt enabled.
 *          - Divide/Multiply Overflow (V flag) Interrupt enabled.
 *          - Accumulation Overflow (Q flag) Interrupt enabled.
 * @showinitializer
 ******************************************************************************/
#define MMAU_DZIE_EN_VIE_EN_QIE_EN_CONFIG                                      \
(tMMAU){                                                                       \
/* CR */ SET(MMAU_CSR_DZIE_MASK)|SET(MMAU_CSR_VIE_MASK)|SET(MMAU_CSR_QIE_MASK) \
}
/*! @} End of mmau_callback_config                                            */

/******************************************************************************
 * MMAU callback registered by the MMAU_InstallCallback() function
 *
 *//*! @addtogroup mmau_callback
 * @{
 ******************************************************************************/
/*! @brief MMAU_CALLBACK type declaration                                     */
/*! MMAU_CALLBACK type declaration                                            */
typedef enum
{
  DZIF_CALLBACK =1, ///< For divide, divide-by-zero error
  VIF_CALLBACK  =2, ///< Product in MUL, MAC or quotient of a divide overflows
  QIF_CALLBACK  =4  ///< Accumulation overflows during a MAC instruction
} MMAU_CALLBACK_TYPE;

/*! @brief MMAU_CALLBACK function declaration                                 */
typedef void (*MMAU_CALLBACK)(MMAU_CALLBACK_TYPE type);
/*! @} End of mmau_callback                                                   */

/******************************************************************************
 * MMAU instruction macros.
 *
 *//*! @addtogroup mmau_macros
 * @{
 ******************************************************************************/
/***************************************************************************//*!
 * @brief   DMA request enable/disable.
 * @details Call @ref MMAU_EnableDMA macro to configure MMAU to allow (true) or
 *          prevent (false) the DMA request to fetch the result and program new
 *          computation instruction.
 * @param   mode      Enable (true) or disable (false) DMA request generation.
 ******************************************************************************/
#define MMAU_EnableDMA(mode)  { MMAU_CSR=((MMAU_CSR&~(1ul<<16))|((mode)<<16)); }

/***************************************************************************//*!
 * @brief   Sets CPU/DMA access to MMAU Operand, Accumulator and Control/Status
 *          registers.
 * @details Call @ref MMAU_SetAccess macro to set CPU/DMA access mode to MMAU
 *          Operand, Accumulator and Control/Status registers. MMAU registers
 *          can either be accessed in Supervisor Mode (true) or in both User and
 *          Supervisor Modes (false). In Supervisor Mode, when CPU/DMA access
 *          registers in User Mode, MMAU will terminate the access with an bus
 *          error.
 * @param   mode      Supervisor Mode (true) or both User and Supervisor Modes
 *                    (false).
 ******************************************************************************/
#define MMAU_SetAccess(mode)  { MMAU_CSR=((MMAU_CSR&~(1ul<<17))|((mode)<<17)); }

/***************************************************************************//*!
 * @brief   Get instruction result flags.
 * @details Call @ref MMAU_GetInstrFlags macro to get instruction result flags.
 * @return  @ref uint32 @ref mmau_instruction_flags.
 * @see     @ref MMAU_WriteInstrFlags.
 ******************************************************************************/
#define MMAU_GetInstrFlags()  (MMAU_CSR&(MMAU_Q|MMAU_V|MMAU_DZ|MMAU_N))

/***************************************************************************//*!
 * @brief   Write instruction result flags.
 * @details Call @ref MMAU_WriteInstrFlags macro to write instruction result
 *          flags.
 * @param   mask      Select one or more OR'ed @ref mmau_instruction_flags.
 * @see     @ref MMAU_GetInstrFlags.
 ******************************************************************************/
#define MMAU_WriteInstrFlags(mask)  { MMAU_CSR_IF_CLR=(mask); }

/***************************************************************************//*!
 * @brief   Get interrupt flags.
 * @details Call @ref MMAU_GetIntFlags macro to get interrupt flags.
 * @return  @ref uint32 @ref mmau_interrupt_flags.
 * @see     @ref MMAU_ClrIntFlags.
 ******************************************************************************/
#define MMAU_GetIntFlags()          (MMAU_CSR&(MMAU_QIF|MMAU_VIF|MMAU_DZIF))

/***************************************************************************//*!
 * @brief   Clear interrupt flags.
 * @details Call @ref MMAU_ClrIntFlags macro to clear interrupt flags.
 * @param   mask      Select one or more OR'ed @ref mmau_interrupt_flags.
 * @see     @ref MMAU_GetIntFlags.
 ******************************************************************************/
#define MMAU_ClrIntFlags(mask)      { MMAU_CSR_IF_CLR=(mask); }

#pragma diag_suppress 1287
#define store_state(p)                                                         \
{                                                                              \
  register uint32 _src = (uint32)(MMAU__REGRW|MMAU__X0);                       \
  register uint32 _dst = (uint32)p;                                            \
  __asm volatile                                                               \
  (                                                                            \
    "ldm _src ,{_src,r2,r3,r4,r5,r6,r7}\n"                                     \
    "stm _dst!,{_src,r2,r3,r4,r5,r6,r7}\n"                                     \
  );                                                                           \
}
/***************************************************************************//*!
 * @brief   Store MMAU internal state to the software stack.
 * @details The @ref MMAU_StoreState function stores MMAU internal state
 *          including operand, accumulator and control/status registers to the
 *          software stack.
 * @note    Call this function at entry point of any interrupt service routine
 *          which uses @ref mmau_macros. At the exit of such interrupt service
 *          routine you should call @ref MMAU_RestoreState function.
 * @see     @ref MMAU_RestoreState
 ******************************************************************************/
#define MMAU_StoreState() tMMAU_STATE volatile __tmp; store_state(&__tmp)

#pragma diag_suppress 1287
#define restore_state(p)                                                       \
{                                                                              \
  register uint32 _src = (uint32)p;                                            \
  register uint32 _dst = (uint32)(MMAU__REGRW|MMAU__X0);                       \
  __asm volatile                                                               \
  (                                                                            \
    "ldm _src ,{_src,r2,r3,r4,r5,r6,r7}\n"                                     \
    "stm _dst!,{_src,r2,r3,r4,r5,r6,r7}\n"                                     \
  );                                                                           \
}
/***************************************************************************//*!
 * @brief   Restore MMAU internal state from the software stack.
 * @details The @ref MMAU_RestoreState function restores MMAU internal state
 *          including operand, accumulator and control/status registers from the
 *          software stack.
 * @note    Call this function at exit of any interrupt service routine
 *          which uses @ref mmau_macros. At entry point of such interrupt
 *          service routine you should call @ref MMAU_StoreState function.
 * @see     @ref MMAU_StoreState
 ******************************************************************************/
#define MMAU_RestoreState() restore_state(&__tmp)

/***************************************************************************//*!
 * @brief   Installs callback function for MMAU interrupt vector 36.
 * @details This function installs callback function for MMAU interrupt vector
 *          30.
 * @param   cfg       Select one of the @ref mmau_callback_config.
 * @param   ip        Select one of the Select one of the @ref cm0plus_prilvl.
 * @param   callback  Pointer to the @ref mmau_callback.
 * @note    Implemented as a function call.
 ******************************************************************************/
#define MMAU_InstallCallback(cfg,ip,callback)                                  \
                                          MMAU_InstallCallback(cfg,ip,callback)
/*! @} End of mmau_macros                                                     */

/******************************************************************************
 * public function prototypes                                                 *
 ******************************************************************************/
extern void MMAU_InstallCallback(tMMAU cfg, uint8 ip, MMAU_CALLBACK pCallback);

/******************************************************************************
 * interrupt function prototypes                                              *
 ******************************************************************************/
extern void mmau_isr (void);

/******************************************************************************
 * MMAU instruction set.
 *
 *//*! @addtogroup uint_instructions
 * @{
 ******************************************************************************/

/***************************************************************************//*!
 * @brief   Load A10 accumulator register of the MMAU by 64-bit unsigned value.
 * @details The @ref ulda_d function loads A10 accumulator register of the MMAU
 *          by 64-bit unsigned value.
 * @param   dval    @ref uint64 unsigned value.
 ******************************************************************************/
static inline void ulda_d (register uint64 dval)
{
  *((uint64 volatile *)(MMAU__REGRW|MMAU__A10))= dval;
}

/***************************************************************************//*!
 * @brief   Read 32-bit unsigned value from the A0 accumulator register of the
 *          MMAU.
 * @details The @ref l_urda function reads 32-bit unsigned value from the A0
 *          accumulator register of the MMAU.
 * @return  @ref uint32 unsigned value.
 ******************************************************************************/
static inline uint32 l_urda (void)
{
  return *((uint32 volatile *)(MMAU__REGRW|MMAU__A0));
}

/***************************************************************************//*!
 * @brief   Read 64-bit unsigned value from the A10 accumulator register of the
 *          MMAU.
 * @details The @ref d_urda function reads 64-bit unsigned value from the A10
 *          accumulator register of the MMAU.
 * @return  @ref uint64 unsigned value.
 ******************************************************************************/
static inline uint64 d_urda (void)
{
  return *((uint64 volatile *)(MMAU__REGRW|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit unsigned values returning a 64-bit unsigned
 *          product.
 * @details The @ref d_umul_ll function multiplies two 32-bit unsigned values
 *          returning a 64-bit unsigned product.
 * @param   lval1   @ref uint32 unsigned value.
 * @param   lval2   @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_umul_ll (register uint32 lval1, register uint32 lval2)
{
  *((uint32 volatile *)(MMAU__UMUL|MMAU__X2))= lval1;
  *((uint32 volatile *)(MMAU__UMUL|MMAU__X3))= lval2;
  return *((uint64 volatile *)(MMAU__UMUL|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit unsigned values.
 * @details The @ref umul_ll function multiplies two 32-bit unsigned values.
 * @param   lval1   @ref uint32 unsigned value.
 * @param   lval2   @ref uint32 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void umul_ll (register uint32 lval1, register uint32 lval2)
{
  *((uint32 volatile *)(MMAU__UMUL|MMAU__X2))= lval1;
  *((uint32 volatile *)(MMAU__UMUL|MMAU__X3))= lval2;
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit unsigned value with 32-bit unsigned value returning
 *          a 64-bit unsigned product.
 * @details The @ref d_umul_dl function multiplies 64-bit unsigned value with
 *          32-bit unsigned value returning a 64-bit unsigned product.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_umul_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMULD|MMAU__X1))= dval;
  *((uint32 volatile *)(MMAU__UMULD|MMAU__X3))= lval;
  return *((uint64 volatile *)(MMAU__UMULD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit unsigned value with 32-bit unsigned value
 *          returning saturated 64-bit unsigned product.
 * @details The @ref d_umuls_dl function multiplies 64-bit unsigned value with
 *          32-bit unsigned value returning saturated 64-bit unsigned product.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline uint64 d_umuls_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMULD|MMAU__X1|MMAU__SAT))= dval;
  *((uint32 volatile *)(MMAU__UMULD|MMAU__X3|MMAU__SAT))= lval;
  return *((uint64 volatile *)(MMAU__UMULD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit unsigned value with 32-bit unsigned value.
 * @details The @ref umul_dl function multiplies 64-bit unsigned value with
 *          32-bit unsigned value.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void umul_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMULD|MMAU__X1))= dval;
  *((uint32 volatile *)(MMAU__UMULD|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit unsigned value with 32-bit unsigned
 *          value.
 * @details The @ref umuls_dl function multiplies 64-bit unsigned value with
 *          32-bit unsigned value.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void umuls_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMULD|MMAU__X1|MMAU__SAT))= dval;
  *((uint32 volatile *)(MMAU__UMULD|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit unsigned value with 64-bit unsigned value stored in
 *          the A10 register of the MMAU returning a 64-bit unsigned product.
 * @details The @ref d_umula_l function multiplies 32-bit unsigned value with
 *          64-bit unsigned value stored in the A10 register of the MMAU
 *          returning a 64-bit unsigned product.
 * @param   lval    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_umula_l (register uint32 lval)
{
  *((uint32 volatile *)(MMAU__UMULDA|MMAU__X3))= lval;
  return *((uint64 volatile *)(MMAU__UMULDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit unsigned value with 64-bit unsigned value
 *          stored in the A10 register of the MMAU returning saturated 64-bit
 *          unsigned product.
 * @details The @ref d_umulas_l function multiplies 32-bit unsigned value with
 *          64-bit unsigned value stored in the A10 register of the MMAU
 *          returning saturated 64-bit unsigned product.
 * @param   lval    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline uint64 d_umulas_l (register uint32 lval)
{
  *((uint32 volatile *)(MMAU__UMULDA|MMAU__X3|MMAU__SAT))= lval;
  return *((uint64 volatile *)(MMAU__UMULDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit unsigned value with 64-bit unsigned value stored in
 *          the A10 register of the MMAU.
 * @details The @ref umula_l function multiplies 32-bit unsigned value with
 *          64-bit unsigned value stored in the A10 register of the MMAU.
 * @param   lval    @ref uint32 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void umula_l (register uint32 lval)
{
  *((uint32 volatile *)(MMAU__UMULDA|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit unsigned value with 64-bit unsigned value
 *          stored in the A10 register of the MMAU.
 * @details The @ref umulas_l function multiplies 32-bit unsigned value with
 *          64-bit unsigned value stored in the A10 register of the MMAU.
 * @param   lval    @ref uint32 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void umulas_l (register uint32 lval)
{
  *((uint32 volatile *)(MMAU__UMULDA|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit unsigned values and add product with value
 *          stored in the A10 register of the MMAU returning a 64-bit unsigned
 *          A10 register value.
 * @details The @ref d_umac_ll function multiplies two 32-bit unsigned values
 *          and add product with value stored in the A10 register of the MMAU
 *          returning a 64-bit unsigned A10 register value.
 * @param   lval1   @ref uint32 unsigned value.
 * @param   lval2   @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_umac_ll (register uint32 lval1, register uint32 lval2)
{
  *((uint32 volatile *)(MMAU__UMAC|MMAU__X2))= lval1;
  *((uint32 volatile *)(MMAU__UMAC|MMAU__X3))= lval2;
  return *((uint64 volatile *)(MMAU__UMAC|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit unsigned values and add product with
 *          value stored in the A10 register of the MMAU returning a 64-bit
 *          unsigned A10 register value.
 * @details The @ref d_umacs_ll function multiplies two 32-bit unsigned values
 *          and add product with value stored in the A10 register of the MMAU
 *          returning saturated 64-bit unsigned A10 register value.
 * @param   lval1   @ref uint32 unsigned value.
 * @param   lval2   @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline uint64 d_umacs_ll (register uint32 lval1, register uint32 lval2)
{
  *((uint32 volatile *)(MMAU__UMAC|MMAU__X2|MMAU__SAT))= lval1;
  *((uint32 volatile *)(MMAU__UMAC|MMAU__X3|MMAU__SAT))= lval2;
  return *((uint64 volatile *)(MMAU__UMAC|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit unsigned values and add product with value
 *          stored in the A10 register of the MMAU.
 * @details The @ref umac_ll function multiplies two 32-bit unsigned values and
 *          add product with value stored in the A10 register of the MMAU.
 * @param   lval1   @ref uint32 unsigned value.
 * @param   lval2   @ref uint32 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void umac_ll (register uint32 lval1, register uint32 lval2)
{
  *((uint32 volatile *)(MMAU__UMAC|MMAU__X2))= lval1;
  *((uint32 volatile *)(MMAU__UMAC|MMAU__X3))= lval2;
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit unsigned values and add product with
 *          value stored in the A10 register of the MMAU.
 * @details The @ref umacs_ll function multiplies two 32-bit unsigned values and
 *          add product with value stored in the A10 register of the MMAU.
 * @param   lval1   @ref uint32 unsigned value.
 * @param   lval2   @ref uint32 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void umacs_ll (register uint32 lval1, register uint32 lval2)
{
  *((uint32 volatile *)(MMAU__UMAC|MMAU__X2|MMAU__SAT))= lval1;
  *((uint32 volatile *)(MMAU__UMAC|MMAU__X3|MMAU__SAT))= lval2;
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit unsigned value with 32-bit unsigned value and add
 *          product with value stored in the A10 register of the MMAU returning
 *          a 64-bit unsigned A10 register value.
 * @details The @ref d_umac_dl function multiplies 64-bit unsigned value with
 *          32-bit unsigned value and add product with value stored in the A10
 *          register of the MMAU returning a 64-bit unsigned A10 register value.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_umac_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMACD|MMAU__X1))= dval;
  *((uint32 volatile *)(MMAU__UMACD|MMAU__X3))= lval;
  return *((uint64 volatile *)(MMAU__UMACD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit unsigned value with 32-bit unsigned value
 *          and add product with value stored in the A10 register of the MMAU
 *          returning saturated 64-bit unsigned A10 register value.
 * @details The @ref d_umacs_dl function multiplies 64-bit unsigned value with
 *          32-bit unsigned value and add product with value stored in the A10
 *          register of the MMAU returning saturated 64-bit unsigned A10
 *          register value.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline uint64 d_umacs_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMACD|MMAU__X1|MMAU__SAT))= dval;
  *((uint32 volatile *)(MMAU__UMACD|MMAU__X3|MMAU__SAT))= lval;
  return *((uint64 volatile *)(MMAU__UMACD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit unsigned value with 32-bit unsigned value and add
 *          product with value stored in the A10 register of the MMAU.
 * @details The @ref umac_dl function multiplies 64-bit unsigned value with
 *          32-bit unsigned value and add product with value stored in the A10
 *          register of the MMAU.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void umac_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMACD|MMAU__X1))= dval;
  *((uint32 volatile *)(MMAU__UMACD|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit unsigned value with 32-bit unsigned value
 *          and add product with value stored in the A10 register of the MMAU.
 * @details The @ref umacs_dl function multiplies 64-bit unsigned value with
 *          32-bit unsigned value and add product with value stored in the A10
 *          register of the MMAU.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void umacs_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMACD|MMAU__X1|MMAU__SAT))= dval;
  *((uint32 volatile *)(MMAU__UMACD|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit unsigned value by value stored in the A10 register
 *          of the MMAU and add product with 64-bit unsigned value returning a
 *          64-bit unsigned A10 register value.
 * @details The @ref d_umaca_dl function multiplies 32-bit unsigned value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit unsigned value returning a 64-bit unsigned A10 register
 *          value.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_umaca_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMACDA|MMAU__X1))= dval;
  *((uint32 volatile *)(MMAU__UMACDA|MMAU__X3))= lval;
  return *((uint64 volatile *)(MMAU__UMACDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit unsigned value by value stored in the A10
 *          register of the MMAU and add product with 64-bit unsigned value
 *          returning a saturated 64-bit unsigned A10 register value.
 * @details The @ref d_umacas_dl function multiplies 32-bit unsigned value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit unsigned value returning saturated 64-bit unsigned A10
 *          register value.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline uint64 d_umacas_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMACDA|MMAU__X1|MMAU__SAT))= dval;
  *((uint32 volatile *)(MMAU__UMACDA|MMAU__X3|MMAU__SAT))= lval;
  return *((uint64 volatile *)(MMAU__UMACDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit unsigned value by value stored in the A10 register
 *          of the MMAU and add product with 64-bit unsigned value.
 * @details The @ref umaca_dl function multiplies 32-bit unsigned value by value
 *          stored in the A10 register of the MMAU and add product with 64-bit
 *          unsigned value.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void umaca_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMACDA|MMAU__X1))= dval;
  *((uint32 volatile *)(MMAU__UMACDA|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit unsigned value by value stored in the A10
 *          register of the MMAU and add product with 64-bit unsigned value.
 * @details The @ref umacas_dl function multiplies 32-bit unsigned value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit unsigned value.
 * @param   dval    @ref uint64 unsigned value.
 * @param   lval    @ref uint32 unsigned value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void umacas_dl (register uint64 dval, register uint32 lval)
{
  *((uint64 volatile *)(MMAU__UMACDA|MMAU__X1|MMAU__SAT))= dval;
  *((uint32 volatile *)(MMAU__UMACDA|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit unsigned values returning a 32-bit unsigned
 *          quotient.
 * @details The @ref l_udiv_ll function divides two 32-bit unsigned values
 *          returning a 32-bit unsigned quotient.
 * @param   lnum    @ref uint32 unsigned value.
 * @param   lden    @ref uint32 unsigned value.
 * @return  @ref uint32 unsigned value.
 * @note    Quotient is stored in A0 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint32 l_udiv_ll (register uint32 lnum, register uint32 lden)
{
  *((uint32 volatile *)(MMAU__UDIV|MMAU__X2))= lnum;
  *((uint32 volatile *)(MMAU__UDIV|MMAU__X3))= lden;
  return *((uint32 volatile *)(MMAU__UDIV|MMAU__A0));
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit unsigned values.
 * @details The @ref udiv_ll function divides two 32-bit unsigned values.
 * @param   lnum    @ref uint32 unsigned value.
 * @param   lden    @ref uint32 unsigned value.
 * @note    Quotient is stored in A0 register of the MMAU for next computation.
 ******************************************************************************/
static inline void udiv_ll (register uint32 lnum, register uint32 lden)
{
  *((uint32 volatile *)(MMAU__UDIV|MMAU__X2))= lnum;
  *((uint32 volatile *)(MMAU__UDIV|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit unsigned value by 32-bit unsigned value returning a
 *          64-bit unsigned quotient.
 * @details The @ref d_udiv_dl function divides 64-bit unsigned value by 32-bit
 *          unsigned value returning a 64-bit unsigned quotient.
 * @param   dnum    @ref uint64 unsigned value.
 * @param   lden    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_udiv_dl (register uint64 dnum, register uint32 lden)
{
  *((uint64 volatile *)(MMAU__UDIVD|MMAU__X1))= dnum;
  *((uint32 volatile *)(MMAU__UDIVD|MMAU__X3))= lden;
  return *((uint64 volatile *)(MMAU__UDIVD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit unsigned value by 32-bit unsigned value.
 * @details The @ref udiv_dl function divides 64-bit unsigned value by 32-bit
 *          unsigned value.
 * @param   dnum    @ref uint64 unsigned value.
 * @param   lden    @ref uint32 unsigned value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void udiv_dl (register uint64 dnum, register uint32 lden)
{
  *((uint64 volatile *)(MMAU__UDIVD|MMAU__X1))= dnum;
  *((uint32 volatile *)(MMAU__UDIVD|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide two 64-bit unsigned values returning a 64-bit unsigned
 *          quotient.
 * @details The @ref d_udiv_dd function divides two 64-bit unsigned values
 *          returning a 64-bit unsigned quotient.
 * @param   dnum    @ref uint64 unsigned value.
 * @param   dden    @ref uint64 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_udiv_dd (register uint64 dnum, register uint64 dden)
{
  *((uint64 volatile *)(MMAU__UDIVDD|MMAU__X0))= dnum;
  *((uint64 volatile *)(MMAU__UDIVDD|MMAU__X2))= dden;
  return *((uint64 volatile *)(MMAU__UDIVDD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide two 64-bit unsigned values.
 * @details The @ref udiv_dd function divides two 64-bit unsigned values.
 * @param   dnum    @ref uint64 unsigned value.
 * @param   dden    @ref uint64 unsigned value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void udiv_dd (register uint64 dnum, register uint64 dden)
{
  *((uint64 volatile *)(MMAU__UDIVDD|MMAU__X0))= dnum;
  *((uint64 volatile *)(MMAU__UDIVDD|MMAU__X2))= dden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit unsigned value stored in the A10 register of the MMAU
 *          by 32-bit unsigned value returning a 64-bit unsigned quotient.
 * @details The @ref d_udiva_l function divides 64-bit unsigned value stored in
 *          the A10 register of the MMAU by 32-bit unsigned value returning a
 *          64-bit unsigned quotient.
 * @param   lden    @ref uint32 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_udiva_l (register uint32 lden)
{
  *((uint32 volatile *)(MMAU__UDIVDA|MMAU__X3))= lden;
  return *((uint64 volatile *)(MMAU__UDIVDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit unsigned value stored in the A10 register of the MMAU
 *          by 32-bit unsigned value.
 * @details The @ref udiva_l function divides 64-bit unsigned value stored in
 *          the A10 register of the MMAU by 32-bit unsigned value.
 * @param   lden    @ref uint32 unsigned value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void udiva_l (register uint32 lden)
{
  *((uint32 volatile *)(MMAU__UDIVDA|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit unsigned value stored in the A10 register of the MMAU
 *          by 64-bit unsigned value returning a 64-bit unsigned quotient.
 * @details The @ref d_udiva_d function divides 64-bit unsigned value stored in
 *          the A10 register of the MMAU by 64-bit unsigned value returning a
 *          64-bit unsigned quotient.
 * @param   dden    @ref uint64 unsigned value.
 * @return  @ref uint64 unsigned value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline uint64 d_udiva_d (register uint64 dden)
{
  *((uint64 volatile *)(MMAU__UDIVDDA|MMAU__X2))= dden;
  return *((uint64 volatile *)(MMAU__UDIVDDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit unsigned value stored in the A10 register of the MMAU
 *          by 64-bit unsigned value.
 * @details The @ref udiva_d function divides 64-bit unsigned value stored in
 *          the A10 register of the MMAU by 64-bit unsigned value.
 * @param   dden    @ref uint64 unsigned value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void udiva_d (register uint64 dden)
{
  *((uint64 volatile *)(MMAU__UDIVDDA|MMAU__X2))= dden;
}

/***************************************************************************//*!
 * @brief   Compute and return a 16-bit unsigned square root of the 32-bit
 *          unsigned radicand.
 * @details The @ref s_usqr_l function computes and returns a 16-bit unsigned
 *          square root of the 32-bit unsigned radicand.
 * @param   lrad    @ref uint32 unsigned radicand.
 * @return  @ref uint16 unsigned square root.
 * @note    Square root is stored in A0 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline uint16 s_usqr_l (register uint32 lrad)
{
  *((uint32 volatile *)(MMAU__USQR|MMAU__X3))= lrad;
  return *((uint16 volatile *)(MMAU__USQR|MMAU__A0));
}

/***************************************************************************//*!
 * @brief   Compute a 16-bit unsigned square root of the 32-bit unsigned
 *          radicand.
 * @details The @ref usqr_l function computes a 16-bit unsigned square root of
 *          the 32-bit unsigned radicand.
 * @param   lrad    @ref uint32 unsigned radicand.
 * @note    Square root is stored in A0 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void usqr_l (register uint32 lrad)
{
  *((uint32 volatile *)(MMAU__USQR|MMAU__X3))= lrad;
}

/***************************************************************************//*!
 * @brief   Compute and return a 32-bit unsigned square root of the 64-bit
 *          unsigned radicand.
 * @details The @ref l_usqr_d function computes and returns a 32-bit unsigned
 *          square root of the 64-bit unsigned radicand.
 * @param   drad    @ref uint64 unsigned radicand.
 * @return  @ref uint32 unsigned square root.
 * @note    Square root is stored in A0 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline uint32 l_usqr_d (register uint64 drad)
{
  *((uint64 volatile *)(MMAU__USQRD|MMAU__X2))= drad;
  return *((uint32 volatile *)(MMAU__USQRD|MMAU__A0));
}

/***************************************************************************//*!
 * @brief   Compute a 32-bit unsigned square root of the 64-bit unsigned
 *          radicand.
 * @details The @ref usqr_d function computes a 32-bit unsigned square root of
 *          the 64-bit unsigned radicand.
 * @param   drad    @ref uint64 unsigned radicand.
 * @note    Square root is stored in A0 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void usqr_d (register uint64 drad)
{
  *((uint64 volatile *)(MMAU__USQRD|MMAU__X2))= drad;
}

/***************************************************************************//*!
 * @brief   Compute and return a 32-bit unsigned square root of the radicand
 *          stored in the A10 register of the MMAU.
 * @details The @ref l_usqra function computes and returns a 32-bit unsigned
 *          square root of the radicand stored in the A10 register of the MMAU.
 * @return  @ref uint32 unsigned square root.
 * @note    Square root is stored in A0 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline uint32 l_usqra (void)
{
  (void) *((uint32 volatile *)(MMAU__REGRW|MMAU__A0)); /* dummy read */
  return *((uint32 volatile *)(MMAU__USQRDA|MMAU__A0));
}

/*! @} End of uint_instructions                                               */

/******************************************************************************
 * MMAU instruction set.
 *
 *//*! @addtogroup int_instructions
 * @{
 ******************************************************************************/

/***************************************************************************//*!
 * @brief   Load A10 accumulator register of the MMAU by 64-bit integer value.
 * @details The @ref slda_d function loads A10 accumulator register of the MMAU
 *          by 64-bit integer value.
 * @param   dval    @ref int64 integer value.
 ******************************************************************************/
static inline void slda_d (register int64 dval)
{
  *((int64 volatile *)(MMAU__REGRW|MMAU__A10))= dval;
}

/***************************************************************************//*!
 * @brief   Read 32-bit integer value from the A0 accumulator register of the
 *          MMAU.
 * @details The @ref l_srda function reads 32-bit integer value from the A0
 *          accumulator register of the MMAU.
 * @return  @ref int32 integer value.
 ******************************************************************************/
static inline int32 l_srda (void)
{
  return *((int32 volatile *)(MMAU__REGRW|MMAU__A0));
}

/***************************************************************************//*!
 * @brief   Read 64-bit integer value from the A10 accumulator register of the
 *          MMAU.
 * @details The @ref d_srda function reads 64-bit integer value from the A10
 *          accumulator register of the MMAU.
 * @return  @ref int64 integer value.
 ******************************************************************************/
static inline int64 d_srda (void)
{
  return *((int64 volatile *)(MMAU__REGRW|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit integer values returning a 64-bit integer
 *          product.
 * @details The @ref d_smul_ll function multiplies two 32-bit integer values
 *          returning a 64-bit integer product.
 * @param   lval1   @ref int32 integer value.
 * @param   lval2   @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_smul_ll (register int32 lval1, register int32 lval2)
{
  *((int32 volatile *)(MMAU__SMUL|MMAU__X2))= lval1;
  *((int32 volatile *)(MMAU__SMUL|MMAU__X3))= lval2;
  return *((int64 volatile *)(MMAU__SMUL|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit integer values.
 * @details The @ref smul_ll function multiplies two 32-bit integer values.
 * @param   lval1   @ref int32 integer value.
 * @param   lval2   @ref int32 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void smul_ll (register int32 lval1, register int32 lval2)
{
  *((int32 volatile *)(MMAU__SMUL|MMAU__X2))= lval1;
  *((int32 volatile *)(MMAU__SMUL|MMAU__X3))= lval2;
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit integer value with 32-bit integer value returning a
 *          64-bit integer product.
 * @details The @ref d_smul_dl function multiplies 64-bit integer value with
 *          32-bit integer value returning a 64-bit integer product.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_smul_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMULD|MMAU__X1))= dval;
  *((int32 volatile *)(MMAU__SMULD|MMAU__X3))= lval;
  return *((int64 volatile *)(MMAU__SMULD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit integer value with 32-bit integer value
 *          returning saturated 64-bit integer product.
 * @details The @ref d_smuls_dl function multiplies 64-bit integer value with
 *          32-bit integer value returning saturated 64-bit integer product.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_smuls_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMULD|MMAU__X1|MMAU__SAT))= dval;
  *((int32 volatile *)(MMAU__SMULD|MMAU__X3|MMAU__SAT))= lval;
  return *((int64 volatile *)(MMAU__SMULD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit integer value with 32-bit integer value.
 * @details The @ref smul_dl function multiplies 64-bit integer value with
 *          32-bit integer value.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void smul_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMULD|MMAU__X1))= dval;
  *((int32 volatile *)(MMAU__SMULD|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit integer value with 32-bit integer value.
 * @details The @ref smuls_dl function multiplies 64-bit integer value with
 *          32-bit integer value.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void smuls_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMULD|MMAU__X1|MMAU__SAT))= dval;
  *((int32 volatile *)(MMAU__SMULD|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit integer value with 64-bit integer value stored in
 *          the A10 register of the MMAU returning a 64-bit integer product.
 * @details The @ref d_smula_l function multiplies 32-bit integer value with
 *          64-bit integer value stored in the A10 register of the MMAU
 *          returning a 64-bit integer product.
 * @param   lval    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_smula_l (register int32 lval)
{
  *((int32 volatile *)(MMAU__SMULDA|MMAU__X3))= lval;
  return *((int64 volatile *)(MMAU__SMULDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit integer value with 64-bit integer value
 *          stored in the A10 register of the MMAU returning saturated 64-bit
 *          integer product.
 * @details The @ref d_smulas_l function multiplies 32-bit integer value with
 *          64-bit integer value stored in the A10 register of the MMAU
 *          returning saturated 64-bit integer product.
 * @param   lval    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_smulas_l (register int32 lval)
{
  *((int32 volatile *)(MMAU__SMULDA|MMAU__X3|MMAU__SAT))= lval;
  return *((int64 volatile *)(MMAU__SMULDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit integer value with 64-bit integer value stored in
 *          the A10 register of the MMAU.
 * @details The @ref smula_l function multiplies 32-bit integer value with
 *          64-bit integer value stored in the A10 register of the MMAU.
 * @param   lval    @ref int32 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void smula_l (register int32 lval)
{
  *((int32 volatile *)(MMAU__SMULDA|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit integer value with 64-bit integer value
 *          stored in the A10 register of the MMAU.
 * @details The @ref smulas_l function multiplies 32-bit integer value with
 *          64-bit integer value stored in the A10 register of the MMAU.
 * @param   lval    @ref int32 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void smulas_l (register int32 lval)
{
  *((int32 volatile *)(MMAU__SMULDA|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit integer values and add product with value stored
 *          in the A10 register of the MMAU returning a 64-bit integer A10
 *          register value.
 * @details The @ref d_smac_ll function multiplies two 32-bit integer values and
 *          add product with value stored in the A10 register of the MMAU
 *          returning a 64-bit integer A10 register value.
 * @param   lval1   @ref int32 integer value.
 * @param   lval2   @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_smac_ll (register int32 lval1, register int32 lval2)
{
  *((int32 volatile *)(MMAU__SMAC|MMAU__X2))= lval1;
  *((int32 volatile *)(MMAU__SMAC|MMAU__X3))= lval2;
  return *((int64 volatile *)(MMAU__SMAC|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit integer values and add product with
 *          value stored in the A10 register of the MMAU returning a 64-bit
 *          integer A10 register value.
 * @details The @ref d_smacs_ll function multiplies two 32-bit integer values
 *          and add product with value stored in the A10 register of the MMAU
 *          returning saturated 64-bit integer A10 register value.
 * @param   lval1   @ref int32 integer value.
 * @param   lval2   @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_smacs_ll (register int32 lval1, register int32 lval2)
{
  *((int32 volatile *)(MMAU__SMAC|MMAU__X2|MMAU__SAT))= lval1;
  *((int32 volatile *)(MMAU__SMAC|MMAU__X3|MMAU__SAT))= lval2;
  return *((int64 volatile *)(MMAU__SMAC|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit integer values and add product with value stored
 *          in the A10 register of the MMAU.
 * @details The @ref smac_ll function multiplies two 32-bit integer values and
 *          add product with value stored in the A10 register of the MMAU.
 * @param   lval1   @ref int32 integer value.
 * @param   lval2   @ref int32 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void smac_ll (register int32 lval1, register int32 lval2)
{
  *((int32 volatile *)(MMAU__SMAC|MMAU__X2))= lval1;
  *((int32 volatile *)(MMAU__SMAC|MMAU__X3))= lval2;
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit integer values and add product with
 *          value stored in the A10 register of the MMAU.
 * @details The @ref smacs_ll function multiplies two 32-bit integer values and
 *          add product with value stored in the A10 register of the MMAU.
 * @param   lval1   @ref int32 integer value.
 * @param   lval2   @ref int32 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void smacs_ll (register int32 lval1, register int32 lval2)
{
  *((int32 volatile *)(MMAU__SMAC|MMAU__X2|MMAU__SAT))= lval1;
  *((int32 volatile *)(MMAU__SMAC|MMAU__X3|MMAU__SAT))= lval2;
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit integer value with 32-bit integer value and add
 *          product with value stored in the A10 register of the MMAU returning
 *          a 64-bit integer A10 register value.
 * @details The @ref d_smac_dl function multiplies 64-bit integer value with
 *          32-bit integer value and add product with value stored in the A10
 *          register of the MMAU returning a 64-bit integer A10 register value.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_smac_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMACD|MMAU__X1))= dval;
  *((int32 volatile *)(MMAU__SMACD|MMAU__X3))= lval;
  return *((int64 volatile *)(MMAU__SMACD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit integer value with 32-bit integer value
 *          and add product with value stored in the A10 register of the MMAU
 *          returning saturated 64-bit integer A10 register value.
 * @details The @ref d_smacs_dl function multiplies 64-bit integer value with
 *          32-bit integer value and add product with value stored in the A10
 *          register of the MMAU returning saturated 64-bit integer A10 register
 *          value.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_smacs_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMACD|MMAU__X1|MMAU__SAT))= dval;
  *((int32 volatile *)(MMAU__SMACD|MMAU__X3|MMAU__SAT))= lval;
  return *((int64 volatile *)(MMAU__SMACD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit integer value with 32-bit integer value and add
 *          product with value stored in the A10 register of the MMAU.
 * @details The @ref smac_dl function multiplies 64-bit integer value with
 *          32-bit integer value and add product with value stored in the A10
 *          register of the MMAU.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void smac_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMACD|MMAU__X1))= dval;
  *((int32 volatile *)(MMAU__SMACD|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit integer value with 32-bit integer value
 *          and add product with value stored in the A10 register of the MMAU.
 * @details The @ref smacs_dl function multiplies 64-bit integer value with
 *          32-bit integer value and add product with value stored in the A10
 *          register of the MMAU.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void smacs_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMACD|MMAU__X1|MMAU__SAT))= dval;
  *((int32 volatile *)(MMAU__SMACD|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit integer value by value stored in the A10 register of
 *          the MMAU and add product with 64-bit integer value returning a
 *          64-bit integer A10 register value.
 * @details The @ref d_smaca_dl function multiplies 32-bit integer value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit integer value returning a 64-bit integer A10 register value.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_smaca_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMACDA|MMAU__X1))= dval;
  *((int32 volatile *)(MMAU__SMACDA|MMAU__X3))= lval;
  return *((int64 volatile *)(MMAU__SMACDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit integer value by value stored in the A10
 *          register of the MMAU and add product with 64-bit integer value
 *          returning a saturated 64-bit integer A10 register value.
 * @details The @ref d_smacas_dl function multiplies 32-bit integer value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit integer value returning saturated 64-bit integer A10 register
 *          value.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_smacas_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMACDA|MMAU__X1|MMAU__SAT))= dval;
  *((int32 volatile *)(MMAU__SMACDA|MMAU__X3|MMAU__SAT))= lval;
  return *((int64 volatile *)(MMAU__SMACDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit integer value by value stored in the A10 register of
 *          the MMAU and add product with 64-bit integer value.
 * @details The @ref smaca_dl function multiplies 32-bit integer value by value
 *          stored in the A10 register of the MMAU and add product with 64-bit
 *          integer value.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void smaca_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMACDA|MMAU__X1))= dval;
  *((int32 volatile *)(MMAU__SMACDA|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit integer value by value stored in the A10
 *          register of the MMAU and add product with 64-bit integer value.
 * @details The @ref smacas_dl function multiplies 32-bit integer value by value
 *          stored in the A10 register of the MMAU and add product with 64-bit
 *          integer value.
 * @param   dval    @ref int64 integer value.
 * @param   lval    @ref int32 integer value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void smacas_dl (register int64 dval, register int32 lval)
{
  *((int64 volatile *)(MMAU__SMACDA|MMAU__X1|MMAU__SAT))= dval;
  *((int32 volatile *)(MMAU__SMACDA|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit integer values returning a 32-bit integer
 *          quotient.
 * @details The @ref l_sdiv_ll function divides two 32-bit integer values
 *          returning a 32-bit integer quotient.
 * @param   lnum    @ref int32 integer value.
 * @param   lden    @ref int32 integer value.
 * @return  @ref int32 integer value.
 * @note    Quotient is stored in A0 register of the MMAU for next computation.
 ******************************************************************************/
static inline int32 l_sdiv_ll (register int32 lnum, register int32 lden)
{
  *((int32 volatile *)(MMAU__SDIV|MMAU__X2))= lnum;
  *((int32 volatile *)(MMAU__SDIV|MMAU__X3))= lden;
  return *((int32 volatile *)(MMAU__SDIV|MMAU__A0));
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit integer values returning a 32-bit integer
 *          quotient.
 * @details The @ref l_sdivs_ll function divides two 32-bit integer values
 *          returning a 32-bit integer quotient.
 * @param   lnum    @ref int32 integer value.
 * @param   lden    @ref int32 integer value.
 * @return  @ref int32 integer value.
 * @note    Saturated quotient is stored in A0 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int32 l_sdivs_ll (register int32 lnum, register int32 lden)
{
  *((int32 volatile *)(MMAU__SDIV|MMAU__X2|MMAU__SAT))= lnum;
  *((int32 volatile *)(MMAU__SDIV|MMAU__X3|MMAU__SAT))= lden;
  return *((int32 volatile *)(MMAU__SDIV|MMAU__A0|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit integer values.
 * @details The @ref sdiv_ll function divides two 32-bit integer values.
 * @param   lnum    @ref int32 integer value.
 * @param   lden    @ref int32 integer value.
 * @note    Quotient is stored in A0 register of the MMAU for next computation.
 ******************************************************************************/
static inline void sdiv_ll (register int32 lnum, register int32 lden)
{
  *((int32 volatile *)(MMAU__SDIV|MMAU__X2))= lnum;
  *((int32 volatile *)(MMAU__SDIV|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit integer values.
 * @details The @ref sdivs_ll function divides two 32-bit integer values.
 * @param   lnum    @ref int32 integer value.
 * @param   lden    @ref int32 integer value.
 * @note    Saturated quotient is stored in A0 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void sdivs_ll (register int32 lnum, register int32 lden)
{
  *((int32 volatile *)(MMAU__SDIV|MMAU__X2|MMAU__SAT))= lnum;
  *((int32 volatile *)(MMAU__SDIV|MMAU__X3|MMAU__SAT))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value by 32-bit integer value returning a
 *          64-bit integer quotient.
 * @details The @ref d_sdiv_dl function divides 64-bit integer value by 32-bit
 *          integer value returning a 64-bit integer quotient.
 * @param   dnum    @ref int64 integer value.
 * @param   lden    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_sdiv_dl (register int64 dnum, register int32 lden)
{
  *((int64 volatile *)(MMAU__SDIVD|MMAU__X1))= dnum;
  *((int32 volatile *)(MMAU__SDIVD|MMAU__X3))= lden;
  return *((int64 volatile *)(MMAU__SDIVD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value by 32-bit integer value returning a
 *          64-bit integer quotient.
 * @details The @ref d_sdivs_dl function divides 64-bit integer value by 32-bit
 *          integer value returning a 64-bit integer quotient.
 * @param   dnum    @ref int64 integer value.
 * @param   lden    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_sdivs_dl (register int64 dnum, register int32 lden)
{
  *((int64 volatile *)(MMAU__SDIVD|MMAU__X1|MMAU__SAT))= dnum;
  *((int32 volatile *)(MMAU__SDIVD|MMAU__X3|MMAU__SAT))= lden;
  return *((int64 volatile *)(MMAU__SDIVD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value by 32-bit integer value.
 * @details The @ref sdiv_dl function divides 64-bit integer value by 32-bit
 *          integer value.
 * @param   dnum    @ref int64 integer value.
 * @param   lden    @ref int32 integer value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void sdiv_dl (register int64 dnum, register int32 lden)
{
  *((int64 volatile *)(MMAU__SDIVD|MMAU__X1))= dnum;
  *((int32 volatile *)(MMAU__SDIVD|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value by 32-bit integer value.
 * @details The @ref sdivs_dl function divides 64-bit integer value by 32-bit
 *          integer value.
 * @param   dnum    @ref int64 integer value.
 * @param   lden    @ref int32 integer value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void sdivs_dl (register int64 dnum, register int32 lden)
{
  *((int64 volatile *)(MMAU__SDIVD|MMAU__X1|MMAU__SAT))= dnum;
  *((int32 volatile *)(MMAU__SDIVD|MMAU__X3|MMAU__SAT))= lden;
}

/***************************************************************************//*!
 * @brief   Divide two 64-bit integer values returning a 64-bit integer
 *          quotient.
 * @details The @ref d_sdiv_dd function divides two 64-bit integer values
 *          returning a 64-bit integer quotient.
 * @param   dnum    @ref int64 integer value.
 * @param   dden    @ref int64 integer value.
 * @return  @ref int64 integer value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_sdiv_dd (register int64 dnum, register int64 dden)
{
  *((int64 volatile *)(MMAU__SDIVDD|MMAU__X0))= dnum;
  *((int64 volatile *)(MMAU__SDIVDD|MMAU__X2))= dden;
  return *((int64 volatile *)(MMAU__SDIVDD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide two 64-bit integer values returning a 64-bit integer
 *          quotient.
 * @details The @ref d_sdivs_dd function divides two 64-bit integer values
 *          returning a 64-bit integer quotient.
 * @param   dnum    @ref int64 integer value.
 * @param   dden    @ref int64 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_sdivs_dd (register int64 dnum, register int64 dden)
{
  *((int64 volatile *)(MMAU__SDIVDD|MMAU__X0|MMAU__SAT))= dnum;
  *((int64 volatile *)(MMAU__SDIVDD|MMAU__X2|MMAU__SAT))= dden;
  return *((int64 volatile *)(MMAU__SDIVDD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide two 64-bit integer values.
 * @details The @ref sdiv_dd function divides two 64-bit integer values.
 * @param   dnum    @ref int64 integer value.
 * @param   dden    @ref int64 integer value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void sdiv_dd (register int64 dnum, register int64 dden)
{
  *((int64 volatile *)(MMAU__SDIVDD|MMAU__X0))= dnum;
  *((int64 volatile *)(MMAU__SDIVDD|MMAU__X2))= dden;
}

/***************************************************************************//*!
 * @brief   Divide two 64-bit integer values.
 * @details The @ref sdivs_dd function divides two 64-bit integer values.
 * @param   dnum    @ref int64 integer value.
 * @param   dden    @ref int64 integer value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void sdivs_dd (register int64 dnum, register int64 dden)
{
  *((int64 volatile *)(MMAU__SDIVDD|MMAU__X0|MMAU__SAT))= dnum;
  *((int64 volatile *)(MMAU__SDIVDD|MMAU__X2|MMAU__SAT))= dden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value stored in the A10 register of the MMAU
 *          by 32-bit integer value returning a 64-bit integer quotient.
 * @details The @ref d_sdiva_l function divides 64-bit integer value stored in
 *          the A10 register of the MMAU by 32-bit integer value returning a
 *          64-bit integer quotient.
 * @param   lden    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_sdiva_l (register int32 lden)
{
  *((int32 volatile *)(MMAU__SDIVDA|MMAU__X3))= lden;
  return *((int64 volatile *)(MMAU__SDIVDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value stored in the A10 register of the MMAU
 *          by 32-bit integer value returning saturated 64-bit integer quotient.
 * @details The @ref d_sdivas_l function divides 64-bit integer value stored in
 *          the A10 register of the MMAU by 32-bit integer value returning
 *          a saturated 64-bit integer quotient.
 * @param   lden    @ref int32 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_sdivas_l (register int32 lden)
{
  *((int32 volatile *)(MMAU__SDIVDA|MMAU__X3|MMAU__SAT))= lden;
  return *((int64 volatile *)(MMAU__SDIVDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value stored in the A10 register of the MMAU
 *          by 32-bit integer value.
 * @details The @ref sdiva_l function divides 64-bit integer value stored in the
 *          A10 register of the MMAU by 32-bit integer value.
 * @param   lden    @ref int32 integer value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void sdiva_l (register int32 lden)
{
  *((int32 volatile *)(MMAU__SDIVDA|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value stored in the A10 register of the MMAU
 *          by 32-bit integer value.
 * @details The @ref sdivas_l function divides 64-bit integer value stored in
 *          the A10 register of the MMAU by 32-bit integer value.
 * @param   lden    @ref int32 integer value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void sdivas_l (register int32 lden)
{
  *((int32 volatile *)(MMAU__SDIVDA|MMAU__X3|MMAU__SAT))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value stored in the A10 register of the MMAU
 *          by 64-bit integer value returning a 64-bit integer quotient.
 * @details The @ref d_sdiva_d function divides 64-bit integer value stored in
 *          the A10 register of the MMAU by 64-bit integer value returning a
 *          64-bit integer quotient.
 * @param   dden    @ref int64 integer value.
 * @return  @ref int64 integer value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline int64 d_sdiva_d (register int64 dden)
{
  *((int64 volatile *)(MMAU__SDIVDDA|MMAU__X2))= dden;
  return *((int64 volatile *)(MMAU__SDIVDDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value stored in the A10 register of the MMAU
 *          by 64-bit integer value returning saturated 64-bit integer quotient.
 * @details The @ref d_sdivas_d function divides 64-bit integer value stored in
 *          the A10 register of the MMAU by 64-bit integer value returning
 *          a saturated 64-bit integer quotient.
 * @param   dden    @ref int64 integer value.
 * @return  @ref int64 integer value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline int64 d_sdivas_d (register int64 dden)
{
  *((int64 volatile *)(MMAU__SDIVDDA|MMAU__X2|MMAU__SAT))= dden;
  return *((int64 volatile *)(MMAU__SDIVDDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value stored in the A10 register of the MMAU
 *          by 64-bit integer value.
 * @details The @ref sdiva_d function divides 64-bit integer value stored in the
 *          A10 register of the MMAU by 64-bit integer value.
 * @param   dden    @ref int64 integer value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void sdiva_d (register int64 dden)
{
  *((int64 volatile *)(MMAU__SDIVDDA|MMAU__X2))= dden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit integer value stored in the A10 register of the MMAU
 *          by 64-bit integer value.
 * @details The @ref sdivas_d function divides 64-bit integer value stored in
 *          the A10 register of the MMAU by 64-bit integer value.
 * @param   dden    @ref int64 integer value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void sdivas_d (register int64 dden)
{
  *((int64 volatile *)(MMAU__SDIVDDA|MMAU__X2|MMAU__SAT))= dden;
}

/*! @} End of int_instructions                                               */

/******************************************************************************
 * MMAU instruction set.
 *
 *//*! @addtogroup frac_instructions
 * @{
 ******************************************************************************/

/***************************************************************************//*!
 * @brief   Load A10 accumulator register of the MMAU by 64-bit fractional
 *          value.
 * @details The @ref lda_d function loads A10 accumulator register of the MMAU
 *          by 64-bit fractional value.
 * @param   dval    @ref frac64 fractional value.
 ******************************************************************************/
static inline void lda_d (register frac64 dval)
{
  *((frac64 volatile *)(MMAU__REGRW|MMAU__A10))= dval;
}

/***************************************************************************//*!
 * @brief   Load upper A1 accumulator register of the MMAU by 32-bit fractional
 *          value and clear lower A0 accumulator register.
 * @details The @ref lda_l function loads upper A1 accumulator register of the
 *          MMAU by 32-bit fractional value and clears lower A0 accumulator
 *          register.
 * @param   lval    @ref frac32 fractional value.
 ******************************************************************************/
static inline void lda_l (register frac32 lval)
{
  *((frac32 volatile *)(MMAU__REGRW|MMAU__A0))= 0l;
  *((frac32 volatile *)(MMAU__REGRW|MMAU__A1))= lval;
}

/***************************************************************************//*!
 * @brief   Read 32-bit fractional value from the A1 accumulator register of the
 *          MMAU.
 * @details The @ref l_rda function reads 32-bit fractional value from the A1
 *          accumulator register of the MMAU.
 * @return  @ref frac32 fractional value.
 ******************************************************************************/
static inline frac32 l_rda (void)
{
  return *((frac32 volatile *)(MMAU__REGRW|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Read 64-bit fractional value from the A10 accumulator register of
 *          the MMAU.
 * @details The @ref d_rda function reads 64-bit fractional value from the A10
 *          accumulator register of the MMAU.
 * @return  @ref frac64 fractional value.
 ******************************************************************************/
static inline frac64 d_rda (void)
{
  return *((frac64 volatile *)(MMAU__REGRW|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit fractional values returning a 32-bit fractional
 *          product.
 * @details The @ref l_mul_ll function multiplies two 32-bit fractional values
 *          returning a 32-bit fractional product.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac32 l_mul_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X2))= lval1;
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X3))= lval2;
  return *((frac32 volatile *)(MMAU__QMUL|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit fractional values returning saturated
 *          32-bit fractional product.
 * @details The @ref l_muls_ll function multiplies two 32-bit fractional values
 *          returning saturated 32-bit fractional product.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_muls_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X2|MMAU__SAT))= lval1;
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X3|MMAU__SAT))= lval2;
  return *((frac32 volatile *)(MMAU__QMUL|MMAU__A1|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit fractional values returning a 64-bit fractional
 *          product.
 * @details The @ref d_mul_ll function multiplies two 32-bit fractional values
 *          returning a 64-bit fractional product.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac64 d_mul_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X2))= lval1;
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X3))= lval2;
  return *((frac64 volatile *)(MMAU__QMUL|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit fractional values returning saturated
 *          64-bit fractional product.
 * @details The @ref d_muls_ll function multiplies two 32-bit fractional values
 *          returning saturated 64-bit fractional product.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac64 d_muls_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X2|MMAU__SAT))= lval1;
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X3|MMAU__SAT))= lval2;
  return *((frac64 volatile *)(MMAU__QMUL|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit fractional values.
 * @details The @ref mul_ll function multiplies two 32-bit fractional values.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void mul_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X2))= lval1;
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X3))= lval2;
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit fractional values.
 * @details The @ref muls_ll function multiplies two 32-bit fractional values.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void muls_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X2|MMAU__SAT))= lval1;
  *((frac32 volatile *)(MMAU__QMUL|MMAU__X3|MMAU__SAT))= lval2;
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit fractional value with 32-bit fractional value
 *          returning a 32-bit fractional product.
 * @details The @ref l_mul_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value returning a 32-bit fractional product.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac32 l_mul_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMULD|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMULD|MMAU__X3))= lval;
  return *((frac32 volatile *)(MMAU__QMULD|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit fractional value with 32-bit fractional
 *          value returning saturated 32-bit fractional product.
 * @details The @ref l_muls_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value returning saturated 32-bit fractional
 *          product.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_muls_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMULD|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMULD|MMAU__X3|MMAU__SAT))= lval;
  return *((frac32 volatile *)(MMAU__QMULD|MMAU__A1|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit fractional value with 32-bit fractional value
 *          returning a 64-bit fractional product.
 * @details The @ref d_mul_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value returning a 64-bit fractional product.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac64 d_mul_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMULD|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMULD|MMAU__X3))= lval;
  return *((frac64 volatile *)(MMAU__QMULD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit fractional value with 32-bit fractional
 *          value returning saturated 64-bit fractional product.
 * @details The @ref d_muls_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value returning saturated 64-bit fractional
 *          product.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac64 d_muls_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMULD|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMULD|MMAU__X3|MMAU__SAT))= lval;
  return *((frac64 volatile *)(MMAU__QMULD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit fractional value with 32-bit fractional value.
 * @details The @ref mul_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void mul_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMULD|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMULD|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit fractional value with 32-bit fractional
 *          value.
 * @details The @ref muls_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void muls_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMULD|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMULD|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit fractional value with 64-bit fractional value stored
 *          in the A10 register of the MMAU returning a 32-bit fractional
 *          product.
 * @details The @ref l_mula_l function multiplies 32-bit fractional value with
 *          64-bit fractional value stored in the A10 register of the MMAU
 *          returning a 32-bit fractional product.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac32 l_mula_l (register frac32 lval)
{
  *((frac32 volatile *)(MMAU__QMULDA|MMAU__X3))= lval;
  return *((frac32 volatile *)(MMAU__QMULDA|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit fractional value with 64-bit fractional
 *          value stored in the A10 register of the MMAU returning saturated
 *          32-bit fractional product.
 * @details The @ref l_mulas_l function multiplies 32-bit fractional value with
 *          64-bit fractional value stored in the A10 register of the MMAU
 *          returning saturated 32-bit fractional product.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_mulas_l (register frac32 lval)
{
  *((frac32 volatile *)(MMAU__QMULDA|MMAU__X3|MMAU__SAT))= lval;
  return *((frac32 volatile *)(MMAU__QMULDA|MMAU__A1|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit fractional value with 64-bit fractional value stored
 *          in the A10 register of the MMAU returning a 64-bit fractional
 *          product.
 * @details The @ref d_mula_l function multiplies 32-bit fractional value with
 *          64-bit fractional value stored in the A10 register of the MMAU
 *          returning a 64-bit fractional product.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac64 d_mula_l (register frac32 lval)
{
  *((frac32 volatile *)(MMAU__QMULDA|MMAU__X3))= lval;
  return *((frac64 volatile *)(MMAU__QMULDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit fractional value with 64-bit fractional
 *          value stored in the A10 register of the MMAU returning saturated
 *          64-bit fractional product.
 * @details The @ref d_mulas_l function multiplies 32-bit fractional value with
 *          64-bit fractional value stored in the A10 register of the MMAU
 *          returning saturated 64-bit fractional product.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac64 d_mulas_l (register frac32 lval)
{
  *((frac32 volatile *)(MMAU__QMULDA|MMAU__X3|MMAU__SAT))= lval;
  return *((frac64 volatile *)(MMAU__QMULDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit fractional value with 64-bit fractional value stored
 *          in the A10 register of the MMAU.
 * @details The @ref mula_l function multiplies 32-bit fractional value with
 *          64-bit fractional value stored in the A10 register of the MMAU.
 * @param   lval    @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void mula_l (register frac32 lval)
{
  *((frac32 volatile *)(MMAU__QMULDA|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit fractional value with 64-bit fractional
 *          value stored in the A10 register of the MMAU.
 * @details The @ref mulas_l function multiplies 32-bit fractional value with
 *          64-bit fractional value stored in the A10 register of the MMAU.
 * @param   lval    @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void mulas_l (register frac32 lval)
{
  *((frac32 volatile *)(MMAU__QMULDA|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit fractional values and add product with value
 *          stored in the A10 register of the MMAU returning a 32-bit fractional
 *          A10 register value.
 * @details The @ref l_mac_ll function multiplies two 32-bit fractional values
 *          and add product with value stored in the A10 register of the MMAU
 *          returning a 32-bit fractional A1 register value.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac32 l_mac_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X2))= lval1;
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X3))= lval2;
  return *((frac32 volatile *)(MMAU__QMAC|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit fractional values and add product
 *          with value stored in the A10 register of the MMAU returning a 32-bit
 *          fractional A10 register value.
 * @details The @ref l_macs_ll function multiplies two 32-bit fractional values
 *          and add product with value stored in the A10 register of the MMAU
 *          returning saturated 32-bit fractional A1 register value.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_macs_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X2|MMAU__SAT))= lval1;
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X3|MMAU__SAT))= lval2;
  return *((frac32 volatile *)(MMAU__QMAC|MMAU__A1|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit fractional values and add product with value
 *          stored in the A10 register of the MMAU returning a 64-bit fractional
 *          A10 register value.
 * @details The @ref d_mac_ll function multiplies two 32-bit fractional values
 *          and add product with value stored in the A10 register of the MMAU
 *          returning a 64-bit fractional A10 register value.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac64 d_mac_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X2))= lval1;
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X3))= lval2;
  return *((frac64 volatile *)(MMAU__QMAC|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit fractional values and add product
 *          with value stored in the A10 register of the MMAU returning a 64-bit
 *          fractional A10 register value.
 * @details The @ref d_macs_ll function multiplies two 32-bit fractional values
 *          and add product with value stored in the A10 register of the MMAU
 *          returning saturated 64-bit fractional A10 register value.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac64 d_macs_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X2|MMAU__SAT))= lval1;
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X3|MMAU__SAT))= lval2;
  return *((frac64 volatile *)(MMAU__QMAC|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply two 32-bit fractional values and add product with value
 *          stored in the A10 register of the MMAU.
 * @details The @ref mac_ll function multiplies two 32-bit fractional values and
 *          add product with value stored in the A10 register of the MMAU.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void mac_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X2))= lval1;
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X3))= lval2;
}

/***************************************************************************//*!
 * @brief   Saturating multiply two 32-bit fractional values and add product
 *          with value stored in the A10 register of the MMAU.
 * @details The @ref macs_ll function multiplies two 32-bit fractional values
 *          and add product with value stored in the A10 register of the MMAU.
 * @param   lval1   @ref frac32 fractional value.
 * @param   lval2   @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void macs_ll (register frac32 lval1, register frac32 lval2)
{
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X2|MMAU__SAT))= lval1;
  *((frac32 volatile *)(MMAU__QMAC|MMAU__X3|MMAU__SAT))= lval2;
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit fractional value with 32-bit fractional value and
 *          add product with value stored in the A10 register of the MMAU
 *          returning a 32-bit fractional A10 register value.
 * @details The @ref l_mac_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value and add product with value stored in the A10
 *          register of the MMAU returning a 32-bit fractional A1 register
 *          value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac32 l_mac_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACD|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMACD|MMAU__X3))= lval;
  return *((frac32 volatile *)(MMAU__QMACD|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit fractional value with 32-bit fractional
 *          value and add product with value stored in the A10 register of the
 *          MMAU returning saturated 32-bit fractional A10 register value.
 * @details The @ref l_macs_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value and add product with value stored in the A10
 *          register of the MMAU returning saturated 32-bit fractional A1
 *          register value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_macs_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACD|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMACD|MMAU__X3|MMAU__SAT))= lval;
  return *((frac32 volatile *)(MMAU__QMACD|MMAU__A1|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit fractional value with 32-bit fractional value and
 *          add product with value stored in the A10 register of the MMAU
 *          returning a 64-bit fractional A10 register value.
 * @details The @ref d_mac_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value and add product with value stored in the A10
 *          register of the MMAU returning a 64-bit fractional A10 register
 *          value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac64 d_mac_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACD|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMACD|MMAU__X3))= lval;
  return *((frac64 volatile *)(MMAU__QMACD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit fractional value with 32-bit fractional
 *          value and add product with value stored in the A10 register of the
 *          MMAU returning saturated 64-bit fractional A10 register value.
 * @details The @ref d_macs_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value and add product with value stored in the A10
 *          register of the MMAU returning saturated 64-bit fractional A10
 *          register value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac64 d_macs_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACD|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMACD|MMAU__X3|MMAU__SAT))= lval;
  return *((frac64 volatile *)(MMAU__QMACD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 64-bit fractional value with 32-bit fractional value and
 *          add product with value stored in the A10 register of the MMAU.
 * @details The @ref mac_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value and add product with value stored in the A10
 *          register of the MMAU.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void mac_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACD|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMACD|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 64-bit fractional value with 32-bit fractional
 *          value and add product with value stored in the A10 register of the
 *          MMAU.
 * @details The @ref macs_dl function multiplies 64-bit fractional value with
 *          32-bit fractional value and add product with value stored in the A10
 *          register of the MMAU.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void macs_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACD|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMACD|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit fractional value by value stored in the A10 register
 *          of the MMAU and add product with 64-bit fractional value returning a
 *          32-bit fractional A10 register value.
 * @details The @ref l_maca_dl function multiplies 32-bit fractional value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit fractional value returning a 32-bit fractional A1 register
 *          value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac32 l_maca_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACDA|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMACDA|MMAU__X3))= lval;
  return *((frac32 volatile *)(MMAU__QMACDA|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit fractional value by value stored in the
 *          A10 register of the MMAU and add product with 64-bit fractional
 *          value returning a saturated 32-bit fractional A10 register value.
 * @details The @ref l_macas_dl function multiplies 32-bit fractional value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit fractional value returning saturated 32-bit fractional A1
 *          register value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_macas_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACDA|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMACDA|MMAU__X3|MMAU__SAT))= lval;
  return *((frac32 volatile *)(MMAU__QMACDA|MMAU__A1|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit fractional value by value stored in the A10 register
 *          of the MMAU and add product with 64-bit fractional value returning a
 *          64-bit fractional A10 register value.
 * @details The @ref d_maca_dl function multiplies 32-bit fractional value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit fractional value returning a 64-bit fractional A10 register
 *          value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac64 d_maca_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACDA|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMACDA|MMAU__X3))= lval;
  return *((frac64 volatile *)(MMAU__QMACDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit fractional value by value stored in the
 *          A10 register of the MMAU and add product with 64-bit fractional
 *          value returning a saturated 64-bit fractional A10 register value.
 * @details The @ref d_macas_dl function multiplies 32-bit fractional value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit fractional value returning saturated 64-bit fractional A10
 *          register value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac64 d_macas_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACDA|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMACDA|MMAU__X3|MMAU__SAT))= lval;
  return *((frac64 volatile *)(MMAU__QMACDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Multiply 32-bit fractional value by value stored in the A10 register
 *          of the MMAU and add product with 64-bit fractional value.
 * @details The @ref maca_dl function multiplies 32-bit fractional value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit fractional value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @note    Product is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void maca_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACDA|MMAU__X1))= dval;
  *((frac32 volatile *)(MMAU__QMACDA|MMAU__X3))= lval;
}

/***************************************************************************//*!
 * @brief   Saturating multiply 32-bit fractional value by value stored in the
 *          A10 register of the MMAU and add product with 64-bit fractional
 *          value.
 * @details The @ref macas_dl function multiplies 32-bit fractional value by
 *          value stored in the A10 register of the MMAU and add product with
 *          64-bit fractional value.
 * @param   dval    @ref frac64 fractional value.
 * @param   lval    @ref frac32 fractional value.
 * @note    Saturated product is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void macas_dl (register frac64 dval, register frac32 lval)
{
  *((frac64 volatile *)(MMAU__QMACDA|MMAU__X1|MMAU__SAT))= dval;
  *((frac32 volatile *)(MMAU__QMACDA|MMAU__X3|MMAU__SAT))= lval;
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit fractional values returning a 32-bit fractional
 *          quotient.
 * @details The @ref l_div_ll function divides two 32-bit fractional values
 *          returning a 32-bit fractional quotient.
 * @param   lnum    @ref frac32 fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Quotient is stored in A1 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac32 l_div_ll (register frac32 lnum, register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIV|MMAU__X2))= lnum;
  *((frac32 volatile *)(MMAU__QDIV|MMAU__X3))= lden;
  return *((frac32 volatile *)(MMAU__QDIV|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit fractional values returning a 32-bit fractional
 *          quotient.
 * @details The @ref l_divs_ll function divides two 32-bit fractional values
 *          returning a 32-bit fractional quotient.
 * @param   lnum    @ref frac32 fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Saturated quotient is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_divs_ll (register frac32 lnum, register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIV|MMAU__X2|MMAU__SAT))= lnum;
  *((frac32 volatile *)(MMAU__QDIV|MMAU__X3|MMAU__SAT))= lden;
  return *((frac32 volatile *)(MMAU__QDIV|MMAU__A1|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit fractional values.
 * @details The @ref div_ll function divides two 32-bit fractional values.
 * @param   lnum    @ref frac32 fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @note    Quotient is stored in A1 register of the MMAU for next computation.
 ******************************************************************************/
static inline void div_ll (register frac32 lnum, register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIV|MMAU__X2))= lnum;
  *((frac32 volatile *)(MMAU__QDIV|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide two 32-bit fractional values.
 * @details The @ref divs_ll function divides two 32-bit fractional values.
 * @param   lnum    @ref frac32 fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @note    Saturated quotient is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void divs_ll (register frac32 lnum, register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIV|MMAU__X2|MMAU__SAT))= lnum;
  *((frac32 volatile *)(MMAU__QDIV|MMAU__X3|MMAU__SAT))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value by 32-bit fractional value returning
 *          a 64-bit fractional quotient.
 * @details The @ref d_div_dl function divides 64-bit fractional value by 32-bit
 *          fractional value returning a 64-bit fractional quotient.
 * @param   dnum    @ref frac64 fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac64 d_div_dl (register frac64 dnum, register frac32 lden)
{
  *((frac64 volatile *)(MMAU__QDIVD|MMAU__X1))= dnum;
  *((frac32 volatile *)(MMAU__QDIVD|MMAU__X3))= lden;
  return *((frac64 volatile *)(MMAU__QDIVD|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value by 32-bit fractional value returning
 *          a 64-bit fractional quotient.
 * @details The @ref d_divs_dl function divides 64-bit fractional value by
 *          32-bit fractional value returning a 64-bit fractional quotient.
 * @param   dnum    @ref frac64 fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac64 d_divs_dl (register frac64 dnum, register frac32 lden)
{
  *((frac64 volatile *)(MMAU__QDIVD|MMAU__X1|MMAU__SAT))= dnum;
  *((frac32 volatile *)(MMAU__QDIVD|MMAU__X3|MMAU__SAT))= lden;
  return *((frac64 volatile *)(MMAU__QDIVD|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value by 32-bit fractional value.
 * @details The @ref div_dl function divides 64-bit fractional value by 32-bit
 *          fractional value.
 * @param   dnum    @ref frac64 fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void div_dl (register frac64 dnum, register frac32 lden)
{
  *((frac64 volatile *)(MMAU__QDIVD|MMAU__X1))= dnum;
  *((frac32 volatile *)(MMAU__QDIVD|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value by 32-bit fractional value.
 * @details The @ref divs_dl function divides 64-bit fractional value by 32-bit
 *          fractional value.
 * @param   dnum    @ref frac64 fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void divs_dl (register frac64 dnum, register frac32 lden)
{
  *((frac64 volatile *)(MMAU__QDIVD|MMAU__X1|MMAU__SAT))= dnum;
  *((frac32 volatile *)(MMAU__QDIVD|MMAU__X3|MMAU__SAT))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value stored in the A10 register of the
 *          MMAU by 32-bit fractional value returning a 32-bit fractional
 *          quotient.
 * @details The @ref l_diva_l function divides 64-bit fractional value stored in
 *          the A10 register of the MMAU by 32-bit fractional value returning a
 *          32-bit fractional quotient.
 * @param   lden    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Quotient is stored in A1 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac32 l_diva_l (register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIVDA|MMAU__X3))= lden;
  return *((frac32 volatile *)(MMAU__QDIVDA|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value stored in the A10 register of the
 *          MMAU by 32-bit fractional value returning saturated 32-bit
 *          fractional quotient.
 * @details The @ref l_divas_l function divides 64-bit fractional value stored
 *          in the A10 register of the MMAU by 32-bit fractional value returning
 *          a saturated 32-bit fractional quotient.
 * @param   lden    @ref frac32 fractional value.
 * @return  @ref frac32 fractional value.
 * @note    Saturated quotient is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_divas_l (register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIVDA|MMAU__X3|MMAU__SAT))= lden;
  return *((frac32 volatile *)(MMAU__QDIVDA|MMAU__A1|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value stored in the A10 register of the
 *          MMAU by 32-bit fractional value returning a 64-bit fractional
 *          quotient.
 * @details The @ref d_diva_l function divides 64-bit fractional value stored in
 *          the A10 register of the MMAU by 32-bit fractional value returning a
 *          64-bit fractional quotient.
 * @param   lden    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline frac64 d_diva_l (register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIVDA|MMAU__X3))= lden;
  return *((frac64 volatile *)(MMAU__QDIVDA|MMAU__A10));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value stored in the A10 register of the
 *          MMAU by 32-bit fractional value returning saturated 64-bit
 *          fractional quotient.
 * @details The @ref d_divas_l function divides 64-bit fractional value stored
 *          in the A10 register of the MMAU by 32-bit fractional value returning
 *          a saturated 64-bit fractional quotient.
 * @param   lden    @ref frac32 fractional value.
 * @return  @ref frac64 fractional value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac64 d_divas_l (register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIVDA|MMAU__X3|MMAU__SAT))= lden;
  return *((frac64 volatile *)(MMAU__QDIVDA|MMAU__A10|MMAU__SAT));
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value stored in the A10 register of the
 *          MMAU by 32-bit fractional value.
 * @details The @ref diva_l function divides 64-bit fractional value stored in
 *          the A10 register of the MMAU by 32-bit fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @note    Quotient is stored in A10 register of the MMAU for next computation.
 ******************************************************************************/
static inline void diva_l (register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIVDA|MMAU__X3))= lden;
}

/***************************************************************************//*!
 * @brief   Divide 64-bit fractional value stored in the A10 register of the
 *          MMAU by 32-bit fractional value.
 * @details The @ref divas_l function divides 64-bit fractional value stored in
 *          the A10 register of the MMAU by 32-bit fractional value.
 * @param   lden    @ref frac32 fractional value.
 * @note    Saturated quotient is stored in A10 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void divas_l (register frac32 lden)
{
  *((frac32 volatile *)(MMAU__QDIVDA|MMAU__X3|MMAU__SAT))= lden;
}

/***************************************************************************//*!
 * @brief   Compute and return a 16-bit fractional square root of the 32-bit
 *          fractional radicand.
 * @details The @ref s_sqr_l function computes and returns a 16-bit fractional
 *          square root of the 32-bit fractional radicand.
 * @param   lrad    @ref frac32 fractional radicand.
 * @return  @ref frac16 fractional square root.
 * @note    Square root is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac16 s_sqr_l (register frac32 lrad)
{
  *((frac32 volatile *)(MMAU__QSQR|MMAU__X3))= lrad;
  return (frac16)((*((frac32 volatile *)(MMAU__QSQR|MMAU__A1)))>>16);
}

/***************************************************************************//*!
 * @brief   Compute a 16-bit fractional square root of the 32-bit fractional
 *          radicand.
 * @details The @ref sqr_l function computes a 16-bit fractional square root of
 *          the 32-bit fractional radicand.
 * @param   lrad    @ref frac32 fractional radicand.
 * @note    Square root is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void sqr_l (register frac32 lrad)
{
  *((frac32 volatile *)(MMAU__QSQR|MMAU__X3))= lrad;
}

/***************************************************************************//*!
 * @brief   Compute and return a 32-bit fractional square root of the 32-bit
 *          fractional radicand.
 * @details The @ref l_sqr_l function computes and returns a 32-bit fractional
 *          square root of the 32-bit fractional radicand.
 * @param   lrad    @ref frac32 fractional radicand.
 * @return  @ref frac32 fractional square root.
 * @note    Square root is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_sqr_l (register frac32 lrad)
{
  *((frac32 volatile *)(MMAU__QSQRD|MMAU__X2))= 0l;
  *((frac32 volatile *)(MMAU__QSQRD|MMAU__X3))= lrad;
  return *((frac32 volatile *)(MMAU__QSQRD|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Compute and return a 32-bit fractional square root of the 64-bit
 *          fractional radicand.
 * @details The @ref l_sqr_d function computes and returns a 32-bit fractional
 *          square root of the 64-bit fractional radicand.
 * @param   drad    @ref frac64 fractional radicand.
 * @return  @ref frac32 fractional square root.
 * @note    Square root is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_sqr_d (register frac64 drad)
{
  *((frac64 volatile *)(MMAU__QSQRD|MMAU__X2))= drad;
  return *((frac32 volatile *)(MMAU__QSQRD|MMAU__A1));
}

/***************************************************************************//*!
 * @brief   Compute a 32-bit fractional square root of the 64-bit fractional
 *          radicand.
 * @details The @ref sqr_d function computes a 32-bit fractional square root of
 *          the 64-bit fractional radicand.
 * @param   drad    @ref frac64 fractional radicand.
 * @note    Square root is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline void sqr_d (register frac64 drad)
{
  *((frac64 volatile *)(MMAU__QSQRD|MMAU__X2))= drad;
}

/***************************************************************************//*!
 * @brief   Compute and return a 32-bit fractional square root of the radicand
 *          stored in the A10 register of the MMAU.
 * @details The @ref l_sqra function computes and returns a 32-bit fractional
 *          square root of the radicand stored in the A10 register of the MMAU.
 * @return  @ref frac32 fractional square root.
 * @note    Square root is stored in A1 register of the MMAU for next
 *          computation.
 ******************************************************************************/
static inline frac32 l_sqra (void)
{
  (void) *((frac32 volatile *)(MMAU__REGRW|MMAU__A1)); /* dummy read */
  return *((frac32 volatile *)(MMAU__QSQRDA|MMAU__A1));
}
/*! @} End of frac_instructions                                               */
#endif /* __MMAU_H */
