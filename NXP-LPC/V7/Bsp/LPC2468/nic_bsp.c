

#include <includes.h>
#include <netif/ethernetif.h>
#include "lwipopts.h"

#include "applib.h"
#include "emac.h"
#include "emac_def.h"
#include "dm9161.h"	
#include "dm9161_def.h"
#include "ioLPC24xx.H" 
#include <LPC24xx.H>

#define  DM9161AE_INIT_AUTO_NEG_RETRIES        3

#define  DM9161AE_OUI                   0x00606E
#define  DM9161AE_VNDR_MDL                  0x08

 
#define MAX_ADDR_LEN 6
struct rt_lpc24xx_eth
{
	/* inherit from ethernet device */
	struct eth_device parent;

	/* interface address info. */
	INT8U  dev_addr[MAX_ADDR_LEN];			/* hw address	*/
};

typedef struct {                        /* RX Descriptor struct              */
	INT32U Packet;
	INT32U Ctrl;
} RX_Desc;

typedef struct {                        /* RX Status struct                  */
	INT32U Info;
	INT32U HashCRC;
} RX_Stat;

typedef struct {                        /* TX Descriptor struct              */
	INT32U Packet;
	INT32U Ctrl;
} TX_Desc;

typedef struct {                        /* TX Status struct                  */
	INT32U Info;
} TX_Stat;

static            RX_Desc *rb_descriptors ;
static            TX_Desc *tb_descriptors ;

static  RX_Stat *Rx_Stat ; /* Must be 8-Byte alligned   */
static  TX_Stat *Tx_Stat ;
static  INT8U       *RxBufBaseAddr;
static  INT8U       *TxBufBaseAddr;

#define  EMAC_RX_DESC_BASE_ADDR                (EMAC_RAM_BASE_ADDR)
#define  EMAC_RX_STATUS_BASE_ADDR              (EMAC_RX_DESC_BASE_ADDR   + (NUM_RX_FRAG * sizeof(RX_Desc)))
#define  EMAC_TX_DESC_BASE_ADDR                (EMAC_RX_STATUS_BASE_ADDR + (NUM_RX_FRAG * sizeof(RX_Stat)))
#define  EMAC_TX_STATUS_BASE_ADDR              (EMAC_TX_DESC_BASE_ADDR   + (NUM_TX_FRAG * sizeof(TX_Desc)))
#define  EMAC_RX_BUFF_BASE_ADDR                (EMAC_TX_STATUS_BASE_ADDR + (NUM_TX_FRAG * sizeof(TX_Stat)))
#define  EMAC_TX_BUFF_BASE_ADDR                (EMAC_RX_BUFF_BASE_ADDR   + (NUM_RX_FRAG * ETH_FRAG_SIZE))
 
static struct rt_lpc24xx_eth lpc24xx_device;

//static struct rt_semaphore tx_sem;
static OS_EVENT * tx_sem;


//  function added to initialize Rx Descriptors
void rx_descr_init (void)
{
	/* Initialize Receive Descriptor and Status array. */
	unsigned int i = 0;

	rb_descriptors      =  (RX_Desc *)(EMAC_RX_DESC_BASE_ADDR);
 	Rx_Stat            =  (RX_Stat  *)(EMAC_RX_STATUS_BASE_ADDR);
 	RxBufBaseAddr       =  (INT8U *)(EMAC_RX_BUFF_BASE_ADDR);
 
	for (i = 0; i < NUM_RX_FRAG; i++) 
	{ 
		rb_descriptors[i].Packet  = (INT32U)(RxBufBaseAddr + (i * ETH_FRAG_SIZE));
		rb_descriptors[i].Ctrl    = RCTRL_INT | (ETH_FRAG_SIZE-1);
		Rx_Stat[i].Info    = 0;
		Rx_Stat[i].HashCRC = 0;
	}

	/* Set EMAC Receive Descriptor Registers. */
	MAC_RXDESCRIPTOR    = (INT32U)&rb_descriptors[0];
	MAC_RXSTATUS        = (INT32U)&Rx_Stat[0];
 
	MAC_RXDESCRIPTORNUM = NUM_RX_FRAG-1;

	/* Rx Descriptors Point to 0 */
	MAC_RXCONSUMEINDEX  = 0;
}


//  function added to initialize Tx Descriptors
void tx_descr_init (void) 
{
	unsigned int i;

	tb_descriptors      =  (TX_Desc *)(EMAC_TX_DESC_BASE_ADDR);
	Tx_Stat            =  (TX_Stat  *)(EMAC_TX_STATUS_BASE_ADDR);
	TxBufBaseAddr       =  (INT8U *)(EMAC_TX_BUFF_BASE_ADDR);

	for (i = 0; i < NUM_TX_FRAG; i++) 
	{
		tb_descriptors[i].Packet =  (INT32U)(TxBufBaseAddr + (i * ETH_FRAG_SIZE));
		tb_descriptors[i].Ctrl   = 0;
		Tx_Stat[i].Info   = 0;
	}

	/* Set EMAC Transmit Descriptor Registers. */
	MAC_TXDESCRIPTOR    = (INT32U)&tb_descriptors[0];
	MAC_TXSTATUS        = (INT32U)&Tx_Stat[0];
	MAC_TXDESCRIPTORNUM = NUM_TX_FRAG-1;

	/* Tx Descriptors Point to 0 */
	MAC_TXPRODUCEINDEX  = 0;
}
static  void  EMAC_RxPktDiscard ( )
{
	MAC_RXCONSUMEINDEX      = (MAC_RXCONSUMEINDEX + 1) % NUM_RX_FRAG;
}
/*********************************************************************************************************
** 函数名称: nic_isr_handler
** 函数名称: nic_isr_handler
**
** 功能描述：  interrupt service routine
**
** 输　入:  int vector
**          
** 输　出:   void
**         
** 全局变量:  
** 调用模块: 无
**
** 作　者:  LiJin
** 日　期:  2009年8月11日
** 备  注:  
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 备  注: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static void nic_isr_handler(   )
{
//    INT32U status   =  (MAC_INTSTATUS & MAC_INTENABLE);
    INT32U status               =  MAC_INTSTATUS;    
	INT16U  n_new;
	INT32U  nRsv =  MAC_RSV;
	INT32U  nSta =  MAC_STATUS;

    if (status & INT_RX_DONE) // if receive packet
    {
		INT8U result;
		MAC_INTCLEAR = status;
		/* a frame has been received */
		result = eth_device_ready(&(lpc24xx_device.parent));
		if (result != OS_TRUE)
		{//如果发送失败,那么说明队列满了,处理不过来,那么就把这封信扔掉
			EMAC_RxPktDiscard();
		}
 		MAC_INTCLEAR            = (INT_RX_DONE);                            /* Clear the interrupt flags        */
    }

	/* check for receive overrun */
	if (status & INT_RX_OVERRUN)
	{
		MAC_INTCLEAR = INT_RX_OVERRUN;
	//	eth->rx_overrun++;
		MAC_COMMAND |= CR_RX_RES;
		/* usleep(?); */
		MAC_COMMAND |= CR_RX_EN;
		MAC_MAC1 |= MAC1_REC_EN;
	}
	/* check for transmit underrun */
	if (status & INT_TX_UNDERRUN)
	{
		MAC_INTCLEAR = INT_TX_UNDERRUN;
	//	eth->tx_underrun++;
		MAC_COMMAND |= CR_TX_RES;;
		/* usleep(?); */
		MAC_COMMAND |= CR_TX_EN;
	}

	//???????????
	VICVectAddr = 0;            //interrupt close 通知中断控制器中断结束
}


INT8U   NetIF_MAC_Addr[NET_IF_ADDR_SIZE];      /* NIC's MAC addr.                                      */
/*********************************************************************************************************
** 函数名称: SetMacID
** 函数名称: SetMacID
**
** 功能描述：  设置芯片物理地址,物理地址已经存储在程序空间内 
**
** 输　入:  INT8U * mac_ptr
**          
** 输　出:   void
**         
** 全局变量:  
** 调用模块: 无
**
** 作　者:  LiJin
** 日　期:  2009年7月30日
** 备  注:  
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 备  注: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void set_mac_id( )   
{
	MAC_SA0   =  (NetIF_MAC_Addr[5] << 8) |(NetIF_MAC_Addr[4]);                   /* Write the MAC Address, octect 2 and 1 to the EMAC        */
	MAC_SA1   =  (NetIF_MAC_Addr[3] << 8) |(NetIF_MAC_Addr[2]);                   /* Write the MAC Address, octect 4 and 3 to the EMAC        */
	MAC_SA2  =  (NetIF_MAC_Addr[1] << 8) | (NetIF_MAC_Addr[0]);                  /* Write the MAC Address, octect 6 and 5 to the EMAC        */
 }

static  void  AppInitTCPIP (void)
{  
	NetIF_MAC_Addr[0] = 0x00;
	NetIF_MAC_Addr[1] = 0x38;
	NetIF_MAC_Addr[2] = 0x6c;
	NetIF_MAC_Addr[3] = 0xa2;
	NetIF_MAC_Addr[4] = 0x45;
	NetIF_MAC_Addr[5] = 0x3e;

	/* Update MAC address */
	lpc24xx_device.dev_addr[0] = 0x00;
	lpc24xx_device.dev_addr[1] = 0x38;
	lpc24xx_device.dev_addr[2] = 0x6c;
	lpc24xx_device.dev_addr[3] = 0xa2;
	lpc24xx_device.dev_addr[4] = 0x45;
	lpc24xx_device.dev_addr[5] = 0x3e;

// 	err             = Net_Init();                               /* Initialize uC/TCP-IP                                     */
// 
// 	AppNetIP        = NetASCII_Str_to_IP("10.10.1.129",  &err);
// 	AppNetMsk       = NetASCII_Str_to_IP("255.255.255.0", &err);
// 	AppNetGateway   = NetASCII_Str_to_IP("10.10.1.1",   &err);

//	err             = NetIP_CfgAddrThisHost(AppNetIP, AppNetMsk);
//	err             = NetIP_CfgAddrDfltGateway(AppNetGateway);
}
/*********************************************************************************************************
** 函数名称: phy_hw_init
** 函数名称: phy_hw_init
**
** 功能描述：  
**
** 输　入:  void
**          
** 输　出:   void
**         
** 全局变量:  
** 调用模块: 无
**
** 作　者:  LiJin
** 日　期:  2009年8月8日
** 备  注:  
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 备  注: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  phy_hw_init (void)
{ /* Configure I/O and the RMII / MII interface pins          */
  //	PINSEL2             =   0x50150105;	                                /* Selects P1[0,1,4,8,9,10,14,15]                           */
	//	PINSEL3             =   0x00000005;	                                /* Selects P1[17:16]                                        */
	PINSEL2             =   0x50150105;	   
	PINSEL3             =   0x00000005;	        
}
/*********************************************************************************************************
** 函数名称: nic_linkup
** 函数名称: nic_linkup
**
** 功能描述：  Message from NIC that the ethernet link is up.
**
** 输　入:  void
**          
** 输　出:   void
**         
** 全局变量:  
** 调用模块: 无
**
** 作　者:  LiJin
** 日　期:  2009年8月10日
** 备  注:  WARNING: Called in interruption context most of the time.
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 备  注: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  nic_linkup (void)
{
	INT32U  link_speed;
	INT32U  link_duplex;

	link_speed              =   get_phy_link_speed();                  /* Read the PHY's current link speed                    */
	link_duplex             =   get_phy_link_duplex();                 /* Read the PHY's current link duplex mode              */

	if (link_speed == NET_PHY_SPD_0 || link_duplex == NET_PHY_DUPLEX_UNKNOWN) 
	{   /* Inform the EMAC about the current PHY settings       */
		nic_link_change(NET_PHY_SPD_0, NET_PHY_DUPLEX_UNKNOWN);    
	}
	else
	{                /* Inform the EMAC about the current PHY settings       */
		nic_link_change(link_speed,    link_duplex);  
	}
}
/*********************************************************************************************************
** 函数名称: nic_linkdown
** 函数名称: nic_linkdown
**
** 功能描述：  Message from NIC that the ethernet link is down.
**
** 输　入:  void
**          
** 输　出:   void
**         
** 全局变量:  
** 调用模块: 无
**
** 作　者:  LiJin
** 日　期:  2009年8月10日
** 备  注:  WARNING: Called in interruption context most of the time.
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 备  注: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void  nic_linkdown (void)
{
	INT32U  link_speed;
	INT32U  link_duplex;

	link_speed              =   get_phy_link_speed();                  /* Read the PHY's current link speed                    */
	link_duplex             =   get_phy_link_duplex();                 /* Read the PHY's current link duplex mode              */

	if (link_speed == NET_PHY_SPD_0 || link_duplex == NET_PHY_DUPLEX_UNKNOWN)
	{    /* Inform the EMAC about the current PHY settings       */
		nic_link_change(NET_PHY_SPD_0, NET_PHY_DUPLEX_UNKNOWN);   
	} 
	else
	{  /* Inform the EMAC about the current PHY settings       */
		nic_link_change(link_speed,    link_duplex);                
	}
}

#define  MAIN_OSC_FRQ              11059200L
#define  IRC_OSC_FRQ               11059200L
#define  RTC_OSC_FRQ                  32768L


INT32U  bsp_cpu_clk_freq (void)
{
	INT32U  msel;
	INT32U  nsel;
	INT32U  fin;
	INT32U  pll_clk_feq;                                    /* When the PLL is enabled, this is Fcco                    */
	INT32U  clk_div;
	INT32U  clk_freq;

	switch (CLKSRCSEL & 0x03)
	{                                 /* Determine the current clock source                       */
		case 0:
			fin        =  IRC_OSC_FRQ;
			break;
		case 1:
			fin        =  MAIN_OSC_FRQ;
			break;
		case 2:
			fin        =  RTC_OSC_FRQ;
			break;
		default:
			fin        =  IRC_OSC_FRQ;
			break;
	}

	if ((PLLSTAT & (1 << 25)) > 0) 
	{                                                              /* If the PLL is currently enabled and connected        */
		msel        = (INT32U)(PLLSTAT & 0x3FFF) + 1;           /* Obtain the PLL multiplier                            */
		nsel        = (INT32U)((PLLSTAT >>   16) & 0x0F) + 1;   /* Obtain the PLL divider                               */
		pll_clk_feq = (2 * msel * fin / nsel);                      /* Compute the PLL output frequency                     */
	} 
	else
	{
		pll_clk_feq = (fin);                                        /* The PLL is bypassed                                  */
	}

	clk_div         = (INT8U)(CCLKCFG & 0x0F) + 1;             /* Obtain the CPU core clock divider                    */
	clk_freq        = (INT32U)(pll_clk_feq / clk_div);          /* Compute the ARM Core clock frequency                 */

	return (clk_freq);
}
 
void rt_delayms(INT32U ms)
{
	//执行11059200次语句，花时3.570S，差不多每ms计算3097条语句
	INT32U count=3339*ms;
	INT32U i;
	for (i=0;i<count;i++)
	{
		__asm{ NOP };
	}
}


void  nic_int_init  (void)
{
	VICIntSelect       &= ~(1 << VIC_ETHERNET);                         /* Configure the Ethernet VIC interrupt source for IRQ      */
	VICVectAddr21       =  (CPU_INT32U)nic_isr_handler;              /* Set the vector address                                   */
	VICIntEnable        =  (1 << VIC_ETHERNET);                         /* Enable the VIC interrupt source, but no local sources    */
}



/* ethernet device interface */
INT8U IsTxDescFull( )
{
	INT32U TxProduceIndex = MAC_TXPRODUCEINDEX;
	INT32U TxConsumeIndex = MAC_TXCONSUMEINDEX;	 
	
	if (TxConsumeIndex == (TxProduceIndex+1)%NUM_TX_FRAG)
	{	
		return TRUE;	
	}
	return FALSE;
}
INT8U IsTxDescEmpty( )
{
	INT32U TxProduceIndex = MAC_TXPRODUCEINDEX;
	INT32U TxConsumeIndex = MAC_TXCONSUMEINDEX;	 

	if (TxConsumeIndex == TxProduceIndex)
	{
		return OS_TRUE;
	}

	return OS_FALSE;
}
 
#define   MIN(x, y)   ((x)   >   (y)   ?   (y)   :   (x)) 
void lpc24xxether_write_frame( struct pbuf* p )
{
	struct pbuf* q;
	INT8U *pDescBuf = NULL;
	INT8U *pSrcBuf = NULL;
	INT16U nTxBufOffset = 0,nDescOffset = 0,nPduLen = 0,nSendLen = 0;

	INT32U TxProduceIndex = MAC_TXPRODUCEINDEX;
	INT32U TxConsumeIndex = MAC_TXCONSUMEINDEX;	 

	if (p == NULL)
		return  ;
 
	pDescBuf = (INT8U *)tb_descriptors[TxProduceIndex].Packet;

	for (q = p; q != NULL; q = q->next)
	{
		nTxBufOffset = 0;
		if (q && q->len)
		{
			pSrcBuf = (INT8U*) q->payload;

			while( nTxBufOffset < q->len )
			{
				nPduLen = MIN(ETH_FRAG_SIZE-nDescOffset,q->len-nTxBufOffset);
				if (nPduLen)
				{
					memcpy(&pDescBuf[nDescOffset], &pSrcBuf[nTxBufOffset], nPduLen );
					nSendLen += nPduLen;

					if (nSendLen >= p->tot_len)
					{
						tb_descriptors[TxProduceIndex].Ctrl =  (EMAC_TX_DESC_OVERRIDE   |        /* Override the defaults from the MAC internal registers    */
							EMAC_TX_DESC_PAD        |        /* Add padding for frames < 64 bytes                        */
							EMAC_TX_DESC_LAST       |        /* No additional descriptors to follow, this is the last    */
							EMAC_TX_DESC_CRC)       |        /* Append the CRC automatically                             */
							(nPduLen + nDescOffset- 1);                       /* Write the size of the frame, starting from 0             */
					}
					else
					{
						tb_descriptors[TxProduceIndex].Ctrl =  (EMAC_TX_DESC_OVERRIDE   |        /* Override the defaults from the MAC internal registers    */
							EMAC_TX_DESC_PAD        |        /* Add padding for frames < 64 bytes                        */
							EMAC_TX_DESC_CRC)       |        /* Append the CRC automatically                             */
							(nPduLen + nDescOffset- 1);                       /* Write the size of the frame, starting from 0             */
					}

					nTxBufOffset += nPduLen;
					nDescOffset += nPduLen;
				
					//更新
					if (nDescOffset >= ETH_FRAG_SIZE )
					{
						TxProduceIndex ++;
						if (TxProduceIndex >= NUM_TX_FRAG)
						{
							TxProduceIndex -= NUM_TX_FRAG;
						}
						pDescBuf = (INT8U *)tb_descriptors[TxProduceIndex].Packet;
					}
					else if (nSendLen >= p->tot_len)
					{
						TxProduceIndex ++;
						if (TxProduceIndex >= NUM_TX_FRAG)
						{
							TxProduceIndex -= NUM_TX_FRAG;
						}
						break;
					}
				}
			}
		}							   
	}
	MAC_TXPRODUCEINDEX = TxProduceIndex;
}
/*
* Transmit packet.
*/ 
 
INT8U lpc24xxether_tx( eth_device_t dev, struct pbuf* p)
{
	struct pbuf* q;
	INT8U err = OS_NO_ERR;

	SetLedBlinking(1,1,0);
// 	err = IsTxDescEmpty();
// 	if (err == FALSE)
// 	{
// 		return 0;
// 	}
// 	while(!IsTxDescEmpty())
// 	{
// 		OSTimeDly(5);
// 	}

	/* lock tx operation */
//	rt_sem_take(&tx_sem, RT_WAITING_FOREVER);
	OSSemPend(tx_sem,0,&err);
	lpc24xxether_write_frame (p);
	OSSemPost(tx_sem);

	return 0;
}
/*
*********************************************************************************************************
*                                         NIC_RxGetNRdy()
*
* Description : Determines how many packets we are ready to be processed.
*
* Parameters  : None.
*
* Returns     : Number of NIC buffers that are ready to be processed by the stack.
*********************************************************************************************************
*/ 
static  INT16U  get_rx_pkt_cnt (void)
{
	INT16U     n_rdy;
	INT16U     rxconsumeix;
	INT16U     rxproduceix;

	rxconsumeix =   MAC_RXCONSUMEINDEX;
	rxproduceix =   MAC_RXPRODUCEINDEX;

	if (rxproduceix < rxconsumeix) 
	{ /* If the produce index has wrapped around                  */
		n_rdy   =   NUM_RX_FRAG - rxconsumeix + rxproduceix;
	}
	else
	{ /* If consumeix is < produceix, then no wrap around occured */
		n_rdy   =   rxproduceix - rxconsumeix;
	}

	return (n_rdy);
}

INT16U get_nic_rx_frame_size (void)
{
	INT16U     rxconsumeix;
	INT16U     rxproduceix;
	INT16U     desc_cnt=0 , i =0;
	INT16U     rx_frame_size = 0;
	INT32U   rxstatus;
	INT8U   flag = OS_FALSE;

	INT32U  nRsv =  MAC_RSV;
	INT32U  nSta =  MAC_STATUS;


	rxconsumeix =   MAC_RXCONSUMEINDEX;
	rxproduceix =   MAC_RXPRODUCEINDEX;
	if (rxconsumeix == rxproduceix )
	{
		return rx_frame_size;
	}

	if (rxproduceix < rxconsumeix) 
	{ /* If the produce index has wrapped around                  */
	 	desc_cnt   =   NUM_RX_FRAG - rxconsumeix + rxproduceix;
	}
	else
	{ /* If consumeix is < produceix, then no wrap around occured */
	 	desc_cnt   =   rxproduceix - rxconsumeix;	
	}

	for (i = rxconsumeix;  ; i++)
	{
		rxstatus   =  Rx_Stat[i].Info;   
		rxstatus       &= (RINFO_NO_DESCR    |                       /* Obtain the descriptor error bits                         */
			RINFO_OVERRUN    |
			RINFO_ALIGN_ERR  |
			RINFO_LEN_ERR    |
			RINFO_SYM_ERR    |
			RINFO_CRC_ERR    |
			RINFO_FAIL_FILT);

		if (rxstatus > 0)
	 		break;			 
	 	else
		{
			rx_frame_size  =  Rx_Stat[i].Info & RINFO_SIZE;
			rx_frame_size  -= 3;

			if (rxstatus | RINFO_LAST_FLAG)
			{//最后一封
				break;
			}				 
		}
		if (rxproduceix > rxconsumeix)
		{
			if (i >= rxproduceix) //到头了
				break;
		}
		else
		{
			if (i >= NUM_RX_FRAG-1)
			{
				i = 0;
				flag = OS_TRUE;
			}
			if (flag && i >= rxproduceix)
				break;		
		}					
	}
	return rx_frame_size;
}
INT16U get_nic_rx_pkt_size (void)
{
	INT16U   size;
	INT32U   rxstatus;

	rxstatus        =  Rx_Stat[MAC_RXCONSUMEINDEX].Info;             /* Accquire the status word for this desciptor              */

	rxstatus       &= (RINFO_NO_DESCR    |                       /* Obtain the descriptor error bits                         */
		RINFO_OVERRUN    |
		RINFO_ALIGN_ERR  |
		RINFO_LEN_ERR    |
		RINFO_SYM_ERR    |
		RINFO_CRC_ERR    |
		RINFO_FAIL_FILT);

	if (rxstatus > 0)
	{                      /* If any receive errors (except range error) occured       */
		size        =  0;  /* then return 0 so upper layer call NetNIC_RxPktDiscard()  */
	}
	else
	{
		size        =  Rx_Stat[MAC_RXCONSUMEINDEX].Info & RINFO_SIZE;     /* Obtain the fragment size from the status struct pointer  */
		size       -=  3;                                               /* +1 since the size is 0 based, -4 to ignore FCS           */
	}

	if (size < 0) 
	{    /* Ensure that the subtraction didnt cause an underflow     */
		size = 0;
	}

	return (size);  /* Return the size of the current frame  */
}


INT8U  NetIF_IsValidPktSize (INT16U  size)
{
	INT8U  valid;

	valid = DEF_YES;

	if (size  < NET_IF_FRAME_MIN_SIZE)
	{
		valid = DEF_NO;
	}

	if (size  > NET_IF_FRAME_MAX_CRC_SIZE) 
	{
		valid = DEF_NO;
	}

	return (valid);
}


static  CPU_INT32U        NIC_RxNRdyCtr = 0;
static  CPU_INT16U  NIC_RxGetNRdy (void)
{
	INT16U     n_rdy;
	INT16U     rxconsumeix;
	INT16U     rxproduceix;

	rxconsumeix =   MAC_RXCONSUMEINDEX;
	rxproduceix =   MAC_RXPRODUCEINDEX;

	if (rxproduceix < rxconsumeix)
	{                                    /* If the produce index has wrapped around                  */
		n_rdy   =   NUM_RX_FRAG - rxconsumeix + rxproduceix;
	} 
	else
	{                                                            /* If consumeix is < produceix, then no wrap around occured */
		n_rdy   =   rxproduceix - rxconsumeix;
	}

	return (n_rdy);
}
/*********************************************************************************************************
** 函数名称: lpc24xxether_read_frame
** 函数名称: lpc24xxether_read_frame
**
** 功能描述：  从描述符中读取数据
**
** 输　入:  INT8U * ptr  数据的目的地
** 输　入:  INT32U section_length  目的地长度
** 输　入:  INT32U total      总长
**          
** 输　出:   void
**         
** 全局变量:  
** 调用模块: 无
**
** 作　者:  LiJin
** 日　期:  2009年8月13日
** 备  注:  
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 备  注: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void lpc24xxether_read_frame(INT8U* ptr, INT32U section_length, INT32U total)
{
	register INT32U buf_remain, section_remain;
	static INT32U section_read = 0, buf_offset = 0, frame_read = 0;
	static INT8U* src_ptr = RT_NULL;

	INT32U RxProduceIndex = MAC_RXPRODUCEINDEX;
	INT32U RxConsumeIndex = MAC_RXCONSUMEINDEX;	 

	src_ptr = (INT8U *)(rb_descriptors[RxConsumeIndex].Packet );
	/* Loop until we have obtained the required amount of data. */
	section_read = 0;//
	while( section_read < section_length )
	{
		buf_remain = (total - buf_offset);
		section_remain = section_length - section_read;

		if( section_remain > buf_remain )
		{//目标空间够大
			/* more data on section than buffer size */
			memcpy(&ptr[ section_read ], &src_ptr[buf_offset], buf_remain);
			section_read += buf_remain;
			frame_read += buf_remain;

			/* free buffer */
			//rb_descriptors[current_rb_index].Packet &= ~RxDESC_FLAG_OWNSHIP;
			Rx_Stat[MAC_RXCONSUMEINDEX].Info     = 0;                       //Clear status for debugging purposes                      */

			/* move to the next frame. */
			MAC_RXCONSUMEINDEX      = (MAC_RXCONSUMEINDEX + 1) % NUM_RX_FRAG;     

			/* Reset the variables for the new buffer. */
			src_ptr = (INT8U *)(rb_descriptors[MAC_RXCONSUMEINDEX].Packet );
			buf_offset = 0;
		}
		else
		{
			/* more data on buffer than section size */
			memcpy(&ptr[section_read], &src_ptr[buf_offset], section_remain);
			buf_offset += section_remain;
			section_read += section_remain;
			frame_read += section_remain;

			/* finish this read */
			if((frame_read >= total) || (buf_offset >= ETH_FRAG_SIZE))
			{
				/* free buffer */
				Rx_Stat[MAC_RXCONSUMEINDEX].Info     = 0;                       //Clear status for debugging purposes                      */

				/* move to the next frame. */
				MAC_RXCONSUMEINDEX      = (MAC_RXCONSUMEINDEX + 1) % NUM_RX_FRAG;     

				src_ptr = (INT8U*)(rb_descriptors[MAC_RXCONSUMEINDEX].Packet );
				buf_offset = 0;
			}
		}
	}
} 

struct pbuf *lpc24xxether_rx(eth_device_t dev)
{
	struct pbuf *p = RT_NULL;
 
	INT32U RxProduceIndex = MAC_RXPRODUCEINDEX;
	INT32U RxConsumeIndex = MAC_RXCONSUMEINDEX;	 

	struct pbuf* q;
	INT16U  pkt_len = 0;
	INT16U  pkt_cnt = 0;
	INT8U   ret = OS_FALSE;
	
	if (RxProduceIndex == RxConsumeIndex)
		return RT_NULL;
// 	while(RxProduceIndex == RxConsumeIndex  )
//  { //缓冲区为空，继续等待
// 		rt_thread_delay(5);
// 		RxProduceIndex = MAC_RXPRODUCEINDEX;
// 		RxConsumeIndex = MAC_RXCONSUMEINDEX;	 
// 	}

	SetLedBlinking(2,1,0);
	 
	pkt_len = get_nic_rx_frame_size();
	//判断一下 pkt_len 是否有效，如果无效，则丢弃
	ret = NetIF_IsValidPktSize(pkt_len);
	if (ret == OS_FALSE)
	{
		 EMAC_RxPktDiscard();
		 return NULL;
	 }
 
	//
	if (pkt_len)
	{
		p = pbuf_alloc(PBUF_RAW, pkt_len, PBUF_RAM);
		if(p != RT_NULL)
		{
			for(q = p; q != RT_NULL; q= q->next)
				lpc24xxether_read_frame(q->payload, q->len, pkt_len);
		}
		else
		{//如果内存申请不到，那么需要对描述符进行处理，扔掉部分数据包
			//rt_kprintf("no memory in pbuf\n");
			 EMAC_RxPktDiscard();
			 SetLedBlinking(7,5,0);
		}
	}
 
	return p;
}

void update_mac_address(struct rt_lpc24xx_eth* device)
{
	MAC_SA0 = (device->dev_addr[5] << 8) | (device->dev_addr[4]);
	MAC_SA1 = (device->dev_addr[3] << 8)  | (device->dev_addr[2]);
	MAC_SA2 = (device->dev_addr[1] << 8)  | (device->dev_addr[0]);
}


/*********************************************************************************************************
** 函数名称: lpc24xxether_register
** 函数名称: lpc24xxether_register
**
** 功能描述：  注册MAC设备
**
** 输　入:  char * name
**          
** 输　出:   int
**         
** 全局变量:  
** 调用模块: 无
**
** 作　者:  LiJin
** 日　期:  2009年7月30日
** 备  注:  
**-------------------------------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 备  注: 
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
int lpc24xxether_register(char *name)
{
 	INT8U result;	   
	/* init rt-thread device interface */
  	lpc24xx_device.parent.eth_rx			= lpc24xxether_rx;
  	lpc24xx_device.parent.eth_tx			= lpc24xxether_tx; 

	tx_sem = OSSemCreate(1);
//	rt_sem_init(&tx_sem, "emac", 1, RT_IPC_FLAG_FIFO);
	//与协议栈相关
	result = eth_device_init(& (lpc24xx_device.parent) , (char*)name);
	RT_ASSERT(result == OS_TRUE);

	lwip_sys_init( );

	//MAC PHY硬件的初始化
	lpc24xxether_init( & lpc24xx_device );
	return OS_TRUE;
}
INT8U SetHWAddr( INT8U *pAddr )
{
	if (pAddr)
	{
		memcpy(pAddr,lpc24xx_device.dev_addr, 6);
	}
}


void rt_hw_eth_init()
{
    AppInitTCPIP();
	lpc24xxether_register("E0");	 
}

