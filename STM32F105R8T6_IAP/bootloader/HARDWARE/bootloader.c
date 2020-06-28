#include "main.h"

typedef void (*pFunction)(void);

#define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
#define FLASH_SIZE                        (0x10000)  /* 64 kBytes */

/**
  * @brief  ��������д��ָ����ַ��Flash�� ��
  * @param  Address Flash��ʼ��ַ��
  * @param  Data ���ݴ洢����ʼ��ַ��
  * @param  DataNum �����ֽ�����
  * @retval ������д״̬��
  */
FLASH_Status USART_BOOT_ProgramDatatoFlash(uint32_t StartAddress,uint8_t *pData,uint32_t DataNum) 
{
  FLASH_Status FLASHStatus = FLASH_COMPLETE;

  uint32_t i;

  if(StartAddress < APP_EXE_FLAG_START_ADDR){
    return FLASH_ERROR_PG;
  }
   /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

  for(i=0; i < (DataNum >> 2); i++)
  {
    FLASHStatus = FLASH_ProgramWord(StartAddress, *((uint32_t*)pData));
    if (FLASHStatus == FLASH_COMPLETE){
      StartAddress += 4;
      pData += 4;
    }else{ 
      return FLASHStatus;
    }
  }
  return	FLASHStatus;
}

/**
  * @brief  ����ָ�����������Flash���� ��
  * @param  StartPage ��ʼ������ַ
  * @param  EndPage ����������ַ
  * @retval ��������״̬  
  */
FLASH_Status USART_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr)
{
  uint32_t i;
  FLASH_Status FLASHStatus=FLASH_COMPLETE;

  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

  for(i = StartPageAddr; i <= EndPageAddr; i += PAGE_SIZE){
    FLASHStatus = FLASH_ErasePage(i);
    if(FLASHStatus!=FLASH_COMPLETE){
      FLASH_Lock();
      return	FLASHStatus;	
    }
  }
  FLASH_Lock();
  return FLASHStatus;
}

void USART_BOOT_JumpToApplication(uint32_t Addr)
{
	static  pFunction Jump_To_Application;
	__IO uint32_t JumpAddress;
  /* Test if user code is programmed starting from address "ApplicationAddress" */
	if (((*(__IO uint32_t*)Addr) & 0x2FFE0000) == 0x20000000)
	{
    /* Jump to user application */
		JumpAddress = *(__IO uint32_t*)(Addr + 4);
		Jump_To_Application = (pFunction)JumpAddress;
		__set_PRIMASK(1); //�ر������ж�
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*)Addr);
		Jump_To_Application();
	}
}

