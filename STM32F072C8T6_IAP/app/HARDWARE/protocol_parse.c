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
  * @brief  执行主机下发的命令
  * @param  pRxMessage CAN总线消息
  * @retval 无
  */
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage)
{
  CanTxMsg TxMessage;
  uint8_t can_cmd = (pRxMessage->ExtId) & CMD_MASK;//ID的bit0~bit3位为命令码
  uint8_t can_addr = (pRxMessage->ExtId >> CMD_WIDTH);//ID的bit4~bit11位为节点地址
  uint32_t BaudRate;
  uint32_t exe_type;
  //判断接收的数据地址是否和本节点地址匹配，若不匹配则直接返回，不做任何事情
  if((can_addr != CAN_BOOT_GetAddrData()) && (can_addr != 0))
	{
    return;
  }
  TxMessage.DLC = 0;
  TxMessage.ExtId = 0;
  TxMessage.IDE = CAN_Id_Extended;
  TxMessage.RTR = CAN_RTR_Data;

  //cmd_list.set_baundrate，设置节点波特率，具体波特率信息存储在Data[0]到Data[3]中
  //更改波特率后，适配器也需要更改为相同的波特率，否则不能正常通信
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
	
  //cmd_list.check_version，节点在线检测
  //节点收到该命令后返回固件版本信息和固件类型，该命令在Bootloader程序和APP程序都必须实现
  if(can_cmd == cmd_list.check_version)
	{
    if(can_addr != 0x00)
		{
      TxMessage.ExtId = (CAN_BOOT_GetAddrData() << CMD_WIDTH) | can_cmd;
      TxMessage.Data[0] = (uint8_t)(FW_VER >> 24);//主版本号，两字节
      TxMessage.Data[1] = (uint8_t)(FW_VER >> 16);
      TxMessage.Data[2] = (uint8_t)(FW_VER >> 8); //次版本号，两字节
      TxMessage.Data[3] = (uint8_t)(FW_VER >> 0);
      TxMessage.Data[4] = (uint8_t)(FW_TYPE>>24);
      TxMessage.Data[5] = (uint8_t)(FW_TYPE>>16);
      TxMessage.Data[6] = (uint8_t)(FW_TYPE>>8);
      TxMessage.Data[7] = (uint8_t)(FW_TYPE>>0);
      TxMessage.DLC = 8;
      CAN_WriteData(&TxMessage);
    }
  }
	
  //cmd_list.excute，控制程序跳转到指定地址执行
  //该命令在Bootloader和APP程序中都必须实现,验证跳转是否成功可再check version
  if(can_cmd == cmd_list.excute)
	{
    exe_type = (pRxMessage->Data[0] << 24) | (pRxMessage->Data[1] << 16)
							|(pRxMessage->Data[2] << 8) | (pRxMessage->Data[3] << 0);
    if(exe_type == CAN_BL_BOOT)
		{
			FLASH_Unlock();
      CAN_BOOT_ErasePage(APP_EXE_FLAG_START_ADDR,APP_EXE_FLAG_START_ADDR);//擦除写入到Flash中的APP执行标志，复位运行后，即可执行Bootloader程序
			FLASH_Lock();
			__set_PRIMASK(1);  //关闭所有中断
			NVIC_SystemReset();
    }
  }
}

/**
  * @brief  获取节点地址信息
  * @param  None
  * @retval 节点地址。
  */
uint8_t CAN_BOOT_GetAddrData(void)
{
  return Read_CAN_Address();
}

/* 处理F105下发的数据包解析 */
void handle_can_queue(void)
{
	can_frame_t from_mcu_can_data;
	if (CAN_OK == CanQueueRead(&can_queue_send, &from_mcu_can_data))
	{
		CAN_BOOT_ExecutiveCommand(&from_mcu_can_data);
	}
}
