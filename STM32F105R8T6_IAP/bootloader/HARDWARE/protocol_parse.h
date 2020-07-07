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
	uint8_t data[128];  //���ڷ���ÿ��bin����������1024����(len���Ƴ��ȣ�����128 Byte)
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

void JumpFirmwareSuccess(void);
void HandleUsartQueue(void);
void handle_can_queue(void);

#endif


