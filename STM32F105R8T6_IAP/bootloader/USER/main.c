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

//���ز�����תapp
void LocalJumpApp(void)
{
	if((*((uint32_t *)APP_START_ADDR) != 0xFFFFFFFF)){
		USART_BOOT_JumpToApplication(APP_START_ADDR);
  }
}   


int main(void)
{ 
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR)==0x78563412){
    USART_BOOT_JumpToApplication(APP_START_ADDR);
  }
	__set_PRIMASK(0);//�������ж�
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����2
	delay_init();	    			 											 //��ʱ������ʼ��	  
	LED_Init();		  		 													 //��ʼ����LED���ӵ�Ӳ���ӿ�
//	CAN_Configuration(125000);										 //CAN������125000
//	CanQueueInit();																 //CAN���г�ʼ��
	USART1_Init(115200); 													 //���ڳ�ʼ��Ϊ256000
	UsartQueueInit(&usart1_send);									 //���ڶ��г�ʼ��

	LED3 = 1;
	
	USART1_Send_Data("stm32105-bootloader",strlen("stm32105-bootloader"));
	
	while(1)
	{
		handle_usart_queue();
//		handle_can_queue();
	}   	   
}












