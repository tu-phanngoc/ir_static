/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "ampm_gsm_common.h"
#include "ampm_gsm_dial_up.h"
#include "ampm_gsm_ring.h"
uint8_t DialupAtCallback0(uint8_t *buf);
uint8_t Ampm_GsmATDone(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmATDone1(AMPM_CMD_PHASE_TYPE phase);
uint8_t dialupTryCnt = 0;
Timeout_Type dialupTimeout;
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CGDCONT = {NULL,DialupAtCallback0,NULL,NULL,"OK","ERROR"};

const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_4	= {NULL,(void *)&ampmAtCmd_AT_DIALUP,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};
const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_3	= {(void *)&dialupCmdProcess_4,(void *)&ampmAtCmd_AT_CGDCONT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};
const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_2	= {(void *)&dialupCmdProcess_3,(void *)&ampmAtCmd_AT_CGACT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};
const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_1	= {(void *)&dialupCmdProcess_3,(void *)&ampmAtCmd_ATH,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};
const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_Start	= {(void *)&dialupCmdProcess_1,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmATDone,1,1000,100};

const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_Stop_End1	= {NULL,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};
const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_Stop_End	= {NULL,(void *)&ampmAtCmd_ATH,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};
const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_Stop_1= {(void *)&dialupCmdProcess_Stop_End,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmATDone1,1,1000,100};
const AMPM_CMD_PROCESS_TYPE dialupCmdProcess_Stop	= {(void *)&dialupCmdProcess_Stop_1,(void *)&ampmAtCmd_GotoCmdMode,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};

uint8_t Ampm_GsmATDone(AMPM_CMD_PHASE_TYPE phase)
{
	if(Ampm_Ringing_RI_isActive())
 	{
     	Ampm_SendAtCmdNow(&dialupCmdProcess_Start);
    }
	return 1;
}

uint8_t Ampm_GsmATDone1(AMPM_CMD_PHASE_TYPE phase)
{
	if(Ampm_Ringing_RI_isActive())
 	{
     	Ampm_SendAtCmdNow(&dialupCmdProcess_Stop_End1);
    }
	return 1;
}

uint8_t DialupAtCallback0(uint8_t *buf)
{
	return (uint8_t )ampm_sprintf((char *)buf,"AT+CGDCONT=1,\"IP\",\"%s\"\r",ampm_GprsApn);
}



enum{
	AMPM_GSM_DIALUP_START_PHASE = 0,
	AMPM_GSM_DIALUP_RUNING_PHASE,
	AMPM_GSM_DIALUP_OK,
	AMPM_GSM_DIALUP_FAIL,
}ampmGsmDialupPhase = AMPM_GSM_DIALUP_START_PHASE;

void Ampm_GSM_DialUp_Reset(void)
{
		ampmGsmDialupPhase = AMPM_GSM_DIALUP_START_PHASE;
		dialupTryCnt = 0;
		InitTimeout(&dialupTimeout,SYSTICK_TIME_SEC(5));
}

uint8_t Ampm_GSM_DialUp_IsOk(void)
{
		if(ampmGsmDialupPhase ==  AMPM_GSM_DIALUP_OK)
			return 1;
		return 0;
}

uint8_t Ampm_GSM_DialUp_IsDone(void)
{
		switch(ampmGsmDialupPhase)
	{
		case AMPM_GSM_DIALUP_FAIL:
		case AMPM_GSM_DIALUP_OK:
			return 1;
		default:
			return 0;
	}
}

uint8_t Ampm_GSM_DialUp(void)
{
	switch(ampmGsmDialupPhase)
	{
        case AMPM_GSM_DIALUP_START_PHASE:
        			if(Ampm_Ringing_RI_isActive())
        				break;
					if(CheckTimeout(&dialupTimeout) == SYSTICK_TIMEOUT)
					{
						if(Ampm_SendAtCheck_IsEmpty())
						{
							if(Ampm_SendAtCmd(&dialupCmdProcess_Start)){
								ampmGsmDialupPhase = AMPM_GSM_DIALUP_RUNING_PHASE;
								AMPM_GSM_LIB_DBG("AMPM_GSM_DAILUP:AMPM_GSM_DIALUP_RUNING_PHASE \r\n");
							}
						}
					}
        break;
        case AMPM_GSM_DIALUP_RUNING_PHASE:

					if(Ampm_SendAtCheck_IsEmpty() &&  ampm_GSM_CmdPhase == AMPM_CMD_OK){
							// put a dummy PPP packet
							COMM_Putc(0x7E);
							COMM_Putc(0xFF);
							COMM_Putc(0x7D);
							COMM_Putc(0x23);
							COMM_Putc(0xC0);
							COMM_Putc(0x21);
							COMM_Putc(0x7E);
							ampmGsmDialupPhase = AMPM_GSM_DIALUP_OK;
							AMPM_GSM_LIB_DBG("AMPM_GSM_DAILUP:AMPM_GSM_DIALUP_OK \r\n");
							dialupTryCnt = 0;
					}else	if(ampm_GSM_CmdPhase == AMPM_CMD_ERROR){
						if(dialupTryCnt++ >= 10){
							dialupTryCnt = 0;
							ampm_ModemResetFlag = 1;
							COMM_Puts("+++");
							ampmGsmDialupPhase = AMPM_GSM_DIALUP_FAIL;
							AMPM_GSM_LIB_DBG("AMPM_GSM_DAILUP:AMPM_GSM_DIALUP_FAIL \r\n");
						}else{
							InitTimeout(&dialupTimeout,SYSTICK_TIME_SEC(5));
							Ampm_SendAtCmd(&dialupCmdProcess_Stop);
							ampmGsmDialupPhase = AMPM_GSM_DIALUP_START_PHASE;
							AMPM_GSM_LIB_DBG("AMPM_GSM_DAILUP:AMPM_GSM_DIALUP_START_PHASE \r\n");
						}
					}
        break;
        case AMPM_GSM_DIALUP_OK:
                
                return 1;
        case AMPM_GSM_DIALUP_FAIL:

                return 0;
        default:
            ampmGsmDialupPhase = AMPM_GSM_DIALUP_FAIL;
          break;
	}
	return 0;
}
