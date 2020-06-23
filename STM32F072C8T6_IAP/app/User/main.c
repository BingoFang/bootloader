//////////////////////////////////////////////////////////////////////////////////	 
//  �� �� ��   : v1.0
//  ��    ��   : youfang
//  ��������   : 2020-06-15
//  ����޸�   : 
//  ��������   : ����can app
//******************************************************************************/

#include "main.h"
#include <string.h>

/*
IAP�������򻮷�,flash:64kb,sram:16kb,page size 2kb
==========================================
	��boot�� 		��start addr��    ��size��
------------------------------------------
	IROM			0x08000000			0x2800		
	IRAM			0x20000000			0x4000 		
==========================================

==========================================
	��param�� 	��start addr��    ��size��
------------------------------------------
	IROM			0x08002800			0x800		
==========================================

==========================================
	��app�� 		��start addr��    ��size��
------------------------------------------
	IROM			0x08003000			0xD000		
	IRAM			0x200000C0			0x3F40 		
==========================================
*/

CBL_CMD_LIST CMD_List = 
{
  .Erase = 0x00,      //����APP��������
  .WriteInfo = 0x01,  //���ö��ֽ�д������ز�����д��ʼ��ַ����������
  .Write = 0x02,      //�Զ��ֽ���ʽд����
  .Check = 0x03,      //���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
  .SetBaudRate = 0x04,//���ýڵ㲨����
  .Excute = 0x05,     //ִ�й̼�
  .CmdSuccess = 0x08, //����ִ�гɹ�
  .CmdFaild = 0x09,   //����ִ��ʧ��
};

extern CanRxMsg CAN_RxMessage;
extern volatile uint8_t CAN_RxMsgFlag;//���յ�CAN���ݺ�ı�־
uint8_t CAN_TxMsgBuf[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};

__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));

void VectorTableOffset(void)
{
  uint8_t i;
  
  /* Relocate by software the vector table to the internal SRAM at 0x20000000  
  ���ƫ��DCD�ж�������0x20000000����startup_stm32f0xx.s��48���ж�Ԫ�أ���ƫ��48*4�ֽ�
	��Ҫ��0x200000C0֮�󣬱������ⱻ�޸ġ�*/
  for(i = 0; i < 48; i++)
  {
    VectorTable[i] = *(__IO uint32_t*)(APP_START_ADDR + (i << 2));
  }
  /* Enable the SYSCFG peripheral clock*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  /* Remap SRAM at 0x00000000 */
	SYSCFG->CFGR1 |= 0x03;
}

uint8_t i;
int main(void)
{
	VectorTableOffset();  //��������ж�������
	
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0xFFFFFFFF){
		__align(4) static unsigned char data[4] = {0x12, 0x34, 0x56, 0x78};
		FLASH_Unlock();
		CAN_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_START_ADDR, data, 4);
    FLASH_Lock();
  }
  __set_PRIMASK(0);//�������ж�
	
	
	LED_Init();
	delay_init();
	CAN_Configuration(125000);
	
	for (i = 0; i < 3; i++)
	{
		if (0 == CAN_SendMsg(CAN_TxMsgBuf,8))
		{
			LED3_ON();	LED4_ON();
			delay_ms(2000);
			LED3_OFF(); LED4_OFF();
		}
		else
		{
				LED3_ON();	LED4_ON();
		}
		delay_ms(1500);
	}
	
  while (1)
  {	
//		if(CAN_RxMsgFlag){
//     CAN_BOOT_ExecutiveCommand(&CAN_RxMessage);
//     CAN_RxMsgFlag = 0;
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
