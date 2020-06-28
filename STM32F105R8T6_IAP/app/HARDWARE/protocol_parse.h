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
	uint8_t erase;
	uint8_t write_info;
	uint8_t write_bin;
	uint8_t check_version;
	uint8_t set_baund;
	uint8_t excute;
	uint8_t cmd_success;
	uint8_t cmd_failed;
}cmd_list_t;


typedef __packed struct
{
	uint8_t cmd;
	uint8_t data[128];  //���ڷ���ÿ��bin�ļ�Ϊ1024����(����128 B)
}data_info_t;


typedef __packed struct
{
	uint8_t port;
	data_info_t data_info;
}protocol_info_t;

typedef struct
{
	uint8_t port;
	void (*handle)(uint8_t *data, uint8_t len);
}protocol_entry_t;


void prepare_protocol(uint8_t rx_data);
void handle_usart_queue(void);
void handle_can_queue(void);

#endif


