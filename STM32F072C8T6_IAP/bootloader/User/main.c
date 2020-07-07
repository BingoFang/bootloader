/*************************************************************************
 * 文    件 : main.c
 * 编 写 人	：bingofang
 * 描    述	：bootloader实现CAN数据升级管理
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

//本地直接跳转app
void LocalJumpApp(void)
{
	  if((*((uint32_t *)APP_START_ADDR)!=0xFFFFFFFF)){
    CAN_BOOT_JumpToApplication(APP_START_ADDR);
    }
}

int main(void)
{
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0x78563412)
	{
    CAN_BOOT_JumpToApplication(APP_START_ADDR);
  }
  __set_PRIMASK(0);//开启总中断
	
	LED_Init();
	DelayInit();
  CAN_Configuration(125000);	//can通信配置
	
	/* 灯光提示 */
	LED3_ON(); LED4_ON();
	delay_ms(2000);
	LED3_OFF(); LED4_OFF();		
	
	JumpFirmwareSuccess();	   //程序跳转成功回复
	
  while (1)
  {	
		HandleCanQueue();
  } 
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
