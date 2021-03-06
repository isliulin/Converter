#include "myconfig.h"
#define WAIT_CANMBOX_MAXTIME   50     //等待发送完成信号量超时时间设为50ms
#define CAN_SEND_MAXTIME       500
#define SYNC_DATA_MAXTIME      50     //同步数据最长时间为5s
extern OS_EVENT * can1Mbox;

u8 CAN_threshold;




CAN_DATA_FRAME *g_canDataFrame[MAX_CAN_CHANNEL];
CAN_DATA_FRAME *g_canDataAckFrame[MAX_CAN_CHANNEL];  //应答链表
CAN_DATA_FRAME *g_canDataActFrame[MAX_CAN_CHANNEL];  //主动链表

CAN_SEND_FRAME *g_can1SendFrame;  //CAN1发送主动帧链表
CAN_SEND_FRAME *g_can2SendFrame;  //CAN2发送主动帧链表
ReplyFrame can1CtlAckFrame,can1CtlResFrame,can1InfoAckFrame,can2InfoAckFrame;
/*typedef struct _CAN_BPR_INIT
{
	float bps;
	u8 tsjw;
	u8 tbs1;
	u8 tbs2;
	u16 brp;
}CAN_BPR_INIT;*/
CAN_BPR_INIT CANBprInit[MAX_BPS_SIZE] =
{
    {CAN_BPS_5K,1,CAN_BS1_13tq,CAN_BS2_2tq,525},
    {CAN_BPS_10K,1,CAN_BS1_6tq,CAN_BS2_1tq,525},
    {CAN_BPS_20K,1,CAN_BS1_5tq,CAN_BS2_1tq,300},
	  {CAN_BPS_333K,1,CAN_BS1_10tq,CAN_BS2_2tq,97},
    {CAN_BPS_50K,1,CAN_BS1_6tq,CAN_BS2_1tq,105},
    {CAN_BPS_125K,1,CAN_BS1_6tq,CAN_BS2_1tq,42},
    {CAN_BPS_250K,1,CAN_BS1_6tq,CAN_BS2_1tq,21},
    {CAN_BPS_500K,1,CAN_BS1_5tq,CAN_BS2_1tq,12},
    {CAN_BPS_1000K,1,CAN_BS1_15tq,CAN_BS2_5tq,2},
    {CAN_BPS_25K,1,CAN_BS1_6tq,CAN_BS2_1tq,210},
    {CAN_BPS_30K,1,CAN_BS1_6tq,CAN_BS2_1tq,175},
    {CAN_BPS_800K,1,CAN_BS1_2tq,CAN_BS2_1tq,13},
    {CAN_BPS_833K,1,CAN_BS1_6tq,CAN_BS2_1tq,63},
    {CAN_BPS_34K,1,CAN_BS1_3tq,CAN_BS2_1tq,247},//zijia  34

};
/****************************************************************
功能：CAN硬件初始化函数
入口参数：u8 brp：波特率
返回值：无
*****************************************************************/
void CAN_Hardware_Init(u8 brp)
{
    if(brp==0)
    {
        CAN_HardwareInit(CAN1_CHANNEL,CAN_BPS_50K,CAN_Mode_Normal,CAN1_RX0_INT_ENABLE);
        CAN_HardwareInit(CAN2_CHANNEL,CAN_BPS_50K,CAN_Mode_Normal,CAN2_RX0_INT_ENABLE);
    }
    else if(brp==1)
    {
        CAN_HardwareInit(CAN1_CHANNEL,CAN_BPS_20K,CAN_Mode_Normal,CAN1_RX0_INT_ENABLE);
        CAN_HardwareInit(CAN2_CHANNEL,CAN_BPS_20K,CAN_Mode_Normal,CAN2_RX0_INT_ENABLE);
    }
    else if(brp==2)
    {
        CAN_HardwareInit(CAN1_CHANNEL,CAN_BPS_10K,CAN_Mode_Normal,CAN1_RX0_INT_ENABLE);
        CAN_HardwareInit(CAN2_CHANNEL,CAN_BPS_10K,CAN_Mode_Normal,CAN2_RX0_INT_ENABLE);
    }
    else if(brp==3)
    {
        CAN_HardwareInit(CAN1_CHANNEL,CAN_BPS_5K,CAN_Mode_Normal,CAN1_RX0_INT_ENABLE);
        CAN_HardwareInit(CAN2_CHANNEL,CAN_BPS_5K,CAN_Mode_Normal,CAN2_RX0_INT_ENABLE);
    }
    else if(brp==4)
    {
        CAN_HardwareInit(CAN1_CHANNEL,CAN_BPS_125K,CAN_Mode_Normal,CAN1_RX0_INT_ENABLE);
        CAN_HardwareInit(CAN2_CHANNEL,CAN_BPS_125K,CAN_Mode_Normal,CAN2_RX0_INT_ENABLE);
    }

}

u8 CAN_Mode_Init(CAN_TypeDef *canChan,CAN_InitTypeDef *CAN_InitStructure,CAN_FilterInitTypeDef *CAN_FilterInitStructure,u8 rxIntType)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    CAN_Init(canChan, CAN_InitStructure);   // 初始化CAN1
    CAN_FilterInit(CAN1,CAN_FilterInitStructure);//滤波器初始化
    if (CAN1_RX0_INT_ENABLE	& rxIntType)
    {
        CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0消息挂号中断允许.
        NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;     // 主优先级为6
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

    }

    if (CAN1_RX1_INT_ENABLE	& rxIntType)
    {
        CAN_ITConfig(CAN1,CAN_IT_FMP1,ENABLE);//FIFO1消息挂号中断允许.
        NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;     // 主优先级为8
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
    if (CAN2_RX0_INT_ENABLE	& rxIntType)
    {
        CAN_ITConfig(CAN2,CAN_IT_FMP0,ENABLE);//FIFO0消息挂号中断允许.
        NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;     // 主优先级为7
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
    if (CAN2_RX1_INT_ENABLE	& rxIntType)
    {
        CAN_ITConfig(CAN2,CAN_IT_FMP1,ENABLE);//FIFO1消息挂号中断允许.
        NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;     // 主优先级为9
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    }
    return 0;
}
/****************************************************************
功能：CAN硬件初始化函数
入口参数：u8 canChan:CAN通道,float bps：波特率,u8 mode：工作模式,u8 rxIntType
返回值：无
*****************************************************************/
void CAN_HardwareInit(u8 canChan,float bps,u8 mode,u8 rxIntType)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_InitTypeDef  CAN_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    u8 i = 0;
    switch (canChan)
    {
    case CAN1_CHANNEL:
#if CAN1_USE_GPIOA_EN
        //使能相关时钟
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能PORTA时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟
        //初始化GPIO
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11| GPIO_Pin_12;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
        GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化PA11,PA12
        //引脚复用映射配置
        GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_CAN1); //GPIOA11复用为CAN1
        GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_CAN1); //GPIOA12复用为CAN1
#elif CAN1_USE_GPIOB_EN
        //使能相关时钟
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能PORTB时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟
        //初始化GPIO
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8| GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
        GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化PB8,PB9
        //引脚复用映射配置
        GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_CAN1); //GPIOB8复用为CAN1
        GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_CAN1); //GPIOB9复用为CAN1
#elif CAN1_USE_GPIOD_EN
        //使能相关时钟
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//使能PORTD时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟
        //初始化GPIO
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
        GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化PD0,PD1
        //引脚复用映射配置
        GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_CAN1); //GPIOD0复用为CAN1
        GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_CAN1); //GPIOD1复用为CAN1
#endif
        for (i = 0; i < MAX_BPS_SIZE; i++)
        {
            if (CANBprInit[i].bps == bps)
            {
                CAN_InitStructure.CAN_TTCM=DISABLE;	//非时间触发通信模式
                CAN_InitStructure.CAN_ABOM=ENABLE;	//软件自动离线管理
                CAN_InitStructure.CAN_AWUM=DISABLE;//睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
                CAN_InitStructure.CAN_NART=DISABLE;	//禁止报文自动传送
                CAN_InitStructure.CAN_RFLM=DISABLE;	//报文不锁定,新的覆盖旧的
                CAN_InitStructure.CAN_TXFP=DISABLE;	//优先级由报文标识符决定
                CAN_InitStructure.CAN_Mode = mode;	 //模式设置
                CAN_InitStructure.CAN_SJW = CANBprInit[i].tsjw;	//重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1tq~CAN_SJW_4tq
                CAN_InitStructure.CAN_BS1 = CANBprInit[i].tbs1; //Tbs1范围CAN_BS1_1tq ~CAN_BS1_16tq
                CAN_InitStructure.CAN_BS2 = CANBprInit[i].tbs2;//Tbs2范围CAN_BS2_1tq ~	CAN_BS2_8tq
                CAN_InitStructure.CAN_Prescaler = CANBprInit[i].brp;  //分频系数(Fdiv)为brp+1
                i = MAX_BPS_SIZE;
            }
        }
        //配置过滤器
        if (rxIntType & CAN1_RX0_INT_ENABLE)
        {
            CAN_FilterInitStructure.CAN_FilterNumber = 0;	  //过滤器0
            CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
            CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //32位
            CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;////32位ID
            CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
            CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;//32位MASK
            CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
            CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;//过滤器0关联到FIFO0
            CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //激活过滤器0
            CAN_Mode_Init(CAN1,&CAN_InitStructure,&CAN_FilterInitStructure,rxIntType);
        }
        if (rxIntType & CAN1_RX1_INT_ENABLE)
        {
            CAN_FilterInitStructure.CAN_FilterNumber = 1;	  //过滤器0
            CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
            CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //32位
            CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;////32位ID
            CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
            CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;//32位MASK
            CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
            CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;//过滤器0关联到FIFO0
            CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //激活过滤器0
            CAN_Mode_Init(CAN1,&CAN_InitStructure,&CAN_FilterInitStructure,rxIntType);
        }
        break;
    case CAN2_CHANNEL:
        //使能相关时钟
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能PORTB时钟
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);//使能CAN2时钟
        //初始化GPIO
#if CAN2_USE_GPIOB5_6_EN
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
#elif CAN2_USE_GPIOB12_13_EN
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
#endif
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
        GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化PB12,PB13或PB5、PB6
        //引脚复用映射配置
#if CAN2_USE_GPIOB5_6_EN
        GPIO_PinAFConfig(GPIOB,GPIO_PinSource5,GPIO_AF_CAN2); //GPIOB5复用为CAN2
        GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_CAN2); //GPIOB6复用为CAN2
#elif CAN2_USE_GPIOB12_13_EN
        GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_CAN2); //GPIOB12复用为CAN2
        GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_CAN2); //GPIOB13复用为CAN2
#endif
        for (i = 0; i < MAX_BPS_SIZE; i++)
        {
            if (CANBprInit[i].bps == bps)
            {
                CAN_InitStructure.CAN_TTCM=DISABLE;	//非时间触发通信模式
                CAN_InitStructure.CAN_ABOM=ENABLE;	//软件自动离线管理
                CAN_InitStructure.CAN_AWUM=DISABLE;//睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
                CAN_InitStructure.CAN_NART=DISABLE;	//禁止报文自动传送
                CAN_InitStructure.CAN_RFLM=DISABLE;	//报文不锁定,新的覆盖旧的
                CAN_InitStructure.CAN_TXFP=DISABLE;	//优先级由报文标识符决定
                CAN_InitStructure.CAN_Mode = mode;	 //模式设置
                CAN_InitStructure.CAN_SJW = CANBprInit[i].tsjw;	//重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1tq~CAN_SJW_4tq
                CAN_InitStructure.CAN_BS1 = CANBprInit[i].tbs1; //Tbs1范围CAN_BS1_1tq ~CAN_BS1_16tq
                CAN_InitStructure.CAN_BS2 = CANBprInit[i].tbs2;//Tbs2范围CAN_BS2_1tq ~	CAN_BS2_8tq
                CAN_InitStructure.CAN_Prescaler = CANBprInit[i].brp;  //分频系数(Fdiv)为brp+1
                i = MAX_BPS_SIZE;
            }
        }
        if (rxIntType & CAN2_RX0_INT_ENABLE)
        {
            CAN_FilterInitStructure.CAN_FilterNumber = 14;	  //过滤器2
            CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
            CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //32位
            CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;////32位ID
            CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
            CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;//32位MASK
            CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
            CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;//过滤器2关联到FIFO0
            CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //激活过滤器1
            CAN_Mode_Init(CAN2,&CAN_InitStructure,&CAN_FilterInitStructure,rxIntType);
        }
        if (rxIntType & CAN2_RX1_INT_ENABLE)
        {
            CAN_FilterInitStructure.CAN_FilterNumber = 3;	  //过滤器2
            CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
            CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //32位
            CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;////32位ID
            CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
            CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;//32位MASK
            CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
            CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;//过滤器2关联到FIFO0
            CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //激活过滤器1
            CAN_Mode_Init(CAN2,&CAN_InitStructure,&CAN_FilterInitStructure,rxIntType);
        }
        break;
    default:
        break;
    }
}

/*******************************************************************************
** 函数名称: CANSEND_Init
** 功能描述: can发送中断配置
** 参数说明: None
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-04-29
********************************************************************************/
void CANSEND_Init(void)
{
    NVIC_InitTypeDef   NVIC_InitStructure;

    NVIC_Init(&NVIC_InitStructure);
    CAN_ITConfig(CAN2,CAN_IT_TME,ENABLE);//发送邮箱空中断
    NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /*****************/
    CAN_ITConfig(CAN1,CAN_IT_TME,ENABLE);//发送邮箱空中断
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
/****************************************************************
功能：CAN通道过滤器数量配置函数
入口参数：u8 canChan：CAN通道
返回值：无;
*****************************************************************/
void CAN_FMR_Config(u8 canChan)
{
    CAN1->FMR |= FMR_INIT;  //过滤器工作在初始化模式
    if(canChan==CAN1_CHANNEL)
    {
        CAN1->FMR &= ~0xFF00;   //过滤器复位后配置清除
        CAN1->FMR |=  0x1A00;   //为CAN1口分配27个滤波器，CAN2口分配1个滤波器
    }
    else if(canChan==CAN2_CHANNEL)
    {
        CAN1->FMR &= ~0xFF00;   //过滤器复位后配置清除
        CAN1->FMR |= 0x0100;   //为CAN1口分配1个滤波器，CAN2口分配27个滤波器
    }
    CAN1->FMR &= ~FMR_INIT; //过滤器生效
}


/****************************************************************
功能：CAN软件初始化函数    wangkai:就是创建初始化各个链表的头结点
入口参数：无
返回值：无;
*****************************************************************/
void CAN_Software_Init()
{
    //初始化can1接受数据链表头结点
    g_canDataFrame[CAN1_CHANNEL] = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
    if (g_canDataFrame[CAN1_CHANNEL]!= NULL)
    {
        g_canDataFrame[CAN1_CHANNEL]->nextMsg = NULL;
    }
    //初始化can2接受数据链表头结点
    g_canDataFrame[CAN2_CHANNEL] = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
    if (g_canDataFrame[CAN2_CHANNEL]!= NULL)
    {
        g_canDataFrame[CAN2_CHANNEL]->nextMsg = NULL;
    }
    //初始化can1需要应答的数据链表头结点
    g_canDataAckFrame[CAN1_CHANNEL] = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
    if (g_canDataAckFrame[CAN1_CHANNEL]!= NULL)
    {
        g_canDataAckFrame[CAN1_CHANNEL]->nextMsg = NULL;
    }
    //初始化can2需要应答的数据链表头结点
    g_canDataAckFrame[CAN2_CHANNEL] = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
    if (g_canDataAckFrame[CAN2_CHANNEL]!= NULL)
    {
        g_canDataAckFrame[CAN2_CHANNEL]->nextMsg = NULL;
    }

    //初始化can1不需要应答的数据链表头结点
    g_canDataActFrame[CAN1_CHANNEL] = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
    if (g_canDataActFrame[CAN1_CHANNEL]!= NULL)
    {
        g_canDataActFrame[CAN1_CHANNEL]->nextMsg = NULL;
    }
    //初始化can2不需要应答的数据链表头结点
    g_canDataActFrame[CAN2_CHANNEL] = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
    if (g_canDataActFrame[CAN2_CHANNEL]!= NULL)
    {
        g_canDataActFrame[CAN2_CHANNEL]->nextMsg = NULL;
    }

    //CAN1主动帧发送链表初始化
    g_can1SendFrame = (CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));
    if (g_can1SendFrame!= NULL)
    {
        g_can1SendFrame->nextMsg = NULL;
    }
    //CAN2主动帧发送链表初始化
    g_can2SendFrame = (CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));
    if (g_can2SendFrame!= NULL)
    {
        g_can2SendFrame->nextMsg = NULL;
    }
}



/****************************************************************
功能：CAN_id动态增加函数
入口参数：u8 *id：id列表,u8 *cnt:id已有个数,temp:准备比较加入的新ID
返回值：无;
*****************************************************************/
void CAN_Update_ID(u8 *id,u8 *cnt,u8 temp)
{
    u8 i=0;
    if((*cnt)==0)
    {
        id[*cnt] = temp;
        (*cnt)++;
    }
    else
    {
        while(i<*cnt)
        {
            if(temp==id[i])
                break;
            else
                i++;
        }
        if(i==(*cnt))  //为新ID,就要在列表中更新
        {
            //防止地址越界处理
            if((*cnt)<ID_LIST_MAXSIZE)
            {
                id[*cnt] = temp;
                (*cnt)++;
            }
        }
    }
}

/****************************************************************
功能：CAN数据解析接口函数
入口参数：CAN_DATA_FRAME *frameBuf：解析后的数据缓存区指针,CanRxMsg *msg：消息缓存区指针
返回值：接收的数据长度
wangkai：就是将对应的*msg指定的消息复制到*framebuf结构体中，framebuf应该是链表申请的结点
*****************************************************************/
u8 CAN_DataAnalyze(CAN_DATA_FRAME *frameBuf,CanRxMsg *msg)
{

    switch (msg->IDE)
    {
    case CAN_ID_STD:
        frameBuf->IDE = msg->IDE;
        frameBuf->RTR = msg->RTR;
        frameBuf->dataLen = msg->DLC;
        frameBuf->id.canId = msg->StdId;
        memcpy(&frameBuf->canMsg,msg->Data,msg->DLC);
        memset(msg,0,sizeof(CanRxMsg));
        break;
    case CAN_ID_EXT:
        frameBuf->IDE = msg->IDE;
        frameBuf->RTR = msg->RTR;
        frameBuf->dataLen = msg->DLC;
        frameBuf->id.canId = msg->ExtId;/*************/
        memcpy(&frameBuf->canMsg,msg->Data,msg->DLC);//将msg数据的数据段复制到framebuf中
        memset(msg,0,sizeof(CanRxMsg));//此处为何要清零
        break;
    default:
        memset(msg,0,sizeof(CanRxMsg));
        break;
    }
    return frameBuf->RTR;
}




/****************************************************************
功能：CAN数据接收接口函数
入口参数：CAN_TypeDef *canChan：CAN通道号,u8 FIFONum-FIFO索引,CAN_DATA_FRAME *buf-数据接收缓存区指针

*****************************************************************/
u8 CAN_Receive_Msg(u8 canChan,u8 FIFONum,CAN_DATA_FRAME *buf)
{
    CanRxMsg RxMessage;
    switch (canChan)
    {
    case CAN1_CHANNEL:
        /*没有接收到数据,直接退出*/
        if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)
        {
            return 0;
        }
//		  g_markBit.isCAN1Recv = YES;
        CAN_Receive(CAN1,CAN_FIFO0, &RxMessage);//读取数据
        if (CAN_DataAnalyze(buf,&RxMessage) == 0)
        {
            memset(buf,0,sizeof(CAN_DATA_FRAME));
            return 0;
        }
        break;
    case CAN2_CHANNEL:
        /*没有接收到数据,直接退出*/
        if( CAN_MessagePending(CAN2,CAN_FIFO0)==0)
        {
					 //printf("错误数据\r\n");
            return 2;
        }
//			g_markBit.isCAN2Recv = YES;
        CAN_Receive(CAN2,CAN_FIFO0, &RxMessage);//读取数据
        if (RxMessage.RTR == CAN_RTR_Remote)//如果是远程帧//直接post信号量不再进行入队列
        {
//          OS_ENTER_CRITICAL();
//				
//          OS_EXIT_CRITICAL();
					  CAN_DataAnalyze(RTRbuf,&RxMessage);
					if(RTRbuf->id.idBit.recvDeviceId==ThisTransitionNumber)//只有发给本设备的应答帧才会计数信号量
					{
            OSSemPost(can2InfoAckSem);
//					  printf("收到一个应答包\r\n");
            memset(&RxMessage,0,sizeof(CAN_DATA_FRAME));//清空RxMessage
//            OS_ENTER_CRITICAL();
//						myfree(SRAMIN,RTRbuf);
//            OS_ENTER_CRITICAL();

					}
            return CAN_RTR_Remote;
        }
        else //如果是数据帧
        {
            CAN_DataAnalyze(buf,&RxMessage);
            return CAN_RTR_Data;
        }
    //wk：将接受到的数据放到buf所指向的申请好的节点中 在if中先执行后判断
    // break;
    default:
        break;
    }
    return buf->dataLen;
}

/****************************************************************
功能：CAN通信错误指示函数
入口参数：u8 canChan：CAN通道号
返回值：发送结果，0表示发送失败，1表示发送成功
*****************************************************************/
void CAN_ComErr_Display(u8 canChan)
{
    u8 rec = 0;        //硬件接收错误计数器
    u8 tec = 0;        //硬件发送错误计数器
    u8 lec = 0;        //上一次错误代码
    if(canChan==CAN1_CHANNEL)
    {
        rec = (CAN1->ESR&0xff000000)>>24;
        tec = (CAN1->ESR&0x00ff0000)>>16;
        lec = (CAN1->ESR&0x00000070)>>4;
        if((rec>CAN_threshold)||(tec>CAN_threshold)||((lec>0)&&(lec<7)))     //标准严格
        {
//				LED0 = 0;    //故障指示灯亮
        }
        else
        {
            //LED0 = 1;
        }
        CAN1->ESR |= 0x00000070; //错误类型手动置为7
    }
    else if(canChan==CAN2_CHANNEL)
    {
        rec = (CAN2->ESR&0xff000000)>>24;
        tec = (CAN2->ESR&0x00ff0000)>>16;
        lec = (CAN2->ESR&0x00000070)>>4;
        if((rec>CAN_threshold)||(tec>CAN_threshold)||((lec>0)&&(lec<7)))     //标准严格
        {
            //LED0 = 0;              //故障指示灯亮
        }
        else
        {
            //LED0 = 1;
        }
        CAN2->ESR |= 0x00000070; //错误类型手动置为7
    }
}


/****************************************************************
功能：CAN单帧发送函数
入口参数：u8 canChan：CAN通道号,CAN_DATA_FRAME *frameBuf：数据缓存区指针
返回值：发送结果：发送忙碌，发送成功，发送失败,总线关闭
*****************************************************************/
u8  CAN_Frame_Send(u8 canChan,CAN_DATA_FRAME *frame)
{
    static CAN_SEND_RESULT can1_send_state = SEND_IDLE;//can1_send_state是个枚举变量，先定义位空闲
    static CAN_SEND_RESULT can2_send_state = SEND_IDLE;
    OS_CPU_SR cpu_sr;
    u8 can1txok;
    u8 can2txok;
    u8 err = 0;
    if(canChan==CAN1_CHANNEL)
    {
        OS_ENTER_CRITICAL();
        if(can1_send_state== SEND_IDLE)   //判断CAN1发送资源是否空闲 空闲进入
        {
            //通信错误状态指示
            can1_send_state = SEND_BUSY;//wk:将状态为置位忙，防止其他的发送任务占用
            OS_EXIT_CRITICAL();
            CAN_ComErr_Display(canChan);//进行任务统计
            if(!(CAN1->ESR & CAN_BOFF_CHECK)) //总线未关闭  wk：后期需要了解ESR寄存器
            {
                CAN_Send_Msg(CAN1_CHANNEL,frame->id.canId,frame->IDE,&frame->canMsg,frame->dataLen);
//调用CAN发送函数，入口参数：u8 canChan：CAN通道号,u32 canId:消息ID,u8 frameType:消息ID版本，2.0A or 2.0B,u8* msg:数据缓存区指针,u8 len:数据长度
                can1txok = (u32)OSMboxPend(can1Mbox,WAIT_CANMBOX_MAXTIME,&err);  //等待发送完成邮箱
                if(can1txok==SEND_SUCCESS)			        //CAN1发送成功
                {
                    can1txok = 0;
                    can1_send_state = SEND_IDLE;	//wk:发送资源空闲
                    return SEND_SUCCESS;             //发送成功
                }
                else
                {
                    can1_send_state = SEND_IDLE; //wk：失败也要将资源空闲，防止总线空闲而不能发送
                    return SEND_FAILURE;             //发送失败
                }
            }
            else                                     //总线关闭  wk：else总是和最近的if配对
            {
                can1_send_state = SEND_IDLE;
                return SEND_BUS_CLOSED;
            }
        }
        OS_EXIT_CRITICAL();
    }

    //can2的处理过程同can1
    else if(canChan==CAN2_CHANNEL)
    {
        OS_ENTER_CRITICAL();
        if(can2_send_state== SEND_IDLE)   //判断CAN2发送资源是否空闲
        {
            can2_send_state = SEND_BUSY;
            OS_EXIT_CRITICAL();
            //通信错误状态指示
            CAN_ComErr_Display(canChan);
            if(!(CAN2->ESR & CAN_BOFF_CHECK))  //总线未关闭
            {
                CAN_Send_Msg(CAN2_CHANNEL,frame->id.canId,frame->IDE,&frame->canMsg,frame->dataLen);
                can2txok = (u32)OSMboxPend(can2Mbox,WAIT_CANMBOX_MAXTIME,&err); //等待发送完成邮箱
                /*此处的信号量在can发送中断中*/
                if(can2txok==SEND_SUCCESS)		       //CAN2发送成功
                {
                    can2txok = 0;
                    can2_send_state = SEND_IDLE;     //释放CAN2发送通道
                    return SEND_SUCCESS;		         //发送成功
                }
                else
                {
                    can2_send_state = SEND_IDLE;             //发送失败
                    return SEND_FAILURE;
                }
            }
            else                           //总线关闭
            {
                can2_send_state = SEND_IDLE;
                return SEND_BUS_CLOSED;
            }
        }
        OS_EXIT_CRITICAL();
    }
    return SEND_BUSY;
}



/*******************************************************************************
** 函数名称: CAN_RTRframe_Send
** 功能描述: CAN通道远程帧发送函数
** 参数说明: canChan: CAN通道
**			 id: CAN远程帧ID
**			 IDE: 消息标识符类型，STM32中标准帧该字段为0x0,扩展帧为0x4,定义有宏； CAN_Id_Standard CAN_Id_Extended
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-06-26
********************************************************************************/
u8  CAN_RTRframe_Send(u8 canChan,u32 id,u8 IDE)
{
    static CAN_SEND_RESULT can1_send_state = SEND_IDLE;//can1_send_state是个枚举变量，先定义位空闲
    static CAN_SEND_RESULT can2_send_state = SEND_IDLE;
    OS_CPU_SR cpu_sr;
    u8 can1txok;
    u8 can2txok;
    u8 err = 0;
    if(canChan==CAN1_CHANNEL)
    {
        OS_ENTER_CRITICAL();
        if(can1_send_state== SEND_IDLE)   //判断CAN1发送资源是否空闲 空闲进入
        {
            //通信错误状态指示
            can1_send_state = SEND_BUSY;//wk:将状态为置位忙，防止其他的发送任务占用
            OS_EXIT_CRITICAL();
            CAN_ComErr_Display(canChan);//进行任务统计
            if(!(CAN1->ESR & CAN_BOFF_CHECK)) //总线未关闭  wk：后期需要了解ESR寄存器
            {
                CAN_RTRsend_Msg(CAN1_CHANNEL,id,IDE);
//调用CAN发送函数，入口参数：u8 canChan：CAN通道号,u32 canId:消息ID,u8 frameType:消息ID版本，2.0A or 2.0B,u8* msg:数据缓存区指针,u8 len:数据长度
                can1txok = (u32)OSMboxPend(can1Mbox,WAIT_CANMBOX_MAXTIME,&err);  //等待发送完成邮箱
                if(can1txok==SEND_SUCCESS)			        //CAN1发送成功
                {
                    can1txok = 0;
                    can1_send_state = SEND_IDLE;	//wk:发送资源空闲
                    return SEND_SUCCESS;             //发送成功
                }
                else
                {
                    can1_send_state = SEND_IDLE; //wk：失败也要将资源空闲，防止总线空闲而不能发送
                    return SEND_FAILURE;             //发送失败
                }
            }
            else                                     //总线关闭  wk：else总是和最近的if配对
            {
                can1_send_state = SEND_IDLE;
                return SEND_BUS_CLOSED;
            }
        }
        OS_EXIT_CRITICAL();
    }

    //can2的处理过程同can1
    else if(canChan==CAN2_CHANNEL)
    {
        OS_ENTER_CRITICAL();
        if(can2_send_state== SEND_IDLE)   //判断CAN2发送资源是否空闲
        {
            can2_send_state = SEND_BUSY;
            OS_EXIT_CRITICAL();
            //通信错误状态指示
            CAN_ComErr_Display(canChan);
            if(!(CAN2->ESR & CAN_BOFF_CHECK))  //总线未关闭
            {
                CAN_RTRsend_Msg(CAN2_CHANNEL,id,IDE);
                can2txok = (u32)OSMboxPend(can2Mbox,WAIT_CANMBOX_MAXTIME,&err); //等待发送完成邮箱
                /*此处的信号量在can发送中断中*/
                if(can2txok==SEND_SUCCESS)		       //CAN2发送成功
                {
                    can2txok = 0;
                    can2_send_state = SEND_IDLE;     //释放CAN2发送通道
                    return SEND_SUCCESS;		         //发送成功
                }
                else
                {
                    can2_send_state = SEND_IDLE;             //发送失败
                    return SEND_FAILURE;
                }
            }
            else                           //总线关闭
            {
                can2_send_state = SEND_IDLE;
                return SEND_BUS_CLOSED;
            }
        }
        OS_EXIT_CRITICAL();
    }
    return SEND_BUSY;
}

/****************************************************************
功能：CAN数据处理函数,
入口参数：u8 canChan：CAN通道号,CAN_DATA_FRAME *frameBuf：数据缓存区指针wk：即保存的所有数据的链表指针
返回值：无

wk：
*****************************************************************/
void CAN_DataDeal(u8 canChan,CAN_DATA_FRAME *frameBuf)
{
    CAN_DATA_FRAME *deleteNode = NULL,*tempNode = NULL,*newMsgNode = NULL;
    OS_CPU_SR cpu_sr;
//	u8 sendId;
    u8 recvId;  //接收设备地址
    tempNode = frameBuf;//wk:用一个零时的指针指向头结点，当对这个零时结点进行改变时，不影响头结点
    switch(canChan)
    {
    case CAN1_CHANNEL:
    {
        while (tempNode->nextMsg != NULL)//wk：每次都是在头结点后的第一个结点，对他进行处理，处理后删除这个结点712--718
        {
            switch (tempNode->nextMsg->id.idBit.ackBit)//wk:应答标志位，是否需要应答
            {
            case FRAME_ACTIVE://wk：为主动帧//为需要应答的帧
            {
                OS_ENTER_CRITICAL();
                newMsgNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
                OS_EXIT_CRITICAL();
                if (newMsgNode != NULL)
                {
                    memcpy(newMsgNode,tempNode->nextMsg,sizeof(CAN_DATA_FRAME));

                    tempNode = g_canDataActFrame[CAN1_CHANNEL];/*wangkai：主动帧表头结点，在软件初始化中已经定义的全局变量*/
                    /*把tempnode赋值为主动帧头结点，对他进行遍历*/
                    while(tempNode->nextMsg != NULL)
                    {
                        tempNode = tempNode->nextMsg;
                    }
                    /*在主动帧链表的最后加上 在收到的数据帧链表中提取的主动帧结点*/
                    tempNode->nextMsg = newMsgNode;
                    newMsgNode->nextMsg = NULL;
                    OSSemPost(can1ActSem);
                }
                break;
            }

            /*应答帧同主动帧，也是先将所有的接受数据链表进行遍历找到其中的应答帧（根据应答标志位）*/
            /*再把应答帧链表进行遍历，到最后一个结点，之后将从遍历找到的应答帧结点接到应答帧链表的后面*/
            case FRAME_REPLY://为应答帧
            {
                OS_ENTER_CRITICAL();
                newMsgNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
                OS_EXIT_CRITICAL();
                if (newMsgNode != NULL)
                {
                    memcpy(newMsgNode,tempNode->nextMsg,sizeof(CAN_DATA_FRAME));
                    tempNode = g_canDataAckFrame[CAN1_CHANNEL];//用一个数组存放can1、2的主动帧头结点
                    while(tempNode->nextMsg != NULL)
                    {
                        tempNode = tempNode->nextMsg;
                    }
                    tempNode->nextMsg = newMsgNode;
                    newMsgNode->nextMsg = NULL;
                    OSSemPost(can1AckSem);
                }
                break;
            }
            default:
                break;
            }
            tempNode = frameBuf;//重新指向接受数据缓存头结点
            deleteNode = tempNode->nextMsg;		//把第一个一个结点删除
            tempNode->nextMsg = deleteNode->nextMsg;
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,deleteNode);//释放内存
            OS_EXIT_CRITICAL();
            //}/*wangkai：改动到此处？？？否则待处理链表的头结点之后一直是非空  就跑死了*/
        }
        break;

    }


    /*wk：can2同can1就是把自己的接受到的额数据链表中，每次分析第一个结点，看是主动帧还是应答帧，对应的放在各自的链表中，之后结点进行释放育删除*/
    case CAN2_CHANNEL:
    {
        while (tempNode->nextMsg != NULL)
        {
					/*如果此时是主从情况，并且这个时候帧的主索引不是这个设备类型，那说明不属于这个设备的帧，丢弃*/
					 if(tempNode->nextMsg->id.MasteridBit.MasterslaveBit==MasterSlave&&tempNode->nextMsg->id.MasteridBit.MainIndex!=EquipmentType&&tempNode->nextMsg->id.MasteridBit.MainIndex!=0)
					 {
						  //删除操作
                tempNode = frameBuf;
                deleteNode = tempNode->nextMsg;
                tempNode->nextMsg = deleteNode->nextMsg;
                OS_ENTER_CRITICAL();
                myfree(SRAMIN,deleteNode);
                OS_EXIT_CRITICAL();
					 }
					 else//其他
					 {
            recvId=tempNode->nextMsg->id.idBit.recvDeviceId;
            /*对id号进行判断，防止多解析出现*/
            if(recvId==ThisTransitionNumber||recvId==0xff)//为本机或者是广播
            {
                LED_BlinkTime.LED1_Times++;//让灯1闪烁一下
                switch (tempNode->nextMsg->id.idBit.ackBit)
                {
                case FRAME_ACTIVE:
                {
//                    if(tempNode->nextMsg->dataLen!=0)//添加对数据长度的判断，在主动帧中，不会出现长度为0的情况
//                    {
                    OS_ENTER_CRITICAL();
                    newMsgNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
                    OS_EXIT_CRITICAL();
                    if (newMsgNode != NULL)
                    {
                        memcpy(newMsgNode,tempNode->nextMsg,sizeof(CAN_DATA_FRAME));
                        tempNode = g_canDataActFrame[CAN2_CHANNEL];
                        while(tempNode->nextMsg != NULL)
                        {
                            tempNode = tempNode->nextMsg;
                        }
                        tempNode->nextMsg = newMsgNode;
                        newMsgNode->nextMsg = NULL;
                        OSSemPost(can2ActSem);
                    }
                    break;
//                    }
//                    else break;
                }
                case FRAME_REPLY:
                {
                    //发送信号量给任务
                    OS_ENTER_CRITICAL();
                    newMsgNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
                    OS_EXIT_CRITICAL();
                    if (newMsgNode != NULL)
                    {
                        memcpy(newMsgNode,tempNode->nextMsg,sizeof(CAN_DATA_FRAME));
                        tempNode = g_canDataAckFrame[CAN2_CHANNEL];
                        while(tempNode->nextMsg != NULL)
                        {
                            tempNode = tempNode->nextMsg;
                        }
                        tempNode->nextMsg = newMsgNode;
                        newMsgNode->nextMsg = NULL;
                        OSSemPost(can2AckSem);
                    }
                    break;
                }
                default:
                    break;
                }
                tempNode = frameBuf;
                deleteNode = tempNode->nextMsg;
                tempNode->nextMsg = deleteNode->nextMsg;
                OS_ENTER_CRITICAL();
                myfree(SRAMIN,deleteNode);
                OS_EXIT_CRITICAL();
            }
            else
            {
                //删除操作
                tempNode = frameBuf;
                deleteNode = tempNode->nextMsg;
                tempNode->nextMsg = deleteNode->nextMsg;
                OS_ENTER_CRITICAL();
                myfree(SRAMIN,deleteNode);
                OS_EXIT_CRITICAL();
            }
					}
        }
        break;
    }
    default:
        break;
    }
}

/*******************************************************************************
** 函数名称: CAN_Act_DataDeal
** 功能描述:
** 参数说明: canChan: [输入/出]
**			 frameBuf: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-06-26 这个函数中不可有等待等引起调度的功能
********************************************************************************/
void CAN_Act_DataDeal(u8 canChan,CAN_DATA_FRAME *frameBuf)
{
    CAN_DATA_FRAME *deleteNode = NULL,*tempNode = NULL;
//		INT8U oserr;
    u32 id;
//    u8 device_id;
    u8 sendId;
    u8 recvId;
    u8 index;
    OS_CPU_SR cpu_sr;
    tempNode = frameBuf;
    switch(canChan)
    {
    case CAN1_CHANNEL:
    {
        while(tempNode->nextMsg != NULL)
        {
            sendId = tempNode->nextMsg->id.idBit.recvDeviceId;
            recvId = tempNode->nextMsg->id.idBit.sendDeviceId;
            //device_id = tempNode->nextMsg->id.idBit.sendDeviceId;//获得发送设备ID，以后可做判断，此处无用
            index=tempNode->nextMsg->id.idBit.index;
            id = Get_CAN_ExId(true,sendId,recvId)|index;//交换发送地址与接收地址
            tempNode->nextMsg->id.idBit.ackBit = FRAME_REPLY;//应答标志位置1
            //OSTimeDlyHMSM(0,0,0,5);
            CAN_Send_Frame_App(canChan,id,CAN_Id_Extended,&tempNode->nextMsg->canMsg,sizeof(CAN_MSG),frameBuf,CAN_ACK_RETRY_TIME);//发送
            deleteNode = tempNode->nextMsg;
            tempNode->nextMsg = deleteNode->nextMsg;
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,deleteNode);
            OS_EXIT_CRITICAL();
        }
        break;
    }
    case CAN2_CHANNEL:
    {
        while(tempNode->nextMsg != NULL)
        {
            index=tempNode->nextMsg->id.idBit.index;
            if(tempNode->nextMsg->id.idBit.recvDeviceId==0xff)
            {
                Broadcast_Judegment(canChan,tempNode->nextMsg);//理论上到不了这里
            }
            else
            {
                if(tempNode->nextMsg->id.MasteridBit.MasterslaveBit==ManyMaster)//如果是多主的帧//也就是小车11111
                {
                    Index_Judegment(canChan,id,index,tempNode->nextMsg,frameBuf);
                }
                else//如果是主从//也就是域000000
                {
                    Master_Index_judegment(canChan,tempNode->nextMsg,frameBuf);
                }
            }
            deleteNode = tempNode->nextMsg;
            tempNode->nextMsg = deleteNode->nextMsg;
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,deleteNode);
            OS_EXIT_CRITICAL();
        }
        break;
    }
    default:
        break;
    }
}

/*******************************************************************************
** 函数名称: ACKSendFram
** 功能描述: 应答帧发送函数，针对多主和主从进行区别和优化
** 参数说明: canChan: [输入/出]
**			 frameBuf: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-14
********************************************************************************/
void ACKSendFram(u8 canChan,CAN_DATA_FRAME *tempNode)
{
    u32 id;
    //u8 device_id;
    u8 sendId;
    u8 recvId;
    u8 index;
	  u8 SendRes;
		u8 ReSendRes;
    u16 MSindex;
    /*当收到的帧为主从帧时*/
    if(tempNode->id.idBit.MasterslaveBit==ManyMaster)//是否是多主//多主为1//主从为0
    {
        index=tempNode->id.idBit.index;//索引是八位
        sendId = tempNode->id.idBit.recvDeviceId;//将发送设备号和接收设备号码对调
        recvId = tempNode->id.idBit.sendDeviceId;//将发送设备号和接收设备号码对调
        // device_(null)id = tempNode->nextMsg->id.idBit.sendDeviceId;
        id = Get_CAN_ExId(FRAME_REPLY,sendId,recvId)|index|(ManyMaster<<28);//多主标志置1//发送帧标志置1
        tempNode->id.idBit.ackBit = FRAME_REPLY;
       SendRes = CAN_RTRframe_Send(canChan,id,CAN_Id_Extended);//立刻回复一个远程帧，应答//通道号//id
			if(SendRes==SEND_SUCCESS)
			{
				//printf("<事务ID：%llu>调用应答帧发送函数成功\r\n",printfcount);
			}
			else if(SendRes==SEND_FAILURE)
			{
				ReSendRes = CAN_RTRframe_Send(canChan,id,CAN_Id_Extended);//立刻回复一个远程帧，应答//通道号//id
				//printf("<事务ID：%llu>（ERR）调用应答帧发送函数失败，重发代码%d\r\n",printfcount,ReSendRes);//成功是4，失败是3
			}
			else if(SendRes==SEND_BUS_CLOSED)
			{
				//printf("<事务ID：%llu>调用应答帧发送函数成功",printfcount);
			}
    }
    else if(tempNode->id.idBit.MasterslaveBit==MasterSlave)//主从模式下 0000
    {
        MSindex=(tempNode->id.idBit.index+(tempNode->id.idBit.sendDeviceId<<8));//ID
        //device_id=tempNode->id.idBit.recvDeviceId;
        id=(MasterSlave<<28)+(FRAME_REPLY<<26)+(ThisTransitionNumber<<16)+MSindex;//多主标志0//应答帧标志11//设备id//主子索引
        CAN_RTRframe_Send(canChan,id,CAN_Id_Extended);//立刻回复一个远程帧，应答//通道号//id
    }
    else return;

}



/*wangkai：对应答帧处理线程做了改动，仅仅发薕SSemPost(can2InfoAckSem)信号量并且删除应答帧任务链表的第一个结点*/
void CAN_Ack_DataDeal(u8 canChan,CAN_DATA_FRAME *frameBuf)//wk:framebuf因该是应答帧头指针
{
    CAN_DATA_FRAME *deleteNode = NULL,*tempNode = NULL;
    OS_CPU_SR cpu_sr;
    tempNode = frameBuf;

    switch(canChan)
    {
    case CAN1_CHANNEL:
    {
        while (tempNode->nextMsg != NULL)
        {
            OSSemPost(can1InfoAckSem);
            deleteNode = tempNode->nextMsg;
            tempNode->nextMsg = deleteNode->nextMsg;
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,deleteNode);
            OS_EXIT_CRITICAL();
        }
        break;
    }
    case CAN2_CHANNEL:
    {

        while (tempNode->nextMsg != NULL)
        {

//            if(tempNode->nextMsg->dataLen!=0)//说明是不是远程帧，是需要执行动作的主动帧
//            {
                if(tempNode->nextMsg->id.idBit.recvDeviceId==0xff)
                {
                    Broadcast_Judegment(canChan,tempNode->nextMsg);
                }
                else
                    StateQuery_Index_judegment(tempNode->nextMsg);//不需要应答的帧
//            }
            deleteNode = tempNode->nextMsg;
            tempNode->nextMsg = deleteNode->nextMsg;
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,deleteNode);
            OS_EXIT_CRITICAL();
        }
        break;
    }
    default:
        break;
    }

}



/****************************************************************
功能：CAN1接收中断服务函数  wk：接收的数据组成链表
入口参数：无
返回值：无;
*****************************************************************/
#if CAN1_RX0_INT_ENABLE	//使能RX0中断				    
void CAN1_RX0_IRQHandler(void)
{

    u8 datalen = 0;
    OS_CPU_SR cpu_sr;
    CAN_DATA_FRAME *tempMsgNode = NULL,*newMsgNode = NULL;
    OSIntEnter();
    OS_ENTER_CRITICAL();
    newMsgNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
    if (newMsgNode != NULL)
    {
        OS_EXIT_CRITICAL();
        datalen =  CAN_Receive_Msg(CAN1_CHANNEL, CAN1_FIFO0,newMsgNode);
        if (datalen > 0)
        {
            tempMsgNode = g_canDataFrame[CAN1_CHANNEL];
            /*tempmesnode指向接接受数据的头结点*/
            while (tempMsgNode->nextMsg != NULL)
            {
                tempMsgNode = tempMsgNode->nextMsg;
            }
            /*到链表的最后*/
            tempMsgNode->nextMsg = newMsgNode;
            newMsgNode->nextMsg = NULL;
            OSSemPost(can1RecvSem);

        }
        else
        {
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,newMsgNode);
            OS_EXIT_CRITICAL();
        }
    }
    else
    {
        OS_EXIT_CRITICAL();
    }
    OSIntExit();
}
#endif




/****************************************************************
功能：CAN2接收中断服务函数
入口参数：无
返回值：无;
*****************************************************************/
#if CAN2_RX0_INT_ENABLE	//使能RX0中断

void CAN2_RX0_IRQHandler(void)
{
    OS_CPU_SR cpu_sr;
    u8 framflage;
    CAN_DATA_FRAME *tempMsgNode = NULL,*newMsgNode = NULL;
    OSIntEnter();
    OS_ENTER_CRITICAL();
    newMsgNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//中断尽量不申请内存
    if (newMsgNode != NULL)
    {
        OS_EXIT_CRITICAL();
        framflage=CAN_Receive_Msg(CAN2_CHANNEL, CAN2_FIFO0,newMsgNode);//帧数据

        if(framflage==CAN_RTR_Data)//如果是数据帧
        {
            tempMsgNode = g_canDataFrame[CAN2_CHANNEL];
            while (tempMsgNode->nextMsg != NULL)
            {
                tempMsgNode = tempMsgNode->nextMsg;
            }
            tempMsgNode->nextMsg = newMsgNode;
            newMsgNode->nextMsg = NULL;
            OSSemPost(can2RecvSem);
        }
        else//如果不是数据帧，不用入队列
        {
            myfree(SRAMIN,newMsgNode);
        }
    }
    else
    {
        OS_EXIT_CRITICAL();
    }
    OSIntExit();
}

#endif



/****************************************************************
功能：CAN1发送中断服务函数
入口参数：无
返回值：无;
*****************************************************************/
void CAN1_TX_IRQHandler(void)
{
    u8 Tx1OK;
    u8 flag;
	  OSIntEnter();//2019年11月5日 13点04分  王凯：发送函数没有加中断层数统计
    if(SET == CAN_GetITStatus(CAN1,CAN_IT_TME))
    {
        flag = (CAN1->TSR & CAN_TSR_TXOK0)>>1;
        if(1==flag)//2019-07-11:修改，要将要同过邮箱发送的结果值进行转换下，原因是邮箱中不能发送0
        {
            Tx1OK=SEND_SUCCESS;//在枚举中是4
        }
        else
        {
            Tx1OK=SEND_FAILURE;//是3
        }
        OSMboxPost(can1Mbox,(void *)Tx1OK);
        CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
    }
    OSIntExit();//2019年11月5日 13点04分  王凯：发送终端没有中断层数统计

}


/****************************************************************
功能：CAN2发送中断服务函数
入口参数：无
返回值：无;
*****************************************************************/
void CAN2_TX_IRQHandler(void)
{
    u8 Tx2OK;
    u8 flag;
		OSIntEnter();/*2019年11月5日 13点04分  王凯：发送函数没有加中断层数统计！*/
    if(SET == CAN_GetITStatus(CAN2,CAN_IT_TME))
    {
        flag = (CAN2->TSR & CAN_TSR_TXOK0)>>1;
        if(1==flag)//2019-07-11:修改，要将要同过邮箱发送的结果值进行转换下，原因是邮箱中不能发送0
        {
            Tx2OK=SEND_SUCCESS;//在枚举中是4
        }
        else
        {
            Tx2OK=SEND_FAILURE;//是3
        }
        OSMboxPost(can2Mbox,(void *)Tx2OK);
        LED_BlinkTime.LED2_Times++;
        CAN_ClearITPendingBit(CAN2, CAN_IT_TME);

    }
	 OSIntExit();/*2019年11月5日 13点04分  王凯：发送终端没有中断层数统计！*/

}



/****************************************************************
功能：CAN数据帧发送接口函数
入口参数：u8 canChan：CAN通道号,u32 canId:消息ID,
u8 frameType:消息ID版本，2.0A or 2.0B,u8* msg:数据缓存区指针,u8 len:数据长度
返回值：0,成功; 其他,失败;
*****************************************************************/
u8 CAN_Send_Msg(u8 channel,u32 id,u8 frameType,CAN_MSG *msg,u8 len)
{
    u8 res=0;
    OS_CPU_SR cpu_sr;
    static CanTxMsg TxMessage;
    OS_ENTER_CRITICAL();
    TxMessage.IDE = frameType;      //扩展帧
    if(frameType==CAN_Id_Standard)  //id号
    {
        TxMessage.StdId = id;
    }
    else if(frameType==CAN_Id_Extended)
    {
        TxMessage.ExtId = id;
    }
    TxMessage.RTR = 0;		  // 消息类型为数据帧
    TxMessage.DLC = len;		//一帧8字节
    memcpy(TxMessage.Data,msg,len);
    OS_EXIT_CRITICAL();
    switch(channel)
    {
    case CAN1_CHANNEL:
    {
        res = CAN_Transmit(CAN1, &TxMessage);
//				g_markBit.isCAN1Send = YES;
    }
    break;
    case CAN2_CHANNEL:
    {
        res = CAN_Transmit(CAN2, &TxMessage);
//				g_markBit.isCAN2Send = YES;
    }
    break;
    default:
        break;
    }
    return res;
}



/*******************************************************************************
** 函数名称: CAN_RTRsend_Msg
** 功能描述: CAN通道远程帧发送接口函数
** 参数说明: channel: CAN通道号
**			 id: 帧ID
**			 frameType: 息标识符类型，CAN协议中标准帧为显性，扩展帧隐形，STM32中标准帧该字段为0x0,扩展帧为0x4,定义有宏
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-06-26
********************************************************************************/
u8 CAN_RTRsend_Msg(u8 channel,u32 id,u8 frameType)
{
    u8 res=0;
    OS_CPU_SR cpu_sr;
    static CanTxMsg TxMessage;
    OS_ENTER_CRITICAL();
    TxMessage.IDE = frameType;      //扩展帧
    if(frameType==CAN_Id_Standard)  //id号
    {
        TxMessage.StdId = id;
    }
    else if(frameType==CAN_Id_Extended)
    {
        TxMessage.ExtId = id;
    }
    TxMessage.RTR = CAN_RTR_REMOTE;		  // 消息类型为远程帧
    OS_EXIT_CRITICAL();
    switch(channel)
    {
    case CAN1_CHANNEL:
    {
        res = CAN_Transmit(CAN1, &TxMessage);
//				g_markBit.isCAN1Send = YES;
    }
    break;
    case CAN2_CHANNEL:
    {
        res = CAN_Transmit(CAN2, &TxMessage);
//				g_markBit.isCAN2Send = YES;
    }
    break;
    default:
        break;
    }
    return res;
}

/****************************************************************
功能：CAN通信错误统计函数
入口参数：u8 canChan：CAN通道号,u8:上一次错误类型
返回值：无
*****************************************************************/
void CAN_Err_Statistic(u8 channel,u8 lec)
{
//	if(channel==CAN1_CHANNEL)
//	{
//			if(lec>0&&lec<7)        //上一次有效错误类型范围为0到7
//			{
//				if(lec==ERR_FILL)
//				{
//					if(savedata.can1_err_cnt_fill<INT_MAX)
//					{
//							savedata.can1_err_cnt_fill++;
//					}
//				}
//				else if(lec==ERR_FORMAT)
//				{
//					if(savedata.can1_err_cnt_format<INT_MAX)
//					{
//							savedata.can1_err_cnt_format++;
//					}
//				}
//				else if(lec==ERR_ACK)
//				{
//					if(savedata.can1_err_cnt_ack<INT_MAX)
//					{
//							savedata.can1_err_cnt_ack++;
//					}
//				}
//				else if(lec==ERR_INVISIBLE)
//				{
//					if(savedata.can1_err_cnt_invisible<INT_MAX)
//					{
//							savedata.can1_err_cnt_invisible++;
//					}
//				}
//				else if(lec==ERR_VISIBLE)
//				{
//					if(savedata.can1_err_cnt_visible<INT_MAX)
//					{
//							savedata.can1_err_cnt_visible++;
//					}
//				}
//				else if(lec==ERR_CRC)
//				{
//					if(savedata.can1_err_cnt_crc<INT_MAX)
//					{
//							savedata.can1_err_cnt_crc++;
//					}
//
//				}
//			}
//	}
//	else if(channel==CAN2_CHANNEL)
//	{
//			if(lec>0&&lec<7)
//			{
//					if(lec==ERR_FILL)
//					{
//						if(savedata.can2_err_cnt_fill<INT_MAX)
//						{
//								savedata.can2_err_cnt_fill++;
//						}
//					}
//					else if(lec==ERR_FORMAT)
//					{
//						if(savedata.can2_err_cnt_format<INT_MAX)
//						{
//								savedata.can2_err_cnt_format++;
//						}
//					}
//					else if(lec==ERR_ACK)
//					{
//						if(savedata.can2_err_cnt_ack<INT_MAX)
//						{
//								savedata.can2_err_cnt_ack++;
//						}
//					}
//					else if(lec==ERR_INVISIBLE)
//					{
//						if(savedata.can2_err_cnt_invisible<INT_MAX)
//						{
//								savedata.can2_err_cnt_invisible++;
//						}
//					}
//					else if(lec==ERR_VISIBLE)
//					{
//						if(savedata.can2_err_cnt_visible<INT_MAX)
//						{
//								savedata.can2_err_cnt_visible++;
//						}
//					}
//					else if(lec==ERR_CRC)
//					{
//						if(savedata.can2_err_cnt_crc<INT_MAX)
//						{
//								savedata.can2_err_cnt_crc++;
//						}
//					}
//			}
//	}
//
}




/****************************************************************
功能：CAN错误门限加载函数
入口参数：u8 err_limit：错误门限
返回值：0,成功; 其他,失败;
*****************************************************************/
void Load_CAN_Threshold(u8 err_limit)
{
    if(err_limit==0)
    {
        CAN_threshold = CAN_ESR_MIN;
    }
    else if(err_limit==1)
    {
        CAN_threshold = CAN_ESR_MID;
    }
    else if(err_limit==2)
    {
        CAN_threshold = CAN_ESR_MAX;
    }
    //Modbus_Fill_CANErrCnt();
}



/****************************************************************
功能：CAN数据填充函数函数
入口参数：u32 canId:消息ID,u8 frameType:消息ID版本，2.0A or 2.0B,u8* msg:数据缓存区指针,u8 len:数据长度，CAN_DATA_FRAME *frameBuf:发送数据缓冲区
返回值：无
*****************************************************************/
void CAN_Fill_Msg(u32 id,u8 frameType,CAN_MSG *msg,u8 len,CAN_DATA_FRAME *frameBuf)
{
    frameBuf->id.canId = id;
    frameBuf->IDE = frameType;
    frameBuf->dataLen = len;
    memcpy(&frameBuf->canMsg,msg,sizeof(CAN_MSG));
}




///****************************************************************
//功能：获取CAN扩展帧地址
//入口参数：bool ackBit,u8 sendAddr,u8 recvAddr
//返回值：CAN扩展帧地址;
//*****************************************************************/
u32 Get_CAN_ExId(bool ackBit,u8 sendAddr,u8 recvAddr)
{
    u32 ExId;
    ExId = (ackBit<<26)|(recvAddr<<16)|(sendAddr<<8);
    return ExId;
}



void CAN_Fill_Data(u8 data0,u8 data1,u8 data2,u8 data3,u8 data4,u8 data5,u8 data6,u8 data7,CAN_MSG *msg)
{

    msg->dataBuf[0] = data2;
    msg->dataBuf[1] = data3;
    msg->dataBuf[2] = data4;
    msg->dataBuf[3] = data5;
    msg->dataBuf[4] = data6;
    msg->dataBuf[5] = data7;
}

//void CAN_Fill_Mission(uint8_t mainIndex,Mission task,CAN_MSG *msg)
//{
//往下分为任务正常信息和任务异常信息

//	Mission mission;
//	if(task.status<MISSION_SUSPEND || (task.status==MISSION_FINISH && task.faultCode==MISSION_ENABLED))
//	{
//		//任务就绪
//		//*(data+0)=mainIndex;
//		mission.status=task.status;
//		mission.sendSystemId=task.sendSystemId;
//		mission.recvSystemId=task.recvSystemId;
//		mission.sendDeviceId=task.sendDeviceId;
//		mission.speed=task.speed;
//		mission.recvDeviceId=task.recvDeviceId;
//		mission.prior=task.prior;
//		mission.carrierId=task.carrierId;
//		mission.mode=task.mode;
//	}
//	else if(task.status>=MISSION_SUSPEND && task.status<MISSION_FINISH)
//	{
//		//任务挂起
//		//*(data+0)=mainIndex;
//		mission.status=MISSION_SUSPEND+task.faultCode;
//		mission.sendSystemId=task.faultSystemId;
//		mission.recvSystemId=task.recvSystemId;
//		mission.sendDeviceId=task.faultDeviceId;
//		mission.speed=task.speed;
//		mission.recvDeviceId=task.recvDeviceId;
//		mission.prior=task.prior;
//		mission.carrierId=task.carrierId;
//		mission.mode=task.mode;
//	}
//	else if(task.status>=MISSION_FINISH)
//	{
//		//任务结束
//		//*(data+0)=mainIndex;
//		mission.status=MISSION_FINISH+task.faultCode;
//		mission.sendSystemId=task.faultSystemId;
//		mission.recvSystemId=task.recvSystemId;
//		mission.sendDeviceId=task.faultDeviceId;
//		mission.speed=task.speed;
//		mission.recvDeviceId=task.recvDeviceId;
//		mission.prior=task.prior;
//		mission.carrierId=task.carrierId;
//		mission.mode=task.mode;
//	}
//	msg->mainIndex = mainIndex;
//	memcpy(&msg->subIndex,&mission,7);
//}


/*wk：组包？*/
bool CAN_Send_Frame_App(u8 canChan,u32 id,u8 frameType,CAN_MSG *canMsg,u8 len,CAN_DATA_FRAME *frameBuf,u16 sendRetryTime)
{
    OS_CPU_SR cpu_sr;
    u16 cntCAN1 =0;
    u16 cntCAN2 =0;
    switch(canChan)
    {
    case CAN1_CHANNEL:
        OS_ENTER_CRITICAL();
        frameBuf = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
        OS_EXIT_CRITICAL();
        if (frameBuf!= NULL)
        {
            frameBuf->nextMsg = NULL;
            CAN_Fill_Msg(id,CAN_Id_Extended,canMsg,len,frameBuf);
            //2018.6.11 YQ修改
            //OSTimeDlyHMSM(0,0,0,5);//STM32可以不加
            while(CAN_Frame_Send(canChan,frameBuf)!=SEND_SUCCESS)//发送
            {
                OSTimeDlyHMSM(0,0,0,5);
                cntCAN1++;
                if(cntCAN1>=sendRetryTime)
                {
                    OS_ENTER_CRITICAL();
                    myfree(SRAMIN,frameBuf);
                    OS_EXIT_CRITICAL();
                    return false;                    //CAN总线有问题导致发送失败
                }
            }
            //发送成功
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,frameBuf);
            OS_EXIT_CRITICAL();
            return true;
        }

        break;
    case CAN2_CHANNEL:
        OS_ENTER_CRITICAL();
        frameBuf = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
        OS_EXIT_CRITICAL();
        if (frameBuf!= NULL)
        {
            frameBuf->nextMsg = NULL;
        }
        CAN_Fill_Msg(id,CAN_Id_Extended,canMsg,len,frameBuf);
        //OSTimeDlyHMSM(0,0,0,2);   //帧与帧之间增加2ms的间隔
        while(CAN_Frame_Send(canChan,frameBuf)!=SEND_SUCCESS)
        {
            OSTimeDlyHMSM(0,0,0,20);
            cntCAN2++;
            if(cntCAN2>=sendRetryTime)
            {
                OS_ENTER_CRITICAL();
                myfree(SRAMIN,frameBuf);
                OS_EXIT_CRITICAL();
                return false;
            }
        }
        //发送成功
        OS_ENTER_CRITICAL();
        myfree(SRAMIN,frameBuf);
        OS_EXIT_CRITICAL();
        return true;
    //break;
    default:
        return false;
        //break;
    }
    return false;
}



//void CAN_Send_Frame_App(u8 canChan,u32 id,u8 frameType,CAN_MSG *canMsg,u8 len,CAN_DATA_FRAME *frameBuf,u16 sendRetryTime)
//{
//		OS_CPU_SR cpu_sr;
//		u16 cntCAN1 =0;
//		u16 cntCAN2 =0;
//	  switch(canChan)
//		{
//					case CAN1_CHANNEL:
//						OS_ENTER_CRITICAL();
//						frameBuf = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
//						OS_EXIT_CRITICAL();
//						if (frameBuf!= NULL)
//						{
//							frameBuf->nextMsg = NULL;
//						}
//						CAN_Fill_Msg(id,CAN_Id_Extended,canMsg,sizeof(CAN_MSG),frameBuf);

//						while(CAN_Frame_Send(canChan,frameBuf)!=SEND_SUCCESS)
//						{
//							OSTimeDlyHMSM(0,0,0,2);
//							cntCAN1++;
//							if(cntCAN1>=sendRetryTime)
//							{
//								break;
//							}
//						}

//						//发送成功
//						OS_ENTER_CRITICAL();
//						myfree(SRAMIN,frameBuf);
//						OS_EXIT_CRITICAL();
//						break;
//					case CAN2_CHANNEL:
//						OS_ENTER_CRITICAL();
//						frameBuf = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
//						OS_EXIT_CRITICAL();
//						if (frameBuf!= NULL)
//						{
//							frameBuf->nextMsg = NULL;
//						}
//						//memset(&canMsg,0,sizeof(CAN_MSG));  //BUF内容都清零
//						CAN_Fill_Msg(id,CAN_Id_Extended,canMsg,sizeof(CAN_MSG),frameBuf);

//						while(CAN_Frame_Send(canChan,frameBuf)!=SEND_SUCCESS)
//						{
//							OSTimeDlyHMSM(0,0,0,2);
//							cntCAN2++;
//							if(cntCAN2>=sendRetryTime)
//							{
//								break;
//							}
//						}
//						//发送成功
//						OS_ENTER_CRITICAL();
//						myfree(SRAMIN,frameBuf);
//						OS_EXIT_CRITICAL();
//						break;
//					default:
//						break;
//			}
//}







///****************************************************************
//功能：CAN1心跳任务
//入口参数：无
//返回值：无;
//*****************************************************************/
void CAN1_HeartBeat_Task(void *pdata)
{
//		CAN_DATA_FRAME *frameBuf;
//		CAN_MSG canMsg;
//		u32 id;
//		u8 i;
//		pdata = pdata;
//		while(1)
//		{
//				for(i=0;i<DEVICE_NUM;i++)
//				{
//					if (i==DOMAIN_ID)                        continue;
//					if (channel1[i].commStatus<=COMM_CLOSED) continue;
    //定时心跳发送,为了减轻站点在同一时间的工作量，取消定时5S给站点发送心跳包，而是站点发送一个主动心跳包，域控制器回复应答心跳包
//					channel1[i].sendTimer++;
//					if (channel1[i].sendTimer>=50)       //5S发出给下面站点的心跳
//					{
//						channel1[i].sendTimer=0;
//						id = Get_CAN_ExId(true,DOMAIN_ID,i);
//						CAN_Fill_Data(0,0,0,0,0,0,0,0,&canMsg);
//						OSTimeDlyHMSM(0,0,0,5);
//						CAN_Send_Frame_App(CAN1_CHANNEL,id,CAN_Id_Extended,&canMsg,sizeof(CAN_MSG),frameBuf,CAN_HEARTBEAT_RETRY_TIME);
//					}

    //检测心跳间隔超时
//					channel1[i].recvTimer++;
//					if (channel1[i].recvTimer>60)       //检测下面站点的心跳超时为6s
//					{
//						HeartBeatTimeOut(CAN1_CHANNEL,&channel1[i]);
//					}
//					RayErr_Filter(sys->devices[i]);				//光感常亮滤波函数
//				}
//				//必须等到系统数据加载完成后再进行更新系统设备寄存器动作
//				UpdateModbusRegInfo();//更新MODBUS寄存器数值
//				OSTimeDlyHMSM(0,0,0,100);
//
//		}
}


/****************************************************************
功能：CAN2心跳任务
入口参数：无
返回值：无;
*****************************************************************/
void CAN2_HeartBeat_Task(void *pdata)
{
//		//static u8 syncTimer=0;
//		CAN_DATA_FRAME *frameBuf;
//		u32 id;
//		u8  i;
//		CAN_MSG canMsg;
//		pdata = pdata;
//		while(1)
//		{
//				for(i=0;i<SYSTEM_NUM;i++)
//				{
//					if (i==sys->systemId)                    continue;
//					if (channel2[i].commStatus<=COMM_CLOSED) continue;
//					//定时心跳发送
//					channel2[i].sendTimer++;
//					if (channel2[i].sendTimer>=50)
//					{
//						 channel2[i].sendTimer=0;
//						id = Get_CAN_ExId(true,sys->systemId,i);  //模拟出来
//						CAN_Fill_Data(0,0,0,0,0,0,0,0,&canMsg);
//						CAN_Send_Frame_App(CAN2_CHANNEL,id,CAN_Id_Extended,&canMsg,sizeof(CAN_MSG),frameBuf,CAN_HEARTBEAT_RETRY_TIME);
//					}
//
//					//心跳间隔超时
//					channel2[i].recvTimer++;
//					if (channel2[i].recvTimer>60)
//					{
//						HeartBeatTimeOut(CAN2_CHANNEL,&channel2[i]);
//					}
//
//					//同步计时
//					if(systems[i]->isDataSync)  //同步进行中
//					{
//						if(systems[i]->syncTimer<SYNC_DATA_MAXTIME)
//						{
//							systems[i]->syncTimer++;
//						}
//						else      //同步超时
//						{
//							systems[i]->syncTimer = 0;
//							systems[i]->isDataSync = 0;
//							//2018.11.4,这里需要修改，只要同步状态数据
//							Domain_Online(systems[i]);   //同步超时，要求对方域控制器重新同步数据,这里需要修改，可以只同步状态数据，不要同步同步任务数据
//						}
//					}
//					else
//					{
//						systems[i]->syncTimer = 0;
//					}
//
//					}
//					OSTimeDlyHMSM(0,0,0,100);
//		}
}

//将CAN主动帧压入队列
void CAN_Post_Queue(u8 canChan,CAN_SEND_FRAME *frame)
{
    OS_CPU_SR cpu_sr;
    CAN_SEND_FRAME *tempMsgNode = NULL,*newMsgNode = NULL;
    OS_ENTER_CRITICAL();
    newMsgNode = (CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));
    if (newMsgNode != NULL)
    {
        OS_EXIT_CRITICAL();
        memcpy(newMsgNode,frame,sizeof(CAN_SEND_FRAME));
        if(canChan==CAN1_CHANNEL)
        {
            tempMsgNode = g_can1SendFrame;
        }
        else if(canChan==CAN2_CHANNEL)
        {
            tempMsgNode = g_can2SendFrame;
        }
        while (tempMsgNode->nextMsg != NULL)
        {
            tempMsgNode = tempMsgNode->nextMsg;
        }
        /*遍历到链表的最后*/
        tempMsgNode->nextMsg = newMsgNode;//将fram指向的结点放在队列的最后
        newMsgNode->nextMsg = NULL;
        if(canChan==CAN1_CHANNEL)
        {
            OSSemPost(can1SendSem);
        }
        else if(canChan==CAN2_CHANNEL)
        {
            OSSemPost(can2SendSem);
        }
    }
    else
    {
        OS_EXIT_CRITICAL();
    }
}

//CAN1发送主动信息帧
void CAN1_Send_Frame(CAN_SEND_FRAME *frame)
{
    u8 err = 0;
    // u8 recvId;
    CAN_DATA_FRAME *frameBuf;
    bool sendRes;
    u8 cntRetry = 0;
    while(1)
    {
        sendRes = CAN_Send_Frame_App(CAN1_CHANNEL,frame->id,CAN_Id_Extended,&frame->canMsg,frame->len,frameBuf,CAN_INFO_RETRY_TIME);

        if(sendRes==false)
        {
            return;
        }
        //recvId = frame->id&0x000000ff;
//				SetReplyFrame(&can1InfoAckFrame,recvId,frame->canMsg.mainIndex,frame->canMsg.subIndex);
        OSSemPend(can1InfoAckSem,MAX_ACK_TIMEOUT,&err);
        //等待邮箱
        if(err==OS_ERR_NONE)
        {
            memset(&can1InfoAckFrame,0,sizeof(ReplyFrame));
            return;
        }
        else if(err==OS_ERR_TIMEOUT)
        {
            cntRetry++;
            //  frame->id=frame->id|0x02000000;//重发标志
            if(cntRetry>=MAX_RETRY_TIMES)
            {
                memset(&can1InfoAckFrame,0,sizeof(ReplyFrame));
                return;
            }
        }
        else
        {
            memset(&can1InfoAckFrame,0,sizeof(ReplyFrame));
            return;
        }
    }
}

//CAN2发送主动信息帧
void CAN2_Send_Frame(CAN_SEND_FRAME *frame)
{
    u8 err = 0;
    //u8 recvId;
    CAN_DATA_FRAME *frameBuf;
    bool sendRes;
    u8 cntRetry = 0;
    while(1)
    {
        //2018.6.22 YQ修改,为了防止发送不成功，将发送重试时间改为1000
        sendRes = CAN_Send_Frame_App(CAN2_CHANNEL,frame->id,CAN_Id_Extended,&frame->canMsg,frame->len,frameBuf,CAN2_INFO_RETRY_TIME);
        //这里将CAN2_Send_Frame的形参扩展CAN_Id_Extended帧类型
        if(sendRes==false)
        {
            return;
        }
        //recvId = frame->id&0x000000ff;
//				SetReplyFrame(&can2InfoAckFrame,recvId,frame->canMsg.mainIndex,frame->canMsg.subIndex);
        OSSemPend(can2InfoAckSem,MAX_ACK_TIMEOUT,&err); 				//等待应答信号量
        /*wangkai:为了方便调试，此处无限等待信号量*/
        //OSSemPend(can2InfoAckSem,0,&err);        //等待应答信号量

        if(err==OS_ERR_NONE)//如果对方及时回复
        {
            memset(&can2InfoAckFrame,0,sizeof(ReplyFrame));
            return;
        }
        else if(err==OS_ERR_TIMEOUT)//如果对方没有及时回复，重发3次
        {
            cntRetry++;
            //frame->id=frame->id|0x02000000;//重发标志

            if(cntRetry>=MAX_RETRY_TIMES)
            {
                memset(&can2InfoAckFrame,0,sizeof(ReplyFrame));
							 // printf("目标在重试后没有回复！\r\n");
                return;
            }
        }
        else
        {
            memset(&can2InfoAckFrame,0,sizeof(ReplyFrame));
            return;
        }
    }
}

u32 CAN_FramID_Change(CAN_DATA_FRAME *frameBuf)
{

    u32 id;
    // u8 device_id;
    u8 sendId;
    u8 recvId;
    u8 index;
    CAN_DATA_FRAME * tempNode;

    index=tempNode->id.idBit.index;
    sendId = tempNode->id.idBit.recvDeviceId;
    recvId = tempNode->id.idBit.sendDeviceId;
    // device_id = tempNode->id.idBit.sendDeviceId;
    id = Get_CAN_ExId(FRAME_REPLY,sendId,recvId)|index;
    tempNode->id.idBit.ackBit = FRAME_REPLY;
    return id;
}


/*******************************************************************************
** 函数名称: CAN2_Single_Send
** 功能描述: CAN单帧发送 分为多主和主从
** 参数说明: tempNode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-06-28
********************************************************************************/
void CAN2_Single_Send(CAN_DATA_FRAME * tempNode)
{
    INT32U id;
    bool sendRes;//can帧发送是否成功
    INT16U MSindex;
    //INT8U device_id;
    CAN_DATA_FRAME *frameBuf;
    /*在多主情况下时候*/
    if(tempNode->id.idBit.MasterslaveBit==ManyMaster)//如果数据帧是多主的发过来的
    {
        id=CAN_FramID_Change(tempNode)|(ManyMaster<<28);//
        CAN_Send_Frame_App(CAN2_CHANNEL,id,CAN_Id_Extended,&tempNode->canMsg,tempNode->dataLen,frameBuf,CAN_ACK_RETRY_TIME);
    }
    else if(tempNode->id.idBit.MasterslaveBit==MasterSlave)//如果是主从
    {
        HeartBeat.BeatTime=0;
        /*当有主从帧发送的时候，就把心跳帧计时清零*/

        MSindex=(tempNode->id.idBit.index+(tempNode->id.idBit.sendDeviceId<<8));//主从模式下的索引号
        //device_id=tempNode->id.idBit.recvDeviceId;//设备号就是本机的设备号，也就是接受到的帧的接收地址号
        id=(MasterSlave<<28)+(FRAME_REPLY<<26)+(ThisTransitionNumber<<16)+MSindex;//多主标志0//应答帧标志0//域id//主子索引
        sendRes = CAN_Send_Frame_App(CAN2_CHANNEL,id,CAN_Id_Extended,&tempNode->canMsg,tempNode->dataLen,frameBuf,CAN_ACK_RETRY_TIME);
//        if(sendRes!=true&&TransStatus.DeviceMode!=standAlone)//返回值不是真说明can出现了问题,如果是脱机状态，就不报离线了
//        {
//            TransStatus.DeviceMode=OffLine;//can帧发不出去，说明离线了
//        }
//        else
//        {
//            TransStatus.DeviceMode=1;
//        }

    }
    else return;
}

