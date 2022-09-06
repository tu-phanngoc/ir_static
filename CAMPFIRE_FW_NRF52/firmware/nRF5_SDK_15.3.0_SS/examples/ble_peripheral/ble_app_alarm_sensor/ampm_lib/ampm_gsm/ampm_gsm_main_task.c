
/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
//#include "resolv.h"
//#include "resolver.h"
#include "ampm_gsm_main_task.h"
#include "ppp/ppp.h"
#include "system_config.h"
#include "led.h"
#ifdef PPP_LWIP
#include "lwip/tcpip.h"
#endif

#ifdef CMSIS_OS_RTX 
osThreadDef(AMPM_GSM_MainTask, osPriorityNormal, 2048);
#endif
/*
D =  AT CMD DEFAULT
F = AT CMD do not check sucess
0 =  AT CMD
1 =  GSM POWER PIN CLR
2 =  GSM POWER PIN SET
3 = DELAY
4 =  GSM RESET PIN CLR
5 =  GSM RESET PIN SET
6 =  GSM DTR PIN CLR
7 =  GSM DTR PIN SET
S = STOP	@@D$AT+IPR=0\r$\r\n\
*/

const uint8_t ampm_AtGsmCmdStart[] = {
	"@@1\r\n\
	@@3$1000\r\n\
	@@2$\r\n\
	@@3$1500\r\n\
	@@F$+++$\r\n\
	@@0$AT\r$0$0$OK$NOT USE$5$1000$100\r\n\
	@@0$ATI\r$ATI\r\r\n$3$OK$ERROR$3$1000$100\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATI\r$\r\n\
	@@D$AT&D1\r$\r\n\
	@@D$AT&C1\r$\r\n\
	@@D$AT+CMGF=1\r$\r\n\
	@@D$AT^SCFG=\"URC/Datamode/Ringline\",\"on\"\r$\r\n\
	@@D$AT+CNMI=3,1,0,0,1\r$\r\n\
	@@D$AT+CLIP=1\r$\r\n\
	@@D$AT^SM20=0\r$\r\n\
	@@0$AT+CREG?\r$0$0$+CREG: 0,1$NOT USE$20$3000$100\r\n\
	@@0$AT+CGSN\r$+CGSN\r\r\n$1$OK$ERROR$3$1000$100\r\n\
	@@0$AT^SCID\r$^SCID: $2$OK$ERROR$3$1000$100\r\n\
	@@0$AT+CIMI\r$+CIMI\r\r\n$6$OK$ERROR$3$3000$100\r\n\
	@@0$AT+COPS=0,2\r$0$0$OK$NOT USE$20$3000$100\r\n\
	@@G$AT^SIND=nitz,1\r$^SIND: nitz,1,\"$5$OK$ERROR$1$3000$100\r\n\
	@@G$AT+CSQ\r$+CSQ:$4$OK$ERROR$1$2000$100\r\n\
	@@S"
};

const uint8_t ampm_AtGsmCmdStartBGS2_W[] = {
	"@@0$AT\r$0$0$OK$NOT USE$3$1000$100\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATI\r$\r\n\
	@@D$AT&D1\r$\r\n\
	@@D$AT&C1\r$\r\n\
	@@D$AT+CMGF=1\r$\r\n\
	@@D$AT^SCFG=\"URC/Datamode/Ringline\",\"on\"\r$\r\n\
	@@D$AT+CNMI=3,1,0,0,1\r$\r\n\
	@@D$AT+CLIP=1\r$\r\n\
	@@D$AT^SM20=0\r$\r\n\
	@@0$AT+CREG?\r$0$0$+CREG: 0,1$NOT USE$20$3000$100\r\n\
	@@0$AT+CGSN\r$+CGSN\r\r\n$1$OK$ERROR$3$1000$100\r\n\
	@@0$AT^SCID\r$^SCID: $2$OK$ERROR$3$1000$100\r\n\
	@@0$AT+CIMI\r$+CIMI\r\r\n$6$OK$ERROR$3$3000$100\r\n\
	@@0$AT+COPS=0,2\r$0$0$OK$NOT USE$20$3000$100\r\n\
	@@G$AT^SIND=nitz,1\r$^SIND: nitz,1,\"$5$OK$ERROR$1$3000$100\r\n\
	@@G$AT+CSQ\r$+CSQ:$4$OK$ERROR$1$2000$100\r\n\
	@@S"
};

//@@D$AT+CSCS=\"UCS2\"\r$\r\n

const uint8_t ampm_AtGsmCmdStartBGS2_E[] = {
	"@@0$AT\r$0$0$OK$NOT USE$3$1000$100\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATI\r$\r\n\
	@@D$AT&D1\r$\r\n\
	@@D$AT&C1\r$\r\n\
	@@D$AT+CMGF=1\r$\r\n\
	@@D$AT^SCFG=\"URC/Datamode/Ringline\",\"on\"\r$\r\n\
	@@D$AT+CNMI=3,1,0,0,1\r$\r\n\
	@@D$AT+CLIP=1\r$\r\n\
	@@D$AT+CUSD=1\r$\r\n\
	@@D$AT+CSCS=\"GSM\"\r$\r\n\
	@@D$AT^SM20=0\r$\r\n\
	@@0$AT+CREG?\r$0$0$+CREG: 0,1$NOT USE$20$3000$100\r\n\
	@@0$AT+CGSN\r$+CGSN\r\r\n$1$OK$ERROR$3$1000$100\r\n\
	@@0$AT^SCID\r$^SCID: $2$OK$ERROR$3$1000$100\r\n\
	@@0$AT+CIMI\r$+CIMI\r\r\n$6$OK$ERROR$3$3000$100\r\n\
	@@0$AT+COPS?\r$+COPS: 0,0,\"$7$OK$ERROR$3$1000$100\r\n\
	@@0$AT+COPS=0,2\r$0$0$OK$NOT USE$20$3000$100\r\n\
	@@G$AT^SIND=nitz,1\r$^SIND: nitz,1,\"$5$OK$ERROR$1$3000$100\r\n\
	@@G$AT+CSQ\r$+CSQ:$4$OK$ERROR$1$2000$100\r\n\
	@@S"
};


const uint8_t ampm_AtGsmCmdStartBG2_E[] = {
	"@@0$AT\r$0$0$OK$NOT USE$3$1000$100\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATI\r$\r\n\
	@@D$AT&D1\r$\r\n\
	@@D$AT&C1\r$\r\n\
	@@D$AT\\Q3\r$\r\n\
	@@D$AT+CMGF=1\r$\r\n\
	@@D$AT^SCFG=\"URC/Datamode/Ringline\",\"on\"\r$\r\n\
	@@D$AT+CNMI=3,1,0,0,1\r$\r\n\
	@@D$AT+CLIP=1\r$\r\n\
	@@0$AT+CREG?\r$0$0$+CREG: 0,1$NOT USE$20$3000$100\r\n\
	@@0$AT+CGSN\r$+CGSN\r\r\n$1$OK$ERROR$3$1000$100\r\n\
	@@0$AT^SCID\r$^SCID: $2$OK$ERROR$3$1000$100\r\n\
	@@0$AT+CIMI\r$+CIMI\r\r\n$6$OK$ERROR$3$3000$100\r\n\
	@@0$AT+COPS=0,2\r$0$0$OK$NOT USE$20$3000$100\r\n\
	@@G$AT^SIND=nitz,1\r$^SIND: nitz,1,\"$5$OK$ERROR$1$3000$100\r\n\
	@@G$AT+CSQ\r$+CSQ:$4$OK$ERROR$1$2000$100\r\n\
	@@S"
};





const uint8_t ampm_AtGsmCmdStartM95[] = {
	"@@0$AT\r$0$0$OK$NOT USE$3$1000$100\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATI\r$\r\n\
	@@D$AT&C1\r$\r\n\
	@@D$AT+CMGF=1\r$\r\n\
	@@D$AT+CNMI=2,1,0,0,0\r$\r\n\
	@@D$AT+CLIP=1\r$\r\n\
	@@D$AT+CUSD=1\r$\r\n\
	@@D$AT+CSCS=\"GSM\"\r$\r\n\
	@@D$AT+QAUDCH=2\r$\r\n\
	@@0$AT+CREG?\r$0$0$+CREG: 0,1$NOT USE$20$3000$100\r\n\
	@@0$AT+CGSN\r$+CGSN\r\r\n$1$OK$ERROR$3$3000$100\r\n\
	@@0$AT+QCCID\r$+QCCID\r\r\n$2$OK$ERROR$3$3000$100\r\n\
	@@0$AT+CIMI\r$+CIMI\r\r\n$6$OK$ERROR$3$3000$100\r\n\
	@@0$AT+COPS=0,2\r$0$0$OK$NOT USE$20$3000$100\r\n\
	@@0$AT+COPS?\r$+COPS: 0,2,\"$7$OK$ERROR$3$1000$100\r\n\
	@@G$AT+CSQ\r$+CSQ:$4$OK$ERROR$1$2000$100\r\n\
	@@S"
};


const uint8_t ampm_AtGsmCmdStartM26[] = {
	"@@0$AT\r$0$0$OK$NOT USE$3$1000$100\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATI\r$\r\n\
	@@D$AT&C1\r$\r\n\
	@@D$AT+CMGF=1\r$\r\n\
	@@D$AT+CNMI=2,1,0,0,0\r$\r\n\
	@@D$AT+CLIP=1\r$\r\n\
	@@0$AT+CREG?\r$0$0$+CREG: 0,1$NOT USE$20$3000$100\r\n\
	@@0$AT+CGSN\r$+CGSN\r\r\n$1$OK$ERROR$3$3000$100\r\n\
	@@0$AT+QCCID\r$+QCCID\r\r\n$2$OK$ERROR$3$3000$100\r\n\
	@@0$AT+COPS=0,2\r$0$0$OK$NOT USE$20$3000$100\r\n\
	@@0$AT+COPS?\r$+COPS: 0,2,\"$7$OK$ERROR$3$1000$100\r\n\
	@@G$AT+CSQ\r$+CSQ:$4$OK$ERROR$1$2000$100\r\n\
	@@S"
};

const uint8_t ampm_AtGsmCmdStartSIM900[] = {
	"@@0$AT\r$0$0$OK$NOT USE$3$1000$100\r\n\
	@@D$ATZ\r$\r\n\
	@@D$ATI\r$\r\n\
	@@D$AT&C1\r$\r\n\
	@@D$AT+CMGF=1\r$\r\n\
	@@D$AT+CNMI=2,1,0,0,0\r$\r\n\
	@@D$AT+CLIP=1\r$\r\n\
	@@0$AT+CREG?\r$0$0$+CREG: 0,1$NOT USE$20$3000$100\r\n\
	@@0$AT+CGSN\r$+CGSN\r\r\n$1$OK$ERROR$3$1000$100\r\n\
	@@0$AT+COPS=0,2\r$0$0$OK$NOT USE$20$3000$100\r\n\
	@@G$AT+CSQ\r$+CSQ:$4$OK$ERROR$1$2000$100\r\n\
	@@S"
};



const uint8_t ampm_AtGsmCmdStartUbloxG100[] = {
	"@@0$AT\r$0$0$OK$NOT USE$3$1000$100\r\n\
	@@D$ATZ\r$\r\n\
	@@D$AT&K0\r$\r\n\
	@@D$AT+UPSV=2\r$\r\n\
	@@D$AT+CMGF=1\r$\r\n\
	@@D$AT+CNMI=2,1,0,0,0\r$\r\n\
	@@D$AT+CLIP=1\r$\r\n\
	@@0$AT+CREG?\r$0$0$+CREG: 0,1$NOT USE$20$3000$100\r\n\
	@@0$AT+CGSN\r$+CGSN\r\r\n$1$OK$ERROR$3$1000$100\r\n\
	@@0$AT+CCID\r$+CCID: $2$OK$ERROR$3$1000$100\r\n\
	@@0$AT+COPS=0,2\r$0$0$OK$NOT USE$20$3000$100\r\n\
	@@G$AT+CSQ\r$+CSQ:$4$OK$ERROR$1$2000$100\r\n\
	@@S"
};


extern uint8_t *ampm_AtGsmCmdStart_pt;
extern void PPP_Init(void);
extern void RESOLVER_Manage(void);
extern int8_t PPP_IsLinked(void);
extern uint8_t PPP_IsDead(void);
extern void RESOLVER_Reset(void);
extern void PPP_SetConnect(void);
extern uint32_t PPP_ManageLink(void);
extern void PPP_SetAuthentication(int8_t* usr, int8_t* pwd);
extern void Ampm_MainSmsRecvCallback(uint8_t *buf);
uint32_t Ampm_MainSwithToIdle(uint16_t cnt, uint8_t c);
uint8_t flagSystemStatus = 0;

enum PPPStatus
{
  PPP_CONNECTING,
  PPP_CONNECTED,
  PPP_DISCONNECTED,
};

#ifdef PPP_LWIP
#include "netif/ppp/pppos.h"
#include "netif/ppp/pppapi.h"
char *pppUser;
char *pppPass;
/* The PPP control block */
ppp_pcb *ppp;
extern void tcpip_thread(void *arg);
/* The PPP IP interface */
struct netif ppp_netif;
uint8_t pppBuf[256];
#endif

#define AMPM_CHECK_SMS_INTERVAL 3600000 //60min



AMPM_GSM_MODE ampm_GSM_mode = AMPM_GSM_NONE;


extern uint16_t CMD_CfgParse(char *buff,uint8_t *smsSendBuff,uint32_t smsLenBuf,uint16_t *dataOutLen,uint8_t pwdCheck);

AMPM_GSM_MAIN_PHASE_TYPE ampmGSM_TaskPhase;

uint32_t GsmGetUTCTime(uint16_t cnt,uint8_t c);
uint32_t GsmGetTime(uint16_t cnt,uint8_t c);
GSM_DATE_TIME sysGsmTime;
uint8_t gsmGetTimeFlag = 0;
int32_t sysGsmTimezone = 0;

uint32_t ampmGsm1SecCnt = 0;
uint32_t smsCheckTime = 0;
uint8_t gsmMainTaskTryCnt;
Timeout_Type tAmpmMainTimeoutTask;
Timeout_Type tRingTask;

uint8_t gsmStatus = 0;
uint16_t smsBusyCnt = 0;
uint16_t smsNotBusyCnt = 0;
uint8_t Ampm_GsmMainCallPhaseFinished(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmMainATPhaseFinished(AMPM_CMD_PHASE_TYPE phase);
extern void all_sms_init(void);
uint8_t  (*Ampm_TcpIpCallback)(void);
uint8_t  (*Ampm_TcpIpInit)(void);


const AMPM_GSM_AT_CMD_PACKET_TYPE smsRecvCmd_Read_AT_CCLK = {"AT+CCLK?\r",NULL,"+CCLK: \"",GsmGetTime,"OK","ERROR"};//delay = 500,timeout = 3000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE sendCmd_AT_CGSN = {"AT+CGSN\r",NULL,"+CGSN: \"",Ampm_GSM_GetIMEI,"OK","ERROR"};//delay = 500,timeout = 3000,retry = 1

const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CFUN_1 = {"AT+CFUN=1\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CFUN_9 = {"AT+CFUN=9\r",NULL,NULL,NULL,"OK","ERROR"};//delay = 100,timeout = 1000,retry = 1

const AMPM_CMD_PROCESS_TYPE ampmMainCmdProcess_CHLD	= {
		(void *)&ampmCmdProcess_AT,(void *)&ampmAtCmd_AT_CHLD	,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmMainCallPhaseFinished,1,2000,500};

const AMPM_CMD_PROCESS_TYPE ampmMainCmdProcess_End	= {
NULL,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,2000,100};		
		
const AMPM_CMD_PROCESS_TYPE ampmMainCmdProcess_AT_CFUN_1	= {
(void *)&ampmMainCmdProcess_End,(void *)&ampmAtCmd_AT_CFUN_1,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,3000,100};

const AMPM_CMD_PROCESS_TYPE ampmMainCmdProcess_AT_CFUN_9	= {
NULL,(void *)&ampmAtCmd_AT_CFUN_9,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,3000,100};	

const AMPM_CMD_PROCESS_TYPE ampmMainCmdProcess_ATH	= {
(void *)&ampmMainCmdProcess_AT_CFUN_9,(void *)&ampmAtCmd_ATH,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,3000,100};
		
const AMPM_CMD_PROCESS_TYPE ampmMainCmdProcess_AT	= {
(void *)&ampmMainCmdProcess_ATH,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmMainATPhaseFinished,1,3000,500};




enum PPPStatus ppp_status = PPP_DISCONNECTED;

#ifdef PPP_LWIP
#include "lwip/dns.h"
//Link Callback
void pppCallback(void *ctx,int errCode, void *arg)
{
  switch ( errCode )
  {
    //No error
    case PPPERR_NONE:
      {
        struct ppp_addrs* addrs = (struct ppp_addrs*) arg;
        //m_ip = IpAddr(&(addrs->our_ipaddr)); //Set IP
      }
      ppp_status = PPP_CONNECTED;
      break;
    default:
      //Disconnected
      AMPM_GSM_LIB_DBG("PPPNetIf: Callback errCode = %d.\n", errCode);
      ppp_status = PPP_DISCONNECTED;
    break;
  }
}

/* PPP status callback example */
static void status_cb(ppp_pcb *pcb, int err_code, void *ctx) {
  struct netif *pppif = ppp_netif(pcb);
  LWIP_UNUSED_ARG(ctx);

  switch(err_code) {
    case PPPERR_NONE: {
#if LWIP_DNS
      const ip_addr_t *ns;
#endif /* LWIP_DNS */
			ppp_status = PPP_CONNECTED;
			//pppapi_set_default(ppp);
			netif_set_default(ppp->netif);
      printf("status_cb: Connected\n");
#if PPP_IPV4_SUPPORT
      printf("   our_ipaddr  = %s\n", ipaddr_ntoa(&pppif->ip_addr));
      printf("   his_ipaddr  = %s\n", ipaddr_ntoa(&pppif->gw));
      printf("   netmask     = %s\n", ipaddr_ntoa(&pppif->netmask));
#if LWIP_DNS
      ns = dns_getserver(0);
      printf("   dns1        = %s\n", ipaddr_ntoa(ns));
      ns = dns_getserver(1);
      printf("   dns2        = %s\n", ipaddr_ntoa(ns));
#endif /* LWIP_DNS */
#endif /* PPP_IPV4_SUPPORT */
#if PPP_IPV6_SUPPORT
      printf("   our6_ipaddr = %s\n", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
#endif /* PPP_IPV6_SUPPORT */
      break;
    }
    case PPPERR_PARAM: {
      printf("status_cb: Invalid parameter\n");
      break;
    }
    case PPPERR_OPEN: {
      printf("status_cb: Unable to open PPP session\n");
      break;
    }
    case PPPERR_DEVICE: {
      printf("status_cb: Invalid I/O device for PPP\n");
      break;
    }
    case PPPERR_ALLOC: {
      printf("status_cb: Unable to allocate resources\n");
      break;
    }
    case PPPERR_USER: {
      printf("status_cb: User interrupt\n");
      break;
    }
    case PPPERR_CONNECT: {
			ppp_status = PPP_DISCONNECTED;
      printf("status_cb: Connection lost\n");
      break;
    }
    case PPPERR_AUTHFAIL: {
			ppp_status = PPP_DISCONNECTED;
      printf("status_cb: Failed authentication challenge\n");
      break;
    }
    case PPPERR_PROTOCOL: {
			ppp_status = PPP_DISCONNECTED;
      printf("status_cb: Failed to meet protocol\n");
      break;
    }
    case PPPERR_PEERDEAD: {
      printf("status_cb: Connection timeout\n");
			ppp_status = PPP_DISCONNECTED;
      break;
    }
    case PPPERR_IDLETIMEOUT: {
      printf("status_cb: Idle Timeout\n");
			ppp_status = PPP_DISCONNECTED;
      break;
    }
    case PPPERR_CONNECTTIME: {
      printf("status_cb: Max connect time reached\n");
			ppp_status = PPP_DISCONNECTED;
      break;
    }
    case PPPERR_LOOPBACK: {
      printf("status_cb: Loopback detected\n");
      break;
    }
    default: {
      printf("status_cb: Unknown error code %d\n", err_code);
      break;
    }
  }

/*
 * This should be in the switch case, this is put outside of the switch
 * case for example readability.
 */

  if (err_code == PPPERR_NONE) {
    return;
  }

  /* ppp_close() was previously called, don't reconnect */
  if (err_code == PPPERR_USER) {
    /* ppp_free(); -- can be called here */
    return;
  }

  /*
   * Try to reconnect in 30 seconds, if you need a modem chatscript you have
   * to do a much better signaling here ;-)
   */
  ppp_connect(pcb, 30);
  /* OR ppp_listen(pcb); */
}

/*
 * PPPoS serial output callback
 *
 * ppp_pcb, PPP control block
 * data, buffer to write to serial port
 * len, length of the data buffer
 * ctx, optional user-provided callback context pointer
 *
 * Return value: len if write succeed
 */
static u32_t output_cb(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx) {
	int i;
	for(i = 0;i < len;i++)
		COMM_Putc(data[i]);
}

u32_t data_tryread(ppp_pcb *pcb,u8_t *data, u32_t len)
{
  u32_t recvd = 0; //bytes received
  while(len)
  {
    if(COMM_Getc(data) == 0)
    {
      data++;
			len--;
			recvd++;
    }
		else
			return recvd;
  }
  return recvd;
}
#endif

void Ampm_Gsm_Error(char *err)
{
	//while(1)
	{
		SysTick_DelayMs(1000);
		AMPM_GSM_LIB_DBG("AmpmGsmError:%s\r\n",err);
	}
}


void Ampm_GsmSetMode(AMPM_GSM_MODE mode)
{
	ampm_GSM_mode = mode;
}


uint8_t Ampm_GsmMainATPhaseFinished(AMPM_CMD_PHASE_TYPE phase)
{
	if(ampmGSM_TaskPhase == AMPM_GSM_MAIN_GO_TO_SLEEP_PHASE)
	{
		Ampm_SendAtCmdNow(&ampmMainCmdProcess_ATH);
	}
	else//wake up
	{
		Ampm_SendAtCmdNow(&ampmMainCmdProcess_AT_CFUN_1);
	}
	return 1;
}

uint8_t Ampm_GsmMainCallPhaseFinished(AMPM_CMD_PHASE_TYPE phase)
{
		ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
		return 1;
}

uint8_t Ampm_GsmIsWorking(void)
{
	if(ampmGSM_TaskPhase == AMPM_GSM_MAIN_DATACALL_PHASE)
	{
		return 1;
	}	
	return 0;
}

uint8_t Ampm_Gsm_IsIdle(void)
{
	if(ampmGSM_TaskPhase == AMPM_GSM_MAIN_IDLE_PHASE
	&& Ampm_Ringing_GetPhase() != AMPM_GSM_RING_IDLE_PHASE
	&& Ampm_SmsCheckMessage_IsEmpty() 
	&& Ampm_VoiceCallCheckList_IsEmpty()
	&& ampm_NewSmsFlag == 0
	)
	{
		return 1;
	}	
	return 0;
}

uint8_t Ampm_GsmIsGood(void)
{
	if(ampmGSM_TaskPhase > AMPM_GSM_MAIN_STARTUP_PHASE)
	{
		return 1;
	}	
	return 0;
}

uint32_t Ampm_MainSwithToIdle(uint16_t cnt, uint8_t c)
{
	//IO_ToggleSetStatus(&ledGprsCtrl,100,1000,IO_TOGGLE_ENABLE,0xffffffff);
	flagSystemStatus &= ~(SYS_GPRS_OK | SYS_SERVERS_MASH);
	Ampm_GSM_DialUp_Reset();
	#ifdef PPP_UIP
	PPP_Init();
	#elif defined(PPP_LWIP)
	if(ppp)
		ppp_close(ppp, 0);
	#endif
	Ampm_TcpIpInit();
	ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
	return 0;
}

AMPM_GSM_MAIN_PHASE_TYPE Ampm_GsmGetPhase(void)
{
	return ampmGSM_TaskPhase;
}

uint8_t TcpIpTaskInit(void)
{
	return 0;
}

uint8_t TcpIpTask(void)
{	
    return 0;
}


void AMPM_GSM_Init(char *apn, char *usr,char *pwr,uint8_t (*tcpIpCallback)(void),uint8_t (*tcpIpInit)(void))
{
	Ampm_GsmSetApn((uint8_t *)apn);
	#ifdef PPP_UIP
	PPP_SetAuthentication((int8_t *)usr, (int8_t *)pwr);
	#elif defined(PPP_LWIP)
	pppUser = usr;
	pppPass = pwr;
	tcpip_init(NULL,NULL);
	ppp = pppos_create(&ppp_netif,output_cb, status_cb,NULL);
	ppp_set_auth(ppp, PPPAUTHTYPE_PAP, pppUser, pppPass);
	#endif
	if(tcpIpCallback)
		Ampm_TcpIpCallback = tcpIpCallback;
	else
		Ampm_TcpIpCallback = TcpIpTask;
	if(tcpIpInit)
		Ampm_TcpIpInit = tcpIpInit;
	else
		Ampm_TcpIpInit = TcpIpTaskInit;
	ampmGSM_TaskPhase = AMPM_GSM_MAIN_INIT_PHASE;
	Ampm_SmsTask_Init(Ampm_MainSmsRecvCallback);
	AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_INIT_PHASE \r\n");
	#ifdef CMSIS_OS_RTX
	osThreadCreate (osThread(AMPM_GSM_MainTask), NULL);
	#endif

}

void AMPM_GSM_MainTask(void const * argument)
{
	uint8_t ch;
	#ifdef CMSIS_OS_RTX 
	for(;;)
	#endif
	{	
		
		if(ampm_ModemResetFlag)
		{
			ampm_ModemResetFlag = 0;
			ampmGSM_TaskPhase = AMPM_GSM_MAIN_INIT_PHASE;
			AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_INIT_PHASE \r\n");
		}
		else if(smsTaskFailCnt >= SMS_MAX_RETRY)
		{
			Ampm_Gsm_Error("Sms service error\r\n");
			Ampm_SmsTask_Init(Ampm_MainSmsRecvCallback);
			smsTaskFailCnt = 0;
			all_sms_init();
			ampmGSM_TaskPhase = AMPM_GSM_MAIN_INIT_PHASE;
			AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_INIT_PHASE \r\n");
		}
		switch(ampmGSM_TaskPhase)
		{
			case AMPM_GSM_MAIN_INIT_PHASE:
//				LedSetStatus(&led1Ctr,500,500,LED_TURN_OFF,0xffffffff);
				gsmStatus = 0;
				gotoDataModeFlag = 0;
				gotoCmdModeFlag = 0;
				Ampm_RingingReset();
				AMPM_GSM_Startup_Init();
				Ampm_GSM_DialUp_Reset();
				#ifdef PPP_UIP
				PPP_Init();
				#elif defined(PPP_LWIP)
				#endif
				Ampm_TcpIpInit();
				Ampm_CallTask_Init(NULL);
				list_init(ampm_GSM_CmdList);
				ampm_GSM_CmdPhase = AMPM_CMD_OK;
				ampmGSM_TaskPhase = AMPM_GSM_MAIN_STARTUP_PHASE;
				flagSystemStatus &= ~(SYS_GSM_OK | SYS_GPRS_OK | SYS_SERVERS_MASH);
				AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_STARTUP_PHASE \r\n");
				InitTimeout(&tAmpmMainTimeoutTask,SYSTICK_TIME_SEC(60));
			break;
			case AMPM_GSM_MAIN_STARTUP_PHASE:
				if(AMPM_GSM_Startup((const uint8_t *)ampm_AtGsmCmdStart)){
					//IO_ToggleSetStatus(&ledGprsCtrl,100,1000,IO_TOGGLE_ENABLE,0xffffffff);
					//LedSetStatus(&led1Ctr,500,0,LED_TURN_ON,0xffffffff);
					if (flagGotSimCPOS && strstr((char *)gsmSimCPOSBuf, VIETTEL_PLMN)) {
						flagSystemStatus |= SYS_SIM_VIETTEL_OK;
					}
					else{
						flagSystemStatus &= ~SYS_SIM_VIETTEL_OK;
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_INIT_PHASE;
						break;
					}
					flagSystemStatus |= SYS_GSM_OK;
					gsmStatus = 1;
					//Ampm_SendSmsPolling("0978779222","hello\n");
					if(gsmGetTimeFlag && modemJustPowerOff)
					{
						gsmGetTimeFlag = 0;
						modemJustPowerOff = 0;
						Ampm_SendCommand(modemOk,modemError,1000,3,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d\"\r",sysGsmTime.year % 100,sysGsmTime.month,sysGsmTime.mday,sysGsmTime.hour,sysGsmTime.min,sysGsmTime.sec);
					}
					if(Ampm_StartRecvSms())
					{
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_SMS_RECV_PHASE;
						InitTimeout(&tAmpmMainTimeoutTask,SYSTICK_TIME_SEC(60));
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_SMS_RECV_PHASE \r\n");
						break;
					}
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;  
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
				}
			break;
			case	AMPM_GSM_MAIN_SMS_RECV_PHASE:
				if(Ampm_RecvSms_IsFinished() || (CheckTimeout(&tAmpmMainTimeoutTask) == SYSTICK_TIMEOUT)){
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
				}
			break;

			case	AMPM_GSM_MAIN_SMS_SEND_PHASE:
				if(Ampm_SmsTask() || (CheckTimeout(&tAmpmMainTimeoutTask) == SYSTICK_TIMEOUT))
				{
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
					ampm_NewSmsFlag = 1;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
				}
				if(Ampm_SendAtCheck_IsEmpty()
					&& Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase))
				{
					if(Ampm_Ringing_GetPhase() == AMPM_GSM_RING_IS_A_CALL)//if have incomming call
					{
						Ampm_VoiceCallStartRecvCall();
						Ampm_RingingReset();
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_VOICECALL_PHASE;
						InitTimeout(&tAmpmMainTimeoutTask,SYSTICK_TIME_SEC(120));
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_VOICECALL_PHASE \r\n");
					}
				}
			break;
			case	AMPM_GSM_MAIN_VOICECALL_PHASE:
				if(Ampm_CallTask() || (CheckTimeout(&tAmpmMainTimeoutTask) == SYSTICK_TIMEOUT))
				{
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
					Ampm_RingingReset();
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
				}
			break;
			case	AMPM_GSM_MAIN_DATACALL_PHASE:
				gotoDataModeFlag = 0;
				if(gotoCmdModeFlag || ampm_GSM_mode != AMPM_GSM_GPRS_ENABLE){
					//if(CheckTimeout(&tTcpDataIsBusy) == SYSTICK_TIMEOUT 
						//|| Ampm_Ringing_GetPhase() == AMPM_GSM_RING_IS_A_CALL)
					{
						gsmMainTaskTryCnt = 0;
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE;
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE \r\n");
					}
				}
				#ifdef PPP_UIP
				PPP_ManageLink();		
				#endif
				
				#ifdef PPP_UIP
				if(PPP_IsDead())
				#elif defined(PPP_LWIP)
				if(CheckTimeout(&tPPP) == SYSTICK_TIMEOUT)
				{
					InitTimeout(&tPPP,SYSTICK_TIME_MS(100));
					int len = data_tryread(ppp,pppBuf,sizeof(pppBuf));
					if(len > 0)
					{
						pppos_input(ppp,pppBuf,len);
						//pppos_input_tcpip(ppp,pppBuf,len);
					}
				}
				if(ppp_status == PPP_DISCONNECTED)
				#else
				if(1)
				#endif
				{
					//IO_ToggleSetStatus(&ledGprsCtrl,100,1000,IO_TOGGLE_ENABLE,0xffffffff);
					flagSystemStatus &= ~(SYS_GPRS_OK | SYS_SERVERS_MASH);
					Ampm_GSM_DialUp_Reset();
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE \r\n");
				}
				#ifdef PPP_UIP
				else if(PPP_IsLinked() && Ampm_TcpIpCallback && Ampm_TcpIpInit)
				#elif defined(PPP_LWIP)
				else if((ppp_status == PPP_CONNECTED) && Ampm_TcpIpCallback && Ampm_TcpIpInit)
				#endif
				{
					Ampm_RingingEnable();
					#ifdef PPP_UIP
					RESOLVER_Manage();
					#elif defined(PPP_LWIP)
					#endif
					if(Ampm_TcpIpCallback())
					{
						#ifdef PPP_UIP
						PPP_Init();
						#elif defined(PPP_LWIP)
						if(ppp)
						{
							ppp_close(ppp, 0);
							ppp_status = PPP_DISCONNECTED;
						}
						#endif
						//IO_ToggleSetStatus(&ledGprsCtrl,100,1000,IO_TOGGLE_ENABLE,0xffffffff);
						flagSystemStatus &= ~(SYS_GPRS_OK | SYS_SERVERS_MASH);
						Ampm_TcpIpInit();
						Ampm_GSM_DialUp_Reset();
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE;
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE \r\n");
					}
				}
				else
				{
					#ifdef PPP_UIP
					RESOLVER_Reset();
					#endif
				}
			break;
			case AMPM_GSM_MAIN_DIAL_UP_PHASE:
				Ampm_GSM_DialUp();
				if(Ampm_Ringing_GetPhase() == AMPM_GSM_RING_IS_A_CALL)
				{
//					AMPM_GSM_LIB_DBG("*************************\r\n");
//					AMPM_GSM_LIB_DBG("*************************\r\n");
//					ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
//					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
						Ampm_VoiceCallStartRecvCall();
						Ampm_RingingReset();
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_VOICECALL_PHASE;
						InitTimeout(&tAmpmMainTimeoutTask,SYSTICK_TIME_SEC(120));
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_VOICECALL_PHASE \r\n");
					break;
				}
				if(Ampm_GSM_DialUp_IsDone()){
					if(Ampm_GSM_DialUp_IsOk()){
						Ampm_RingingDisable();
						gotoCmdModeFlag = 0;
						gsmStatus = 2;
						#ifdef PPP_UIP
						PPP_SetConnect();
						#elif defined(PPP_LWIP)
						if(ppp)
						{
							ppp_connect(ppp, 0);
							ppp_status = PPP_CONNECTING;
						}
						#endif
						if(Ampm_TcpIpInit) Ampm_TcpIpInit();
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_DATACALL_PHASE;
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_DATACALL_PHASE \r\n");
					}else{
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
					}
				}
			break;
			case AMPM_GSM_MAIN_GOTO_DATA_MODE_PHASE:
				if(Ampm_Ringing_RI_isActive())
				{					
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
					break;
				}
				#ifdef PPP_UIP
				if(Ampm_GSM_DialUp_IsOk() && PPP_IsLinked())	
				#elif defined(PPP_LWIP)
				if(Ampm_GSM_DialUp_IsOk() && ppp_status == PPP_CONNECTED)
				#endif
				{
						if(Ampm_CmdTask_SendCmd(Ampm_MainSwithToIdle,"NO CARRIER",1000, "CONNECT", modemError, 3000, 3, "ATO\r")  == AMPM_GSM_RES_OK){
							ampmGSM_TaskPhase = AMPM_GSM_MAIN_WAIT_GOTO_DATA_MODE_PHASE;
							AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_WAIT_GOTO_DATA_MODE_PHASE \r\n");
						}
				}else{
					//IO_ToggleSetStatus(&ledGprsCtrl,100,1000,IO_TOGGLE_ENABLE,0xffffffff);
					flagSystemStatus &= ~(SYS_GPRS_OK  | SYS_SERVERS_MASH);
					Ampm_GSM_DialUp_Reset();
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_DIAL_UP_PHASE;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_DIAL_UP_PHASE \r\n");
					gotoCmdModeFlag = 0;
					#ifdef PPP_UIP
					PPP_Init();
					#endif
					Ampm_TcpIpInit();
				}
			break;
			case AMPM_GSM_MAIN_WAIT_GOTO_DATA_MODE_PHASE:
				if(Ampm_Ringing_RI_isActive())
				{					
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE \r\n");
					break;
				}
				if(Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase)){
					if(ampm_GSM_CmdPhase == AMPM_CMD_OK){
						gsmMainTaskTryCnt = 0;
						gotoCmdModeFlag = 0;
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_DATACALL_PHASE;
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_DATACALL_PHASE \r\n");
					}else{
						//IO_ToggleSetStatus(&ledGprsCtrl,100,1000,IO_TOGGLE_ENABLE,0xffffffff);
						flagSystemStatus &= ~(SYS_GPRS_OK | SYS_SERVERS_MASH);
						Ampm_GSM_DialUp_Reset();
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE;
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE \r\n");
					}
				}
			break;
			case AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE:
				gsmMainTaskTryCnt = 0;
				Ampm_RingingEnable();
				ampmGSM_TaskPhase = AMPM_GSM_MAIN_GOTO_CMD_MODE_CHECK_PHASE;
				AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_GOTO_CMD_MODE_CHECK_PHASE \r\n");
				break;
			case AMPM_GSM_MAIN_GOTO_CMD_MODE_CHECK_PHASE:
				if(gsmMainTaskTryCnt && ampm_GSM_CmdPhase == AMPM_CMD_OK)
				{
					gsmMainTaskTryCnt = 0;
					gotoCmdModeFlag = 0;
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
				}
				else if(Ampm_CmdTask_SendCmd(NULL,NULL,500, modemOk, "NO CARRIER", 2000, 0 ,"+++")  == AMPM_GSM_RES_OK){
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_WAIT_GOTO_CMD_MODE_PHASE;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_WAIT_GOTO_CMD_MODE_PHASE \r\n");
				}
			case AMPM_GSM_MAIN_WAIT_GOTO_CMD_MODE_PHASE:
				if(Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase)){
					if(ampm_GSM_CmdPhase == AMPM_CMD_OK){
						gotoCmdModeFlag = 0;
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
						AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_IDLE_PHASE \r\n");
					}else{
						Ampm_CmdTask_SendCmd(NULL,NULL,500, modemOk, modemError, 1000, 1, "AT\r");
						if(gsmMainTaskTryCnt++ < 3){
							ampmGSM_TaskPhase = AMPM_GSM_MAIN_GOTO_CMD_MODE_CHECK_PHASE;
							AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_GOTO_CMD_MODE_CHECK_PHASE \r\n");
						}else{
							ampmGSM_TaskPhase = AMPM_GSM_MAIN_INIT_PHASE;
							AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_INIT_PHASE \r\n");
						}
					}
				}
			break;
			case	AMPM_GSM_MAIN_IDLE_PHASE:
				if(Ampm_Ringing_GetPhase() == AMPM_GSM_RING_IS_A_CALL)//if have incomming call
				{
					Ampm_VoiceCallStartRecvCall();
					Ampm_RingingReset();
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_VOICECALL_PHASE;
					InitTimeout(&tAmpmMainTimeoutTask,SYSTICK_TIME_SEC(120));
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_VOICECALL_PHASE \r\n");
					break;
				}
				else if(!Ampm_VoiceCallCheckList_IsEmpty())
				{
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_VOICECALL_PHASE;
					InitTimeout(&tAmpmMainTimeoutTask,SYSTICK_TIME_SEC(120));
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_VOICE_CALL_OUT_PHASE \r\n");
					break;
				}
				else if(Ampm_SmsCheckMessage_IsEmpty() && 
					(ampm_NewSmsFlag || SysTick_Get() - smsCheckTime > AMPM_CHECK_SMS_INTERVAL))
				{
					
					if((smsNotBusyCnt >= 3))
					{
						smsCheckTime = SysTick_Get();
						if(Ampm_StartRecvSms())
						{
							ampm_NewSmsFlag = 0;
							ampmGSM_TaskPhase = AMPM_GSM_MAIN_SMS_RECV_PHASE;
							InitTimeout(&tAmpmMainTimeoutTask,SYSTICK_TIME_SEC(60));
							AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_SMS_RECV_PHASE \r\n");
						}
					}
					break;
				}
				else if(!Ampm_SmsCheckMessage_IsEmpty())
				{
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_SMS_SEND_PHASE;
					InitTimeout(&tAmpmMainTimeoutTask,SYSTICK_TIME_SEC(60));
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_SMS_SEND_PHASE \r\n");
					break;
				}
				if((Ampm_Ringing_GetPhase() != AMPM_GSM_RING_IDLE_PHASE) 
					|| Ampm_Ringing_RI_isActive()
				)
				{
					break;
				}
				else if(ampm_GSM_mode == AMPM_GSM_GPRS_ENABLE)
				{
					gotoDataModeFlag = 1;
				}
				else if(ampm_GSM_mode == AMPM_GSM_SLEEP_MODE)
				{
					if(Ampm_SendAtCmd(&ampmMainCmdProcess_AT))
					{
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_GO_TO_SLEEP_PHASE;
						break;
					}
				}
				if(gotoDataModeFlag)
				{
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_GOTO_DATA_MODE_PHASE;
					AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:AMPM_GSM_MAIN_GOTO_DATA_MODE_PHASE \r\n");
				}
			break;
			case AMPM_GSM_MAIN_GO_TO_SLEEP_PHASE:
				if(Ampm_SendAtCheck_IsEmpty() && Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase)){
					if(ampm_GSM_CmdPhase == AMPM_CMD_OK){
						#ifdef PPP_UIP
						PPP_Init();
						#endif
						Ampm_TcpIpInit();
						//IO_ToggleSetStatus(&ledGprsCtrl,100,1000,IO_TOGGLE_ENABLE,0xffffffff);
						flagSystemStatus &= ~(SYS_GPRS_OK | SYS_SERVERS_MASH);
						Ampm_GSM_DialUp_Reset();
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_SLEEP_PHASE;
					}else{
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_INIT_PHASE;
					}
				}
			break;
			case AMPM_GSM_MAIN_SLEEP_PHASE:
				if(ampm_GSM_mode != AMPM_GSM_SLEEP_MODE)
				{
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
					MODEM_Wakeup();
					Ampm_SendAtCmdNow(&ampmMainCmdProcess_AT);
					ampmGSM_TaskPhase = AMPM_GSM_MAIN_WAKEUP_PHASE;
				}
			break;
			case AMPM_GSM_MAIN_WAKEUP_PHASE:
				if(Ampm_SendAtCheck_IsEmpty() && Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase)){
					if(ampm_GSM_CmdPhase == AMPM_CMD_OK){
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_IDLE_PHASE;
					}else{
						ampmGSM_TaskPhase = AMPM_GSM_MAIN_INIT_PHASE;
					}
				}
			break;
			default:
			ampmGSM_TaskPhase = AMPM_GSM_MAIN_INIT_PHASE;
			break;
		}
		
		if(
			(!Ampm_SmsCheckMessage_IsEmpty() 
				|| !Ampm_VoiceCallCheckList_IsEmpty()
				|| Ampm_Ringing_GetPhase() != AMPM_GSM_RING_IDLE_PHASE
				|| (ampm_NewSmsFlag && Ampm_Is_RingingEnable())
				|| SysTick_Get() - smsCheckTime > AMPM_CHECK_SMS_INTERVAL
			)
		)
		{
			Ampm_GsmGotoCmdMode();
		}

		if(ampmGSM_TaskPhase != AMPM_GSM_MAIN_DATACALL_PHASE)
		{
			ampm_AtCommandParserEnable = 1;
			Ampm_Cmd_Task(&ampm_GSM_CmdPhase,&ampmGSM_CmdSend,ampm_GSM_CmdList,&ampm_GSM_TaskTimeout);
		}
		else
			ampm_AtCommandParserEnable = 0;

		if(ampmGSM_TaskPhase > AMPM_GSM_MAIN_STARTUP_PHASE){
			Ampm_RingingProcess();
		}

		if(CheckTimeout(&tRingTask) == SYSTICK_TIMEOUT)
		{
			InitTimeout(&tRingTask,SYSTICK_TIME_SEC(1));
			if(!Ampm_SmsCheckMessage_IsEmpty())
			{
				smsNotBusyCnt = 0;
				smsBusyCnt++;
				if(smsBusyCnt >= 600)//10 min
				{
					smsBusyCnt = 0;
					ampm_ModemResetFlag = 1;
				}
			}
			else
			{
				smsNotBusyCnt++;
			}
			ampmGsm1SecCnt++;
			if(ampmGsm1SecCnt % 10 == 0)
			{
				AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:ampmGSM_TaskPhase=%d\r\n",ampmGSM_TaskPhase);
				AMPM_GSM_LIB_DBG("AMPM_GSM_MAIN:ampmGSM_TaskPhase=%d\r\n",ampmGSM_TaskPhase);
			}
			Ampm_RingingTimer();
			Ampm_SmsTaskPeriodic_1Sec();
			Ampm_CallTaskPeriodic_1Sec();
		}
	}
}

//+CCLK: "17/12/15,16:00:58+28"
//+CCLK: "13/12/11,11:15:41+00
//AT+CCLK? +CCLK: "17/12/28,19:42:23+28"
uint32_t GsmGetTime(uint16_t cnt,uint8_t c)
{
	static uint16_t number;
	if(c < '0' && c > '9' && c != '/' && c != ':' && c != ',' && c != '+' && c != '"')
	{
		return 0;
	}
	//AMPM_GSM_LIB_DBG("%c",c);
	switch(cnt)
	{
		case 0:
			number = c - '0';
		break;
		case 1:
			number *= 10;
			number += c - '0';
			sysGsmTime.year = number + 2000;
			if(sysGsmTime.year < 2013)
				return 0;
		break;
		case 3:
			number = c - '0';
		break;
		case 4:
			number *= 10;
			number += c - '0';
			sysGsmTime.month = number;
			if(sysGsmTime.month > 12)
					return 0;
		break;
		case 6:
			number = c - '0';
		break;
		case 7:
			number *= 10;
			number += c - '0';
			sysGsmTime.mday = number;
			if(sysGsmTime.mday > 31)
					return 0;
		break;

		case 9:
			number = c - '0';
		break;
		case 10:
			number *= 10;
			number += c - '0';
			sysGsmTime.hour = number;
			if(sysGsmTime.hour > 24)
				return 0;
		break;
		case 12:
			number = c - '0';
		break;
		case 13:
			number *= 10;
			number += c - '0';
			sysGsmTime.min = number;
			if(sysGsmTime.min > 59)
					return 0;
		break;

		case 15:
			number = c - '0';
		break;
		case 16:
			number *= 10;
			number += c - '0';
			sysGsmTime.sec = number;
			if(sysGsmTime.sec > 59)
					return 0;
		break;
		case 17:
		case 18:
		case 19:
		case 20:
			if(c == '"')
			{
				//gsmGetTimeFlag = 1;
				//UpdateRtcTime(TIME_GetSec((DATE_TIME *)&sysGsmTime));
				AMPM_GSM_LIB_DBG("\"sysGsmTime\":%04u-%02u-%02u %02u:%02u:%02u\r\n",sysGsmTime.year,sysGsmTime.month,sysGsmTime.mday,sysGsmTime.hour,sysGsmTime.min,sysGsmTime.sec);
				#include "lib/sys_time.h"
				//extern void RTC_SetTime(DATE_TIME time);
				//RTC_SetTime(*((DATE_TIME *)&sysGsmTime));
				return 0;
			}
			break;
		case 21:
		return 0;
		default:
		break;
	}
	return 0xff;
}


//^SIND: nitz,1,"13/12/11,11:15:41",+28,0
uint32_t GsmGetUTCTime(uint16_t cnt,uint8_t c)
{
	static uint16_t number,timezone_pos = 0;
	if(c < '0' && c > '9' && c != '/' && c != ':' && c != ',' && c != '+')
	{
		return 0;
	}
	switch(cnt)
	{
		case 0:
			number = c - '0';
		break;
		case 1:
			number *= 10;
			number += c - '0';
			sysGsmTime.year = number + 2000;
			if(sysGsmTime.year < 2013)
				return 0;
		break;
		case 3:
			number = c - '0';
		break;
		case 4:
			number *= 10;
			number += c - '0';
			sysGsmTime.month = number;
			if(sysGsmTime.month > 12)
					return 0;
		break;
		case 6:
			number = c - '0';
		break;
		case 7:
			number *= 10;
			number += c - '0';
			sysGsmTime.mday = number;
			if(sysGsmTime.mday > 31)
					return 0;
		break;

		case 9:
			number = c - '0';
		break;
		case 10:
			number *= 10;
			number += c - '0';
			sysGsmTime.hour = number;
			if(sysGsmTime.hour > 24)
				return 0;
		break;
		case 12:
			number = c - '0';
		break;
		case 13:
			number *= 10;
			number += c - '0';
			sysGsmTime.min = number;
			if(sysGsmTime.min > 59)
					return 0;
		break;

		case 15:
			number = c - '0';
		break;
		case 16:
			number *= 10;
			number += c - '0';
			sysGsmTime.sec = number;
			if(sysGsmTime.sec > 59)
					return 0;
		break;
		case 19:
			if(c == '+')
			{
				timezone_pos = 1;
			}
			else
				return 0;
		break;
		case 20:
			number = c - '0';
		break;
		case 21:
			if(c >= '0' && c <= '9')
			{
				number *= 10;
				number += c - '0';
			}
			if(timezone_pos)
				sysGsmTimezone = (int32_t)number*15*60;
			else
				sysGsmTimezone = -((int32_t)number*15*60);
				gsmGetTimeFlag = 1;
			return 0;
		default:
		break;
	}
	return 0xff;
}



extern const uint8_t ampm_AtGsmCmdStartBG2_E[];
extern const uint8_t ampm_AtGsmCmdStartBGS2_E[];
extern const uint8_t ampm_AtGsmCmdStartM95[];
extern const uint8_t ampm_AtGsmCmdStartM26[];

AMPM_GSM_MODULE_TYPE ampmGsmModuleType = THIS_MODULE_NOT_SUPPORT;

COMPARE_TYPE cmpBGS2_W;
COMPARE_TYPE cmpBGS2_E;
COMPARE_TYPE cmpBGS1_E;
COMPARE_TYPE cmpBG2_E;
COMPARE_TYPE cmpM95;
COMPARE_TYPE cmpM35;
COMPARE_TYPE cmpM26;
COMPARE_TYPE cmpUbloxG100;
COMPARE_TYPE cmp;
COMPARE_TYPE cmpSIM900;

uint32_t Ampm_GSM_GetATI(uint16_t cnt,uint8_t c)
{
	

	if(cnt == 0)
	{
		InitFindData(&cmp,"\r\nOK");
		InitFindData(&cmpBGS2_W,"BGS2-W");
		InitFindData(&cmpBGS2_E,"BGS2-E");
		InitFindData(&cmpBGS1_E,"BGS1-E");
		InitFindData(&cmpBG2_E,"BG2-E");
		InitFindData(&cmpM95,"M95");
		InitFindData(&cmpM35,"M35");
		InitFindData(&cmpM26,"M26");
		InitFindData(&cmpSIM900,"SIM900");
		InitFindData(&cmpUbloxG100,"G100");
	}
	if(FindData(&cmp,c) == 0 || cnt >= 45)
	{
		AMPM_GSM_LIB_DBG("AMPM_GSM_INIT:Got ATI \r\n");
		if(ampmGsmModuleType == THIS_MODULE_NOT_SUPPORT)
		{
			#ifdef PPP_UIP
			PPP_SetAuthenticationLogin(1);
			#endif
			AMPM_GSM_LIB_DBG("Sorry we do not support your module \r\n");
			AMPM_GSM_LIB_DBG("Pls contact thienhaiblue@gmail.com to have a good support \r\n");
			AMPM_GSM_LIB_DBG("Thanks! \r\n");
		}
		return 0;
	}
	if(FindData(&cmpBGS2_W,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(1);
		#endif
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpBGS2_W.buff);
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartBGS2_W;
		ampmGsmModuleType = BGS2_W;
		return 0;
	}
	if(FindData(&cmpBGS2_E,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(1);
		#endif
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpBGS2_E.buff);
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartBGS2_E;
		ampmGsmModuleType = BGS2_E;
		return 0;
	}
	if(FindData(&cmpBGS1_E,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(0);
		#endif
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpBGS1_E.buff);
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartBGS2_E;
		ampmGsmModuleType = BGS2_E;
		return 0;
	}
	if(FindData(&cmpBG2_E,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(1);
		#endif
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartBG2_E;
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpBG2_E.buff);
		ampmGsmModuleType = BG2_E;
		return 0;
	}
	if(FindData(&cmpM95,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(1);
		#endif
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartM95;
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpM95.buff);
		ampmGsmModuleType = M95;
		return 0;
	}
	if(FindData(&cmpM35,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(1);
		#endif
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartM95;
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpM95.buff);
		ampmGsmModuleType = M95;
		return 0;
	}
	if(FindData(&cmpM26,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(1);
		#endif
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartM26;
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpM26.buff);
		ampmGsmModuleType = M26;
		return 0;
	}
	if(FindData(&cmpSIM900,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(1);
		#endif
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartSIM900;
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpSIM900.buff);
		ampmGsmModuleType = SIM900;
		return 0;
	}
	if(FindData(&cmpUbloxG100,c) == 0)
	{
		#ifdef PPP_UIP
		PPP_SetAuthenticationLogin(0);
		#endif
		ampm_AtGsmCmdStart_pt = (uint8_t *)ampm_AtGsmCmdStartUbloxG100;
		AMPM_GSM_LIB_DBG("YOUR MODULE IS:Got ATI:%s \r\n",cmpM95.buff);
		ampmGsmModuleType = UBLOX_G100;
		return 0;
	}
	return 0xff;
}

