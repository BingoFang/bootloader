/*************************************************************************
 * 文    件 : main.c
 * 编 写 人	：fmq
 * 描    述	：bootloader实现UART升级管理
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

/* 本地测试直接跳转app */
void LocalJumpApp(void)
{
	if((*((uint32_t *)APP_START_ADDR) != 0xFFFFFFFF))
	{
		USART_BOOT_JumpToApplication(APP_START_ADDR);
  }
}   

int main(void)
{ 
	/* 检查升级标志，判断是否有升级标志符 */
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0x78563412)
	{
    USART_BOOT_JumpToApplication(APP_START_ADDR);
  }
	__set_PRIMASK(0);															 //开启总中断
	
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组2
	DelayInit();	    			 											 //延时函数初始化	  
	LED_Init();		  		 													 //初始化与LED连接的硬件接口
	USART1_Init(115200); 													 //串口配置初始化
	IWDG_Init(4, 625);														 //溢出定时为1S,跳转固件失败会复位看门狗
	UsartQueueInit(&usart1_send);									 //串口队列初始化
	
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{
		/* IWDG flag set */
		LED3 = 1;
		LED4 = 1;
		
		RCC_ClearFlag();
	}
	else
	{
		LED3 = 0;
		LED4 = 0;
	}

	LED3 = 1;
	JumpFirmwareSuccess();													//程序跳转成功回复
	
	/* 网关上传新固件，下发查询版本；（1）进入bootloader模式时设备主动请求 （2）app模式等待下发，
	f105作为消息中转者，确保F105的正常应答至关重要！
		 多机升级，can设备有ID标识，可根据设备地址来逐个交互升级，下发升级版本号由网关决定。*/
	while(1)
	{
		IWDG_ReloadCounter();
		HandleUsartQueue();
	}   	   
}












