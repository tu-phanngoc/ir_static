
#include "ppp.h"
#include "uip_ppp.h"
#include "lib/sys_tick.h"

// number of dialing attempts before reseting physical layer
#define PPP_DIALING_ATTEMPT			3

U8 *pppRxBuf = uip_buf;
#if defined ( __ICCARM__ )
#pragma pack(1) 
U8 pppTxBuf[PPP_BUFFER_SIZE + 1];
#pragma pack()
#elif defined (__CC_ARM)
U8 pppTxBuf[PPP_BUFFER_SIZE + 1] __attribute__((aligned(4)));
#endif



U8 pppUseESC,pppUseESC_flag=0;
U8 pppRestartCount;
U8 pppErrCnt = 0;
U16 pppFrameSize;
U8 pppMsgID;
U32 pppTick;
U32 pppManagerLinkTick = 0;
U8 pppTimerOn;
U8 pppDataPrepared;
U8 pppIpAddr[4];
U8 pppIsEsc = 0;
uint32_t pppPhaseTick = 0;
U8 pppDialAttempts = 0;
I8 pppUsr[16], pppPwd[16];
U8 pppApn_UseLoginPass = 0;

uip_ipaddr_t priDnsServerIp;
uip_ipaddr_t secDnsServerIp;


typedef enum{
	PPP_STATE_INITIAL,
	PPP_STATE_STARTING,
	PPP_STATE_CLOSED,
	PPP_STATE_STOPPED,
	PPP_STATE_CLOSING,
	PPP_STATE_STOPPING,
	PPP_STATE_REQ_SENT,
	PPP_STATE_ACK_RCVD,
	PPP_STATE_ACK_SENT,
	PPP_STATE_OPENED
}PPP_STATE;

PPP_STATE lcpState, papState, ipcpState;

enum{
	PPP_PHASE_DEAD,
	PPP_PHASE_CONNECT,
	PPP_PHASE_ESTABLISH,
	PPP_PHASE_AUTHENTICATION,
	PPP_PHASE_NETWORK,
	PPP_PHASE_TERMINATE,
	PPP_PHASE_IP
}pppPhase;

typedef enum{
	PPP_LAYER_PHYSICAL,
	PPP_LAYER_AUTHENTICATION,
	PPP_LAYER_NETWORK,
}PPP_LAYER;

typedef enum{
	PPP_EVENT_UP,				// lower layer is Up
	PPP_EVENT_DOWN,			// lower layer is Down
	PPP_EVENT_OPEN,			// administrative Open
	PPP_EVENT_CLOSE,		// administrative Close
	PPP_EVENT_TO_PLUS,	// Timeout with counter > 0
	PPP_EVENT_TO_MINUS,	// Timeout with counter expired
	PPP_EVENT_RCR_PLUS,	// Receive-Configure-Request (Good)
	PPP_EVENT_RCR_MINUS,// Receive-Configure-Request (Bad)
	PPP_EVENT_RCA,			// Receive-Configure-Ack
	PPP_EVENT_RCN,			// Receive-Configure-Nak/Rej
	PPP_EVENT_RTR,			// Receive-Terminate-Request
	PPP_EVENT_RTA,			// Receive-Terminate-Ack
	PPP_EVENT_RUC,			// Receive-Unknown-Code
	PPP_EVENT_RXJ_PLUS,	// Receive-Code-Reject (permitted) or Receive-Protocol-Reject
	PPP_EVENT_RXJ_MINUS,// Receive-Code-Reject (catastrophic) or Receive-Protocol-Reject
	PPP_EVENT_RXR				// Receive-Echo-Request or Receive-Echo-Reply or Receive-Discard-Request
}PPP_EVENT;

enum{
	WAIT_HEADER,
	GET_DATA,
}pppInState;

uint16_t PPP__In(void);
static void PPP__ManageState(PPP_STATE *state, PPP_EVENT event, PPP_LAYER layer);
static void PPP__ProcessPacket(void);
static void PPP__Timer(void);
static void PPP__Open(void);
static void PPP__SendConfigReq(void);
static void PPP__SendConfigAck(void);
static void PPP__SendConfigNackRej(void);
static void PPP__SendTermReq(void);
static void PPP__SendTermAck(void);
static void PPP__SendCodeRej(void);
static void PPP__SendEchoReply(void);
static void PPP__ThisLayerUp(PPP_LAYER layer);
static void PPP__ThisLayerDown(PPP_LAYER layer);
static void PPP__ThisLayerStarted(PPP_LAYER layer);
static void PPP__ThisLayerFinished(PPP_LAYER layer);
static I8 PPP__CheckLCPOptions(void);


/**
* \brief Init PPP datalink layer
*/
void PPP_Init(void)
{
	pppPhase = PPP_PHASE_DEAD;
}
/**
* \brief manage internal PPP state machine
*/
void PPP_SetAuthentication(int8_t* usr, int8_t* pwd)
{
	strcpy(pppUsr, usr);
	strcpy(pppPwd, pwd);
}

void PPP_SetAuthenticationLogin(U8 login)
{
	pppApn_UseLoginPass = login;
}
void PPP_SetConnect(void)
{
	uint8_t c;
	pppPhase = PPP_PHASE_CONNECT;
	pppPhaseTick = SysTick_Get();
	while(RINGBUF_Get(commRxRingBuf, &c)==0);
}

uint8_t PPP_IsDead(void)
{
	return pppPhase == PPP_PHASE_DEAD;
}

uint32_t PPP_ManageLink(void)//working
{
	switch(pppPhase){
		case PPP_PHASE_DEAD:	
		pppPhaseTick = SysTick_Get();
		break;
		case PPP_PHASE_CONNECT:
			
			if((SysTick_Get() - pppPhaseTick >= 1000)
			|| RINGBUF_GetFill(commRxRingBuf))
			{
				SysTick_DelayMs(50); 
				PPP__Open(); 
				uip_init();
				pppPhaseTick = SysTick_Get();
				pppManagerLinkTick = SysTick_Get();
			}
		break;
		case PPP_PHASE_ESTABLISH:
		case PPP_PHASE_AUTHENTICATION:
		case PPP_PHASE_NETWORK:
			if(COMM_CarrierDetected())
				PPP__ThisLayerDown(PPP_LAYER_PHYSICAL);
			if(pppErrCnt >= 3 || SysTick_Get() - pppPhaseTick >= 60000) //60Sec
			{
				pppPhase = PPP_PHASE_DEAD;
			}
			PPP__Timer();
			if(SysTick_Get() - pppManagerLinkTick >= 50)
			{
				pppManagerLinkTick = SysTick_Get();
				PPP__In();
			}
			break;
			
		case PPP_PHASE_IP:
			if(COMM_CarrierDetected())
				PPP__ThisLayerDown(PPP_LAYER_PHYSICAL);
				
			PPP__In();
			
			UIP_Periodic();
			break;
		default:
			break;
	}
	return 0;
}
/**
* force PPP to reinit physical layer
*/
void PPP_ReInit(void)
{
	PPP__ThisLayerDown(PPP_LAYER_PHYSICAL);
	pppPhase = PPP_PHASE_DEAD;
}
int8_t PPP_IsLinked(void)
{
	return pppPhase == PPP_PHASE_IP;
}
static void PPP__Open(void)
{
	pppInState = WAIT_HEADER;
	pppIsEsc = 0;
	pppUseESC = 1;
	pppRestartCount = PPP_MAX_RESTART;
	pppErrCnt = 0;
	pppMsgID = 0;
	pppDataPrepared = 0;
	pppTimerOn = 0;
	pppPhase = PPP_PHASE_ESTABLISH;
	lcpState = PPP_STATE_INITIAL;
	papState = PPP_STATE_INITIAL;
	ipcpState = PPP_STATE_INITIAL;
	PPP__ManageState(&lcpState, PPP_EVENT_OPEN, PPP_LAYER_PHYSICAL);
	PPP__ManageState(&lcpState, PPP_EVENT_UP, PPP_LAYER_PHYSICAL);
}

// manage the restart timer
// restart timer expire every 3s
// restart timer only runs in Stopping, Closing, Ack-Received, Ack-Sent state
// and always stop in other states
// Only the Send-Configure-Request, Send-Terminate-Request
// and Zero-Restart-Count actions start or re-start the Restart timer
static void PPP__Timer(void)
{

	if(!pppTimerOn || SysTick_Get() - pppTick < SYSTICK_SECOND * 5)
		return;
	
	pppTick = SysTick_Get();

	if(pppRestartCount)
	{
		PPP_Info("PPP: Event TO+\r\n");
		
		switch(pppPhase)
		{
			case PPP_PHASE_ESTABLISH:
				PPP__ManageState(&lcpState, PPP_EVENT_TO_PLUS, PPP_LAYER_PHYSICAL);
				break;
				
			case PPP_PHASE_AUTHENTICATION:
				PPP__ManageState(&papState, PPP_EVENT_TO_PLUS, PPP_LAYER_AUTHENTICATION);
				break;
				
			case PPP_PHASE_NETWORK:
				PPP__ManageState(&ipcpState, PPP_EVENT_TO_PLUS, PPP_LAYER_NETWORK);
				break;
				
			default:
				break;
		}
		
	}
	else
	{
		PPP_Info("PPP: Event TO-\r\n");
		switch(pppPhase)
		{
			case PPP_PHASE_ESTABLISH:
				PPP__ManageState(&lcpState, PPP_EVENT_TO_MINUS, PPP_LAYER_PHYSICAL);
				break;
				
			case PPP_PHASE_AUTHENTICATION:
				PPP__ManageState(&papState, PPP_EVENT_TO_MINUS, PPP_LAYER_AUTHENTICATION);
				break;
				
			case PPP_PHASE_NETWORK:
				PPP__ManageState(&ipcpState, PPP_EVENT_TO_MINUS, PPP_LAYER_NETWORK);
				break;
				
			default:
				break;
		}
	}
}

uint16_t PPP__In(void)
{	
	U8 c;
	U16 fcs;
	U16 rxFcs;
	U16 res = 0,i;
	U16 exit = 0;
	while(RINGBUF_Get(commRxRingBuf, &c)==0){
		switch(pppInState){
			case WAIT_HEADER:
				if(c != 0x7E) break;
				pppFrameSize = 0;
				pppIsEsc = 0;
				pppInState = GET_DATA;
				break;
				
			case GET_DATA:
				if(c == 0x7E){
					if(pppFrameSize >= 4){
						pppRxBuf[pppFrameSize] = 0;
						fcs = FCS_Get(pppRxBuf, pppFrameSize - 2);
						rxFcs = ((U16)pppRxBuf[pppFrameSize-1]<<8) | pppRxBuf[pppFrameSize-2];
						
						if(fcs == rxFcs){
							
//							PPP_Info("PPP: Got:%dBytes:",pppFrameSize);
//							for(i = 0; i < pppFrameSize;i++)
//							{
//								PPP_Info("%02X",pppRxBuf[i]);	
//							}
//							PPP_Info("\r\n");
							if(pppRxBuf[0] == 0x21)
							{
								for(i = pppFrameSize + 3;i >= 3;i--)
								{
									pppRxBuf[i] = pppRxBuf[i-3];
								}
								pppRxBuf[0] = 0xFF;
								pppRxBuf[1] = 0x03;
								pppRxBuf[2] = 0x00;
								pppFrameSize += 3;
							}
							else if(pppRxBuf[0] != 0xFF && pppRxBuf[1] == 0x21)
							{
								for(i = pppFrameSize + 2;i >= 2;i--)
								{
									pppRxBuf[i] = pppRxBuf[i-2];
								}
								pppRxBuf[0] = 0xFF;
								pppRxBuf[1] = 0x03;
								pppFrameSize += 2;
							}
							PPP__ProcessPacket();
							res = pppFrameSize;
							exit = 1;
							
							pppInState = WAIT_HEADER;
						}else{
							PPP_Info("PPP: FCS fails, discard\r\n");
						}
					}
					pppInState = WAIT_HEADER;
					break;
				}
				
				if(c == 0x7D){
					pppIsEsc = 1;
					break;
				}
				
				if(pppIsEsc){
					c = 0x20 ^ c;
					pppIsEsc = 0;
				}
				pppRxBuf[pppFrameSize++] = c;
				
				break;
		}
		if(exit) break;
	}
	return res;
	
}
static void PPP__ProcessPacket(void)
{
	U16 packetType = ((U16)pppRxBuf[3]<<8) | pppRxBuf[2];
	switch(packetType){
		
		/**************************************
		* LCP
		**************************************/
		case LCP_PACKET:
			PPP_Info("PPP: Got LCP ");
			switch(pppRxBuf[4]){
				case CONFIG_REQ:
					PPP_Info("CONFIG_REQ\r\n");
					switch(PPP__CheckLCPOptions()){
						case 0:
							PPP__ManageState(&lcpState, PPP_EVENT_RCR_PLUS, PPP_LAYER_PHYSICAL);
							break;
						case 1:
							PPP__ManageState(&lcpState, PPP_EVENT_RCR_MINUS, PPP_LAYER_PHYSICAL);
							break;
						case 2:
							PPP__ManageState(&lcpState, PPP_EVENT_RCR_MINUS, PPP_LAYER_PHYSICAL);
							break;
					}
					break;
					
				case CONFIG_ACK:
					PPP_Info("CONFIG_ACK\r\n");
					
					if(pppRxBuf[5] != pppMsgID){
						PPP_Info(", Out of sync\r\n");
						break;
					}
					
					PPP_Info("");
					
					PPP__ManageState(&lcpState, PPP_EVENT_RCA, PPP_LAYER_PHYSICAL);
					break;
					
				case CONFIG_NACK:
					PPP_Info("CONFIG_NACK\r\n");
					
					if(pppRxBuf[5] != pppMsgID){
						PPP_Info(", Out of sync\r\n");
						break;
					}
					
					PPP_Info("");
					
					// remote peer NACK, we take the NACKed option to negotiate again
					// this is an oversimplified implementation, but it should work well
					// because we negotiate the ACCM option only
					// better implementation should take the NACKed options, combined with 
					// others and negotiate again
					memcpy(pppTxBuf, pppRxBuf, sizeof pppTxBuf);
					pppDataPrepared = 1;
					
					PPP__ManageState(&lcpState, PPP_EVENT_RCN, PPP_LAYER_PHYSICAL);
					break;
					
				case CONFIG_REJ:
					PPP_Info("CONFIG_REJ\r\n");
					
					if(pppRxBuf[5] != pppMsgID){
						PPP_Info(", Out of sync\r\n");
						break;
					}
					
					// Should remove the REJed options, an negotiate again
					// Since we only negotiate ACCM, we should never see this packet
					PPP_Info("");
					
					PPP__ManageState(&lcpState, PPP_EVENT_RCN, PPP_LAYER_PHYSICAL);
					break;
				
				case DISC_REQ:
				case ECHO_REPLY:
				case ECHO_REQ:	
					PPP_Info("ECHO_REQ/ECHO_REPLY/DISC_REQ\r\n");
					PPP__ManageState(&lcpState, PPP_EVENT_RXR, PPP_LAYER_PHYSICAL);
					break;
					
				case TERM_REQ:
					PPP_Info("TERM_REQ\r\n");
					
					memcpy(pppTxBuf, pppRxBuf, sizeof pppTxBuf);
					pppTxBuf[4] = TERM_ACK;
					
					PPP__ManageState(&lcpState, PPP_EVENT_RTR, PPP_LAYER_PHYSICAL);
					break;
				
				default:
					PPP_Info("UNKNOWN\r\n");
					PPP__ManageState(&lcpState, PPP_EVENT_RUC, PPP_LAYER_PHYSICAL);
					break;
			}
			break;
			
		/**************************************
		* PAP
		**************************************/
		case PAP_PACKET:
			PPP_Info("PPP: Got PAP ");
			switch(pppRxBuf[4]){
				case CONFIG_REQ:
					// Authentication is our job, remote peer should never send this packet
					PPP_Info("CONFIG_REQ\r\n");
					PPP__ManageState(&papState, PPP_EVENT_RCR_PLUS, PPP_LAYER_AUTHENTICATION);
					break;
					
				case CONFIG_ACK:
					PPP_Info("CONFIG_ACK");
					
					if(pppRxBuf[5] != pppMsgID){
						PPP_Info(", Out of sync\r\n");
						break;
					}
					
					PPP_Info("");
					// remote peer should never send CONFIG_REQ, 
					// so we have to give FSM the RCR+ event manually
					PPP__ManageState(&papState, PPP_EVENT_RCR_PLUS, PPP_LAYER_AUTHENTICATION);
					PPP__ManageState(&papState, PPP_EVENT_RCA, PPP_LAYER_AUTHENTICATION);
					break;
					
				case CONFIG_NACK:
					PPP_Info("CONFIG_NACK");
					
					if(pppRxBuf[5] != pppMsgID){
						PPP_Info(", Out of sync\r\n");
						break;
					}
					// authentication failed
					PPP_Info("");
					PPP__ManageState(&papState, PPP_EVENT_RCN, PPP_LAYER_AUTHENTICATION);
					break;
					
				case CONFIG_REJ:
					PPP_Info("CONFIG_REJ");
					
					if(pppRxBuf[5] != pppMsgID){
						PPP_Info(", Out of sync\r\n");
						break;
					}
					
					PPP_Info("");
					PPP__ManageState(&papState, PPP_EVENT_RCA, PPP_LAYER_AUTHENTICATION);
					break;
					
			}
			break;
			
		/**************************************
		* IPCP
		**************************************/
		case IPCP_PACKET:
			PPP_Info("PPP: Got IPCP ");
			switch(pppRxBuf[4]){
				case CONFIG_REQ:								// this packet contains our IP address, we will ack and then
					PPP_Info("CONFIG_REQ\r\n");
					PPP__ManageState(&ipcpState, PPP_EVENT_RCR_PLUS, PPP_LAYER_NETWORK);
					break;
					
				case CONFIG_NACK:
					PPP_Info("CONFIG_NACK");
					
					if(pppRxBuf[5] != pppMsgID){
						PPP_Info(", Out of sync\r\n");
						break;
					}
					
					PPP_Info("");
					
					memcpy(pppTxBuf, pppRxBuf, sizeof pppTxBuf);
					pppDataPrepared = 1;
					
					PPP__ManageState(&ipcpState, PPP_EVENT_RCN, PPP_LAYER_NETWORK);
					break;
					
				case CONFIG_ACK:								// link is established successfully
					PPP_Info("CONFIG_ACK");
					
					if(pppRxBuf[5] != pppMsgID){
						PPP_Info(", Out of sync\r\n");
						break;
					}
					
					PPP_Info("");
					
					PPP__ManageState(&ipcpState, PPP_EVENT_RCA, PPP_LAYER_NETWORK);
					break;
			}
			break;
		
		/**************************************
		* IP DATAGRAM
		**************************************/
		case IP_DATAGRAM:					// PPP link is established successfully, we will pass all IP packet 
			PPP_Debug("PPP: Got IP\r\n");
			UIP_Input();
			break;
			
	}
}
static I8 PPP__CheckLCPOptions(void)
{
	U16 reqPacketLen;
	U8 *ptr = &pppRxBuf[8];
	U8 *ptr2 = &pppTxBuf[8];
	U8 flag = 0;
	U8 len = pppRxBuf[7] - 4;	// option data length = payload length - (FCS = 2) - (length field itself = 2)
	pppTxBuf[0] = 0xFF;
	pppTxBuf[1] = 0x03;
	pppTxBuf[2] = 0xC0;
	pppTxBuf[3] = 0x21;
	pppTxBuf[5] = pppRxBuf[5];
	pppTxBuf[7] = 4;
	
	while(len){
		switch(*ptr){
			case 1:						// MRU
				reqPacketLen = ((U16)ptr[3]<<8) | ptr[2];
				
				if(reqPacketLen < sizeof uip_buf){
					if(flag<2){			// if there is not any options to be rejected
						flag = 1;		// signal to NACK
						*ptr2++ = 1;*ptr2++ = 4;
						*ptr2++ = (sizeof uip_buf)>>8;*ptr2++ = (sizeof uip_buf)&0xFF;
						pppTxBuf[7] += 4;
					}
				}
				
				ptr += 4;
				len -= 4;
				break;
				
			case 2:				// ACCM, dont bother to negotiate this option
				if(!ptr[2] && !ptr[3] && !ptr[4] && !ptr[5]){
					pppUseESC_flag = 1;
				}
//				else if(flag < 2)
//				{
//				/*
//						 * fail we only support default or all zeros
//						 */
//						PPP_Info(("We only support default or all zeros for ACCM "));
//						flag = 1;
//						*ptr2++ = 2;
//						*ptr2++ = 0x6;
//						*ptr2++ = 0;
//						*ptr2++ = 0;
//						*ptr2++ = 0;
//						*ptr2++ = 0;
//				}
				ptr += 6;
				len -= 6;
				break;
				
			case 3:				// Authentication protocol
				if(ptr[2]!=0xC0 || ptr[3]!=0x23){		// only accept PAP
					if(flag<2){							// if there was not any options to be rejected
						flag = 1;						// signal to NACK, we only accept PAP
						*ptr2++ = 3;*ptr2++ = 4;
						*ptr2++ = 0xC0;*ptr2++ = 0x23;
						pppTxBuf[7] += 4;
					}
				}
				len -= ptr[1];
				ptr += ptr[1];
				break;
				
			case 4:				// Quality-Protocol
				if(ptr[1] == 0){
					PPP_Info("PPP: LCP Option len==0\r\n");
					len = 0;
					break;
				}
				
				if(flag!=2)
					ptr2 = &pppTxBuf[8];
				flag = 2;
				memcpy(ptr2, ptr, ptr[1]);
				pppTxBuf[7] += ptr[1];
				len -= ptr[1];
				ptr += ptr[1];
				break;
				
			case 5:				// Magic number, just ACK, do nothing
				len -= 6;
				ptr += 6;
				break;
				
			case 7:				// Protocol compression, just ACK
				len -= 2;
				ptr += 2;
				break;
			case 8:				// Address + Control field compression, just ACK
				len -= 2;
				ptr += 2;
				break;
				
			default:
				PPP_Info("PPP: Invalid Option\r\n");
				len = 0;
				break;
		}
	}
	return flag;
}
// encapsulate PPP packet with HDLC control bytes
// len: PPP data + 2 HDLC control bytes + 2 protocol bytes + 2 checksum bytes
void PPP_Send(U8* packet, U16 len)
{
	U16 fcs = 0,i;
	U8 c;
	// perform CRC16 checksum on the packet 
	// (inlcudes 2 HDLC control bytes and 2 protocol bytes)
	fcs = FCS_Get(packet, len - 2);
	packet[len-2] = fcs & 0xFF;
	packet[len-1] = (fcs >> 8) & 0xFF;

//	PPP_Info("PPP: SEND:%dBytes:",len);
//	for(i = 0; i < len;i++)
//	{
//		PPP_Info("%02X",packet[i]);	
//	}
//	PPP_Info("\r\n");
	
	//while(RINGBUF_GetFill(commTxRingBuf));
	COMM_Putc(0x7E);
	while(len--) {
		if(*packet < 0x20){
			if(pppUseESC){
				COMM_Putc(0x7D);
				COMM_Putc(*packet ^ 0x20);
			}else{
				COMM_Putc(*packet);
			}
		}else{
			switch(*packet){
				case 0x7E:
					COMM_Putc(0x7D);
					COMM_Putc(0x5E);
					break;
	
				case 0x7D:
					COMM_Putc(0x7D);
					COMM_Putc(0x5D);
					break;
	
				default:
					COMM_Putc(*packet);
					break;
			}
		}
		packet++;
	}
	COMM_Putc(0x7E);
	
}

static int  avRandomized = 0;       /* Set when truely randomized. */
static u32_t avRandomSeed = 0;      /* Seed used for random number generation. */

u32_t sys_jiffies(void) {
    static u32_t jiffies = 0;
    jiffies += 1 + (SysTick_Get()/10);
    return jiffies;
}

void avRandomInit(void)
{
  avRandomSeed += sys_jiffies(); /* XXX */
  /* Initialize the Borland random number generator. */
  srand((unsigned)avRandomSeed);
}


void avRandomize(void)
{
  static u32_t last_jiffies;

  if (!avRandomized) {
    avRandomized = !0;
    avRandomInit();
    /* The initialization function also updates the seed. */
  } else {
    /* avRandomSeed += (avRandomSeed << 16) + TM1; */
    avRandomSeed += (sys_jiffies() - last_jiffies); /* XXX */
  }
  last_jiffies = sys_jiffies();
}

static uint32_t gen_rand(void)
{
  return ((((u32_t)rand() << 16) + rand()) + avRandomSeed);
}


static void PPP__SendConfigReq(void)
{	
	uint32_t rand;
	uint8_t *pt;
	switch(pppPhase){
		case PPP_PHASE_ESTABLISH:
			PPP_Info("PPP: Send LCP CONFIG_REQ\r\n");
			
			if(!pppDataPrepared){
				pppTxBuf[2] = 0xC0;
				pppTxBuf[3] = 0x21;
				pppTxBuf[6] = 0;
				pppTxBuf[7] = 0x0A;			// data size
				pppTxBuf[8] = 2;			// option 2, ACCM
				pppTxBuf[9] = 6;			// option size
				pppTxBuf[10] = 0x0;	
				pppTxBuf[11] = 0x0;
				pppTxBuf[12] = 0x0;
				pppTxBuf[13] = 0x0;
				/*avRandomize();
				
				rand = gen_rand();
				pt = (uint8_t *)&rand;
				pppTxBuf[2] = 0xC0;
				pppTxBuf[3] = 0x21;
				pppTxBuf[6] = 0;
				pppTxBuf[7] = 0x14;			// data size
				pppTxBuf[8] = 2;			// option 2, ACCM
				pppTxBuf[9] = 6;			// option size
				pppTxBuf[10] = 0x0;	
				pppTxBuf[11] = 0x0;
				pppTxBuf[12] = 0x0;
				pppTxBuf[13] = 0x0;
				
				pppTxBuf[14] = 0x05;
				pppTxBuf[15] = 0x06;
				pppTxBuf[16] = pt[0];
				pppTxBuf[17] = pt[1];
				pppTxBuf[18] = pt[2];
				pppTxBuf[19] = pt[3];
//				pppTxBuf[16] = 0x78;
//				pppTxBuf[17] = 0x12;
//				pppTxBuf[18] = 0xA0;
//				pppTxBuf[19] = 0xB0;
				pppTxBuf[20] = 0x07;
				pppTxBuf[21] = 0x02;
				pppTxBuf[22] = 0x08;
				pppTxBuf[23] = 0x02;*/
			}
			break;
		
		case PPP_PHASE_AUTHENTICATION:
			PPP_Info("PPP: Send PAP CONFIG_REQ, usr: %s, pwd: %s\r\n", pppUsr, pppPwd);
			if(pppApn_UseLoginPass)
			{
				if(!pppDataPrepared){
					pppTxBuf[2] = 0xC0;
					pppTxBuf[3] = 0x23;
					pppTxBuf[6] = 0;
					pppTxBuf[7] = strlen(pppUsr) + strlen(pppPwd) + 6;
					pppTxBuf[8] = strlen(pppUsr);
					
					memcpy(&pppTxBuf[9], pppUsr, strlen(pppUsr));
					pppTxBuf[9 + strlen(pppUsr)] = strlen(pppPwd);
					
					memcpy(&pppTxBuf[10 + strlen(pppUsr)], pppPwd, strlen(pppPwd));
				}
			}
			else
			{
				//while(1);
				PPP_Info("PPP: PAP Up\r\n");
				pppPhase = PPP_PHASE_NETWORK;
				pppMsgID = 0;
				PPP__ManageState(&ipcpState, PPP_EVENT_OPEN, PPP_LAYER_NETWORK);
				PPP__ManageState(&ipcpState, PPP_EVENT_UP, PPP_LAYER_NETWORK);
				//return;
			}
			break;
		
		case PPP_PHASE_NETWORK:
			PPP_Info("PPP: Send IPCP CONFIG_REQ\r\n");
			
			if(!pppDataPrepared){
				pppTxBuf[2] = 0x80;
				pppTxBuf[3] = 0x21;
				pppTxBuf[6] = 0;
				pppTxBuf[7] = 22;
				
				pppTxBuf[8] = 3;				// IP address
				pppTxBuf[9] = 0x06;
				pppTxBuf[10] = 0;pppTxBuf[11] = 0;pppTxBuf[12] = 0;pppTxBuf[13] = 0;
				
				pppTxBuf[14] = 0x81;			// Primary DNS server
				pppTxBuf[15] = 0x06;
				pppTxBuf[16] = 0;pppTxBuf[17] = 0;pppTxBuf[18] = 0;pppTxBuf[19] = 0;
				
				pppTxBuf[20] = 0x83;			// secondary DNS server
				pppTxBuf[21] = 0x06;
				pppTxBuf[22] = 0;pppTxBuf[23] = 0;pppTxBuf[24] = 0;pppTxBuf[25] = 0;
			}
			
			
			break;
		
		default:
			break;
	}
	
	pppTxBuf[0] = 0xFF;
	pppTxBuf[1] = 0x03;
	pppTxBuf[4] = CONFIG_REQ;
	pppTxBuf[5] = ++pppMsgID;
	pppDataPrepared = 0;
	pppRestartCount --;
	if(pppRestartCount == 0)
		pppErrCnt++;
	pppTick = SysTick_Get();
	pppTimerOn = 1;
	
	PPP_Send(pppTxBuf, pppTxBuf[7]+6);
}
static void PPP__SendConfigAck(void)
{
	PPP_Info("PPP: Send CONFIG_ACK\r\n");
	
	memcpy(pppTxBuf, pppRxBuf, sizeof pppTxBuf);
	pppTxBuf[4] = CONFIG_ACK;
	PPP_Send(pppTxBuf, pppTxBuf[7]+6);
}
static void PPP__SendConfigNackRej(void)
{
	PPP_Info("PPP: Send CONFIG_NACK/REJ\r\n");
	
	PPP_Send(pppTxBuf, pppTxBuf[7]+6);
}
static void PPP__SendTermReq(void)
{
	pppTick = SysTick_Get();
	pppTimerOn = 1;
	
	PPP_Info("PPP: Send LCP TERM_REQ, not implemented\r\n");
}
static void PPP__SendTermAck(void)
{
	PPP_Info("PPP: Send TERM_ACK\r\n");
	
	memcpy(pppTxBuf, pppRxBuf, sizeof pppTxBuf);
	pppTxBuf[4] = TERM_ACK;
	PPP_Send(pppTxBuf, pppTxBuf[7]+6);
}
static void PPP__SendCodeRej(void)
{
	PPP_Info("PPP: Send CODE_REJ, not implemented\r\n");
}
static void PPP__SendEchoReply(void)
{
	PPP_Info("PPP: Send ECHO_REPLY\r\n");
	
	memcpy(pppTxBuf, pppRxBuf, sizeof pppTxBuf);
	pppTxBuf[4] = ECHO_REPLY;
	PPP_Send(pppTxBuf, pppTxBuf[7]+6);
}
static void PPP__ThisLayerUp(PPP_LAYER layer)
{
	U8 i = 0;
	switch(layer){
		case PPP_LAYER_PHYSICAL:
			PPP_Info("PPP: LCP Up\r\n");
			pppPhase = PPP_PHASE_AUTHENTICATION;
			if(pppUseESC_flag)
			{
				pppUseESC_flag = 0;
				pppUseESC = 0;				// turn off ESC sequence if remote server support
			}
			pppMsgID = 0;
			PPP__ManageState(&papState, PPP_EVENT_OPEN, PPP_LAYER_AUTHENTICATION);
			PPP__ManageState(&papState, PPP_EVENT_UP, PPP_LAYER_AUTHENTICATION);
			break;
								  
		case PPP_LAYER_AUTHENTICATION:
			PPP_Info("PPP: PAP Up\r\n");
			pppPhase = PPP_PHASE_NETWORK;
			pppMsgID = 0;
			PPP__ManageState(&ipcpState, PPP_EVENT_OPEN, PPP_LAYER_NETWORK);
			PPP__ManageState(&ipcpState, PPP_EVENT_UP, PPP_LAYER_NETWORK);
			break;
			
		case PPP_LAYER_NETWORK:
			PPP_Info("PPP: IPCP Up\r\n");
			
			pppRxBuf[7] -= 4;					
			
			while(pppRxBuf[7] > 0){						// find local IP address
				if(pppRxBuf[8+i]==0x03){
                                    pppIpAddr[0] = pppRxBuf[10+i];
                                    pppIpAddr[1] = pppRxBuf[11+i];
                                    pppIpAddr[2] = pppRxBuf[12+i];
                                    pppIpAddr[3] = pppRxBuf[13+i];
                                    
                                    UIP_SetHostAddr();
                                    
                                    PPP_Info("PPP: Local IP: %u.%u.%u.%u\r\n", pppRxBuf[10+i], pppRxBuf[11+i], pppRxBuf[12+i], pppRxBuf[13+i]);
				}
				if(pppRxBuf[8+i]==0x81){
                                    PPP_Info("PPP: PRI DNS: %u.%u.%u.%u\r\n", pppRxBuf[10+i], pppRxBuf[11+i], pppRxBuf[12+i], pppRxBuf[13+i]);
                                    uip_ipaddr(&priDnsServerIp, pppRxBuf[10+i], pppRxBuf[11+i], pppRxBuf[12+i], pppRxBuf[13+i]);
				}
				if(pppRxBuf[8+i]==0x83){
                                    PPP_Info("PPP: SEC DNS: %u.%u.%u.%u\r\n", pppRxBuf[10+i], pppRxBuf[11+i], pppRxBuf[12+i], pppRxBuf[13+i]);
                                    uip_ipaddr(&secDnsServerIp, pppRxBuf[10+i], pppRxBuf[11+i], pppRxBuf[12+i], pppRxBuf[13+i]);
				}
				
				pppRxBuf[7] -= 6;
				i += 6;
			}
			
			pppDialAttempts = 0;				// link establish sucessfully, clear the dial counter
			
			pppPhase = PPP_PHASE_IP;
			
			break;
			
		default:
			break;
	}
}
static void PPP__ThisLayerDown(PPP_LAYER layer)
{
	switch(layer){
		case PPP_LAYER_PHYSICAL:
			PPP_Info("PPP: LCP Down\r\n");
			PPP__ManageState(&papState, PPP_EVENT_DOWN, PPP_LAYER_AUTHENTICATION);
			pppPhase = PPP_PHASE_DEAD;
			break;
			
		case PPP_LAYER_AUTHENTICATION:
			PPP_Info("PPP: PAP Down\r\n");
			PPP__ManageState(&ipcpState, PPP_EVENT_DOWN, PPP_LAYER_NETWORK);
			pppPhase = PPP_PHASE_DEAD;
			break;
			
		case PPP_LAYER_NETWORK:
			PPP_Info("PPP: IPCP Down\r\n");
			pppPhase = PPP_PHASE_DEAD;
			break;
			
		default:
			break;
	}
}
static void PPP__ThisLayerStarted(PPP_LAYER layer)
{
	switch(layer){
		case PPP_LAYER_PHYSICAL:
			PPP_Info("PPP: LCP Started\r\n");
			break;
			
		case PPP_LAYER_AUTHENTICATION:
			PPP_Info("PPP: PAP Started\r\n");
			break;
			
		case PPP_LAYER_NETWORK:
			PPP_Info("PPP: IPCP Started\r\n");
			break;
			
		default:
			break;
	}
}
static void PPP__ThisLayerFinished(PPP_LAYER layer)
{
	switch(layer){
		case PPP_LAYER_PHYSICAL:
			PPP_Info("PPP: LCP Finished\r\n");
			PPP__ManageState(&papState, PPP_EVENT_DOWN, PPP_LAYER_AUTHENTICATION);
			pppPhase = PPP_PHASE_DEAD;
			break;
			
		case PPP_LAYER_AUTHENTICATION:
			PPP_Info("PPP: PAP Finished\r\n");
			PPP__ManageState(&ipcpState, PPP_EVENT_DOWN, PPP_LAYER_NETWORK);
			pppPhase = PPP_PHASE_DEAD;
			break;
			
		case PPP_LAYER_NETWORK:
			PPP_Info("PPP: IPCP Finished\r\n");
			pppPhase = PPP_PHASE_DEAD;
			break;
			
		default:
			break;
	}
}
// manage PPP state transition
static void PPP__ManageState(PPP_STATE *state, PPP_EVENT event, PPP_LAYER layer)
{
	switch(*state){
		case PPP_STATE_INITIAL:
			pppTimerOn = 0;
			switch(event){
				case PPP_EVENT_UP:
					*state = PPP_STATE_CLOSED;
					break;
					
				case PPP_EVENT_OPEN:
					*state = PPP_STATE_STARTING;
					PPP__ThisLayerStarted(layer);
					break;
					
				case PPP_EVENT_CLOSE:
					break;
					
				case PPP_EVENT_DOWN:
				case PPP_EVENT_TO_PLUS:
				case PPP_EVENT_TO_MINUS:
				case PPP_EVENT_RCR_PLUS:
				case PPP_EVENT_RCR_MINUS:
				case PPP_EVENT_RCA:
				case PPP_EVENT_RCN:
				case PPP_EVENT_RTR:
				case PPP_EVENT_RTA:
				case PPP_EVENT_RUC:
				case PPP_EVENT_RXJ_PLUS:
				case PPP_EVENT_RXJ_MINUS:
				case PPP_EVENT_RXR:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
			}
			break;
			
		case PPP_STATE_STARTING:
			pppTimerOn = 0;
			switch(event){
				case PPP_EVENT_UP:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Configure-Request
					PPP__SendConfigReq();
//					COMM_Putc(0x7E);
//					COMM_Putc(0xFF);
//					COMM_Putc(0x7D);
//					COMM_Putc(0x23);
//					COMM_Putc(0xC0);
//					COMM_Putc(0x21);
//					COMM_Putc(0x7E);
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_OPEN:
					break;
					
				case PPP_EVENT_CLOSE:
					// This-Layer-Finished
					PPP__ThisLayerFinished(layer);
					*state = PPP_STATE_INITIAL;
					break;
					
				case PPP_EVENT_DOWN:
				case PPP_EVENT_TO_PLUS:
				case PPP_EVENT_TO_MINUS:
				case PPP_EVENT_RCR_PLUS:
				case PPP_EVENT_RCR_MINUS:
				case PPP_EVENT_RCA:
				case PPP_EVENT_RCN:
				case PPP_EVENT_RTR:
				case PPP_EVENT_RTA:
				case PPP_EVENT_RUC:
				case PPP_EVENT_RXJ_PLUS:
				case PPP_EVENT_RXJ_MINUS:
				case PPP_EVENT_RXR:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
			}
			break;
			
		case PPP_STATE_CLOSED:
			pppTimerOn = 0;
			switch(event){
				
				case PPP_EVENT_DOWN:
					*state = PPP_STATE_INITIAL;
					break;
					
				case PPP_EVENT_OPEN:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Configure-Request
					PPP__SendConfigReq();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_CLOSE:
					break;
					
				case PPP_EVENT_RCR_PLUS:
				case PPP_EVENT_RCR_MINUS:
				case PPP_EVENT_RCA:
				case PPP_EVENT_RCN:
				case PPP_EVENT_RTR:
				case PPP_EVENT_RXJ_PLUS:
				case PPP_EVENT_RXJ_MINUS:
					// Send-Terminate-Ack
					PPP__SendTermAck();
					break;
					
				case PPP_EVENT_RUC:
					// Send-Code-Reject
					PPP__SendCodeRej();
					break;
				
				case PPP_EVENT_RTA:
				case PPP_EVENT_RXR:
					break;
					
				case PPP_EVENT_UP:
				case PPP_EVENT_TO_PLUS:
				case PPP_EVENT_TO_MINUS:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
			}
			break;
			
		case PPP_STATE_STOPPED:
			pppTimerOn = 0;
			switch(event){
				case PPP_EVENT_DOWN:
					*state = PPP_STATE_STARTING;
					PPP__ThisLayerStarted(layer);
					break;
					
				case PPP_EVENT_OPEN:
					break;
					
				case PPP_EVENT_CLOSE:
					*state = PPP_STATE_CLOSED;
					break;
					
				case PPP_EVENT_RCR_PLUS:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Configure-Request
					PPP__SendConfigReq();
					// Send-Configure-Ack
					PPP__SendConfigAck();
					*state = PPP_STATE_ACK_SENT;
					break;
					
				case PPP_EVENT_RCR_MINUS:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Configure-Request
					PPP__SendConfigReq();
					// Send-Configure-Nak/Rej
					PPP__SendConfigNackRej();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_RCA:
				case PPP_EVENT_RCN:
				case PPP_EVENT_RTR:
					// Send-Terminate-Ack
					PPP__SendTermAck();
					break;
					
				case PPP_EVENT_RTA:
					break;
					
				case PPP_EVENT_RUC:
					// Send-Code-Reject
					PPP__SendCodeRej();
					break;
					
				case PPP_EVENT_RXJ_PLUS:
					break;
					
				case PPP_EVENT_RXJ_MINUS:
					// This-Layer-Finished
					PPP__ThisLayerFinished(layer);
					break;
					
				case PPP_EVENT_RXR:
					break;
				
				case PPP_EVENT_UP:
				case PPP_EVENT_TO_PLUS:
				case PPP_EVENT_TO_MINUS:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
			}
			break;
			
		case PPP_STATE_CLOSING:
			switch(event){
				case PPP_EVENT_UP:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
					
				case PPP_EVENT_DOWN:
					*state = PPP_STATE_INITIAL;
					break;
					
				case PPP_EVENT_OPEN:
					*state = PPP_STATE_STOPPING;
					break;
					
				case PPP_EVENT_CLOSE:
					break;
					
				case PPP_EVENT_TO_PLUS:
					// Send-Terminate-Request
					PPP__SendTermReq();
					break;
					
				case PPP_EVENT_TO_MINUS:
				case PPP_EVENT_RXJ_MINUS:
				case PPP_EVENT_RTA:
					// This-Layer-Finished
					PPP__ThisLayerFinished(layer);
					*state = PPP_STATE_CLOSED;
					break;
					
				case PPP_EVENT_RCR_PLUS:
				case PPP_EVENT_RCR_MINUS:
				case PPP_EVENT_RCA:
				case PPP_EVENT_RCN:
				case PPP_EVENT_RXJ_PLUS:
				case PPP_EVENT_RXR:
					break;
					
				case PPP_EVENT_RTR:
					// Send-Terminate-Ack
					PPP__SendTermAck();
					break;
					
				case PPP_EVENT_RUC:
					// Send-Code-Reject
					PPP__SendCodeRej();
					break;
			}
			break;
			
		case PPP_STATE_STOPPING:
			switch(event){
				case PPP_EVENT_UP:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
					
				case PPP_EVENT_DOWN:
					*state = PPP_STATE_STARTING;
					break;
					
				case PPP_EVENT_OPEN:
					*state = PPP_STATE_STOPPING;
					break;
					
				case PPP_EVENT_CLOSE:
					*state = PPP_STATE_CLOSING;
					break;
					
				case PPP_EVENT_TO_PLUS:
					// Send-Terminate-Request
					PPP__SendTermReq();
					break;
					
				case PPP_EVENT_TO_MINUS:
				case PPP_EVENT_RTA:
				case PPP_EVENT_RXJ_MINUS:
					// This-Layer-Finished
					PPP__ThisLayerFinished(layer);
					*state = PPP_STATE_STOPPED;
					break;
					
				case PPP_EVENT_RCR_PLUS:
				case PPP_EVENT_RCR_MINUS:
				case PPP_EVENT_RCA:
				case PPP_EVENT_RCN:
				case PPP_EVENT_RXR:
				case PPP_EVENT_RXJ_PLUS:
					break;
					
				case PPP_EVENT_RTR:
					// Send-Terminate-Ack
					PPP__SendTermAck();
					break;
					
				case PPP_EVENT_RUC:
					// Send-Code-Reject
					PPP__SendCodeRej();
					break;

			}
			break;
			
		case PPP_STATE_REQ_SENT:
			switch(event){
				case PPP_EVENT_UP:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
					
				case PPP_EVENT_DOWN:
					*state = PPP_STATE_STARTING;
					break;
					
				case PPP_EVENT_OPEN:
				case PPP_EVENT_RXR:
				case PPP_EVENT_RTA:
				case PPP_EVENT_RXJ_PLUS:
					break;
					
				case PPP_EVENT_CLOSE:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Terminate-Request
					PPP__SendTermReq();
					*state = PPP_STATE_CLOSING;
					break;
					
				case PPP_EVENT_TO_PLUS:
					// Send-Configure-Request
					PPP__SendConfigReq();
					break;
					
				case PPP_EVENT_TO_MINUS:
					// This-Layer-Finished
					PPP__ThisLayerFinished(layer);
					*state = PPP_STATE_STOPPED;
					break;
					
				case PPP_EVENT_RCR_PLUS:
					// Send-Configure-Ack
					PPP__SendConfigAck();
					*state = PPP_STATE_ACK_SENT;
					break;
					
				case PPP_EVENT_RCR_MINUS:
					// Send-Configure-Nak/Rej
					PPP__SendConfigNackRej();
					break;
					
				case PPP_EVENT_RCA:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					*state = PPP_STATE_ACK_RCVD;
					break;
					
				case PPP_EVENT_RCN:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Configure-Request
					PPP__SendConfigReq();
					break;
					
				case PPP_EVENT_RTR:
					// Send-Terminate-Ack
					PPP__SendTermAck();
					break;
					
				case PPP_EVENT_RUC:
					// Send-Code-Reject
					PPP__SendCodeRej();
					break;
							
				case PPP_EVENT_RXJ_MINUS:
					// This-Layer-Finished
					PPP__ThisLayerFinished(layer);
					*state = PPP_STATE_STOPPED;
					break;

			}
			break;
		case PPP_STATE_ACK_RCVD:
			switch(event){
				case PPP_EVENT_UP:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
					
				case PPP_EVENT_DOWN:
					*state = PPP_STATE_STARTING;
					break;
					
				case PPP_EVENT_OPEN:
					break;
					
				case PPP_EVENT_CLOSE:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Terminate-Request
					PPP__SendTermReq();
					*state = PPP_STATE_CLOSING;
					break;
					
				case PPP_EVENT_TO_PLUS:
					// Send-Configure-Request
					PPP__SendConfigReq();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_TO_MINUS:
				case PPP_EVENT_RXJ_MINUS:
					// This-Layer-Finished
					PPP__ThisLayerFinished(layer);
					*state = PPP_STATE_STOPPED;
					break;
					
				case PPP_EVENT_RCR_PLUS:
					// Send-Configure-Ack
					PPP__SendConfigAck();
					// This-Layer-Up
					PPP__ThisLayerUp(layer);
					*state = PPP_STATE_OPENED;
					break;
					
				case PPP_EVENT_RCR_MINUS:
					// Send-Configure-Nak/Rej
					PPP__SendConfigNackRej();
					break;
					
				case PPP_EVENT_RCA:
				case PPP_EVENT_RCN:
					// Send-Configure-Request
					PPP__SendConfigReq();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_RTR:
					// Send-Terminate-Ack
					PPP__SendTermAck();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_RUC:
					// Send-Code-Reject
					// send here
					break;
					
				case PPP_EVENT_RTA:
				case PPP_EVENT_RXJ_PLUS:						
				case PPP_EVENT_RXR:
					break;
			}
			break;
			
		case PPP_STATE_ACK_SENT:
			switch(event){
				case PPP_EVENT_UP:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
					
				case PPP_EVENT_DOWN:
					*state = PPP_STATE_STARTING;
					break;
					
				case PPP_EVENT_OPEN:
					break;
					
				case PPP_EVENT_CLOSE:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Terminate-Request
					PPP__SendTermReq();
					*state = PPP_STATE_CLOSING;
					break;
					
				case PPP_EVENT_TO_PLUS:
					// Send-Configure-Request
					PPP__SendConfigReq();
					break;
					
				case PPP_EVENT_TO_MINUS:
				case PPP_EVENT_RXJ_MINUS:
					// This-Layer-Finished
					PPP__ThisLayerFinished(layer);
					*state = PPP_STATE_STOPPED;
					break;
					
				case PPP_EVENT_RCR_PLUS:
					// Send-Configure-Ack
					PPP__SendConfigAck();
					break;
					
				case PPP_EVENT_RCR_MINUS:
					// Send-Configure-Nak/Rej
					PPP__SendConfigNackRej();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_RCA:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// This-Layer-Up
					PPP__ThisLayerUp(layer);
					*state = PPP_STATE_OPENED;
					break;
					
				case PPP_EVENT_RCN:
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Configure-Request
					PPP__SendConfigReq();
					break;
					
				case PPP_EVENT_RTR:
					// Send-Terminate-Ack
					PPP__SendTermAck();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_RUC:
					// Send-Code-Reject
					PPP__SendCodeRej();
					break;
					
				case PPP_EVENT_RTA:
				case PPP_EVENT_RXJ_PLUS:
				case PPP_EVENT_RXR:
					break;
			}
			break;
			
		case PPP_STATE_OPENED:
			pppTimerOn = 0;
			switch(event){
				case PPP_EVENT_UP:
				case PPP_EVENT_TO_PLUS:
				case PPP_EVENT_TO_MINUS:
					PPP_Debug("\r\nIllegal event\r\n");
					break;
					
				case PPP_EVENT_DOWN:
					// This-Layer-Down
					PPP__ThisLayerDown(layer);
					*state = PPP_STATE_STARTING;
					break;
					
				case PPP_EVENT_OPEN:
					break;
					
				case PPP_EVENT_CLOSE:
					// This-Layer-Down
					PPP__ThisLayerDown(layer);
					// Initialize-Restart-Count
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Terminate-Request
					PPP__SendTermReq();
					*state = PPP_STATE_CLOSING;
					break;
					
				case PPP_EVENT_RCR_PLUS:
					// This-Layer-Down
					PPP__ThisLayerDown(layer);
					// Send-Configure-Request
					PPP__SendConfigReq();
					// Send-Configure-Ack
					PPP__SendConfigAck();
					// send here
					*state = PPP_STATE_ACK_SENT;
					break;
					
				case PPP_EVENT_RCR_MINUS:
					// This-Layer-Down
					PPP__ThisLayerDown(layer);
					// Send-Configure-Request
					PPP__SendConfigReq();
					// Send-Configure-Nak/Rej
					PPP__SendConfigNackRej();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_RCA:
				case PPP_EVENT_RCN:
					// This-Layer-Down
					PPP__ThisLayerDown(layer);
					// Send-Configure-Request
					PPP__SendConfigReq();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_RTR:
					// This-Layer-Down
					PPP__ThisLayerDown(layer);
					// Zero-Restart-Count
					pppRestartCount = 0;
					pppTimerOn = 1;
					pppTick = SysTick_Get();
					// Send-Terminate-Ack
					PPP__SendTermAck();
					*state = PPP_STATE_STOPPING;
					break;
					
				case PPP_EVENT_RTA:
					// This-Layer-Down
					PPP__ThisLayerDown(layer);
					// Send-Configure-Request
					PPP__SendConfigReq();
					*state = PPP_STATE_REQ_SENT;
					break;
					
				case PPP_EVENT_RUC:
					// Send-Code-Reject
					PPP__SendCodeRej();
					break;
					
				case PPP_EVENT_RXJ_PLUS:
					break;
					
				case PPP_EVENT_RXJ_MINUS:
					// This-Layer-Down
					PPP__ThisLayerDown(layer);
					pppRestartCount = PPP_MAX_RESTART;
					// Send-Terminate-Request
					PPP__SendTermReq();
					*state = PPP_STATE_STOPPING;
					break;
					
				case PPP_EVENT_RXR:
					// Send-Echo-Reply
					PPP__SendEchoReply();
					break;
			}
			break;
	}
}
