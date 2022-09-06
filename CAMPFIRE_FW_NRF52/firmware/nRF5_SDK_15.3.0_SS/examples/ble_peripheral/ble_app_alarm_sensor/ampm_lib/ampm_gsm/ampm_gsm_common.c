/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "ampm_gsm_common.h"

uint8_t csqBuff[16];
uint8_t cengBuff[128];
float csqValue;
uint8_t gsmSignal;
int8_t s_signal_dbm = -1;
uint8_t ampm_ModemResetFlag = 0;

char s_rssi_dBm[32] = { 0, 0, 109, 107, 105, 103, 101, 99, 97, 95, 93, 91, 89,
		87, 85, 83, 81, 79, 77, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57, 55, 53,
		52, };

const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CSCS_GSM= {"AT+CSCS=\"GSM\"\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT = {"AT\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_GotoCmdMode = {"+++",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CHLD = {"AT+CHLD=0\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_ATH = {"ATH\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CSQ = {"AT+CSQ\r",NULL,"+CSQ: ",Ampm_GsmGetCSQ,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CEND = {"AT+CENG?\r",NULL,"+CENG: 0,",Ampm_GsmGetCENG,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CLIP = {"AT+CLIP=1\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 2000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CMGF_1= {"AT+CMGF=1\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CMGF_0= {"AT+CMGF=0\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CMGL = {"AT+CMGL=\"ALL\"\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 500,timeout = 3000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CGACT = {"AT+CGACT=0\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_DIALUP = {"ATDT*99***1#\r",NULL,NULL,NULL,"CONNECT","ERROR"};
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CREG = {"AT+CREG?\r",NULL,NULL,NULL,"+CREG: 0,1","ERROR"};

const AMPM_CMD_PROCESS_TYPE ampmCmdProcess_AT	= {
		NULL,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};

const AMPM_CMD_PROCESS_TYPE ampmCmdProcess_ATH	= {
NULL,(void *)&ampmAtCmd_ATH,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};


uint32_t Ampm_GsmGetCENG(uint16_t cnt,uint8_t c)
{
	if((c == '\r') || (c == '\n') || cnt >= sizeof(cengBuff))
	{
		cengBuff[cnt] = '\0';
		return 0;
	}
	cengBuff[cnt] = c;
	return 0xff;
}

uint32_t Ampm_GsmGetCSQ(uint16_t cnt,uint8_t c)
{
	uint32_t t1,t2;
	if((c == '\r') || (c == '\n') || cnt >= sizeof(csqBuff))
	{
		csqBuff[cnt] = '\0';
		sscanf((const char *)csqBuff,"%d,%d",&t1,&t2);
		csqValue = (uint8_t)t1;
		if((csqValue < 4) || (csqValue == 99))
			gsmSignal = 0;
		else if(csqValue < 10)
			gsmSignal = 1;
		else if(csqValue < 16)
			gsmSignal = 2;
		else if(csqValue < 22)
			gsmSignal = 3;
		else if(csqValue < 28)
			gsmSignal = 4;
		else if(csqValue < 51)
			gsmSignal = 5;
		else 
			gsmSignal = 0;
		s_signal_dbm = s_rssi_dBm[gsmSignal];
		return 0;
	}
	csqBuff[cnt] = c;
	return 0xff;
}

