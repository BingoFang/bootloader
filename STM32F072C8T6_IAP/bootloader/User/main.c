//////////////////////////////////////////////////////////////////////////////////	 
//  版 本 号   : v1.0
//  作    者   : youfang
//  生成日期   : 2020-06-15
//  最近修改   : 
//  功能描述   : 测试can bootloader
//******************************************************************************/

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

CBL_CMD_LIST CMD_List = 
{
  .Erase = 0x00,      //擦除APP区域数据
  .WriteInfo = 0x01,  //设置多字节写数据相关参数（写起始地址，数据量）
  .Write = 0x02,      //以多字节形式写数据
  .Check = 0x03,      //检测节点是否在线，同时返回固件信息
  .SetBaudRate = 0x04,//设置节点波特率
  .Excute = 0x05,     //执行固件
  .CmdSuccess = 0x08, //命令执行成功
  .CmdFaild = 0x09,   //命令执行失败
};

extern CanRxMsg CAN_RxMessage;
extern volatile uint8_t CAN_RxMsgFlag;//接收到CAN数据后的标志
uint8_t CAN_TxMsgBuf[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};

//本地直接跳转app
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
	
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR)==0x78563412){
		LED2_ON();
		delay_ms(2000);
    CAN_BOOT_JumpToApplication(APP_START_ADDR);
  }
  __set_PRIMASK(0);//开启总中断
  CAN_Configuration(125000);	//can通信配置
	LED1_ON();
	delay_ms(2000);
	
	LocalJumpApp();
	
  while (1)
  {	
//		if(CAN_RxMsgFlag){ 
//    CAN_BOOT_ExecutiveCommand(&CAN_RxMessage);
//    CAN_RxMsgFlag = 0;
//		}
		if (CAN_RxMsgFlag)
		{
			CAN_RxMsgFlag = 0;
			if(0 == strncmp((char *)CAN_TxMsgBuf, (char *)CAN_RxMessage.Data, sizeof(CAN_TxMsgBuf)))
			{
				LED3_ON();
				delay_ms(2000);
				LED3_OFF();
			}
			else
			{
				LED4_ON();
				delay_ms(2000);
				LED4_OFF();				
			} 
		}
  } 
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
