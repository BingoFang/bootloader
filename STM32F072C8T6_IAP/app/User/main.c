/*************************************************************************
 * ��    �� : main.c
 * �� д ��	��bingofang
 * ��    ��	��appʵ��ҵ���߼���CAN����
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
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));

void VectorTableOffset(void)
{
  uint8_t i;
  
  /* Relocate by software the vector table to the internal SRAM at 0x20000000  
  ���ƫ��DCD�ж�������0x20000000����startup_stm32f0xx.s��48���ж�Ԫ�أ���ƫ��48*4�ֽ�
	��Ҫ��0x200000C0֮�󣬱������ⱻ�޸ġ�*/
  for(i = 0; i < 48; i++)
  {
    VectorTable[i] = *(__IO uint32_t*)(APP_START_ADDR + (i << 2));
  }
  /* Enable the SYSCFG peripheral clock*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  /* Remap SRAM at 0x00000000 */
	SYSCFG->CFGR1 |= 0x03;
}


int main(void)
{
	VectorTableOffset();  //��������ж�������
	
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0xFFFFFFFF){
		__align(4) static unsigned char data[4] = {0x12, 0x34, 0x56, 0x78};
		FLASH_Unlock();
		CAN_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_START_ADDR, data, 4);
    FLASH_Lock();
  }
  __set_PRIMASK(0);//�������ж�
	
	
	LED_Init();
	delay_init();
	CAN_Configuration(125000);
	
	LED1_ON(); LED2_ON();
	delay_ms(2000);
	LED1_OFF(); LED2_OFF();		
	
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
