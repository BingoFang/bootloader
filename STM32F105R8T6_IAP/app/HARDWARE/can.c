#include "main.h"

/* Private typedef -----------------------------------------------------------*/
typedef  struct {
  unsigned char   SJW;
  unsigned char   BS1;
  unsigned char   BS2;
  unsigned short  PreScale;
} tCAN_BaudRate;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
CanRxMsg CAN_RxMessage;
volatile uint8_t CAN_RxMsgFlag=0;//接收到CAN数据后的标志
volatile uint8_t TimeOutFlag;				///<定时器超时标志
tCAN_BaudRate  CAN_BaudRateInitTab[]= {      // CLK=36MHz
   {CAN_SJW_1tq,CAN_BS1_10tq,CAN_BS2_1tq,3},     // 1M
   {CAN_SJW_1tq,CAN_BS1_8tq,CAN_BS2_1tq,4},     // 900K
   {CAN_SJW_2tq,CAN_BS1_13tq,CAN_BS2_1tq,13},     // 800K
   {CAN_SJW_1tq,CAN_BS1_16tq,CAN_BS2_1tq,3},     // 666K
   {CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_1tq,4},     // 600K
   {CAN_SJW_1tq,CAN_BS1_16tq,CAN_BS2_1tq,4},     // 500K
   {CAN_SJW_1tq,CAN_BS1_16tq,CAN_BS2_1tq,5},     // 400K
   {CAN_SJW_1tq,CAN_BS1_15tq,CAN_BS2_1tq,7},    // 300K
   {CAN_SJW_1tq,CAN_BS1_16tq,CAN_BS2_1tq,8},    // 250K
   {CAN_SJW_1tq,CAN_BS1_14tq,CAN_BS2_1tq,10},	// 225K
   {CAN_SJW_1tq,CAN_BS1_16tq,CAN_BS2_1tq,10},    // 200K
   {CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_1tq,15},	// 160K
   {CAN_SJW_1tq,CAN_BS1_14tq,CAN_BS2_1tq,15},    // 150K
   {CAN_SJW_1tq,CAN_BS1_8tq,CAN_BS2_1tq,25},	// 144K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_1tq,36},   // 125K
   {CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_1tq,20},	// 120K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_1tq,45},    // 100K
   {CAN_SJW_1tq,CAN_BS1_14tq,CAN_BS2_1tq,25},   // 90K
   {CAN_SJW_1tq,CAN_BS1_16tq,CAN_BS2_1tq,25},   // 80K
   {CAN_SJW_1tq,CAN_BS1_14tq,CAN_BS2_1tq,30},	// 75K
   {CAN_SJW_1tq,CAN_BS1_13tq,CAN_BS2_1tq,40},    // 60K
   {CAN_SJW_1tq,CAN_BS1_14tq,CAN_BS2_1tq,45},    // 50K
   {CAN_SJW_1tq,CAN_BS1_16tq,CAN_BS2_1tq,50},    // 40K
   {CAN_SJW_1tq,CAN_BS1_14tq,CAN_BS2_1tq,75},   // 30K
   {CAN_SJW_1tq,CAN_BS1_16tq,CAN_BS2_1tq,100},   // 20K
};
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  通过波特率的值获取波特率参数表索引值
  * @param  BaudRate CAN总线波特率，单位为bps
  * @retval 波特率参数表索引值
  */
uint32_t CAN_GetBaudRateNum(uint32_t BaudRate)
{
    switch(BaudRate){
        case 1000000 :return 0;
        case 900000 :return 1;
        case 800000 :return 2;
        case 666000 :return 3;
        case 600000 :return 4;
        case 500000 :return 5;
        case 400000 :return 6;
        case 300000 :return 7;
        case 250000 :return 8;
        case 225000:return 9;
        case 200000 :return 10;
        case 160000:return 11;
        case 150000 :return 12;
        case 144000:return 13;
        case 125000 :return 14;
        case 120000:return 15;
        case 100000 :return 16;
        case 90000 :return 17;
        case 80000 :return 18;
        case 75000:return 19;
        case 60000 :return 20;
        case 50000 :return 21;
        case 40000 :return 22;
        case 30000 :return 23;
        case 20000 :return 24;
        default:return 0;
    }
}

/**
  * @brief  CAN引脚配置
  * @param  None
  * @retval None
  */
void CAN_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
    
  /*外设时钟设置*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

  /* Configure CAN pin: RX PA11*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	            // 上拉输入
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure CAN pin: TX PA12 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		         // 复用推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		       // 输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_8);											   // 拉低，进入normal mode
}
/**
  * @brief  CAN接收中断配置
  * @param  None
  * @retval None
  */
void CAN_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable CAN RX0 interrupt IRQ channel */
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
/**
  * @brief  配置CAN接收过滤器
  * @param  FilterNumber 过滤器号
  * @param  can_addr CAN节点地址，该参数非常重要，同一个CAN总线网络其节点地址不能重复
  * @retval None
  */
void CAN_ConfigFilter(uint8_t FilterNumber,uint16_t can_addr)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

	//设置CAN接收过滤器
  CAN_FilterInitStructure.CAN_FilterNumber = FilterNumber;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	
	//以下4个为0表示接收任何数据
//  CAN_FilterInitStructure.CAN_FilterIdHigh=can_addr>>(16-CMD_WIDTH-3);
//  CAN_FilterInitStructure.CAN_FilterIdLow=(can_addr<<(CMD_WIDTH+3))|0x04;
//  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=ADDR_MASK>>(16-CMD_WIDTH-3);;
//  CAN_FilterInitStructure.CAN_FilterMaskIdLow=(ADDR_MASK<<(CMD_WIDTH+3))|0x04;
  CAN_FilterInitStructure.CAN_FilterIdHigh= 0x00;
  CAN_FilterInitStructure.CAN_FilterIdLow= 0x00;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x00;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x00;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;

  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}
/**
  * @brief  初始化CAN
  * @param  BaudRate CAN总线波特率
  * @retval None
  */
void CAN_Configuration(uint32_t BaudRate)
{
  CAN_InitTypeDef        CAN_InitStructure;
	
  /* CAN register init */
  CAN_NVIC_Configuration();
  CAN_GPIO_Configuration();

	CAN_DeInit(CAN1);
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;	//非时间触发通信模式
  CAN_InitStructure.CAN_ABOM = DISABLE; //软件自动离线管理	
  CAN_InitStructure.CAN_AWUM = DISABLE; //睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
  CAN_InitStructure.CAN_NART = ENABLE; //禁止报文自动传送
  CAN_InitStructure.CAN_RFLM = DISABLE; //报文不锁定,新的覆盖旧的
  CAN_InitStructure.CAN_TXFP = DISABLE; //优先级由报文标识符决定 
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//正常模式
	CAN_InitStructure.CAN_SJW = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].SJW;//配置波特率
  CAN_InitStructure.CAN_BS1 = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].BS1;
  CAN_InitStructure.CAN_BS2 = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].BS2;
  CAN_InitStructure.CAN_Prescaler = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].PreScale;
  CAN_Init(CAN1,&CAN_InitStructure);
	
  //设置CAN接收过滤器
  CAN_ConfigFilter(0,0x00);//广播地址，接受广播命令
//  CAN_ConfigFilter(1,Read_CAN_Address());//本节点真实地址

  //使能接收中断
  CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE);
}


/**
  * @brief  发送一帧CAN数据
  * @param  CANx CAN通道号
	* @param  TxMessage CAN消息指针
  * @retval None
  */
uint8_t CAN_WriteData(CanTxMsg *TxMessage)
{
  uint8_t TransmitMailbox;   
  uint32_t	TimeOut=0;
  TransmitMailbox = CAN_Transmit(CAN1,TxMessage);
  while(CAN_TransmitStatus(CAN1,TransmitMailbox)!=CAN_TxStatus_Ok){
    TimeOut++;
    if(TimeOut > 10000000){
      return 1;
    }
  }
  return 0;
}
/**
  * @brief  CAN接收中断处理函数
  * @param  None
  * @retval None
  */
void CAN1_RX0_IRQHandler(void)
{
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
	{
		CAN_Receive(CAN1, CAN_FIFO0, &CAN_RxMessage);
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
		
		CanQueueWrite(&can_queue_send,(can_frame_t *)&CAN_RxMessage);
	}
}

/**
  * @brief  获取CAN节点地址，该函数根据自己的实际情况进行修改
  * @param  None
  * @retval None
  */
uint16_t Read_CAN_Address(void)
{
  return 0x1234;//返回的地址值需要根据实际情况进行修改
}

/*********************************END OF FILE**********************************/
