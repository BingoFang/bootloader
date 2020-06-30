#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "stm32f0xx.h"

#define CMD_WIDTH  		 	4         //��Ҫ�޸�
#define CMD_MASK    		0xF       //��Ҫ�޸�
#define CAN_ID_TYPE 		1         //1Ϊ��չ֡��0Ϊ��׼֡����Ҫ�޸�
#define ADDR_MASK   		0x1FFFFFF //��Ҫ�޸�

#define APP_EXE_FLAG_START_ADDR    ((uint32_t)0x08002800)//APP����ɹ����б�־�洢��ַ
#define APP_START_ADDR             ((uint32_t)0x08003000)//APP������ʼ��ַ

#define CAN_BL_BOOT     0x55555555
#define CAN_BL_APP      0xAAAAAAAA
#define FW_TYPE         CAN_BL_BOOT

typedef struct
{
	uint8_t write_info;
	uint8_t write_bin;
	uint8_t check_version;
	uint8_t set_baundrate;
	uint8_t excute;
	uint8_t request;
	uint8_t cmd_success;
	uint8_t cmd_failed;
}cmd_list_t;

uint32_t GetSector(uint32_t Address);
FLASH_Status CAN_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr);
uint16_t CAN_BOOT_GetAddrData(void);
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage);
void CAN_BOOT_JumpToApplication(__IO uint32_t Addr);
FLASH_Status CAN_BOOT_ProgramDatatoFlash(uint32_t StartAddress,uint8_t *pData,uint32_t DataNum);

#endif

