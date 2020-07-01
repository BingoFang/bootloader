/*************************************************************************
 * ��    �� : main.c
 * �� д ��	��bingofang
 * ��    ��	��bootloaderʵ��CAN������������
 * ��дʱ��	��2020-06-30
 * ��    ��	��v1.0
**************************************************************************/
#include "main.h"
#include <string.h>

/*
IAP�������򻮷�,flash:64kb,sram:16kb,page size 2kb
==========================================
	��boot�� 		��start addr��    ��size��
------------------------------------------
	IROM			0x08000000			0x2800		
	IRAM			0x20000000			0x4000 		
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
	IRAM			0x200000C0			0x3F40 		
==========================================
*/


extern CanRxMsg CAN_RxMessage;
extern volatile uint8_t CAN_RxMsgFlag;//���յ�CAN���ݺ�ı�־

//����ֱ����תapp
void LocalJumpApp(void)
{
	  if((*((uint32_t *)APP_START_ADDR)!=0xFFFFFFFF)){
    CAN_BOOT_JumpToApplication(APP_START_ADDR);
    }
}


int main(void)
{
	LED_Init();
	delay_init();
	
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR)==0x78563412)
	{
    CAN_BOOT_JumpToApplication(APP_START_ADDR);
  }
  __set_PRIMASK(0);//�������ж�
  CAN_Configuration(125000);	//canͨ������
	
	LED3_ON(); LED4_ON();
	delay_ms(2000);
	LED3_OFF(); LED4_OFF();		
	
	dev_active_request();	 //��bootloaderģʽ����������
	
  while (1)
  {	
		if (CAN_RxMsgFlag)
		{	
			CAN_RxMsgFlag = 0;
			CAN_BOOT_ExecutiveCommand(&CAN_RxMessage);
		}
//		handle_can_queue();
  } 
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
