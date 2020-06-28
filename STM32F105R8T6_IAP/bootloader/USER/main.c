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

//本地测试跳转app
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
	__set_PRIMASK(0);//开启总中断
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组2
	delay_init();	    			 											 //延时函数初始化	  
	LED_Init();		  		 													 //初始化与LED连接的硬件接口
//	CAN_Configuration(125000);										 //CAN波特率125000
//	CanQueueInit();																 //CAN队列初始化
	USART1_Init(115200); 													 //串口初始化为256000
	UsartQueueInit(&usart1_send);									 //串口队列初始化

	LED3 = 1;
	
	USART1_Send_Data("stm32105-bootloader",strlen("stm32105-bootloader"));
	
	while(1)
	{
		handle_usart_queue();
//		handle_can_queue();
	}   	   
}












