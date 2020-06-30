#include "main.h"
#include "string.h"

#define HEAD  					 0xAA
#define TAIL  					 0x55
#define ACK_CMD					 0xF0
#define DETACH_DATA_INFO_SIZE   6

#define UART_BL_BOOT     0x55555555
#define UART_BL_APP      0xAAAAAAAA
#define FW_TYPE          UART_BL_BOOT
#define FW_VER					 0x00010000		//v1.0
#define ACTIVE_REQUEST   0xFFFFFFFF

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
	uart_tx_buf[cnt++] = (uint8_t)(crc >> 8); //高位先发
	uart_tx_buf[cnt++] = (uint8_t)(crc & 0xFF);
	uart_tx_buf[cnt++] = TAIL;
	
	USART1_SendData(uart_tx_buf, cnt);  //不定长数据应答
}

void dev_active_request(void)
{
	data_info_t data_info_request = {0};
	data_info_request.cmd = cmd_list.request | ACK_CMD;
	data_info_request.data[0] = (uint8_t)(ACTIVE_REQUEST >> 24); 
	data_info_request.data[1] = (uint8_t)(ACTIVE_REQUEST >> 16);
	data_info_request.data[2] = (uint8_t)(ACTIVE_REQUEST >> 8);	 
	data_info_request.data[3] = (uint8_t)(ACTIVE_REQUEST >> 0); 
	ack_uart_protocol(&data_info_request, 6, UART_PROTOCOL_PORT);
}

static void handle_uart_protocol(uint8_t *data, uint8_t len)
{
	uint8_t ret,i;
	uint8_t uart_reserve,uart_cmd;
	uint16_t crc_data;
	uint32_t exe_type;
	uint32_t baund_rate;
	uint32_t addr_offset;
	static uint32_t data_size = 0,data_index = 0;
	static uint32_t start_addr = APP_START_ADDR;
	__align(4) static uint8_t	data_temp[PAGE_SIZE + 2];
	
	/* 擦除固件所需flash大小空间,每页写入。 */
	data_info_t *data_info_p = (data_info_t *)data;
	uart_cmd = data_info_p->cmd;
	uart_reserve = data_info_p->option.reserve;
	
	/* addr_offset:按page首地址偏移
		 start_addr:整page首地址
		 data_size: 整page大小写入，不足1 page的写入剩余data_size */
	if (uart_cmd == cmd_list.write_info)
	{
		addr_offset = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16) 
							| (data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
		start_addr = APP_START_ADDR + addr_offset;
	
		data_size = (data_info_p->data[4] << 24) | (data_info_p->data[5] << 16) 
							| (data_info_p->data[6] << 8) | (data_info_p->data[7] << 0);
		data_index = 0;
		
		__set_PRIMASK(1);
		FLASH_Unlock();
		ret = USART_BOOT_ErasePage(start_addr, start_addr);
    FLASH_Lock();
		__set_PRIMASK(0);
		
		if (ret == FLASH_COMPLETE)
		{
			data_info_p->cmd |= ACK_CMD;
			data_info_p->data[0] = cmd_list.cmd_success;
		}
		else
		{
			data_info_p->cmd |= ACK_CMD;
			data_info_p->data[0] = cmd_list.cmd_failed;
		}
		
		/* 串口回复 */
		ack_uart_protocol(data_info_p, 3, UART_PROTOCOL_PORT);
	}
	
	/* 持续接收完整包完毕后，验证正确后写入 */
	if (uart_cmd == cmd_list.write_bin)
	{
		if ((data_index < data_size) && (data_index < (PAGE_SIZE + 2)))
		{
			for (i = 0; i < (len - 2); i++)  //减去cmd，reserve
			{
				data_temp[data_index++] = data_info_p->data[i];
			}
		}
		
		if ((data_index >= data_size) || (data_index >= (PAGE_SIZE + 2)))
		{
			crc_data = crc16_xmodem(data_temp,data_size - 2);//对接收到的数据做CRC校验，保证数据完整性
      if(crc_data == ((data_temp[data_size - 2] << 8) | (data_temp[data_size - 1]))) //高位先发
			{
				__set_PRIMASK(1);
				FLASH_Unlock();
				ret = USART_BOOT_ProgramDatatoFlash(start_addr,data_temp,data_size - 2);
				FLASH_Lock();
				__set_PRIMASK(0);
				
				if (ret == FLASH_COMPLETE)
				{
					/* 对写入的数据再次做CRC校验，保证数据完整性. */
					crc_data = crc16_xmodem((const unsigned char *)(start_addr),data_size - 2);
					if (crc_data == ((data_temp[data_size - 2] << 8) | (data_temp[data_size - 1])))
					{
						data_info_p->cmd |= ACK_CMD;
						data_info_p->data[0] = cmd_list.cmd_success;
					}
					else	//写入flash后重校验失败，重发此包。
					{
						data_info_p->cmd |= ACK_CMD; 
						data_info_p->data[0] = cmd_list.cmd_failed;					
					}
					
					/* 串口回复 */
					ack_uart_protocol(data_info_p, 3, UART_PROTOCOL_PORT);					
				}	
				else //未写入flash，重发此包。
				{
					data_info_p->cmd |= ACK_CMD;
					data_info_p->data[0] = cmd_list.cmd_failed;
					
					/* 串口回复 */
					ack_uart_protocol(data_info_p, 3, UART_PROTOCOL_PORT);
				}
			}
			else //接收数据校验失败，未写入flash，重发此包。
			{
				data_info_p->cmd |= ACK_CMD;
				data_info_p->data[0] = cmd_list.cmd_failed;
				
				/* 串口回复 */
				ack_uart_protocol(data_info_p, 3, UART_PROTOCOL_PORT);
			}
		}
	}
	
	/* 设置波特率 */
	if (uart_cmd == cmd_list.set_baundrate)
	{
		baund_rate = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16) 
							| (data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
		
		data_info_p->cmd |= ACK_CMD;
		data_info_p->data[0] = cmd_list.cmd_success;
		
		/* 串口回复 */
		ack_uart_protocol(data_info_p, 3, UART_PROTOCOL_PORT);
		
		USART1_Init(baund_rate);  //在未改变波特率前将应答发出，修改正确波特率后通信。
	}
	
	/* 查询版本号和固件类型 */
	if (uart_cmd == cmd_list.check_version)
	{
		data_info_p->cmd |= ACK_CMD;
		data_info_p->data[0] = (uint8_t)(FW_VER >> 24); //主版本号
		data_info_p->data[1] = (uint8_t)(FW_VER >> 16);
		data_info_p->data[2] = (uint8_t)(FW_VER >> 8);	//次版本号
		data_info_p->data[3] = (uint8_t)(FW_VER >> 0);
		data_info_p->data[4] = (uint8_t)(FW_TYPE >> 24);//固件类型
		data_info_p->data[5] = (uint8_t)(FW_TYPE >> 16);
		data_info_p->data[6] = (uint8_t)(FW_TYPE >> 8);
		data_info_p->data[7] = (uint8_t)(FW_TYPE >> 0);
		
		/* 串口回复 */
		ack_uart_protocol(data_info_p, 10, UART_PROTOCOL_PORT);
	}
	
	if (uart_cmd == cmd_list.excute)
	{
		exe_type = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16)
							|(data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
    if(exe_type == UART_BL_APP)
		{
      if((*((uint32_t *)APP_START_ADDR) != 0xFFFFFFFF))
			{
        USART_BOOT_JumpToApplication(APP_START_ADDR);
      }
    }
	}
}
	
static void handle_can_protocol(uint8_t *data, uint8_t len)
{
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
	uint16_t crc16 = (uint16_t)(*(data + len - 3)) << 8 | *(data + len - 2);//高位先发
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
		data_len = rx_data - DETACH_DATA_INFO_SIZE; //去除head,len,port,crc,tail
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

/* 处理由cpu发来的串口队列数据 */
void handle_usart_queue(void)
{
	uint8_t cpu_send_mcu_uart_data;
	if (USART_QUEUE_OK == UsartQueuePop(&usart1_send, &cpu_send_mcu_uart_data))
	{
		prepare_protocol(cpu_send_mcu_uart_data);
	}
}

