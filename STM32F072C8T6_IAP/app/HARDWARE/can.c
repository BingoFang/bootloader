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
volatile uint8_t CAN_RxMsgFlag=0;//���յ�CAN���ݺ�ı�־
volatile uint8_t TimeOutFlag;				///<��ʱ����ʱ��־
tCAN_BaudRate  CAN_BaudRateInitTab[]= {      // CLK=48MHz
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,6},     // 1M
   {CAN_SJW_1tq,CAN_BS1_8tq,CAN_BS2_4tq,4},     // 900K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_3tq,6},     // 800K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_1tq,9},     // 666K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_1tq,10},     // 600K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,12},     // 500K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_1tq,15},     // 400K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_3tq,16},    // 300K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,24},    // 250K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_3tq,21},	// 225K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,30},    // 200K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_3tq,30},	// 160K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,40},    // 150K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_3tq,37},	// 144K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,48},   // 125K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,50},	// 120K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,60},    // 100K
   {CAN_SJW_1tq,CAN_BS1_8tq,CAN_BS2_4tq,41},   // 90K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,75},   // 80K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,80},	// 75K
   {CAN_SJW_1tq,CAN_BS1_5tq,CAN_BS2_2tq,100},    // 60K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_3tq,96},    // 50K
   {CAN_SJW_1tq,CAN_BS1_7tq,CAN_BS2_4tq,100},    // 40K
   {CAN_SJW_1tq,CAN_BS1_10tq,CAN_BS2_5tq,100},   // 30K
   {CAN_SJW_1tq,CAN_BS1_15tq,CAN_BS2_8tq,100},   // 20K
};
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/  

/**
  * @brief  ͨ�������ʵ�ֵ��ȡ�����ʲ���������ֵ
  * @param  BaudRate CAN���߲����ʣ���λΪbps
  * @retval �����ʲ���������ֵ
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
  * @brief  CAN��������
  * @param  None
  * @retval None
  */
void CAN_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
    
  /*����ʱ������*/
  RCC_APB2PeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN, ENABLE);

	/* ����GPIO����CAN_RX CAN_TX */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_4);		
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_4);		
	
  /* Configure CAN pin: RX PA11  TX PA12 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		         // �����������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		         // �������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_10);											 // ���ͣ�����normal mode
}
/**
  * @brief  CAN�����ж�����
  * @param  None
  * @retval None
  */
void CAN_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable CAN RX0 interrupt IRQ channel */
  NVIC_InitStructure.NVIC_IRQChannel = CEC_CAN_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
/**
  * @brief  ����CAN���չ�����
  * @param  FilterNumber ��������
  * @param  can_addr CAN�ڵ��ַ���ò����ǳ���Ҫ��ͬһ��CAN����������ڵ��ַ�����ظ�
  * @retval None
  */
void CAN_ConfigFilter(uint8_t FilterNumber,uint16_t can_addr)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

	//����CAN���չ�����
  CAN_FilterInitStructure.CAN_FilterNumber = FilterNumber;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	
	//����4��Ϊ0��ʾ�����κ�����
//  CAN_FilterInitStructure.CAN_FilterIdHigh=can_addr>>(16-CMD_WIDTH-3);
//  CAN_FilterInitStructure.CAN_FilterIdLow=(can_addr<<(CMD_WIDTH+3))|0x04;
//  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=ADDR_MASK>>(16-CMD_WIDTH-3);;
//  CAN_FilterInitStructure.CAN_FilterMaskIdLow=(ADDR_MASK<<(CMD_WIDTH+3))|0x04;
	CAN_FilterInitStructure.CAN_FilterIdHigh=0x00;
  CAN_FilterInitStructure.CAN_FilterIdLow=0x00;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x00;;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x00;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;


  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}
/**
  * @brief  ��ʼ��CAN
  * @param  BaudRate CAN���߲�����
  * @retval None
  */
void CAN_Configuration(uint32_t BaudRate)
{
  CAN_InitTypeDef        CAN_InitStructure;
	
  /* CAN register init */
  CAN_NVIC_Configuration();
  CAN_GPIO_Configuration();

	CAN_DeInit(CAN);
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;	//��ʱ�䴥��ͨ��ģʽ
  CAN_InitStructure.CAN_ABOM = DISABLE; //����Զ����߹���	
  CAN_InitStructure.CAN_AWUM = DISABLE; //˯��ģʽͨ���������(���CAN->MCR��SLEEPλ)
  CAN_InitStructure.CAN_NART = ENABLE; //��ֹ�����Զ�����
  CAN_InitStructure.CAN_RFLM = DISABLE; //���Ĳ�����,�µĸ��Ǿɵ�
  CAN_InitStructure.CAN_TXFP = DISABLE; //���ȼ��ɱ��ı�ʶ������ 
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//����ģʽ
	CAN_InitStructure.CAN_SJW = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].SJW;//���ò�����Ϊ1M
  CAN_InitStructure.CAN_BS1 = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].BS1;
  CAN_InitStructure.CAN_BS2 = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].BS2;
  CAN_InitStructure.CAN_Prescaler = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].PreScale;
  CAN_Init(CAN,&CAN_InitStructure);
	
  //����CAN���չ�����
  CAN_ConfigFilter(0,0x00);//�㲥��ַ�����ܹ㲥����
//  CAN_ConfigFilter(1,0x1234);//���ڵ���ʵ��ַ

  //ʹ�ܽ����ж�
  CAN_ITConfig(CAN,CAN_IT_FMP0, ENABLE);
}


/**
  * @brief  ����һ֡CAN����
  * @param  CANx CANͨ����
	* @param  TxMessage CAN��Ϣָ��
  * @retval None
  */
uint8_t CAN_WriteData(CanTxMsg *TxMessage)
{
  uint8_t TransmitMailbox;   
  uint32_t	TimeOut=0;
  TransmitMailbox = CAN_Transmit(CAN,TxMessage);
  while(CAN_TransmitStatus(CAN,TransmitMailbox)!=CAN_TxStatus_Ok){
    TimeOut++;
    if(TimeOut > 10000000){
      return 1;
    }
  }
  return 0;
}
/**
  * @brief  CAN�����жϴ�����
  * @param  None
  * @retval None
  */
void CEC_CAN_IRQHandler(void)
{
	if (CAN_GetITStatus(CAN, CAN_IT_FMP0) != RESET)
	{
		CAN_Receive(CAN, CAN_FIFO0, &CAN_RxMessage);
		CAN_ClearITPendingBit(CAN, CAN_IT_FMP0);
		CAN_RxMsgFlag = 1;  //����ǰ��̨��ѯ��Ƿ����������ݰ�����������ᷢ������
//		CanQueueWrite(&can_queue_send,(can_frame_t *)&CAN_RxMessage);
	}
}

/**
  * @brief  ��ȡCAN�ڵ��ַ���ú��������Լ���ʵ����������޸�
  * @param  None
  * @retval None
  */
uint8_t Read_CAN_Address(void)
{
  return 0x01;//���صĵ�ֵַ��Ҫ����ʵ����������޸�
}

/*********************************END OF FILE**********************************/
