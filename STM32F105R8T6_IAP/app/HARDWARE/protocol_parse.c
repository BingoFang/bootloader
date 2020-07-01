#include "main.h"
#include "string.h"

#define HEAD  					 0xAA
#define TAIL  					 0x55
#define ACK_CMD					 0xF0
#define RESERVE					 0x00
#define DETACH_DATA_INFO_SIZE   6

#define UART_BL_BOOT     0x55555555
#define UART_BL_APP      0xAAAAAAAA
#define FW_TYPE          UART_BL_APP
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

static void ack_to_cpu_uart_protocol(data_info_t *data,uint8_t len, uint8_t port)
{
	uint8_t uart_tx_buf[125] = {0};
	uint8_t cnt = 0;
	uint16_t crc;
	
	uart_tx_buf[cnt++] = HEAD;
	uart_tx_buf[cnt++] = len + DETACH_DATA_INFO_SIZE;
	uart_tx_buf[cnt++] = port;
	memcpy(uart_tx_buf + cnt, data, len);
	cnt += len;
	crc = crc16_xmodem(uart_tx_buf, cnt);
	uart_tx_buf[cnt++] = (uint8_t)(crc >> 8); //��λ�ȷ�
	uart_tx_buf[cnt++] = (uint8_t)(crc & 0xFF);
	uart_tx_buf[cnt++] = TAIL;
	
	USART1_SendData(uart_tx_buf, cnt);  //����������Ӧ��
}


static void handle_uart_local(uint8_t *data, uint8_t len)
{
	uint8_t uart_reserve,uart_cmd;
	uint32_t exe_type;
	uint32_t baund_rate;

	/* �����̼�����flash��С�ռ�,ÿҳд�롣 */
	data_info_t *data_info_uart = (data_info_t *)data;
	uart_cmd = data_info_uart->cmd;
	uart_reserve = data_info_uart->option.reserve;
	
	/* ���ò����� */
	if (uart_cmd == cmd_list.set_baundrate)
	{
		baund_rate = (data_info_uart->data[0] << 24) | (data_info_uart->data[1] << 16) 
							| (data_info_uart->data[2] << 8) | (data_info_uart->data[3] << 0);
		
		data_info_uart->cmd |= ACK_CMD;
		data_info_uart->option.reserve = RESERVE;
		data_info_uart->data[0] = cmd_list.cmd_success;
		
		/* ���ڻظ� */
		ack_to_cpu_uart_protocol(data_info_uart, 3, UART_PROTOCOL_PORT);
		
		USART1_Init(baund_rate);  //��δ�ı䲨����ǰ��Ӧ�𷢳����޸���ȷ�����ʺ�ͨ�š�
	}
	
	/* ��ѯ����汾�� */
	if (uart_cmd == cmd_list.check_version)
	{
		data_info_uart->cmd |= ACK_CMD;
		data_info_uart->option.reserve = RESERVE;
		data_info_uart->data[0] = (uint8_t)(FW_VER >> 24); //���汾��
		data_info_uart->data[1] = (uint8_t)(FW_VER >> 16);
		data_info_uart->data[2] = (uint8_t)(FW_VER >> 8);	//�ΰ汾��
		data_info_uart->data[3] = (uint8_t)(FW_VER >> 0);
		data_info_uart->data[4] = (uint8_t)(FW_TYPE >> 24);//�̼�����
		data_info_uart->data[5] = (uint8_t)(FW_TYPE >> 16);
		data_info_uart->data[6] = (uint8_t)(FW_TYPE >> 8);
		data_info_uart->data[7] = (uint8_t)(FW_TYPE >> 0);
		
		/* ���ڻظ� */
		ack_to_cpu_uart_protocol(data_info_uart, 10, UART_PROTOCOL_PORT);
	}
	
	/* ִ�г�����ת */
	if (uart_cmd == cmd_list.excute)
	{
		exe_type = (data_info_uart->data[0] << 24) | (data_info_uart->data[1] << 16)
							|(data_info_uart->data[2] << 8) | (data_info_uart->data[3] << 0);
    if(exe_type == UART_BL_BOOT)
		{
			FLASH_Unlock();
      USART_BOOT_ErasePage(APP_EXE_FLAG_START_ADDR,APP_EXE_FLAG_START_ADDR);//����д�뵽Flash�е�APPִ�б�־����λ���к󣬼���ִ��Bootloader����
			FLASH_Lock();
			__set_PRIMASK(1);  //�ر������ж�
			NVIC_SystemReset();
		}
	}
}
	
static void handle_can_transmit(uint8_t *data, uint8_t len)
{
	CanTxMsg TxMessage;
	uint8_t i;
	uint8_t can_addr,can_cmd;
	uint32_t exe_type;
	uint32_t baund_rate;
	uint32_t addr_offset;
	static uint32_t data_size = 0,data_index = 0;
	__align(4) static uint8_t	data_temp[PAGE_SIZE + 2];
	
	/* �����̼�����flash��С�ռ�,ÿҳд�롣 */
	data_info_t *data_info_can = (data_info_t *)data;
	can_cmd = data_info_can->cmd;
	can_addr = data_info_can->option.addr;
	
	TxMessage.DLC = 0;
	TxMessage.ExtId = 0;  ////ID��bit0~bit3λΪ�����룬bit4~bit11λΪ�ڵ��ַ
	TxMessage.IDE = CAN_Id_Extended;
	TxMessage.RTR = CAN_RTR_Data;
	
	/* ����ƫ�����洢��data[0]��data[3],���ݴ�С�洢��data[4]��data[7] */
	if (can_cmd == cmd_list.write_info)
	{
		addr_offset = (data_info_can->data[0] << 24) | (data_info_can->data[1] << 16) 
								| (data_info_can->data[2] << 8) | (data_info_can->data[3] << 0);
		data_size = (data_info_can->data[4] << 24) | (data_info_can->data[5] << 16) 
							| (data_info_can->data[6] << 8) | (data_info_can->data[7] << 0);		
		data_index = 0;
		
		/* CAN����֡ת�� */
		TxMessage.ExtId = (can_addr << CMD_WIDTH) | can_cmd;
		TxMessage.Data[0] = (uint8_t)(addr_offset >> 24);
		TxMessage.Data[1] = (uint8_t)(addr_offset >> 16);
		TxMessage.Data[2] = (uint8_t)(addr_offset >> 8);
		TxMessage.Data[3] = (uint8_t)(addr_offset >> 0);
		TxMessage.Data[4] = (uint8_t)(data_size >> 24);
		TxMessage.Data[5] = (uint8_t)(data_size >> 16);
		TxMessage.Data[6] = (uint8_t)(data_size >> 8);
		TxMessage.Data[7] = (uint8_t)(data_size >> 0);
		TxMessage.DLC = 8;
		CAN_WriteData(&TxMessage);
	}
	
	/* ���չ̼����ݣ�������2KB�������CAN����֡���ת��������������У�� */
	if (can_cmd == cmd_list.write_bin)
	{
		if ((data_index < data_size) && (data_index < (PAGE_SIZE + 2)))
		{
			for (i = 0; i < (len - 2); i++)  //��ȥcmd��addr
			{
				data_temp[data_index++] = data_info_can->data[i];
			}
		}
		
		if ((data_index >= data_size) || (data_index >= (PAGE_SIZE + 2)))
		{
			uint16_t len_sub_integer = data_index / 8;
			uint16_t len_sub_remain = data_index % 8;
			uint16_t len_sub_total = len_sub_integer + (len_sub_remain > 0 ? 1 : 0);
			uint16_t package_num;
			
			TxMessage.ExtId = (can_addr << CMD_WIDTH) | can_cmd;
					
			for (package_num = 0; package_num < len_sub_total; package_num++)
			{
				memset(TxMessage.Data, 0, 8);
				if (package_num < len_sub_integer)
				{
					memcpy(TxMessage.Data, &data_temp[package_num * 8], 8);
					TxMessage.DLC = 8;
					CAN_WriteData(&TxMessage);
				}
				else
				{
					memcpy(TxMessage.Data, &data_temp[package_num * 8], len_sub_remain);
					TxMessage.DLC = len_sub_remain;
					CAN_WriteData(&TxMessage);
				}
			}
		}	
	}
	
	/* ���ò����� */
	if (data_info_can->cmd == cmd_list.set_baundrate)
	{
		baund_rate = (data_info_can->data[0] << 24) | (data_info_can->data[1] << 16) 
							| (data_info_can->data[2] << 8) | (data_info_can->data[3] << 0);
		
		/* CAN����֡͸�� */
		TxMessage.ExtId = (can_addr << CMD_WIDTH) | can_cmd;
		TxMessage.Data[0] = (uint8_t)(baund_rate >> 24);
		TxMessage.Data[1] = (uint8_t)(baund_rate >> 16);
		TxMessage.Data[2] = (uint8_t)(baund_rate >> 8);
		TxMessage.Data[3] = (uint8_t)(baund_rate >> 0);
		TxMessage.DLC = 4;
		CAN_WriteData(&TxMessage);
	}
	
	/* ��ѯ�汾�ź͹̼����� */
	if (data_info_can->cmd == cmd_list.check_version)
	{
		/* CAN����֡ת�� */
		TxMessage.ExtId = (can_addr << CMD_WIDTH) | can_cmd;
		TxMessage.DLC = 0;
		CAN_WriteData(&TxMessage);
	}
	
	/* ִ�г�����ת */
	if (data_info_can->cmd == cmd_list.excute)
	{
		exe_type = (data_info_can->data[0] << 24) | (data_info_can->data[1] << 16)
							|(data_info_can->data[2] << 8) | (data_info_can->data[3] << 0);
		
		/* CAN����֡͸�� */
		TxMessage.ExtId = (can_addr << CMD_WIDTH) | can_cmd;
		TxMessage.Data[0] = (uint8_t)(exe_type >> 24);
		TxMessage.Data[1] = (uint8_t)(exe_type >> 16);
		TxMessage.Data[2] = (uint8_t)(exe_type >> 8);
		TxMessage.Data[3] = (uint8_t)(exe_type >> 0);
		TxMessage.DLC = 4;
		CAN_WriteData(&TxMessage);
	}
}
	
static void handle_zigbee_transmit(uint8_t *data, uint8_t len)
{
}
	
static void handle_rs485_transmit(uint8_t *data, uint8_t len)
{
}
	
static void handle_i2c_transmit(uint8_t *data, uint8_t len)
{
}

static const protocol_entry_t package_items[] = 
{
	{UART_PROTOCOL_PORT, 				handle_uart_local},
	{CAN_PROTOCOL_PORT, 				handle_can_transmit},
	{ZIGBEE_PROTOCOL_PORT, 			handle_zigbee_transmit},
	{RS485_PROTOCOL_PORT, 			handle_rs485_transmit},
	{I2C_PROTOCOL_PORT, 				handle_i2c_transmit},
	{0xFF,											NULL},
};

static void parse_from_mcu_can_protocol(CanRxMsg *pRxMessage)
{
  uint8_t can_cmd = (pRxMessage->ExtId) & CMD_MASK;//ID��bit0~bit3λΪ������
  uint8_t can_addr = (pRxMessage->ExtId >> CMD_WIDTH);//ID��bit4~bit11λΪ�ڵ��ַ
	uint32_t addr_offset,data_size;
	
	data_info_t data_info_can = {0};
	
	if (can_cmd == cmd_list.check_version)
	{
		data_info_can.cmd = can_cmd | ACK_CMD;
		data_info_can.option.addr = can_addr;
		data_info_can.data[0] = pRxMessage->Data[0];
		data_info_can.data[1] = pRxMessage->Data[1];
		data_info_can.data[2] = pRxMessage->Data[2];
		data_info_can.data[3] = pRxMessage->Data[3];
		data_info_can.data[4] = pRxMessage->Data[4];
		data_info_can.data[5] = pRxMessage->Data[5];
		data_info_can.data[6] = pRxMessage->Data[6];
		data_info_can.data[7] = pRxMessage->Data[7];
		
		/* ���ڻظ� */
		ack_to_cpu_uart_protocol(&data_info_can, 10, CAN_PROTOCOL_PORT);
	}
	else /* ��cmd_list.check_version��ظ�һ�� */
	{
		data_info_can.cmd = can_cmd | ACK_CMD;
		data_info_can.option.addr = can_addr;
		data_info_can.data[0] = pRxMessage->Data[0];
		
		/* ���ڻظ� */
		ack_to_cpu_uart_protocol(&data_info_can, 3, CAN_PROTOCOL_PORT);
	}
}

static void parse_from_cpu_uart_protocol(uint8_t *data, uint8_t len)
{
	uint16_t crc = crc16_xmodem(data, len - 3);
	uint16_t crc16 = (uint16_t)(*(data + len - 3)) << 8 | *(data + len - 2);//��λ�ȷ�
	if (crc16 != crc) return;
	
	protocol_info_t protocol_info = {0};
	protocol_info.port	= data[2];
	memcpy(&protocol_info.data_info, data + 3, len - DETACH_DATA_INFO_SIZE);
	
	const protocol_entry_t *protocol_entry;
	for (protocol_entry = package_items; protocol_entry->handle != NULL; protocol_entry++)
	{
		if (protocol_info.port == protocol_entry->port)
		{
			protocol_entry->handle((uint8_t *)&protocol_info.data_info, len - DETACH_DATA_INFO_SIZE);
		}
	}
}

void receive_from_cpu_uart_protocol(uint8_t rx_data)
{
	static uint8_t rx_buf[256] = {0};
	static uint8_t data_len = 0,data_cnt = 0;
	static uint8_t state = 0;
	
	if (state == 0 && rx_data == HEAD) //head
	{
		state = 1;
		rx_buf[0] = rx_data;
	}
	else if (state == 1 && rx_data < 200) //len
	{
		state = 2;
		rx_buf[1] = rx_data;
		data_len = rx_data - DETACH_DATA_INFO_SIZE; //ȥ��head,len,port,crc,tail
		data_cnt = 0;
	}
	else if (state == 2) //port
	{
		state = 3;
		rx_buf[2] = rx_data;
	}
	else if (state == 3 && data_len > 0) //protocol data
	{
		rx_buf[3 + data_cnt++] = rx_data;
		data_len--;
		if (data_len == 0) state = 4;
	}
	else if (state == 4) //crc16_h
	{
		state = 5;
		rx_buf[3 + data_cnt++] = rx_data;
	}
	else if (state == 5) //crc16_l
	{
		state = 6;
		rx_buf[3 + data_cnt++] = rx_data;
	}
	else if (state == 6 && rx_data == TAIL) //tail
	{
		state = 0;
		rx_buf[3 + data_cnt++] = rx_data;
		parse_from_cpu_uart_protocol(rx_buf,3 + data_cnt);
	}
	else
	{
		state = 0;
	}
}

/* ������cpu�����Ĵ������ݣ���Ӧcpu */
void handle_usart_queue(void)
{
	uint8_t from_cpu_uart_data;
	if (USART_QUEUE_OK == UsartQueuePop(&usart1_send, &from_cpu_uart_data))
	{
		receive_from_cpu_uart_protocol(from_cpu_uart_data);
	}
}

/* ����F072��Ӧ��CAN����֡����Ӧcpu */
void handle_can_queue(void)
{
	can_frame_t from_mcu_can_data;
	if (CAN_OK == CanQueueRead(&can_queue_send, &from_mcu_can_data))
	{
		/* ���CAN����֡������Ϊ���ڸ�ʽ��ӦCPU */
		parse_from_mcu_can_protocol(&from_mcu_can_data);
	}
}
