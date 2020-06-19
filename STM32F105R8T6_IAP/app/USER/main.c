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

//粗延时函数，微秒
void soft_delay_us(u16 time)
{
	u16 i=0;
	while(time--)
	{
		i=10;//自己定义
		while(i--);
	}
}
	
//毫秒级的延时
void soft_delay_ms(u16 time)
{
	u16 i=0;
	while(time--)
	{
		i=12000;//自己定义
		while(i--);
	}
}


int main(void)
{ 
	SCB->VTOR = FLASH_BASE | APP_START_ADDR;
	
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0xFFFFFFFF){
	__align(4) static unsigned char data[4] = {0x12, 0x34, 0x56, 0x78};
	FLASH_Unlock();
	USART_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_START_ADDR, data, 4);
   FLASH_Lock();
  }
  __set_PRIMASK(0);//开启总中断
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	delay_init();	    	 //延时函数初始化	  
	uart_init(256000);	 //串口初始化为256000
	LED_Init();		  		 //初始化与LED连接的硬件接口
	
	LED4 = 1;
	delay_ms(1000);


	while(1)
	{
	}   	   
}












