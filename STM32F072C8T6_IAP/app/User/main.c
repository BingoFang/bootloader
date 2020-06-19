//////////////////////////////////////////////////////////////////////////////////	 
//  �� �� ��   : v1.0
//  ��    ��   : youfang
//  ��������   : 2020-06-15
//  ����޸�   : 
//  ��������   : ����can app
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

int main(void)
{
//	VectorTableOffset();  //��������ж�������
	
	if(*((uint32_t *)APP_EXE_FLAG_START_ADDR) == 0xFFFFFFFF){
		__align(4) static unsigned char data[4] = {0x12, 0x34, 0x56, 0x78};
		FLASH_Unlock();
		CAN_BOOT_ProgramDatatoFlash(APP_EXE_FLAG_START_ADDR, data, 4);
    FLASH_Lock();
  }
  __set_PRIMASK(0);//�������ж�
	
	
	LED_Init();
	delay_init();
	CAN_Configuration(1000000);

	LED3_ON(); 
	delay_ms(5000);
	LED4_ON();
	
	delay_ms(5000);
	LED3_OFF();
	LED4_OFF();
	
  while (1)
  {	
		if(CAN_RxMsgFlag){
//     CAN_BOOT_ExecutiveCommand(&CAN_RxMessage);
     CAN_RxMsgFlag = 0;
		}
  } 
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
