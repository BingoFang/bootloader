#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "sys.h"  

#define APP_EXE_FLAG_START_ADDR    ((uint32_t)0x08002800)//APP程序成功运行标志存储地址
#define APP_START_ADDR             ((uint32_t)0x08003000)//APP程序起始地址

FLASH_Status USART_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr);
void USART_BOOT_JumpToApplication(uint32_t Addr);
FLASH_Status USART_BOOT_ProgramDatatoFlash(uint32_t StartAddress,uint8_t *pData,uint32_t DataNum);

#endif


