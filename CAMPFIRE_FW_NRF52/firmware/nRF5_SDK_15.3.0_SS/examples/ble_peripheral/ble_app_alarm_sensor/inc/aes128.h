/******************** (C) COPYRIGHT 2008 STMicroelectronics ******************** 
* File Name          : aes128.h 
* Author             : MCD Application Team 
* Version            : V1.0.0 
* Date               : 10/06/2008 
* Description        : aes128 header file 
******************************************************************************** 
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS 
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME. 
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, 
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING 
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS. 
*******************************************************************************/ 
 
/* Define to prevent recursive inclusion -------------------------------------*/ 
#ifndef __AES_128_H 
#define __AES_128_H 
 
#include "stdint.h"
/* Includes ------------------------------------------------------------------*/ 
 
/******************************************************************************  
 Used to specify the AES algorithm version.    
 There are two possible AES implementations:   
 1 - version with unique key-schedule            
     using 256 + 256 + 10*4 bytes data = 552 bytes of look-up tables.  
    encryption spend about 19us, decryption spend about 21us
 2 - version with different key-schedule for encryption and decryption   
     using 256 + 256 + 10*4 + 256*4*2 bytes data = 2058 bytes of look-up table. 
    encryption spend about 16us, decryption spend about 32us
    Version 2 is faster than version 1 but requires more memory.   
*******************************************************************************/   
#define CRL_AES_ALGORITHM 1

/* Exported types ------------------------------------------------------------*/ 
/* Exported constants --------------------------------------------------------*/ 
#define AES_BLOCK_SIZE  4  /* Number of 32 bit words to store an AES128 block. */ 
#define AES_KEY_SIZE    4  /* Number of 32 bit words to store an AES128 key. */ 
#define AES_EXPKEY_SIZE 44 /* Number of 32bits words to store in an AES128 expanded key. */ 
                           /* The expanded key is the key after the keyschedule. */ 
#define AES_TEXT_SIZE   16   /* Number of 8 bit bytes*/

/* User's secret key */
#define AES128_KEY_DEFAULT_0    0x10111213
#define AES128_KEY_DEFAULT_1    0x14151617
#define AES128_KEY_DEFAULT_2    0x18191a1b
#define AES128_KEY_DEFAULT_3    0x1c1d1e1f

#ifdef __AES_128_C
uint32_t ASE128SecretKey[AES_KEY_SIZE] = {AES128_KEY_DEFAULT_0,
                                        AES128_KEY_DEFAULT_1,
                                        AES128_KEY_DEFAULT_2,
                                        AES128_KEY_DEFAULT_3};
uint32_t AES128_exp_EcyptKey[AES_EXPKEY_SIZE];
uint32_t AES128_exp_DecryptKey[AES_EXPKEY_SIZE];
#else
extern uint32_t ASE128SecretKey[AES_KEY_SIZE];
extern uint32_t AES128_exp_EcyptKey[AES_EXPKEY_SIZE];
extern uint32_t AES128_exp_DecryptKey[AES_EXPKEY_SIZE];
#endif

/* Exported macro ------------------------------------------------------------*/ 
/* Exported functions --------------------------------------------------------*/ 
 #ifdef __AES_128_C
/* According to key computes the expanded key exp for AES128 encryption. */ 
void AES_keyschedule_enc(uint32_t* key, uint32_t* exp); 
/* According to key computes the expanded key exp for AES128 decryption. */ 
void AES_keyschedule_dec(uint32_t* key, uint32_t* exp); 
 
/* Encrypts, according to the expanded key expkey, one block of 16 bytes  
   at address 'input_pointer' into the block at address 'output_pointer'. 
   They can be the same.*/ 
void AES_encrypt(uint32_t* input_pointer, uint32_t* output_pointer, uint32_t* expkey); 
 
/* Decrypts, according to the expanded key expkey, one block of 16 bytes  
   at address 'input_pointer' into the block at address 'output_pointer'. 
   They can be the same.*/ 
void AES_decrypt(uint32_t* input_pointer, uint32_t* output_pointer, uint32_t* expkey); 
#else
extern void AES_keyschedule_enc(uint32_t* key, uint32_t* exp); 
extern void AES_keyschedule_dec(uint32_t* key, uint32_t* exp);
extern void AES_encrypt(uint32_t* input_pointer, uint32_t* output_pointer, uint32_t* expkey);
extern void AES_decrypt(uint32_t* input_pointer, uint32_t* output_pointer, uint32_t* expkey); 
#endif
 
#endif /* __AES_128_H */ 
 
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/ 
