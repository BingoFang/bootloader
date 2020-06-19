//////////////////////////////////////////////////////////////////////////////////	 
//  �� �� ��   : v1.0
//  ��    ��   : youfang
//  ��������   : 2020-06-15
//  ����޸�   : 
//  ��������   : ����can bootloader
//******************************************************************************/

#include "main.h"

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
		delay_ms(5000);
    CAN_BOOT_JumpToApplication(APP_START_ADDR);
  }
  __set_PRIMASK(0);//�������ж�
//  CAN_Configuration(1000000);	//canͨ������
	LED1_ON();
	delay_ms(5000);
	
  while (1)
  {	
//		if(CAN_RxMsgFlag){
//    CAN_BOOT_ExecutiveCommand(&CAN_RxMessage);
//    CAN_RxMsgFlag = 0;
//		}
		LocalJumpApp();
  } 
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
