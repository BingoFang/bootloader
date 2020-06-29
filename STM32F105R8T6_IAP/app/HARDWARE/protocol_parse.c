#include "main.h"
#include "string.h"

#define HEAD  					 0xAA
#define TAIL  					 0x55
#define DETACH_DATA_INFO_SIZE   6
#define ACK_CMD					 0xF0

#define UART_BL_BOOT     0x55555555
#define UART_BL_APP      0xAAAAAAAA
#define FW_TYPE          UART_BL_APP
#define FW_VER					 0x00010001		//v1.1

cmd_list_t cmd_list = 
{
	.write_info 		= 0x01,
	.write_bin 			= 0x02,
	.check_version 	= 0x03,
	.set_baund 			= 0x04,
	.excute 				= 0x05,
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
	
	USART1_Send_Data(uart_tx_buf, cnt);  //不定长数据应答
}


static void handle_uart_protocol(uint8_t *data, uint8_t len)
{
	uint32_t exe_type;
	uint32_t baund_rate;

	/* 擦除固件所需flash大小空间,每页写入。 */
	data_info_t *data_info_p = (data_info_t *)data;
	
	/* 设置波特率 */
	if (data_info_p->cmd == cmd_list.set_baund)
	{
		__set_PRIMASK(1);
		baund_rate = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16) 
							| (data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
		__set_PRIMASK(0);
		
		data_info_p->cmd |= ACK_CMD;
		data_info_p->data[0] = cmd_list.cmd_success;
		
		/* 串口回复 */
		ack_uart_protocol(data_info_p, 2, UART_PROTOCOL_PORT);
		
		USART1_Init(baund_rate);  //在未改变波特率前将应答发出，修改正确波特率后通信。
	}
	
	if (data_info_p->cmd == cmd_list.check_version)
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
		ack_uart_protocol(data_info_p, 9, UART_PROTOCOL_PORT);
	}
	
	if (data_info_p->cmd == cmd_list.excute)
	{
		exe_type = (data_info_p->data[0] << 24) | (data_info_p->data[1] << 16)
							|(data_info_p->data[2] << 8) | (data_info_p->data[3] << 0);
    if(exe_type == UART_BL_BOOT)
		{
			FLASH_Unlock();
      USART_BOOT_ErasePage(APP_EXE_FLAG_START_ADDR,APP_EXE_FLAG_START_ADDR);//擦除写入到Flash中的APP执行标志，复位运行后，即可执行Bootloader程序
			FLASH_Lock();
			__set_PRIMASK(1);  //关闭所有中断
			NVIC_SystemReset();
		}
	}
}
	
static void handle_can_protocol(uint8_t *data, uint8_t len)
{
	/* can协议内容不含有cmd，直接为can数据帧格式 */
	CanTxMsg *TxMessage;
	TxMessage = (CanTxMsg *)data;
	
	/* 串口接收数据直接can透传转发 */
	CAN_WriteData(TxMessage);
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

/* 处理072回复的can队列数据，通过uart发回cpu */
void handle_can_queue(void)
{
	can_frame_t mcu_recv_mcu_can_data;
	if (CAN_OK == CanQueueRead(&can_queue_send, &mcu_recv_mcu_can_data))
	{
		/* 数据帧整合到protocol字符格式中,串口回复 */
		ack_uart_protocol((data_info_t *)&mcu_recv_mcu_can_data, sizeof(mcu_recv_mcu_can_data), CAN_PROTOCOL_PORT);
	}
}
