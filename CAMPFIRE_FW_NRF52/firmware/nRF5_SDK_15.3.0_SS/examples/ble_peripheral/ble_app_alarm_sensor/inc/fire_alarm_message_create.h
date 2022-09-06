#ifndef __FIRE_ALARM_MESSAGE_CREATE_H__
#define __FIRE_ALARM_MESSAGE_CREATE_H__
#include <stdint.h>


uint16_t CreateLoginMsg(uint8_t *input,uint8_t *output);
uint16_t CreateHeartBitMsg(uint8_t *input,uint8_t *output);
uint16_t DeCryptAes128(uint8_t *data_in,uint8_t *data_out,uint16_t length,uint8_t *pass);

uint16_t FireAlarmCreateMsg(uint8_t *input,uint16_t input_len,uint8_t *output,uint16_t output_max_size,uint8_t *pass);
uint16_t FireAlarmParsingMsg(uint8_t *input,uint16_t input_len,uint8_t *output,uint16_t output_max_size,uint8_t *pass);
uint16_t FireAlarmCreateSmsPassword(char *phone,char *device_id,char *device_key,uint8_t *output);


#endif

