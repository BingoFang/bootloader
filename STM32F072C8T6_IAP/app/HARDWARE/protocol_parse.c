#include "main.h"

#define ACK_CMD					 0xF0

#define CAN_BL_BOOT     0x55555555
#define CAN_BL_APP      0xAAAAAAAA
#define FW_TYPE          CAN_BL_APP
#define FW_VER					 0x00010000		//v1.0

cmd_list_t cmd_list = 
{
	.write_info 		= 0x01,
	.write_bin 			= 0x02,
	.check_version 	= 0x03,
	.set_baundrate 	= 0x04,
	.excute 				= 0x05,
	.request        = 0x06,
	.cmd_success 		= 0x08,
	.cmd_failed 		= 0x09,
};

/**
  * @brief  ִ�������·�������
  * @param  pRxMessage CAN������Ϣ
  * @retval ��
  */
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage)
{
  CanTxMsg TxMessage;
  uint8_t can_cmd = (pRxMessage->ExtId) & CMD_MASK;//ID��bit0~bit3λΪ������
  uint8_t can_addr = (pRxMessage->ExtId >> CMD_WIDTH);//ID��bit4~bit11λΪ�ڵ��ַ
  uint32_t BaudRate;
  uint32_t exe_type;
  //�жϽ��յ����ݵ�ַ�Ƿ�ͱ��ڵ��ַƥ�䣬����ƥ����ֱ�ӷ��أ������κ�����
  if((can_addr != CAN_BOOT_GetAddrData()) && (can_addr != 0))
	{
    return;
  }
  TxMessage.DLC = 0;
  TxMessage.ExtId = 0;
  TxMessage.IDE = CAN_Id_Extended;
  TxMessage.RTR = CAN_RTR_Data;

  //cmd_list.set_baundrate�����ýڵ㲨���ʣ����岨������Ϣ�洢��Data[0]��Data[3]��
  //���Ĳ����ʺ�������Ҳ��Ҫ����Ϊ��ͬ�Ĳ����ʣ�����������ͨ��
  if(can_cmd == cmd_list.set_baundrate)
	{ 
    BaudRate = (pRxMessage->Data[0] << 24) | (pRxMessage->Data[1] << 16)
							|(pRxMessage->Data[2] << 8) | (pRxMessage->Data[3] << 0);
    if(can_addr != 0x00)
		{
      TxMessage.ExtId = (CAN_BOOT_GetAddrData() << CMD_WIDTH) | can_cmd;
			TxMessage.Data[0] = cmd_list.cmd_success;
      TxMessage.DLC = 1;
      CAN_WriteData(&TxMessage);
			CAN_Configuration(BaudRate);
    }
  }
	
  //cmd_list.check_version���ڵ����߼��
  //�ڵ��յ�������󷵻ع̼��汾��Ϣ�͹̼����ͣ���������Bootloader�����APP���򶼱���ʵ��
  if(can_cmd == cmd_list.check_version)
	{
    if(can_addr != 0x00)
		{
      TxMessage.ExtId = (CAN_BOOT_GetAddrData() << CMD_WIDTH) | can_cmd;
      TxMessage.Data[0] = (uint8_t)(FW_VER >> 24);//���汾�ţ����ֽ�
      TxMessage.Data[1] = (uint8_t)(FW_VER >> 16);
      TxMessage.Data[2] = (uint8_t)(FW_VER >> 8); //�ΰ汾�ţ����ֽ�
      TxMessage.Data[3] = (uint8_t)(FW_VER >> 0);
      TxMessage.Data[4] = (uint8_t)(FW_TYPE>>24);
      TxMessage.Data[5] = (uint8_t)(FW_TYPE>>16);
      TxMessage.Data[6] = (uint8_t)(FW_TYPE>>8);
      TxMessage.Data[7] = (uint8_t)(FW_TYPE>>0);
      TxMessage.DLC = 8;
      CAN_WriteData(&TxMessage);
    }
  }
	
  //cmd_list.excute�����Ƴ�����ת��ָ����ִַ��
  //��������Bootloader��APP�����ж�����ʵ��,��֤��ת�Ƿ�ɹ�����check version
  if(can_cmd == cmd_list.excute)
	{
    exe_type = (pRxMessage->Data[0] << 24) | (pRxMessage->Data[1] << 16)
							|(pRxMessage->Data[2] << 8) | (pRxMessage->Data[3] << 0);
    if(exe_type == CAN_BL_BOOT)
		{
			FLASH_Unlock();
      CAN_BOOT_ErasePage(APP_EXE_FLAG_START_ADDR,APP_EXE_FLAG_START_ADDR);//����д�뵽Flash�е�APPִ�б�־����λ���к󣬼���ִ��Bootloader����
			FLASH_Lock();
			__set_PRIMASK(1);  //�ر������ж�
			NVIC_SystemReset();
    }
  }
}

/**
  * @brief  ��ȡ�ڵ��ַ��Ϣ
  * @param  None
  * @retval �ڵ��ַ��
  */
uint8_t CAN_BOOT_GetAddrData(void)
{
  return Read_CAN_Address();
}

/* ����F105�·������ݰ����� */
void handle_can_queue(void)
{
	can_frame_t from_mcu_can_data;
	if (CAN_OK == CanQueueRead(&can_queue_send, &from_mcu_can_data))
	{
		CAN_BOOT_ExecutiveCommand(&from_mcu_can_data);
	}
}
