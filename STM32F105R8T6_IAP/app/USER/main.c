#include "main.h"
#include <string.h>

/*
IAP�������򻮷�,flash:64kb,sram:64kb,page size 2kb
==========================================
	��boot�� 		��start addr��    ��size��
------------------------------------------
	IROM			0x08000000			0x2800		
	IRAM			0x20000000			0x10000 		
==========================================

==========================================
	��param�� 	��start addr��    ��size��
------------------------------------------
	IROM			0x08002800			0x800		
==========================================

==========================================
	��app�� 		��start addr��    ��size��
------------------------------------------
	IROM			0x08003000			0xD000		
	IRAM			0x20000000			0x10000 		
==========================================
*/


TestStatus status;
extern CanRxMsg CAN_RxMessage;
extern volatile uint8_t CAN_RxMsgFlag;
uint8_t CAN_TxMsgBuf[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
uint8_t i;

int main(void)
{ 
	SCB->VTOR = FLASH_BASE | APP_START_ADDR;
	
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0xFFFFFFFF){
	__align(4) static unsigned char data[4] = {0x12, 0x34, 0x56, 0x78};
	FLASH_Unlock();
	USART_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_START_ADDR, data, 4);
   FLASH_Lock();
  }
  __set_PRIMASK(0);//�������ж�
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	delay_init();	    	 //��ʱ������ʼ��	  
	uart_init(256000);	 //���ڳ�ʼ��Ϊ256000
	LED_Init();		  		 //��ʼ����LED���ӵ�Ӳ���ӿ�
	CAN_Configuration(125000);  
	
	for (i = 0; i < 3; i++)
	{
		if (0 == CAN_SendMsg(CAN_TxMsgBuf,8))
		{
			LED3 = 1; LED4 = 1;
			delay_ms(1000);
			LED3 = 0; LED4 = 0;
		}
		else
		{
			LED3 = 1; LED4 = 1;
		}
		delay_ms(1500);
	}
	
	while(1)
	{
		if (CAN_RxMsgFlag)
		{
			CAN_RxMsgFlag = 0;
			if(0 == strncmp((char *)CAN_TxMsgBuf,(char *)CAN_RxMessage.Data, sizeof(CAN_TxMsgBuf)))
			{
				LED3 = 1;
				delay_ms(1000);  
				LED3 = 0;
			}
			else
			{
				LED4 = 1;
				delay_ms(1000);
				LED4 = 0;				
			}
		}
	}   	   
}












