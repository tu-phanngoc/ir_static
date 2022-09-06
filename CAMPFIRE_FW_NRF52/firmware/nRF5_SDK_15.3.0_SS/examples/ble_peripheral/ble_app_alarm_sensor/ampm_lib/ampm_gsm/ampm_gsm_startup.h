/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#ifndef AMPM_GSM_STARTUP_H__
#define AMPM_GSM_STARTUP_H__
#include <stdint.h>
#define GSM_MAX_STARTUP_CMD_LIST       1024

typedef enum{
	THIS_MODULE_NOT_SUPPORT = 0,
	BG2_E,
	BGS2_E,
	BGS2_W,
	M95,
	M26,
	SIM900,
	UBLOX_G100,
}AMPM_GSM_MODULE_TYPE;

extern uint8_t gsmSimCIDBuf[32];
extern uint8_t flagGotSimCID;
extern uint8_t modemCIDPass;
extern uint8_t modemJustPowerOff;
extern uint8_t gsmIMEIBuf[18];
extern uint8_t flagGotIMEI;
extern uint8_t modemIMEIPass;

extern uint8_t gsmSimCIMIBuf[32];
extern uint8_t flagGotSimCIMI;
extern uint8_t modemCIMIPass;

extern uint8_t gsmSimCPOSBuf[32];
extern uint8_t flagGotSimCPOS;
extern uint8_t modemCPOSPass;

uint8_t AMPM_GSM_Startup(const uint8_t *cmdList);
uint8_t AMPM_GSM_Startup_Init(void);
uint32_t Ampm_GSM_GetIMEI(uint16_t cnt,uint8_t c);
uint32_t Ampm_GSM_GetSimCID(uint16_t cnt,uint8_t c);
#endif
