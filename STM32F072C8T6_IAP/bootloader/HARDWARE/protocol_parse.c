#include "main.h"
#include <string.h>

#define ACK_CMD					 0xF0

#define CAN_BL_BOOT      0x55555555
#define CAN_BL_APP       0xAAAAAAAA
#define FW_TYPE          CAN_BL_BOOT
#define FW_VER					 0x00010000		//v1.0

cmd_list_t cmd_list = 
{
	.write_info 		= 0x01,
	.write_bin 			= 0x02,
	.check_version 	= 0x03,
	.set_baundrate 	= 0x04,
	.excute 				= 0x05,
	.cmd_success 		= 0x08,
	.cmd_failed 		= 0x09,
};

void JumpFirmwareSuccess(void)
{
	CanTxMsg TxMessage = {0};
	
	TxMessage.ExtId = (CAN_BOOT_GetAddrData() << CMD_WIDTH | cmd_list.excute);
	TxMessage.IDE = CAN_Id_Extended;
	TxMessage.RTR = CAN_RTR_Data;
	
	TxMessage.Data[0] = (uint8_t)(FW_TYPE >> 24); 
	TxMessage.Data[1] = (uint8_t)(FW_TYPE >> 16); 
	TxMessage.Data[2] = (uint8_t)(FW_TYPE >> 8);
	TxMessage.Data[3] = (uint8_t)(FW_TYPE >> 0); 
	TxMessage.DLC = 4;
	
	CAN_WriteData(&TxMessage);
}

/**
  * @brief  执行主机下发的命令
  * @param  pRxMessage CAN总线消息
  * @retval 无
  */
#define TEST  //测试阶段
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage)
{
  CanTxMsg TxMessage;
  uint8_t ret,i;
  uint8_t can_cmd = (pRxMessage->ExtId) & CMD_MASK;//ID的bit0~bit3位为命令码
  uint8_t can_addr = (pRxMessage->ExtId >> CMD_WIDTH);//ID的bit4~bit11位为节点地址
  uint32_t BaudRate;
  uint16_t crc_data;
  uint32_t addr_offset;
  uint32_t exe_type;
  uint32_t FlashSize;
  static uint32_t start_addr = APP_START_ADDR;
  static uint32_t data_size=0;
  static uint32_t data_index=0;
  __align(4) static uint8_t	data_temp[PAGE_SIZE + 2];
  //判断接收的数据地址是否和本节点地址匹配，若不匹配则直接返回，不做任何事情，地址0为广播模式
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
	
  //cmd_list.write_info，设置写Flash数据的相关信息，比如数据起始地址，数据大小
  //数据偏移地址存储在Data[0]到Data[3]中，数据大小存储在Data[4]到Data[7]中，并擦除对应大小page
  if(can_cmd == cmd_list.write_info)
	{
		TxMessage.ExtId = (CAN_BOOT_GetAddrData() << CMD_WIDTH) | can_cmd;
		
    addr_offset = (pRxMessage->Data[0] << 24) | (pRxMessage->Data[1] << 16) 
								 |(pRxMessage->Data[2] << 8) | (pRxMessage->Data[3] << 0);
    start_addr = APP_START_ADDR + addr_offset;
		
    data_size = (pRxMessage->Data[4] << 24) | (pRxMessage->Data[5] << 16) 
							 |(pRxMessage->Data[6] << 8) | (pRxMessage->Data[7] << 0);
    data_index = 0;
 
		#ifndef TEST
			__set_PRIMASK(1);
			FLASH_Unlock();
			ret = CAN_BOOT_ErasePage(start_addr, start_addr);
			FLASH_Lock();
			__set_PRIMASK(0);
		#else
			__set_PRIMASK(1);
			FLASH_Unlock();
			ret = CAN_BOOT_ErasePage(start_addr, APP_END_ADDR);
			FLASH_Lock();
			__set_PRIMASK(0);
		#endif
		
    if(can_addr != 0x00)
		{
			if (ret == FLASH_COMPLETE)
				TxMessage.Data[0] = cmd_list.cmd_success;
			else
				TxMessage.Data[0] = cmd_list.cmd_failed;
			
      TxMessage.DLC = 1;
      CAN_WriteData(&TxMessage);
    }
		else
		{
			TxMessage.Data[0] = cmd_list.cmd_failed;
			TxMessage.DLC = 1;
      CAN_WriteData(&TxMessage);
		}
  }
	
  //cmd_list.write_bin，先将数据存储在本地缓冲区中，然后计算数据的CRC，若校验正确则写数据到Flash中
  //每次执行该数据，数据缓冲区的数据字节数会增加pRxMessage->DLC字节，当数据量达到data_size（包含2字节CRC校验码）字节后
  //对数据进行CRC校验，若数据校验无误，则将数据写入Flash中
  //该函数在Bootloader程序中必须实现，APP程序可以不用实现
  if(can_cmd == cmd_list.write_bin)
	{
		TxMessage.ExtId = (CAN_BOOT_GetAddrData() << CMD_WIDTH) | can_cmd;
		
		#ifndef TEST
    if((data_index < data_size) && (data_index < PAGE_SIZE + 2))
		{
      for(i = 0; i < pRxMessage->DLC; i++)
			{
        data_temp[data_index++] = pRxMessage->Data[i];
      }
    }
    if((data_index >= data_size) || (data_index >= PAGE_SIZE + 2))
		{
      crc_data = crc16_xmodem(data_temp,data_size - 2);//对接收到的数据做CRC校验，保证数据完整性
      if(crc_data == ((data_temp[data_size - 2] << 8) | (data_temp[data_size - 1])))
			{
        __set_PRIMASK(1);
        FLASH_Unlock();
        ret = CAN_BOOT_ProgramDatatoFlash(start_addr,data_temp,data_size - 2);
        FLASH_Lock();	
        __set_PRIMASK(0);
        if(can_addr != 0x00)
				{
          if(ret==FLASH_COMPLETE)
					{
            crc_data = crc16_xmodem((const unsigned char*)(start_addr),data_size - 2);//再次对写入Flash中的数据进行CRC校验，确保写入Flash的数据无误
            if(crc_data != ((data_temp[data_size - 2] << 8) | (data_temp[data_size - 1])))
							TxMessage.Data[0] = cmd_list.cmd_failed;
						else
							TxMessage.Data[0] = cmd_list.cmd_success;
          }
					else
					{
						TxMessage.Data[0] = cmd_list.cmd_failed;
          }
          TxMessage.DLC = 1;
          CAN_WriteData(&TxMessage);
        }
      }
			else
			{
				TxMessage.Data[0] = cmd_list.cmd_failed;
        TxMessage.DLC = 1;
        CAN_WriteData(&TxMessage);
      }
    }
		#else
			if ((data_index < data_size) && (data_index < PAGE_SIZE))
			{
				for (i = 0; i < pRxMessage->DLC; i++)
				{
					data_temp[data_index++] = pRxMessage->Data[i];
				}
				if (pRxMessage->DLC < 8)
					goto no_enough;
			}
			if (data_index >= PAGE_SIZE)
			{
				no_enough:
				__set_PRIMASK(1);
				FLASH_Unlock();
				ret = CAN_BOOT_ProgramDatatoFlash(start_addr, data_temp, data_index);
				FLASH_Lock();
				__set_PRIMASK(0);
				
				
				memset(data_temp, 0, PAGE_SIZE + 2);
				data_index = 0;
				start_addr += PAGE_SIZE;
			}
		#endif
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
    if(exe_type == CAN_BL_APP)
		{
      if((*((uint32_t *)APP_START_ADDR) != 0xFFFFFFFF))
			{
        CAN_BOOT_JumpToApplication(APP_START_ADDR);
      }
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
void HandleCanQueue(void)
{
	can_frame_t from_mcu_can_data;
	if (CAN_OK == CanQueueRead(&can_queue_send, &from_mcu_can_data))
	{
		CAN_BOOT_ExecutiveCommand(&from_mcu_can_data);
	}
}


