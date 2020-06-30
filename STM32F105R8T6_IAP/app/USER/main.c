/*************************************************************************
 * ��    �� : main.c
 * �� д ��	��bingofang
 * ��    ��	��appʵ��ҵ���߼�����תbootloader����
 * ��дʱ��	��2020-06-30
 * ��    ��	��v1.0
**************************************************************************/
#include "main.h"

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


int main(void)
{ 
	SCB->VTOR = FLASH_BASE | APP_START_ADDR;
	
	/* ���������־λ */
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0xFFFFFFFF)
	{
		__align(4) static unsigned char data[4] = {0x12, 0x34, 0x56, 0x78};
		FLASH_Unlock();
		USART_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_START_ADDR, data, 4);
		FLASH_Lock();
  }
  __set_PRIMASK(0);//�������ж�
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����2
	delay_init();	    	 													 //��ʱ������ʼ��	  
	LED_Init();		  		 													 //��ʼ����LED���ӵ�Ӳ���ӿ�
	CAN_Configuration(125000);										 //CAN������125000
	CanQueueInit();																 //CAN���г�ʼ��
	USART1_Init(256000); 													 //���ڳ�ʼ��Ϊ256000
	UsartQueueInit(&usart1_send);									 //���ڶ��г�ʼ��
	
	LED4 = 1;

	while(1)
	{
		handle_usart_queue();
//		handle_can_queue();
	}   	   
}












