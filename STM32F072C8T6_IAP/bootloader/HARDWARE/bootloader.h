#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "stm32f0xx.h"

#define CMD_WIDTH  		 	4         //��Ҫ�޸�
#define CMD_MASK    		0xF       //��Ҫ�޸�
#define CAN_ID_TYPE 		1         //1Ϊ��չ֡��0Ϊ��׼֡����Ҫ�޸�
#define ADDR_MASK   		0x1FFFFFF //��Ҫ�޸�

#define APP_EXE_FLAG_START_ADDR    ((uint32_t)0x08002800)//APP����ɹ����б�־�洢��ַ
#define APP_START_ADDR             ((uint32_t)0x08003000)//APP������ʼ��ַ


FLASH_Status CAN_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr);
void CAN_BOOT_JumpToApplication(__IO uint32_t Addr);
FLASH_Status CAN_BOOT_ProgramDatatoFlash(uint32_t StartAddress,uint8_t *pData,uint32_t DataNum);

#endif

