#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "stm32f0xx.h"

#define CMD_WIDTH  		 	4         //不要修改
#define CMD_MASK    		0xF       //不要修改
#define CAN_ID_TYPE 		1         //1为扩展帧，0为标准帧，不要修改
#define ADDR_MASK   		0x1FFFFFF //不要修改

#define APP_EXE_FLAG_START_ADDR    ((uint32_t)0x08002800)//APP程序成功运行标志存储地址
#define APP_START_ADDR             ((uint32_t)0x08003000)//APP程序起始地址


FLASH_Status CAN_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr);
void CAN_BOOT_JumpToApplication(__IO uint32_t Addr);
FLASH_Status CAN_BOOT_ProgramDatatoFlash(uint32_t StartAddress,uint8_t *pData,uint32_t DataNum);

#endif

