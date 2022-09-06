
#include <stdio.h>
#include "fire_alarm_message_create.h"
#include "system_config.h"
//-----------aes128 + base64--------------
#include "b64.h"
#include "aes128.h"
#include "lib/crc16.h"
/*
main key aes 128:	cqOFmsVXuvpfCoZi 
T1 	
mess encrypt:	{/DcfX15lhGyT8ErN9p9gBGxaBqULOURvge/GG7cxVFL3eJpTCfebjg7w5erTf1mpK2pG014ynsRCKuztY4cKOwSfdYLza5nRlly47X/h/9Y=01986} 	
checksum:		01986 	
payload:		2017-07-14 16:30:43,2,W4,880000000001,T1,452040666805809,******,123456,0,000 
S1 	
payload:		2017-07-14 16:30:37,S1,1,000 	
mess encrypt:	{/DcfX15lhGyT8ErN9p9gBDi1xRBUdC41y3B7y/RQbOg=14717} 	
checksum:		14717 
-------------------------------------------------------------------- 
device key aes 128:	1234123412341234 
T3: 	
mess encrypt:	[4qykDQSOD8lbQM9PJP39TJCVNohwOvP+9QsGb/hXPaYr0nF6j711MVYleIMtX6NhezpPC6zTAPZPk0oFpv7lrlZcAnmVdjsly4uY8q/PvW8=56243] 	
checksum:		56243 	
payload:		2017-07-14 16:30:44,2,W4,880000000001,T3,0,E,0,N,0,0,0,2,0.0.0.0,001813,0,001 
S3: 	
payload:		,S3,1,001 	
mess encrypt:	[Hur/Ko7JWcNOmQIZ9V56dQ==40424] 	
checksum:		40424 
-------------------------------------------------------------------- 
device key aes 128:	1234123412341234 
T3: 	
mess encrypt:	[4qykDQSOD8lbQM9PJP39TK9r2+0fn7FPbn9etSzj114r0nF6j711MVYleIMtX6NhezpPC6zTAPZPk0oFpv7lris4b03Y7sDAFgDYQckAWyM=07669] 	
checksum:		07669 	payload:		2017-07-14 16:30:54,2,W4,880000000001,T3,0,E,0,N,0,0,0,2,0.0.0.0,001813,0,002 
S3: 	
payload:		,S3,1,002 	
mess encrypt:	[hwJGac801Ck5Bqy+BWsHxg==52551] 	
checksum:		52551
*/

const uint16_t crc16_table[256] =
{
	0,49345,49537,320,49921,960,640,49729,50689,1728,1920,51009,1280,50625,50305,1088,52225,3264,3456,52545,3840,53185,52865,3648,2560,51905,52097,2880,51457,2496,2176,51265,
	55297,6336,6528,55617,6912,56257,55937,6720,7680,57025,57217,8000,56577,7616,7296,56385,5120,54465,54657,5440,55041,6080,5760,54849,53761,4800,4992,54081,4352,53697,53377,4160,
	61441,12480,12672,61761,13056,62401,62081,12864,13824,63169,63361,14144,62721,13760,13440,62529,15360,64705,64897,15680,65281,16320,16000,65089,64001,15040,15232,64321,14592,63937,63617,14400,
	10240,59585,59777,10560,60161,11200,10880,59969,60929,11968,12160,61249,11520,60865,60545,11328,58369,9408,9600,58689,9984,59329,59009,9792,8704,58049,58241,9024,57601,8640,8320,57409,
	40961,24768,24960,41281,25344,41921,41601,25152,26112,42689,42881,26432,42241,26048,25728,42049,27648,44225,44417,27968,44801,28608,28288,44609,43521,27328,27520,43841,26880,43457,43137,26688,
	30720,47297,47489,31040,47873,31680,31360,47681,48641,32448,32640,48961,32000,48577,48257,31808,46081,29888,30080,46401,30464,47041,46721,30272,29184,45761,45953,29504,45313,29120,28800,45121,
	20480,37057,37249,20800,37633,21440,21120,37441,38401,22208,22400,38721,21760,38337,38017,21568,39937,23744,23936,40257,24320,40897,40577,24128,23040,39617,39809,23360,39169,22976,22656,38977,
	34817,18624,18816,35137,19200,35777,35457,19008,19968,36545,36737,20288,36097,19904,19584,35905,17408,33985,34177,17728,34561,18368,18048,34369,33281,17088,17280,33601,16640,33217,32897,16448
};
static uint8_t aes_exp_key[250];
//#define aes_exp_key	mainBuf
static void Crc16_Init(void);

uint16_t EnCryptAes128(uint8_t *data_in,uint8_t *data_out,uint16_t length,uint8_t *pass)
{
		uint8_t buf[256];
		uint32_t key[4];
		uint16_t i;
		uint16_t count = length;
		uint8_t PKCS7;
		if(length % 16)
			count	= 16 * (length / 16 + 1);
		PKCS7 = count - length;
		if(length >= 256) return 0;
		memcpy((uint8_t *)key,pass,16);
		memset(buf,PKCS7,256);
		memcpy(buf,data_in,length);
		AES_keyschedule_enc((uint32_t *)&key,(uint32_t *)aes_exp_key);
		for(i = 0;i < count; i += 16)
		{
			AES_encrypt((uint32_t *)&buf[i],(uint32_t *)&data_out[i],(uint32_t *)aes_exp_key);
		}
		return count;
}

uint16_t DeCryptAes128(uint8_t *data_in,uint8_t *data_out,uint16_t length,uint8_t *pass)
{
		uint8_t buf[256];
		uint32_t key[4];
		uint16_t i;
		if(length >= 256) return 0;
		memcpy((uint8_t *)key,pass,16);
		memset(buf,0,256);
		memcpy(buf,data_in,length);
		AES_keyschedule_dec((uint32_t *)&key,(uint32_t *)aes_exp_key);
		for(i = 0;i < length; i += 16)
		{
			AES_decrypt((uint32_t *)&buf[i],(uint32_t *)&data_out[i],(uint32_t *)aes_exp_key);
		}
		return i;
}

unsigned short
crc16 (const char *buf, int len, unsigned short sd)
{
  int counter;
  unsigned short crc = sd;
  for (counter = 0; counter < len; counter++)
    crc = (crc >> 8) ^ crc16_table[((crc ) ^ *buf++) & 0x00FF];
  return crc;
}


static uint16_t ComputeChecksum(uint8_t *bytes,uint16_t len)
{
	uint16_t num1 = 0;
	uint8_t num2;
	int index;
	for (index = 0; index < len; ++index)
	{
		num2 = (num1 ^ bytes[index]);
		num1 = (num1 >> 8 ^ crc16_table[num2]);
	}
	return num1;
}

uint16_t FireAlarmCreateSmsPassword(char *phone,char *device_id,char *device_key,uint8_t *output)
{
	uint16_t len,crc;
	char buf[64],buf_out[64];
	char *pt;
	strcpy(buf,phone);
	strcat(buf,(char *)device_id);
	len = EnCryptAes128((uint8_t *)buf,(uint8_t *)buf_out,strlen(buf),(uint8_t *)device_key);
	pt = b64_encode((uint8_t *)buf_out,len);
	len += strlen(pt);
	pt[6+8] = 0;
	strcpy((char *)output,&pt[6]);
	free(pt);
	return len;
}

uint16_t FireAlarmCreateMsg(uint8_t *input,uint16_t input_len,uint8_t *output,uint16_t output_max_size,uint8_t *pass)
{
	uint16_t len,crc;
	char buf[16];
	char *pt;
	if(input_len >= output_max_size)
		return 0;
	len = EnCryptAes128(input,output,input_len,pass);
	pt = b64_encode(output,len);
	crc = ComputeChecksum((uint8_t *)pt,strlen(pt));
	len = sprintf(buf,"%05d",crc);
	len += strlen(pt);
	if(len >= output_max_size)
		return 0;
	strcpy((char *)output,pt);
	strcat((char *)output,buf);
	free(pt);
	return len;
}

uint16_t FireAlarmParsingMsg(uint8_t *input,uint16_t input_len,uint8_t *output,uint16_t output_max_size,uint8_t *pass)
{
	uint16_t len,crc;
	char buf[16];
	char *pt;
	if(input_len >= output_max_size)
		return 0;
	crc = ComputeChecksum(input,input_len - 5);
	sprintf(buf,"%05d",crc);
	if(memcmp(&input[input_len - 5],buf,5) != NULL)
		return 0;
	pt = (char *)b64_decode_ex((const char *)input,input_len - 5,(size_t *)&len);
	if(len >= output_max_size || (len % 16))
		return 0;
	len = DeCryptAes128((uint8_t *)pt,output,len,pass);
	free(pt);
	return len;
}



uint16_t CreateLoginMsg(uint8_t *input,uint8_t *output)
{
	uint16_t len,crc;
	char buf[128];
	char *pt;
	len = EnCryptAes128(input,output,strlen((const char *)input),sysCfg.mainAesKey);
	pt = b64_encode(output,len);
	crc = ComputeChecksum(pt,strlen(pt));
	output[0] = '{';
	output[1] = 0;
	strcat((char *)output,pt);
	sprintf(buf,"%05d}",crc);
	strcat((char *)output,buf);
	free(pt);
	return strlen((char *)output);
}

uint16_t CreateHeartBitMsg(uint8_t *input,uint8_t *output)
{
	uint16_t len,crc;
	char buf[32];
	char *pt;
	len = EnCryptAes128(input,output,strlen((const char *)input),sysCfg.deviceKey);
	pt = b64_encode(output,len);
	crc = ComputeChecksum(pt,strlen(pt));
	output[0] = '[';
	output[1] = 0;
	strcat((char *)output,pt);
	sprintf(buf,"%05d]",crc);
	strcat((char *)output,buf);
	free(pt);
	return strlen((char *)output);
}


