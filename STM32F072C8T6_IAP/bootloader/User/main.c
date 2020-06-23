//////////////////////////////////////////////////////////////////////////////////	 
//  �� �� ��   : v1.0
//  ��    ��   : youfang
//  ��������   : 2020-06-15
//  ����޸�   : 
//  ��������   : ����can bootloader
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

//����ֱ����תapp
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
  __set_PRIMASK(0);//�������ж�
  CAN_Configuration(125000);	//canͨ������
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
