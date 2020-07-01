/*************************************************************************
 * 文    件 : main.c
 * 编 写 人	：bingofang
 * 描    述	：app实现业务逻辑和CAN交互
 * 编写时间	：2020-06-30
 * 版    本	：v1.0
**************************************************************************/
#include "main.h"
#include <string.h>

/*
IAP升级区域划分,flash:64kb,sram:16kb,page size 2kb
==========================================
	‖boot‖ 		‖start addr‖    ‖size‖
------------------------------------------
	IROM			0x08000000			0x2800		
	IRAM			0x20000000			0x4000 		
==========================================

==========================================
	‖param‖ 	‖start addr‖    ‖size‖
------------------------------------------
	IROM			0x08002800			0x800		
==========================================

==========================================
	‖app‖ 		‖start addr‖    ‖size‖
------------------------------------------
	IROM			0x08003000			0xD000		
	IRAM			0x200000C0			0x3F40 		
==========================================
*/


extern CanRxMsg CAN_RxMessage;
extern volatile uint8_t CAN_RxMsgFlag;//接收到CAN数据后的标志
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));

void VectorTableOffset(void)
{
  uint8_t i;
  
  /* Relocate by software the vector table to the internal SRAM at 0x20000000  
  软件偏移DCD中断向量表到0x20000000，在startup_stm32f0xx.s中48个中断元素，需偏移48*4字节
	需要在0x200000C0之后，避免意外被修改。*/
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
	VectorTableOffset();  //软件搬运中断向量表
	
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0xFFFFFFFF){
		__align(4) static unsigned char data[4] = {0x12, 0x34, 0x56, 0x78};
		FLASH_Unlock();
		CAN_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_START_ADDR, data, 4);
    FLASH_Lock();
  }
  __set_PRIMASK(0);//开启总中断
	
	
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
