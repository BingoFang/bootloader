/*************************************************************************
 * 文    件 : main.c
 * 编 写 人	：fmq
 * 描    述	：app实现指令解析和uart转can数据透传
 * 编写时间	：2020-06-30
 * 版    本	：v1.0
**************************************************************************/
#include "main.h"

/*
IAP升级区域划分,flash:64kb,sram:64kb,page size 2kb
==========================================
	‖boot‖ 		‖start addr‖    ‖size‖
------------------------------------------
	IROM			0x08000000			0x2800		
	IRAM			0x20000000			0x10000 		
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
	IRAM			0x20000000			0x10000 		
==========================================
*/


int main(void)
{ 
	SCB->VTOR = FLASH_BASE | APP_START_ADDR;
	
	/* 检查升级标志位 */
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0xFFFFFFFF)
	{
		__align(4) static unsigned char data[4] = {0x12, 0x34, 0x56, 0x88};
		FLASH_Unlock();
		USART_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_START_ADDR, data, 4);
		FLASH_Lock();
  }
  __set_PRIMASK(0);//开启总中断
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组2
	DelayInit();	    	 													 //延时函数初始化	  
	LED_Init();		  		 													 //初始化与LED连接的硬件接口
	IWDG_Init(4, 625);														 //溢出定时为1S
	CAN_Configuration(125000);										 //CAN配置初始化
	CanQueueInit();																 //CAN队列初始化
	USART1_Init(115200); 													 //串口初始化为256000
	UsartQueueInit(&usart1_send);									 //串口队列初始化
	
	LED4 = 1;
	JumpFirmwareSuccess();	   										 //程序跳转成功回复

	while(1)
	{
		IWDG_ReloadCounter();
		
		HandleUsartQueue();
		HandleCanQueue();
	}   	   
}













