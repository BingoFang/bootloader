#ifndef __PROTOCOL_PARSE_H__
#define __PROTOCOL_PARSE_H__

#include "sys.h"

typedef enum
{
	UART_PROTOCOL_PORT = 0x01,
	CAN_PROTOCOL_PORT,
	ZIGBEE_PROTOCOL_PORT,
	RS485_PROTOCOL_PORT,
	I2C_PROTOCOL_PORT,
}PROTOCOL_TYPE;

typedef struct
{
	uint8_t write_info;
	uint8_t write_bin;
	uint8_t check_version;
	uint8_t set_baundrate;
	uint8_t excute;
	uint8_t request;
	uint8_t cmd_success;
	uint8_t cmd_failed;
}cmd_list_t;


typedef  struct
{
	uint8_t cmd;
	union
	{
		uint8_t reserve;
		uint8_t addr;
	}option;
	uint8_t data[128];  //串口发送每包bin文件为1024整除(建议128 B)
}data_info_t;


typedef struct
{
	uint8_t port;
	data_info_t data_info;
}protocol_info_t;

typedef struct
{
	uint8_t port;
	void (*handle)(uint8_t *data, uint8_t len);
}protocol_entry_t;

void dev_active_request(void);
void receive_from_cpu_uart_protocol(uint8_t rx_data);
void handle_usart_queue(void);
void handle_can_queue(void);

#endif


