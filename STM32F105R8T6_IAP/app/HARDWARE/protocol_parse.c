#include "main.h"
#include "string.h"

#define HEAD  					 0xAA
#define TAIL  					 0x55
#define DETACH_DATA_INFO_SIZE   6
#define ACK_CMD					 0xF0

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
static void ack_uart_protocol(data_info_t *data,uint8_t len, uint8_t port)
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
	
	USART1_Send_Data(uart_tx_buf, cnt);  //����������Ӧ��
}


static void handle_uart_protocol(uint8_t *data, uint8_t len)
{
	uint8_t uart_reserve,uart_cmd;
	uint32_t exe_type;
	uint32_t baund_rate;

	/* �����̼�����flash��С�ռ�,ÿҳд�롣 */
	data_info_t *data_info_p = (data_info_t *)data;
	uart_cmd = data_info_p->cmd;
	uart_reserve = data_info_p->option.reserve;
	
	/* ���ò����� */
	if (uart_cmd == cmd_list.set_baundrate)
	{
		__set_PRIMASK(1);
		baund_rate = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16) 
							| (data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
		__set_PRIMASK(0);
		
		data_info_p->cmd |= ACK_CMD;
		data_info_p->data[0] = cmd_list.cmd_success;
		
		/* ���ڻظ� */
		ack_uart_protocol(data_info_p, 3, UART_PROTOCOL_PORT);
		
		USART1_Init(baund_rate);  //��δ�ı䲨����ǰ��Ӧ�𷢳����޸���ȷ�����ʺ�ͨ�š�
	}
	
	if (uart_cmd == cmd_list.check_version)
	{
		data_info_p->cmd |= ACK_CMD;
		data_info_p->data[0] = (uint8_t)(FW_VER >> 24); //���汾��
		data_info_p->data[1] = (uint8_t)(FW_VER >> 16);
		data_info_p->data[2] = (uint8_t)(FW_VER >> 8);	//�ΰ汾��
		data_info_p->data[3] = (uint8_t)(FW_VER >> 0);
		data_info_p->data[4] = (uint8_t)(FW_TYPE >> 24);//�̼�����
		data_info_p->data[5] = (uint8_t)(FW_TYPE >> 16);
		data_info_p->data[6] = (uint8_t)(FW_TYPE >> 8);
		data_info_p->data[7] = (uint8_t)(FW_TYPE >> 0);
		
		/* ���ڻظ� */
		ack_uart_protocol(data_info_p, 10, UART_PROTOCOL_PORT);
	}
	
	if (uart_cmd == cmd_list.excute)
	{
		exe_type = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16)
							|(data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
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
	
static void handle_can_protocol(uint8_t *data, uint8_t len)
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
	data_info_t *data_info_p = (data_info_t *)data;
	can_cmd = data_info_p->cmd;
	can_addr = data_info_p->option.addr;
	
	TxMessage.DLC = 0;
	TxMessage.ExtId = 0;
	TxMessage.IDE = CAN_Id_Extended;
	TxMessage.RTR = CAN_RTR_Data;
	
	/* ����ƫ�����洢��data[0]��data[3],���ݴ�С�洢��data[4]��data[7] */
	if (can_cmd == cmd_list.write_info)
	{
		addr_offset = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16) 
							| (data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
		data_size = (data_info_p->data[4] << 24) | (data_info_p->data[5] << 16) 
							| (data_info_p->data[6] << 8) | (data_info_p->data[7] << 0);		
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
	
	
	/* ���պ����ݺ�CAN�����������ݣ�����������У�� */
	if (can_cmd == cmd_list.write_bin)
	{
		if ((data_index < data_size) && (data_index < (PAGE_SIZE + 2)))
		{
			for (i = 0; i < (len - 2); i++)  //��ȥcmd��addr
			{
				data_temp[data_index++] = data_info_p->data[i];
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
	if (data_info_p->cmd == cmd_list.set_baundrate)
	{
		baund_rate = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16) 
							| (data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
		
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
	if (data_info_p->cmd == cmd_list.check_version)
	{
		/* CAN����֡ת�� */
		TxMessage.ExtId = (can_addr << CMD_WIDTH) | can_cmd;
		TxMessage.DLC = 0;
		CAN_WriteData(&TxMessage);
	}
	
	if (data_info_p->cmd == cmd_list.excute)
	{
		exe_type = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16)
							|(data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
		
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
	
static void handle_zigbee_protocol(uint8_t *data, uint8_t len)
{
}
	
static void handle_rs485_protocol(uint8_t *data, uint8_t len)
{
}
	
static void handle_i2c_protocol(uint8_t *data, uint8_t len)
{
}

static const protocol_entry_t package_items[] = 
{
	{UART_PROTOCOL_PORT, 				handle_uart_protocol},
	{CAN_PROTOCOL_PORT, 				handle_can_protocol},
	{ZIGBEE_PROTOCOL_PORT, 			handle_zigbee_protocol},
	{RS485_PROTOCOL_PORT, 			handle_rs485_protocol},
	{I2C_PROTOCOL_PORT, 				handle_i2c_protocol},
	{0xFF,											NULL},
};

static void prase_protocol(uint8_t *data, uint8_t len)
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

void prepare_protocol(uint8_t rx_data)
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
		prase_protocol(rx_buf,3 + data_cnt);
	}
	else
	{
		state = 0;
	}
}

/* ������cpu�����Ĵ��ڶ������� */
void handle_usart_queue(void)
{
	uint8_t cpu_send_mcu_uart_data;
	if (USART_QUEUE_OK == UsartQueuePop(&usart1_send, &cpu_send_mcu_uart_data))
	{
		prepare_protocol(cpu_send_mcu_uart_data);
	}
}

/* ����072�ظ���can�������ݣ�ͨ��uart����cpu */
void handle_can_queue(void)
{
	can_frame_t mcu_recv_mcu_can_data;
	if (CAN_OK == CanQueueRead(&can_queue_send, &mcu_recv_mcu_can_data))
	{
		/* ����֡���ϵ�protocol�ַ���ʽ��,���ڻظ� */
		ack_uart_protocol((data_info_t *)&mcu_recv_mcu_can_data, sizeof(mcu_recv_mcu_can_data), CAN_PROTOCOL_PORT);
	}
}